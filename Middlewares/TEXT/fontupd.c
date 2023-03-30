#include "fontupd.h"
#include "FATFS/src/ff.h"
#include "BSP/W25QXX/w25qxx.h"
#include "BSP/ATK_MD0700/atk_md0700.h"
#include "FATFS/exfuns/exfuns.h"
//#include "lcd.h"
#include "string.h"
#include "malloc.h"
#include "SYSTEM/delay/delay.h"
#include "SYSTEM/usart/usart.h"
#include "font.h"
#include "log.h"
//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F103������
//�ֿ���� ��������
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2017/6/1
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

//�ֿ�����ռ�õ�����������С(3���ֿ�+unigbk��+�ֿ���Ϣ=3238700�ֽ�,Լռ791��W25QXX����)
/* One sector = 4 KB*/
//#define FONTSECSIZE	 	791
//�ֿ�����ʼ��ַ
//#define FONTINFOADDR 	1024*1024*12 					//Explorer STM32F4�Ǵ�12M��ַ�Ժ�ʼ����ֿ�
														//ǰ��12M��fatfsռ����.
														//12M�Ժ����3���ֿ�+UNIGBK.BIN,�ܴ�С3.09M,���ֿ�ռ����,���ܶ�!
														//15.10M�Ժ�,�û���������ʹ��.����������100K�ֽڱȽϺ�.
														
//���������ֿ������Ϣ����ַ����С��
FontHeader fontHeader;

//uint8_t font_flag;

//�ֿ����ڴ����е�·��
//uint8_t*const GBK24_PATH="/SYSTEM/FONT/GBK24.FON";		//GBK24�Ĵ��λ��
//uint8_t*const GBK16_PATH="/SYSTEM/FONT/GBK16.FON";		//GBK16�Ĵ��λ��
//uint8_t*const GBK12_PATH="/SYSTEM/FONT/GBK12.FON";		//GBK12�Ĵ��λ��
//uint8_t*const UNIGBK_PATH="/SYSTEM/FONT/UNIGBK.BIN";	//UNIGBK.BIN�Ĵ��λ��

//��ʾ��ǰ������½���
//x,y:����
//size:�����С
//fsize:�����ļ���С
//pos:��ǰ�ļ�ָ��λ��
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
		atk_md0700_show_num(x,y,t,3, mapping_font_size(size), ATK_MD0700_BLACK);//��ʾ��ֵ
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
#if 0
	FIL * fftemp;
	uint8_t *tempbuf;
	uint16_t bread;
	uint32_t offx = 0;
#endif
	uint8_t res = 0;
	uint32_t fontAddr = 0;
	char* fontPath = mymalloc(SRAMIN, 100);
#if 0
//	char font_update_log_buf[100];
	uint8_t pos;
	fftemp = (FIL*)mymalloc(SRAMIN,sizeof(FIL));	//�����ڴ�
	if(fftemp==NULL) {
// Test start
		ERROR_THROW("fftemp == NULL : updata_fontx()");
// Test end
		res = 1;
	}
	
	tempbuf = mymalloc(SRAMIN,4096);					//����4096���ֽڿռ�
	if(tempbuf==NULL) {
// Test start
		ERROR_THROW("tempbuf == NULL : updata_fontx()");
// Test end
		res = 1;
	}
#endif
	getFontPath(fontPath, fontName, fontSize);
 	res = f_open(temp_file,(const TCHAR*)fontPath, FA_READ);
	if (res != FR_OK) {
		infinite_throw("updata_fontx() ==> Fail to open file corresponding to the fontPath.");
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
		while(res == FR_OK)//��ѭ��ִ��
		{
	 		res = f_read(fftemp, tempbuf, 4096, (UINT*)&bread);			//��ȡ����	 
			if(res!=FR_OK) {
// Test start
				while (1) {
					printf("Fail to read from file: %s\r\n", fontPath);
					delay_ms(10);
				}
// Test end
				break;									//ִ�д���
			}
			W25QXX_Write(tempbuf, fontAddr+offx, 4096);	//��0��ʼд��4096������
	  		offx += bread;
/* !! Maybe need a function to refactor the code ==> Start */
			if (offx%(5*4096) == 0) {
				sprintf(font_update_log_buf+pos, "%s%s%s%s%7d%s%7d", 
					FontNameStr[fontName], ":", FontSizeStr[fontSize], " || ", offx+1, "/", fftemp->obj.objsize);
				PRINTLN_MSG(font_update_log_buf);
			}
/* !! Maybe need a function to refactor the code ==> End */
			
//			fupd_prog(x,y,size,fftemp->obj.objsize,offx);	 	//������ʾ
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
				break;								//������.
			}
// Test	start	
			printf("In infinite loop of updata_fontx...\r\n");
			delay_ms(20);
// Test end
	 	}
		f_close(fftemp);
