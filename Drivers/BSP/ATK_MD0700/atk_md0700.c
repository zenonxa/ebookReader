/**
 ****************************************************************************************************
 * @file        atk_md0700.c
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

#include "./BSP/ATK_MD0700/atk_md0700.h"
#include "./BSP/ATK_MD0700/atk_md0700_font.h"
#include "./BSP/ATK_MD0700/atk_md0700_fsmc.h"
#include "./SYSTEM/delay/delay.h"

/* ATK-MD0700ģ��LCD��ƷID */
#define ATK_MD0700_PID              0x61    /* ��ƷID */

/* ATK-MD0700ģ��LCDɨ�跽�� */
#define ATK_MD0700_SCAN_DIR_L2R_U2D (0x0000)
#define ATK_MD0700_SCAN_DIR_L2R_D2U (0x0080)
#define ATK_MD0700_SCAN_DIR_R2L_U2D (0x0040)
#define ATK_MD0700_SCAN_DIR_R2L_D2U (0x00C0)
#define ATK_MD0700_SCAN_DIR_U2D_L2R (0x0020)
#define ATK_MD0700_SCAN_DIR_U2D_R2L (0x0060)
#define ATK_MD0700_SCAN_DIR_D2U_L2R (0x00A0)
#define ATK_MD0700_SCAN_DIR_D2U_R2L (0x00E0)

/* ATK-MD0700ģ��״̬���ݽṹ�� */
static struct
{
    uint8_t pid;                        /* ������PID */
    uint16_t width;                     /* LCD��� */
    uint16_t height;                    /* LCD�߶� */
    atk_md0700_lcd_scan_dir_t scan_dir; /* LCDɨ�跽�� */
    atk_md0700_lcd_disp_dir_t disp_dir; /* LCD��ʾ���� */
} g_atk_md0700_sta = {0};

/**
 * @brief       ��ȡATK-MD0700ģ��LCD������PID
 * @param       ��
 * @retval      LCD��������ƷID
 */
static uint8_t atk_md0700_get_pid(void)
{
    uint8_t pid;
    
    atk_md0700_fsmc_write_cmd(0xA1);
    
    atk_md0700_fsmc_read_dat();
    atk_md0700_fsmc_read_dat();
    pid = atk_md0700_fsmc_read_dat();
    
    return pid;
}

/**
 * @brief       ATK-MD0700ģ��Ĵ�����ʼ��
 * @param       ��
 * @retval      ��
 */
static void atk_md0700_reg_init(void)
{
    atk_md0700_fsmc_write_cmd(0xE2);
    atk_md0700_fsmc_write_dat(0x1D);
    atk_md0700_fsmc_write_dat(0x02);
    atk_md0700_fsmc_write_dat(0x04);
    delay_us(100);
    atk_md0700_fsmc_write_cmd(0xE0);
    atk_md0700_fsmc_write_dat(0x01);
    delay_ms(10);
    atk_md0700_fsmc_write_cmd(0xE0);
    atk_md0700_fsmc_write_dat(0x03);
    delay_ms(12);
    atk_md0700_fsmc_write_cmd(0x01);
    delay_ms(10);
    atk_md0700_fsmc_write_cmd(0xE6);
    atk_md0700_fsmc_write_dat(0x2F);
    atk_md0700_fsmc_write_dat(0xFF);
    atk_md0700_fsmc_write_dat(0xFF);
    atk_md0700_fsmc_write_cmd(0xB0);
    atk_md0700_fsmc_write_dat(0x20);
    atk_md0700_fsmc_write_dat(0x00);
    atk_md0700_fsmc_write_dat(0x03);
    atk_md0700_fsmc_write_dat(0x1F);
    atk_md0700_fsmc_write_dat(0x01);
    atk_md0700_fsmc_write_dat(0xDF);
    atk_md0700_fsmc_write_dat(0x00);
    atk_md0700_fsmc_write_cmd(0xB4);
    atk_md0700_fsmc_write_dat(0x04);
    atk_md0700_fsmc_write_dat(0x1F);
    atk_md0700_fsmc_write_dat(0x00);
    atk_md0700_fsmc_write_dat(0x2E);
    atk_md0700_fsmc_write_dat(0x00);
    atk_md0700_fsmc_write_dat(0x00);
    atk_md0700_fsmc_write_dat(0x00);
    atk_md0700_fsmc_write_dat(0x00);
    atk_md0700_fsmc_write_cmd(0xB6);
    atk_md0700_fsmc_write_dat(0x02);
    atk_md0700_fsmc_write_dat(0x0C);
    atk_md0700_fsmc_write_dat(0x00);
    atk_md0700_fsmc_write_dat(0x17);
    atk_md0700_fsmc_write_dat(0x16);
    atk_md0700_fsmc_write_dat(0x00);
    atk_md0700_fsmc_write_dat(0x00);
    atk_md0700_fsmc_write_cmd(0xF0);
    atk_md0700_fsmc_write_dat(0x03);
    atk_md0700_fsmc_write_cmd(0x29);
    atk_md0700_fsmc_write_cmd(0xD0);
    atk_md0700_fsmc_write_dat(0x00);
    atk_md0700_fsmc_write_cmd(0xBE);
    atk_md0700_fsmc_write_dat(0x05);
    atk_md0700_fsmc_write_dat(0xFE);
    atk_md0700_fsmc_write_dat(0x01);
    atk_md0700_fsmc_write_dat(0x00);
    atk_md0700_fsmc_write_dat(0x00);
    atk_md0700_fsmc_write_dat(0x00);
    atk_md0700_fsmc_write_cmd(0xB8);
    atk_md0700_fsmc_write_dat(0x03);
    atk_md0700_fsmc_write_dat(0x01);
    atk_md0700_fsmc_write_cmd(0xBA);
    atk_md0700_fsmc_write_dat(0X01);
}

