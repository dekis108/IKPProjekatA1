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


/*
* Frees all the memory taken by list.
*/
void FreeGenericList(NODE **head) {
    NODE* temp = *head;
    if (temp == NULL) {
        return;
    }

    NODE* next;
    DWORD dwWaitResult;
    while (temp != NULL) {
        next = temp->next;
        free(temp->data);
        temp->data = NULL;
        temp->next = NULL;
        free(temp);
        temp = next;
    }

    *head = NULL;
}

/*
void DeleteNode(NODE ** head, void * toDelete) {
    NODE* current = *head;
    char* toDeleteBytes;
    char* currentBytes;
    bool found = false;
    memcpy(toDeleteBytes, toDelete, sizeof(toDelete));
    while (current != NULL) {
        memcpy(currentBytes, current, sizeof(current));
        for (int i = 0; i < sizeof(toDelete); i++) {
            if (toDeleteBytes[i] != currentBytes[i]) {
                found = false;
            }
            else {
                found = true;
            }
            if (i == sizeof(toDelete) && found == true) {
                NODE* temp;
                temp = current->next;
                current->next = temp->next;
                free(temp);
                break;
            }
        }
        current = current->next;
    }
}
*/

bool DeleteNode(NODE** head, void* toDelete, size_t size ) {
    NODE* current = *head;
    if (current == NULL) {
        return false;
    }

    NODE* prev = NULL;
    char*  target = (char*)malloc(size);
    memcpy(target, toDelete, size);
    char *data = (char*)malloc(size);
    while (current != NULL) {
        memcpy(data, current->data, size);
        if (*target == *data) {
            //printf("found\n");

            if (prev == NULL) {
                (*head) = current->next;
            }
            else {
                prev->next = current->next;
            }
            free(current->data);
            free(current);
            free(data);
            free(target);
            return true;
        }
        prev = current;
        current = current->next;
    }
    free(target);
    free(data);
    return false;
}

