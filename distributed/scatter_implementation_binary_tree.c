#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <math.h>
#include <time.h>
#define P 3

typedef struct elements_count
{
    int left;
    int right;
};

typedef struct elements
{
    int * left;
    int * right;
};

// Get levels in a tree of n elements
int tree_level(int n)
{
    int l = 1;
    l+=trunc(log(n) / log(2.0));
    return l;
}

// Get the rank level in the tree
int rank_level(int rank)
{
    int l = 1;
    l+=trunc(log(rank+1) / log(2.0));
    return l;
}

// Return tree elements depending on the levels
int tree_elements_count(int l)
{
    return (1<<(l)) - 1;
}

// N is the size of the current binary tree
struct elements_count tree_elements_count_left_right(int n)
{
    int l = tree_level(n);
    int s = tree_elements_count(l);
    int left,right;
    if (s > n)
    {
        int r = s - n;
        int t = tree_elements_count(l-1);
        left = (t/2) + (l > r ) ? l : r ;
        right = (t/2)  + ( r > l ) ?  r%l:0  ;
    }
    else
    {
        left = right = (n - 1) / 2 ;
    }
    struct elements_count result = {left,right};
    return result;
}

void fill_t(int * input, int * result, int n, int rank)
{
    result[0] = input[rank];
    int k = 1;
    int start_ = (rank * 2 ) + 1;
    for (int j = rank_level(rank); j<tree_level(P); j*=2)
    {
        int i;
        for(i = start_; i <start_ + j && i<P; i++)
        {
            result[k] = input[i];
            k++;
            printf("%d\n",k);
        }
        start_ = (start_ * 2 ) + 1;
    }
    printf("k>%d\n",k);
}

// Get elements in the binary tree of root rank.
int get_n(int rank)
{
    int n = 1;
    int start_ = (rank * 2 ) + 1;
    for (int j = rank_level(rank); j<P; j*=2)
    {
        int i;
        for(i = start_; i <start_ + j && i<P; i++)
        {
            n++;
        }
        start_ = (start_ * 2 ) + 1;
    }
    return n;
}

struct elements tree_elements_left_right(int * arr,int rank, int n)
{

    struct elements_count ec = tree_elements_count_left_right(n);
    printf("left right -> %d,%d",ec.left,ec.right);
    int * left = (int*)malloc( ec.left * sizeof(int));
    int * right = (int*)malloc( ec.right * sizeof(int));
    printf("My element: %d\n", arr[0]);

    fill_t(arr, left, n, rank);
    fill_t(arr, right, n, rank);
    struct elements e = { left,right};
    return e;
}


// Launch with four threads
int main(int argc, char **argv)
{
    const int root = 0;
    int rank, world_size,tag=0;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &world_size);

    MPI_Status status;
    if(rank == root)
    {
        int * tab_to_scatter = (int*)malloc( P * sizeof(int));
        srand(time(NULL)+rank);
        for (int i = 0; i < P; i++)
        {
            tab_to_scatter[i] = rand() % 100;
            printf("%d\t", tab_to_scatter[i]);
        }
        printf("\n");
        struct elements e = tree_elements_left_right(tab_to_scatter, rank, P);
        for (int i = 0; i<sizeof(e.left); i++)
        {
            printf("%d\t",e.left[i]);
        }
        printf("\n");

        for (int i = 0; i<sizeof(e.right); i++)
        {
            printf("%d\t",e.right[i]);
        }

        MPI_Send(e.left,1, MPI_INT,(rank * 2 ) + 1,tag,MPI_COMM_WORLD);
        MPI_Send(e.right,1, MPI_INT,(rank * 2 ) + 2,tag,MPI_COMM_WORLD);
        //free(tab_to_scatter);
    }
    else
    {
        // receive
        int count_reception = get_n(rank);
        int * reception = (int*) malloc (count_reception * sizeof(int));
        int from = (rank % 2 == 0) ? (rank/2) -1 : rank/2;
        MPI_Recv(reception, count_reception, MPI_INT, from, tag, MPI_COMM_WORLD, &status);
        for (int i = 0; i<count_reception; i++)
        {
            printf("%d\t",reception[i]);
        }
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
