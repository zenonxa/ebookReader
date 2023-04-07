/**
 ****************************************************************************************************
 * @file        atk_md0700_touch.c
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

#include "./BSP/ATK_MD0700/atk_md0700_touch.h"
#include "./BSP/ATK_MD0700/atk_md0700_touch_iic.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/usart/usart.h"
#include "BSP/TIMER/timer.h"
#include "log.h"
#include "util.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#if (ATK_MD0700_USING_TOUCH != 0)

/* ATK-MD0700模块触摸部分寄存器定义 */
#    define ATK_MD0700_TOUCH_REG_DEVIDE_MODE 0x00 /* 设备模式控制寄存器 */
#    define ATK_MD0700_TOUCH_REG_ID_G_MODE 0xA4 /* 中断模式控制寄存器 */
#    define ATK_MD0700_TOUCH_REG_ID_G_THGROUP                                  \
        0x80 /* 有效触摸阈值配置寄存器                              \
              */
#    define ATK_MD0700_TOUCH_REG_ID_G_PERIODACTIVE                             \
        0x88                                    /* 激活周期配置寄存器 \
                                                 */
#    define ATK_MD0700_TOUCH_REG_TD_STATUS 0x02 /* 触摸状态寄存器 */
#    define ATK_MD0700_TOUCH_REG_TP1 0x03       /* 触摸点1数据寄存器 */
#    define ATK_MD0700_TOUCH_REG_TP2 0x09       /* 触摸点2数据寄存器 */
#    define ATK_MD0700_TOUCH_REG_TP3 0x0F       /* 触摸点3数据寄存器 */
#    define ATK_MD0700_TOUCH_REG_TP4 0x15       /* 触摸点4数据寄存器 */
#    define ATK_MD0700_TOUCH_REG_TP5 0x1B       /* 触摸点5数据寄存器 */

/* 触摸状态寄存器掩码 */
#    define ATK_MD0700_TOUCH_TD_STATUS_MASK_CNT 0x0F

#    define PI 3.1415926f
#    define TouchOffsetErrorValue 20

/* ATK-MD0700模块触摸点数据寄存器 */
static const uint16_t g_atk_md0700_touch_tp_reg[ATK_MD0700_TOUCH_TP_MAX] = {
    ATK_MD0700_TOUCH_REG_TP1, ATK_MD0700_TOUCH_REG_TP2,
    ATK_MD0700_TOUCH_REG_TP3, ATK_MD0700_TOUCH_REG_TP4,
    ATK_MD0700_TOUCH_REG_TP5,
};

Position point_prev[ATK_MD0700_TOUCH_TP_ENABLE_CNT];
Position point_cur[ATK_MD0700_TOUCH_TP_ENABLE_CNT];

uint8_t PressingTime_prev = 0;

char* SlideDirectionStr[Slide_Direction_Cnt + 1] = {
    "slide to left", "slide to right",    "slide up",
    "slide down",    "Invalid direction",
};

char* TouchEventStr[Touch_Event_Cnt + 1] = {
    "No event", "ShortPress", "LongPress", "Move", "Invalid event",
};

SlideDirection flip_vertically(uint8_t* pSlideDirection);
TouchEvent     touchFlagMappingToEvent(TouchFlag touchFlag);

/**
 * @brief       ATK-MD0700模块触摸硬件初始化
 * @param       无
 * @retval      无
 */
static void atk_md0700_touch_hw_init(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};

    /* 使能时钟 */
    ATK_MD0700_TOUCH_PEN_GPIO_CLK_ENABLE();
    ATK_MD0700_TOUCH_CS_GPIO_CLK_ENABLE();

    /* 初始化PEN引脚 */
    gpio_init_struct.Pin   = ATK_MD0700_TOUCH_PEN_GPIO_PIN;
    gpio_init_struct.Mode  = GPIO_MODE_INPUT;
    gpio_init_struct.Pull  = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ATK_MD0700_TOUCH_PEN_GPIO_PORT, &gpio_init_struct);

    /* 初始化TCS引脚 */
    gpio_init_struct.Pin   = ATK_MD0700_TOUCH_CS_GPIO_PIN;
    gpio_init_struct.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull  = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ATK_MD0700_TOUCH_CS_GPIO_PORT, &gpio_init_struct);
}

