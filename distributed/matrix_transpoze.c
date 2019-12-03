#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <time.h>
#define M 4
#define N 4
#define THREADS_NUMBER 2

// Launch with two threads
int main(int argc, char **argv)
{
    const int root = 0;
    int rank, world_size,tag =0;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &world_size);
    int* mat = (int*)malloc(M*N * sizeof(int));

    if (rank == root )
    {
        srand(time(NULL)+rank);
        for(int i = 0; i< N*M; i++)
        {
            mat[i] = rand() % 100;
            if ((i + 1)%M == 0)
                printf("%d \n", mat[i]);
            else
                printf("%d \t", mat[i]);
        }
    }

    bool parity = ((rank%2) == 0);
    MPI_Datatype dt;
    // MPI_Datatype attributes
    int nbr_blocks = N; // A block here means a cell in the matrix, hence cell X N.
    int block_length = 1; // Trivial
    int step = M; // Remember the matrix is stored as flat array hence the step M.
    int * receive = (int*)malloc(M*N * sizeof(int));

    // Dt type vector since it will be a vector containing the first column of each line
    MPI_Type_vector(nbr_blocks, block_length, step, MPI_INT, &dt);
    MPI_Type_commit(&dt);

    int to_send_to = (parity) ? rank + 1 : rank - 1 ;
    int to_receive_from = (!parity) ? rank - 1 : rank + 1;
    for(int i = 0; i< M; i++)
    {
        // Root sends dt from matrix.
        if (rank == root)
            MPI_Send(mat+i,1, dt,to_send_to,tag,MPI_COMM_WORLD);
        // The other receives it as transpoze and display..
        // Warning: the transpoze is never stored.
        else
        {
            MPI_Recv(receive, N, MPI_INT, to_receive_from, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // Display the N received.
            for(int j=0 ; j< N; j++)
            {
                printf("%d \t", receive[j]);
            }
            printf("\n");
        }
    }

    free(receive);
    free(mat);
    MPI_Type_free(&dt);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
