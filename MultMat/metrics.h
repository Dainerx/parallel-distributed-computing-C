
double speedup(double seq_time, double parallel_time);
double efficiency(double seq_time, double parallel_time, int num_threads);
double cost(double parallel_time, int num_threads);
void print_metrics(const char **labels, float **metrics);