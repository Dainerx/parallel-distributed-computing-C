#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <omp.h>
#include <time.h>
#define LINES_A 100
#define COLUMNS_A 400
#define LINES_B 400
#define COLUMNS_B 100

#include "matrix_util.h"

/*
  Cette fonction permet de rendre une matrice 1D  en deux dimensions
  La matrice 1D en entrée est représentée par lignes
*/
void unflat_Lin(int *A, int lines, int columns, int **A_unflatted)
{

  for(int i=0; i<lines; i++)
  {
    for(int j=0; j< columns; j++)
    {
        A_unflatted[i][j] = A[i * columns + j];
    }
  }
}


/*
  Cette fonction permet de faire une multiplication séquentielle de deux matrices A et B
*/
double sequential_mult(int **mat_A, int **mat_B, int **mat_C)
{
    double start, end, cpu_time_used;
    start = omp_get_wtime();
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
    end = omp_get_wtime();
    cpu_time_used = (end - start);
    return cpu_time_used;
}

/*
  Cette fonction permet de tester l'égalité de deux matrices en entrée
*/
bool mat_equal(int **A, int** B)
{
  bool correct = true;
  for (int i = 0; i < LINES_A; i++)
  {
      for (int j = 0; j < COLUMNS_B; j++)
      {
        if (A[i][j] != B[i][j])
        {
            printf("Found no equal cell: (%d,%d) a(%d,%d)=%d ; b(%d,%d)=%d\n", i, j, i, j, A[i][j], i, j, B[i][j]);
            correct = false;
        }
      }
  }
  return correct;
}

/*
  Cette fonction permet de convertir des matrices de deux dimensions en une dimension
  Elle prend en paramètre deux matrices A et B, La matrice A va être transformée en une matrice 1D présentée ligne après ligne
  Et la matrice B va être transformée en une matrice 1D présentée colonne après colonne

*/
void convertMat(int **matrixA, int **matrixB, int *a, int *b)
{
  int i;

    for ( i = 0; i < LINES_A; i++)
    {
        for (int j = 0; j < COLUMNS_A; j++)
        {
            a[i * COLUMNS_A + j] = matrixA[i][j];
        }
    }

    for (int i = 0; i < LINES_B; i++)
    {
        for (int j = 0; j < COLUMNS_B; j++)
        {
            b[j * LINES_B + i] = matrixB[i][j];
        }
    }
}
/*
  Cette fonction permet de faire une multiplication des deux matrices A et B dans une matrice C
*/
 void mult(int **A, int **B, int **C, int linesA, int columnsA,int linesB, int columnsB)
 {
   int sum;
   for (int i = 0; i < linesA; i++)
   {
       for (int j = 0; j < columnsB; j++)
       {
           sum = 0;
           for (int k = 0; k < columnsA; k++)
           {
               sum += (A[i][k] * B[k][j]);
           }
           C[i][j] = sum;
       }
   }
 }

 /*
   Cette fontion permet de faire la multiplication d'une partie de la matrice A reçue par chaque machine avec la matrice B connue par toutes les machines
 */
void get_res_lines_C(int *local_res,int *received_part_A,int *flatB, int nbr_linesA_received)
{
  int k = 0;
  int c = 0;
  int t;
  int sizeLineA = 0;

  for(int linesA = 0; linesA < nbr_linesA_received; linesA++)
  {
    while (k < LINES_B * COLUMNS_B)
    {
      t = 0;
      for (int i = 0; i < COLUMNS_A; i++)
      {
        t += received_part_A[i+sizeLineA] * flatB[i + k];
      }

      local_res[c] = t;
      k += LINES_B;
      c += 1;
    }
    k = 0;
    sizeLineA += COLUMNS_A;
  }
}

