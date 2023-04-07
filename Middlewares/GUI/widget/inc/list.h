#ifndef __LIST_H__
#define __LIST_H__

#include "gui.h"
#include "color.h"

typedef struct List_struct
{
    Obj obj;
    COLOR_DATTYPE borderColor;
    uint16_t itemCnt;
    uint8_t borderWidth;
    uint8_t lineWidth;
    uint8_t itemWidth;
    uint8_t itemHeight;
    Obj* scroller;
} List;


#endif
