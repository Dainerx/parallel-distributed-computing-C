#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <time.h>
#define M 4
#define N 3
#define THREADS_NUMBER 2

// Launch with two threads
int main(int argc, char **argv)
{
    int rank, world_size,tag =0;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &world_size);
    int* mat = (int*)malloc(M*N * sizeof(int));
    srand(time(NULL)+rank);
    for(int i = 0; i< N*M; i++)
    {
        mat[i] = rand() % 100;
        if ((i + 1)%M == 0)
            printf("%d \n", mat[i]);
        else
            printf("%d \t", mat[i]);
    }
    printf("\n");

    bool parity = ((rank%2) == 0);

    //Simple communication without using MPI data type.
    /*
    int k, i;
    k = i = 0; // k used because the matrix is stored as flat array.
    while(i<N)
    {

        int to_send_to = (pairity) ? rank + 1 : rank - 1 ;
        int to_recieve_from = (!pairity) ? rank - 1 : rank + 1;
        MPI_Send(&mat[k],1, MPI_INT,to_send_to,tag,MPI_COMM_WORLD);
        MPI_Send(&mat[k+1],1, MPI_INT,to_send_to,tag,MPI_COMM_WORLD);

        MPI_Recv(&token, 1, MPI_INT, to_recieve_from, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("I am %d, I received %d from %d\n", rank, token, to_recieve_from);
        MPI_Recv(&token, 1, MPI_INT, to_send_to, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("I am %d, I received %d from %d\n", rank, token, to_recieve_from);
        k += M;
        i++;
    }
    */

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

    // Send dt from matrix.
    MPI_Send(mat,1, dt,to_send_to,tag,MPI_COMM_WORLD);
    // Receive it.
    MPI_Recv(receive, 1, dt, to_receive_from, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("First column: \n");
    for(int i=0 ; i< N*M; i=i+M)
    {
        printf("I am %d I received %d from %d \n",rank, receive[i], to_receive_from);

    }

    MPI_Send(mat + 1,1, dt,to_send_to,tag,MPI_COMM_WORLD);
    MPI_Recv(receive, 1, dt, to_receive_from, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Second column: \n");
    for(int i=0; i< N*M; i=i+M)
    {
        printf("I am %d I received %d from %d \n",rank, receive[i], to_receive_from);
    }

    free(receive);
    free(mat);
    MPI_Type_free(&dt);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
