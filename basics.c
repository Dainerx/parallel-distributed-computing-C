#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

int main() {
    printf("number of proc: %d\n", omp_get_num_procs());
    printf("max of thread: %d\n", omp_get_max_threads());

    if (omp_get_dynamic()) {
        printf("By default, dynamic is enabled\n");
    } else {
        printf("By default, dynamic is not enabled\n");
    }

    omp_set_dynamic(2);
    if (omp_get_dynamic()) {
        printf("Dynamic can be enabled\n");
    } else {
        printf("Dynamic cannot be enabled\n");
    }



    if (omp_get_nested()) {
        printf("By default, set_nested is enabled\n");
    } else {
        printf("By default, set_nested is not enabled\n");

    }
    omp_set_nested(2);
    if (omp_get_nested()) {
        printf("set_nested can be enabled\n");
    } else {
        printf("set_nested cannot be enabled\n");
    }

#pragma omp parallel
    {
        int tid = omp_get_thread_num();
        printf("thread number %d\n", tid);
    }
}