/**
 * @brief       ATK-MD0700模块触摸硬件复位
 * @param       无
 * @retval      无
 */
static void atk_md0700_touch_hw_reset(void)
{
    ATK_MD0700_TOUCH_TCS(0);
    delay_ms(20);
    ATK_MD0700_TOUCH_TCS(1);
    delay_ms(50);
}

/**
 * @brief       ATK-MD0700模块触摸寄存器初始化
 * @param       无
 * @retval      无
 */
static void atk_md0700_touch_reg_init(void)
{
    uint8_t dat;

    /* 设备模式控制寄存器 */
    dat = 0x00;
    atk_md0700_touch_iic_write_reg(ATK_MD0700_TOUCH_REG_DEVIDE_MODE, &dat,
                                   sizeof(dat));

    /* 中断模式控制寄存器 */
    dat = 0x00;
    atk_md0700_touch_iic_write_reg(ATK_MD0700_TOUCH_REG_ID_G_MODE, &dat,
                                   sizeof(dat));

    /* 有效触摸阈值配置寄存器 */
    dat = 22;
    atk_md0700_touch_iic_write_reg(ATK_MD0700_TOUCH_REG_ID_G_THGROUP, &dat,
                                   sizeof(dat));

    /* 激活周期配置寄存器，不能小于12 */
    dat = 12;
    atk_md0700_touch_iic_write_reg(ATK_MD0700_TOUCH_REG_ID_G_PERIODACTIVE, &dat,
                                   sizeof(dat));
}

/**
 * @brief       ATK-MD0700模块触摸初始化
 * @param       无
 * @retval      无
 */
void atk_md0700_touch_init(void)
{
    atk_md0700_touch_hw_init();
    atk_md0700_touch_hw_reset();
    atk_md0700_touch_iic_init();
    atk_md0700_touch_reg_init();
}

/**
 * @brief       ATK-MD0700模块触摸扫描
 * @note        连续调用间隔需大于4ms
 * @param       point: 扫描到的触摸点信息
 *              cnt  : 需要扫描的触摸点数量（1~ATK_MD0700_TOUCH_TP_MAX）
 * @retval      0   : 没有扫描到触摸点
 *              其他: 实际获取到的触摸点信息数量
 */
uint8_t atk_md0700_touch_scan(Position* point, uint8_t cnt)
{
    uint8_t                   tp_stat;
    uint8_t                   tp_cnt;
    uint8_t                   point_index;
    atk_md0700_lcd_disp_dir_t dir;
    uint8_t                   tpn_info[4];
    Position  point_raw;

    if ((cnt == 0) || (cnt > ATK_MD0700_TOUCH_TP_MAX)) {
        return 0;
    }

    for (point_index = 0; point_index < cnt; point_index++) {
        if (&point[point_index] == NULL) {
            return 0;
        }
    }

    atk_md0700_touch_iic_read_reg(ATK_MD0700_TOUCH_REG_TD_STATUS, &tp_stat,
                                  sizeof(tp_stat));
    tp_cnt = tp_stat & ATK_MD0700_TOUCH_TD_STATUS_MASK_CNT;
    if ((tp_cnt != 0) && (tp_cnt <= ATK_MD0700_TOUCH_TP_MAX)) {
        tp_cnt = (cnt < tp_cnt) ? cnt : tp_cnt;

        for (point_index = 0; point_index < tp_cnt; point_index++) {
            atk_md0700_touch_iic_read_reg(
                g_atk_md0700_touch_tp_reg[point_index], tpn_info,
                sizeof(tpn_info));
            point_raw.x = (uint16_t)((tpn_info[0] & 0x0F) << 8) | tpn_info[1];
            point_raw.y = (uint16_t)((tpn_info[2] & 0x0F) << 8) | tpn_info[3];

            dir = atk_md0700_get_disp_dir();
            switch (dir) {
                case ATK_MD0700_LCD_DISP_DIR_0: {
                    point[point_index].x =
                        atk_md0700_get_lcd_width() - point_raw.x;
                    point[point_index].y = point_raw.y;
                    break;
                }
                case ATK_MD0700_LCD_DISP_DIR_90: {
                    point[point_index].x = point_raw.y;
                    point[point_index].y = point_raw.x;
                    break;
                }
                case ATK_MD0700_LCD_DISP_DIR_180: {
                    point[point_index].x = point_raw.x;
                    point[point_index].y =
                        atk_md0700_get_lcd_height() - point_raw.y;
                    break;
                }
                case ATK_MD0700_LCD_DISP_DIR_270: {
                    point[point_index].x =
                        atk_md0700_get_lcd_width() - point_raw.y;
                    point[point_index].y =
                        atk_md0700_get_lcd_height() - point_raw.x;
                    break;
                }
            }
        }

        return tp_cnt;
    } else {
        return 0;
    }
}