/**
 * @brief       ����ATK-MD0700ģ���е�ַ
 * @param       ��
 * @retval      ��
 */
static void atk_md0700_set_column_address(uint16_t sc, uint16_t ec)
{
    switch (g_atk_md0700_sta.disp_dir)
    {
        case ATK_MD0700_LCD_DISP_DIR_0:
        case ATK_MD0700_LCD_DISP_DIR_180:
        {
            atk_md0700_fsmc_write_cmd(0x2B);
            break;
        }
        case ATK_MD0700_LCD_DISP_DIR_90:
        case ATK_MD0700_LCD_DISP_DIR_270:
        {
            atk_md0700_fsmc_write_cmd(0x2A);
            break;
        }
    }
    
    atk_md0700_fsmc_write_dat((uint8_t)(sc >> 8) & 0xFF);
    atk_md0700_fsmc_write_dat((uint8_t)sc & 0xFF);
    atk_md0700_fsmc_write_dat((uint8_t)(ec >> 8) & 0xFF);
    atk_md0700_fsmc_write_dat((uint8_t)ec & 0xFF);
}

/**
 * @brief       ����ATK-MD0700ģ��ҳ��ַ
 * @param       ��
 * @retval      ��
 */
static void atk_md0700_set_page_address(uint16_t sp, uint16_t ep)
{
    switch (g_atk_md0700_sta.disp_dir)
    {
        case ATK_MD0700_LCD_DISP_DIR_0:
        case ATK_MD0700_LCD_DISP_DIR_180:
        {
            atk_md0700_fsmc_write_cmd(0x2A);
            break;
        }
        case ATK_MD0700_LCD_DISP_DIR_90:
        case ATK_MD0700_LCD_DISP_DIR_270:
        {
            atk_md0700_fsmc_write_cmd(0x2B);
            break;
        }
    }
    
    atk_md0700_fsmc_write_dat((uint8_t)(sp >> 8) & 0xFF);
    atk_md0700_fsmc_write_dat((uint8_t)sp & 0xFF);
    atk_md0700_fsmc_write_dat((uint8_t)(ep >> 8) & 0xFF);
    atk_md0700_fsmc_write_dat((uint8_t)ep & 0xFF);
}

/**
 * @brief       ��ʼдATK-MD0700ģ���Դ�
 * @param       ��
 * @retval      ��
 */
static void atk_md0700_start_write_memory(void)
{
    atk_md0700_fsmc_write_cmd(0x2C);
}

/**
 * @brief       ��ʼ��ATK-MD0700ģ���Դ�
 * @param       ��
 * @retval      ��
 */
static void atk_md0700_start_read_memory(void)
{
    atk_md0700_fsmc_write_cmd(0x2E);
}

/**
 * @brief       ƽ��������x^y
 * @param       x: ����
 *              y: ָ��
 * @retval      x^y
 */
static uint32_t atk_md0700_pow(uint8_t x, uint8_t y)
{
    uint8_t loop;
    uint32_t res = 1;
    
    for (loop=0; loop<y; loop++)
    {
        res *= x;
    }
    
    return res;
}

/**
 * @brief       ATK-MD0700ģ���ʼ��
 * @param       ��
 * @retval      ATK_MD0700_EOK  : ATK_MD0700ģ���ʼ���ɹ�
 *              ATK_MD0700_ERROR: ATK_MD0700ģ���ʼ��ʧ��
 */
