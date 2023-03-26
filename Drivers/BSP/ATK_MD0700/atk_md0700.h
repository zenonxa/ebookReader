/**
 ****************************************************************************************************
 * @file        atk_md0700.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-06-21
 * @brief       ATK-MD0700ģ����������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32F103������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#ifndef __ATK_MD0700_H
#define __ATK_MD0700_H

#include "./SYSTEM/sys/sys.h"

/* ����ATK-MD0700ģ��LCD�ߴ� */
#define ATK_MD0700_LCD_WIDTH            480
#define ATK_MD0700_LCD_HEIGHT           800

/* �����Ƿ�ʹ��ATK-MD0700ģ�鴥�� */
#define ATK_MD0700_USING_TOUCH          1

/* ����ATK-MD0700ģ�����õ����� */
#define ATK_MD0700_FONT_12              1
#define ATK_MD0700_FONT_16              1
#define ATK_MD0700_FONT_24              1
#define ATK_MD0700_FONT_32              1

/* Ĭ�����ô��� */
#ifndef ATK_MD0700_USING_TOUCH
#define ATK_MD0700_USING_TOUCH 1
#endif

/* Ĭ������12������ */
#if ((ATK_MD0700_FONT_12 == 0) && (ATK_MD0700_FONT_16 == 0) && (ATK_MD0700_FONT_24 == 0) && (ATK_MD0700_FONT_32 == 0))
#undef ATK_MD0700_FONT_12
#defien ATK_MD0700_FONT_12 1
#endif

/* ����ͷ�ļ� */
#if (ATK_MD0700_USING_TOUCH != 0)
#include "./BSP/ATK_MD0700/atk_md0700_touch.h"
#endif

/* ATK-MD0700ģ��LCDɨ�跽��ö�� */
typedef enum
{
    ATK_MD0700_LCD_SCAN_DIR_L2R_U2D = 0x0000,   /* �����ң����ϵ��� */
    ATK_MD0700_LCD_SCAN_DIR_L2R_D2U = 0x0080,   /* �����ң����µ��� */
    ATK_MD0700_LCD_SCAN_DIR_R2L_U2D = 0x0040,   /* ���ҵ��󣬴��ϵ��� */
    ATK_MD0700_LCD_SCAN_DIR_R2L_D2U = 0x00C0,   /* ���ҵ��󣬴��µ��� */
    ATK_MD0700_LCD_SCAN_DIR_U2D_L2R = 0x0020,   /* ���ϵ��£������� */
    ATK_MD0700_LCD_SCAN_DIR_U2D_R2L = 0x0060,   /* ���ϵ��£����ҵ��� */
    ATK_MD0700_LCD_SCAN_DIR_D2U_L2R = 0x00A0,   /* ���µ��ϣ������� */
    ATK_MD0700_LCD_SCAN_DIR_D2U_R2L = 0x00E0,   /* ���µ��ϣ����ҵ��� */
} atk_md0700_lcd_scan_dir_t;

/* ATK-MD0700ģ��LCD��ת����ö�� */
typedef enum
{
    ATK_MD0700_LCD_DISP_DIR_0 = 0x00,           /* LCD˳ʱ����ת0����ʾ���� */
    ATK_MD0700_LCD_DISP_DIR_90,                 /* LCD˳ʱ����ת90����ʾ���� */
    ATK_MD0700_LCD_DISP_DIR_180,                /* LCD˳ʱ����ת180����ʾ���� */
    ATK_MD0700_LCD_DISP_DIR_270,                /* LCD˳ʱ����ת270����ʾ���� */
} atk_md0700_lcd_disp_dir_t;

/* ATK-MD0700ģ��LCD��ʾ����ö�� */
typedef enum
{
#if (ATK_MD0700_FONT_12 != 0)
    ATK_MD0700_LCD_FONT_12 = 12,             /* 12������ */
#endif
#if (ATK_MD0700_FONT_16 != 0)
    ATK_MD0700_LCD_FONT_16 = 16,             /* 16������ */
#endif
#if (ATK_MD0700_FONT_24 != 0)
    ATK_MD0700_LCD_FONT_24 = 24,             /* 24������ */
#endif
#if (ATK_MD0700_FONT_32 != 0)
    ATK_MD0700_LCD_FONT_32 = 32,             /* 32������ */
#endif
} atk_md0700_lcd_font_t;

/* ATK-MD0700ģ��LCD��ʾ����ģʽö�� */
typedef enum
{
    ATK_MD0700_NUM_SHOW_NOZERO = 0x00,  /* ���ָ�λ0����ʾ */
    ATK_MD0700_NUM_SHOW_ZERO,           /* ���ָ�λ0��ʾ */
} atk_md0700_num_mode_t;

