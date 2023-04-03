/**
 ****************************************************************************************************
 * @file        atk_md0700_touch.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-06-21
 * @brief       ATK-MD0700模块触摸驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F103开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#ifndef __ATK_MD0700_TOUCH_H
#define __ATK_MD0700_TOUCH_H

#include "./BSP/ATK_MD0700/atk_md0700.h"

#if (ATK_MD0700_USING_TOUCH != 0)

/* ATK-MD0700模块最大触摸点数量 */
#    define ATK_MD0700_TOUCH_TP_MAX 5
#    define ATK_MD0700_TOUCH_TP_ENABLE_CNT 1

/* 引脚定义 */
#    define ATK_MD0700_TOUCH_PEN_GPIO_PORT GPIOF
#    define ATK_MD0700_TOUCH_PEN_GPIO_PIN GPIO_PIN_10
#    define ATK_MD0700_TOUCH_PEN_GPIO_CLK_ENABLE()                             \
        do {                                                                   \
            __HAL_RCC_GPIOF_CLK_ENABLE();                                      \
        } while (0)
#    define ATK_MD0700_TOUCH_CS_GPIO_PORT GPIOF
#    define ATK_MD0700_TOUCH_CS_GPIO_PIN GPIO_PIN_11
#    define ATK_MD0700_TOUCH_CS_GPIO_CLK_ENABLE()                              \
        do {                                                                   \
            __HAL_RCC_GPIOF_CLK_ENABLE();                                      \
        } while (0)

/* IO操作 */
#    define ATK_MD0700_TOUCH_READ_PEN()                                        \
        HAL_GPIO_ReadPin(ATK_MD0700_TOUCH_PEN_GPIO_PORT,                       \
                         ATK_MD0700_TOUCH_PEN_GPIO_PIN)
#    define ATK_MD0700_TOUCH_PEN(x)                                            \
        do {                                                                   \
            x ? HAL_GPIO_WritePin(ATK_MD0700_TOUCH_PEN_GPIO_PORT,              \
                                  ATK_MD0700_TOUCH_PEN_GPIO_PIN,               \
                                  GPIO_PIN_SET) :                              \
                HAL_GPIO_WritePin(ATK_MD0700_TOUCH_PEN_GPIO_PORT,              \
                                  ATK_MD0700_TOUCH_PEN_GPIO_PIN,               \
                                  GPIO_PIN_RESET);                             \
        } while (0)
#    define ATK_MD0700_TOUCH_TCS(x)                                            \
        do {                                                                   \
            x ? HAL_GPIO_WritePin(ATK_MD0700_TOUCH_CS_GPIO_PORT,               \
                                  ATK_MD0700_TOUCH_CS_GPIO_PIN,                \
                                  GPIO_PIN_SET) :                              \
                HAL_GPIO_WritePin(ATK_MD0700_TOUCH_CS_GPIO_PORT,               \
                                  ATK_MD0700_TOUCH_CS_GPIO_PIN,                \
                                  GPIO_PIN_RESET);                             \
        } while (0)

/* 触摸点坐标数据结构 */
typedef struct
{
    int16_t x; /* 触摸点X坐标 */
    int16_t y; /* 触摸点Y坐标 */
} atk_md0700_touch_point_t;

/* Info of a rectangle area, including position o fstart point, width and height
 */
typedef struct
{
    atk_md0700_touch_point_t startPoint;
    uint16_t                 width;
    uint16_t                 height;
} AreaInfo;

extern uint8_t PressingTime_prev;

typedef enum {
    Slide_To_Left = 0,
    Slide_To_Right,
    Slide_Up,
    Slide_Down,
    Slide_Direction_Min     = Slide_To_Left,           /* Min */
    Slide_Direction_Max     = Slide_Down,              /* Max */
    Slide_Direction_Default = Slide_Direction_Min,     /* Default */
    Slide_Direction_Cnt     = Slide_Direction_Max + 1, /* The number of all */
    Slide_Direction_None    = Slide_Direction_Cnt,     /* Invalid value */
} SlideDirection;

extern char* SlideDirectionStr[Slide_Direction_Cnt + 1];

typedef enum {
    OnPress = 0,
    ShortPressing,
    LongPressing,
    Moving,
    OnRelease,
    Touch_State_Min     = OnPress,
    Touch_State_Max     = OnRelease,           /* Max */
    Touch_State_Default = Touch_State_Min,     /* Default */
    Touch_State_Cnt     = Touch_State_Max + 1, /* The number of all */
    Touch_State_None    = Touch_State_Cnt,     /* Invalid value */
} TouchState;

typedef enum {
    NoEvent = 0,
    ShortPress,
    LongPress,
    Move,
    Touch_Event_Min     = NoEvent,
    Touch_Event_Max     = Move,                /* Max */
    Touch_Event_Default = Touch_Event_Min,     /* Default */
    Touch_Event_Cnt     = Touch_Event_Max + 1, /* The number of all */
    Touch_Event_None    = Touch_Event_Cnt,     /* Invalid value */
} TouchEvent;

extern char* TouchEventStr[Touch_Event_Cnt + 1];

typedef enum {
    MovingFlag = 0,
    LongPressingFlag,
    ShortPressingFlag,
    Touch_Flag_Min     = MovingFlag,
    Touch_Flag_Max     = ShortPressingFlag,  /* Max */
    Touch_Flag_Default = Touch_Flag_Min,     /* Default */
    Touch_Flag_Cnt     = Touch_Flag_Max + 1, /* The number of all */
    Touch_Flag_None    = Touch_Flag_Cnt,     /* Invalid value */
} TouchFlag;

extern atk_md0700_touch_point_t point_prev[ATK_MD0700_TOUCH_TP_ENABLE_CNT];
extern atk_md0700_touch_point_t point_cur[ATK_MD0700_TOUCH_TP_ENABLE_CNT];

// #    define setTouchStateFlag(state, flag) ((state) | (1 << (flag)))
// #    define getTouchStateFlag(state, flag) (((state) >> (flag)) & 0x01)
// #    define clearTouchStateFlag(state) ((state)&0x00)

/* 错误代码 */
#    define ATK_MD0700_TOUCH_EOK 0   /* 没有错误 */
#    define ATK_MD0700_TOUCH_ERROR 1 /* 错误 */

/* 操作函数 */
void    atk_md0700_touch_init(void); /* ATK-MD0700模块触摸初始化 */
uint8_t atk_md0700_touch_scan(atk_md0700_touch_point_t* point,
                              uint8_t cnt); /* ATK-MD0700模块触摸扫描 */
float   getSlideAngle(int16_t dy, int16_t dx);
SlideDirection getSlideDirection(uint16_t startX,
                                 uint16_t startY,
                                 uint16_t endX,
                                 uint16_t endY);
TouchState     touchEventUpdate(uint8_t* pState, uint8_t* pFlag);
TouchEvent     getTouchEvent(uint8_t flag);

uint8_t getTouchFlag(uint8_t flag, TouchFlag touchFlag);
void    setTouchFlag(uint8_t* flag, TouchFlag touchFlag);
void    clearTouchFlag(uint8_t* flag);
#endif /* ATK_MD0700_USING_TOUCH */

#endif