int main(int argc, char **argv)
{
    const int root = 0;
    int rank, world_size;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &world_size);

    srand(time(NULL)+rank);
    int **mat_A;
    int **mat_B;
    int *flatA;
    int* res_final;
    int **mat_C;

    double start, end, time_used_mpi;
    if((LINES_A * COLUMNS_A) % world_size != 0)
    {
      printf("Le nombre de machines doit etre divisible par LINES_A * COLUMNS_A\n");
      return -1;
    }
    // S'assurer que chaque machine reçoie un certain nombre de lignes de la matrice A et non juste une partie d'une ligne
    if((LINES_A * COLUMNS_A / world_size) % COLUMNS_A != 0)
    {
      printf(" lingesA * colunnesA /nombre de machines doit etre divisible sur colonnesA\n");
      return -2;
    }
    int *flatB = malloc((LINES_B * COLUMNS_B) * sizeof(int));

    // Le machine root va allouer et initialiser les deux matrices A et B. Ensuite, elle va distribuer la matrice B à toutes les autres machines, et envoie
    // pour chaque machine sa partie de la matrice A
    if(rank == root)
    {
      mat_A = malloc_mat(LINES_A, COLUMNS_A);
      mat_B = malloc_mat(LINES_B, COLUMNS_B);
      mat_C = malloc_mat(LINES_A, COLUMNS_B);

      fill_mat(mat_A, LINES_A, COLUMNS_A);
      fill_mat(mat_B, LINES_B, COLUMNS_B);

      int time_seq = sequential_mult(mat_A, mat_B, mat_C);
      start= MPI_Wtime();

      flatA =  malloc((LINES_A * COLUMNS_A) * sizeof(int));
      // Conversion des matrices en une dimension
      convertMat(mat_A, mat_B,flatA,flatB);

      // Le root va allouer la taille mémoire pour contenir le résultat de la multiplication
      res_final = malloc((LINES_A*COLUMNS_B)*sizeof(int));
    }

    // Toutes les machines vont attendre que la machine root fini de remplir les matrices A et B
    MPI_Barrier(MPI_COMM_WORLD);

    // Envoie de la matrice B a tout le monde par la machine root; B sera partagée
    MPI_Bcast(flatB,LINES_B*COLUMNS_B,MPI_INT,root,MPI_COMM_WORLD);

    int length_revceived_A = COLUMNS_A * LINES_A / world_size;
    // Toutes les machines vont allouer un espace pour recevoir leur partie de la matrice A
    int* received_part_A = (int *)malloc(length_revceived_A *sizeof(int));

    // Le root envoie à toutes les machines sa part de la matrice A y compris lui
    if(received_part_A)
    {
      MPI_Scatter(flatA,length_revceived_A, MPI_INT, received_part_A, length_revceived_A, MPI_INT, root, MPI_COMM_WORLD);
    }
    int nbr_linesA_received = length_revceived_A / COLUMNS_A;

    // Chaque machine dispose désormais de la matrice B en 1D (B est partagée) et un nombre x de lignes de la matrice A
    // Chacune doit avoir comme résultat x lignes de la matrice C
    // Ensuite tout le monde envoie sa partie calculée au root (gather)

    // Chaque machine alloue un espace pour mettre le résultat du calcul de la partie de A reçu * la matrice B
    int* local_res = (int*) malloc(nbr_linesA_received*COLUMNS_B*sizeof(int));

    // Chacune calcule x lignes de la matrice C
    get_res_lines_C(local_res,received_part_A,flatB, nbr_linesA_received);

    // Tout le monde envoie sa partie calculée au root (gather)
    //Le root rençoi le résultat de toutes les machines
    MPI_Gather(local_res,nbr_linesA_received*COLUMNS_B , MPI_INT, res_final, nbr_linesA_received*COLUMNS_B, MPI_INT, root, MPI_COMM_WORLD );

    // Toutes les machines vont attendre que le root reçoi les résulats de toutes les machines pour que
    // la machine root désalloue ce qui a ete alloue
    MPI_Barrier(MPI_COMM_WORLD);

    if(rank == root)
    {
      end= MPI_Wtime();
      time_used_mpi = end - start;
      printf("Time used MPI scatter and gather: %f:\n", time_used_mpi);
      int ** CC = malloc_mat( LINES_A, COLUMNS_B );
      unflat_Lin(res_final, LINES_A, COLUMNS_B, CC );

      if(!mat_equal(mat_C, CC))
      {
        printf("error mat not equal \n");
        return -1;
      }
      // DesalLocation
      free_mat(mat_A, LINES_A );
      free_mat(mat_B, LINES_B);
      free(flatA);
    }

    free(flatB);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