uint8_t atk_md0700_init(void)
{
    uint8_t pid;
    
    atk_md0700_fsmc_init();             /* ATK-MD0700ģ��FSMC�ӿڳ�ʼ�� */
    pid = atk_md0700_get_pid();         /* ��ȡATK-MD0700ģ��LCD������PID */
    if (pid != ATK_MD0700_PID)
    {
        return ATK_MD0700_ERROR;
    }
    else
    {
        g_atk_md0700_sta.pid = pid;
        g_atk_md0700_sta.width = ATK_MD0700_LCD_WIDTH;
        g_atk_md0700_sta.height = ATK_MD0700_LCD_HEIGHT;
    }
    atk_md0700_reg_init();
    atk_md0700_backlight_config(UINT8_MAX);
	atk_md0700_set_disp_dir(ATK_MD0700_LCD_DISP_DIR_0);
	atk_md0700_clear(ATK_MD0700_WHITE);
    atk_md0700_display_on();
#if (ATK_MD0700_USING_TOUCH != 0)
    atk_md0700_touch_init();
#endif
    
    return ATK_MD0700_EOK;
}

/**
 * @brief       ��ȡATK-MD0700ģ��LCD���
 * @param       ��
 * @retval      ATK-MD0700ģ��LCD���
 */
uint16_t atk_md0700_get_lcd_width(void)
{
    return g_atk_md0700_sta.width;
}

/**
 * @brief       ��ȡATK-MD0700ģ��LCD�߶�
 * @param       ��
 * @retval      ATK-MD0700ģ��LCD�߶�
 */
uint16_t atk_md0700_get_lcd_height(void)
{
    return g_atk_md0700_sta.height;
}

/**
 * @brief       ����ATK-MD0700ģ��LCD��������
 * @param       pwm: LCD�������ȣ�0�����255������
 * @retval      ��
 */
void atk_md0700_backlight_config(uint8_t pwm)
{
    atk_md0700_fsmc_write_cmd(0xBE);
    atk_md0700_fsmc_write_cmd(0x05);
    atk_md0700_fsmc_write_dat(pwm);
    atk_md0700_fsmc_write_dat(0x01);
    atk_md0700_fsmc_write_dat(0xFF);
    atk_md0700_fsmc_write_dat(0x00);
    atk_md0700_fsmc_write_dat(0x00);
}

/**
 * @brief       ����ATK-MD0700ģ��LCD��ʾ
 * @param       ��
 * @retval      ��
 */
void atk_md0700_display_on(void)
{
    atk_md0700_fsmc_write_cmd(0x29);
}

/**
 * @brief       �ر�ATK-MD0700ģ��LCD��ʾ
 * @param       ��
 * @retval      ��
 */
void atk_md0700_display_off(void)
{
    atk_md0700_fsmc_write_cmd(0x28);
}

/**
 * @brief       ����ATK-MD0700ģ��LCDɨ�跽��
 * @param       scan_dir: ATK_MD0700_LCD_SCAN_DIR_L2R_U2D: �����ң����ϵ���
 *                        ATK_MD0700_LCD_SCAN_DIR_L2R_D2U: �����ң����µ���
 *                        ATK_MD0700_LCD_SCAN_DIR_R2L_U2D: ���ҵ��󣬴��ϵ���
 *                        ATK_MD0700_LCD_SCAN_DIR_R2L_D2U: ���ҵ��󣬴��µ���
 *                        ATK_MD0700_LCD_SCAN_DIR_U2D_L2R: ���ϵ��£�������
 *                        ATK_MD0700_LCD_SCAN_DIR_U2D_R2L: ���ϵ��£����ҵ���
 *                        ATK_MD0700_LCD_SCAN_DIR_D2U_L2R: ���µ��ϣ�������
 *                        ATK_MD0700_LCD_SCAN_DIR_D2U_R2L: ���µ��ϣ����ҵ���
 * @retval      ATK_MD0700_EOK   : ����ATK-MD0700ģ��LCDɨ�跽��ɹ�
 *              ATK_MD0700_ERROR : ����ATK-MD0700ģ��LCDɨ�跽��ʧ��
*               ATK_MD0700_EINVAL: �����������
 */
