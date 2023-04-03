#include "fontupd.h"
#include "BSP/ATK_MD0700/atk_md0700.h"
#include "BSP/W25QXX/w25qxx.h"
#include "FATFS/exfuns/exfuns.h"
#include "FATFS/src/ff.h"
// #include "lcd.h"
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

// uint8_t font_flag;

// 字库存放在磁盘中的路径
// uint8_t*const GBK24_PATH="/SYSTEM/FONT/GBK24.FON";		//GBK24的存放位置
// uint8_t*const GBK16_PATH="/SYSTEM/FONT/GBK16.FON";		//GBK16的存放位置
// uint8_t*const GBK12_PATH="/SYSTEM/FONT/GBK12.FON";		//GBK12的存放位置
// uint8_t*const UNIGBK_PATH="/SYSTEM/FONT/UNIGBK.BIN";	//UNIGBK.BIN的存放位置

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

/*
 * Declaration:
 * 	Update one font library.
 *
 * Parameter:
 * 	fontName: name of the font
 * 	fontSize: size of the font
 *
 * Return:
 * 	0	: success;
 *	else: error
 */
/**
 * @description: Update the font specified by FontName and FontSize
 * @param {FontName} fontName:	Name of font
 * @param {FontSize} fontSize:	Size of font
 * @return {uint8_t} result:	0:		success
 * 								other:	fail
 */
uint8_t update_fontx(FontName fontName, FontSize fontSize)
{
#if 0
	FIL * fftemp;
	uint8_t *tempbuf;
	uint16_t bread;
	uint32_t offx = 0;
#endif
    uint8_t  res      = 0;
    uint32_t fontAddr = 0;
    char*    fontPath = mymalloc(SRAMIN, 100);
#if 0
//	char font_update_log_buf[100];
	uint8_t pos;
	fftemp = (FIL*)mymalloc(SRAMIN,sizeof(FIL));	//分配内存
	if(fftemp==NULL) {
// Test start
		ERROR_THROW("fftemp == NULL : update_fontx()");
// Test end
		res = 1;
	}
	
	tempbuf = mymalloc(SRAMIN,4096);					//分配4096个字节空间
	if(tempbuf==NULL) {
// Test start
		ERROR_THROW("tempbuf == NULL : update_fontx()");
// Test end
		res = 1;
	}
#endif
    getFontPath(fontPath, fontName, fontSize);
    res = f_open(temp_file, (const TCHAR*)fontPath, FA_READ);
    if (res != FR_OK) {
        infinite_throw("update_fontx() ==> Fail to open file corresponding to "
                       "the fontPath.");
    } else {
        f_close(temp_file);
    }
    // 	if(res == FR_OK)
    //	{
    fontAddr = getFontAddr(fontName, fontSize);
    //		if ((fontName == Font_Name_None) && (fontSize == Font_Size_None)) {
    //			strcpy(font_update_log_buf, "Update UNIGBK.bin ==> ");
    //		} else {
    //			strcpy(font_update_log_buf, "Update font ==> ");
    //		}
    //		pos = strlen (font_update_log_buf);
    res = load_file_to_flash(fontPath, fontAddr);
#if 0
		while(res == FR_OK)//死循环执行
		{
	 		res = f_read(fftemp, tempbuf, 4096, (UINT*)&bread);			//读取数据	 
			if(res!=FR_OK) {
// Test start
				while (1) {
					printf("Fail to read from file: %s\r\n", fontPath);
					delay_ms(10);
				}
// Test end
				break;									//执行错误
			}
			W25QXX_Write(tempbuf, fontAddr+offx, 4096);	//从0开始写入4096个数据
	  		offx += bread;
/* !! Maybe need a function to refactor the code ==> Start */
			if (offx%(5*4096) == 0) {
				sprintf(font_update_log_buf+pos, "%s%s%s%s%7d%s%7d", 
					FontNameStr[fontName], ":", FontSizeStr[fontSize], " || ", offx+1, "/", fftemp->obj.objsize);
				PRINTLN_MSG(font_update_log_buf);
			}
/* !! Maybe need a function to refactor the code ==> End */
			
//			fupd_prog(x,y,size,fftemp->obj.objsize,offx);	 	//进度显示
			if(bread!=4096) {
// Test start
//				while (1) {
//					printf("Finish writing file to flash: %s\r\n", fontPath);
//					delay_ms(10);
//				}
// Test end
/* !! Maybe need a function to refactor the code ==> Start */
				if (offx%(5*4096) != 0) {
					sprintf(font_update_log_buf+pos, "%s%s%s%s%7d%s%7d", 
						FontNameStr[fontName], ":", FontSizeStr[fontSize], " || ", offx+1, "/", fftemp->obj.objsize);
					PRINTLN_MSG(font_update_log_buf);
				}
/* !! Maybe need a function to refactor the code ==> End */
				break;								//读完了.
			}
// Test	start	
			printf("In infinite loop of update_fontx...\r\n");
			delay_ms(20);
// Test end
	 	}
		f_close(fftemp);
#endif
//	}
#if 0
	myfree(SRAMIN,fftemp);	//释放内存
	myfree(SRAMIN,tempbuf);	//释放内存
#endif
    myfree(SRAMIN, fontPath);
    return res;
}
// 更新字体文件,UNIGBK,GBK12,GBK16,GBK24一起更新
// x,y:提示信息的显示地址
// size:字体大小
// src:字库来源磁盘."0:",SD卡;"1:",FLASH盘,"2:",U盘.
// 提示信息字体大小
// 返回值:0,更新成功;
//		 其他,错误代码.
/*
 * Declaration:
 * 	Update font lib, also including UNIGBK.bin.
 *
 * Parameter:
 *	param1:
 * 	param2:
 * 	param3:
 *
 * Return:
 * 	0	:
 *	else	:
 */
