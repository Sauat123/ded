#include <stdio.h>
#include <stdlib.h>

typedef struct Node{
    int data;
    struct Node* prev;
    struct Node* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
    int size;
} DoubleLinkedList;

DoubleLinkedList* DoubleLinkedList_init();
Node* create_node(int data);
void delete_at_Nth_position(DoubleLinkedList* list, int n);

void insert_at_Nth_position(DoubleLinkedList* list, int data, int n);

int main()
{
    DoubleLinkedList* list = DoubleLinkedList_init();

    insert_at_Nth_position(list, 1, 0);   // [1]
    insert_at_Nth_position(list, 2, 1);   // [1, 2]
    insert_at_Nth_position(list, 3, 1);   // [1, 3, 2]
    insert_at_Nth_position(list, 4, 3);   // [1, 3, 2, 4]
    insert_at_Nth_position(list, 5, 0);   // [5, 1, 3, 2, 4]
    delete_at_Nth_position(list, 2);      // [5, 1, 2, 4]

    printf("Double Linked List: ");
    Node* current = list->head;

    while (current != NULL)
    {
        printf("%d ", current->data);
        current = current->next;
    }

    printf("\n");

    return 0;
}

DoubleLinkedList* DoubleLinkedList_init()
{

    DoubleLinkedList* list = (DoubleLinkedList*)calloc(sizeof(DoubleLinkedList), 1);

    if (list == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    return list;
}

Node* create_node(int data)
{

    Node* new_node = (Node*)calloc(sizeof(Node), 1);

    if (new_node == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    new_node->data = data;
    new_node->prev = NULL;
    new_node->next = NULL;

    return new_node;
}

void insert_at_Nth_position(DoubleLinkedList* list, int data, int n)
{

    if (n < 0 || n > list->size)
    {
        fprintf(stderr, "Invalid position.\n");
        exit(EXIT_FAILURE);
    }

    Node* new_node = create_node(data);

    if (n == 0)
    {
        if (list->head == NULL)
        {
            list->head = new_node;
            list->tail = new_node;
        }

        else
        {
            new_node->next = list->head;
            list->head->prev = new_node;
            list->head = new_node;
        }
    }

    //printf("num = %d \n", n);

    else if (n == list->size)
    {
        new_node->prev = list->tail;
        list->tail->next = new_node;
        list->tail = new_node;
    }

    else
    {
        Node* current = list->head;
        int i = 0;

        while (i < n - 1)
        {
            current = current->next;
            i++;
        }

        //printf("current node = %d \n", i);

        new_node->prev = current;
        new_node->next = current->next;

        current->next->prev = new_node;
        current->next = new_node;
    }

    list->size++;
}

void delete_at_Nth_position(DoubleLinkedList* list, int n)
{
    if (n < 0 || n >= list->size)
    {
        fprintf(stderr, "Invalid position for delete.\n");
        return;
    }


    Node* node_to_delete = NULL;

    if (n == 0)
    {
        node_to_delete = list->head;
        list->head = node_to_delete->next;
        if (list->head != NULL)
        {
            list->head->prev = NULL;
        }
    }

    else if (n == list->size - 1)
    {
        node_to_delete = list->tail;
        list->tail = node_to_delete->prev;
        if (list->tail != NULL)
        {
            list->tail->next = NULL;
        }
    }

    else
    {
        Node* current = list->head;
        int i = 0;
        while (i < n)
        {
            current = current->next;
            i++;
        }

        //printf("node to delete = %d \n", i);

        node_to_delete = current;
        current->prev->next = current->next;
        current->next->prev = current->prev;
    }

    free(node_to_delete);
    list->size--;
}

