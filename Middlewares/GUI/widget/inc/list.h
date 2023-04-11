#ifndef __LIST_H__
#define __LIST_H__

#include "LinkedList.h"
#include "color.h"
#include "gui.h"
#include "widget/inc/textarea.h"

#define RELATIVE_LOCATE_ENABLE 1

typedef struct List_struct
{
    Obj         obj;
    Obj         scroller;
    Border      border;
    LinkedNode* headItem;
    LinkedList* itemList;
    Textarea*   headlineTextarea;
    uint16_t    headlineHeight;
    uint16_t    itemHeight;
    uint8_t     dividingLineHeight;
    void (*DrawList)(struct List_struct*);
} List;

typedef LinkedList ListItem;

List* NewList(uint16_t xpos,
              uint16_t ypos,
              uint16_t width,
              uint16_t height,
              Border*  border,
              uint16_t headlineHeight,
              uint16_t itemHeight,
              Obj*     scroller,
              uint8_t  dividingLineHeight);
void  AppendSubListItem(List* list, uint16_t index, Obj* obj);
void  AppendSubList(List* list);
void  SetListHeadlineTextarea(List* list, Textarea* textarea);

#endif