typedef enum {
	SINGLE_COLOR_BLOCK,
	MULTI_COLOR_BLOCK,
} atk_md0700_lcd_fill_mode_t;

/* ������ɫ���壨RGB565�� */
#define ATK_MD0700_WHITE        0xFFFF
#define ATK_MD0700_BLACK        0x0000
#define ATK_MD0700_BLUE         0x001F
#define ATK_MD0700_BRED         0XF81F
#define ATK_MD0700_GRED         0XFFE0
#define ATK_MD0700_GBLUE        0X07FF
#define ATK_MD0700_RED          0xF800
#define ATK_MD0700_MAGENTA      0xF81F
#define ATK_MD0700_GREEN        0x07E0
#define ATK_MD0700_CYAN         0x7FFF
#define ATK_MD0700_YELLOW       0xFFE0
#define ATK_MD0700_BROWN        0XBC40
#define ATK_MD0700_BRRED        0XFC07
#define ATK_MD0700_GRAY         0X8430

/* ������� */
#define ATK_MD0700_EOK          0   /* û�д��� */
#define ATK_MD0700_ERROR        1   /* ���� */
#define ATK_MD0700_EINVAL       2   /* �Ƿ����� */

/* �������� */
uint8_t atk_md0700_init(void);                                                                                                                          /* ATK-MD0700ģ���ʼ�� */
uint16_t atk_md0700_get_lcd_width(void);                                                                                                                /* ��ȡATK-MD0700ģ��LCD��� */
uint16_t atk_md0700_get_lcd_height(void);                                                                                                               /* ��ȡATK-MD0700ģ��LCD�߶� */
void atk_md0700_backlight_config(uint8_t pwm);                                                                                                          /* ����ATK-MD0700ģ��LCD�������� */
void atk_md0700_display_on(void);                                                                                                                       /* ����ATK-MD0700ģ��LCD��ʾ */
void atk_md0700_display_off(void);                                                                                                                      /* �ر�ATK-MD0700ģ��LCD��ʾ */
uint8_t atk_md0700_set_scan_dir(atk_md0700_lcd_scan_dir_t scan_dir);                                                                                    /* ����ATK-MD0700ģ��LCDɨ�跽�� */
uint8_t atk_md0700_set_disp_dir(atk_md0700_lcd_disp_dir_t disp_dir);                                                                                    /* ����ATK-MD0700ģ��LCD��ʾ���� */
atk_md0700_lcd_scan_dir_t atk_md0700_get_scan_dir(void);                                                                                                /* ��ȡATK-MD0700ģ��LCDɨ�跽�� */
atk_md0700_lcd_disp_dir_t atk_md0700_get_disp_dir(void);                                                                                                /* ��ȡATK-MD0700ģ��LCD��ʾ���� */
void atk_md0700_fill(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t* color, atk_md0700_lcd_fill_mode_t fill_mode);                        /* ATK-MD0700ģ��LCD������� */
void atk_md0700_clear(uint16_t color);                                                                                                                  /* ATK-MD0700ģ��LCD���� */
void atk_md0700_draw_point(uint16_t x, uint16_t y, uint16_t color);                                                                                     /* ATK-MD0700ģ��LCD���� */
uint16_t atk_md0700_read_point(uint16_t x, uint16_t y);                                                                                                 /* ATK-MD0700ģ��LCD���� */
void atk_md0700_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);                                                          /* ATK-MD0700ģ��LCD���߶� */
void atk_md0700_draw_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);                                                          /* ATK-MD0700ģ��LCD�����ο� */
void atk_md0700_draw_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color);                                                                        /* ATK-MD0700ģ��LCD��Բ�ο� */
void atk_md0700_show_char(uint16_t x, uint16_t y, char ch, atk_md0700_lcd_font_t font, uint16_t color);                                                 /* ATK-MD0700ģ��LCD��ʾ1���ַ� */
void atk_md0700_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *str, atk_md0700_lcd_font_t font, uint16_t color);            /* ATK-MD0700ģ��LCD��ʾ�ַ��� */
void atk_md0700_show_xnum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, atk_md0700_num_mode_t mode, atk_md0700_lcd_font_t font, uint16_t color);   /* ATK-MD0700ģ��LCD��ʾ���֣��ɿ�����ʾ��λ0 */
void atk_md0700_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len, atk_md0700_lcd_font_t font, uint16_t color);                                /* ATK-MD0700ģ��LCD��ʾ���֣�����ʾ��λ0 */
void atk_md0700_show_pic(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *pic);                                                        /* ATK-MD0700ģ��LCDͼƬ */
atk_md0700_lcd_font_t mapping_font_size(uint8_t size);		// ת��u8����Ϊ�ڲ�atk_md0700_lcd_font_t���ͣ���ʾ�����С

#endif
