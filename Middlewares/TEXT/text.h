#ifndef __TEXT_H__
#define __TEXT_H__
#include "fontupd.h"
#include <stm32f1xx.h>
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

// 得到汉字的点阵码
void Get_HzMat(unsigned char* code,
               unsigned char* mat,
               FontName       fontName,
               FontSize       fontSize);
// 在指定位置显示一个汉字
void Show_Font(uint16_t x,
               uint16_t y,
               uint8_t* font,
               FontName fontName,
               FontSize fontSize,
               uint8_t  mode);
// 在指定位置显示一个字符串
uint8_t* Show_Str(uint16_t x,
                  uint16_t y,
                  uint16_t width,
                  uint16_t height,
                  uint8_t* str,
                  FontName fontName,
                  FontSize fontSize,
                  uint8_t  mode);
void     Show_Str_Mid(uint16_t x,
                      uint16_t y,
                      uint16_t width,
                      uint16_t height,
                      uint8_t* str,
                      FontName fontName,
                      FontSize fontSize,
                      uint16_t len,
                      uint8_t  mode);
uint8_t  getLineSpace(FontSize fontSize);
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
                   bool      drawOption);
#endif
