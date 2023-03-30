#ifndef __FONTUPD_H__
#define __FONTUPD_H__	 
#include "SYSTEM/sys/sys.h"
#include "font.h"
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


//������Ϣ�����ַ,ռ33���ֽ�,��1���ֽ����ڱ���ֿ��Ƿ����.����ÿ8���ֽ�һ��,�ֱ𱣴���ʼ��ַ���ļ���С														   
//extern uint32_t FONTINFOADDR;
//�ֿ���Ϣ�ṹ�嶨��
//���������ֿ������Ϣ����ַ����С��

#if 0
/* Information of one font item, including address and size in flash */
typedef struct {
	uint32_t addr;	/* address in flash of the font library */
} FontItem;
#endif

extern FontHeader fontHeader;	//�ֿ���Ϣ�ṹ��

uint32_t fupd_prog(uint16_t x,uint16_t y,uint8_t size,uint32_t fsize,uint32_t pos);	//��ʾ���½���
uint8_t updata_fontx(FontName fontName, FontSize fontSize);						//����ָ���ֿ�
uint8_t update_font(void);														//����ȫ���ֿ�
uint8_t update_mapping_table(void);

#endif
