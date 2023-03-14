#include "SYSTEM/sys/sys.h"
#include "SYSTEM/delay/delay.h"
#include "SYSTEM/usart/usart.h"
#include "BSP/LED/led.h"
#include "BSP/KEY/key.h"
#include "BSP/TIMER/timer.h"
#include "BSP/SRAM/sram.h"
#include "lvgl.h"
#include "examples/porting/lv_port_disp.h"
#include "examples/porting/lv_port_indev.h"
#include "Test/test_atk_md0700.h"
#include <stm32f1xx.h>

/**
 * @brief       显示实验信息
 * @param       无
 * @retval      无
 */
void show_mesg(void)
{
    /* 串口输出实验信息 */
    printf("\n");
    printf("********************************\r\n");
    printf("STM32\r\n");
    printf("ATK-MD0700\r\n");
    printf("ATOM@ALIENTEK\r\n");
    printf("********************************\r\n");
    printf("\r\n");
}

void LED_Toggle(void) {
	static uint8_t i = 0;
	if (i%2 == 0) {
		LED0(0);
		LED1(1);
		++i;
	} else {
		LED0(1);
		LED1(0);
		--i;
	}
}

int main(void)
{
    HAL_Init();                         /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* 设置时钟, 72Mhz */
    delay_init(72);                     /* 延时初始化 */
    usart_init(115200);                 /* 串口初始化为115200 */
    led_init();                         /* 初始化LED */
    key_init();                         /* 初始化按键 */
    show_mesg();                        /* 显示实验信息 */
#if 0
	test_atk_md0700();
	while(1) {
		printf("Running normally...\r\n");
		LED_Toggle();
		delay_ms(500);
	}
#else
	SRAM_Init();
	TIM3_Init(1000-1,72-1);
	lv_init();
	lv_port_disp_init();
	lv_port_indev_init();
	
	lv_obj_t* switch_obj = lv_switch_create(lv_scr_act());
	lv_obj_set_size(switch_obj, 120, 60);
	lv_obj_align(switch_obj, LV_ALIGN_CENTER, 0, 0);
	
	while (1) {
		delay_ms(5);
		lv_timer_handler();
	}
#endif
}
