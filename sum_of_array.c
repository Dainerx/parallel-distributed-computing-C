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

    printf("\nNb.threads\tN\tSum\tTime\tSpeed up\tComment\n");
    double *tt = malloc(sizeof (double) * n);
    srand(getpid());
    //init array
    for (int i = 0; i < n; i++) {
        tt[i] = rand() % MAXVAL;
    }

    //start eq
    clock_t start, end;
    double cpu_time_used_seq;
    int i = 0;
    double sum = 0;
    start = clock();
    for (i = 0; i < n; i++) {
        sum += tt[i];
    }
    end = clock();
    cpu_time_used_seq = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("%d\t%d\t%f\t%f\tNA\tNA\n", 1, n, sum, cpu_time_used_seq);
    //end seq

    sum = 0;
    double lsum = 0;
    omp_set_num_threads(p);
    //start parallel without reduction
    //this will divide work among threads in non equal way, if the operation had 
    //a huge cost we might end up having the final thread doing a shit ton of work compared to the other threads
    double start_parallel = omp_get_wtime();
#pragma omp parallel private(i,lsum) 
    {
#pragma omp for
        for (i = 0; i < n; i++) {
            //see what index each thread is working on
            //printf("thread %d -> %d\n", omp_get_thread_num(), i);
            lsum += tt[i];
        }
        //synchronization section
#pragma omp critical
        {
            sum += lsum;
        }
    }
    //end parallel without reduction
    double end_parallel = omp_get_wtime();
    double cpu_time_used_parallel = (double) (end_parallel - start_parallel);
    printf("%d\t%d\t%f\t%f\t%f\tparallelism without reduction\n", p, n, sum, cpu_time_used_parallel, cpu_time_used_seq / cpu_time_used_parallel);
    sum = 0;
    //start parallel with reduction
    start_parallel = omp_get_wtime();
#pragma omp parallel for reduction(+:sum)
    for (i = 0; i < n; i++) {
        //see what index each thread is working on
        //printf("thread %d -> %d\n", omp_get_thread_num(), i);
        sum += tt[i];
    }
    //end parallel with reduction
    end_parallel = omp_get_wtime();
    cpu_time_used_parallel = (double) (end_parallel - start_parallel);
    printf("%d\t%d\t%f\t%f\t%f\tparallelism with reduction\n", p, n, sum, cpu_time_used_parallel, cpu_time_used_seq / cpu_time_used_parallel);

    sum = 0;
    int chunk = n / (p * 2); //this is faster than the default scheduling by compiler. n/p would be slower though.
    //printf("Setting chunk to %d\n", chunk);
    start_parallel = omp_get_wtime();
#pragma omp parallel shared(chunk) private (i)
    {
#pragma omp for reduction(+:sum) schedule(dynamic,chunk)
        for (i = 0; i < n; i++) {
            sum += tt[i];
        }
    }
    end_parallel = omp_get_wtime();
    cpu_time_used_parallel = (double) (end_parallel - start_parallel);
    printf("%d\t%d\t%f\t%f\t%f\tparallelism dynamic schedule with reduction\n", p, n, sum, cpu_time_used_parallel, cpu_time_used_seq / cpu_time_used_parallel);

    sum = 0;
    lsum = 0;
    chunk = n / (p * 2); //this is faster than the default scheduling by compiler. n/p would be slower though.
    //printf("Setting chunk to %d\n", chunk);
    //should not be faster.
    start_parallel = omp_get_wtime();
#pragma omp parallel shared(chunk) private (i,lsum)
    {
#pragma omp for schedule(dynamic,chunk)
        for (i = 0; i < n; i++) {
            lsum += tt[i];
        }
#pragma omp atomic
        sum += lsum;
    }
    end_parallel = omp_get_wtime();
    cpu_time_used_parallel = (double) (end_parallel - start_parallel);
    printf("%d\t%d\t%f\t%f\t%f\tparallelism dynamic schedule without reduction\n", p, n, sum, cpu_time_used_parallel, cpu_time_used_seq / cpu_time_used_parallel);

}
