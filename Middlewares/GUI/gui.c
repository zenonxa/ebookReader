#include "gui.h"
#include "BSP/ATK_MD0700/atk_md0700.h"
#include "log.h"
#include "text.h"
#include "util.h"
#include "widget/inc/button.h"
#include "widget/inc/list.h"
#include "widget/inc/textarea.h"
#include <string.h>

uint8_t GUI_FONT_NAME = GUI_FONT_TYPE_DEFAULT;
uint8_t GUI_FONT_SIZE = GUI_FONT_SIZE_DEFAULT;

Font      publicFont;
Border    publicBorder;
AlignType publicAlignType;

extern u16 FOREGROUND_COLOR;  // 画笔颜色
extern u16 BACKGROUND_COLOR;  // 背景色

uint16_t* pGUI_FOREGROUND_COLOR = &FOREGROUND_COLOR;
uint16_t* pGUI_BACKGROUND_COLOR = &BACKGROUND_COLOR;

void fillArea(u16 x, u16 y, u16 width, u16 height, COLOR_DATTYPE color);

void GUI_setForeColor(COLOR_DATTYPE color)
{
    // COLOR_DATTYPE backColor = GUI_getBackColor();
    // if (color == backColor) {
    //     GUI_setBackColor(RGB565_WHITE-backColor);
    // }
    *pGUI_FOREGROUND_COLOR = color;
}

void GUI_setBackColor(COLOR_DATTYPE color)
{
    // COLOR_DATTYPE foreColor = GUI_getForeColor();
    // if (color == foreColor) {
    //     GUI_setForeColor(RGB565_WHITE-foreColor);
    // }
    *pGUI_BACKGROUND_COLOR = color;
}

COLOR_DATTYPE GUI_getBackColor(void)
{
    return *pGUI_BACKGROUND_COLOR;
}
COLOR_DATTYPE GUI_getForeColor(void)
{
    return *pGUI_FOREGROUND_COLOR;
}

void GUI_SetFontName(FontName fontName)
{
    GUI_FONT_NAME = fontName;
}

void GUI_SetFontSize(FontSize fontSize)
{
    GUI_FONT_SIZE = fontSize;
}

FontName GUI_GetFontName()
{
    return (FontName)GUI_FONT_NAME;
}

FontSize GUI_GetFontSize()
{
    return (FontSize)GUI_FONT_SIZE;
}

uint16_t GUI_GetXORColor(uint16_t color)
{
    return (0xffff - color);
}

void GUI_DrawStr(Obj* obj, const char* str, AlignType* pAlignType)
{
    uint16_t x       = obj->x;
    uint16_t y       = obj->y;
    uint16_t len     = getSize(GUI_GetFontSize()) / 2 * strlen(str);
    uint16_t lineCnt = len / obj->width + (len % obj->width ? 1 : 0);
    if (len <= obj->width) {
        switch (pAlignType->horizonal) {
            case AlignHorizonalType_LEFT: x = obj->x; break;
            case AlignHorizonalType_RIGHT: x = obj->x + obj->width - len; break;
            case AlignHorizonalType_CENTER:
                x = obj->x + (obj->width - len) / 2;
                break;
            default: break;
        }
    }
    if (lineCnt * getSize(GUI_GetFontSize()) <= obj->height) {
        switch (pAlignType->vertical) {
            case AlignVerticalType_TOP: y = obj->y; break;
            case AlignVerticalType_BOTTOM:
                y = obj->y + obj->height - lineCnt * getSize(GUI_GetFontSize());
                break;
            case AlignVerticalType_MIDDLE:
                y = obj->y +
                    (obj->height - lineCnt * getSize(GUI_GetFontSize())) / 2;
                break;
            default: break;
        }
    }

    // x = obj->x;
    // y = obj->y + (obj->height - getSize(GUI_GetFontSize())) / 2;
    // if (obj->type == Obj_Type_Textarea) {
    //     log_n("%swidth: %d, strlen(str): %d", ARROW_STRING, obj->width,
    //     strlen(str)); log_n("x: %d, y: %d", obj->x, obj->y);
    // }
    // Show_Str_Mid(x, y, obj->width, obj->height, (uint8_t*)str,
    //              GUI_GetFontName(), GUI_GetFontSize(), obj->width, 1);
    Show_Str(x, y, obj->width, obj->height, (uint8_t*)str, strlen(str),
             GUI_GetFontName(), GUI_GetFontSize(), 1, &isOverOnePage);
}