#endif
//	}
#if 0
	myfree(SRAMIN,fftemp);	//�ͷ��ڴ�
	myfree(SRAMIN,tempbuf);	//�ͷ��ڴ�
#endif
	myfree(SRAMIN, fontPath);
	return res;
} 
//���������ļ�,UNIGBK,GBK12,GBK16,GBK24һ�����
//x,y:��ʾ��Ϣ����ʾ��ַ
//size:�����С
//src:�ֿ���Դ����."0:",SD��;"1:",FLASH��,"2:",U��.
//��ʾ��Ϣ�����С										  
//����ֵ:0,���³ɹ�;
//		 ����,�������.
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
	uint8_t pname[100];
	uint8_t res = 0;		   
 	uint16_t i,j;
//	uint32_t *buf;
//	FIL *temp_file;
	
	res = 0xff;
#if 0
//	pname = mymalloc(SRAMIN,100);	//����100�ֽ��ڴ�  
//	buf = mymalloc(SRAMIN,4096);	//����4K�ֽ��ڴ�
//	fftemp = (FIL*)mymalloc(SRAMIN, sizeof(FIL));	//�����ڴ�
	if(buf==NULL || pname==NULL || fftemp==NULL)
	{
		myfree(SRAMIN,fftemp);
		myfree(SRAMIN,pname);
		myfree(SRAMIN,buf);
//		ERROR_THROW("Fail to malloc for buf, pname, fftemp.");
		return 5;	//�ڴ�����ʧ��
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
//	myfree(SRAMIN,fftemp);//�ͷ��ڴ�
	
	if(res == FR_OK)//�ֿ��ļ�������.
	{
		log_n("%sErasing sectors...", ARROW_STRING);

//		atk_md0700_show_string(x, y, 240, 320, "Erasing sectors... ", mapping_font_size(size), ATK_MD0700_BLACK);//��ʾ���ڲ�������	
//		printf("Erasing flash sector: %4d/%4d\r\n", 0, FONTSECSIZE);
		Progress_Init(&logParam.progress, 0, FONTSECSIZE);
		for(i=0; i<FONTSECSIZE; i++)	//�Ȳ����ֿ�����,���д���ٶ�
		{
			Progress_Update(&logParam.progress, i);
			print_log(Flash_Erase_Log, &logParam);
//			fupd_prog(x+20*size/2,y,size,FONTSECSIZE,i);//������ʾ
			W25QXX_Read((uint8_t*)flash_buffer, ((FONT_OCCUPY_ALL/4096)+i)*4096, FLASH_BUFFER_SIZE);//������������������
			for(j=0; j<1024; j++)//У������
			{
				if(*((uint32_t*)flash_buffer+j) != 0XFFFFFFFF) {
					break;//��Ҫ����
				}
			}
			if(j!=1024) {
				W25QXX_Erase_Sector((FONT_OCCUPY_ALL/4096)+i);	//��Ҫ����������
			}
//			/* Print erasing flash log */
//			if ((i+1)%50 == 0) {
//				printf("Erasing flash sector: %4d/%4d\r\n", i+1, FONTSECSIZE);
////				delay_ms(10);
//			}
		}
		/* The log of the erasing operation for the last sector */
//		if ((FONTSECSIZE)%50 != 0) {
//			printf("Erasing flash sector: %4d/%4d\r\n", i+1, FONTSECSIZE);
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
					res = updata_fontx((FontName)i, (FontSize)j);
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
		if(res){
//			myfree(SRAMIN, pname);
// Test	start	
			infinite_throw("updata_fontx() ==> pname free error\r\n");
// Test end
			return 4;
		}
		//ȫ�����º���
		/* Prepare FontHeader OK */
		fontHeader.fontok = FLAG_OK;
		fontHeader.ugbkok = FLAG_OK;
		
		W25QXX_Write((uint8_t*)&fontHeader, FONT_HEADER_ADDR, sizeof(fontHeader));	//�����ֿ���Ϣ
		log_n("Write fontHeader to flash ==> \r\n\
		fontHeader.fontok = FLAG_OK, \r\n\b\
		fontHeader.ugbkok = FLAG_OK");
	}
//	myfree(SRAMIN,pname);//�ͷ��ڴ� 
//	myfree(SRAMIN,buf);
	return res;//�޴���.			 
}


uint8_t update_mapping_table(void) {
	uint8_t res = 0;
	char fname[100];
	uint32_t fileAddr = getMappingTableAddr();
	getMappingTablePath((uint8_t*)fname);
	res = load_file_to_flash(fname, fileAddr);
	return res;
}
