#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>
#include <stdarg.h>
#include "matrix_util.h"
#include "solver.h"
#include "metrics.h"
#define LINES_A 4
#define COLUMS_A 3
#define LINES_B 3
#define COLUMS_B 4

int print_colored(int color, const char *format, ...)
{
  switch (color)
  {
  case 1:                 //red
    printf("\033[0;31m"); // sets color for red
    break;
  case 2:
    printf("\033[0;31m"); // sets color for green
    break;
  default:
    break;
  }
  va_list arg;
  int done;

  va_start(arg, format);
  done = vfprintf(stdout, format, arg);
  va_end(arg);
  printf("\033[0m"); // resets the text to default color
  return done;
}
bool check_input(int la, int ca, int lb, int cb)
{
  const char *DIMENSION_INTEGRITY = "ERROR: LINES_B # COLUMNS_A. A: %d x %d, B: %d x %d.\n";
  const char *DIMENSION_MAX = "ERROR: LINES * COLUMNS > 10e6. A: %d x %d, B: %d x %d.\n";

  if (ca != lb)
  {
    print_colored(1, DIMENSION_INTEGRITY, la, ca, lb, cb);
    return false;
  }
  if (la * ca > 10e6 || lb * cb > 10e6)
  {
    print_colored(1, DIMENSION_MAX, la, ca, lb, cb);
    return false;
  }
  return true;
}

int main(int argc, char *argv[])
{
  if (!check_input(LINES_A, COLUMS_A, LINES_B, COLUMS_B))
  {
    return -1;
  }

  int **mat_A = malloc_mat(LINES_A, COLUMS_A);
  int **mat_B = malloc_mat(LINES_B, COLUMS_B);
  int **mat_C = malloc_mat(LINES_A, COLUMS_B);
  const char *labels[3] = {"seq", "parallel", "parallel optimized"};
  srand(time(NULL));
  fill_mat(mat_A, LINES_A, COLUMS_A);
  fill_mat(mat_B, LINES_B, COLUMS_B);

  double cpu_time_used_seq;
  cpu_time_used_seq = sequential_mult(mat_A, mat_B, mat_C);
  float **metrics = malloc_matf(3, 4);
  metrics[0][0] = cpu_time_used_seq;
  metrics[0][1] = 1;
  metrics[0][2] = 1;
  metrics[0][3] = 1;

  int num_threads = 2;
  double cpu_time_used_parallel = parallel_mult(num_threads, mat_A, mat_B, mat_C);
  metrics[1][0] = cpu_time_used_parallel;
  metrics[1][1] = speedup(cpu_time_used_seq, cpu_time_used_parallel);
  metrics[1][2] = efficiency(cpu_time_used_seq, cpu_time_used_parallel, num_threads);
  metrics[1][3] = cost(cpu_time_used_parallel, num_threads);

  cpu_time_used_parallel = optimized_parallel_multiply(num_threads, mat_A, mat_B, mat_C);
  metrics[2][0] = cpu_time_used_parallel;
  metrics[2][1] = speedup(cpu_time_used_seq, cpu_time_used_parallel);
  metrics[2][2] = efficiency(cpu_time_used_seq, cpu_time_used_parallel, num_threads);
  metrics[2][3] = cost(cpu_time_used_parallel, num_threads);
  print_metrics(labels, metrics);
  /*printf("\nMat C\n");
  display_mat(mat_C, LINES_A, COLUMS_B);*/
  /*printf("\nMat A\n");
  display_mat(mat_A, LINES_A, COLUMS_A);
  printf("\nMat B\n");
  display_mat(mat_B, LINES_B, COLUMS_B);
  printf("\nMat C\n");
  display_mat(mat_C, LINES_A, COLUMS_B);*/

  free_mat(mat_A, LINES_A);
  free_mat(mat_B, LINES_B);
  free_mat(mat_C, LINES_A);
  return EXIT_SUCCESS;
}
