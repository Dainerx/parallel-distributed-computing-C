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
struct CmdInput ci;

int main(int argc, char **argv)
{
    const int root = 0;
    int rank, world_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    ci = read_input(rank, argc, argv);
    if (check_input(rank, ci) == false)
    {
        MPI_Finalize();
        return EXIT_SUCCESS;
    }
    else
    {
        print_colored(rank, 2, "    OK: Input valid.");
        print_colored(rank, 0, "\n    A: (%d,%d)\n    B: (%d,%d)\n", ci.lines_a, ci.columns_a, ci.lines_b, ci.columns_b);
    }

    const char *labels[2] = {"seq native", "distributed"};
    double metrics[2][4];

    // Ask root to fill the matrices A and B
    if (rank == root)
    {
        int **mat_A = malloc_mat(ci.lines_a, ci.columns_a);
        int **mat_B = malloc_mat(ci.lines_b, ci.columns_b);
        // Sets a new seed for rand.
        srand(time(NULL));
        // Fill matrices with random values.
        fill_mat(mat_A, ci.lines_a, ci.columns_a);
        fill_mat(mat_B, ci.lines_b, ci.columns_b);

        mat_C_Seq = malloc_mat(ci.lines_a, ci.columns_b);
        init_solver(ci.lines_a, ci.columns_a, ci.lines_b, ci.columns_b);
        metrics[0][0] = sequential_mult(mat_A, mat_B, mat_C_Seq);
        metrics[0][1] = 1;
        metrics[0][2] = 1;
        metrics[0][3] = 1;
    }
    MPI_Finalize();
    return EXIT_SUCCESS;
}