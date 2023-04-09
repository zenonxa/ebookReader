#include "gui.h"
#include "BSP/ATK_MD0700/atk_md0700.h"
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

bool GUI_isTarget(Obj* obj, Position* point)
{
    bool     res  = false;
    uint16_t xMin = obj->x;
    uint16_t xMax = obj->x + obj->width - 1;
    uint16_t yMin = obj->y;
    uint16_t yMax = obj->y + obj->height - 1;
    if ((point_cur[0].x >= xMin) && (point_cur[0].x <= xMax) &&
        (point_cur[0].y >= yMin) && (point_cur[0].y <= yMax)) {
        res = true;
    }
    return res;
}

bool GUI_GetBorderFlag(Obj* obj, BorderFlagBit botderFlagBit)
{
    bool flag = 0;
    switch (obj->type) {
        case BUTTON: flag = ((Button*)obj)->borderFlag; break;
        case LIST: flag = ((List*)obj)->border.borderFlag; break;
        case WINDOW: break;
    }
    flag = (flag >> botderFlagBit) & 0x01;
    return flag;
}

void draw_widget(Obj* obj)
{
    switch (obj->type) {
        case BUTTON: ((Button*)obj)->DrawButton((Button*)obj); break;
        default: break;
    }
}

void drawBorder(Obj* obj, uint16_t borderWidth)
{
    uint16_t borderColor = ATK_MD0700_BLACK;
    if (GUI_GetBorderFlag(obj, BORDER_TOP)) {
        atk_md0700_fill(obj->x - borderWidth, obj->y - borderWidth,
                        obj->x + obj->width + borderWidth - 1, obj->y - 1,
                        &borderColor, SINGLE_COLOR_BLOCK);
    }
    if (GUI_GetBorderFlag(obj, BORDER_LEFT)) {
        atk_md0700_fill(obj->x - borderWidth, obj->y - borderWidth, obj->x - 1,
                        obj->y + obj->height + borderWidth - 1, &borderColor,
                        SINGLE_COLOR_BLOCK);
    }
    if (GUI_GetBorderFlag(obj, BORDER_RIGHT)) {
        atk_md0700_fill(obj->x + obj->width, obj->y - borderWidth,
                        obj->x + obj->width + borderWidth - 1,
                        obj->y + obj->height + borderWidth - 1, &borderColor,
                        SINGLE_COLOR_BLOCK);
    }
    if (GUI_GetBorderFlag(obj, BORDER_BOTTOM)) {
        atk_md0700_fill(obj->x - borderWidth, obj->y + obj->height,
                        obj->x + obj->width + borderWidth - 1,
                        obj->y + obj->height + borderWidth - 1, &borderColor,
                        SINGLE_COLOR_BLOCK);
    }
}

bool checkBoundary(uint16_t x,
                   uint16_t y,
                   uint16_t width,
                   uint16_t height,
                   Obj*     obj)
{
    if ((obj->x < x) || ((obj->x + obj->width - 1) > (x + width - 1)) ||
        (obj->y < y) || ((obj->y + obj->height - 1) > (y + height - 1))) {
        return false;
    }
    return true;
}
