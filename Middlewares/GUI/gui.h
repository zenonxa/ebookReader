#ifndef __GUI_H__
#define __GUI_H__

#include "font.h"
#include "SYSTEM/SYS/sys.h"

typedef struct Obj_struct
{
    uint16_t x; /* Pos X of the start point */
    uint16_t y; /* Pos Y of the start point */
    uint16_t width;
    uint16_t height;
    void (*Draw)(void*); /* Interface to draw the widget*/
} Obj;

#define InitOBJParam(pObj, xpos, ypos, w, h, fun)                              \
    do {                                                                       \
        ((Obj*)pObj)->x      = xpos;                                           \
        ((Obj*)pObj)->y      = ypos;                                           \
        ((Obj*)pObj)->width  = w;                                              \
        ((Obj*)pObj)->height = h;                                              \
        ((Obj*)pObj)->Draw   = &fun;                                           \
    } while (0)

#endif
