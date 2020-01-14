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
    k += 3;
    c +=1;
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
    int* res_final;

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
      flatA =  malloc((LINES_A * COLUMNS_A) * sizeof(int));
      convertMat(mat_A, mat_B,flatA,flatB);

      printf ("mat B flated\n");
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
      printf("\n\n");

      res_final = malloc((LINES_A*COLUMNS_B)*sizeof(int));
    }
    // Envoie de la matrice B a tout le monde; B sera partagée
    MPI_Bcast(flatB,LINES_B*COLUMNS_B,MPI_INT,root,MPI_COMM_WORLD);
    if(rank != root)
    {
    //  if(flatB)
        //  printf("je suis %d et j'ai reçu %d %d %d\n", rank,flatB[0], flatB[1], flatB[2] );

    }

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

  /*  for(int i = 0; i<COLUMNS_B; i++)
    {
      printf("%d\t",local_res[i]);
    }
    printf("\n");
    */
    MPI_Gather(local_res,COLUMNS_B , MPI_INT, res_final, COLUMNS_B, MPI_INT, root, MPI_COMM_WORLD );

    if(rank == root)
    {
      for(int i =0; i<LINES_A*COLUMNS_B; i++)
        {
          printf("%d\t", res_final[i]);
        }
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
