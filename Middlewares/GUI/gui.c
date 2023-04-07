#include "gui.h"
#include "text.h"
#include "util.h"
#include "widget/inc/button.h"
#include "widget/inc/list.h"

uint8_t GUI_FONT_TYPE = GUI_FONT_TYPE_DEFAULT;
uint8_t GUI_FONT_SIZE = GUI_FONT_SIZE_DEFAULT;

extern u16 FOREGROUND_COLOR;  // 画笔颜色
extern u16 BACKGROUND_COLOR;  // 背景色

uint16_t* pGUI_FOREGROUND_COLOR = &FOREGROUND_COLOR;
uint16_t* pGUI_BACKGROUND_COLOR = &FOREGROUND_COLOR;

void GUI_setForeColor(COLOR_DATTYPE color)
{
    *pGUI_FOREGROUND_COLOR = color;
}

void GUI_setBackColor(COLOR_DATTYPE color)
{
    *pGUI_BACKGROUND_COLOR = color;
}

void GUI_SetFontType(FontName fontName)
{
    GUI_FONT_TYPE = fontName;
}

void GUI_SetFontSize(FontSize fontSize)
{
    GUI_FONT_SIZE = fontSize;
}

FontName GUI_GetFontType()
{
    return (FontName)GUI_FONT_TYPE;
}

FontSize GUI_GetFontSize()
{
    return (FontSize)GUI_FONT_SIZE;
}

uint16_t GUI_GetXORColor(uint16_t color)
{
    return (0xffff - color);
}

void GUI_DrawStr(Obj* obj, const char* str)
{
    uint16_t x = obj->x;
    uint16_t y = obj->y + (obj->height - getSize(GUI_GetFontSize())) / 2;
    Show_Str_Mid(x, y, (uint8_t*)str, GUI_GetFontType(), GUI_GetFontSize(),
                 obj->width, 1);
}

bool GUI_isClicked(Obj* obj, Position* point)
{
    bool res = false;
    uint16_t xMin = obj->x;
    uint16_t xMax = obj->x + obj->width - 1;
    uint16_t yMin = obj->y;
    uint16_t yMax = obj->y + obj->height - 1;
    if ((point_cur[0].x >= xMin) && (point_cur[0].x <= xMax) &&
        (point_cur[0].x >= yMin) && (point_cur[0].x <= yMax)) {
            res = true;
        }
    return res;
}
