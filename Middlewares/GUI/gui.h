#ifndef __GUI_H__
#define __GUI_H__

#include "BSP/ATK_MD0700/atk_md0700_util.h"
#include "SYSTEM/SYS/sys.h"
#include "font.h"

typedef uint16_t COLOR_DATTYPE;

extern uint8_t   GUI_FONT_TYPE;
extern uint8_t   GUI_FONT_SIZE;
extern uint16_t* pGUI_FOREGROUND_COLOR;
extern uint16_t* pGUI_BACKGROUND_COLOR;

#define GUI_MALLOC_SOURCE SRAMEX
#define LINE_WIDTH_DEFAULT 5
#define GUI_FONT_TYPE_DEFAULT Font_SimSun
#define GUI_FONT_SIZE_DEFAULT PX12

typedef enum {
    BUTTON = 0,
    LIST,
    WINDOW,
    Obj_Type_Min     = BUTTON,
    Obj_Type_Max     = WINDOW,           /* Max */
    Obj_Type_Default = Obj_Type_Min,     /* Default */
    Obj_Type_Cnt     = Obj_Type_Max + 1, /* The number of all */
    Obj_Type_None    = Obj_Type_Cnt,     /* Invalid value */
} ObjType;

typedef struct Obj_struct
{
    uint16_t x; /* Pos X of the start point */
    uint16_t y; /* Pos Y of the start point */
    uint16_t width;
    uint16_t height;
    uint8_t  type;
    // struct Obj_struct* subObj;
    // void (*Draw)(struct Obj_struct*); /* Interface to draw the widget*/
    /* void (*Response)(void*); */
} Obj;

typedef struct
{
    uint16_t borderColor;
    uint8_t  borderWidth;
    uint8_t  borderFlag;
} Border;

#define BORDER_FLAG(borderFlagBit) (0x01 << (borderFlagBit))

#define BORDER_ALL                                                             \
    (BORDER_FLAG(BORDER_TOP) | BORDER_FLAG(BORDER_LEFT) |                      \
     BORDER_FLAG(BORDER_RIGHT) | BORDER_FLAG(BORDER_BOTTOM))
/*    ((0x01 << BORDER_TOP) | (0x01 << BORDER_LEFT) | (0x01 << BORDER_RIGHT) | \
     (0x01 << BORDER_BOTTOM))
     */
#define BORDER_NULL 0x00
typedef enum {
    BORDER_TOP = 0,
    BORDER_LEFT,
    BORDER_RIGHT,
    BORDER_BOTTOM,
    BorderFlagMin     = BORDER_TOP,
    BorderFlagMax     = BORDER_BOTTOM,     /* Max */
    BorderFlagDefault = BorderFlagMin,     /* Default */
    BorderFlagCnt     = BorderFlagMax + 1, /* The number of all */
    BorderFlagNone    = BorderFlagCnt,     /* Invalid value */
} BorderFlagBit;

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
void     GUI_SetFontType(FontName fontName);
void     GUI_SetFontSize(FontSize fontSize);
void     GUI_setForeColor(COLOR_DATTYPE color);
void     GUI_setBackColor(COLOR_DATTYPE color);
uint16_t GUI_GetXORColor(uint16_t color);
void     GUI_DrawStr(Obj* obj, const char* str);
bool     GUI_isTarget(Obj* obj, Position* point);
bool     GUI_GetBorderFlag(Obj* obj, BorderFlagBit botderFlagBit);
void     draw_widget(Obj* obj);
void     drawBorder(Obj* obj, uint16_t borderWidth);
bool     checkBoundary(uint16_t x,
                       uint16_t y,
                       uint16_t width,
                       uint16_t height,
                       Obj*     obj);
#endif