uint8_t update_font(void)
{
    uint8_t  pname[100];
    uint8_t  res = 0;
    uint16_t i, j;
    //	uint32_t *buf;
    //	FIL *temp_file;

    res = 0xff;
#if 0
//	pname = mymalloc(SRAMIN,100);	//申请100字节内存  
//	buf = mymalloc(SRAMIN,4096);	//申请4K字节内存
//	fftemp = (FIL*)mymalloc(SRAMIN, sizeof(FIL));	//分配内存
	if(buf==NULL || pname==NULL || fftemp==NULL)
	{
		myfree(SRAMIN,fftemp);
		myfree(SRAMIN,pname);
		myfree(SRAMIN,buf);
//		ERROR_THROW("Fail to malloc for buf, pname, fftemp.");
		return 5;	//内存申请失败
	}
#endif
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
    if (res != FR_OK) {
        infinite_throw("Fail to check all font file.");
    }
    //	myfree(SRAMIN,fftemp);//释放内存

    if (res == FR_OK)  // 字库文件都存在.
    {
        log_n("%sErasing sectors...", ARROW_STRING);

        //		atk_md0700_show_string(x, y, 240, 320, "Erasing sectors... ",
        //mapping_font_size(size), ATK_MD0700_BLACK);//提示正在擦除扇区
        //		printf("Erasing flash sector: %4d/%4d\r\n", 0, FONTSECSIZE);
        Progress_Init(0, FONTSECSIZE);
        for (i = 0; i < FONTSECSIZE; i++)  // 先擦除字库区域,提高写入速度
        {
            Progress_Update(i);
            print_log(Flash_Erase_Log);
            //			fupd_prog(x+20*size/2,y,size,FONTSECSIZE,i);//进度显示
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
            //			/* Print erasing flash log */
            //			if ((i+1)%50 == 0) {
            //				printf("Erasing flash sector: %4d/%4d\r\n", i+1,
            //FONTSECSIZE);
            ////				delay_ms(10);
            //			}
        }
        /* The log of the erasing operation for the last sector */
        //		if ((FONTSECSIZE)%50 != 0) {
        //			printf("Erasing flash sector: %4d/%4d\r\n", i+1,
        //FONTSECSIZE);
        //		}
        log_n("%sErasing flash finished!", ARROW_STRING);
        //		myfree(SRAMIN, buf);
        //		buf = NULL;

        //		/* Loading mapping table [UNIGBK.BIN] to flash */
        res = update_mapping_table();
        // Test start
        if (res) {
            infinite_throw("Fail to load UNIGBK.bin to flash.");
        }
        // Test end
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
        // Test
        if (res) {
            infinite_throw("Fail to load font library[%s:%s] to flash.\r\n",
                           FontNameStr[i], FontSizeStr[j]);
        }
        if (res) {
            //			myfree(SRAMIN, pname);
            // Test	start
            infinite_throw("update_fontx() ==> pname free error\r\n");
            // Test end
            return 4;
        }
        // 全部更新好了
        /* Prepare FontHeader OK */
        fontHeader.fontok = FLAG_OK;
        fontHeader.ugbkok = FLAG_OK;

        W25QXX_Write((uint8_t*)&fontHeader, FONT_HEADER_ADDR,
                     sizeof(fontHeader));  // 保存字库信息
        log_n("Write fontHeader to flash ==> \r\n\
		fontHeader.fontok = FLAG_OK, \r\n\b\
		fontHeader.ugbkok = FLAG_OK");
    }
    //	myfree(SRAMIN,pname);//释放内存
    //	myfree(SRAMIN,buf);
    return res;  // 无错误.
}

uint8_t update_mapping_table(void)
{
    uint8_t  res = 0;
    char     fname[100];
    uint32_t fileAddr = getMappingTableAddr();
    getMappingTablePath((uint8_t*)fname);
    res = load_file_to_flash(fname, fileAddr);
    return res;
}
