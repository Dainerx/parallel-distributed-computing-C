#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <omp.h>
#include <time.h>
#include <omp.h>
#define LINES_A 40
#define COLUMNS_A 30
#define LINES_B 30
#define COLUMNS_B 30
#include "matrix_util.h"

// Remplissage d'une matrice avec des valeurs aléatoires
void fill_matt(int **mat, int lines, int colums)
{
  int j;
#pragma omp parallel for shared(mat) private(j)
  for (int i = 0; i < lines; i++)
  {
    //printf("thread: %d.threads dans omp %d\n", omp_get_thread_num(), omp_get_num_threads());
    //exit;
    for (j = 0; j < colums; j++)
    {
      mat[i][j] = (rand() % MAX_VAL) + MIN_VAL;
    }
  }
}

void convertMat(int **matrixA, int **matrixB, int *a, int *b)
{
  int j;
#pragma omp paralel
  {
#pragma omp parallel for shared(a) private(j)
    for (int i = 0; i < LINES_A; i++)
    {
      for (j = 0; j < COLUMNS_A; j++)
      {
        a[i * COLUMNS_A + j] = matrixA[i][j];
      }
    }
#pragma omp parallel for shared(b) private(j)
    for (int i = 0; i < LINES_B; i++)
    {
      //printf("%d\n", omp_get_num_threads());
      for (j = 0; j < COLUMNS_B; j++)
      {
        b[j * LINES_B + i] = matrixB[i][j];
      }
    }
  }
}

void get_res_line(int *res, int *receive_lineA, int *flatB)
{
  int c = 0;
  int t;
//while(k < LINES_B*COLUMNS_B)
#pragma parallel for shared(res) private(i)
  for (int k = 0; k < LINES_B * COLUMNS_B; k += LINES_B)
  {
    t = 0;
    for (int i = 0; i < COLUMNS_A; i++)
    {
      t += receive_lineA[i] * flatB[i + k];
    }
    res[c] = t;
    //k += LINES_B;
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

  MPI_Status status;

  srand(time(NULL) + rank);
  int **mat_A;
  int **mat_B;
  int *flatA;
  int *res_final;
  int numtasks;

  double start, end, time_used_mpi;
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
  int *flatB = malloc((LINES_B * COLUMNS_B) * sizeof(int));
  if (rank == root)
  {
    printf("%d\n", numtasks);

    mat_A = malloc_mat(LINES_A, COLUMNS_A);
    mat_B = malloc_mat(LINES_B, COLUMNS_B);
    fill_matt(mat_A, LINES_A, COLUMNS_A);
    fill_matt(mat_B, LINES_B, COLUMNS_B);

    start = MPI_Wtime();
    flatA = malloc((LINES_A * COLUMNS_A) * sizeof(int));
    convertMat(mat_A, mat_B, flatA, flatB);

    // Le root va allouer la taille mémoire pour contenir le résultat de la multiplication
    res_final = malloc((LINES_A * COLUMNS_B) * sizeof(int));
  }
  // Envoie de la matrice B a tout le monde; B sera partagée
  MPI_Bcast(flatB, LINES_B * COLUMNS_B, MPI_INT, root, MPI_COMM_WORLD);

  // Chacun alloue un tableau de taille COLUMNS_A pour recevoir sa partie de la matrice A
  // chacun reçoi une ligne de la matrice A
  int *receive_lineA = (int *)malloc(COLUMNS_A * sizeof(int));
  if (receive_lineA)
  {
    MPI_Scatter(flatA, COLUMNS_A, MPI_INT, receive_lineA, COLUMNS_A, MPI_INT, root, MPI_COMM_WORLD);
  }

  // Chacun dispose désormais de la matrice B en 1D (B est partagée) et une ligne de la matrice A
  // Chacun doit avoir comme résultat une ligne de la matrice C
  // Ensuite tout le monde envoie sa partie calculée au root (gather)

  int *local_res = (int *)malloc(COLUMNS_B * sizeof(int));

  // Chacun calcule la ligne de la matrice C
  get_res_line(local_res, receive_lineA, flatB);

  MPI_Gather(local_res, COLUMNS_B, MPI_INT, res_final, COLUMNS_B, MPI_INT, root, MPI_COMM_WORLD);

  if (rank == root)
  {
    end = MPI_Wtime();
    time_used_mpi = end - start;
    printf("temps %f:\n", time_used_mpi);
    //display_linear_mat(res_final,LINES_A,COLUMNS_B);
  }

  free(flatB);
  if (rank == root)
  {
    free_mat(mat_A, LINES_A);
    free_mat(mat_B, LINES_B);
    free(flatA);
  }
  MPI_Finalize();
  return EXIT_SUCCESS;
}
