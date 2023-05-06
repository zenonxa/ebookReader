#include "widget/inc/button.h"
#include "BSP/ATK_MD0700/atk_md0700.h"
#include "MALLOC/malloc.h"
#include "color.h"
#include "gui.h"
#include "text.h"
#include <string.h>

void DrawButton(Button* button);
void DefaultDrawButton(Button* button);
void DefaultDrawButton(Button* button);
void BtDefaultSkin(Button* button);
void BtSkin(Button* button);
void Obj_SkinUnPressed(Obj* obj);
void Obj_SkinPressed(Obj* obj);
void Obj_Skin_base(Obj* obj, COLOR_DATTYPE color);
void draw_ButtonBorder(Button* button);
void BtDefaultText(Button* button);
void BtText(Button* button);
void ButtonOnClickedDefault(Button* button);

Button* NewButton(u16        xpos,
                  u16        ypos,
                  u16        width,
                  u16        height,
                  LocateType locateType,
                  Font*      font,
                  Border*    border,
                  AlignType* alognType,
                  void (*OnClicked)(Button*))
{
    Button* button = (Button*)mymalloc(GUI_MALLOC_SOURCE, sizeof(Button));

    // button->font.fontName  = GUI_FONT_TYPE_DEFAULT;
    // button->font.fontSize  = GUI_FONT_SIZE_DEFAULT;
    button->isPressed          = UNPRESSED;
    button->str                = 0;
    button->font               = *font;
    button->border             = *border;
    ((Obj*)button)->height     = height;
    ((Obj*)button)->width      = width;
    ((Obj*)button)->x          = xpos;
    ((Obj*)button)->y          = ypos;
    ((Obj*)button)->type       = Obj_Type_Button;
    ((Obj*)button)->locateType = locateType;
    button->alignType          = *alognType;
    button->DrawButton         = &DrawButton;
    if (OnClicked) {
        button->OnClicked = OnClicked;
    } else {
        button->OnClicked = &ButtonOnClickedDefault;
    }
    return button;
}

void DrawButton(Button* button)
{
    DefaultDrawButton(button);
}

void DefaultDrawButton(Button* button)
{
    BtDefaultSkin(button);
    BtDefaultText(button);
}

void BtDefaultSkin(Button* button)
{
    BtSkin(button);
}

void BtSkin(Button* button)
{
    draw_ButtonBorder(button);
    if (button->isPressed == BT_PRESSED) {
        Obj_SkinPressed((Obj*)button);
    } else {
        Obj_SkinUnPressed((Obj*)button);
    }
}

void Obj_SkinPressed(Obj* obj)
{
    Obj_Skin_base(obj, RGB888toRGB565(0xcccccc));
}

void Obj_SkinUnPressed(Obj* obj)
{
    Obj_Skin_base(obj, GUI_getBackColor());
}

void Obj_Skin_base(Obj* obj, COLOR_DATTYPE color)
{
    atk_md0700_fill(obj->x, obj->y, obj->x + obj->width - 1,
                    obj->y + obj->height - 1, &color, SINGLE_COLOR_BLOCK);
}

void draw_ButtonBorder(Button* button)
{
    drawBorder((Obj*)button);
}

void BtDefaultText(Button* button)
{
    BtText(button);
}

void BtText(Button* button)
{
    // u16 length;
    if (button->str != 0) {
        FontName fontName = GUI_GetFontName();
        FontSize fontSize = GUI_GetFontSize();
        COLOR_DATTYPE foreColor = GUI_getForeColor();
        GUI_SetFontName((FontName)button->font.fontName);
        GUI_SetFontSize((FontSize)button->font.fontSize);
        // length = strlen(button->str);
        if (button->isPressed == BT_PRESSED) {
            GUI_setForeColor(GUI_GetXORColor(button->font.fontColor));
        } else {
            GUI_setForeColor(button->font.fontColor);
        }
        // GUI_setForeColor(button->font.fontColor);
        // Show_Str_Mid(((Obj*)button)->x, ((Obj*)button)->y,
        // (uint8_t*)(button->str),
        //              GUI_GetFontType(), GUI_GetFontSize(),
        //              ((Obj*)button)->width, 1);
        setPublicAlignType((AlignHorizonalType)button->alignType.horizonal,
                           (AlignVerticalType)button->alignType.vertical);
        GUI_DrawStr(&button->obj, button->str, &publicAlignType);
        // CUIGUI_DrawStr(((Obj*)button)->x + (((Obj*)button)->width - length) /
        // 2,
        //                ((Obj*)button)->y +
        //                    (((Obj*)button)->height - button->fontSize) / 2,
        //                button->fontColor, button->str);
        GUI_SetFontName(fontName);
        GUI_SetFontSize(fontSize);
        GUI_setForeColor(foreColor);
    }
}

void ButtonSetPressed(Obj* obj, u8 isPressed)
{
    Button* bt    = (Button*)obj;
    bt->isPressed = isPressed;
    bt->DrawButton(bt);
}

void ButtonSetFont(Button* button, const FontName fontName)
{
    button->font.fontName = fontName;
    button->DrawButton(button);
}

// 设置按键颜色
void ButtonSetColor(Button* button, COLOR_DATTYPE fontColor)
{
    button->font.fontColor = fontColor;
    button->DrawButton(button);
}

void ButtonSetStr(Button* button, const char* str)
{
    button->str = str;
    button->DrawButton(button);
}

void ButtonOnClickedDefault(Button* button)
{
    ;
}