uint8_t atk_md0700_set_scan_dir(atk_md0700_lcd_scan_dir_t scan_dir)
{
    uint16_t reg36;
    
    switch (g_atk_md0700_sta.disp_dir)
    {
        case ATK_MD0700_LCD_DISP_DIR_0:
        {
            switch (scan_dir)
            {
                case ATK_MD0700_LCD_SCAN_DIR_L2R_U2D:
                {
                    reg36 = ATK_MD0700_LCD_SCAN_DIR_U2D_L2R;
                    break;
                }
                case ATK_MD0700_LCD_SCAN_DIR_L2R_D2U:
                {
                    reg36 = ATK_MD0700_LCD_SCAN_DIR_U2D_R2L;
                    break;
                }
                case ATK_MD0700_LCD_SCAN_DIR_R2L_U2D:
                {
                    reg36 = ATK_MD0700_LCD_SCAN_DIR_D2U_L2R;
                    break;
                }
                case ATK_MD0700_LCD_SCAN_DIR_R2L_D2U:
                {
                    reg36 = ATK_MD0700_LCD_SCAN_DIR_D2U_R2L;
                    break;
                }
                case ATK_MD0700_LCD_SCAN_DIR_U2D_L2R:
                {
                    reg36 = ATK_MD0700_LCD_SCAN_DIR_L2R_U2D;
                    break;
                }
                case ATK_MD0700_LCD_SCAN_DIR_U2D_R2L:
                {
                    reg36 = ATK_MD0700_LCD_SCAN_DIR_L2R_D2U;
                    break;
                }
                case ATK_MD0700_LCD_SCAN_DIR_D2U_L2R:
                {
                    reg36 = ATK_MD0700_LCD_SCAN_DIR_R2L_U2D;
                    break;
                }
                case ATK_MD0700_LCD_SCAN_DIR_D2U_R2L:
                {
                    reg36 = ATK_MD0700_LCD_SCAN_DIR_R2L_D2U;
                    break;
                }
                default:
                {
                    return ATK_MD0700_EINVAL;
                }
            }
            reg36 |= 0x01;
            break;
        }
        case ATK_MD0700_LCD_DISP_DIR_90:
        {
            reg36 = scan_dir;
            break;
        }
        case ATK_MD0700_LCD_DISP_DIR_180:
        {
            switch (scan_dir)
            {
                case ATK_MD0700_LCD_SCAN_DIR_L2R_U2D:
                {
                    reg36 = ATK_MD0700_LCD_SCAN_DIR_U2D_L2R;
                    break;
                }
                case ATK_MD0700_LCD_SCAN_DIR_L2R_D2U:
                {
                    reg36 = ATK_MD0700_LCD_SCAN_DIR_U2D_R2L;
                    break;
                }
                case ATK_MD0700_LCD_SCAN_DIR_R2L_U2D:
                {
                    reg36 = ATK_MD0700_LCD_SCAN_DIR_D2U_L2R;
                    break;
                }
                case ATK_MD0700_LCD_SCAN_DIR_R2L_D2U:
                {
                    reg36 = ATK_MD0700_LCD_SCAN_DIR_D2U_R2L;
                    break;
                }
                case ATK_MD0700_LCD_SCAN_DIR_U2D_L2R:
                {
                    reg36 = ATK_MD0700_LCD_SCAN_DIR_L2R_U2D;
                    break;
                }
                case ATK_MD0700_LCD_SCAN_DIR_U2D_R2L:
                {
                    reg36 = ATK_MD0700_LCD_SCAN_DIR_L2R_D2U;
                    break;
                }
                case ATK_MD0700_LCD_SCAN_DIR_D2U_L2R:
                {
                    reg36 = ATK_MD0700_LCD_SCAN_DIR_R2L_U2D;
                    break;
                }
                case ATK_MD0700_LCD_SCAN_DIR_D2U_R2L:
                {
                    reg36 = ATK_MD0700_LCD_SCAN_DIR_R2L_D2U;
                    break;
                }
                default:
                {
                    return ATK_MD0700_EINVAL;
                }
            }
            reg36 |= 0x02;
            break;
        }
        case ATK_MD0700_LCD_DISP_DIR_270:
        {
            reg36 = scan_dir;
            reg36 |= 0x03;
            break;
        }
        default:
        {
            return ATK_MD0700_ERROR;
        }
    }
    
    g_atk_md0700_sta.scan_dir = (atk_md0700_lcd_scan_dir_t)reg36;
    atk_md0700_fsmc_write_reg(0x36, reg36);
    atk_md0700_set_column_address(0, g_atk_md0700_sta.width - 1);
    atk_md0700_set_page_address(0, g_atk_md0700_sta.height - 1);
    
    return ATK_MD0700_EOK;
}

/**
 * @brief       ����ATK-MD0700ģ��LCD��ʾ����
 * @param       disp_dir: ATK_MD0700_LCD_DISP_DIR_0  : LCD˳ʱ����ת0����ʾ����
 *                        ATK_MD0700_LCD_DISP_DIR_90 : LCD˳ʱ����ת90����ʾ����
 *                        ATK_MD0700_LCD_DISP_DIR_180: LCD˳ʱ����ת180����ʾ����
 *                        ATK_MD0700_LCD_DISP_DIR_270: LCD˳ʱ����ת270����ʾ����
 * @retval      ATK_MD0700_EOK   : ����ATK-MD0700ģ��LCD��ʾ����ɹ�
 *              ATK_MD0700_EINVAL: �����������
 */