uint8_t getTouchFlag(uint8_t flag, TouchFlag touchFlag)
{
    return (flag >> touchFlag) & 0x01;
}

void setTouchFlag(uint8_t* flag, TouchFlag touchFlag)
{
    *flag |= (0x01 << touchFlag);
}

void clearTouchFlag(uint8_t* flag)
{
    uint8_t i;
    for (i = Touch_Flag_Min; i <= Touch_Flag_Max; ++i) {
        *flag &= (~(0x01 << i));
    }
}

/**
 * @description: Calculate the angle of the linked by two given points
 * @param {int} dy:  delta Y
 * @param {int} dx:  delta X
 * @return {float}  angle [0, 360]
 */
float getSlideAngle(int16_t dy, int16_t dx)
{
    return (atan2(dy, dx) * 180 / PI);
}
/**
 * @description:
 * @param {uint16_t} startX  PosX of start position
 * @param {uint16_t} startY  PosY of start position
 * @param {uint16_t} endX    EndX of end position
 * @param {uint16_t} endY    EndY of end position
 * @return {*}
 */
SlideDirection getSlideDirection(uint16_t startX,
                                 uint16_t startY,
                                 uint16_t endX,
                                 uint16_t endY)
{
    uint8_t  slideDirection = Slide_Direction_None;
    uint16_t dy             = endY - startY;
    uint16_t dx             = endX - startX;
    float    angle;

    // 如果滑动距离太短
    if (abs(dx) < 2 && abs(dy) < 2) {
        return (SlideDirection)slideDirection;
    }

    angle = getSlideAngle(dy, dx);
    if (angle >= -45 && angle < 45) {
        slideDirection = Slide_To_Right;
    } else if (angle >= 45 && angle < 135) {
        slideDirection = Slide_Up;
    } else if (angle >= -135 && angle < -45) {
        slideDirection = Slide_Down;
    } else if ((angle >= 135 && angle <= 180) ||
               (angle >= -180 && angle < -135)) {
        slideDirection = Slide_To_Left;
    }
    flip_vertically(&slideDirection);
    return (SlideDirection)slideDirection;
}

/**
 * @description: Flip the SlideDirection vertically
 * @param {uint8_t*} pSlideDirection Pointer to the SlideDirection
 * @return {SlideDirection} The SlideDirection after fliping vertically
 */
SlideDirection flip_vertically(uint8_t* pSlideDirection)
{
    switch (*pSlideDirection) {
        case Slide_Up: *pSlideDirection = Slide_Down; break;
        case Slide_Down: *pSlideDirection = Slide_Up; break;
        case Slide_To_Left:
        case Slide_To_Right:
        default: break;
    }
    return (SlideDirection)*pSlideDirection;
}
/**
 * @description: Update the state of the touch event
 * @param {uint8_t*} pState: the current state of the touch event lifetime
 * @param {uint8_t*} pFlag: the flag relate to the touchState
 * @return {*}
 */
