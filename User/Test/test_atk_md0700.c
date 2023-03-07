#include "BSP/ATK_MD0700/atk_md0700.h"
#include "test_atk_md0700.h"
#include "SYSTEM/usart/usart.h"

void test_atk_md0700(void) {
	uint8_t ret;
	
    /* 初始化ATK-MD0700模块 */
    ret = atk_md0700_init();

    if (ret != 0) {
        printf("ATK-MD0700 init failed!\r\n");
    } 
    /* ATK-MD0700模块LCD清屏 */
    atk_md0700_clear(ATK_MD0700_WHITE);
	
	/* ATK-MD0700模块LCD显示字符串 */
	atk_md0700_show_string(10, 10, ATK_MD0700_LCD_WIDTH, 32, "STM32", ATK_MD0700_LCD_FONT_32, ATK_MD0700_RED);
    atk_md0700_show_string(10, 42, ATK_MD0700_LCD_WIDTH, 24, "ATK-MD0700", ATK_MD0700_LCD_FONT_24, ATK_MD0700_RED);
    //atk_md0700_show_string(10, 66, ATK_MD0700_LCD_WIDTH, 16, "Test program.", ATK_MD0700_LCD_FONT_16, ATK_MD0700_RED);
	//atk_md0700_show_string(10, 78, ATK_MD0700_LCD_WIDTH, 12, "Hello world!.", ATK_MD0700_LCD_FONT_12, ATK_MD0700_RED);
}
