#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <time.h>
#define LINES_A 40
#define COLUMNS_A 30
#define LINES_B 30
#define COLUMNS_B 30

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

void get_res_line(int *res,int *receive_lineA,int *flatB)
{
  int k = 0;
  int c = 0;
  int t;
  while(k < LINES_B*COLUMNS_B)
  {
    t = 0;
    for(int i = 0; i<COLUMNS_A; i++)
    {
      t += receive_lineA[i] * flatB[i+k];
    }
    res[c] = t;
    k += LINES_B;
    c +=1;
  }
}
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
/*void sequential_mult(int **mat_A, int **mat_B, int **mat_C)
{

    double start, end, cpu_time_used;
    //start = omp_get_wtime();
    int sum;
    for (int i = 0; i < LINES_A; i++)
    {
        for (int j = 0; j < COLUMNS_B; j++)
        {
            sum = 0;
            for (int k = 0; k < COLUMNS_A; k++)
            {
                sum += (mat_A[i][k] * mat_B[k][j]);
            }
            mat_C[i][j] = sum;
        }
    }
    //end = omp_get_wtime();
    //cpu_time_used = (end - start);
    //return cpu_time_used;
}
void display_matt(int **mat, int lines, int colums)
{
    for (int i = 0; i < lines; i++)
    {
        for (int j = 0; j < colums; j++)
        {
            printf("%d \t", mat[i][j]);
        }
        printf("\n");
    }
}
*/
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
    int* res_final;

    double start, end, time_used_mpi;

    int *flatB = malloc((LINES_B * COLUMNS_B) * sizeof(int));
    if(rank == root)
    {
      start= MPI_Wtime();
      mat_A = malloc_mat(LINES_A, COLUMNS_A);
      mat_B = malloc_mat(LINES_B, COLUMNS_B);
      fill_mat(mat_A, LINES_A, COLUMNS_A);
      fill_mat(mat_B, LINES_B, COLUMNS_B);
      /*printf ("mat A\n");
      display_mat(mat_A, LINES_A, COLUMNS_A);
      printf("\n\n");
      printf ("mat B\n");
      display_mat(mat_B, LINES_B, COLUMNS_B);
      printf("\n\n");*/
      flatA =  malloc((LINES_A * COLUMNS_A) * sizeof(int));
      convertMat(mat_A, mat_B,flatA,flatB);

      /*printf ("mat B flated\n");
      for(int i=0; i<LINES_B*COLUMNS_B; i++)
      {
        printf("%d\t",flatB[i] );
      }
      printf("\n\n");
      printf ("mat A flated\n");
      for(int i=0; i<LINES_A*COLUMNS_A; i++)
      {
        printf("%d\t",flatA [i] );
      }
      printf("\n\n");*/

      // Le root va allouer la taille mémoire pour contenir le résultat de la multiplication
      res_final = malloc((LINES_A*COLUMNS_B)*sizeof(int));

      /*int **mat_C = malloc_mat(LINES_A, COLUMNS_B);
      sequential_mult(mat_A, mat_B, mat_C);
      display_matt(mat_C,LINES_A,COLUMNS_B);*/
    }
    // Envoie de la matrice B a tout le monde; B sera partagée
    MPI_Bcast(flatB,LINES_B*COLUMNS_B,MPI_INT,root,MPI_COMM_WORLD);

    // Chacun alloue un tableau de taille COLUMNS_A pour recevoir sa partie de la matrice A
    // chacun reçoi une ligne de la matrice A
    int* receive_lineA = (int *)malloc(COLUMNS_A*sizeof(int));
    if(receive_lineA)
    {
      MPI_Scatter(flatA, COLUMNS_A, MPI_INT, receive_lineA, COLUMNS_A,MPI_INT, root, MPI_COMM_WORLD);
      //printf("My rank is %d, I recieved %d %d %d\n", rank, receive_lineA[0], receive_lineA[1], receive_lineA[2]);
    }

    // Chacun dispose désormais de la matrice B en 1D (B est partagée) et une ligne de la matrice A
    // Chacun doit avoir comme résultat une ligne de la matrice C
    // Ensuite tout le monde envoie sa partie calculée au root (gather)

    int* local_res = (int*) malloc(COLUMNS_B*sizeof(int));

    // Chacun calcule la ligne de la matrice C
    get_res_line(local_res,receive_lineA,flatB);
    MPI_Gather(local_res,COLUMNS_B , MPI_INT, res_final, COLUMNS_B, MPI_INT, root, MPI_COMM_WORLD );

    if(rank == root)
    {
      end= MPI_Wtime();
      time_used_mpi = end - start;
      printf("temps %f:", time_used_mpi);
      //display_linear_mat(res_final,LINES_A,COLUMNS_B);
    }

    free(flatB);
    if(rank == root)
       {
         free_mat(mat_A, LINES_A );
         free_mat(mat_B, LINES_B);
      }
    MPI_Finalize();
    return EXIT_SUCCESS;
}
