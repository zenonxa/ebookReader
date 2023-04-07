#include "log.h"
#include "BSP/ATK_MD0700/atk_md0700_touch.h"
#include "BSP/TIMER/timer.h"
#include "SYSTEM/usart/usart.h"
#include "string.h"
#include <stm32f1xx.h>

uint8_t* logBuffer;
LogParam logParam;

extern uint8_t PressingTime_prev;

/* Internal interface, not open to external function */
void print_log_normal(void);
void print_log_erase_flash(void);
void print_log_read_flash(void);
void print_log_write_flash(void);
void print_log_progress_base(uint32_t* record);
void print_log_progress(void);
void print_log_touch_event_move(void);
void print_log_touch_event_press_base(void);
void print_log_touch_event(void);

/**
 * @description: Initialize the parameter structure of log sub-system.
 * @return {void}
 */
void LogParam_Init(void)
{
    logParam.progress         = &g_progress;
    logParam.progressWithInfo = &g_progressWithInfo;
    logParam.touchEventInfo   = &g_touchEventInfo;
    logParam.logBuffer        = (char*)logBuffer;
}

/**
 * @description: Open to external functions, used to print log according to the
 * given LogType
 * @param {LogType} logType: The type of log
 * @return {void}
 */
void print_log(LogType logType)
{
    // while(__HAL_USART_GET_FLAG(&g_uart1_handle, USART_FLAG_TC) == RESET);
    switch (logType) {
        case Normal_Log: print_log_normal(); break;
        case Flash_Erase_Log: print_log_erase_flash(); break;
        case Flash_Read_Log:
            /* Maybe don't need to do anything */
            break;
        case Flash_Write_Log: print_log_write_flash(); break;
        case Touch_Event_Log: print_log_touch_event(); break;
        default: printf("Invalid LogType\r\n");
    }
}

/**
 * @description: Print the the log of plain char-string.
 * @return {void}
 */
void print_log_normal()
{
    char* normal_log = logParam.logInfo;
    log_n("%s", normal_log);
}

/**
 * @description: Print the log of the operation of erasing ex-flash, showing the
 * propgress with "x%"
 * @return {void}
 */
void print_log_erase_flash()
{
    Progress* progress = logParam.progress;
    if (progress->cur_val == 0) {
        sprintf((char*)logBuffer, "%s", "Erasing flash");
    }
    print_log_progress();
}

/**
 * @description: Print the log of the operation of reading ex-flash, showing the
 * propgress with "x%"
 * @return {void}
 */
void print_log_read_flash()
{
    //    ProgressWithInfo* progressWithInfo = logParam.progressWithInfo;
    /* Maybe don't need to do anything */
}

/**
 * @description: Print the log of the operation of writing ex-flash, showing the
 * src and dest of the writing, and the propgress with "x%"
 * @return {void}
 */
void print_log_write_flash()
{
    ProgressWithInfo* progressWithInfo = logParam.progressWithInfo;
    if (progressWithInfo->progress.cur_val == 0) {
        sprintf((char*)logBuffer, "Loading [%s]", progressWithInfo->info);
        if (progressWithInfo->src != NULL) {
            logBufferAppend(" from [%s]", progressWithInfo->src);
        }
        if (progressWithInfo->dest != NULL) {
            logBufferAppend(" to [%s]", progressWithInfo->dest);
        }
        strcat((char*)logBuffer, ".");
        progressWithInfo->log = (char*)logBuffer;
    }
    print_log_progress();
}

/**
 * @description: Show the progress, with the format of "x%"
 * @return {void}
 */
void print_log_progress()
{
    static uint32_t val_record;
    static uint32_t dead_percent_5;
    Progress*       progress = logParam.progress;
    if (progress->cur_val == 0) {
        val_record     = 0;
        dead_percent_5 = progress->dead_val / 20;
    }
    /* print log information when progress is 0% or 100%, and it would also
     * refresh log when the progress having increasing by 5%.				*/
    if ((progress->cur_val == 0) ||
        ((progress->cur_val - val_record) > dead_percent_5) ||
        (progress->cur_val == progress->dead_val)) {
        print_log_progress_base(&val_record);
    }
}

/**
 * @description: The base operation of printing a piece log of the progress,
 * memorizing the current value.
 * @param {uint32_t*} record: A pointer to last value of the progress.
 * @return {void}
 */
void print_log_progress_base(uint32_t* record)
{
    Progress* progress = logParam.progress;
    char*     log      = logParam.logBuffer;
    /* Update record */
    if (progress->cur_val == progress->dead_val) {
        *record = 0;
    } else {
        *record = progress->cur_val;
    }
    /* print progress */
    log_n("%s || %7d/%7d || %3.2f%%", log, progress->cur_val,
          progress->dead_val, 100.0 * progress->cur_val / progress->dead_val);
}

/**
 * @description: Handle the Print log of move event,
 * @return {*}
 */
void print_log_touch_event()
{
    TouchEventInfo* touchEventInfo = logParam.touchEventInfo;
    switch (touchEventInfo->touchEvent) {
        case Move: print_log_touch_event_move(); break;
        case LongPress: print_log_touch_event_press_base(); break;
        case ShortPress: print_log_touch_event_press_base(); break;
        case NoEvent: log_n("[No Event]"); break;
        default: log_n("Invalid event"); break;
    }
}

/**
 * @description: Print log of move event
 * @return {void}
 */
void print_log_touch_event_move()
{
    TouchEventInfo* touchEventInfo = logParam.touchEventInfo;
    log_n("[Event: Move]");
    log_n("Position: (%3d, %3d) --> (%3d, %3d)", touchEventInfo->startPos[0].x,
          touchEventInfo->startPos[0].y, touchEventInfo->endPos[0].x,
          touchEventInfo->endPos[0].y);
    log_n("Angle: %.2f", touchEventInfo->angle);
    log_n("SlideDirection: %s",
          SlideDirectionStr[touchEventInfo->slideDirection]);
    log_n("");
}

/**
 * @description: Print log of press event, including ShortPress and LongPress.
 * @return {void}
 */
void print_log_touch_event_press_base()
{
    TouchEventInfo* touchEventInfo = logParam.touchEventInfo;
    if ((touchEventInfo->touchEvent != LongPress) &&
        (touchEventInfo->touchEvent != ShortPress)) {
        return;
    }
    sprintf(logParam.logBuffer, "[Event: %s]\r\n",
            TouchEventStr[touchEventInfo->touchEvent]);
    logBufferAppend(
        "Position: (%3d, %3d)\r\n",
        touchEventInfo->startPos[0].x +
            (touchEventInfo->endPos[0].x - touchEventInfo->startPos[0].x) / 2,
        touchEventInfo->startPos[0].y +
            (touchEventInfo->endPos[0].y - touchEventInfo->startPos[0].y) / 2);
    logBufferAppend("PressingTime: %2d\r\n", PressingTime_prev);
    log_n("%s", logParam.logBuffer);
    log_n("");
}
