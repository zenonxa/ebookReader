#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "SYSTEM/SYS/sys.h"
#include "gui.h"

typedef struct BT_struct
{
    Obj obj;

    void (*DrawButton)(struct BT_struct*);  // 画出按键
    COLOR_DATTYPE   BKcolor_unpressed;      // 未按下时的背景色
    COLOR_DATTYPE   BKcolor_pressed;        // 按下时的背景色
    COLOR_DATTYPE   fontColor;              // 字体颜色
    const GUI_FONT* pFont;                  // 字体
    u8              ispressed;              // 按下1，没按下0
    const char*     str;                    // 字符串
} Button;

#endif
