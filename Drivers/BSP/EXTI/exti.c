#include "BSP/EXTI/exti.h"
#include "BSP/KEY/key.h"
#include "SYSTEM/delay/delay.h"
// #include "BSY/LED/led.h"
#include "font.h"
//////////////////////////////////////////////////////////////////////////////////
// 本程序只供学习使用，未经作者许可，不得用于其它任何用途
// ALIENTEK STM32F103开发板
// 外部中断驱动代码
// 正点原子@ALIENTEK
// 技术论坛:www.openedv.com
// 创建日期:2019/9/17
// 版本：V1.0
// 版权所有，盗版必究。
// Copyright(C) 广州市星翼电子科技有限公司 2014-2024
// All rights reserved
//////////////////////////////////////////////////////////////////////////////////

extern uint8_t fontNameSelect;
extern uint8_t fontSizeSelect;
extern uint8_t needRerender;
extern bool    noTouchEvent;

// 外部中断初始化
void EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOE_CLK_ENABLE();  // 开启GPIOE时钟
    __HAL_RCC_GPIOF_CLK_ENABLE();  // 开启GPIOF时钟

    /* LCD屏触摸中断引脚[T_PEN] GPIOF.10 */
    GPIO_Initure.Pin  = GPIO_PIN_10;           // PF10
    GPIO_Initure.Mode = GPIO_MODE_IT_FALLING;  // 下降沿触发
    GPIO_Initure.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOF, &GPIO_Initure);

    GPIO_Initure.Pin  = GPIO_PIN_3;            // PE2
    GPIO_Initure.Mode = GPIO_MODE_IT_FALLING;  // 下降沿触发
    GPIO_Initure.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOE, &GPIO_Initure);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);  // 抢占优先级为2，子优先级为1
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);  // 使能中断线2

    // 中断线2-PE2
    HAL_NVIC_SetPriority(EXTI3_IRQn, 2, 2);  // 抢占优先级为2，子优先级为1
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);          // 使能中断线2
}

// 中断服务函数
void EXTI3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);  // 调用中断处理公用函数
}

void EXTI15_10_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_10) != RESET){
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);  // 调用中断处理公用函数
        //HAL_EXTI_ClearPendingBit(EXTI_LINE_10);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_10);
    }
}

// 中断服务程序中需要做的事情
// 在HAL库中所有的外部中断服务函数都会调用此函数
// GPIO_Pin:中断引脚号
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin) {
        case GPIO_PIN_3:
            // delay_ms(100);  // 消抖
            // LED1翻转
            if (KEY1 == 0) {
                fontSizeSelect++;
                if (fontSizeSelect >= Font_Size_Cnt) {
                    fontSizeSelect = Font_Size_Min;
                    fontNameSelect++;
                    if (fontNameSelect >= Font_Name_Cnt) {
                        fontNameSelect = Font_Name_Min;
                    }
                }
                needRerender = 1;
                // LED1 = !LED1;
            }
            break;
        case GPIO_PIN_10: noTouchEvent = false; break;
        default: break; ;
    }
}
