#ifndef _TIMER_H
#define _TIMER_H
#include "SYSTEM/sys/sys.h"
//////////////////////////////////////////////////////////////////////////////////
// 本程序只供学习使用，未经作者许可，不得用于其它任何用途
// ALIENTEK STM32F103开发板
// 定时器驱动代码
// 正点原子@ALIENTEK
// 技术论坛:www.openedv.com
// 创建日期:2019/9/17
// 版本：V1.0
// 版权所有，盗版必究。
// Copyright(C) 广州市星翼电子科技有限公司 2014-2024
// All rights reserved
//////////////////////////////////////////////////////////////////////////////////

#define LongPressingJudgeTime 80
#define ENABLE 1
#define DISABLE 0

/* Flags related to whether the corresponding delay operation is performed. */
extern uint8_t TimerDelay_Press;
/* The incrementing variable for the deferred operation. */
extern uint8_t PressingKeepingTime;

extern TIM_HandleTypeDef TIM3_Handler;  // 定时器句柄

void TIM3_Init(uint16_t arr, uint16_t psc);
#endif
