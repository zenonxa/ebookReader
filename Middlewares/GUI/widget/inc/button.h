#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "SYSTEM/SYS/sys.h"
#include "gui.h"

typedef struct BT_struct
{
    Obj obj;
    void (*DrawButton)(struct BT_struct*);  // 画出按键
    COLOR_DATTYPE BKcolor_unpressed;        // 未按下时的背景色
    COLOR_DATTYPE BKcolor_pressed;          // 按下时的背景色
    COLOR_DATTYPE fontColor;                // 字体颜色
    uint8_t       fontType;
    uint8_t       fontSize;
    bool          ispressed;  // 按下1，没按下0
    uint8_t       borderWidth;
    uint8_t       borderFlag;
    const char*   str;  // 字符串
} Button;

/* Value for Button Pressed State */
#define PRESSED 1
#define UNPRESSED 0
#define BT_PRESSED 1
#define BT_UNPRESSED 0

Button* NewButton(u16      xpos,
                  u16      ypos,
                  u16      width,
                  u16      height,
                  uint16_t fontColor,
                  uint8_t  borderWidth, uint8_t borderFlag);

#endif
