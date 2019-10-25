//mpiexec --hostfile hostfile -n 4 --xterm -1\! ./broadcast
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
int main(int argc, char **argv)
{
    const int root = 0;
    int rank;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int len;
    char machine_name[MPI_MAX_PROCESSOR_NAME];
    if (rank==root)
        MPI_Get_processor_name(&machine_name,&len);
    MPI_Bcast(&machine_name,MPI_MAX_PROCESSOR_NAME, MPI_CHAR,root,MPI_COMM_WORLD);
    if (rank == root)
        printf("I am sending my name: %s\n",machine_name);
    else
        printf("%s\n",machine_name);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
