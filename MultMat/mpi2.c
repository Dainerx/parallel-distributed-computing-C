#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <time.h>
#define LINES_A 2
#define COLUMNS_A 4
#define LINES_B 4
#define COLUMNS_B 3

#include "matrix_util.h"

void convertMat(int **matrixA, int **matrixB, int *a, int *b)
{
#pragma omp parallel for
    for (int i = 0; i < LINES_A; i++)
    {
        for (int j = 0; j < COLUMNS_A; j++)
        {
            a[i * COLUMNS_A + j] = matrixA[i][j];
        }
    }
#pragma omp parallel for
    for (int i = 0; i < LINES_B; i++)
    {
        for (int j = 0; j < COLUMNS_B; j++)
        {
            b[j * LINES_B + i] = matrixB[i][j];
        }
    }
}

int get_res_cell(int *receive_lineA, int *receive_columnB)
{
  int res = 0;
  for(int i = 0; i < COLUMNS_A; i++)
  {
    res += receive_lineA[i] * receive_columnB[i];
  }
  return res;
}

// a refaire pour prendre en compte flatB
void display_linear_mat(int *mat, int lines, int columns)
{
  for(int i=0; i<lines*columns; i++)
  {
    if(i%columns ==0)
    {
      printf("\n");
    }
    printf("%d\t", mat[i]);
  }
}
// Remarque :  le nombre de threads au lancement doit etre egal au nombre de lignes de la matrice A
int main(int argc, char **argv)
{
    const int root = 0;
    int rank, world_size,tag=0;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &world_size);

    MPI_Status status;

    srand(time(NULL)+rank);
    int **mat_A;
    int **mat_B;
    int *flatA;
    int *flatB;
    int* res_final;

    double start, end, time_used_mpi;

    if(rank == root)
    {
      start= MPI_Wtime();
      mat_A = malloc_mat(LINES_A, COLUMNS_A);
      mat_B = malloc_mat(LINES_B, COLUMNS_B);

      fill_mat(mat_A, LINES_A, COLUMNS_A);
      fill_mat(mat_B, LINES_B, COLUMNS_B);

      printf("mat A \n");
      display_mat(mat_A, LINES_A, COLUMNS_A);
      printf("\n\n");


      printf("mat B \n");
      display_mat(mat_B, LINES_B, COLUMNS_B);
      printf("\n\n");
      flatA =  malloc((LINES_A * COLUMNS_A) * sizeof(int));
      flatB = malloc((LINES_B * COLUMNS_B) * sizeof(int));
      convertMat(mat_A, mat_B,flatA,flatB);

      display_linear_mat(flatA, LINES_A, COLUMNS_A);
      printf("\n\n");
      display_linear_mat(flatB, LINES_B, COLUMNS_B);
      printf("\n\n");

      for(int i =0; i< LINES_B*COLUMNS_B; i++)
      {
        printf("%d\t", flatB[i]);
      }
        printf("\n\n");
      // Le root va allouer la taille mémoire pour contenir le résultat de la multiplication
      res_final = malloc((LINES_A*COLUMNS_B)*sizeof(int));
    }

    // Chacun alloue un tableau de taille COLUMNS_A pour recevoir sa partie de la matrice A
    // chacun reçoi une ligne de la matrice A
    int* receive_lineA = (int *)malloc(COLUMNS_A*sizeof(int));
    if(receive_lineA)
    {
      MPI_Scatter(flatA, COLUMNS_A, MPI_INT, receive_lineA, COLUMNS_A,MPI_INT, root, MPI_COMM_WORLD);
    }
    printf("===== A %d, %d %d %d\n",rank,receive_lineA[0], receive_lineA[1], receive_lineA[2] );
    int* receive_columnB = (int *)malloc(LINES_B*sizeof(int));
    if(receive_columnB)
    {
      MPI_Scatter(flatB, LINES_B, MPI_INT, receive_columnB, LINES_B,MPI_INT, root, MPI_COMM_WORLD);
    }
    printf("===== B %d, %d %d\n",rank,receive_columnB[0], receive_columnB[1]);
    // Chacun dispose désormais d'une colonne de la matrice B et une ligne de la matrice A
    // Chacun doit avoir comme résultat une case de la matrice C
    // Ensuite tout le monde envoie sa partie calculée au root (gather)

    int local_res;
    // Chacun calcule lune case de la matrice C
    local_res = get_res_cell(receive_lineA,receive_columnB);
    //printf("rank = %d %d\n",rank, local_res);
    MPI_Gather(&local_res, 1, MPI_INT, res_final, 1, MPI_INT, root, MPI_COMM_WORLD );

    if(rank == root)
    {
      end= MPI_Wtime();
      time_used_mpi = end - start;
      printf("temps %f:\n", time_used_mpi);
      display_linear_mat(res_final,LINES_A,COLUMNS_B);
      printf("\n");
      printf("=fff==== res %d %d %d %d\n",res_final[0], res_final[1],res_final[3]);
    }
    if(rank == root)
       {
         free_mat(mat_A, LINES_A );
         free_mat(mat_B, LINES_B);
         free(flatA);
         free(flatB);
      }
    MPI_Finalize();
    return EXIT_SUCCESS;
}
