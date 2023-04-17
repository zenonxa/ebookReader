#include "text.h"
#include "BSP/ATK_MD0700/atk_md0700.h"
#include "BSP/W25QXX/w25qxx.h"
#include "SYSTEM/sys/sys.h"
#include "SYSTEM/usart/usart.h"
#include "exfuns/exfuns.h"
#include "font.h"
#include "fontupd.h"
#include "gui.h"
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

// extern uint16_t FOREGROUND_COLOR;
// extern uint16_t BACKGROUND_COLOR;

uint8_t getLineSpace(FontSize fontSize);
void    moveCursorToNextLine(uint16_t* pX,
                             uint16_t* pY,
                             uint8_t   size,
                             uint8_t   lineSpace,
                             uint16_t  lineStartX,
                             uint16_t* pLineSpaceY);

void fillLineSpace(uint16_t lineSpaceStartX,
                   uint16_t lineSpaceStartY,
                   uint16_t textAreaWidth,
                   uint16_t lineSpace,
                   uint16_t backgroundColor);

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
    uint16_t y0   = y;
    uint8_t  size = getSize(fontSize);
    // uint8_t dzk[MAX_DZK_SIZE];

    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) *
                    (size);  // 得到字体一个字符对应点阵集所占的字节数
    if (size != 12 && size != 16 && size != 24 && size != 32)
        return;  // 不支持的size
#if TEXT_RENDER_LOG_ENABLE
    log_n("%sDZK for %2x %2x (size: %d, csize: %d)", ARROW_STRING, *font,
          *(font + 1), size, csize);
#endif
    Get_HzMat(font, dzk, fontName, fontSize);  // 得到相应大小的点阵数据
    for (t = 0; t < csize; t++) {
        temp = dzk[t];  // 得到点阵数据
        for (t1 = 0; t1 < 8; t1++) {
#if TEXT_RENDER_LOG_ENABLE
            log("%c ", ((temp & 0x80) == 0x80) ? '*' : ' ');
#endif
            if (temp & 0x80)
                atk_md0700_draw_point(x, y, FOREGROUND_COLOR);
            else if (mode == 0)
                atk_md0700_draw_point(x, y, BACKGROUND_COLOR);
            temp <<= 1;
            y++;
            if ((y - y0) == size) {
                y = y0;
                x++;
#if TEXT_RENDER_LOG_ENABLE
                log_n("");
#endif
                break;
            }
        }
    }
}

/**
 * @description: move the text rendering cursor to the next line
 * @param {uint16_t} pX: pointer to the X pos of the cursor
 * @param {uint16_t} pY: pointer to the Y pos of the cursor
 * @param {uint8_t} size: size of the font
 * @param {uint8_t} lineSpace: the linespace size between two lines
 * @param {uint16_t} lineStartX: the X pos of the start position of the textarea
 * @param {uint16_t} pLineSpaceY: to mark down the Y pos of the linespace start
 * position
 * @return {void}
 */
void moveCursorToNextLine(uint16_t* pX,
                          uint16_t* pY,
                          uint8_t   size,
                          uint8_t   lineSpace,
                          uint16_t  lineStartX,
                          uint16_t* pLineSpaceY)
{
    *pX = lineStartX;
    *pY += size;
    *pLineSpaceY = *pY;
    *pY += lineSpace;
}

/**
 * @description: Fill the space with background color.
 * @param {uint16_t} lineSpaceStartX
 * @param {uint16} lineSpaceStartY
 * @param {uint16_t} textAreaWidth
 * @param {uint16_t} lineSpace
 * @param {uint16_t} backgroundColor
 * @return {*}
 */
void fillLineSpace(uint16_t lineSpaceStartX,
                   uint16_t lineSpaceStartY,
                   uint16_t textAreaWidth,
                   uint16_t lineSpace,
                   uint16_t backgroundColor)
{
    atk_md0700_fill(
        lineSpaceStartX, lineSpaceStartY, lineSpaceStartX + textAreaWidth,
        lineSpaceStartY + lineSpace, &backgroundColor, SINGLE_COLOR_BLOCK);
}

/**
 * @description: 在指定区域内以左对齐的格式显示字符串，支持自动换行
 * @param {uint16_t  } x 起点X坐标
 * @param {uint16_t  } y 起点Y坐标
 * @param {uint16_t  } width 区域宽度
 * @param {uint16_t  } height 区域高度
 * @param {uint8_t*  } str 字符串
 * @param {FontName  } fontName 字体
 * @param {FontSize  } fontSize 字号
 * @param {uint8_t   } mode 叠加模式（0：非叠加，1：叠加）
 * @return {uint8_t*} 返回下一个被绘制的字符串的起始地址
 */