bool GUI_isTarget(Obj* obj, Position* point)
{
    // bool     res  = false;
    // uint16_t xMin = obj->x;
    // uint16_t xMax = obj->x + obj->width - 1;
    // uint16_t yMin = obj->y;
    // uint16_t yMax = obj->y + obj->height - 1;
    // if ((point_cur[0].x >= xMin) && (point_cur[0].x <= xMax) &&
    //     (point_cur[0].y >= yMin) && (point_cur[0].y <= yMax)) {
    //     res = true;
    // }
    // return res;
    return GUI_isInArea(obj->x, obj->y, obj->width, obj->height, point);
}

bool GUI_isInArea(uint16_t  x,
                  uint16_t  y,
                  uint16_t  width,
                  uint16_t  height,
                  Position* pos)
{
    bool     res  = false;
    uint16_t xMin = x;
    uint16_t xMax = x + width - 1;
    uint16_t yMin = y;
    uint16_t yMax = y + height - 1;
    if ((point_cur[0].x >= xMin) && (point_cur[0].x <= xMax) &&
        (point_cur[0].y >= yMin) && (point_cur[0].y <= yMax)) {
        res = true;
    }
    return res;
}

bool GUI_GetBorderFlag(Obj* obj, BorderFlagBit botderFlagBit)
{
    bool flag = (getObjBorder(obj)->borderFlag >> botderFlagBit) & 0x01;
    return flag;
}

// Used when drawing widgets in List
void draw_widget(Obj* obj)
{
    switch (obj->type) {
        case Obj_Type_Button: ((Button*)obj)->DrawButton((Button*)obj); break;
        case Obj_Type_Textarea:
            ((Textarea*)obj)->DrawTextarea((Textarea*)obj);
            break;
        default: break;
    }
}

void drawBorder(Obj* obj)
{
    Border* border = getBorder(obj);
    if (border != NULL) {
        drawBorder_base(obj, border, border->borderColor);
    }
}

void clearBorder(Obj* obj)
{
    Border* border = getBorder(obj);
    if (border != NULL) {
        drawBorder_base(obj, border, GUI_getBackColor());
    }
}

