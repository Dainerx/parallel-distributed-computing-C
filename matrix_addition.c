#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#define MAXVAL 20

int main(int argc, char *argv[]) {
    int n = atoi(argv[1]);
    int p = atoi(argv[2]);
    printf("n = %d\n", n);
    printf("p = %d\n\n", p);

    //matrices memory allocations
    int **a = (int **) malloc(n * sizeof (int*));
    for (int i = 0; i < n; i++) a[i] = (int *) malloc(n * sizeof (int));
    int **b = (int **) malloc(n * sizeof (int*));
    for (int i = 0; i < n; i++) b[i] = (int *) malloc(n * sizeof (int));
    int **c = (int **) malloc(n * sizeof (int*));
    for (int i = 0; i < n; i++) c[i] = (int *) malloc(n * sizeof (int));

    srand(getpid());
    //matrices init
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            a[i][j] = rand() % MAXVAL;
            b[i][j] = rand() % MAXVAL;
        }
    }

    //start seq
    clock_t start, end;
    double cpu_time_used_seq;
    start = clock();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            c[i][j] = a[i][j] + b[i][j];
        }
    }
    end = clock();
    cpu_time_used_seq = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("seq: for n=%d, time taken=%f\n", n, cpu_time_used_seq);
    //end seq

    printf("\n======================================\nUsing Parallelism: \n");
    omp_set_num_threads(p);
    //start parallel
    //for bigger values of n obviously the speedup will remarkably increase.
    double start_parallel = omp_get_wtime();
#pragma omp parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            //see which thread handling entry (i,j)
            //printf("thread %d working on (%d,%d)\n", omp_get_thread_num(), i, j);
            c[i][j] = a[i][j] + b[i][j];
        }
    }
    double end_parallel = omp_get_wtime();
    double cpu_time_used_parallel = (double) (end_parallel - start_parallel);
    printf("parallel: for n=%d, p=%d, time taken=%f, speedup=%f\n", n, p, cpu_time_used_parallel, cpu_time_used_seq / cpu_time_used_parallel);
    
    for(int i=0; i<n; i++)
    {
        free(a[i]);
        free(b[i]);
        free(c[i]);
    }
    free(a);
    free(b);
    free(c);
}
