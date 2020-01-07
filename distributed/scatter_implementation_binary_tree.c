#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <math.h>
#include <time.h>
#define P 8


typedef struct elements
{
    int left_count;
    int * left;
    int right_count;
    int * right;
};

// Get binary tree size of a root rank.
int get_tree_size(int root)
{
    if (root>=P)
        return 0;
    return get_tree_size((root * 2 ) + 1) + 1  + get_tree_size((root * 2 ) + 2);
}

int get_tree_height(int root)
{
    if (root>=P)
        return 0;
    else
    {
        int lheight = get_tree_height((root * 2 ) + 1);
        int rheight = get_tree_height((root * 2 ) + 2);
        if (lheight > rheight)
            return(lheight+1);
        return(rheight+1);
    }
}

// Get index for subtree
int get_ii(int root, int rank_sub, int rank)
{
    if (root >= P)
        return 0;
    if (root == rank)
        return rank_sub;
    return get_ii((root * 2 )+1,(rank_sub * 2 )+1,rank) + get_ii((root * 2 ) + 2,(rank_sub * 2 )+2,rank);

}
// Return elements of one level
int oo = 0;
void fill_t (int * input, int* output, int rank_launcher, int root, int level)
{
    if (root>=P)
        return;
    if (level == 1)
    {
        output[oo] = input[get_ii(rank_launcher,0,root)];
        oo++;
    }
    else if (level > 1)
    {
        fill_t(input,output,rank_launcher,(root * 2 ) + 1, level-1);
        fill_t(input,output,rank_launcher,(root * 2 ) + 2, level-1);
    }
}

struct elements tree_elements_left_right(int * arr,int rank, int n)
{
    printf("my element %d\n",arr[0]);
    int left_count = get_tree_size((rank*2) + 1);
    int right_count = get_tree_size((rank*2) + 2);

    int * left = (int*)malloc( left_count * sizeof(int));
    int * right = (int*)malloc( right_count * sizeof(int));

    int left_height = get_tree_height((rank*2) + 1);
    int right_height = get_tree_height((rank*2) + 2);

    oo = 0;
    for(int i = 1; i<=left_height; i++)
    {
        fill_t(arr, left, rank, (rank*2) + 1,i);
    }

    oo = 0;
    for(int i = 1; i<=right_height; i++)
    {
        fill_t(arr, right, rank, (rank*2) + 2,i);
    }

    struct elements e = { left_count,left,right_count,right};
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
        MPI_Send(e.left,e.left_count, MPI_INT,(rank * 2 ) + 1,tag,MPI_COMM_WORLD);
        MPI_Send(e.right,e.right_count, MPI_INT,(rank * 2 ) + 2,tag,MPI_COMM_WORLD);
        free(tab_to_scatter);
    }
    else
    {
        // Receive
        int count_reception = get_tree_size(rank);
        int * reception = (int*) malloc (count_reception * sizeof(int));
        int from = (rank % 2 == 0) ? (rank/2) -1 : rank/2;
        MPI_Recv(reception, count_reception, MPI_INT, from, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // print received array
        for (int i = 0; i<count_reception; i++)
            printf("%d\t",reception[i]);
        printf("\n");
        struct elements e = tree_elements_left_right(reception, rank, P);
        if (rank*2 + 1 < P && e.left_count > 0 )
            MPI_Send(e.left,e.left_count, MPI_INT,(rank * 2 ) + 1,tag,MPI_COMM_WORLD);
        if (rank*2 + 2 < P && e.right_count > 0 )
            MPI_Send(e.right,e.right_count, MPI_INT,(rank * 2 ) + 2,tag,MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
