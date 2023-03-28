#ifndef __TEXT_H__
#define __TEXT_H__	 
#include <stm32f1xx.h>
#include "fontupd.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F103������
//������ʾ ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2017/6/1
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 
 					     
//�õ����ֵĵ�����
void Get_HzMat(unsigned char *code, unsigned char *mat, FontName fontName, FontSize fontSize);
//��ָ��λ����ʾһ������
void Show_Font(uint16_t x,uint16_t y,uint8_t *font,uint8_t fontName, FontSize fontSize, uint8_t mode);
//��ָ��λ����ʾһ���ַ��� 
void Show_Str(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t*str, 
	FontName fontName, FontSize fontSize,uint8_t mode);
void Show_Str_Mid(uint16_t x,uint16_t y,uint8_t*str, FontName fontName, FontSize fontSize, uint8_t len);
#endif
