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