uint8_t atk_md0700_set_disp_dir(atk_md0700_lcd_disp_dir_t disp_dir)
{
    switch (disp_dir)
    {
        case ATK_MD0700_LCD_DISP_DIR_0:
        {
            g_atk_md0700_sta.width = ATK_MD0700_LCD_WIDTH;
            g_atk_md0700_sta.height = ATK_MD0700_LCD_HEIGHT;
            break;
        }
        case ATK_MD0700_LCD_DISP_DIR_90:
        {
            g_atk_md0700_sta.width = ATK_MD0700_LCD_HEIGHT;
            g_atk_md0700_sta.height = ATK_MD0700_LCD_WIDTH;
            break;
        }
        case ATK_MD0700_LCD_DISP_DIR_180:
        {
            g_atk_md0700_sta.width = ATK_MD0700_LCD_WIDTH;
            g_atk_md0700_sta.height = ATK_MD0700_LCD_HEIGHT;
            break;
        }
        case ATK_MD0700_LCD_DISP_DIR_270:
        {
            g_atk_md0700_sta.width = ATK_MD0700_LCD_HEIGHT;
            g_atk_md0700_sta.height = ATK_MD0700_LCD_WIDTH;
            break;
        }
        default:
        {
            return ATK_MD0700_EINVAL;
        }
    }
    
    g_atk_md0700_sta.disp_dir = disp_dir;
    atk_md0700_set_scan_dir(ATK_MD0700_LCD_SCAN_DIR_L2R_U2D);
    
    return ATK_MD0700_EOK;
}

/**
 * @brief       ��ȡATK-MD0700ģ��LCDɨ�跽��
 * @param       ��
 * @retval      ATK-MD0700ģ��LCDɨ�跽��
 */
atk_md0700_lcd_scan_dir_t atk_md0700_get_scan_dir(void)
{
    return g_atk_md0700_sta.scan_dir;
}

/**
 * @brief       ��ȡATK-MD0700ģ��LCD��ʾ����
 * @param       ��
 * @retval      ATK-MD0700ģ��LCD��ʾ����
 */
atk_md0700_lcd_disp_dir_t atk_md0700_get_disp_dir(void)
{
    return g_atk_md0700_sta.disp_dir;
}

/**
 * @brief       ATK-MD0700ģ��LCD�������
 * @param       xs   : ������ʼX����
 *              ys   : ������ʼY����
 *              xe   : ������ֹX����
 *              ye   : ������ֹY����
 *              color: ���������ɫ
 * @retval      ��
 */
void atk_md0700_fill(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t* color, atk_md0700_lcd_fill_mode_t fill_mode)
{
    uint16_t x_index;
    uint16_t y_index;
    
    atk_md0700_set_column_address(xs, xe);
    atk_md0700_set_page_address(ys, ye);
    atk_md0700_start_write_memory();
    for (y_index=ys; y_index<=ye; y_index++)
    {
        for (x_index=xs; x_index<= xe; x_index++)
        {
            atk_md0700_fsmc_write_dat(*color);
			if (fill_mode == MULTI_COLOR_BLOCK) {
				color++;
			}
        }
    }
}

/**
 * @brief       ATK-MD0700ģ��LCD����
 * @param       color: ������ɫ
 * @retval      ��
 */
void atk_md0700_clear(uint16_t color)
{
    atk_md0700_fill(0, 0, g_atk_md0700_sta.width - 1, g_atk_md0700_sta.height - 1, &color, SINGLE_COLOR_BLOCK);
}

/**
 * @brief       ATK-MD0700ģ��LCD����
 * @param       x    : �������X����
 *              y    : �������Y����
 *              color: ���������ɫ
 * @retval      ��
 */
void atk_md0700_draw_point(uint16_t x, uint16_t y, uint16_t color)
{
    atk_md0700_set_column_address(x, x);
    atk_md0700_set_page_address(y, y);
    atk_md0700_start_write_memory();
    atk_md0700_fsmc_write_dat(color);
}

/**
 * @brief       ATK-MD0700ģ��LCD����
 * @param       x    : �������X����
 *              y    : �������Y����
 * @retval      ���������ɫ
 */
uint16_t atk_md0700_read_point(uint16_t x, uint16_t y)
{
    uint16_t color;
    
    if ((x >= g_atk_md0700_sta.width) || (y >= g_atk_md0700_sta.height))
    {
        return ATK_MD0700_EINVAL;
    }
    
    atk_md0700_set_column_address(x, x);
    atk_md0700_set_page_address(y, y);
    atk_md0700_start_read_memory();
    
    color = atk_md0700_fsmc_read_dat();
    
    return color;
}

