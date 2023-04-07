#ifndef __LINK_LIST_H__
#define __LINK_LIST_H__

#include "SYSTEM/sys/sys.h"

#define LINKEDLIST_MALLOC_SOURCE SRAMEX
#define ElemType int

/* Link node for double direction linked list*/
typedef struct LinkedListNode
{
    ElemType               data;
    struct LinkedListNode* prev; /* Pointer pointing to the previous node */
    struct LinkedListNode* next; /* Pointer pointing to the next node */
} Node;

typedef struct LinkedList
{
    LinkedListNode* head;
    LinkedListNode* tail;
    uint32_t        size; /* Count for the number of the Node in LinkedList*/
} LinkedList;

Node*  create_node(ElemType data);      /* Create a linkedlist node */
void   init_LinkList(LinkedList* list); /* Create a empty linkedlist */
void   free_LinkList(LinkedList* list); /* Delete a linkedlist */
Node*  get_head(LinkedList* list);
Node*  get_tail(LinkedList* list);
size_t get_size(LinkedList* list);
void   show_LinkList(LinkedList* linkedlist);
void   push_head(LinkedList* list,
                 ElemType    data); /* Insert a node before the head node */
void   push_tail(LinkedList* list,
                 ElemType    data); /* Insert a node after the tail node */
bool   insert_node(LinkedList* list, int index, ElemType data);
void   del_head(LinkedList* list);
void   del_tail(LinkedList* list);
bool   del_node(LinkedList* list, ElemType index);
Node*  get_node(LinkedList* list, ElemType index);
Node*  find_data(LinkedList* list, ElemType data);
bool   modify_node(LinkedList* list, int index, ElemType data);
bool   modify_data(LinkedList* list, ElemType data, ElemType val);

#endif
