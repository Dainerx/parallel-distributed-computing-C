#include "metrics.h"
#include "stdio.h"
#include <string.h>

// Calculer les speedup
double speedup(double seq_time, double parallel_time)
{
    return seq_time / parallel_time;
}

// Calculer l'éfficacité
double efficiency(double seq_time, double parallel_time, int num_threads)
{
    return seq_time / (parallel_time * num_threads);
}

// Calculer le cout
double cost(double parallel_time, int num_threads)
{
    return num_threads * parallel_time;
}

// vars as sizes!!!!
// Affichage des métriques
void print_metrics(const char **labels, float **metrics)
{
    char *esp = "   ";
    char *menu = "Solver\t\t\t   Time\t\t\t   Speedup\t\t   Efficiency\t\t   Cost\n";
    printf("\n%s %s", esp, menu);
    printf("   -----------------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < 4; i++)
    {
        printf("%s %s\t\t", esp, labels[i]);
        for (int j = 0; j < 4; j++)
        {
            printf("%s%f\t\t", esp, metrics[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}
