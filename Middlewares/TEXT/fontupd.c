#include "fontupd.h"
#include "BSP/ATK_MD0700/atk_md0700.h"
#include "BSP/W25QXX/w25qxx.h"
#include "FATFS/exfuns/exfuns.h"
#include "FATFS/src/ff.h"
#include "MALLOC/malloc.h"
#include "SYSTEM/delay/delay.h"
#include "SYSTEM/usart/usart.h"
#include "font.h"
#include "log.h"
#include "string.h"
//////////////////////////////////////////////////////////////////////////////////
// 本程序只供学习使用，未经作者许可，不得用于其它任何用途
// ALIENTEK STM32F103开发板
// 字库更新 驱动代码
// 正点原子@ALIENTEK
// 技术论坛:www.openedv.com
// 创建日期:2017/6/1
// 版本：V1.0
// 版权所有，盗版必究。
// Copyright(C) 广州市星翼电子科技有限公司 2014-2024
// All rights reserved
//////////////////////////////////////////////////////////////////////////////////

// 字库区域占用的总扇区数大小(3个字库+unigbk表+字库信息=3238700字节,约占791个W25QXX扇区)
/* One sector = 4 KB*/
// #define FONTSECSIZE	 	791
// 字库存放起始地址
// #define FONTINFOADDR 	1024*1024*12 					//Explorer
// STM32F4是从12M地址以后开始存放字库 前面12M被fatfs占用了.
// 12M以后紧跟3个字库+UNIGBK.BIN,总大小3.09M,被字库占用了,不能动!
// 15.10M以后,用户可以自由使用.建议用最后的100K字节比较好.

// 用来保存字库基本信息，地址，大小等
FontHeader fontHeader;

// 显示当前字体更新进度
// x,y:坐标
// size:字体大小
// fsize:整个文件大小
// pos:当前文件指针位置
uint32_t
fupd_prog(uint16_t x, uint16_t y, uint8_t size, uint32_t fsize, uint32_t pos)
{
    float   prog;
    uint8_t t = 0XFF;
    prog      = (float)pos / fsize;
    prog *= 100;
    if (t != prog) {
        atk_md0700_show_string(x + 3 * size / 2, y, 240, 320, "%",
                               mapping_font_size(size), ATK_MD0700_BLACK);
        t = prog;
        if (t > 100)
            t = 100;
        atk_md0700_show_num(x, y, t, 3, mapping_font_size(size),
                            ATK_MD0700_BLACK);  // 显示数值
    }
    return 0;
}

/**
 * @description: Update the font specified by FontName and FontSize
 * @param {FontName} fontName:	Name of font
 * @param {FontSize} fontSize:	Size of font
 * @return {uint8_t} result:	0:		success
 * 								other:	fail
 */
uint8_t update_fontx(FontName fontName, FontSize fontSize)
{
    uint8_t  res      = 0;
    uint32_t fontAddr = 0;
    char*    fontPath = mymalloc(SRAMIN, 100);

    getFontPath(fontPath, fontName, fontSize);
    res = f_open(temp_file, (const TCHAR*)fontPath, FA_READ);
    check_value_equal(res, FR_OK,
                      "update_fontx() ==> Fail to open file corresponding to "
                      "the fontPath.");
    f_close(temp_file);

    fontAddr = getFontAddr(fontName, fontSize);
    res      = load_file_to_flash(fontPath, fontAddr);

    myfree(SRAMIN, fontPath);
    return res;
}

/**
 * @description: Update Unicode-GBK mapping table and all font library
 * @return {uint8_t} 0: success  1: fail
 */
uint8_t update_font(void)
{
    uint8_t  pname[100];
    uint8_t  res = 0;
    uint16_t i, j;

    res = 0xff;

    /* Check all the font file */
    for (i = Font_Name_Min; i <= Font_Name_Max; ++i) {
        for (j = Font_Size_Min; j <= Font_Size_Max; ++j) {
            getFontPath((char*)pname, (FontName)i, (FontSize)j);
            res = f_open(temp_file, (TCHAR*)pname, FA_READ);
            if (res != FR_OK) {
                break;
            }
            f_close(temp_file);
        }
    }
    check_value_equal(res, FR_OK, "Fail to check all font file.");
    if (res == FR_OK)  // 字库文件都存在.
    {
        log_n("%sErasing sectors...", ARROW_STRING);
        Progress_Init(0, FONTSECSIZE);
        for (i = 0; i < FONTSECSIZE; i++)  // 先擦除字库区域,提高写入速度
        {
            Progress_Update(i);
            print_log(Flash_Erase_Log);
            W25QXX_Read((uint8_t*)flash_buffer,
                        ((FONT_OCCUPY_ALL / 4096) + i) * 4096,
                        FLASH_BUFFER_SIZE);  // 读出整个扇区的内容
            for (j = 0; j < 1024; j++)       // 校验数据
            {
                if (*((uint32_t*)flash_buffer + j) != 0XFFFFFFFF) {
                    break;  // 需要擦除
                }
            }
            if (j != 1024) {
                W25QXX_Erase_Sector((FONT_OCCUPY_ALL / 4096) +
                                    i);  // 需要擦除的扇区
            }
        }
        log_n("%sErasing flash finished!", ARROW_STRING);

        /* Loading mapping table [UNIGBK.BIN] to flash */
        res = update_mapping_table();
        check_value_equal(res, FR_OK, "Fail to load UNIGBK.bin to flash.");
        if (res == 0) {
            /* Loading font library with all kinds of size to flash */
            for (i = Font_Name_Min; i <= Font_Name_Max; ++i) {
                for (j = Font_Size_Min; j <= Font_Size_Max; ++j) {
                    res = update_fontx((FontName)i, (FontSize)j);
                    if (res) {
                        break;
                    }
                }
            }
        }
        check_value_equal(res, FR_OK,
                          "Fail to load font library[%s:%s] to flash.\r\n",
                          FontNameStr[i], FontSizeStr[j]);
        if (res) {
            infinite_throw("update_fontx() ==> pname free error\r\n");
            return 4;
        }
        /* Succeed to update font. Prepare FontHeader OK */
        fontHeader.fontok = FLAG_OK;
        fontHeader.ugbkok = FLAG_OK;

        W25QXX_Write((uint8_t*)&fontHeader, FONT_HEADER_ADDR,
                     sizeof(fontHeader));  // 保存字库信息
        log_n("Write fontHeader to flash ==> \r\n\
		fontHeader.fontok = FLAG_OK, \r\n\b\
		fontHeader.ugbkok = FLAG_OK");
    }
    return res; /* Success */
}

/**
 * @description: Update Unicode-GBK mapping table.
 * @return {uint8_t} 0: success  1: fail
 */
uint8_t update_mapping_table(void)
{
    uint8_t  res = 0;
    char     fname[100];
    uint32_t fileAddr = getMappingTableAddr();
    getMappingTablePath((uint8_t*)fname);
    res = load_file_to_flash(fname, fileAddr);
    return res;
}
