#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#define CHUNK 1014
#define ALLCHUNKS 4056
#define MAXVAL 200000
#define INT_SIZE 4
int main(int argc, char **argv)
{
    const int root = 0;
    int rank;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int data[CHUNK];

    int seed = time(NULL);
    srand(seed + rank);
    for(int i=0; i<CHUNK; i++)
    {
        data[i] = rand() % MAXVAL;
    }

    int max_local = -1;
    for(int i=0; i<CHUNK; i++)
    {
        if (data[i] >= max_local)
            max_local = data[i];
    }

    printf("I am rank %d my Max local %d\n",rank,max_local);

    int max_global = -1;

    MPI_Reduce(&max_local,&max_global,1,MPI_INT,MPI_MAX,root,MPI_COMM_WORLD);
    if (rank==root)
    {
        printf("I am the root %d the max of all maxima %d\n",rank,max_global);
    }
}
