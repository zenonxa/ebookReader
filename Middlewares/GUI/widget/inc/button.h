#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "SYSTEM/SYS/sys.h"
#include "gui.h"

typedef struct BT_struct
{
    Obj obj;
    void (*DrawButton)(struct BT_struct*);  // 画出按键
    void (*OnClicked)(struct BT_struct*);   // 响应按下
    COLOR_DATTYPE BKcolor_unpressed;        // 未按下时的背景色
    COLOR_DATTYPE BKcolor_pressed;          // 按下时的背景色
    bool          ispressed;                // 按下1，没按下0
    Border        border;
    Font          font;
    AlignType     alignType;
    const char*  str;  // 字符串
} Button;

/* Value for Button Pressed State */
#define PRESSED 1
#define UNPRESSED 0
#define BT_PRESSED 1
#define BT_UNPRESSED 0

Button* NewButton(u16        xpos,
                  u16        ypos,
                  u16        width,
                  u16        height,
                  Font*      font,
                  Border*    border,
                  AlignType* alognType,
                  void (*OnClicked)(Button*));
void    ButtonSetStr(Button* button, const char* str);

#endif