TouchState touchEventUpdate(uint8_t* pState, uint8_t* pFlag)
{
    uint8_t tmp, touchPointCnt;
    tmp = atk_md0700_touch_scan(point_cur, ATK_MD0700_TOUCH_TP_ENABLE_CNT);
    /* Debounce for touch event */
    delay_ms(10);
    touchPointCnt =
        atk_md0700_touch_scan(point_cur, ATK_MD0700_TOUCH_TP_ENABLE_CNT);
    if ((touchPointCnt > 0) && (touchPointCnt == tmp)) {
        /* The first moment on press */
        if (*pState == Touch_State_None) {
            *pState       = OnPress;
            point_prev->x = point_cur->x;
            point_prev->y = point_cur->y;
            /* ToDo: Reset timer for LongPress timing */
            PressingKeepingTime = 0;
            //__HAL_TIM_ENABLE(&TIM3_Handler);
            TimerDelay_Press = ENABLE;
            return (TouchState)*pState;
        }
        /* Moving flag exists */
        if (getTouchFlag(*pFlag, MovingFlag)) {
            *pState = Moving;
            return (TouchState)*pState;
        }
        /* position offset is more than error value, need to set moving flag
         */
        if ((abs(point_cur->x - point_prev->x) > TouchOffsetErrorValue) ||
            (abs(point_cur->y - point_prev->y) > TouchOffsetErrorValue)) {
            setTouchFlag(pFlag, MovingFlag);
            *pState = Moving;
            return (TouchState)*pState;
        }
        /* longPressing flag exists */
        if (getTouchFlag(*pFlag, LongPressingFlag)) {
            *pState = LongPressing;
            return (TouchState)*pState;
        }
        /* The timing of the decider(timer) is greater than the decision value
         */
        if (PressingKeepingTime >= LongPressingJudgeTime) {
            setTouchFlag(pFlag, LongPressingFlag);
            TimerDelay_Press = DISABLE;
            //__HAL_TIM_DISABLE(&TIM3_Handler);
            // PressingKeepingTime = 0;
            return (TouchState)*pState;
        }
        /* ShortPressing flag exists */
        if (getTouchFlag(*pFlag, ShortPressingFlag)) {
            *pState = ShortPressing;
            return (TouchState)*pState;
        }
        /* Set ShortPressing flag, and return the */
        setTouchFlag(pFlag, ShortPressingFlag);
        *pState = ShortPressing;
        return (TouchState)*pState;
    } else {
        if ((*pState != Touch_State_None) && (*pState != OnPress)) {
            *pState = OnRelease;
            //__HAL_TIM_DISABLE(&TIM3_Handler);
            PressingTime_prev = PressingKeepingTime;
            TimerDelay_Press  = DISABLE;
            PressingKeepingTime = 0;
            // PressingKeepingTime = 0;
            return (TouchState)*pState;
        }
    }
    return (TouchState)*pState;
}

TouchEvent touchFlagMappingToEvent(TouchFlag touchFlag)
{
    uint8_t touchEvent;
    switch (touchFlag) {
        case MovingFlag: touchEvent = Move; break;
        case LongPressingFlag: touchEvent = LongPress; break;
        case ShortPressingFlag: touchEvent = ShortPress; break;
        default: touchEvent = NoEvent; break;
    }
    return (TouchEvent)touchEvent;
}

TouchEvent getTouchEvent(uint8_t flag)
{
    uint8_t         i          = 0;
    uint8_t         touchEvent = NoEvent;
    static uint16_t dy;
    static uint16_t dx;
    static float    angle;
    static uint8_t  slideDirection;
    for (i = Touch_Flag_Min; i <= Touch_Flag_Max; ++i) {
        if (getTouchFlag(flag, (TouchFlag)i)) {
            touchEvent = touchFlagMappingToEvent((TouchFlag)i);
            break;
        }
    }
    if (touchEvent != NoEvent) {
        dy             = point_cur[0].y - point_prev[0].y;
        dx             = point_cur[0].x - point_prev[0].x;
        angle          = getSlideAngle(dy, dx);
        slideDirection = getSlideDirection(point_prev[0].x, point_prev[0].y,
                                           point_cur[0].x, point_cur[0].y);
        TouchEventInfo_Update(flag, angle, (SlideDirection)slideDirection, (TouchEvent)touchEvent);
        print_log(Touch_Event_Log);
    }
    return (TouchEvent)touchEvent;
}

#endif /* ATK_MD0700_USING_TOUCH */
