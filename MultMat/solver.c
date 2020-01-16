#include <time.h>
#include <omp.h>
#include "solver.h"
#include "strassen.h"
#include "matrix_util.h"

struct Input input;

//Initialiser la structure input par le nombre de lignes et de colonnes des matrices A et B
void init_solver(int la, int ca, int lb, int cb)
{
    input.la = la;
    input.ca = ca;
    input.lb = lb;
    input.cb = cb;
}
/*
  Version séquentielle de base
  Multiplication séquentielle des matrices A et B, le résultal sera dans la matrice C
*/
double sequential_mult(int **mat_A, int **mat_B, int **mat_C)
{
    int lines_a = input.la;
    int columns_a = input.ca, columns_b = input.cb;
    double start, end, cpu_time_used;
    start = omp_get_wtime();
    int sum;
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

double strassen_mult(int **mat_A, int **mat_B, int **mat_C, int **mat_C_final, int n)
{
    double start, end, cpu_time_used;
    start = omp_get_wtime();
    strassen(n, mat_A, mat_B, mat_C);
    for (int i = 0; i < input.la; i++)
    {
        for (int j = 0; j < input.cb; j++)
        {
            mat_C_final[i][j] = mat_C[i][j];
        }
    }
    end = omp_get_wtime();
    cpu_time_used = (end - start);
    return cpu_time_used;
}

double strassen_mult_flat(int **mat_A, int **mat_B, int **mat_C, int **mat_C_final, int n)
{
    int *a = (int *)malloc((n * n) * sizeof(int));
    int *b = (int *)malloc((n * n) * sizeof(int));
    int *c = (int *)malloc((n * n) * sizeof(int));
    flat_mat(n, a, mat_A);
    flat_mat(n, b, mat_B);
    flat_mat(n, c, mat_C);

    double start = omp_get_wtime();
    strassen_flat(n, a, b, c);
    double end = omp_get_wtime();
    for (int i = 0; i < input.la; i++)
    {
        for (int j = 0; j < input.cb; j++)
        {
            mat_C_final[i][j] = c[i * n + j];
        }
    }
    double cpu_time_used = (end - start);
    return cpu_time_used;
}

/*
  Version parallèlisée OMP
  Multiplication en parallel version naive des matrices A et B
*/
double parallel_mult(int num_threads, int **mat_A, int **mat_B, int **mat_C)
{
    int lines_a = input.la;
    int columns_a = input.ca, columns_b = input.cb;
    double start, end, cpu_time_used;
    int i, j, k, t, sum;
    omp_set_dynamic(0);
    omp_set_num_threads(num_threads);
    start = omp_get_wtime();
#pragma omp parallel for schedule(guided) collapse(2) private(i, j, k, t, sum) shared(mat_A, mat_B, mat_C)
    for (i = 0; i < lines_a; i++)
    {
        for (j = 0; j < columns_b; j++)
        {
            sum = 0;
            // A tester avec guided ça donne un bon RESULTAT
            // aussi avec static, auto ou tout simplement ne pas mettre de schedule
            for (k = 0; k < columns_a; k++)
            {
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

// Cette fonction renvoie une structure contenant deux tableaux à une dimension obtenus avec les données des matrices A et B
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

/*
  Version améliorée en parallèle avec une meilleure gestion du cache processeur
  Multiplication en parallel version optimisée des matrices A et B, le résultat se trouve dans la matrice C
*/
double optimized_parallel_multiply(int num_threads, int **matrixA, int **matrixB, int **matrixC)
{
    int lines_a = input.la, lines_b = input.lb;
    int columns_a = input.ca, columns_b = input.cb;

    int i, j, k, iOff, jOff;
    int tot;
    double start, end, cpu_time_used;

    start = omp_get_wtime();
    /*
      Appel de la méthode "convert" qui va retourner une structure contenant deux champs a et b (tableaux en une dimensions),
      qui contiennet les données des matrices A et B respectivement
    */
    struct FlatArraysCouple flat_array_couple = convert(num_threads, matrixA, matrixB);
    int *flatA = flat_array_couple.a;
    int *flatB = flat_array_couple.b;
    omp_set_num_threads(num_threads);

//Le but ici est de faire la multiplication des matrices avec des tableaux à une dimension
#pragma omp parallel shared(matrixC) private(j, k, iOff, jOff, tot)
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

    end = omp_get_wtime();
    cpu_time_used = (end - start);
    return cpu_time_used;
}
