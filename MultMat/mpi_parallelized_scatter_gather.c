#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <omp.h>
#include <time.h>
#include "mpi_utils.h"
#include "matrix_util.h"
#include "metrics.h"
#include "solver.h"

// Variable globale contient l'input
struct CmdInputDistributed ci;

/*
  Cette fonction permet de faire une multiplication séquentielle de deux matrices A et B
*/
double mpi_mult_seq(int **mat_A, int **mat_B, int **mat_C)
{
  double start, end, cpu_time_used;
  start = omp_get_wtime();
  int sum;
  for (int i = 0; i < ci.lines_a; i++)
  {
    for (int j = 0; j < ci.columns_b; j++)
    {
      sum = 0;
      for (int k = 0; k < ci.columns_a; k++)
      {
        sum += (mat_A[i][k] * mat_B[k][j]);
      }
      mat_C[i][j] = sum;
    }
  }
  end = omp_get_wtime();
  cpu_time_used = (end - start);
  return cpu_time_used;
}

/*
  Cette fonction permet de convertir des matrices de deux dimensions en une dimension
  Elle prend en paramètre deux matrices A et B, La matrice A va être transformée en une matrice 1D présentée ligne après ligne
  Et la matrice B va être transformée en une matrice 1D présentée colonne après colonne
*/
void convertMat_parralel(int **matrixA, int **matrixB, int *a, int *b)
{
  int i, j;
#pragma omp parallel
  {
#pragma omp parallel for schedule(guided) private(j) shared(a)
    for (i = 0; i < ci.lines_a; i++)
    {
      for (j = 0; j < ci.columns_a; j++)
      {
        a[i * ci.columns_a + j] = matrixA[i][j];
      }
    }
#pragma omp parallel for schedule(guided) private(j) shared(b)
    for (i = 0; i < ci.lines_b; i++)
    {
      for (j = 0; j < ci.columns_b; j++)
      {
        b[j * ci.lines_b + i] = matrixB[i][j];
      }
    }
  }
}

/*
   Cette fontion permet de faire la multiplication d'une partie de la matrice A reçue par chaque machine avec la matrice B connue par toutes les machines
 */
void get_res_lines_C(int *local_res, int *received_part_A, int *flatB, int nbr_linesA_received)
{
  int k = 0;
  int c = 0;
  int t, i;
  int sizeLineA = 0;
  for (int linesA = 0; linesA < nbr_linesA_received; linesA++)
  {
    for (k = 0; k < ci.lines_b * ci.columns_b; k += ci.lines_b)
    {
      t = 0;
      for (i = 0; i < ci.columns_a; i++)
      {
        t += received_part_A[i + sizeLineA] * flatB[i + k];
      }
      local_res[c] = t;
      c += 1;
    }
    sizeLineA += ci.columns_a;
  }
}

