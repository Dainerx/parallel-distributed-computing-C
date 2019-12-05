#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <time.h>
#define M 4
#define N 3
#define THREADS_NUMBER 4

// Launch with four threads
int main(int argc, char **argv)
{
    const int root = 0;
    int rank, world_size,tag=0;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &world_size);

    srand(time(NULL)+rank);
    MPI_Status status;
    int token;
    int * tab_to_scatter;
    // allocation of an array to be destributed
    if(rank == root)
    {
      tab_to_scatter = (int*)malloc( world_size * sizeof(int));
      for (int i = 0; i < world_size; i++)
      {
          tab_to_scatter[i] = rand() % 100;
          printf("%d\t", tab_to_scatter[i]);
      }
      printf("\n");
    }
    // destribution implementation with a star
    if(rank == root)
    {
      for (int i = 0; i < world_size; i++)
      {
          // i: the index of the neighbor the root is going to send to.
          MPI_Send(&tab_to_scatter[i],1, MPI_INT,i,tag,MPI_COMM_WORLD);
      }
    }
    MPI_Recv(&token, 1, MPI_INT, root, tag, MPI_COMM_WORLD, &status);
    printf("I am %d I recieved %d from %d\n",rank,token, root);

    if(rank == root)
         free(tab_to_scatter);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
