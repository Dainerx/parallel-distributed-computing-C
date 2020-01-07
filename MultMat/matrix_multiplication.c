#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>
#include <stdarg.h>
#include <errno.h>
#include "matrix_util.h"
#include "solver.h"
#include "metrics.h"
#define MAX_DIMENSION 10e6

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
    printf("\033[0m"); // resets the text to default color
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
bool check_input(char *argv[], int *la, int *ca, int *lb, int *cb)
{
  const char *INPUT_FORMAT = "ERROR: matrices' dimensions must be integers.\n";
  const char *DIMENSION_INTEGRITY = "ERROR: LINES_B # COLUMNS_A. A: %d x %d, B: %d x %d.\n";
  const char *DIMENSION_MAX = "ERROR: LINES * COLUMNS > 10e6. A: %d x %d, B: %d x %d.\n";

  char *p;
  errno = 0; // not 'int errno', because the '#include' already defined it
  *la = strtol(argv[1], &p, 10);
  *ca = strtol(argv[2], &p, 10);
  *lb = strtol(argv[3], &p, 10);
  *cb = strtol(argv[4], &p, 10);
  if (*p != '\0' || errno != 0)
  {
    print_colored(1, INPUT_FORMAT);
    return false;
  }
  if (*ca != *lb)
  {
    print_colored(1, DIMENSION_INTEGRITY, *la, *ca, *lb, *cb);
    return false;
  }
  if ((*la) * (*ca) > MAX_DIMENSION || (*lb) * (*cb) > MAX_DIMENSION)
  {
    print_colored(1, DIMENSION_MAX, *la, *ca, *lb, *cb);
    return false;
  }
  return true;
}

int main(int argc, char *argv[])
{
  if (argc != 5)
  {
    print_colored(1, "Please enter the dimensions of the matrices A and B followed by the script name.\n");
    print_colored(0, "Run with flag --help for usage.");
    return 1;
  }
  int lines_a, columns_a, lines_b, columns_b;
  if (!check_input(argv, &lines_a, &columns_a, &lines_b, &columns_b ))
  {
    return 1;
  }
  printf("%d,%d %d,%d \n", lines_a, columns_a, lines_b, columns_b);

  int **mat_A = malloc_mat(lines_a, columns_a);
  int **mat_B = malloc_mat(lines_b, columns_b);
  int **mat_C = malloc_mat(lines_a, columns_b);

  const char *labels[3] = {"seq", "parallel", "parallel optimized"};
  srand(time(NULL));
  fill_mat(mat_A, lines_a, columns_a);
  fill_mat(mat_B, lines_b, columns_b);
  init_solver(lines_a, columns_a, lines_b, columns_b);

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

  free_mat(mat_A, lines_a);
  free_mat(mat_B, lines_b);
  free_mat(mat_C, lines_a);
  return EXIT_SUCCESS;
}
