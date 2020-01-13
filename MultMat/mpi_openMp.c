#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <time.h>
#define LINES_A 4
#define COLUMNS_A 3
#define LINES_B 3
#define COLUMNS_B 3
#define THREADS_NUMBER 4
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
// Launch with four threads
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

    int *flatB = malloc((LINES_B * COLUMNS_B) * sizeof(int));
    if(rank == root)
    {
      mat_A = malloc_mat(LINES_A, COLUMNS_A);
      mat_B = malloc_mat(LINES_B, COLUMNS_B);
      fill_mat(mat_A, LINES_A, COLUMNS_A);
      fill_mat(mat_B, LINES_B, COLUMNS_B);
      printf ("mat A\n");
      display_mat(mat_A, LINES_A, COLUMNS_A);
      printf("\n\n");
      printf ("mat B\n");
      display_mat(mat_B, LINES_B, COLUMNS_B);
      printf("\n\n");
      int *flatA =  malloc((LINES_A * COLUMNS_A) * sizeof(int));
      convertMat(mat_A, mat_B,flatA,flatB);

      printf ("mat B flated\n");
      for(int i=0; i<LINES_B*COLUMNS_B; i++)
      {
        printf("%d\t",flatB[i] );
      }
      printf("\n\n");
    }
    // Envoie de la matrice B a tout le monde; B sera partagée
    MPI_Bcast(flatB,LINES_B*COLUMNS_B,MPI_INT,root,MPI_COMM_WORLD);
    if(rank != root)
    {
      if(flatB)
          printf("je suis %d et j'ai reçu %d %d %d\n", rank,flatB[0], flatB[1], flatB[2] );

    }
    free(flatB);
    if(rank == root)
       {
         free_mat(mat_A, LINES_A );
        free_mat(mat_B, LINES_B);

      }
    /*int* receive = (int *)malloc(COLUMNS_A*sizeof(int));
    MPI_Scatter(tab, CHUNK, MPI_INT, receive, CHUNK,MPI_INT, root, MPI_COMM_WORLD);*/

    //MPI_Send(&tab_to_scatter[i],1, MPI_INT,i,tag,MPI_COMM_WORLD);

    //MPI_Recv(&token, 1, MPI_INT, root, tag, MPI_COMM_WORLD, &status);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
