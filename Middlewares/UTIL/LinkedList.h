#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#include "SYSTEM/sys/sys.h"
#include "gui.h"

#define LINKEDLIST_MALLOC_SOURCE SRAMEX
#define ElemType LinkedNodeData

#define LINKEDLIST_FIND_ENABLE 0
#define LINKEDLIST_MODIFY_ENABLE 0

// Forward Declaration for union LinkedNodeData
typedef struct LinkedNode LinkedNode;
typedef struct LinkedList LinkedList;

struct LinkedList
{
    LinkedNode* head;
    LinkedNode* tail;
    uint32_t    size; /* Count for the number of the Node in LinkedList*/
};

typedef union LinkedNodeData
{
    int               data;
    Obj*              obj;
   struct LinkedList subList;
} LinkedNodeData;

/* Link node for double direction linked list*/
struct LinkedNode
{
    ElemType           nodeData;
    struct LinkedNode* prev; /* Pointer pointing to the previous node */
    struct LinkedNode* next; /* Pointer pointing to the next node */
};


LinkedNode* create_node(ElemType* data);       /* Create a linkedlist node */
void        init_LinkedList(LinkedList* list); /* Create a empty linkedlist */
void        free_LinkedList(LinkedList* list); /* Delete a linkedlist */
LinkedNode* get_head(LinkedList* list);
LinkedNode* get_tail(LinkedList* list);
size_t      get_size(LinkedList* list);
void        show_LinkList(LinkedList* linkedlist);
void        push_head(LinkedList* list,
                      ElemType*   data); /* Insert a node before the head node */
void        push_tail(LinkedList* list,
                      ElemType*   data); /* Insert a node after the tail node */
bool        insert_node(LinkedList* list, int index, ElemType* data);
void        del_head(LinkedList* list);
void        del_tail(LinkedList* list);
bool        del_node(LinkedList* list, int index);
LinkedNode* get_node(LinkedList* list, int index);
#if LINKEDLIST_FIND_ENABLE
LinkedNode* find_data(LinkedList* list, ElemType* data);
#endif
#if LINKEDLIST_MODIFY_ENABLE
bool modify_node(LinkedList* list, int index, ElemType* data);
bool modify_data(LinkedList* list, ElemType* data, ElemType* val);
#endif
#endif
