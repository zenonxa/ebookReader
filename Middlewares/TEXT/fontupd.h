/*
 * @Author: KimKey 13829607886@163.com
 * @Date: 2023-03-18 15:07:58
 * @LastEditors: KimKey 13829607886@163.com
 * @LastEditTime: 2023-03-31 19:05:13
 * @FilePath: \ebookReader\Middlewares\TEXT\fontupd.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __FONTUPD_H__
#define __FONTUPD_H__	 
#include "SYSTEM/sys/sys.h"
#include "font.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F103开发板
//字库更新 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2017/6/1
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////


//字体信息保存地址,占33个字节,第1个字节用于标记字库是否存在.后续每8个字节一组,分别保存起始地址和文件大小														   
//extern uint32_t FONTINFOADDR;
//字库信息结构体定义
//用来保存字库基本信息，地址，大小等

#if 0
/* Information of one font item, including address and size in flash */
typedef struct {
	uint32_t addr;	/* address in flash of the font library */
} FontItem;
#endif

extern FontHeader fontHeader;	//字库信息结构体

uint32_t fupd_prog(uint16_t x,uint16_t y,uint8_t size,uint32_t fsize,uint32_t pos);	//显示更新进度
uint8_t update_fontx(FontName fontName, FontSize fontSize);						//更新指定字库
uint8_t update_font(void);														//更新全部字库
uint8_t update_mapping_table(void);

#endif
