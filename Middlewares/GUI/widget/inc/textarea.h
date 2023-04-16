#ifndef __TEXTAREA_H__
#define __TEXTAREA_H__

#include "gui.h"

typedef struct Textarea
{
    Obj   obj;
    char* str;
    void (*DrawTextarea)(struct Textarea*);
    Border        border;
    Font          font;
    COLOR_DATTYPE backColor;
} Textarea;

Textarea* NewTextarea(uint16_t      x,
                      uint16_t      y,
                      uint16_t      width,
                      uint16_t      heigh,
                      LocateType    locateType,
                      Font*         font,
                      Border*       border,
                      COLOR_DATTYPE backColor);

#endif
