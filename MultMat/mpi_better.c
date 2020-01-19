#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>
#include <mpi.h>
#include <time.h>
#include "mpi_utils.h"
#include "matrix_util.h"
#include "metrics.h"
#include "solver.h"

#define TAG_MASTER 1 /* setting a message type */
#define TAG_WORKER 2 /* setting a message type */

// Variable globale contient l'input
struct CmdInputDistributed ci;

double multseq(int mat_A[ci.lines_a][ci.columns_a], int mat_B[ci.lines_b][ci.columns_b], int expected_C[ci.lines_a][ci.columns_b], int actual_C[ci.lines_a][ci.columns_b])
{

    double start, end, cpu_time_used;
    int sum;
    start = omp_get_wtime();
    for (int i = 0; i < ci.lines_a; i++)
    {
        for (int j = 0; j < ci.columns_b; j++)
        {
            sum = 0;
            for (int k = 0; k < ci.columns_a; k++)
            {
                sum += (mat_A[i][k] * mat_B[k][j]);
            }
            expected_C[i][j] = sum;
        }
    }
    end = omp_get_wtime();

    for (int i = 0; i < ci.lines_a; i++)
    {
        for (int j = 0; j < ci.columns_b; j++)
        {
            if (expected_C[i][j] != actual_C[i][j])
                return -1; // wrong result;
        }
    }
    cpu_time_used = (end - start);
    return cpu_time_used;
}

int main(int argc, char **argv)
{
    double start_time_distributed;
    int rank, world_size;
    int sum = 0, i, j, k, rc;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    ci = read_input(rank, world_size, argc, argv);
    if (check_input(rank, ci) == false)
    {
        MPI_Abort(MPI_COMM_WORLD, rc);
        exit(1);
    }
    else
    {
        print_colored(rank, 2, "    OK: Input valid.");
        print_colored(rank, 0, "\n    A: (%d,%d)\n    B: (%d,%d)\n", ci.lines_a, ci.columns_a, ci.lines_b, ci.columns_b);
        print_colored(rank, 0, "\n    Number of workers: %d\n", world_size - 1);
    }

    // Matrices
    int mat_A[ci.lines_a][ci.columns_a];
    int mat_B[ci.lines_b][ci.columns_b];
    int actual_C[ci.lines_a][ci.columns_b];
    int expected_C[ci.lines_a][ci.columns_b];

    const int root = 0;           // The root rank
    int workers = world_size - 1; // How many machines
    // Variables to know how many rows sent to every worker
    int avg_rows, extra_rows, offset;
    int rows; // rows sent to every worker.

    // Ask root to fill the matrices A and B
    if (rank == root)
    {
        // Sets a new seed for rand.
        srand(time(NULL));
        // Fill matrices with random values.
        for (i = 0; i < ci.lines_a; i++)
            for (j = 0; j < ci.columns_a; j++)
                mat_A[i][j] = (rand() % MAX_VAL) + MIN_VAL;
        for (i = 0; i < ci.lines_b; i++)
            for (j = 0; j < ci.columns_b; j++)
                mat_B[i][j] = (rand() % MAX_VAL) + MIN_VAL;

        // Starts counting time.
        start_time_distributed = MPI_Wtime();
        /* Send matrix data to the worker */
        avg_rows = ci.lines_a / workers;
        extra_rows = ci.lines_a % workers;
        offset = 0;
        for (int worker_destination = 1; worker_destination <= workers; worker_destination++)
        {
            rows = (worker_destination <= extra_rows) ? avg_rows + 1 : avg_rows;
            //printf("Sending %d rows to worker %d offset=%d\n", rows, worker_destination, offset);
            MPI_Send(&offset, 1, MPI_INT, worker_destination, TAG_MASTER, MPI_COMM_WORLD);
            MPI_Send(&rows, 1, MPI_INT, worker_destination, TAG_MASTER, MPI_COMM_WORLD);
            MPI_Send(&mat_A[offset][0], rows * ci.columns_a, MPI_INT, worker_destination, TAG_MASTER,
                     MPI_COMM_WORLD);
            MPI_Send(&mat_B, ci.lines_b * ci.columns_b, MPI_INT, worker_destination, TAG_MASTER, MPI_COMM_WORLD);
            offset = offset + rows;
        }

        // Receive results from workers
        for (int worker_source = 1; worker_source <= workers; worker_source++)
        {
            MPI_Recv(&offset, 1, MPI_INT, worker_source, TAG_WORKER, MPI_COMM_WORLD, &status);
            MPI_Recv(&rows, 1, MPI_INT, worker_source, TAG_WORKER, MPI_COMM_WORLD, &status);
            MPI_Recv(&actual_C[offset][0], rows * ci.columns_b, MPI_INT, worker_source, TAG_WORKER,
                     MPI_COMM_WORLD, &status);
            //printf("Received results from worker %d\n", worker_source);
        }

        double computing_time = MPI_Wtime() - start_time_distributed;

        // Print results
        /*
        printf("******************************************************\n");
        printf("Result Matrix:\n");
        for (i = 0; i < ci.lines_a; i++)
        {
            printf("\n");
            for (j = 0; j < ci.columns_b; j++)
                printf("%d   ", actual_C[i][j]);
        }
        printf("\n******************************************************\n");
        printf("Done.\n");
        */
        printf("%f,%f\n", multseq(mat_A, mat_B, expected_C, actual_C), computing_time);
    }

    // If not root
    else
    {
        // Recieve the rows of A and the matrix B from root.
        MPI_Recv(&offset, 1, MPI_INT, root, TAG_MASTER, MPI_COMM_WORLD, &status);
        MPI_Recv(&rows, 1, MPI_INT, root, TAG_MASTER, MPI_COMM_WORLD, &status);
        MPI_Recv(&mat_A, rows * ci.columns_a, MPI_INT, root, TAG_MASTER, MPI_COMM_WORLD, &status);
        MPI_Recv(&mat_B, ci.lines_b * ci.columns_b, MPI_INT, root, TAG_MASTER, MPI_COMM_WORLD, &status);

        // Every worker's compute its part of matC.
        for (k = 0; k < ci.columns_b; k++)
            for (i = 0; i < rows; i++)
            {
                actual_C[i][k] = 0.0;
                for (j = 0; j < ci.columns_a; j++)
                {
                    actual_C[i][k] = actual_C[i][k] + mat_A[i][j] * mat_B[j][k];
                }
            }
        MPI_Send(&offset, 1, MPI_INT, root, TAG_WORKER, MPI_COMM_WORLD);
        MPI_Send(&rows, 1, MPI_INT, root, TAG_WORKER, MPI_COMM_WORLD);
        MPI_Send(&actual_C, rows * ci.columns_b, MPI_INT, root, TAG_WORKER, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}