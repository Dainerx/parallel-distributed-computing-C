#include <time.h>
#include <omp.h>
#include "solver.h"
#include <mpi.h>
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

/**
 * C'est une implémentation de algorithme Strassenqui a moins de complexity run time. 
 * Cet algorithme reduit le nombre de multiplications faites, mais trop cher lorsque il s'agit
 * de allocations mémoires et l'utilisation de processeur du coup pas forcement qu'il soit plus rapide. 
 * */
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

/**
 * C'est une implémentation de Strassen qui transforme les matrices à
 * des tableaux avant de effectuer tout le calcul.
 * La complexité run time c'est la même avec strassen_mult, mais les allocations de mémoire 
 * sont beaucoup plus moins. 
 * */
double strassen_mult_flat(int **mat_A, int **mat_B, int **mat_C, int **mat_C_final, int n)
{
    int *a = (int *)malloc((n * n) * sizeof(int));
    int *b = (int *)malloc((n * n) * sizeof(int));
    int *c = (int *)malloc((n * n) * sizeof(int));
    flat_mat(n, n, a, mat_A);
    flat_mat(n, n, b, mat_B);
    flat_mat(n, n, c, mat_C);

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
#pragma omp parallel for schedule(guided) collapse(2) private(j, k, t, sum) shared(mat_A, mat_B, mat_C)
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

void compute_local_c(int rank, int *lineA, int *flatB, int *local_c)
{
    int k = 0;
    int c = 0;
    int t;
    while (k < input.lb * input.cb)
    {
        t = 0;
        for (int i = 0; i < input.ca; i++)
        {
            t += lineA[i] * flatB[i + k];
        }
        local_c[c] = t;
        k += input.lb;
        c += 1;
        //printf("%d\t", local_c[c]);
    }
    //printf("\n");
}

double mpi_solver(int **matrixA, int **matrixB, int **matrixC)
{
    const int root = 0;
    int rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int *flatA = NULL;
    int *flatB = (int *)malloc((input.lb * input.cb) * sizeof(int));
    int *flatC = NULL;

    double start;
    if (rank == root)
    {
        start = MPI_Wtime();

        // Le root va convertir les matrices à des tableaux
        flatA = (int *)malloc((input.la * input.ca) * sizeof(int));
        for (int i = 0; i < input.la; i++)
        {
            for (int j = 0; j < input.ca; j++)
            {
                flatA[i * input.ca + j] = matrixA[i][j];
            }
        }
        for (int i = 0; i < input.lb; i++)
        {
            for (int j = 0; j < input.cb; j++)
            {
                flatB[j * input.lb + i] = matrixB[i][j];
            }
        }
        // Le root va allouer la taille mémoire pour contenir le résultat de la multiplication
        flatC = malloc((input.la * input.cb) * sizeof(int));
    }

    if (flatB != NULL)
    {
        //printf("Before brodcast rank =  %d, cb = %d, cb = %d\n", rank, input.cb, input.lb);
        // Envoie de la matrice B a tout le monde; B sera partagée
        MPI_Bcast(flatB, input.lb * input.cb, MPI_INT, root, MPI_COMM_WORLD);
    }

    // Chacun alloue un tableau de taille COLUMNS_A pour recevoir sa partie de la matrice A
    // chacun reçoi une ligne de la matrice A
    int *lineA_to_recieve = (int *)malloc(input.ca * sizeof(int));
    if (lineA_to_recieve != NULL)
    {
        // this is wrong
        MPI_Scatter(flatA, input.ca, MPI_INT, lineA_to_recieve, input.ca, MPI_INT, root, MPI_COMM_WORLD);
    }

    // Chacun dispose désormais de la matrice B en 1D (B est partagée) et une ligne de la matrice A
    // Chacun doit avoir comme résultat une ligne de la matrice C
    // Ensuite tout le monde envoie sa partie calculée au root (gather)
    int *local_c_result = (int *)malloc(input.cb * sizeof(int));
    // Chacun calcule la ligne de la matrice C
    if (local_c_result != NULL)
        compute_local_c(rank, lineA_to_recieve, flatB, local_c_result);

    if (flatC != NULL)
    {
        printf("rank: %d\n", rank);
        // Le root récupère le résultat de la muliplication
        MPI_Gather(local_c_result, input.cb, MPI_INT, flatC, input.cb, MPI_INT, root, MPI_COMM_WORLD);
        printf("rank: %d\n", rank);
    }

    free(local_c_result);

    //final result in matrix_c
    double time_used_mpi;
    if (rank == root)
    {
        double end = MPI_Wtime();
        free(flatA);
        //free(flatB);
        free(flatC);
    }

    MPI_Finalize();
    return 0.0;
}