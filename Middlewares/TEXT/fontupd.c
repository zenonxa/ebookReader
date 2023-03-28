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
	uint32_t fontAddr = 0;						    
	FIL * fftemp;
	uint8_t *tempbuf;
 	uint8_t res;
	uint16_t bread;
	uint32_t offx = 0;
	char fontPath[32];
	char font_update_log_buf[100];
	uint8_t pos;
	fftemp = (FIL*)mymalloc(SRAMIN,sizeof(FIL));	//�����ڴ�
	if(fftemp==NULL) {
// Test
		ERROR_THROW("fftemp == NULL : updata_fontx()");
		res = 1;
	}
	
	tempbuf = mymalloc(SRAMIN,4096);					//����4096���ֽڿռ�
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
			case 0:												//����UNIGBK.BIN
				ftinfo.ugbkaddr=FONTINFOADDR+sizeof(ftinfo);	//��Ϣͷ֮�󣬽���UNIGBKת�����
				ftinfo.ugbksize=fftemp->obj.objsize;			//UNIGBK��С
				flashaddr=ftinfo.ugbkaddr;
				break;
			case 1:
				ftinfo.f12addr=ftinfo.ugbkaddr+ftinfo.ugbksize;	//UNIGBK֮�󣬽���GBK12�ֿ�
				ftinfo.gbk12size=fftemp->obj.objsize;			//GBK12�ֿ��С
				flashaddr=ftinfo.f12addr;						//GBK12����ʼ��ַ
				break;
			case 2:
				ftinfo.f16addr=ftinfo.f12addr+ftinfo.gbk12size;	//GBK12֮�󣬽���GBK16�ֿ�
				ftinfo.gbk16size=fftemp->obj.objsize;			//GBK16�ֿ��С
				flashaddr=ftinfo.f16addr;						//GBK16����ʼ��ַ
				break;
			case 3:
				ftinfo.f24addr=ftinfo.f16addr+ftinfo.gbk16size;	//GBK16֮�󣬽���GBK24�ֿ�
				ftinfo.gkb24size=fftemp->obj.objsize;					//GBK24�ֿ��С
				flashaddr=ftinfo.f24addr;						//GBK24����ʼ��ַ
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
		while(res == FR_OK)//��ѭ��ִ��
		{
	 		res = f_read(fftemp, tempbuf, 4096, (UINT*)&bread);			//��ȡ����	 
			if(res!=FR_OK)
				break;									//ִ�д���
			W25QXX_Write(tempbuf, fontAddr+offx, 4096);	//��0��ʼд��4096������
	  		offx += bread;
			if ((offx+1)%50 == 0) {
				sprintf(font_update_log_buf+pos, "%s%s%s%s%7d%s%7d", FontNameStr[fontName], ":", FontSizeStr[fontSize], " || ", offx+1, "/", fftemp->obj.objsize);
//				sprintf(font_update_log_buf+pos + strlen(font_update_log_buf+pos), "%s:%s || %d/%d||", FontNameStr[fontName], FontSizeStr[fontSize], offx+1, fftemp->obj.objsize);
//				printf("%s:%s || %d/%d||\r\n", FontNameStr[fontName], FontSizeStr[fontSize], offx+1, fftemp->obj.objsize);
				printf("%s\r\n", font_update_log_buf);
				delay_ms(20);
			}
			
//			fupd_prog(x,y,size,fftemp->obj.objsize,offx);	 	//������ʾ
			if(bread!=4096)
				break;								//������.
// Test			
			printf("In infinite loop of updata_fontx...\r\n");
			delay_ms(20);
	 	}
		f_close(fftemp);		
	} 
	myfree(SRAMIN,fftemp);	//�ͷ��ڴ�
	myfree(SRAMIN,tempbuf);	//�ͷ��ڴ�
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
	uint8_t *pname;
	uint32_t *buf;
	uint8_t res=0;		   
 	uint16_t i,j;
	FIL *fftemp;
	
	res = 0xff;
	pname=mymalloc(SRAMIN,100);	//����100�ֽ��ڴ�  
	buf=mymalloc(SRAMIN,4096);	//����4K�ֽ��ڴ�  
	fftemp=(FIL*)mymalloc(SRAMIN,sizeof(FIL));	//�����ڴ�
	if(buf==NULL||pname==NULL||fftemp==NULL)
	{
		myfree(SRAMIN,fftemp);
		myfree(SRAMIN,pname);
		myfree(SRAMIN,buf);
//		ERROR_THROW("Fail to malloc for buf, pname, fftemp.");
		return 5;	//�ڴ�����ʧ��
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
	//�Ȳ����ļ��Ƿ����� 
	strcpy((char*)pname,(char*)src);	//copy src���ݵ�pname
	strcat((char*)pname,(char*)UNIGBK_PATH); 
 	res=f_open(fftemp,(const TCHAR*)pname,FA_READ); 
 	if(res)
		rval|=1<<4;//���ļ�ʧ��  
	strcpy((char*)pname,(char*)src);	//copy src���ݵ�pname
	strcat((char*)pname,(char*)GBK12_PATH); 
 	res=f_open(fftemp,(const TCHAR*)pname,FA_READ); 
 	if(res)
		rval|=1<<5;//���ļ�ʧ��  
	strcpy((char*)pname,(char*)src);	//copy src���ݵ�pname
	strcat((char*)pname,(char*)GBK16_PATH); 
 	res=f_open(fftemp,(const TCHAR*)pname,FA_READ); 
 	if(res)
		rval|=1<<6;//���ļ�ʧ��  
	strcpy((char*)pname,(char*)src);	//copy src���ݵ�pname
	strcat((char*)pname,(char*)GBK24_PATH); 
 	res=f_open(fftemp,(const TCHAR*)pname,FA_READ); 
 	if(res)
		rval|=1<<7;//���ļ�ʧ��
#endif
	
	myfree(SRAMIN,fftemp);//�ͷ��ڴ�
	
	if(res == FR_OK)//�ֿ��ļ�������.
	{
		for (i = 0; i < 10; ++i) {
			printf("Erasing sectors...\r\n");
			delay_ms(10);
		}

//		atk_md0700_show_string(x, y, 240, 320, "Erasing sectors... ", mapping_font_size(size), ATK_MD0700_BLACK);//��ʾ���ڲ�������	
		for(i=0;i<FONTSECSIZE;i++)	//�Ȳ����ֿ�����,���д���ٶ�
		{
			//fupd_prog(x+20*size/2,y,size,FONTSECSIZE,i);//������ʾ
			W25QXX_Read((uint8_t*)buf,((FONT_OCCUPY_ALL/4096)+i)*4096,4096);//������������������
			for(j=0;j<1024;j++)//У������
			{
				if(buf[j]!=0XFFFFFFFF)break;//��Ҫ����
			}
			if(j!=1024)
				W25QXX_Erase_Sector((FONT_OCCUPY_ALL/4096)+i);	//��Ҫ����������
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
		strcpy((char*)pname,(char*)src);				//copy src���ݵ�pname
		strcat((char*)pname,(char*)UNIGBK_PATH); 
		res=updata_fontx(x+20*size/2,y,size,pname,0);	//����UNIGBK.BIN
		if(res){myfree(SRAMIN,pname);return 1;}
		atk_md0700_show_string(x, y, 240, 320, "Updating GBK12.BIN  ", mapping_font_size(size), ATK_MD0700_BLACK);
		strcpy((char*)pname,(char*)src);				//copy src���ݵ�pname
		strcat((char*)pname,(char*)GBK12_PATH); 
		res=updata_fontx(x+20*size/2,y,size,pname,1);	//����GBK12.FON
		if(res){myfree(SRAMIN,pname);return 2;}
		atk_md0700_show_string(x,y,240,320,"Updating GBK16.BIN  ", mapping_font_size(size), ATK_MD0700_BLACK);
		strcpy((char*)pname,(char*)src);				//copy src���ݵ�pname
		strcat((char*)pname,(char*)GBK16_PATH); 
		res=updata_fontx(x+20*size/2,y,size,pname,2);	//����GBK16.FON
		if(res){myfree(SRAMIN,pname);return 3;}
		atk_md0700_show_string(x, y, 240, 320, "Updating GBK24.BIN  ", mapping_font_size(size), ATK_MD0700_BLACK);
		strcpy((char*)pname,(char*)src);				//copy src���ݵ�pname
		strcat((char*)pname,(char*)GBK24_PATH); 
		res=updata_fontx(x+20*size/2,y,size,pname,3);	//����GBK24.FON
#endif		
		if(res){
			myfree(SRAMIN,pname);
// Test			
			ERROR_THROW("updata_fontx() ==> pname free error\r\n");
			return 4;
		}
		//ȫ�����º���
		/* Prepare FontHeader OK */
		fontHeader.fontok = FLAG_OK;
		fontHeader.ugbkok = FLAG_OK;
		
		W25QXX_Write((uint8_t*)&fontHeader, FONT_HEADER_ADDR, sizeof(fontHeader));	//�����ֿ���Ϣ
		for (i = 0; i < 10; ++i) {
			printf("Write fontHeader to flash ==> \r\n\
			\b fontHeader.fontok = FLAG_OK, fontHeader.ugbkok = FLAG_OK");
		}
	}
	myfree(SRAMIN,pname);//�ͷ��ڴ� 
	myfree(SRAMIN,buf);
	return res;//�޴���.			 
}


