#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_VAL 10
#define MIN_VAL 2

int **malloc_mat(int lines, int colums);
float **malloc_matf(int lines, int colums);

void free_mat(int **mat, int lines);

void fill_mat(int **mat, int lines, int columns);

void display_mat(int **mat, int lines, int colums);

void add_mats(int n, int **a, int **b, int **c);

void substract_mats(int n, int **a, int **b, int **c);

void multiply_mats(int n, int **a, int **b, int **c);

void make_square(int **mat_squared, int **mat, int line, int column, int n);

int get_max(int a, int b);

bool equal_mats(int lines, int columns, int **a, int **b);
