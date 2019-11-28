#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#define M 10
#define N 10
#define THREADS_NUMBER 4

int states[THREADS_NUMBER] = {-1,-1,-1,-1};

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

int two_to_power_custom(int x)
{
    int res = 1;
    for(int i = 0; i < x; i++)
        res *= 2;
    return res;
}


int main(int argc, char **argv)
{
    const int root = 0;
    int rank, world_size,token, tag =0;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &world_size);
    token = rank;

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

    // Binomial tree
    int steps = log2_custom(world_size);
    for (int i = 0; i<steps; i++)
    {
        printf("step %d\n",i);
        int to_send_to = rank + two_to_power_custom(i);
        if (to_send_to < world_size)
        {
            if ((rank == root && i>=0)||(rank != root && i>=(log2_custom(rank) + 1 )))
            {
                printf("I am %d, I sent to %d\n",rank,to_send_to);
                states[to_send_to] = rank;
                MPI_Send(&token,1, MPI_INT,to_send_to,tag,MPI_COMM_WORLD);
            }
        }
        MPI_Wait()
        // Reception
        if (states[rank] != -1)
        {
            MPI_Recv(&token, 1, MPI_INT, states[rank], tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("I am %d, I recieved from %d, token=%d",rank,states[rank],token);
            states[rank] = -1;
        }
    }

    /*
    */
    MPI_Finalize();
    return EXIT_SUCCESS;
}
