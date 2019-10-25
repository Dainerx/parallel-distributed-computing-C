#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#define CHUNK 1024
#define INT_SIZE 4

int main(int argc, char **argv)
{
  const int root=0;
  int rank, world_size;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &world_size);
    srand(time(NULL)+rank);
    int* tab;
    if(rank == root)
    {
      int size_tab = world_size * CHUNK;
      tab = (int *)malloc(size_tab*sizeof(int));
      for(int i = 0; i<size_tab; i++)
      {
        tab[i] = (rand()%200) - 100;
      }
    }
    int* receive = (int *)malloc(CHUNK*sizeof(int));
    MPI_Scatter(tab, CHUNK, MPI_INT, receive, CHUNK,MPI_INT, root, MPI_COMM_WORLD);
    double mean = 0;
    printf("mon rang c est %d, j'ai recu %d\n", rank, receive[0]);
    for(int i = 0; i<CHUNK; i++)
    {
      int rec = receive[i];
      receive[i] = rec * rec * rec;
      mean+= receive[i];
    }
    mean /= CHUNK;
    printf("la moyenne est %f\n", mean);
    double* local_mean = (double *)malloc(1*sizeof(double));
    local_mean[0] = mean;
    double* gather_mean;
    if(rank == root)
    {
      gather_mean = (double *)malloc(world_size*sizeof(double));
    }
    MPI_Gather(local_mean, 1, MPI_DOUBLE, gather_mean, 1, MPI_DOUBLE, root, MPI_COMM_WORLD );
    double mean_of_means = 0;
    if(rank == root)
    {
      for(int i = 0; i<world_size; i++)
        mean_of_means+=gather_mean[i];
      mean_of_means/=world_size;
      printf("Mean of all means is %f", mean_of_means);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