uint8_t* Show_Str(uint16_t x,
                  uint16_t y,
                  uint16_t width,
                  uint16_t height,
                  uint8_t* str,
                  FontName fontName,
                  FontSize fontSize,
                  uint8_t  mode)
{
    uint16_t x0        = x;
    uint16_t y0        = y;
    uint8_t  bHz       = 0; /* 0: ASCII, 1: non-ASCII */
    uint8_t  size      = getSize(fontSize);
    uint8_t  lineSpace = getLineSpace(fontSize);
    uint16_t ls_y; /* line space start position */
    /* Do while block when the character is not '\0', and there is space left on
     * the screen for unrendered text. */
    return renderString(x0, y0, width, height, &x, &y, str, 4096, fontName,
                        fontSize, mode, 1);
#if 0
    while (*str != 0) {
        if (!bHz) {            /* 处理第一个字符 */
            if (*str & 0x80) { /* 第一个字符是汉字？ */
                bHz = 1;
            } else { /* 第一个字符不是汉字 */
                /* 若剩余空间已无法再绘制一个ASCII字符，则换行 */
                if (x > (x0 + width - size / 2)) {
                    /* 换行操作包括：X重置、Y增加行高、Y增加行距 */
                    moveCursorToNextLine(&x, &y, size, lineSpace, x0, &ls_y);
                }
                /* 若剩余高度不足以再再渲染一行文字，则结束渲染 */
                if (y > (y0 + height - size)) {
                    break;  // 越界返回
                }
                /* 绘制ASCII字符。若遇到换行符号，则改为另起一行，并将指针后后移
                 */
                if (*str == 13) {
                    moveCursorToNextLine(&x, &y, size, lineSpace, x0, &ls_y);
                    str++;
                } else {
                    atk_md0700_show_char(x, y, *(char*)str,
                                         (atk_md0700_lcd_font_t)size,
                                         FOREGROUND_COLOR);  // 有效部分写入
                    str++;
                    x += size / 2;  // 字符,为全字的一半
                }
            }
        } else {     /* 处理中文字符 */
            bHz = 0; /* 清除汉字标志 */
            /* 若本行剩余空间无法再绘制汉字，则换行 */
            if (x > (x0 + width - size)) {
                /* The space left in this line is not enough */
                moveCursorToNextLine(&x, &y, size, lineSpace, x0, &ls_y);
            }
            /* 若剩余高度无法再绘制汉字，则结束绘制 */
            if (y > (y0 + height - size)) {
                break;  // 越界返回
            }
            Show_Font(x, y, str, fontName, fontSize,
                      mode);  // 显示这个汉字,空心显示
            str += 2;
            x += size;  // 下一个汉字偏移
        }
        // fillLineSpace(x0, ls_y, width, lineSpace, BACKGROUND_COLOR);
    }
    return str;
#endif
}

