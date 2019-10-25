#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define K 10000
#define CHUNK 4990800
int main(int argc, char **argv)
{
    int rank,world_size,tag=0;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD,&world_size);

    char tab[CHUNK];
    char recu[CHUNK];
    double t1 = MPI_Wtime();
    if (rank == 0)
    {
        printf("size of tab %d\n", sizeof tab);
        MPI_Send(tab,CHUNK, MPI_CHAR,(rank+1),tag,MPI_COMM_WORLD);
        printf("I sent %d oct\n",CHUNK);
        MPI_Recv(recu,CHUNK,MPI_CHAR,(rank+1),tag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        printf("I rec %d oct\n",CHUNK);
    }
    else
    {
        MPI_Recv(recu,CHUNK,MPI_CHAR,rank-1,tag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        printf("I rec %d oct\n",CHUNK);
        MPI_Send(tab,CHUNK, MPI_CHAR,rank-1,tag,MPI_COMM_WORLD);
        printf("I sent %d oct\n",CHUNK);
    }


    if (rank == 0 )
    {
        double t2 = MPI_Wtime();
        printf("Duration for = %f seconds",(t2-t1));
    }
    else
    {
        printf("I finished.");
    }
    MPI_Finalize();
    return EXIT_SUCCESS;
}

