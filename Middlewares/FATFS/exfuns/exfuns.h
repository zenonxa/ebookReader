/*
 * @Author: KimKey 13829607886@163.com
 * @Date: 2023-03-20 17:06:44
 * @LastEditors: KimKey 13829607886@163.com
 * @LastEditTime: 2023-04-01 17:29:33
 * @FilePath: \ebookReader\Middlewares\FATFS\exfuns\exfuns.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __EXFUNS_H
#define __EXFUNS_H 					   
#include "SYSTEM/sys/sys.h"
#include "FATFS/src/ff.h"
#include "log.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//FATFS 扩展代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/1/7
//版本：V1.1
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved	
//********************************************************************************
//升级说明
//V1.1
//修正exf_copy函数,文件进度显示错误的bug
////////////////////////////////////////////////////////////////////////////////// 	

#define FLASH_BUFFER_SIZE	4096
//#define MSG_CACHE_SIZE      100

extern FATFS *fs[FF_VOLUMES];  
extern FIL *main_file;
extern FIL *vice_file;
extern FIL *temp_file;
extern UINT br,bw;
extern FILINFO fileinfo;
extern DIR dir;
extern u8 *fatbuf;//SD卡数据缓存区
/* Cache for flash when do reading or writing operation */
extern u8* flash_buffer;
extern u8* dzk;
/* Cache for message to throw */
//extern u8* msg_cache;


//f_typetell返回的类型定义
//根据表FILE_TYPE_TBL获得.在exfuns.c里面定义
#define T_BIN		0X00	//bin文件
#define T_LRC		0X10	//lrc文件

#define T_NES		0X20	//nes文件
#define T_SMS		0X21	//sms文件

#define T_TEXT		0X30	//.txt文件
#define T_C			0X31	//.c文件
#define T_H			0X32    //.h文件

#define T_WAV		0X40	//WAV文件
#define T_MP3		0X41	//MP3文件 
#define T_APE		0X42	//APE文件
#define T_FLAC		0X43	//FLAC文件

#define T_BMP		0X50	//bmp文件
#define T_JPG		0X51	//jpg文件
#define T_JPEG		0X52	//jpeg文件		 
#define T_GIF		0X53	//gif文件  
 
#define T_AVI		0X60	//avi文件  

 
u8 exfuns_init(void);							//申请内存
u8 f_typetell(u8 *fname);						//识别文件类型
u8 exf_getfree(u8 *drv,u32 *total,u32 *free);	//得到磁盘总容量和剩余容量
u32 exf_fdsize(u8 *fdname);						//得到文件夹大小	
u8* exf_get_src_dname(u8* dpfn);																		   
u8 exf_copy(u8(*fcpymsg)(u8*pname,u8 pct,u8 mode),u8 *psrc,u8 *pdst,u32 totsize,u32 cpdsize,u8 fwmode);	   //文件复制
u8 exf_fdcopy(u8(*fcpymsg)(u8*pname,u8 pct,u8 mode),u8 *psrc,u8 *pdst,u32 *totsize,u32 *cpdsize,u8 fwmode);//文件夹复制
uint8_t load_file_to_flash(char* fname, uint32_t flash_addr);												// 使用FATFS将文件加载到Flash中
//void print_loading_log(char*  fileName, uint32_t offset, uint32_t fileSize);
#endif


