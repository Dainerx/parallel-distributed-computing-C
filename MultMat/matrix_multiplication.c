#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#define LINES_A 500
#define COLUMS_A 2000
#define LINES_B 2000
#define COLUMS_B 700
/*#define LINES_A 5
#define COLUMS_A 2
#define LINES_B 2
#define COLUMS_B 3*/
#define MAX_VAL 10
#define MIN_VAL 2

int flatA[LINES_A * COLUMS_A];
int flatB[LINES_B * COLUMS_B];

int ** malloc_mat(int lines, int colums)
{
  int** mat = (int**)malloc(lines * sizeof(int*));
  for(int i = 0; i < lines; i++)
    mat[i] = (int*)malloc(colums * sizeof(int));
  return mat;
}

void fill_mat(int** mat, int lines, int colums)
{
  for(int i = 0; i < lines; i++)
  {
    for(int j = 0; j < colums; j++)
    {
      mat[i][j] = rand() % MAX_VAL + MIN_VAL;
    }
  }
}

void display_mat(int** mat, int lines, int colums)
{
  for(int i = 0; i < lines; i++)
  {
    for(int j = 0; j < colums; j++)
    {
      printf("%d \t", mat[i][j]);
    }
    printf("\n");
  }
}

double sequential_mult(int** mat_A, int** mat_B, int** mat_C )
{
  double start, end, cpu_time_used;
  int sum;
  start = omp_get_wtime();
  for(int i = 0; i < LINES_A; i++)
  {
    for(int j = 0; j < COLUMS_B; j++)
    {
      sum = 0;
      for(int k = 0; k < COLUMS_A; k++)
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

double parallel_mult(int** mat_A, int** mat_B, int** mat_C )
{
  double start, end, cpu_time_used;
  int i, j, k, t, sum;
  omp_set_dynamic(0);
  omp_set_num_threads(omp_get_num_procs());
    printf("le nombre de threads %d\n", omp_get_num_threads());
  /* JE NE COMPRENDS PAS POURQUOI omp_set_num_threads NE CHANGE PAS LE NOMBRE DE THREADS
  ÇA DONNE TOUJOURS 1 */
  int chunk = COLUMS_A/(omp_get_num_threads()*2);
  //int chunk = 250;
  /*A VEFIRIER ou il faut declarer chunk
  car si ici omp_get_num_threads() donne toujours 1 CAR SEQUENTIEL donc le chunk = nombre de colonnes de A
  par contre si c est a l interieur de la boucle  ca change
  */
  start = omp_get_wtime();
  //omp_set_num_threads(LINES_A);
  //omp_set_num_threads(omp_get_num_procs());
  /* IMPORTANT ICI VU QUE LA VALEUR DE CHUNK N EST PAS BONNE, ÇA DONNE LE MEME
  RESULTAT SI ON NE MET PAS schedule(dynamic, chunk) */
  #pragma omp parallel for private(i,j,k) shared(mat_A,mat_B,mat_C)
  for(i = 0; i < LINES_A; i++)
  {
    //printf("I am thread %d \n", omp_get_thread_num());
    //#pragma omp parallel for
    for(j = 0; j < COLUMS_B; j++)
    {
      //printf("I am thread %d \n", omp_get_thread_num());
      sum = 0;
      #pragma omp parallel for schedule(dynamic, chunk) private(t) reduction(+ : sum)
      for(k = 0; k < COLUMS_A; k++)
      {
        //printf("I am thread %d \n", omp_get_thread_num());
        t =  (mat_A[i][k] * mat_B[k][j]);
        sum += t;
      }
      mat_C[i][j] = sum;
    }
  }
  end = omp_get_wtime();
  cpu_time_used = (end - start);
  return cpu_time_used;
}

void convert(int** matrixA, int** matrixB){
	#pragma omp parallel for
	for(int i=0; i<LINES_A; i++){
		for(int j=0; j<COLUMS_A; j++){
			flatA[i * COLUMS_A + j] = matrixA[i][j];
		}
	}
  #pragma omp parallel for
  for(int i=0; i<LINES_B; i++){
    for(int j=0; j<COLUMS_B; j++){
      flatB[j * LINES_B + i] = matrixB[i][j];
    }
  }
  // CORRECT
  /*for(int i=0; i<LINES_A*COLUMS_A; i++)
  {
    printf("%d\t", flatA[i]);
  }
  printf("\n");
  for(int i=0; i<LINES_B*COLUMS_B; i++)
  {
    printf("%d\t", flatB[i]);
  }
  printf("\n");*/

}

double optimizedParallelMultiply(int** matrixA, int** matrixB, int** matrixC){
	/*
		Parallel multiply given input matrices using optimal methods and return resultant matrix
	*/

	int i, j, k, iOff, jOff;
	int tot;
  double start, end, cpu_time_used;
	/* Head */
  start = omp_get_wtime();
	convert(matrixA, matrixB);
	#pragma omp parallel shared(matrixC) private(i, j, k, iOff, jOff, tot) num_threads(40)
	{
		#pragma omp for schedule(static)
		for(i=0; i<LINES_A; i++){
			iOff = i * COLUMS_A;
			for(j=0; j<COLUMS_B; j++){
				jOff = j * LINES_B;
				tot = 0;
				for(k=0; k<COLUMS_A; k++){
					tot += flatA[iOff + k] * flatB[jOff + k];
				}
				matrixC[i][j] = tot;
			}
		}
	}
	/* Tail */
  end = omp_get_wtime();
  cpu_time_used = (end - start);
  return cpu_time_used;
}


int main(int argc, char *argv[])
{
  if(COLUMS_A != LINES_B )
  {
    printf(" Vous allez multiplier deux matrice A et B, le nombre de lignes de la matrice B doit donc etre le nombre de colonnes de la matrice A \n ");
    return -1;
  }

  int** mat_A = malloc_mat(LINES_A, COLUMS_A);
  int** mat_B = malloc_mat(LINES_B, COLUMS_B);
  int** mat_C = malloc_mat(LINES_A, COLUMS_B);

  srand(time(NULL)); // PEUT ETRE AJOUTER QLQ CHOSE ICI
  fill_mat(mat_A, LINES_A, COLUMS_A);
  fill_mat(mat_B, LINES_B, COLUMS_B);

  double cpu_time_used;
  cpu_time_used = sequential_mult(mat_A, mat_B, mat_C);
  printf("Cpu time used in seq: %f\n",cpu_time_used);


  cpu_time_used = parallel_mult(mat_A, mat_B, mat_C);
  printf("Cpu time used in parallel: %f\n",cpu_time_used);
  /*printf("\nMat C\n");
  display_mat(mat_C, LINES_A, COLUMS_B);*/
/*  printf("\nMat C\n");
  display_mat(mat_C, LINES_A, COLUMS_B);*/

  cpu_time_used = optimizedParallelMultiply(mat_A, mat_B, mat_C);
  printf("Cpu time used in optimized parallel: %f\n",cpu_time_used);
  /*printf("\nMat C\n");
  display_mat(mat_C, LINES_A, COLUMS_B);*/
  /*printf("\nMat A\n");
  display_mat(mat_A, LINES_A, COLUMS_A);
  printf("\nMat B\n");
  display_mat(mat_B, LINES_B, COLUMS_B);
  printf("\nMat C\n");
  display_mat(mat_C, LINES_A, COLUMS_B);*/

  /* DON T FORGET TO FREE */
  return EXIT_SUCCESS;
}
