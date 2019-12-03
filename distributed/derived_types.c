#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <time.h>
#define M 4
#define N 3


int main(int argc, char **argv)
{
    int rank, world_size, token,tag =0;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &world_size);
    int* mat = (int*)malloc(M*N * sizeof(int));
    srand(time(NULL)+rank);
    for(int i = 0; i< N*M; i++)
    {
        mat[i] = rand() % 100;
        if (i == (N*M) - 1 )
        printf("%d \n", mat[i]);
        else
        printf("%d \t", mat[i]);
    }
/*    int k, i;
    k = i = 0;
    while(i<N)
    {

        bool pairity = ((rank%2) == 0);
        int to_send_to = (pairity) ? rank + 1 : rank - 1 ;
        int to_recieve_from = (!pairity) ? rank - 1 : rank + 1;
        MPI_Send(&mat[k],1, MPI_INT,to_send_to,tag,MPI_COMM_WORLD);
        MPI_Send(&mat[k+1],1, MPI_INT,to_send_to,tag,MPI_COMM_WORLD);

        MPI_Recv(&token, 1, MPI_INT, to_recieve_from, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("I am %d, I received %d from %d\n", rank, token, to_recieve_from);
        MPI_Recv(&token, 1, MPI_INT, to_send_to, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("I am %d, I received %d from %d\n", rank, token, to_recieve_from);
        k += M;
        i++;
    }
*/
    MPI_Datatype dt;
    int nbblocs = N;
    int longbloc = 1;
    int pas = M;
    int recieve[N*M];
    MPI_Type_vector(nbblocs, longbloc, pas, MPI_INT, &dt);
    MPI_Type_commit(&dt);
  
    bool pairity = ((rank%2) == 0);
    int to_send_to = (pairity) ? rank + 1 : rank - 1 ;
    int to_recieve_from = (!pairity) ? rank - 1 : rank + 1;
    MPI_Send(mat,1, dt,to_send_to,tag,MPI_COMM_WORLD);
    MPI_Send(mat+1,1, dt,to_send_to,tag,MPI_COMM_WORLD);

    MPI_Recv(&recieve, 1, dt, to_recieve_from, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    for(int i =0 ; i< N*M; i=i+M)
    {
      printf(" I am %d I received %d from %d \n",rank, recieve[i], to_recieve_from);
    }

    MPI_Recv(&recieve, 1, dt, to_recieve_from, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    for(int i =1 ; i< N*M; i=i+M)
    {
      printf(" I am %d I received %d from %d \n",rank, recieve[i], to_recieve_from);
    }

    MPI_Type_free(&dt);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