/**
 * @brief       ATK-MD0700ģ��LCD���߶�
 * @param       x1   : �����߶ζ˵�1��X����
 *              y1   : �����߶ζ˵�1��Y����
 *              x2   : �����߶ζ˵�2��X����
 *              y2   : �����߶ζ˵�2��Y����
 *              color: �����߶ε���ɫ
 * @retval      ��
 */
void atk_md0700_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t x_delta;
    uint16_t y_delta;
    int16_t x_sign;
    int16_t y_sign;
    int16_t error;
    int16_t error2;
    
    x_delta = (x1 < x2) ? (x2 - x1) : (x1 - x2);
    y_delta = (y1 < y2) ? (y2 - y1) : (y1 - y2);
    x_sign = (x1 < x2) ? 1 : -1;
    y_sign = (y1 < y2) ? 1 : -1;
    error = x_delta - y_delta;
    
    atk_md0700_draw_point(x2, y2, color);
    
    while ((x1 != x2) || (y1 != y2))
    {
        atk_md0700_draw_point(x1, y1, color);
        
        error2 = error << 1;
        if (error2 > -y_delta)
        {
            error -= y_delta;
            x1 += x_sign;
        }
        
        if (error2 < x_delta)
        {
            error += x_delta;
            y1 += y_sign;
        }
    }
}

/**
 * @brief       ATK-MD0700ģ��LCD�����ο�
 * @param       x1   : �������ο�˵�1��X����
 *              y1   : �������ο�˵�1��Y����
 *              x2   : �������ο�˵�2��X����
 *              y2   : �������ο�˵�2��Y����
 *              color: �������ο����ɫ
 * @retval      ��
 */
void atk_md0700_draw_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    atk_md0700_draw_line(x1, y1, x2, y1, color);
    atk_md0700_draw_line(x1, y2, x2, y2, color);
    atk_md0700_draw_line(x1, y1, x1, y2, color);
    atk_md0700_draw_line(x2, y1, x2, y2, color);
}

/**
 * @brief       ATK-MD0700ģ��LCD��Բ�ο�
 * @param       x    : ����Բ�ο�ԭ���X����
 *              y    : ����Բ�ο�ԭ���Y����
 *              r    : ����Բ�ο�İ뾶
 *              color: ����Բ�ο����ɫ
 * @retval      ��
 */
void atk_md0700_draw_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color)
{
    int32_t x_t;
    int32_t y_t;
    int32_t error;
    int32_t error2;
    
    x_t = -r;
    y_t = 0;
    error = 2 - 2 * r;
    
    do {
        atk_md0700_draw_point(x - x_t, y + y_t, color);
        atk_md0700_draw_point(x + x_t, y + y_t, color);
        atk_md0700_draw_point(x + x_t, y - y_t, color);
        atk_md0700_draw_point(x - x_t, y - y_t, color);
        
        error2 = error;
        if (error2 <= y_t)
        {
            y_t++;
            error = error + (y_t * 2 + 1);
            if ((-x_t == y_t) && (error2 <= x_t))
            {
                error2 = 0;
            }
        }
        
        if (error2 > x_t)
        {
            x_t++;
            error = error + (x_t * 2 + 1);
        }
    } while (x_t <= 0);
}

/**
 * @brief       ATK-MD0700ģ��LCD��ʾ1���ַ�
 * @param       x    : ����ʾ�ַ���X����
 *              y    : ����ʾ�ַ���Y����
 *              ch   : ����ʾ�ַ�
 *              font : ����ʾ�ַ�������
 *              color: ����ʾ�ַ�����ɫ
 * @retval      ��
 */
