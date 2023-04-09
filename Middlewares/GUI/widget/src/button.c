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

Button* NewButton(u16      xpos,
                  u16      ypos,
                  u16      width,
                  u16      height,
                  uint16_t fontColor,
                  uint8_t  borderWidth,
                  uint8_t  borderFlag)
{
    Button* button = (Button*)mymalloc(GUI_MALLOC_SOURCE, sizeof(Button));

    button->ispressed      = UNPRESSED;
    button->str            = 0;
    button->fontColor      = fontColor;
    button->borderWidth    = borderWidth;
    button->borderFlag     = borderFlag;
    button->fontType       = GUI_FONT_TYPE_DEFAULT;
    button->fontSize       = GUI_FONT_SIZE_DEFAULT;
    ((Obj*)button)->height = height;
    ((Obj*)button)->width  = width;
    ((Obj*)button)->x      = xpos;
    ((Obj*)button)->y      = ypos;
    ((Obj*)button)->type   = BUTTON;
    button->DrawButton     = &DrawButton;
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
    if (button->ispressed == BT_PRESSED) {
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
    Obj_Skin_base(obj, RGB888toRGB565(0xffffff));
}

void Obj_Skin_base(Obj* obj, COLOR_DATTYPE color)
{
    atk_md0700_fill(obj->x, obj->y, obj->x + obj->width - 1,
                    obj->y + obj->height - 1, &color, SINGLE_COLOR_BLOCK);
}

void draw_ButtonBorder(Button* button)
{
    drawBorder((Obj*)button, button->borderWidth);
}

void BtDefaultText(Button* button)
{
    BtText(button);
}

void BtText(Button* button)
{
    u16 length;

    if (button->str != 0) {
        GUI_SetFontType(button->fontType);
        GUI_SetFontSize(button->fontSize);
        length = strlen(button->str);
        if (button->ispressed == BT_PRESSED) {
            GUI_setForeColor(GUI_GetXORColor(button->fontColor));
        } else {
            GUI_setForeColor(button->fontColor);
        }
        // Show_Str_Mid(((Obj*)button)->x, ((Obj*)button)->y,
        // (uint8_t*)(button->str),
        //              GUI_GetFontType(), GUI_GetFontSize(),
        //              ((Obj*)button)->width, 1);
        GUI_DrawStr(&button->obj, button->str);
        // CUIGUI_DrawStr(((Obj*)button)->x + (((Obj*)button)->width - length) /
        // 2,
        //                ((Obj*)button)->y +
        //                    (((Obj*)button)->height - button->fontSize) / 2,
        //                button->fontColor, button->str);
    }
}

void ButtonSetPressed(Obj* obj, u8 isPressed)
{
    Button* bt    = (Button*)obj;
    bt->ispressed = isPressed;
    bt->DrawButton(bt);
}

void ButtonSetFont(Button* button, const FontName fontName)
{
    button->fontType = fontName;
    button->DrawButton(button);
}

// 设置按键颜色
void ButtonSetColor(Button* button, COLOR_DATTYPE fontColor)
{
    button->fontColor = fontColor;
    button->DrawButton(button);
}