int main(int argc, char **argv)
{
  const int root = 0;
  int rank, world_size;
  int rc = 0;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  ci = read_input(rank, world_size, argc, argv);
  if (check_input(rank, ci) == false)
  {
    MPI_Abort(MPI_COMM_WORLD, rc);
    exit(1);
  }
  else
  {
    print_colored(rank, 2, "    OK: Input valid.");
    print_colored(rank, 0, "\n    A: (%d,%d)\n    B: (%d,%d)\n", ci.lines_a, ci.columns_a, ci.lines_b, ci.columns_b);
  }

  srand(time(NULL) + rank);
  int **mat_A;
  int **mat_B;
  int *flatA;
  int *res_final;
  int **mat_C;
  double time_used_seq;

  double start, end, time_used_mpi;
  if ((ci.lines_a * ci.columns_a) % world_size != 0)
  {
    printf("Le nombre de machines doit etre divisible par LINES_A * COLUMNS_A\n");
    return -1;
  }
  // S'assurer que chaque machine reçoie un certain nombre de lignes de la matrice A et non juste une partie d'une ligne
  if ((ci.lines_a * ci.columns_a / world_size) % ci.columns_a != 0)
  {
    printf(" lingesA * colunnesA /nombre de machines doit etre divisible sur colonnesA\n");
    return -2;
  }
  int *flatB = malloc((ci.lines_b * ci.columns_b) * sizeof(int));

  // Le machine root va allouer et initialiser les deux matrices A et B. Ensuite, elle va distribuer la matrice B à toutes les autres machines, et envoie
  // pour chaque machine sa partie de la matrice A
  if (rank == root)
  {
    mat_A = malloc_mat(ci.lines_a, ci.columns_a);
    mat_B = malloc_mat(ci.lines_b, ci.columns_b);
    mat_C = malloc_mat(ci.lines_a, ci.columns_b);

    fill_mat(mat_A, ci.lines_a, ci.columns_a);
    fill_mat(mat_B, ci.lines_b, ci.columns_b);

    time_used_seq = mpi_mult_seq(mat_A, mat_B, mat_C);
    start = MPI_Wtime();

    flatA = malloc((ci.lines_a * ci.columns_a) * sizeof(int));
    // Conversion des matrices en une dimension
    //double start1 = MPI_Wtime();
    //convertMat(mat_A, mat_B, flatA, flatB);
    //double end1 = MPI_Wtime();
    //double start2 = MPI_Wtime();
    convertMat_parralel(mat_A, mat_B, flatA, flatB);
    //double end2 = MPI_Wtime();
    //printf("convert mat speed: %f\n", (end1 - start1) / (end2 - start2));
    // Le root va allouer la taille mémoire pour contenir le résultat de la multiplication
    res_final = malloc((ci.lines_a * ci.columns_b) * sizeof(int));
  }

  // Toutes les machines vont attendre que la machine root fini de remplir les matrices A et B
  MPI_Barrier(MPI_COMM_WORLD);

  // Envoie de la matrice B a tout le monde par la machine root; B sera partagée
  MPI_Bcast(flatB, ci.lines_b * ci.columns_b, MPI_INT, root, MPI_COMM_WORLD);

  int length_revceived_A = ci.columns_a * ci.lines_a / world_size;
  // Toutes les machines vont allouer un espace pour recevoir leur partie de la matrice A
  int *received_part_A = (int *)malloc(length_revceived_A * sizeof(int));

  // Le root envoie à toutes les machines sa part de la matrice A y compris lui
  if (received_part_A)
  {
    MPI_Scatter(flatA, length_revceived_A, MPI_INT, received_part_A, length_revceived_A, MPI_INT, root, MPI_COMM_WORLD);
  }
  int nbr_linesA_received = length_revceived_A / ci.columns_a;

  // Chaque machine dispose désormais de la matrice B en 1D (B est partagée) et un nombre x de lignes de la matrice A
  // Chacune doit avoir comme résultat x lignes de la matrice C
  // Ensuite tout le monde envoie sa partie calculée au root (gather)

  // Chaque machine alloue un espace pour mettre le résultat du calcul de la partie de A reçu * la matrice B
  int *local_res = (int *)malloc(nbr_linesA_received * ci.columns_b * sizeof(int));

  // Chacune calcule x lignes de la matrice C
  get_res_lines_C(local_res, received_part_A, flatB, nbr_linesA_received);

  // Tout le monde envoie sa partie calculée au root (gather)
  //Le root rençoi le résultat de toutes les machines
  MPI_Gather(local_res, nbr_linesA_received * ci.columns_b, MPI_INT, res_final, nbr_linesA_received * ci.columns_b, MPI_INT, root, MPI_COMM_WORLD);

  // Toutes les machines vont attendre que le root reçoi les résulats de toutes les machines pour que
  // la machine root désalloue ce qui a ete alloue
  MPI_Barrier(MPI_COMM_WORLD);

  if (rank == root)
  {
    end = MPI_Wtime();
    time_used_mpi = end - start;
    printf("\n");
    printf("    Time used MPI using scatter and gather: %f:\n", time_used_mpi);
    printf("    Time used in sequential: %f:\n", time_used_seq);

    if (!result_correct(ci.lines_a, ci.columns_b, res_final, mat_C))
    {
      printf("error mat not equal \n");
      return -1;
    }
    // Desallocation
    free_mat(mat_A, ci.lines_a);
    free_mat(mat_B, ci.lines_b);
    free(flatA);
  }

  free(flatB);
  MPI_Finalize();
  return EXIT_SUCCESS;
}