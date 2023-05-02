#include "widget/inc/textarea.h"
#include "MALLOC/malloc.h"
#include "color.h"

void DrawTextareaDefault(Textarea* textarea);
void drawText(Textarea* textarea);
void TextareaOnClickedDefault(void);

Textarea* NewTextarea(uint16_t      x,
                      uint16_t      y,
                      uint16_t      width,
                      uint16_t      heigh,
                      LocateType    locateType,
                      AlignType*    alignType,
                      Font*         font,
                      Border*       border,
                      COLOR_DATTYPE backColor,
                      void (*OnClicked)(struct Textarea*))
{
    Textarea* textarea =
        (Textarea*)mymalloc(GUI_MALLOC_SOURCE, sizeof(Textarea));
    ((Obj*)textarea)->type       = Obj_Type_Textarea;
    ((Obj*)textarea)->x          = x;
    ((Obj*)textarea)->y          = y;
    ((Obj*)textarea)->width      = width;
    ((Obj*)textarea)->height     = heigh;
    ((Obj*)textarea)->locateType = locateType;
    textarea->alignType          = *alignType;
    textarea->backColor          = backColor;
    textarea->DrawTextarea       = &DrawTextareaDefault;
    textarea->str                = 0;
    if (font) {
        textarea->font = *font;
    } else {
        textarea->font.fontName  = Font_SimSun;
        textarea->font.fontSize  = PX16;
        textarea->font.fontColor = RGB888toRGB565(0x000000);
    }
    if (border) {
        textarea->border = *border;
    } else {
        textarea->border.borderColor = RGB888toRGB565(0x000000);
        textarea->border.borderWidth = 3;
        textarea->border.borderFlag  = BORDER_FLAG(BORDER_NULL);
    }
    textarea->OnClicked =
        (OnClicked != NULL) ? OnClicked : &TextareaOnClickedDefault;
    return textarea;
}

void DrawTextarea(Textarea* textarea)
{
    DrawTextareaDefault(textarea);
}

void DrawTextareaDefault(Textarea* textarea)
{
    // getObjBorder((Obj*)textarea);
    drawBorder((Obj*)textarea);
    drawText(textarea);
}

void TextareaSetStr(Textarea* textarea, char* str)
{
    textarea->str = str;
    textarea->DrawTextarea(textarea);
}

void drawText(Textarea* textarea)
{
    COLOR_DATTYPE foreColor = GUI_getBackColor();
    GUI_setBackColor(textarea->backColor);
    ObjSkin((Obj*)textarea);
    if (textarea->str) {
        /* 暂存现有字体属性 */
        FontName fontName = GUI_GetFontName();
        FontSize fontSize = GUI_GetFontSize();
        /* 导入组件字体属性 */
        GUI_SetFontName((FontName)textarea->font.fontName);
        GUI_SetFontSize((FontSize)textarea->font.fontSize);
        /* 渲染字符串 */
        GUI_DrawStr((Obj*)textarea, textarea->str, &textarea->alignType);
        /* 恢复字体属性 */
        GUI_SetFontName(fontName);
        GUI_SetFontSize(fontSize);
    }
    GUI_setBackColor(foreColor);
}

void TextareaOnClickedDefault(void)
{
    ;
}
