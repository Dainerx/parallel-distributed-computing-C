#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>
#include <stdarg.h>
#include <errno.h>
#include "matrix_util.h"
#include "solver.h"
#include "metrics.h"
#include "cmd.h"
#include "strassen.h"

int main(int argc, char *argv[])
{

  // Si c'est le root; on va executer tous les solvers qui ne sont pas distriubés
  struct CmdInput ci = read_input(argc, argv);
  /*
      Vérifier la syntaxe ainsi que le contenu de la ligne de commande et afficher un message d'erreur en cas d'erreur
      exemple: si un flag n'a pas été renseigné ou les valeurs sont négatives ou le nombre de colonnes de la matrice A
      est different du nombre de lignes de la matrice B, un message d'erreur personnalisé est affiché
      La commande est: -a 700 -b 900 -c 900 -d 600 -n 25
    */
  if (check_input(ci) == false)
  {
    return 1;
  }
  print_colored(2, "    OK: Input valid.");
  print_colored(0, "\n    A: (%d,%d)\n    B: (%d,%d)\n", ci.lines_a, ci.columns_a, ci.lines_b, ci.columns_b);
  int num_threads = ci.num_threads;
  print_colored(0, "    Threads number: %d \n", num_threads);

  // Allocation des matrice A, B et C
  int **mat_A = malloc_mat(ci.lines_a, ci.columns_a);
  int **mat_B = malloc_mat(ci.lines_b, ci.columns_b);
  // La matrice mat_C_Seq aura toujours le résultat de la multiplication correcte.
  // On va l'utiliser pour savoir si les autres solvers ont un résultat correct ou non.
  int **mat_C_Seq = malloc_mat(ci.lines_a, ci.columns_b);
  // La matrice mat_C_Other par contre elle est variante, elle sera le résultat de chaque
  // solver.
  int **mat_C_Other = malloc_mat(ci.lines_a, ci.columns_b);

  const char *labels[4] = {"sequential native", "sequential strassen", "parallel native", "parallel optimized"};
  float **metrics = malloc_matf(4, 4);
  srand(time(NULL));

  // Remlissage des matrices A et B
  fill_mat(mat_A, ci.lines_a, ci.columns_a);
  fill_mat(mat_B, ci.lines_b, ci.columns_b);

  //Initialiser la structure input par le nombre de lignes et de colonnes des matrices A et B
  init_solver(ci.lines_a, ci.columns_a, ci.lines_b, ci.columns_b);

  // Multiplication séquentielle des matrices A et B
  double cpu_time_used_seq;
  cpu_time_used_seq = sequential_mult(mat_A, mat_B, mat_C_Seq);
  // Mettre les résultats des metriques pour le calcul séquentiel dans la matrice metrics
  metrics[0][0] = cpu_time_used_seq;
  metrics[0][1] = 1;
  metrics[0][2] = 1;
  metrics[0][3] = 1;

  // Multiplication séquentielle en réduisant le nombre de multiplication.
  // SI LES MATRICES NE SONT PAS CARRÉES
  double cpu_time_used_strassen;
  int max = get_max(get_max(ci.lines_a, ci.columns_a), ci.columns_b);
  max = get_power_2(max);
  int **mat_A_squared = malloc_mat(max, max);
  int **mat_B_squared = malloc_mat(max, max);
  int **mat_C_squared = malloc_mat(max, max);

  make_square(mat_A_squared, mat_A, ci.lines_a, ci.columns_a, max);
  make_square(mat_B_squared, mat_B, ci.lines_b, ci.columns_b, max);
  cpu_time_used_strassen = strassen_mult_flat(mat_A_squared, mat_B_squared, mat_C_squared, mat_C_Other, max);
  if (equal_mats(ci.lines_a, ci.columns_b, mat_C_Seq, mat_C_Other) == false)
  {
    print_colored(1, "Multiplication séquentielle en réduisant le nombre de multiplication has produced a wrong result.\n");
  }
  free_mat(mat_A_squared, max);
  free_mat(mat_B_squared, max);
  free_mat(mat_C_squared, max);

  metrics[1][0] = cpu_time_used_strassen;
  metrics[1][1] = 1;
  metrics[1][2] = 1;
  metrics[1][3] = 1;

  // Multiplication en parallèle version naive des matrices A et B (Omp)
  fill_mat(mat_C_Other, ci.lines_a, ci.columns_b); // Réinitialissation des lignes de cache.
  double cpu_time_used_parallel = parallel_mult(num_threads, mat_A, mat_B, mat_C_Other);
  if (equal_mats(ci.lines_a, ci.columns_b, mat_C_Seq, mat_C_Other) == false)
  {
    print_colored(1, "Multiplication en parallèle version naive has produced a wrong result.\n");
  }
  // Mettre les résultats des metriques pour le calcul en parallèle version naive dans la matrice metrics
  metrics[2][0] = cpu_time_used_parallel;
  metrics[2][1] = speedup(cpu_time_used_seq, cpu_time_used_parallel);
  metrics[2][2] = efficiency(cpu_time_used_seq, cpu_time_used_parallel, num_threads);
  metrics[2][3] = cost(cpu_time_used_parallel, num_threads);

  // Multiplication en parallèle version optimisée des matrices A et B
  fill_mat(mat_C_Other, ci.lines_a, ci.columns_b); // Réinitialissation des lignes de cache.
  cpu_time_used_parallel = optimized_parallel_multiply(num_threads, mat_A, mat_B, mat_C_Other);
  if (equal_mats(ci.lines_a, ci.columns_b, mat_C_Seq, mat_C_Other) == false)
  {
    print_colored(1, "Multiplication en parallèle version optimisée has produced a wrong result.\n");
  }

  // Mettre les résultats des metriques pour le calcul en parallèle version optimisée dans la matrice metrics
  metrics[3][0] = cpu_time_used_parallel;
  metrics[3][1] = speedup(cpu_time_used_seq, cpu_time_used_parallel);
  metrics[3][2] = efficiency(cpu_time_used_seq, cpu_time_used_parallel, num_threads);
  metrics[3][3] = cost(cpu_time_used_parallel, num_threads);
  // Tout est bon, faut afficher les metrics.
  // Afficher les metriques pour chaque solveur

  print_colored(2, "\n    Finished computing.\n    Metrics:\n");
  print_metrics(labels, metrics);

  // Désallocation des matrice pour éviter les fuites de mémoire.
  free_mat(mat_A, ci.lines_a);
  free_mat(mat_B, ci.lines_b);
  free_mat(mat_C_Seq, ci.lines_a);
  free_mat(mat_C_Other, ci.lines_a);
  free_mat(metrics, 4);
  return 0;
}
