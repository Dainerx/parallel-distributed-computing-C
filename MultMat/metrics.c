#include "metrics.h"
#include "stdio.h"

double speedup(double seq_time, double parallel_time)
{
    return seq_time / parallel_time;
}
double efficiency(double seq_time, double parallel_time, int num_threads)
{
    return seq_time / (parallel_time * num_threads);
}
double cost(double parallel_time, int num_threads)
{
    return num_threads * parallel_time;
}

// vars as sizes!!!!
void print_metrics(const char **labels, float **metrics)
{
    printf("Solver\t Time\t Speedup\t Effic\t Cost\n");
    for (int i = 0; i < 3; i++)
    {
        printf("%s\t", labels[i]);
        for (int j = 0; j < 4; j++)
        {
            printf("%f\t", metrics[i][j]);
        }
        printf("\n");
    }
}