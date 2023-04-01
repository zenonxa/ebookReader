#include "BSP/EXTI/exti.h"
#include "SYSTEM/delay/delay.h"
#include "BSP/KEY/key.h"
//#include "BSY/LED/led.h"
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

// �ⲿ�жϳ�ʼ��
void EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOE_CLK_ENABLE();  // ����GPIOEʱ��

    GPIO_Initure.Pin  = GPIO_PIN_3;            // PE2
    GPIO_Initure.Mode = GPIO_MODE_IT_FALLING;  // �½��ش���
    GPIO_Initure.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOE, &GPIO_Initure);

    // �ж���2-PE2
    HAL_NVIC_SetPriority(EXTI3_IRQn, 2, 2);  // ��ռ���ȼ�Ϊ2�������ȼ�Ϊ1
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);          // ʹ���ж���2
}

// �жϷ�����
void EXTI3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);  // �����жϴ����ú���
}

// �жϷ����������Ҫ��������
// ��HAL�������е��ⲿ�жϷ�����������ô˺���
// GPIO_Pin:�ж����ź�
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    delay_ms(100);  // ����
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
}
