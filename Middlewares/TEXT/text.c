#include "text.h"
#include "BSP/ATK_MD0700/atk_md0700.h"
#include "BSP/W25QXX/w25qxx.h"
#include "SYSTEM/sys/sys.h"
#include "SYSTEM/usart/usart.h"
#include "font.h"
#include "fontupd.h"
#include "log.h"
#include "string.h"
//////////////////////////////////////////////////////////////////////////////////
// 本程序只供学习使用，未经作者许可，不得用于其它任何用途
// ALIENTEK STM32F103开发板
// 汉字显示 驱动代码
// 正点原子@ALIENTEK
// 技术论坛:www.openedv.com
// 创建日期:2017/6/1
// 版本：V1.0
// 版权所有，盗版必究。
// Copyright(C) 广州市星翼电子科技有限公司 2014-2024
// All rights reserved
//////////////////////////////////////////////////////////////////////////////////

extern uint16_t POINT_COLOR;
extern uint16_t BACK_COLOR;

// code 字符指针开始
// 从字库中查找出字模
// code 字符串的开始地址,GBK码
// mat  数据存放地址 (size/8+((size%8)?1:0))*(size) bytes大小
// size:字体大小
void Get_HzMat(unsigned char* code,
               unsigned char* mat,
               FontName       fontName,
               FontSize       fontSize)
{
    unsigned char qh, ql;
    unsigned char i;
    unsigned long foffset;
    uint32_t      address;
    uint8_t       size;
    uint8_t       csize;
    size  = getSize(fontSize);
    csize = (size / 8 + ((size % 8) ? 1 : 0)) *
            (size);  // 得到字体一个字符对应点阵集所占的字节数
    qh = *code;
    ql = *(++code);
#if defined(USE_DZK_GBK)
    if (qh < 0x81 || ql < 0x40 || ql == 0xff || qh == 0xff)  // 非 常用汉字
    {
        for (i = 0; i < csize; i++)
            *mat++ = 0x00;  // 填充满格
        return;             // 结束访问
    }
    if (ql < 0x7f)
        ql -= 0x40;  // 注意!
    else
        ql -= 0x41;
    qh -= 0x81;
    foffset = ((unsigned long)190 * qh + ql) * csize;  // 得到字库中的字节偏移量
#elif defined(USE_DZK_GBK2312)
    foffset = (((uint32_t)qh - 0xA1) * 94 + ((uint32_t)ql - 0XA1)) * csize;
#endif
    address = getFontAddr(fontName, fontSize);
    W25QXX_Read(mat, address + foffset, csize);
}
// 显示一个指定大小的汉字
// x,y :汉字的坐标
// font:汉字GBK码
// size:字体大小
// mode:0,正常显示,1,叠加显示
void Show_Font(uint16_t x,
               uint16_t y,
               uint8_t* font,
               FontName fontName,
               FontSize fontSize,
               uint8_t  mode)
{
    uint8_t  temp, t, t1;
    uint16_t y0 = y;
    uint8_t  dzk[72];
    uint8_t  size  = getSize(fontSize);
    uint8_t  csize = (size / 8 + ((size % 8) ? 1 : 0)) *
                    (size);  // 得到字体一个字符对应点阵集所占的字节数
    if (size != 12 && size != 16 && size != 24 && size != 32)
        return;  // 不支持的size
    log_n("%sDZK for %2x %2x (size: , csize: %d)", ARROW_STRING, *font,
          *(font + 1), size, csize);
    Get_HzMat(font, dzk, fontName, fontSize);  // 得到相应大小的点阵数据
    for (t = 0; t < csize; t++) {
        temp = dzk[t];  // 得到点阵数据
        for (t1 = 0; t1 < 8; t1++) {
            log("%c ", ((temp & 0x80) == 0x80) ? '*' : ' ');
            if (temp & 0x80)
                atk_md0700_draw_point(x, y, ATK_MD0700_BLACK);
            else if (mode == 0)
                atk_md0700_draw_point(x, y, ATK_MD0700_WHITE);
            temp <<= 1;
            y++;
            if ((y - y0) == size) {
                y = y0;
                x++;
                log_n("");
                break;
            }
        }
    }
}
// 在指定位置开始显示一个字符串
// 支持自动换行
//(x,y):起始坐标
// width,height:区域
// str  :字符串
// size :字体大小
// mode:0,非叠加方式;1,叠加方式
void Show_Str(uint16_t x,
              uint16_t y,
              uint16_t width,
              uint16_t height,
              uint8_t* str,
              FontName fontName,
              FontSize fontSize,
              uint8_t  mode)
{
    uint16_t x0   = x;
    uint16_t y0   = y;
    uint8_t  bHz  = 0;  // 字符或者中文
    uint8_t  size = getSize(fontSize);
    while (*str != 0)  // 数据未结束
    {
        if (!bHz) {
            if (*str > 0x80) {
                bHz = 1;                          // 中文
            } else {                              // 字符
                if (x > (x0 + width - size / 2))  // 换行
                {
                    y += size;
                    x = x0;
                }
                if (y > (y0 + height - size))
                    break;       // 越界返回
                if (*str == 13)  // 换行符号
                {
                    y += size;
                    x = x0;
                    str++;
                } else {
                    atk_md0700_show_char(x, y, *str, size,
                                         ATK_MD0700_BLACK);  // 有效部分写入
                }
                str++;
                x += size / 2;  // 字符,为全字的一半
            }
        } else {
            // 中文
            bHz = 0;                      // 有汉字库
            if (x > (x0 + width - size))  // 换行
            {
                y += size;
                x = x0;
            }
            if (y > (y0 + height - size))
                break;  // 越界返回
            Show_Font(x, y, str, fontName, fontSize,
                      mode);  // 显示这个汉字,空心显示
            str += 2;
            x += size;  // 下一个汉字偏移
        }
    }
}
// 在指定宽度的中间显示字符串
// 如果字符长度超过了len,则用Show_Str显示
// len:指定要显示的宽度
void Show_Str_Mid(uint16_t x,
                  uint16_t y,
                  uint8_t* str,
                  FontName fontName,
                  FontSize fontSize,
                  uint8_t  len)
{
    uint16_t strlenth = 0;
    uint8_t  size     = getSize(fontSize);
    strlenth          = strlen((const char*)str);
    strlenth *= size / 2;
    if (strlenth > len) {
        Show_Str(x, y, ATK_MD0700_LCD_WIDTH, ATK_MD0700_LCD_HEIGHT, str,
                 fontName, fontSize, 1);
    } else {
        strlenth = (len - strlenth) / 2;
        Show_Str(strlenth + x, y, ATK_MD0700_LCD_WIDTH, ATK_MD0700_LCD_HEIGHT,
                 str, fontName, fontSize, 1);
    }
}
