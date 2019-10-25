#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

struct Node
{
    double data;
    struct Node* next;
};


void loop(struct Node* head)
{
    printf("%f -> ",head->data);
    while(head->next!=NULL)
    {
        printf("%f -> ",head->next->data);
        head = head->next;
    }
}

int main()
{
    struct Node* head = NULL;
    struct Node* second = NULL;
    struct Node* third = NULL;

    // allocate 3 nodes in the heap
    head = (struct Node*)malloc(sizeof(struct Node));
    second = (struct Node*)malloc(sizeof(struct Node));
    third = (struct Node*)malloc(sizeof(struct Node));


    head->data = 1.0;
    head->next = second;

    second->data = 2.0;
    second->next = third;

    third->data = 3.0;
    third->next = NULL;

    loop(head);
}

