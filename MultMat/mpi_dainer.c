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
    const int root = 0;
    int rank, world_size;
    int sum = 0, i, j, rc;
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
    }

    int mat_A[ci.lines_a][ci.columns_a];
    int mat_B[ci.lines_b][ci.columns_b];
    int actual_C[ci.lines_a][ci.columns_b];
    int expected_C[ci.lines_a][ci.columns_b];

    int *flatA = malloc((ci.lines_a * ci.columns_b / world_size) * sizeof(int));
    int *flatC = malloc((ci.lines_a * ci.columns_b / world_size) * sizeof(int));

    // For display purposes.
    //const char *labels[2] = {"seq native", "distributed"};
    //double metrics[2][4];

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
    }

    double start;
    if (rank == root)
        start = MPI_Wtime();

    // Blocks everyone waiting for root to finish filling matA and matB and perform sequential solving.
    MPI_Barrier(MPI_COMM_WORLD);
    // Scatter rows of matrix A to different processes
    MPI_Scatter(mat_A, ci.lines_a * ci.columns_a / world_size, MPI_INT, flatA, ci.lines_a * ci.columns_a / world_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Broadcast matrix B to all processes
    MPI_Bcast(mat_B, ci.lines_b * ci.columns_b, MPI_INT, 0, MPI_COMM_WORLD);

    // Perform vector multiplication by all processes

    for (i = 0; i < ci.columns_b; i++)
    {
        for (j = 0; j < ci.lines_b; j++)
        {
            sum = sum + flatA[j] * mat_B[j][i];
        }
        flatC[i] = sum;
        sum = 0;
    }

    MPI_Gather(flatC, ci.lines_a * ci.columns_b / world_size, MPI_INT, actual_C, ci.lines_a * ci.columns_b / world_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0)
    {
        /*
        for (int i = 0; i < ci.lines_a; i++)
        {
            for (int j = 0; j < ci.columns_b; j++)
            {
                printf("%d\t", actual_C[i][j]);
            }
            printf("\n");
        }
        */
        double end = MPI_Wtime();
        printf("distributed time: %f\n", end - start);
        printf("seq time: %f\n", multseq(mat_A, mat_B, expected_C, actual_C));
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}