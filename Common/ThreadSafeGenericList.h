#pragma once
// C program for generic linked list 
#include<stdio.h> 
#include<stdlib.h> 
#include<windows.h>

#define SAFE_DELETE_HANDLE(a)  if(a){CloseHandle(a);}

/* A linked list node */
typedef struct Node
{
    // Any data type can be stored in this node 
    void* data;
    HANDLE mutex;
    struct Node* next;
}NODE;


void InitGenericList(NODE** head);

/* Function to add a node at the beginning of Linked List.
   This function expects a pointer to the data to be added
   and size of the data type */
void GenericListPushAtStart(NODE** head_ref, void* new_data, size_t data_size);

/* Function to print nodes in a given linked list.fpitr is used
to access the function to be used for printing current node data.
Note that different data types need different specifier in printf() */
void PrintGenericList(NODE* node, void (*fptr)(void*));


/*
* Frees all the memory taken by list.
*/
void FreeGenericList(NODE** head);


bool DeleteNode(NODE** head, void* toDelete, size_t size);

