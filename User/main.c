#include "SYSTEM/sys/sys.h"
#include "SYSTEM/delay/delay.h"
#include "SYSTEM/usart/usart.h"
#include "BSP/LED/led.h"
#include "BSP/KEY/key.h"
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

int main(void)
{
	unsigned char i = 0;
    HAL_Init();                         /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* 设置时钟, 72Mhz */
    delay_init(72);                     /* 延时初始化 */
    usart_init(115200);                 /* 串口初始化为115200 */
    led_init();                         /* 初始化LED */
    key_init();                         /* 初始化按键 */
    show_mesg();                        /* 显示实验信息 */
	test_atk_md0700();
	while(1) {
		printf("Running normally...\r\n");
		if (i%2 == 0) {
			LED0(0);
			LED1(1);
			++i;
		} else {
			LED0(1);
			LED1(0);
			i = 0;
		}
		delay_ms(1000);
	}
}
