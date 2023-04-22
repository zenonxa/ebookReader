#include "BSP/EXTI/exti.h"
#include "BSP/KEY/key.h"
#include "SYSTEM/delay/delay.h"
// #include "BSY/LED/led.h"
#include "font.h"
//////////////////////////////////////////////////////////////////////////////////
// ������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
// ALIENTEK STM32F103������
// �ⲿ�ж���������
// ����ԭ��@ALIENTEK
// ������̳:www.openedv.com
// ��������:2019/9/17
// �汾��V1.0
// ��Ȩ���У�����ؾ���
// Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
// All rights reserved
//////////////////////////////////////////////////////////////////////////////////

extern uint8_t fontNameSelect;
extern uint8_t fontSizeSelect;
extern uint8_t needRerender;
extern bool    noTouchEvent;

// �ⲿ�жϳ�ʼ��
void EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOE_CLK_ENABLE();  // ����GPIOEʱ��
    __HAL_RCC_GPIOF_CLK_ENABLE();  // ����GPIOFʱ��

    /* LCD�������ж�����[T_PEN] GPIOF.10 */
    GPIO_Initure.Pin  = GPIO_PIN_10;           // PF10
    GPIO_Initure.Mode = GPIO_MODE_IT_FALLING;  // �½��ش���
    GPIO_Initure.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOF, &GPIO_Initure);

    GPIO_Initure.Pin  = GPIO_PIN_3;            // PE2
    GPIO_Initure.Mode = GPIO_MODE_IT_FALLING;  // �½��ش���
    GPIO_Initure.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOE, &GPIO_Initure);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);  // ��ռ���ȼ�Ϊ2�������ȼ�Ϊ1
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);  // ʹ���ж���2

    // �ж���2-PE2
    HAL_NVIC_SetPriority(EXTI3_IRQn, 2, 2);  // ��ռ���ȼ�Ϊ2�������ȼ�Ϊ1
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);          // ʹ���ж���2
}

// �жϷ�����
void EXTI3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);  // �����жϴ����ú���
}

void EXTI15_10_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_10) != RESET){
        HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);  // �����жϴ����ú���
        //HAL_EXTI_ClearPendingBit(EXTI_LINE_10);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_10);
    }
}

// �жϷ����������Ҫ��������
// ��HAL�������е��ⲿ�жϷ�����������ô˺���
// GPIO_Pin:�ж����ź�
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin) {
        case GPIO_PIN_3:
            // delay_ms(100);  // ����
            // LED1��ת
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