char* renderString(uint16_t  startX,
                   uint16_t  startY,
                   uint16_t  areaWidth,
                   uint16_t  areaHeight,
                   uint16_t* curX,
                   uint16_t* curY,
                   char*     str,
                   uint16_t  limit,
                   FontName  fontName,
                   FontSize  fontSize,
                   uint8_t   mode,
                   bool      drawOption)
{
    static int runCnt = 0;
    log_n("=================================> Time: %d", runCnt++);
    uint8_t bHz       = 0; /* 0: ASCII字符, 1: 汉字字符 */
    uint8_t size      = getSize(fontSize);
    uint8_t lineSpace = getLineSpace(fontSize);
    /* Do while block when the character is not '\0', and there is space left on
     * the screen for unrendered text. */
    char* strOrigin = str;
    // check_value_not_equal(*str, 0, "%s:%s() ==> (*str == 0)", __FILE__,
    //                       __FUNCTION__);
    log_n("%sstartX: %d", ARROW_STRING, startX);
    log_n("%sstartY: %d", ARROW_STRING, startY);
    log_n("%scurX: %d", ARROW_STRING, *curX);
    log_n("%scurY: %d", ARROW_STRING, *curY);
    log_n("%sareaWidth: %d", ARROW_STRING, areaWidth);
    log_n("%sareaHeight: %d", ARROW_STRING, areaHeight);
    while (*str != 0) {
        /* 读取足够limit字符则退出，由于ASCII和汉字大小不等，可能会差距1个字节
         */
        if ((str - strOrigin) >= limit) {
            log_n("%sLength limit. Render page finished.", ARROW_STRING);
            break;
        }
        if (!bHz) {            /* 处理第一个字符 */
            if (*str & 0x80) { /* 第一个字符是汉字？ */
                bHz = 1;
            } else { /* 第一个字符不是汉字 */
                /* 若剩余空间已无法再绘制一个ASCII字符，则换行 */
                if (*curX > (startX + areaWidth - size / 2)) {
                    /* 换行操作包括：X重置、Y增加行高、Y增加行距 */
                    *curX = startX;
                    *curY += size;
                    *curY += lineSpace;
                }
                /* 若剩余高度不足以再再渲染一行文字，则结束渲染 */
                if (*curY > (startY + areaHeight - size)) {
                    log_n("%sRender page finished.", ARROW_STRING);
                    break;  // 越界返回
                }
                /* 绘制ASCII字符。若遇到换行符号，则改为另起一行，并将指针后后移
                 */
                if (*str == 13) {
                    *curX = startX;
                    *curY += size;
                    *curY += lineSpace;
                    str++;
                } else {
                    if (drawOption) {
                        atk_md0700_show_char(*curX, *curY, *(char*)str,
                                             (atk_md0700_lcd_font_t)size,
                                             FOREGROUND_COLOR);  // 有效部分写入
                    }
                }
                str++;
                *curX += size / 2;  // 字符,为全字的一半
            }
        } else {     /* 处理中文字符 */
            bHz = 0; /* 清除汉字标志 */
            /* 若本行剩余空间无法再绘制汉字，则换行 */
            if (*curX > (startX + areaWidth - size)) {
                /* The space left in this line is not enough */
                *curX = startX;
                *curY += size;
                *curY += lineSpace;
            }
            /* 若剩余高度无法再绘制汉字，则结束绘制 */
            if (*curY > (startY + areaHeight - size)) {
                log_n("%sRender page finished.", ARROW_STRING);
                break;  // 越界返回
            }
            if (drawOption) {
                Show_Font(*curX, *curY, str, fontName, fontSize,
                          mode);  // 显示这个汉字,空心显示
            }
            str += 2;
            *curX += size;  // 下一个汉字偏移
        }
    }
    log_n("cur_x: %d, cur_y: %d", *curX, *curY);
    return str;
}
// 在指定宽度的中间显示字符串
// 如果字符长度超过了len,则用Show_Str显示
// len:指定要显示的宽度
// mode: 0:非叠加 1:叠加
void Show_Str_Mid(uint16_t x,
                  uint16_t y,
                  uint16_t width,
                  uint16_t height,
                  uint8_t* str,
                  FontName fontName,
                  FontSize fontSize,
                  uint16_t len,
                  uint8_t  mode)
{
    uint16_t strlenth = 0;
    uint8_t  size     = getSize(fontSize);
    strlenth          = strlen((const char*)str);
    strlenth *= size / 2;
    if (strlenth > len) {
        Show_Str(x, y, width, height, str, fontName, fontSize, mode);
        log_n("%s not middle", ARROW_STRING);
    } else {
        log_n("%s middle", ARROW_STRING);
        strlenth = (len - strlenth) / 2;
        log_n("strlenth = %d, strlenth+x: %d, y: %d", strlenth, strlenth + x,
              y);
        Show_Str(strlenth + x, y, width, height, str, fontName, fontSize, mode);
    }
}

/**
 * @description: Get line space for text rendering with the given font size.
 * @param {FontSize} fontSize: the size of the font
 * @return {uint8_t} line space for text rendering.
 */
uint8_t getLineSpace(FontSize fontSize)
{
    uint8_t lineSpace = getSize(fontSize);
#if 0
    switch (fontSize) {
        case PX12: lineSpace = lineSpace * 12 / 10; break; /* linespace: 14*/
        case PX16: lineSpace = lineSpace * 15 / 10; break; /* linespace: 14*/
        case PX24: lineSpace = lineSpace * 15 / 10; break; /* linespace: 14*/
        case PX32: lineSpace = lineSpace * 20 / 10; break; /* linespace: 14*/
        default:
            log_n("Cannot get line space with the given font size.");
            break;
    }
#endif
    return lineSpace;
}
