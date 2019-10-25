#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define K 10000
int main(int argc, char **argv)
{
    int rank,world_size,tag=0;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD,&world_size);

    int token = 0, recu;

    int exchange = 0;
    double t1 = MPI_Wtime();
    for(int i=0; i<K; i++)
    {
        if (rank == 0)
        {
            MPI_Send(&token,1, MPI_INT,(rank+1),tag,MPI_COMM_WORLD);
            printf("I sent %d to %d\n",token,(rank+1)%world_size);
            MPI_Recv(&recu,1,MPI_INT,(rank+1),tag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            printf("I recieved %d from %d\n",i,rank+1);
        }
        else
        {
            MPI_Recv(&recu,1,MPI_INT,(rank-1)%world_size,tag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            printf("I recieved %d from %d\n",recu,rank-1);
            MPI_Send(&recu,1, MPI_INT,rank-1,tag,MPI_COMM_WORLD);
            printf("I sent %d to %d\n",token,rank-1);
        }
    }

    if (rank == 0 )
    {
        double t2 = MPI_Wtime();
        printf("Duration for %d iterations = %f seconds",K,(t2-t1)/K);
    } else {
        printf("I finished.");
    }
    MPI_Finalize();
    return EXIT_SUCCESS;
}