void atk_md0700_show_char(uint16_t x, uint16_t y, char ch, atk_md0700_lcd_font_t font, uint16_t color)
{
    const uint8_t *ch_code;
    uint8_t ch_width;
    uint8_t ch_height;
    uint8_t ch_size;
    uint8_t ch_offset;
    uint8_t byte_index;
    uint8_t byte_code;
    uint8_t bit_index;
    uint8_t width_index = 0;
    uint8_t height_index = 0;
    
    ch_offset = ch - ' ';
    
    switch (font)
    {
#if (ATK_MD0700_FONT_12 != 0)
        case ATK_MD0700_LCD_FONT_12:
        {
            ch_code = atk_md0700_font_1206[ch_offset];
            ch_width = ATK_MD0700_FONT_12_CHAR_WIDTH;
            ch_height = ATK_MD0700_FONT_12_CHAR_HEIGHT;
            ch_size = ATK_MD0700_FONT_12_CHAR_SIZE;
            break;
        }
#endif
#if (ATK_MD0700_FONT_16 != 0)
        case ATK_MD0700_LCD_FONT_16:
        {
            ch_code = atk_md0700_font_1608[ch_offset];
            ch_width = ATK_MD0700_FONT_16_CHAR_WIDTH;
            ch_height = ATK_MD0700_FONT_16_CHAR_HEIGHT;
            ch_size = ATK_MD0700_FONT_16_CHAR_SIZE;
            break;
        }
#endif
#if (ATK_MD0700_FONT_24 != 0)
        case ATK_MD0700_LCD_FONT_24:
        {
            ch_code = atk_md0700_font_2412[ch_offset];
            ch_width = ATK_MD0700_FONT_24_CHAR_WIDTH;
            ch_height = ATK_MD0700_FONT_24_CHAR_HEIGHT;
            ch_size = ATK_MD0700_FONT_24_CHAR_SIZE;
            break;
        }
#endif
#if (ATK_MD0700_FONT_32 != 0)
        case ATK_MD0700_LCD_FONT_32:
        {
            ch_code = atk_md0700_font_3216[ch_offset];
            ch_width = ATK_MD0700_FONT_32_CHAR_WIDTH;
            ch_height = ATK_MD0700_FONT_32_CHAR_HEIGHT;
            ch_size = ATK_MD0700_FONT_32_CHAR_SIZE;
            break;
        }
#endif
        default:
        {
            return;
        }
    }
    
    if ((x + ch_width > ATK_MD0700_LCD_WIDTH) || (y + ch_height > ATK_MD0700_LCD_HEIGHT))
    {
        return;
    }
    
    for (byte_index=0; byte_index<ch_size; byte_index++)
    {
        byte_code = ch_code[byte_index];
        for (bit_index=0; bit_index<8; bit_index++)
        {
            if ((byte_code & 0x80) != 0)
            {
                atk_md0700_draw_point(x + width_index, y + height_index, color);
            }
            width_index++;
            if (width_index == ch_width)
            {
                width_index = 0;
                height_index++;
                break;
            }
            byte_code <<= 1;
        }
    }
}

/**
 * @brief       ATK-MD0700ģ��LCD��ʾ�ַ���
 * @note        ���Զ����кͻ�ҳ
 * @param       x     : ����ʾ�ַ�����X����
 *              y     : ����ʾ�ַ�����Y����
 *              width : ����ʾ�ַ�������ʾ�߶�
 *              height: ����ʾ�ַ�������ʾ���
 *              str   : ����ʾ�ַ���
 *              font  : ����ʾ�ַ���������
 *              color : ����ʾ�ַ�������ɫ
 * @retval      ��
 */
void atk_md0700_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *str, atk_md0700_lcd_font_t font, uint16_t color)
{
    uint8_t ch_width;
    uint8_t ch_height;
    uint16_t x_raw;
    uint16_t y_raw;
    uint16_t x_limit;
    uint16_t y_limit;
    
    switch (font)
    {
#if (ATK_MD0700_FONT_12 != 0)
        case ATK_MD0700_LCD_FONT_12:
        {
            ch_width = ATK_MD0700_FONT_12_CHAR_WIDTH;
            ch_height = ATK_MD0700_FONT_12_CHAR_HEIGHT;
            break;
        }
#endif
#if (ATK_MD0700_FONT_16 != 0)
        case ATK_MD0700_LCD_FONT_16:
        {
            ch_width = ATK_MD0700_FONT_16_CHAR_WIDTH;
            ch_height = ATK_MD0700_FONT_16_CHAR_HEIGHT;
            break;
        }
#endif
#if (ATK_MD0700_FONT_24 != 0)
        case ATK_MD0700_LCD_FONT_24:
        {
            ch_width = ATK_MD0700_FONT_24_CHAR_WIDTH;
            ch_height = ATK_MD0700_FONT_24_CHAR_HEIGHT;
            break;
        }
#endif
#if (ATK_MD0700_FONT_32 != 0)
        case ATK_MD0700_LCD_FONT_32:
        {
            ch_width = ATK_MD0700_FONT_32_CHAR_WIDTH;
            ch_height = ATK_MD0700_FONT_32_CHAR_HEIGHT;
            break;
        }
#endif
        default:
        {
            return;
        }
    }
    
    x_raw = x;
    y_raw = y;
    x_limit = ((x + width + 1) > ATK_MD0700_LCD_WIDTH) ? ATK_MD0700_LCD_WIDTH : (x + width + 1);
    y_limit = ((y + height + 1) > ATK_MD0700_LCD_HEIGHT) ? ATK_MD0700_LCD_HEIGHT : (y + height + 1);
    
    while ((*str >= ' ') && (*str <= '~'))
    {
        if (x + ch_width >= x_limit)
        {
            x = x_raw;
            y += ch_height;
        }
        
        if (y + ch_height >= y_limit)
        {
            y = x_raw;
            x = y_raw;
        }
        
        atk_md0700_show_char(x, y, *str, font, color);
        
        x += ch_width;
        str++;
    }
}

