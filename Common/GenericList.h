#pragma once
// C program for generic linked list 
#include<stdio.h> 
#include<stdlib.h> 

/* A linked list node */
typedef struct Node
{
    // Any data type can be stored in this node 
    void* data;
    struct Node* next;
}NODE;


void InitGenericList(NODE** head) {
    *head = NULL;
}

/* Function to add a node at the beginning of Linked List.
   This function expects a pointer to the data to be added
   and size of the data type */
void GenericListPushAtStart(NODE** head_ref, void* new_data, size_t data_size)
{
    // Allocate memory for node 
    NODE* new_node = (NODE*)malloc(sizeof(NODE));

    new_node->data = malloc(data_size);
    new_node->next = (*head_ref);

    // Copy contents of new_data to newly allocated memory. 
    memcpy(new_node->data, new_data, data_size);

    // Change head pointer as new node is added at the beginning 
    (*head_ref) = new_node;
}

/* Function to print nodes in a given linked list.fpitr is used
to access the function to be used for printing current node data.
Note that different data types need different specifier in printf() */
void PrintGenericList(NODE * node, void (*fptr)(void*))
{
    while (node != NULL)
    {
        (*fptr)(node->data);
        node = node->next;
    }
}


