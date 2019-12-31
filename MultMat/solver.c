#include <time.h>
#include <omp.h>
#include "solver.h"

struct Input input;

void init_solver(int la, int ca, int lb, int cb)
{
    input.la = la;
    input.ca = ca;
    input.lb = lb;
    input.cb = cb;
}

double sequential_mult(int **mat_A, int **mat_B, int **mat_C)
{
    int lines_a = input.la, lines_b = input.lb;
    int columns_a = input.ca, columns_b = input.cb;
    double start, end, cpu_time_used;
    int sum;
    start = omp_get_wtime();
    for (int i = 0; i < lines_a; i++)
    {
        for (int j = 0; j < columns_b; j++)
        {
            sum = 0;
            for (int k = 0; k < columns_a; k++)
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

double parallel_mult(int num_threads, int **mat_A, int **mat_B, int **mat_C)
{
    int lines_a = input.la, lines_b = input.lb;
    int columns_a = input.ca, columns_b = input.cb;
    double start, end, cpu_time_used;
    int i, j, k, t, sum;
    omp_set_dynamic(0);
    omp_set_num_threads(num_threads);
    int chunk = columns_a / (omp_get_num_threads() * 2);
    start = omp_get_wtime();
/* IMPORTANT ICI VU QUE LA VALEUR DE CHUNK N EST PAS BONNE, ÇA DONNE LE MEME
  RESULTAT SI ON NE MET PAS schedule(dynamic, chunk) */
#pragma omp parallel for private(i, j, k) shared(mat_A, mat_B, mat_C)
    for (i = 0; i < lines_a; i++)
    {
        //printf("le nombre de threads %d\n", omp_get_num_threads());
        //#pragma omp parallel for
        for (j = 0; j < columns_b; j++)
        {
            //printf("I am thread %d \n", omp_get_thread_num());
            sum = 0;
#pragma omp parallel for schedule(dynamic, chunk) private(t) reduction(+ \
                                                                       : sum)
            for (k = 0; k < columns_a; k++)
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
    int lines_a = input.la, lines_b = input.lb;
    int columns_a = input.ca, columns_b = input.cb;
    int *a = malloc((lines_a * columns_a) * sizeof(int));
    int *b = malloc((lines_b * columns_b) * sizeof(int));
    struct FlatArraysCouple flat_array_couple = {a, b};
    omp_set_num_threads(num_threads);
#pragma omp parallel for
    for (int i = 0; i < lines_a; i++)
    {
        for (int j = 0; j < columns_a; j++)
        {
            a[i * columns_a + j] = matrixA[i][j];
        }
    }
#pragma omp parallel for
    for (int i = 0; i < lines_b; i++)
    {
        for (int j = 0; j < columns_b; j++)
        {
            b[j * lines_b + i] = matrixB[i][j];
        }
    }
    return flat_array_couple;
}

double optimized_parallel_multiply(int num_threads, int **matrixA, int **matrixB, int **matrixC)
{
    int lines_a = input.la, lines_b = input.lb;
    int columns_a = input.ca, columns_b = input.cb;
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
        for (i = 0; i < lines_a; i++)
        {
            iOff = i * columns_a;
            for (j = 0; j < columns_b; j++)
            {
                jOff = j * lines_b;
                tot = 0;
                for (k = 0; k < columns_a; k++)
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
