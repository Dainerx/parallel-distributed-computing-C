#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>
#define M 10
#define N 10
#define THREADS_NUMBER 4

/*
int log2_custom (int x)
{
    int count = 0;
    while(x>1)
    {
        x >>= 1;
        count++;
    }
    return count;
}
*/

int start_from(int x)
{
    int p = trunc(log(x) / log(2.0)) + 1;
    return (1 << p); // shift left the bit set on 1 by r in order to produce 2^r
}


int main(int argc, char **argv)
{
    const int root = 0;
    int rank, world_size, tag =16;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &world_size);
    int token = 42; // random value

    // Simple star implementation
    /*
    if (rank == root)
    {
        for (int i = 0; i < world_size; i++)
        {
            if(i != root)
                MPI_Send(&token,1, MPI_INT,i,tag,MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(&token, 1, MPI_INT, root, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process %d, I recieved from root a token of value=%d\n",rank,token);
    }
    */

    // Broadcast using Binomial tree
    MPI_Status status;
    if (rank == root)
    {
        // Move on tree
        for (int i = 1; i < world_size; i*=2)
        {
            // i: the index of the neighbor the root is going to send to.
            MPI_Send(&token,1, MPI_INT,i,tag,MPI_COMM_WORLD);
        }
    }
    else
    {
        // Start by receiving what the root has sent. Note ANY SOURCE here!
        MPI_Recv(&token, 1, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
        printf("I %d recieved from %d\n",rank,status.MPI_SOURCE);
        for(int i = start_from(rank); i+rank  < world_size; i*=2)
        {
            //i + rank very important since we are starting from 2^rank
            MPI_Send(&token,1, MPI_INT,i + rank,tag,MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