/**
 * @brief       ATK-MD0700ģ��LCD��ʾ���֣��ɿ�����ʾ��λ0
 * @param       x    : ����ʾ���ֵ�X����
 *              y    : ����ʾ���ֵ�Y����
 *              num  : ����ʾ����
 *              len  : ����ʾ���ֵ�λ��
 *              mode : ATK_MD0700_NUM_SHOW_NOZERO: ���ָ�λ0����ʾ
 *                     ATK_MD0700_NUM_SHOW_ZERO  : ���ָ�λ0��ʾ
 *              font : ����ʾ���ֵ�����
 *              color: ����ʾ���ֵ���ɫ
 * @retval      ��
 */
void atk_md0700_show_xnum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, atk_md0700_num_mode_t mode, atk_md0700_lcd_font_t font, uint16_t color)
{
    uint8_t ch_width;
    uint8_t len_index;
    uint8_t num_index;
    uint8_t first_nozero = 0;
    char pad;
    
    switch (font)
    {
#if (ATK_MD0700_FONT_12 != 0)
        case ATK_MD0700_LCD_FONT_12:
        {
            ch_width = ATK_MD0700_FONT_12_CHAR_WIDTH;
            break;
        }
#endif
#if (ATK_MD0700_FONT_16 != 0)
        case ATK_MD0700_LCD_FONT_16:
        {
            ch_width = ATK_MD0700_FONT_16_CHAR_WIDTH;
            break;
        }
#endif
#if (ATK_MD0700_FONT_24 != 0)
        case ATK_MD0700_LCD_FONT_24:
        {
            ch_width = ATK_MD0700_FONT_24_CHAR_WIDTH;
            break;
        }
#endif
#if (ATK_MD0700_FONT_32 != 0)
        case ATK_MD0700_LCD_FONT_32:
        {
            ch_width = ATK_MD0700_FONT_32_CHAR_WIDTH;
            break;
        }
#endif
        default:
        {
            return;
        }
    }
    
    switch (mode)
    {
        case ATK_MD0700_NUM_SHOW_NOZERO:
        {
            pad = ' ';
            break;
        }
        case ATK_MD0700_NUM_SHOW_ZERO:
        {
            pad = '0';
            break;
        }
        default:
        {
            return;
        }
    }
    
    for (len_index=0; len_index<len; len_index++)
    {
        num_index = (num / atk_md0700_pow(10, len - len_index - 1)) % 10;
        if ((first_nozero == 0) && (len_index < (len - 1)))
        {
            if (num_index == 0)
            {
                atk_md0700_show_char(x + ch_width * len_index, y, pad, font, color);
                continue;
            }
            else
            {
                first_nozero = 1;
            }
        }
        
        atk_md0700_show_char(x + ch_width * len_index, y, num_index + '0', font, color);
    }
}

/**
 * @brief       ATK-MD0700ģ��LCD��ʾ���֣�����ʾ��λ0
 * @param       x    : ����ʾ���ֵ�X����
 *              y    : ����ʾ���ֵ�Y����
 *              num  : ����ʾ����
 *              len  : ����ʾ���ֵ�λ��
 *              font : ����ʾ���ֵ�����
 *              color: ����ʾ���ֵ���ɫ
 * @retval      ��
 */
void atk_md0700_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len, atk_md0700_lcd_font_t font, uint16_t color)
{
    atk_md0700_show_xnum(x, y, num, len, ATK_MD0700_NUM_SHOW_NOZERO, font, color);
}

/**
 * @brief       ATK-MD0700ģ��LCDͼƬ
 * @note        ͼƬȡģ��ʽ: ˮƽɨ�衢RGB565����λ��ǰ
 * @param       x     : ����ʾͼƬ��X����
 *              y     : ����ʾͼƬ��Y����
 *              width : ����ʾͼƬ�Ŀ��
 *              height: ����ʾͼƬ�ĸ߶�
 *              pic   : ����ʾͼƬ�����׵�ַ
 * @retval      ��
 */
void atk_md0700_show_pic(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *pic)
{
    uint16_t x_index;
    uint16_t y_index;
    
    if ((x + width > ATK_MD0700_LCD_WIDTH) || (y + height > ATK_MD0700_LCD_HEIGHT))
    {
        return;
    }
    
    atk_md0700_set_column_address(x, x + width - 1);
    atk_md0700_set_page_address(y, y + height - 1);
    atk_md0700_start_write_memory();
    for (y_index=y; y_index<=(y + height); y_index++)
    {
        for (x_index=x; x_index<=(x + width); x_index++)
        {
            atk_md0700_fsmc_write_dat(*pic);
            pic++;
        }
    }
}
