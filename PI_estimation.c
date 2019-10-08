#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

#define NBITER  100000000

double f(double x) {
    return (4.0 / (1.0 + x * x));
}

double pi_parallel(int nb_threads, double a, double b, long int N) {
    long int i;
    double pi_value = 0.0;
    double sum = 0.0;
    double xi, h = (b - a) / N;

#pragma omp parallel for num_threads(nb_threads) private(xi) reduction(+:sum) //parallel on number of threads given 
    for (i = 0; i < N; i++) {
        xi = a + h*i;
        sum += f(xi);
    }
    pi_value = h * (sum + (f(a) + f(b)) / 2);
    return pi_value;
}

int main() {
    int nb;
    double r;
    double start, end;
    double time_seq = 0.0;

    printf("\nNb.threads\tPi estimation\tTime\tSpeed up\n");
    for (nb = 1; nb <= 12; nb++) {
        printf("%d\t", nb);
        fflush(stdout);
        start = omp_get_wtime();
        r = pi_parallel(nb, 0, 1, NBITER);
        end = omp_get_wtime();
        if (nb == 1)
            time_seq = end - start;
        printf("\t%1.12lf\t%1.4lf\t%1.4lf\n", r, end - start, time_seq / (end - start));
    }
    return (EXIT_SUCCESS);
} 
