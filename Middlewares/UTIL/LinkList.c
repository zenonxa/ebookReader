// #include <stdio.h>
#include "LinkList.h"
#include "MALLOC/malloc.h"
#include "log.h"

// 创建双向链表结点
Node* create_node(ElemType data)
{
    Node* node =
        (Node*)mymalloc(LINKEDLIST_MALLOC_SOURCE, sizeof(Node));  // 申请内存
    node->data = data;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

void init_LinkList(LinkedList* linkedlist)
{
    linkedlist->head = NULL;
    linkedlist->tail = NULL;
    linkedlist->size = 0;
}

void free_LinkList(LinkedList* linkedlist)
{
    Node* tmp = NULL;
    while (linkedlist->size) {
        Node* tmp        = linkedlist->head;
        linkedlist->head = linkedlist->head->next;
        myfree(LINKEDLIST_MALLOC_SOURCE, tmp);
        tmp = NULL;
        linkedlist->size--;
    }
}

// 获取头结点
Node* get_head(LinkedList* linkedlist)
{
    return linkedlist->head;
}

// 获取尾结点
Node* get_tail(LinkedList* linkedlist)
{
    return linkedlist->tail;
}

// 获取链表长度
size_t get_size(LinkedList* linkedlist)
{
    return linkedlist->size;
}

void show_LinkList(LinkedList* linkedlist)
{
    Node* node = linkedlist->head;
    for (int i = 0; i < linkedlist->size; ++i) {
        log_n("%d ", node->data);
        node = node->next;
    }
}

// 插入头结点
void push_head(LinkedList* linkedlist, ElemType data)
{
    Node* new_node = create_node(data);

    // 链表为空
    if (linkedlist->size == 0) {
        // 插入的结点既是头结点，也是尾结点
        linkedlist->head = new_node;
        linkedlist->tail = new_node;
    }
    // 链表不为空
    else {
        // 将新结点和链表通过前驱指针prev和后继指针next连接起来
        new_node->next         = linkedlist->head;
        linkedlist->head->prev = new_node;
        // 将头结点改为新插入的结点
        linkedlist->head = new_node;
    }
    linkedlist->size++;
}

// 插入尾结点
void push_tail(LinkedList* linkedlist, ElemType data)
{
    Node* new_node = create_node(data);

    // 链表为空
    if (linkedlist->size == 0) {
        // 插入的结点既是尾结点，也是头结点
        linkedlist->tail = new_node;
        linkedlist->head = new_node;
    }
    // 链表不为空
    else {
        // 将新结点和链表通过前驱指针prev和后继指针next连接起来
        linkedlist->tail->next = new_node;
        new_node->prev         = linkedlist->tail;
        // 将尾结点改为新插入的结点
        linkedlist->tail = new_node;
    }
    linkedlist->size++;
}

bool insert_node(LinkedList* linkedlist, int index, ElemType data)
{
    // 插入的位置超出链表的范围
    if (index < 0 || index > linkedlist->size) {
        return false;
    }
    // 插入的位置如果等于0，等同于插入头结点
    if (index == 0) {
        push_head(linkedlist, data);
        return true;
    }
    // 插入的位置如果等于链表的长度，等同于插入尾结点
    else if (index == linkedlist->size) {
        push_tail(linkedlist, data);
        return true;
    }

    Node* tmp = NULL;
    // 插入的位置如果小于等于中间位置，那插入的位置更靠近头结点
    // 从头结点开始往后遍历到插入的前一个位置
    if (index <= linkedlist->size / 2) {
        tmp = linkedlist->head;
        for (int i = 0; i < index - 1; i++) {
            tmp = tmp->next;
        }
    }
    // 插入的位置如果大于中间位置，那插入的位置更靠近尾结点
    // 从尾结点开始往前遍历到插入的前一个位置
    else {
        tmp = linkedlist->tail;
        for (int i = 0; i < linkedlist->size - index; ++i) {
            tmp = tmp->prev;
        }
    }

    Node* new_node = create_node(data);

    tmp->next->prev = new_node;
    new_node->next  = tmp->next;
    tmp->next       = new_node;
    new_node->prev  = tmp;
    linkedlist->size++;
    return true;
}

// 删除头结点
void del_head(LinkedList* linkedlist)
{
    if (linkedlist->size == 0) {
        return;
    }
    if (linkedlist->size == 1) {
        myfree(LINKEDLIST_MALLOC_SOURCE, linkedlist->head);  // 释放内存
        init_list(linkedlist);
        return;
    }

    Node* tmp              = linkedlist->head;  // 保存头结点
    linkedlist->head       = tmp->next;
    linkedlist->head->prev = NULL;

    // 释放内存
    myfree(LINKEDLIST_MALLOC_SOURCE, tmp);
    tmp = NULL;
    linkedlist->size--;
}

void del_tail(LinkedList* linkedlist)
{
    if (linkedlist->size == 0) {
        return;
    }
    if (linkedlist->size == 1) {
        myfree(LINKEDLIST_MALLOC_SOURCE, linkedlist->tail);  // 释放内存
        init_list(linkedlist);
        return;
    }

    Node* tmp              = linkedlist->tail;  // 保存尾结点
    linkedlist->tail       = tmp->prev;
    linkedlist->tail->next = NULL;

    myfree(LINKEDLIST_MALLOC_SOURCE, tmp);  // 释放内存
    tmp = NULL;
    linkedlist->size--;
}

// 删除任意位置结点
bool del_node(LinkedList* linkedlist, ElemType index)
{
    // 判断插入的位置是否超出链表的范围
    if (index < 0 || index > linkedlist->size) {
        return false;
    }
    // 删除的位置如果等于0，等同于删除头结点
    if (index == 0) {
        del_head(linkedlist);
        return true;
    }
    // 删除的位置如果等于链表的长度，等同于删除尾结点
    if (index == linkedlist->size) {
        del_tail(linkedlist);
        return true;
    }

    Node* tmp = NULL;

    // 删除的位置如果小于中间位置
    if (index < linkedlist->size / 2) {
        // 从头结点开始往后遍历到删除的的结点
        tmp = linkedlist->head;
        for (int i = 0; i < index; ++i) {
            tmp = tmp->next;
        }
    }
    // 删除的位置如果大于等于中间位置
    else {
        // 从尾结点开始往前遍历到删除的结点
        tmp = linkedlist->tail;
        for (int i = 0; i < linkedlist->size - index - 1; ++i) {
            tmp = tmp->prev;
        }
    }

    tmp->prev->next = tmp->next;
    tmp->next->prev = tmp->prev;
    myfree(LINKEDLIST_MALLOC_SOURCE, tmp);  // 释放内存
    tmp = NULL;
    linkedlist->size--;
    return true;
}

// 获取任意位置结点
Node* get_node(LinkedList* linkedlist, ElemType index)
{
    // 判断插入的位置是否超出链表的范围
    if (index < 0 || index > linkedlist->size)
        return NULL;
    Node* node = NULL;
    // 目标结点的位置如果小于中间位置
    if (index < linkedlist->size / 2) {
        // 从头结点开始往后遍历到目标结点
        node = linkedlist->head;
        for (int i = 0; i < index; i++) {
            node = node->next;
        }
    }
    // 目标结点的位置如果大于等于中间位置
    else {
        // 从尾结点开始往前遍历到目标结点
        node = linkedlist->tail;
        for (int i = 0; i < linkedlist->size - index - 1; ++i) {
            node = node->prev;
        }
    }
    return node;
}

Node* find_data(LinkedList* linkedlist, ElemType data)
{
    if (linkedlist->size == 0)
        return NULL;
    Node* node = linkedlist->head;
    while (node) {
        if (node->data == data)
            return node;
        node = node->next;
    }
    return NULL;
}

bool modify_node(LinkedList* linkedlist, int index, ElemType data)
{
    Node* node = get_node(linkedlist, index);
    if (node) {
        node->data = data;
        return true;
    }
    return false;
}

bool modify_data(LinkedList* linkedlist, ElemType data, ElemType val)
{
    Node* node = find_data(linkedlist, data);
    if (node) {
        node->data = val;
        return true;
    }
    return false;
}
