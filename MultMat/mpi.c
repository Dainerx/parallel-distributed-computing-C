#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>
#include <mpi.h>
#include <time.h>
#include "mpi_utils.h"
#include "matrix_util.h"
#include "metrics.h"
#include "solver.h"

// Variable globale contient l'input
struct CmdInput ci;

void convertMat(int **matrixA, int **matrixB, int *a, int *b)
{
  int i;

  for (i = 0; i < ci.lines_a; i++)
  {
    for (int j = 0; j < ci.columns_a; j++)
    {
      a[i * ci.columns_a + j] = matrixA[i][j];
    }
  }

  for (int i = 0; i < ci.lines_b; i++)
  {
    for (int j = 0; j < ci.columns_b; j++)
    {
      b[j * ci.lines_b + i] = matrixB[i][j];
    }
  }
}

void compute_local_c(int *res, int *receive_lineA, int *flatB)
{
  int k = 0;
  int c = 0;
  int t;
  while (k < ci.lines_b * ci.columns_b)
  {
    t = 0;
    for (int i = 0; i < ci.columns_a; i++)
    {
      t += receive_lineA[i] * flatB[i + k];
    }
    res[c] = t;
    k += ci.lines_b;
    c += 1;
  }
}
void display_linear_mat(int *mat, int lines, int columns)
{
  for (int i = 0; i < lines * columns; i++)
  {
    if (i % columns == 0)
    {
      printf("\n");
    }
    printf("%d\t", mat[i]);
  }
}
int main(int argc, char **argv)
{
  const int root = 0;
  int rank, world_size, tag = 0;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  ci = read_input(rank, argc, argv);
  if (check_input(rank, ci) == false)
  {
    MPI_Finalize();
    return EXIT_SUCCESS;
  }

  print_colored(rank, 2, "    OK: Input valid.");
  print_colored(rank, 0, "\n    A: (%d,%d)\n    B: (%d,%d)\n", ci.lines_a, ci.columns_a, ci.lines_b, ci.columns_b);

  const char *labels[2] = {"seq native", "distributed"};
  double metrics[2][4];

  int **mat_A;
  int **mat_B;
  int **mat_C_Seq;

  int *flatA;
  int *flatB = malloc((ci.lines_b * ci.columns_b) * sizeof(int));
  int *flatC; //Final result in this array

  double start;
  if (rank == root)
  {
    mat_A = malloc_mat(ci.lines_a, ci.columns_a);
    mat_B = malloc_mat(ci.lines_b, ci.columns_b);
    srand(time(NULL) + rank); // uncessary + rank
    fill_mat(mat_A, ci.lines_a, ci.columns_a);
    fill_mat(mat_B, ci.lines_b, ci.columns_b);

    // La matrice mat_C_Seq aura toujours le résultat de la multiplication correcte.
    // On va l'utiliser pour savoir si les autres solvers ont un résultat correct ou non.
    // Multiplication séquentielle des matrices A et B
    mat_C_Seq = malloc_mat(ci.lines_a, ci.columns_b);
    init_solver(ci.lines_a, ci.columns_a, ci.lines_b, ci.columns_b);
    metrics[0][0] = sequential_mult(mat_A, mat_B, mat_C_Seq);
    metrics[0][1] = 1;
    metrics[0][2] = 1;
    metrics[0][3] = 1;

    start = MPI_Wtime();
    flatA = malloc((ci.lines_a * ci.columns_a) * sizeof(int));
    convertMat(mat_A, mat_B, flatA, flatB);
    // Le root va allouer la taille mémoire pour contenir le résultat de la multiplication
    flatC = malloc((ci.lines_a * ci.columns_b) * sizeof(int));
  }
  // Envoie de la matrice B a tout le monde; B sera partagée
  MPI_Bcast(flatB, ci.lines_b * ci.columns_b, MPI_INT, root, MPI_COMM_WORLD);

  // Chacun alloue un tableau de taille ci.columns_a pour recevoir sa partie de la matrice A
  // chacun reçoi une ligne de la matrice A
  int *receive_lineA = (int *)malloc(ci.columns_a * sizeof(int));
  if (receive_lineA)
  {
    MPI_Scatter(flatA, ci.columns_a, MPI_INT, receive_lineA, ci.columns_a, MPI_INT, root, MPI_COMM_WORLD);
  }

  // Chacun dispose désormais de la matrice B en 1D (B est partagée) et une ligne de la matrice A
  // Chacun doit avoir comme résultat une ligne de la matrice C
  // Ensuite tout le monde envoie sa partie calculée au root (gather)
  int *local_c = (int *)malloc(ci.columns_b * sizeof(int));

  // Chacun calcule la ligne de la matrice C
  compute_local_c(local_c, receive_lineA, flatB);
  MPI_Gather(local_c, ci.columns_b, MPI_INT, flatC, ci.columns_b, MPI_INT, root, MPI_COMM_WORLD);
  free(local_c);

  if (rank == root)
  {
    double end = MPI_Wtime();
    double cpu_time_used_distributed = end - start;
    metrics[1][0] = cpu_time_used_distributed;
    metrics[1][1] = 1;
    metrics[1][2] = 1;
    metrics[1][3] = 1;
    print_metrics(labels, metrics, 2);
  }

  free(flatB);
  if (rank == root)
  {
    free_mat(mat_A, ci.lines_a);
    free_mat(mat_B, ci.lines_b);
    free_mat(mat_C_Seq, ci.lines_a);
    free(flatA);
  }
  MPI_Finalize();
  return EXIT_SUCCESS;
}
