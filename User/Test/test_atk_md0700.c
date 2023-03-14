#include "BSP/ATK_MD0700/atk_md0700.h"
#include "test_atk_md0700.h"
#include "SYSTEM/usart/usart.h"
#include "stdio.h"

void test_atk_md0700(void) {
	uint8_t ret;
	uint16_t x1, x2;
	uint16_t y1, y2;
	char buffer[100];
	uint16_t color;
	
    /* 初始化ATK-MD0700模块 */
    ret = atk_md0700_init();

    if (ret != 0) {
        printf("ATK-MD0700 init failed!\r\n");
    } 
    /* ATK-MD0700模块LCD清屏 */
    atk_md0700_clear(ATK_MD0700_WHITE);
	
	x1 = x2 = 30;
	y1 = y2 = 10;
	/* ATK-MD0700模块LCD显示字符串 */
	atk_md0700_show_string(x1, y1, ATK_MD0700_LCD_WIDTH, 32, "TFT-LCD Module: ATK-MD0700", ATK_MD0700_LCD_FONT_32, ATK_MD0700_RED);
	
	/* 获取屏幕宽高 */
	y1 += 32;
	sprintf(buffer, "Panel width: %d", atk_md0700_get_lcd_width());
    atk_md0700_show_string(x1, y1, ATK_MD0700_LCD_WIDTH, 24, buffer, ATK_MD0700_LCD_FONT_24, ATK_MD0700_BLUE);
	y1 += 24;
	sprintf(buffer, "Panel height: %d", atk_md0700_get_lcd_height());
	atk_md0700_show_string(x1, y1, ATK_MD0700_LCD_WIDTH, 24, buffer, ATK_MD0700_LCD_FONT_24, ATK_MD0700_BLUE);
	
	/* 区域填充 */
	x1 = 0;
	y1 += 24;
	x2 = x1 + 160 - 1;
	y2 = y1 + 160 - 1;

	for (int i = 0; i < 3; i++) {
		switch (i) {
			case 0:
				color = ATK_MD0700_BRED;
				break;
			case 1:
				color = ATK_MD0700_GRED;
				break;
			case 2:
				color = ATK_MD0700_GBLUE;
				break;
			default:
				color = ATK_MD0700_BLACK;
				
		}
		atk_md0700_fill(x1, y1, x2, y2, &color, SINGLE_COLOR_BLOCK);
		x1 += 160 - 1;
		x2 += 160 - 1;
	}
	
	/* 画点 */
	x1 = 30;
	y1 = 300;
	for (int i = 0; i < 3; i++) {
		atk_md0700_draw_point(x1, y1, ATK_MD0700_MAGENTA);
		x1 += 10;
	}
	
	x1 = 30;
	y1 += 5;
	for (int i = 0; i < 3; i++) {
		atk_md0700_draw_point(30, 310, ATK_MD0700_GREEN);
		x1 += 10;
	}
	
	/* 画线 */
	atk_md0700_draw_line(50, 330, 150, 330, ATK_MD0700_BLACK);
	atk_md0700_draw_line(30, 330, 30, 380, ATK_MD0700_BLACK);
	atk_md0700_draw_line(300, 330, 400, 380, ATK_MD0700_BLACK);
	
	/* 画圆形框 */
	atk_md0700_draw_circle(240, 500, 100, ATK_MD0700_YELLOW);
	
	/* ASCII字符显示 */
	atk_md0700_show_char(30, 630, 'A', ATK_MD0700_LCD_FONT_16, ATK_MD0700_BROWN);
	atk_md0700_show_num(50, 630, 68485783, 8, ATK_MD0700_LCD_FONT_16, ATK_MD0700_BRRED);
	atk_md0700_show_xnum(50, 700, 894953, 10, ATK_MD0700_NUM_SHOW_ZERO, ATK_MD0700_LCD_FONT_16, ATK_MD0700_GRAY);
	
	
	//atk_md0700_show_pic();
	
	
    //atk_md0700_show_string(10, 66, ATK_MD0700_LCD_WIDTH, 16, "Test program.", ATK_MD0700_LCD_FONT_16, ATK_MD0700_RED);
	//atk_md0700_show_string(10, 78, ATK_MD0700_LCD_WIDTH, 12, "Hello world!.", ATK_MD0700_LCD_FONT_12, ATK_MD0700_RED);
}
