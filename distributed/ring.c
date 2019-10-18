#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    int rank,world_size,tag=0;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD,&world_size);

    int token = 0, recu;

    if (rank == 0)
    {
        token++;
        MPI_Send(&token,1, MPI_INT,(rank+1)%world_size,tag,MPI_COMM_WORLD);
        printf("I sent %d to %d\n",token,(rank+1)%world_size);
        MPI_Recv(&recu,1,MPI_INT,(rank-1)%world_size,tag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        printf("I recieved %d from %d\n",recu,world_size-1);
    }
    else
    {
        MPI_Recv(&recu,1,MPI_INT,(rank-1)%world_size,tag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        printf("I recieved %d from %d\n",recu,(rank-1)%world_size);
        recu++;
        MPI_Send(&recu,1, MPI_INT,(rank+1)%world_size,tag,MPI_COMM_WORLD);
        printf("I sent %d to %d\n",recu,(rank+1)%world_size);
    }
    MPI_Finalize();
    return EXIT_SUCCESS;
}

