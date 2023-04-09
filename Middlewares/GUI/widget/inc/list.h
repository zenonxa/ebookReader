#ifndef __LIST_H__
#define __LIST_H__

#include "LinkedList.h"
#include "color.h"
#include "gui.h"

typedef struct List_struct
{
    Obj         obj;
    Obj         scroller;
    Border      border;
    LinkedNode* headItem;
    LinkedList* itemList;
    uint8_t     dividingLineWidth;
    uint8_t     headlineHeight;
    uint8_t     itemHeight;
    void (*DrawList)(struct List_struct*);
} List;

typedef LinkedList ListItem;

List* NewList(uint16_t xpos,
              uint16_t ypos,
              uint16_t width,
              uint16_t height,
              Border*  border,
              uint8_t  headlineWidth,
              uint16_t itemHeight,
              Obj*     scroller,
              uint8_t  lineWidth);
void  AppendSubListItem(List* list, Obj* obj);
void  AppendSubList(List* list);

#endif
