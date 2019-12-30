#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>
#define LINES_A 4
#define COLUMS_A 3
#define LINES_B 3
#define COLUMS_B 4

struct FlatArraysCouple;

// Takes the matrices to be computed as arguments.
// Perform matrix multiplication sequentially.
// Returns time taken to compute as double.
double sequential_mult(int **mat_A, int **mat_B, int **mat_C);

// Takes number of threads and the matrices to be computed as arguments.
// Perform matrix multiplication in parallel.
// Returns time taken to compute as a double.
double parallel_mult(int num_threads, int **mat_A, int **mat_B, int **mat_C);

// Takes two matrices and convert them to flat arrays.
// Returns one struct of two flat arrays.
struct FlatArraysCouple convert(int num_threads, int **mat_A, int **mat_B);

// Takes number of threads, flat arrays and matrix to result as arguments.
// Returns the time taken to compute.
double optimized_parallel_multiply(int num_threads, int **mat_A, int **mat_B, int **mat_C);