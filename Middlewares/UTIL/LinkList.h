#ifndef __LINK_LIST_H__
#define __LINK_LIST_H__

#include "SYSTEM/sys/sys.h"

#define LINKEDLIST_MALLOC_SOURCE SRAMEX
#define ElemType int

/* Link node for double direction linked list*/
typedef struct LinkedNode
{
    ElemType           data;
    struct LinkedNode* prev; /* Pointer pointing to the previous node */
    struct LinkedNode* next; /* Pointer pointing to the next node */
} LinkedNode;

typedef struct LinkedList
{
    LinkedNode* head;
    LinkedNode* tail;
    uint32_t    size; /* Count for the number of the Node in LinkedList*/
} LinkedList;

LinkedNode* create_node(ElemType data);      /* Create a linkedlist node */
void        init_LinkedList(LinkedList* list); /* Create a empty linkedlist */
void        free_LinkedList(LinkedList* list); /* Delete a linkedlist */
LinkedNode* get_head(LinkedList* list);
LinkedNode* get_tail(LinkedList* list);
size_t      get_size(LinkedList* list);
void        show_LinkList(LinkedList* linkedlist);
void        push_head(LinkedList* list,
                      ElemType    data); /* Insert a node before the head node */
void        push_tail(LinkedList* list,
                      ElemType    data); /* Insert a node after the tail node */
bool        insert_node(LinkedList* list, int index, ElemType data);
void        del_head(LinkedList* list);
void        del_tail(LinkedList* list);
bool        del_node(LinkedList* list, ElemType index);
LinkedNode* get_node(LinkedList* list, ElemType index);
LinkedNode* find_data(LinkedList* list, ElemType data);
bool        modify_node(LinkedList* list, int index, ElemType data);
bool        modify_data(LinkedList* list, ElemType data, ElemType val);

#endif
