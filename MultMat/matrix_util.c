#include "matrix_util.h"
#include <stdio.h>

// use templates or macros for this
// Allocation d'une matrice avec le nombre de lignes et de colonnes passés en paramètre
int **malloc_mat(int lines, int colums)
{
    int **mat = (int **)malloc(lines * sizeof(int *));
    for (int i = 0; i < lines; i++)
        mat[i] = (int *)malloc(colums * sizeof(int));
    return mat;
}
float **malloc_matf(int lines, int colums)
{
    float **mat = (float **)malloc(lines * sizeof(float *));
    for (int i = 0; i < lines; i++)
        mat[i] = (float *)malloc(colums * sizeof(float));
    return mat;
}

// Désallocation d'une matrice
void free_mat(int **mat, int lines)
{
    for (int i = 0; i < lines; i++)
    {
        free(mat[i]);
    }
    free(mat);
}

// Remplissage d'une matrice avec des valeurs aléatoires
void fill_mat(int **mat, int lines, int colums)
{
    for (int i = 0; i < lines; i++)
    {
        for (int j = 0; j < colums; j++)
        {
            mat[i][j] = (rand() % MAX_VAL) + MIN_VAL;
        }
    }
}

// Affichage d'une matrice
void display_mat(int **mat, int lines, int colums)
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

void add_mats(int n, int **a, int **b, int **c)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            c[i][j] = a[i][j] + b[i][j];
        }
    }
}

void substract_mats(int n, int **a, int **b, int **c)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            c[i][j] = a[i][j] - b[i][j];
        }
    }
}

void multiply_mats(int n, int **a, int **b, int **c)
{
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            c[i][j] = 0;
            for (int t = 0; t < 2; t++)
            {
                c[i][j] = c[i][j] + a[i][t] * b[t][j];
            }
        }
    }
}
void make_square(int **mat_squared, int **mat, int line, int column, int n)
{
    for (int i = 0; i < line; i++)
    {
        for (int j = 0; j < column; j++)
        {
            mat_squared[i][j] = mat[i][j];
        }
    }
}
int get_max(int a, int b)
{
    if (a > b)
        return a;
    else
        return b;
}

bool equal_mats(int lines, int columns, int **a, int **b)
{
    for (int i = 0; i < lines; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            if (a[i][j] != b[i][j])
            {
                printf("Found no equal cell: (%d,%d) a(%d,%d)=%d ; b(%d,%d)=%d\n", i, j, i, j, a[i][j], i, j, b[i][j]);
                return false;
            }
        }
    }
    return true;
}