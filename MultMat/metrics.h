/**
 * Given computing time taken by a sequential algorithm and time taken by a parallel algorithm
 * as doubles returns the speedup.
 */
double speedup(double seq_time, double parallel_time);
/**
 * Given computing time taken by a sequential algorithm, time taken by a parallel algorithm
 * as doubles and number of threads used by the parallel algorithm returns the efficiency.
 */
double efficiency(double seq_time, double parallel_time, int num_threads);
/**
 * Given computing time taken by a parallel algorithm as double
 * and number of threads used by the parallel algorithm returns the cost.
 */
double cost(double parallel_time, int num_threads);
/**
 * Display the metrics of all solvers.
 */
void print_metrics(const char **labels, double metrics[4][4], int n);