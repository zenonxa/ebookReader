#ifndef __GUI_H__
#define __GUI_H__

#include "BSP/ATK_MD0700/atk_md0700_util.h"
#include "SYSTEM/SYS/sys.h"
#include "font.h"

typedef uint16_t         COLOR_DATTYPE;
typedef struct Font      Font;
typedef struct Border    Border;
typedef struct AlignType AlignType;

extern uint8_t   GUI_FONT_NAME;
extern uint8_t   GUI_FONT_SIZE;
extern uint16_t* pGUI_FOREGROUND_COLOR;
extern uint16_t* pGUI_BACKGROUND_COLOR;
extern Font      publicFont;
extern Border    publicBorder;
extern AlignType publicAlignType;

#define GUI_MALLOC_SOURCE SRAMEX
#define LINE_WIDTH_DEFAULT 5
#define GUI_FONT_TYPE_DEFAULT Font_SimSun
#define GUI_FONT_SIZE_DEFAULT PX12

typedef enum {
    Obj_Type_Button = 0,
    Obj_Type_List,
    Obj_Type_Textarea,
    Obj_Type_Window,
    Obj_Type_Min     = Obj_Type_Button,
    Obj_Type_Max     = Obj_Type_Window,  /* Max */
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

typedef struct Border
{
    uint16_t borderColor;
    uint8_t  borderWidth;
    uint8_t  borderFlag;
} Border;

typedef struct Font
{
    uint8_t       fontName;
    uint8_t       fontSize;
    COLOR_DATTYPE fontColor;
} Font;

typedef enum {
    DrawOption_Delay = 0,
    DrawOption_Immediately,
    DrawOption_Min     = DrawOption_Delay,
    DrawOption_Max     = DrawOption_Immediately, /* Max */
    DrawOption_Default = DrawOption_Min,         /* Default */
    DrawOption_Cnt     = DrawOption_Max + 1,     /* The number of all */
    DrawOption_None    = DrawOption_Cnt,         /* Invalid value */
} DrawOption;

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

typedef enum {
    AlignHorizonalType_LEFT = 0,
    AlignHorizonalType_RIGHT,
    AlignHorizonalType_CENTER,
    AlignHorizonalType_Min     = AlignHorizonalType_LEFT,
    AlignHorizonalType_Max     = AlignHorizonalType_CENTER, /* Max */
    AlignHorizonalType_Default = AlignHorizonalType_Min,    /* Default */
    AlignHorizonalType_Cnt = AlignHorizonalType_Max + 1, /* The number of all */
    AlignHorizonalType_None = AlignHorizonalType_Cnt,    /* Invalid value */
} AlignHorizonalType;

typedef enum {
    AlignVerticalType_TOP = 0,
    AlignVerticalType_BOTTOM,
    AlignVerticalType_MIDDLE,
    AlignVerticalType_Min     = AlignVerticalType_TOP,
    AlignVerticalType_Max     = AlignVerticalType_MIDDLE, /* Max */
    AlignVerticalType_Default = AlignVerticalType_Min,    /* Default */
    AlignVerticalType_Cnt  = AlignVerticalType_Max + 1, /* The number of all */
    AlignVerticalType_None = AlignVerticalType_Cnt,     /* Invalid value */
} AlignVerticalType;

typedef struct AlignType
{
    uint8_t horizonal;
    uint8_t vertical;
} AlignType;

#define InitOBJParam(pObj, xPos, yPos, objWidth, objHeight, draw)              \
    do {                                                                       \
        ((Obj*)pObj)->x      = xPos;                                           \
        ((Obj*)pObj)->y      = yPos;                                           \
        ((Obj*)pObj)->width  = objWidth;                                       \
        ((Obj*)pObj)->height = objHeight;                                      \
        ((Obj*)pObj)->Draw   = &draw;                                          \
        /*((Obj*)pObj)->Response = &response;*/                                \
    } while (0)

FontName      GUI_GetFontName(void);
FontSize      GUI_GetFontSize(void);
void          GUI_SetFontName(FontName fontName);
void          GUI_SetFontSize(FontSize fontSize);
void          GUI_setForeColor(COLOR_DATTYPE color);
COLOR_DATTYPE GUI_getForeColor(void);
void          GUI_setBackColor(COLOR_DATTYPE color);
COLOR_DATTYPE GUI_getBackColor(void);
uint16_t      GUI_GetXORColor(uint16_t color);
void          GUI_DrawStr(Obj* obj, const char* str);
bool          GUI_isTarget(Obj* obj, Position* point);
bool          GUI_GetBorderFlag(Obj* obj, BorderFlagBit botderFlagBit);
void          draw_widget(Obj* obj);
void          drawBorder(Obj* obj, Border* border);
bool          checkBoundary(uint16_t x,
                            uint16_t y,
                            uint16_t width,
                            uint16_t height,
                            Obj*     obj);
void          setPublicFont(FontName      fontName,
                            FontSize      fontSize,
                            COLOR_DATTYPE fontColor);
void          setPublicBorder(COLOR_DATTYPE borderColor,
                              uint8_t       borderWidth,
                              uint8_t       borderFlag);
AlignType*    getPublicAlignType(void);
void          setPublicAlignType(AlignHorizonalType horizonal,
                                 AlignVerticalType  vertical);
Border*       getObjBorder(Obj* obj);
void          ObjSkin(Obj* obj);
void fillArea(u16 x, u16 y, u16 width, u16 height, COLOR_DATTYPE color);
bool GUI_isInArea(uint16_t  x,
                  uint16_t  y,
                  uint16_t  width,
                  uint16_t  height,
                  Position* pos);
#endif