void drawBorder_base(Obj* obj, Border* border, COLOR_DATTYPE color)
{
    uint16_t borderColor = color;
    uint16_t borderWidth = border->borderWidth;
    int16_t  startX, startY;
    int16_t  endX, endY;
    // int16_t  width = 0, height = 0;
    if (GUI_GetBorderFlag(obj, BORDER_TOP)) {
        if (obj->y > 0) {
            startX = ((obj->x - borderWidth) >= 0) ? (obj->x - borderWidth) : 0;
            startY = ((obj->y - borderWidth) >= 0) ? (obj->y - borderWidth) : 0;
            endX =
                ((obj->x + obj->width + borderWidth) <= ATK_MD0700_LCD_WIDTH) ?
                    (obj->x + obj->width + borderWidth) :
                    (ATK_MD0700_LCD_WIDTH - 1);
            endY = obj->y - 1;
            atk_md0700_fill(startX, startY, endX, endY, &borderColor,
                            SINGLE_COLOR_BLOCK);
        }
    }
    if (GUI_GetBorderFlag(obj, BORDER_LEFT)) {
        if (obj->x > 0) {
            startX = ((obj->x - borderWidth) >= 0) ? (obj->x - borderWidth) : 0;
            startY = ((obj->y - borderWidth) >= 0) ? (obj->y - borderWidth) : 0;
            endX   = obj->x - 1;
            endY   = ((obj->y + obj->height + borderWidth) <=
                    ATK_MD0700_LCD_HEIGHT) ?
                         ((obj->y + obj->height + borderWidth)) :
                         (ATK_MD0700_LCD_HEIGHT - 1);
            atk_md0700_fill(startX, startY, endX, endY, &borderColor,
                            SINGLE_COLOR_BLOCK);
        }
    }
    if (GUI_GetBorderFlag(obj, BORDER_RIGHT)) {
        if (obj->x + obj->width <= ATK_MD0700_LCD_WIDTH) {
            startX = obj->x + obj->width;
            startY = ((obj->y - borderWidth) >= 0) ? (obj->y - borderWidth) : 0;
            endX =
                ((obj->x + obj->width + borderWidth) <= ATK_MD0700_LCD_WIDTH) ?
                    (obj->x + obj->width + borderWidth) :
                    (ATK_MD0700_LCD_WIDTH - 1);
            endY = ((obj->y + obj->height + borderWidth) <=
                    ATK_MD0700_LCD_HEIGHT) ?
                       ((obj->y + obj->height + borderWidth)) :
                       (ATK_MD0700_LCD_HEIGHT - 1);
            atk_md0700_fill(startX, startY, endX, endY, &borderColor,
                            SINGLE_COLOR_BLOCK);
            // atk_md0700_fill(obj->x + obj->width, obj->y - borderWidth,
            //                 obj->x + obj->width + borderWidth - 1,
            //                 obj->y + obj->height + borderWidth - 1,
            //                 &borderColor, SINGLE_COLOR_BLOCK);
        }
    }
    if (GUI_GetBorderFlag(obj, BORDER_BOTTOM)) {
        if (obj->y + obj->height <= ATK_MD0700_LCD_HEIGHT) {
            startX = ((obj->x - borderWidth) >= 0) ? (obj->x - borderWidth) : 0;
            startY = obj->y + obj->height;
            endX =
                ((obj->x + obj->width + borderWidth) <= ATK_MD0700_LCD_WIDTH) ?
                    (obj->x + obj->width + borderWidth) :
                    (ATK_MD0700_LCD_WIDTH - 1);
            endY = ((obj->y + obj->height + borderWidth) <=
                    ATK_MD0700_LCD_HEIGHT) ?
                       ((obj->y + obj->height + borderWidth)) :
                       (ATK_MD0700_LCD_HEIGHT - 1);
            atk_md0700_fill(startX, startY, endX, endY, &borderColor,
                            SINGLE_COLOR_BLOCK);
            // atk_md0700_fill(obj->x - borderWidth, obj->y + obj->height,
            //                 obj->x + obj->width + borderWidth - 1,
            //                 obj->y + obj->height + borderWidth - 1,
            //                 &borderColor, SINGLE_COLOR_BLOCK);
        }
    }
}

