#include "framework.h"
#include "pch.h"
#include "GenericList.h"



void InitGenericList(NODE** head) {
    *head = NULL;
}

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

void PrintGenericList(NODE* node, void (*fptr)(void*))
{
    while (node != NULL)
    {
        (*fptr)(node->data);
        node = node->next;
    }
}

void FreeGenericList(NODE** head) {
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


bool DeleteNode(NODE** head, void* toDelete, size_t size) {
    NODE* current = *head;
    if (current == NULL) {
        return false;
    }

    NODE* prev = NULL;
    char* target = (char*)malloc(size);
    memcpy(target, toDelete, size);
    char* data = (char*)malloc(size);
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