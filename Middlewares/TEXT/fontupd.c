#include "fontupd.h"
#include "FATFS/src/ff.h"
#include "BSP/W25QXX/w25qxx.h"
#include "BSP/ATK_MD0700/atk_md0700.h"
//#include "lcd.h"
#include "string.h"
#include "malloc.h"
#include "SYSTEM/delay/delay.h"
#include "SYSTEM/usart/usart.h"
#include "font.h"
#include "main.h"
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

//字库区域占用的总扇区数大小(3个字库+unigbk表+字库信息=3238700字节,约占791个W25QXX扇区)
/* One sector = 4 KB*/
//#define FONTSECSIZE	 	791
//字库存放起始地址
//#define FONTINFOADDR 	1024*1024*12 					//Explorer STM32F4是从12M地址以后开始存放字库
														//前面12M被fatfs占用了.
														//12M以后紧跟3个字库+UNIGBK.BIN,总大小3.09M,被字库占用了,不能动!
														//15.10M以后,用户可以自由使用.建议用最后的100K字节比较好.
														
//用来保存字库基本信息，地址，大小等
FontHeader fontHeader;

//uint8_t font_flag;

//字库存放在磁盘中的路径
//uint8_t*const GBK24_PATH="/SYSTEM/FONT/GBK24.FON";		//GBK24的存放位置
//uint8_t*const GBK16_PATH="/SYSTEM/FONT/GBK16.FON";		//GBK16的存放位置
//uint8_t*const GBK12_PATH="/SYSTEM/FONT/GBK12.FON";		//GBK12的存放位置
//uint8_t*const UNIGBK_PATH="/SYSTEM/FONT/UNIGBK.BIN";	//UNIGBK.BIN的存放位置

