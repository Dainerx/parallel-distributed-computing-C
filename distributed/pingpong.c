#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define PING_PONG_LIMIT 10
int main(int argc, char **argv)
{
    int rang;
    int jeton = 17, recu, tag =0;
    MPI_Status   status;
    MPI_Request send_request,recv_request;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rang);

    //pingpong between 2 process
    int neighbor = (rang + 1) % 2; //only works for two
    int req_count = 0;
    while(req_count < PING_PONG_LIMIT)
    {
        if (rang == req_count%2)
        {
            req_count++;
            MPI_Send(&req_count,1, MPI_INT,neighbor,tag,MPI_COMM_WORLD);
            printf("Process %d, I sent %d\n",rang,req_count);
        }
        else
        {
            MPI_Recv(&req_count, 1, MPI_INT, neighbor, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Process %d, I recieved %d\n",rang,req_count);
        }
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
