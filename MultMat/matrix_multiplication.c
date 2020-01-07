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
#include "cmd.h"

int main(int argc, char *argv[])
{
  struct CmdInput ci = read_input(argc, argv);
  if (check_input(ci) == false)
  {
    return 1;
  }

  printf("\n    A: (%d,%d)\n    B: (%d,%d)\n", ci.lines_a, ci.columns_a, ci.lines_b, ci.columns_b);
  int num_threads = ci.num_threads;
  printf("    Threads number: %d \n", num_threads);

  int **mat_A = malloc_mat(ci.lines_a, ci.columns_a);
  int **mat_B = malloc_mat(ci.lines_b, ci.columns_b);
  int **mat_C = malloc_mat(ci.lines_a, ci.columns_b);

  const char *labels[3] = {"seq               ", "parallel          ", "parallel optimized"};
  srand(time(NULL));
  fill_mat(mat_A, ci.lines_a, ci.columns_a);
  fill_mat(mat_B, ci.lines_b, ci.columns_b);
  init_solver(ci.lines_a, ci.columns_a, ci.lines_b, ci.columns_b);

  // Sequential solver.
  double cpu_time_used_seq;
  cpu_time_used_seq = sequential_mult(mat_A, mat_B, mat_C);
  float **metrics = malloc_matf(3, 4);
  metrics[0][0] = cpu_time_used_seq;
  metrics[0][1] = 1;
  metrics[0][2] = 1;
  metrics[0][3] = 1;

  // Parallel solver section
  // Simple parallel solving section.
  fill_mat(mat_C, ci.lines_a, ci.columns_b); // Resets cache lines.
  double cpu_time_used_parallel = parallel_mult(num_threads, mat_A, mat_B, mat_C);
  metrics[1][0] = cpu_time_used_parallel;
  metrics[1][1] = speedup(cpu_time_used_seq, cpu_time_used_parallel);
  metrics[1][2] = efficiency(cpu_time_used_seq, cpu_time_used_parallel, num_threads);
  metrics[1][3] = cost(cpu_time_used_parallel, num_threads);

  // Optimized parallel solving section.
  fill_mat(mat_C, ci.lines_a, ci.columns_b); // Resets cache lines.
  cpu_time_used_parallel = optimized_parallel_multiply(num_threads, mat_A, mat_B, mat_C);
  metrics[2][0] = cpu_time_used_parallel;
  metrics[2][1] = speedup(cpu_time_used_seq, cpu_time_used_parallel);
  metrics[2][2] = efficiency(cpu_time_used_seq, cpu_time_used_parallel, num_threads);
  metrics[2][3] = cost(cpu_time_used_parallel, num_threads);

  // Print metrics for each solver.
  print_metrics(labels, metrics);

  // Free matrices to avoid memory leaks.
  free_mat(mat_A, ci.lines_a);
  free_mat(mat_B, ci.lines_b);
  free_mat(mat_C, ci.lines_a);
  return EXIT_SUCCESS;
}