//显示当前字体更新进度
//x,y:坐标
//size:字体大小
//fsize:整个文件大小
//pos:当前文件指针位置
uint32_t fupd_prog(uint16_t x,uint16_t y,uint8_t size,uint32_t fsize,uint32_t pos)
{
	float prog;
	uint8_t t=0XFF;
	prog=(float)pos/fsize;
	prog *= 100;
	if(t!=prog)
	{
		atk_md0700_show_string(x+3*size/2, y, 240, 320, "%", mapping_font_size(size), ATK_MD0700_BLACK);
		t=prog;
		if(t>100)t=100;
		atk_md0700_show_num(x,y,t,3, mapping_font_size(size), ATK_MD0700_BLACK);//显示数值
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
uint8_t updata_fontx(FontName fontName, FontSize fontSize)
{
	uint32_t fontAddr = 0;						    
	FIL * fftemp;
	uint8_t *tempbuf;
 	uint8_t res;
	uint16_t bread;
	uint32_t offx = 0;
	char fontPath[32];
	char font_update_log_buf[100];
	uint8_t pos;
	fftemp = (FIL*)mymalloc(SRAMIN,sizeof(FIL));	//分配内存
	if(fftemp==NULL) {
// Test
		ERROR_THROW("fftemp == NULL : updata_fontx()");
		res = 1;
	}
	
	tempbuf = mymalloc(SRAMIN,4096);					//分配4096个字节空间
	if(tempbuf==NULL) {
// Test
		ERROR_THROW("tempbuf == NULL : updata_fontx()");
		res = 1;
	}
	getFontPath(fontPath, fontName, fontSize);
 	res=f_open(fftemp,(const TCHAR*)fontPath, FA_READ);
// Test
	if (res != FR_OK) {
		ERROR_THROW("updata_fontx() ==> Fail to open file corresponding to the fontPath.");
	}
 	if(res == FR_OK)
	{
#if 0
		switch(fx)
		{
			case 0:												//更新UNIGBK.BIN
				ftinfo.ugbkaddr=FONTINFOADDR+sizeof(ftinfo);	//信息头之后，紧跟UNIGBK转换码表
				ftinfo.ugbksize=fftemp->obj.objsize;			//UNIGBK大小
				flashaddr=ftinfo.ugbkaddr;
				break;
			case 1:
				ftinfo.f12addr=ftinfo.ugbkaddr+ftinfo.ugbksize;	//UNIGBK之后，紧跟GBK12字库
				ftinfo.gbk12size=fftemp->obj.objsize;			//GBK12字库大小
				flashaddr=ftinfo.f12addr;						//GBK12的起始地址
				break;
			case 2:
				ftinfo.f16addr=ftinfo.f12addr+ftinfo.gbk12size;	//GBK12之后，紧跟GBK16字库
				ftinfo.gbk16size=fftemp->obj.objsize;			//GBK16字库大小
				flashaddr=ftinfo.f16addr;						//GBK16的起始地址
				break;
			case 3:
				ftinfo.f24addr=ftinfo.f16addr+ftinfo.gbk16size;	//GBK16之后，紧跟GBK24字库
				ftinfo.gkb24size=fftemp->obj.objsize;					//GBK24字库大小
				flashaddr=ftinfo.f24addr;						//GBK24的起始地址
				break;
		}
#endif
		fontAddr = getFontAddr(fontName, fontSize);
		if ((fontName == Font_None) && (fontSize == Font_Size_None)) {
			strcpy(font_update_log_buf, "Update UNIGBK.bin ==> ");
		} else {
			strcpy(font_update_log_buf, "Update font ==> ");
		}
		pos = strlen (font_update_log_buf);
		while(res == FR_OK)//死循环执行
		{
	 		res = f_read(fftemp, tempbuf, 4096, (UINT*)&bread);			//读取数据	 
			if(res!=FR_OK)
				break;									//执行错误
			W25QXX_Write(tempbuf, fontAddr+offx, 4096);	//从0开始写入4096个数据
	  		offx += bread;
			if ((offx+1)%50 == 0) {
				sprintf(font_update_log_buf+pos, "%s%s%s%s%7d%s%7d", FontNameStr[fontName], ":", FontSizeStr[fontSize], " || ", offx+1, "/", fftemp->obj.objsize);
//				sprintf(font_update_log_buf+pos + strlen(font_update_log_buf+pos), "%s:%s || %d/%d||", FontNameStr[fontName], FontSizeStr[fontSize], offx+1, fftemp->obj.objsize);
//				printf("%s:%s || %d/%d||\r\n", FontNameStr[fontName], FontSizeStr[fontSize], offx+1, fftemp->obj.objsize);
				printf("%s\r\n", font_update_log_buf);
				delay_ms(20);
			}
			
//			fupd_prog(x,y,size,fftemp->obj.objsize,offx);	 	//进度显示
			if(bread!=4096)
				break;								//读完了.
// Test			
			printf("In infinite loop of updata_fontx...\r\n");
			delay_ms(20);
	 	}
		f_close(fftemp);		
	} 
	myfree(SRAMIN,fftemp);	//释放内存
	myfree(SRAMIN,tempbuf);	//释放内存
	return res;
} 
//更新字体文件,UNIGBK,GBK12,GBK16,GBK24一起更新
//x,y:提示信息的显示地址
//size:字体大小
//src:字库来源磁盘."0:",SD卡;"1:",FLASH盘,"2:",U盘.
//提示信息字体大小										  
//返回值:0,更新成功;
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
	uint8_t *pname;
	uint32_t *buf;
	uint8_t res=0;		   
 	uint16_t i,j;
	FIL *fftemp;
	
	res = 0xff;
	pname=mymalloc(SRAMIN,100);	//申请100字节内存  
	buf=mymalloc(SRAMIN,4096);	//申请4K字节内存  
	fftemp=(FIL*)mymalloc(SRAMIN,sizeof(FIL));	//分配内存
	if(buf==NULL||pname==NULL||fftemp==NULL)
	{
		myfree(SRAMIN,fftemp);
		myfree(SRAMIN,pname);
		myfree(SRAMIN,buf);
//		ERROR_THROW("Fail to malloc for buf, pname, fftemp.");
		return 5;	//内存申请失败
	}
	/* Check all the font file */
	for (i = Font_Min_Value; i <= Font_Max_Value; ++i) {
		for (j = Font_Min_Size; j <= Font_Max_Size; ++j) {
			getFontPath((char*)pname, i, j);
			res = f_open(fftemp, (TCHAR*)pname, FA_READ);
			if (res != FR_OK) {
				break;
			}
		}
	}
//	if (res != FR_OK) {
//		ERROR_THROW("Fail to check all font file.");
//	}
	f_close(fftemp);
#if 0
	//先查找文件是否正常 
	strcpy((char*)pname,(char*)src);	//copy src内容到pname
	strcat((char*)pname,(char*)UNIGBK_PATH); 
 	res=f_open(fftemp,(const TCHAR*)pname,FA_READ); 
 	if(res)
		rval|=1<<4;//打开文件失败  
	strcpy((char*)pname,(char*)src);	//copy src内容到pname
	strcat((char*)pname,(char*)GBK12_PATH); 
 	res=f_open(fftemp,(const TCHAR*)pname,FA_READ); 
 	if(res)
		rval|=1<<5;//打开文件失败  
	strcpy((char*)pname,(char*)src);	//copy src内容到pname
	strcat((char*)pname,(char*)GBK16_PATH); 
 	res=f_open(fftemp,(const TCHAR*)pname,FA_READ); 
 	if(res)
		rval|=1<<6;//打开文件失败  
	strcpy((char*)pname,(char*)src);	//copy src内容到pname
	strcat((char*)pname,(char*)GBK24_PATH); 
 	res=f_open(fftemp,(const TCHAR*)pname,FA_READ); 
 	if(res)
		rval|=1<<7;//打开文件失败
#endif
	
	myfree(SRAMIN,fftemp);//释放内存
	
	if(res == FR_OK)//字库文件都存在.
	{
		for (i = 0; i < 10; ++i) {
			printf("Erasing sectors...\r\n");
			delay_ms(10);
		}

//		atk_md0700_show_string(x, y, 240, 320, "Erasing sectors... ", mapping_font_size(size), ATK_MD0700_BLACK);//提示正在擦除扇区	
		for(i=0;i<FONTSECSIZE;i++)	//先擦除字库区域,提高写入速度
		{
			//fupd_prog(x+20*size/2,y,size,FONTSECSIZE,i);//进度显示
			W25QXX_Read((uint8_t*)buf,((FONT_OCCUPY_ALL/4096)+i)*4096,4096);//读出整个扇区的内容
			for(j=0;j<1024;j++)//校验数据
			{
				if(buf[j]!=0XFFFFFFFF)break;//需要擦除
			}
			if(j!=1024)
				W25QXX_Erase_Sector((FONT_OCCUPY_ALL/4096)+i);	//需要擦除的扇区
			/* Print erasing flash log */
			if ((i+1)%50 == 0) {
				printf("Erasing flash sector: %d/%d\r\n", i+1, FONTSECSIZE);
				delay_ms(10);
			}
		}
		/* The log of the operation for the last sector */
		if ((FONTSECSIZE)%50 != 0) {
			printf("Erasing flash sector: %d/%d\r\n", i+1, FONTSECSIZE);
		}
		myfree(SRAMIN, buf);
//		buf = NULL;
		
		
		res = updata_fontx(Font_None, Font_Size_None);
// Test
		if (res) {
			ERROR_THROW("Fail to load UNIGBK.bin to flash.");
		}
		if (res == 0) {
			for (i = Font_Min_Value; i <= Font_Max_Value; ++i) {
				for (j = Font_Min_Size; j <= Font_Max_Size; ++j) {
					res = updata_fontx(i, j);
					if (res) {
						break;
					}
				}
			}
		}
// Test
		if (res) {
			while (1) {
				printf("Fail to load font library[%s:%s] to flash.\r\n", FontNameStr[i], FontSizeStr[j]);
				delay_ms(20);
			}
		}
		
#if 0
		atk_md0700_show_string(x, y, 240, 320, "Updating UNIGBK.BIN", mapping_font_size(size), ATK_MD0700_BLACK);		
		strcpy((char*)pname,(char*)src);				//copy src内容到pname
		strcat((char*)pname,(char*)UNIGBK_PATH); 
		res=updata_fontx(x+20*size/2,y,size,pname,0);	//更新UNIGBK.BIN
		if(res){myfree(SRAMIN,pname);return 1;}
		atk_md0700_show_string(x, y, 240, 320, "Updating GBK12.BIN  ", mapping_font_size(size), ATK_MD0700_BLACK);
		strcpy((char*)pname,(char*)src);				//copy src内容到pname
		strcat((char*)pname,(char*)GBK12_PATH); 
		res=updata_fontx(x+20*size/2,y,size,pname,1);	//更新GBK12.FON
		if(res){myfree(SRAMIN,pname);return 2;}
		atk_md0700_show_string(x,y,240,320,"Updating GBK16.BIN  ", mapping_font_size(size), ATK_MD0700_BLACK);
		strcpy((char*)pname,(char*)src);				//copy src内容到pname
		strcat((char*)pname,(char*)GBK16_PATH); 
		res=updata_fontx(x+20*size/2,y,size,pname,2);	//更新GBK16.FON
		if(res){myfree(SRAMIN,pname);return 3;}
		atk_md0700_show_string(x, y, 240, 320, "Updating GBK24.BIN  ", mapping_font_size(size), ATK_MD0700_BLACK);
		strcpy((char*)pname,(char*)src);				//copy src内容到pname
		strcat((char*)pname,(char*)GBK24_PATH); 
		res=updata_fontx(x+20*size/2,y,size,pname,3);	//更新GBK24.FON
#endif		
		if(res){
			myfree(SRAMIN,pname);
// Test			
			ERROR_THROW("updata_fontx() ==> pname free error\r\n");
			return 4;
		}
		//全部更新好了
		/* Prepare FontHeader OK */
		fontHeader.fontok = FLAG_OK;
		fontHeader.ugbkok = FLAG_OK;
		
		W25QXX_Write((uint8_t*)&fontHeader, FONT_HEADER_ADDR, sizeof(fontHeader));	//保存字库信息
		for (i = 0; i < 10; ++i) {
			printf("Write fontHeader to flash ==> \r\n\
			\b fontHeader.fontok = FLAG_OK, fontHeader.ugbkok = FLAG_OK");
		}
	}
	myfree(SRAMIN,pname);//释放内存 
	myfree(SRAMIN,buf);
	return res;//无错误.			 
}


