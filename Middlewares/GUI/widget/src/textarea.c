#include "widget/inc/textarea.h"
#include "MALLOC/malloc.h"

void DrawTextareaDefault(Textarea* textarea);
void drawText(Textarea* textarea);

Textarea* NewTextarea(uint16_t      x,
                      uint16_t      y,
                      uint16_t      width,
                      uint16_t      heigh,
                      Font*         font,
                      Border*       border,
                      COLOR_DATTYPE backColor)
{
    Textarea* textarea =
        (Textarea*)mymalloc(GUI_MALLOC_SOURCE, sizeof(Textarea));
    ((Obj*)textarea)->type   = Obj_Type_Textarea;
    ((Obj*)textarea)->x      = x;
    ((Obj*)textarea)->y      = y;
    ((Obj*)textarea)->width  = width;
    ((Obj*)textarea)->height = heigh;
    textarea->backColor      = backColor;
    textarea->DrawTextarea   = &DrawTextareaDefault;
    textarea->str            = 0;
    if (font) {
        textarea->font = *font;
    }
    if (border) {
        textarea->border = *border;
    }
    return textarea;
}

void DrawTextarea(Textarea* textarea)
{
    DrawTextareaDefault(textarea);
}

void DrawTextareaDefault(Textarea* textarea)
{
    // getObjBorder((Obj*)textarea);
    drawBorder((Obj*)textarea, &textarea->border);
    drawText(textarea);
}

void TextareaSetStr(Textarea* textarea, char* str)
{
    textarea->str = str;
    textarea->DrawTextarea(textarea);
}

void drawText(Textarea* textarea)
{
    GUI_setBackColor(textarea->backColor);
    ObjSkin((Obj*)textarea);
    if (textarea->str) {
        GUI_SetFontName((FontName)textarea->font.fontName);
        GUI_SetFontSize((FontSize)textarea->font.fontSize);
        GUI_setForeColor(textarea->font.fontColor);
        GUI_DrawStr((Obj*)textarea, textarea->str);
    }
}
