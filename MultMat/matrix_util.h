#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_VAL 10
#define MIN_VAL 2

/**
 * Allocate a matrix of integers taking as arguments lines and columns.
 */
int **malloc_mat(int lines, int colums);

/**
 * Allocate a matrix of floats taking as arguments lines and columns.
 */
float **malloc_matf(int lines, int colums);

/**
 * Free a dynamic allocated matrix. 
 */
void free_mat(int **mat, int lines);

/**
 * Fill a matrix with random values. 
 */
void fill_mat(int **mat, int lines, int columns);

/**
 * Display a matrix.
 */
void display_mat(int **mat, int lines, int colums);

/**
 * Performs an addition of the two matrix a and b resulting into c.
 */
void add_mats(int n, int **a, int **b, int **c);

/**
 * Performs a substraction of the two matrix a and b resulting into c.
 */
void substract_mats(int n, int **a, int **b, int **c);

/**
 * Performs a multiplication of the two matrix a and b resulting into c.
 */
void multiply_mats(int n, int **a, int **b, int **c);

/**
 * Transform a matrix passed as argument into a square matrix of dimension n.
 */
void make_square(int **mat_squared, int **mat, int line, int column, int n);

/**
 * Returns the max between a and b.
 */
int get_max(int a, int b);

/**
 * Get the upper boundary 2^x of n. 
 * Returns int.
 */
int get_power_2(int n);

/**
 * Allocate a matrix flattened as a 1D array.
 */
int *malloc_flat_mat(int n);

/**
 * Transform a matrix to a 1D array.
 */
void flat_mat(int n, int *m, int **mat);

/**
 * Performs an addition of the two flat matrices a and b resulting into c.
 */
void add_flat_mats(int n, int *a, int *b, int *c);

/**
 * Performs a substraction of the two matrix a and b resulting into c.
 */
void substract_flat_mats(int n, int *a, int *b, int *c);

/**
 * Checks wether two matrices are equal or not.
 * Loop through both matrices comparing every cell.
 * Returns true if equals, false else.
 */
bool equal_mats(int lines, int columns, int **a, int **b);