Border* getBorder(Obj* obj)
{
    Border* border = NULL;
    switch (obj->type) {
        case Obj_Type_Button: border = &((Button*)obj)->border; break;
        case Obj_Type_List: border = &((List*)obj)->border; break;
        case Obj_Type_Textarea: border = &((Textarea*)obj)->border; break;
        default: break;
    }
    return border;
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

void setPublicFont(FontName      fontName,
                   FontSize      fontSize,
                   COLOR_DATTYPE fontColor)
{
    publicFont.fontName  = fontName;
    publicFont.fontSize  = fontSize;
    publicFont.fontColor = fontColor;
}

void setPublicBorder(COLOR_DATTYPE borderColor,
                     uint8_t       borderWidth,
                     uint8_t       borderFlag)
{
    publicBorder.borderColor = borderColor;
    publicBorder.borderWidth = borderWidth;
    publicBorder.borderFlag  = borderFlag;
}

void setPublicAlignType(AlignHorizonalType horizonal,
                        AlignVerticalType  vertical)
{
    publicAlignType.horizonal = horizonal;
    publicAlignType.vertical  = vertical;
}

AlignType* getPublicAlignType(void)
{
    return &publicAlignType;
}

Border* getObjBorder(Obj* obj)
{
    Border* border = NULL;
    switch (obj->type) {
        case Obj_Type_Button: border = &((Button*)obj)->border; break;
        case Obj_Type_List: border = &((List*)obj)->border; break;
        case Obj_Type_Textarea: border = &((Textarea*)obj)->border; break;
        default: break;
    }
    return border;
}

void ObjSkin(Obj* obj)
{
    fillArea(obj->x, obj->y, obj->width, obj->height, GUI_getBackColor());
}

void fillArea(u16 x, u16 y, u16 width, u16 height, COLOR_DATTYPE color)
{
    atk_md0700_fill(x, y, x + width - 1, y + height - 1, &color,
                    SINGLE_COLOR_BLOCK);
}

void drawSelectBox(Obj*          obj,
                   uint16_t      boxBorderWidth,
                   uint16_t      space,
                   COLOR_DATTYPE boxBorderColor)
{
    drawSelectBox_base(obj, boxBorderWidth, space, boxBorderColor);
}

void clearSelectBox(Obj* obj, uint16_t boxBorderWidth, uint16_t space)
{
    drawSelectBox_base(obj, boxBorderWidth, space, GUI_getBackColor());
}

void drawSelectBox_base(Obj*          obj,
                        uint16_t      boxBorderWidth,
                        uint16_t      space,
                        COLOR_DATTYPE boxBorderColor)
{
    int16_t  startX = 0, endX = 0;
    int16_t  startY = 0, endY = 0;
    int16_t  x = obj->x, y = obj->y;
    uint16_t width  = obj->width;
    uint16_t height = obj->height;
    if (GUI_GetBorderFlag(obj, BORDER_TOP) ||
        GUI_GetBorderFlag(obj, BORDER_LEFT) ||
        GUI_GetBorderFlag(obj, BORDER_RIGHT) ||
        GUI_GetBorderFlag(obj, BORDER_BOTTOM)) {
        uint16_t borderWidth = getBorder(obj)->borderWidth;
        x -= borderWidth;
        y -= borderWidth;
        width += borderWidth * 2;
        height += borderWidth * 2;
    }
    x -= space;
    y -= space;
    width += space * 2;
    height += space * 2;
    if (y > 0) {
        startX = ((x - boxBorderWidth) >= 0) ? (x - boxBorderWidth) : 0;
        startY = ((y - boxBorderWidth) >= 0) ? (y - boxBorderWidth) : 0;
        endX   = ((x + width + boxBorderWidth) <= ATK_MD0700_LCD_WIDTH) ?
                     (x + width + boxBorderWidth) :
                     (ATK_MD0700_LCD_WIDTH - 1);
        endY   = y - 1;
        atk_md0700_fill(startX, startY, endX, endY, &boxBorderColor,
                        SINGLE_COLOR_BLOCK);
    }
    if (x > 0) {
        startX = ((x - boxBorderWidth) >= 0) ? (x - boxBorderWidth) : 0;
        startY = ((y - boxBorderWidth) >= 0) ? (y - boxBorderWidth) : 0;
        endX   = x - 1;
        endY   = ((y + height + boxBorderWidth) <= ATK_MD0700_LCD_HEIGHT) ?
                     ((y + height + boxBorderWidth)) :
                     (ATK_MD0700_LCD_HEIGHT - 1);
        atk_md0700_fill(startX, startY, endX, endY, &boxBorderColor,
                        SINGLE_COLOR_BLOCK);
    }

    if (x + width <= ATK_MD0700_LCD_WIDTH) {
        startX = x + width;
        startY = ((y - boxBorderWidth) >= 0) ? (y - boxBorderWidth) : 0;
        endX   = ((x + width + boxBorderWidth) <= ATK_MD0700_LCD_WIDTH) ?
                     (x + width + boxBorderWidth) :
                     (ATK_MD0700_LCD_WIDTH - 1);
        endY   = ((y + height + boxBorderWidth) <= ATK_MD0700_LCD_HEIGHT) ?
                     ((y + height + boxBorderWidth)) :
                     (ATK_MD0700_LCD_HEIGHT - 1);
        atk_md0700_fill(startX, startY, endX, endY, &boxBorderColor,
                        SINGLE_COLOR_BLOCK);
    }
    if (y + height <= ATK_MD0700_LCD_HEIGHT) {
        startX = ((x - boxBorderWidth) >= 0) ? (x - boxBorderWidth) : 0;
        startY = y + height;
        endX   = ((x + width + boxBorderWidth) <= ATK_MD0700_LCD_WIDTH) ?
                     (x + width + boxBorderWidth) :
                     (ATK_MD0700_LCD_WIDTH - 1);
        endY   = ((y + height + boxBorderWidth) <= ATK_MD0700_LCD_HEIGHT) ?
                     ((y + height + boxBorderWidth)) :
                     (ATK_MD0700_LCD_HEIGHT - 1);
        atk_md0700_fill(startX, startY, endX, endY, &boxBorderColor,
                        SINGLE_COLOR_BLOCK);
    }
}