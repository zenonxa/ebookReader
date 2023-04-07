#ifndef __GUI_H__
#define __GUI_H__

#include "SYSTEM/SYS/sys.h"
#include "font.h"
#include "BSP/ATK_MD0700/atk_md0700_util.h"

typedef uint16_t COLOR_DATTYPE;

#define GUI_MALLOC_SOURCE SRAMEX
#define LINE_WIDTH_DEFAULT 5
#define GUI_FONT_TYPE_DEFAULT Font_SimSun
#define GUI_FONT_SIZE_DEFAULT PX12

typedef struct Obj_struct
{
    uint16_t           x; /* Pos X of the start point */
    uint16_t           y; /* Pos Y of the start point */
    uint16_t           width;
    uint16_t           height;
    uint8_t            type;
    struct Obj_struct* subObj;
    void (*Draw)(struct Obj_struct*); /* Interface to draw the widget*/
    /* void (*Response)(void*); */
} Obj;

#define InitOBJParam(pObj, xPos, yPos, objWidth, objHeight, draw)              \
    do {                                                                       \
        ((Obj*)pObj)->x      = xPos;                                           \
        ((Obj*)pObj)->y      = yPos;                                           \
        ((Obj*)pObj)->width  = objWidth;                                       \
        ((Obj*)pObj)->height = objHeight;                                      \
        ((Obj*)pObj)->Draw   = &draw;                                          \
        /*((Obj*)pObj)->Response = &response;*/                                \
    } while (0)

FontName GUI_GetFontType(void);
FontSize GUI_GetFontSize(void);
void GUI_SetFontType(FontName fontName);
void GUI_SetFontSize(FontSize fontSize);
void GUI_setForeColor(COLOR_DATTYPE color);
void GUI_setBackColor(COLOR_DATTYPE color);
uint16_t GUI_GetXORColor(uint16_t color);
void GUI_DrawStr(Obj* obj, const char* str);
bool GUI_isClicked(Obj* obj, Position* point);
#endif
