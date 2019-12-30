#include <time.h>
#include <omp.h>
#include "solver.h"

struct FlatArraysCouple
{
    int *a; // pointer to the matrix
    int *b;
};

double sequential_mult(int **mat_A, int **mat_B, int **mat_C)
{
    double start, end, cpu_time_used;
    int sum;
    start = omp_get_wtime();
    for (int i = 0; i < LINES_A; i++)
    {
        for (int j = 0; j < COLUMS_B; j++)
        {
            sum = 0;
            for (int k = 0; k < COLUMS_A; k++)
            {
                sum += (mat_A[i][k] * mat_B[k][j]);
            }
            //printf("%d,%d\n", i, j);
            mat_C[i][j] = sum;
        }
    }
    end = omp_get_wtime();
    cpu_time_used = (end - start);
    return cpu_time_used;
}

double parallel_mult(int num_threads, int **mat_A, int **mat_B, int **mat_C)
{
    double start, end, cpu_time_used;
    int i, j, k, t, sum;
    omp_set_dynamic(0);
    omp_set_num_threads(num_threads);
    int chunk = COLUMS_A / (omp_get_num_threads() * 2);
    start = omp_get_wtime();
/* IMPORTANT ICI VU QUE LA VALEUR DE CHUNK N EST PAS BONNE, ÇA DONNE LE MEME
  RESULTAT SI ON NE MET PAS schedule(dynamic, chunk) */
#pragma omp parallel for private(i, j, k) shared(mat_A, mat_B, mat_C)
    for (i = 0; i < LINES_A; i++)
    {
        //printf("le nombre de threads %d\n", omp_get_num_threads());
        //#pragma omp parallel for
        for (j = 0; j < COLUMS_B; j++)
        {
            //printf("I am thread %d \n", omp_get_thread_num());
            sum = 0;
#pragma omp parallel for schedule(dynamic, chunk) private(t) reduction(+ \
                                                                       : sum)
            for (k = 0; k < COLUMS_A; k++)
            {
                //printf("I am thread %d \n", omp_get_thread_num());
                t = (mat_A[i][k] * mat_B[k][j]);
                sum += t;
            }
            mat_C[i][j] = sum;
        }
    }
    end = omp_get_wtime();
    cpu_time_used = (end - start);
    return cpu_time_used;
}

struct FlatArraysCouple convert(int num_threads, int **matrixA, int **matrixB)
{
    int *a = malloc((LINES_A * COLUMS_A) * sizeof(int));
    int *b = malloc((LINES_B * COLUMS_B) * sizeof(int));
    struct FlatArraysCouple flat_array_couple = {a, b};
    omp_set_num_threads(num_threads);
#pragma omp parallel for
    for (int i = 0; i < LINES_A; i++)
    {
        for (int j = 0; j < COLUMS_A; j++)
        {
            a[i * COLUMS_A + j] = matrixA[i][j];
        }
    }
#pragma omp parallel for
    for (int i = 0; i < LINES_B; i++)
    {
        for (int j = 0; j < COLUMS_B; j++)
        {
            b[j * LINES_B + i] = matrixB[i][j];
        }
    }
    return flat_array_couple;
}

double optimized_parallel_multiply(int num_threads, int **matrixA, int **matrixB, int **matrixC)
{
    /*
		Parallel multiply given input matrices using optimal methods and return resultant matrix
	*/
    int i, j, k, iOff, jOff;
    int tot;
    double start, end, cpu_time_used;
    /* Head */
    start = omp_get_wtime();
    struct FlatArraysCouple flat_array_couple = convert(num_threads, matrixA, matrixB);
    int *flatA = flat_array_couple.a;
    int *flatB = flat_array_couple.b;
    omp_set_num_threads(num_threads);
#pragma omp parallel shared(matrixC) private(i, j, k, iOff, jOff, tot) num_threads(40)
    {
#pragma omp for schedule(static)
        for (i = 0; i < LINES_A; i++)
        {
            iOff = i * COLUMS_A;
            for (j = 0; j < COLUMS_B; j++)
            {
                jOff = j * LINES_B;
                tot = 0;
                for (k = 0; k < COLUMS_A; k++)
                {
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
