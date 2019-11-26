#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#define M 10
#define N 10


int max_custom(int a, int b)
{
    if (a>=b)
        return a;
    return b;
}

int main(int argc, char **argv)
{
    const int root=0;
    int rank, world_size;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &world_size);
    srand(time(NULL)+rank);

    int **mat = (int **)malloc(M * sizeof(int*));
    for(int i = 0; i < M; i++) mat[i] = (int *)malloc(N * sizeof(int));

    int max_global, max_local;
    max_global = max_local = -1;
    for (int i=0; i<N; i++)
    {
        for (int j = 0; j<M; j++)
        {
            mat[i][j] = rand() % 200;
            max_local = max_custom(mat[i][j],max_local);
        }
    }


    MPI_Reduce(&max_local,&max_global,1,MPI_INT,MPI_MAX,root,MPI_COMM_WORLD);
    if (rank == root)
    {
        MPI_Bcast(&max_global,1, MPI_INT,root,MPI_COMM_WORLD);
        printf("max is %d",max_global);
    }
    else
    {
        printf("max is %d",max_global);
    }


    for(int i=0; i<N; i++)
    {
        free(mat[i]);
    }
    free(mat);
    MPI_Finalize();
    return EXIT_SUCCESS;
}

