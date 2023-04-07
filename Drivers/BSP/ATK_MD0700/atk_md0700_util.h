#ifndef __ATK_MD0700_CONF_H__
#define __ATK_MD0700_CONF_H__

/* 定义ATK-MD0700模块LCD尺寸 */
#define ATK_MD0700_LCD_WIDTH 480
#define ATK_MD0700_LCD_HEIGHT 800

/* 定义是否使用ATK-MD0700模块触摸 */
#define ATK_MD0700_USING_TOUCH 1

/* 定义ATK-MD0700模块启用的字体 */
#define ATK_MD0700_FONT_12 1
#define ATK_MD0700_FONT_16 1
#define ATK_MD0700_FONT_24 1
#define ATK_MD0700_FONT_32 1

#include "SYSTEM/sys/sys.h"

/* 触摸点坐标数据结构 */
typedef struct
{
    int16_t x; /* 触摸点X坐标 */
    int16_t y; /* 触摸点Y坐标 */
} Position;

#endif
