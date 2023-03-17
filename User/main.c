#include "SYSTEM/sys/sys.h"
#include "SYSTEM/delay/delay.h"
#include "SYSTEM/usart/usart.h"
#include "BSP/LED/led.h"
#include "BSP/KEY/key.h"
#include "BSP/TIMER/timer.h"
#include "BSP/SRAM/sram.h"
#include "lvgl.h"
#include "examples/lv_examples.h"
#include "examples/porting/lv_port_disp.h"
#include "examples/porting/lv_port_indev.h"
#include "Test/test_atk_md0700.h"
#include <stm32f1xx.h>

/**
 * @brief       ��ʾʵ����Ϣ
 * @param       ��
 * @retval      ��
 */
void show_mesg(void)
{
    /* �������ʵ����Ϣ */
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
    HAL_Init();                         /* ��ʼ��HAL�� */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* ����ʱ��, 72Mhz */
    delay_init(72);                     /* ��ʱ��ʼ�� */
    usart_init(115200);                 /* ���ڳ�ʼ��Ϊ115200 */
    led_init();                         /* ��ʼ��LED */
    key_init();                         /* ��ʼ������ */
    show_mesg();                        /* ��ʾʵ����Ϣ */
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
	
#if 0
	lv_example_tileview_1();
#else
	lv_example_dropdown_1();
#endif
	
	
	while (1) {
		delay_ms(5);
		lv_timer_handler();
	}
#endif
}
