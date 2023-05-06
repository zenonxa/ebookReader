#ifndef __LOG_H__
#define __LOG_H__

#include "SYSTEM/usart/usart.h"
#include "util.h"

/* Types of log */
typedef enum {
    Normal_Log = 0,
    Flash_Erase_Log,
    Flash_Write_Log,
    Flash_Read_Log,
    Touch_Event_Log,
    Log_Type_Min     = Normal_Log,
    Log_Type_Max     = Touch_Event_Log,
    Log_Type_Default = Log_Type_Min,
    Log_Type_Cnt     = Log_Type_Max + 1,
    Log_Type_None    = 0xff,
} LogType;

/* A structure for parameter of log sub-system */
typedef struct
{
    char*             logInfo;          /* Param for Normal log */
    char*             logBuffer;        /* Param for long log buffer */
    Progress*         progress;         /* Param for Flash Erase Log */
    ProgressWithInfo* progressWithInfo; /* Param for Flash Write and Read Log */
    TouchEventInfo*   touchEventInfo;   /* Param for touch event log */
} LogParam;

extern uint8_t* logBuffer;
extern LogParam logParam;

#define LOG_BUFFER_SIZE 100

#define LED_FLASH_IN_INFINITE_LOOP
#define TEXT_RENDER_LOG_ENABLE 0

/* Some string maybe usually used for log */
#define ARROW_STRING " ========> "

/* print the log to USART interface with no line break */
#define log(format, ...)                                                       \
    do {                                                                       \
        while (__HAL_USART_GET_FLAG(&g_uart1_handle, USART_FLAG_TC) == RESET)  \
            ;                                                                  \
        printf(format, ##__VA_ARGS__);                                         \
    } while (0)

/* print the log to USART interface end with line break */
#define log_n(format, ...)                                                     \
    do {                                                                       \
        log(format, ##__VA_ARGS__);                                            \
        printf("\r\n");                                                        \
    } while (0)

/* Append the log to the tail of the log buffer, which need to send a line break
 * char manually after this code block */
#define logBufferAppend(format, ...)                                           \
    do {                                                                       \
        sprintf(logParam.logBuffer + strlen(logParam.logBuffer), format,       \
                ##__VA_ARGS__);                                                \
    } while (0)

#define CHECK_VALUE_ENABLE 1
#define TIME_SPAN_IN_INFINITE_LOOP 1000 /* ms */
#define infinite_throw(format, ...)                                            \
    do {                                                                       \
        log_n(format, ##__VA_ARGS__);                                          \
        delay_ms(TIME_SPAN_IN_INFINITE_LOOP);                                  \
    } while (1)

#define check_value_not_equal(actual, unexpected, log_format, ...)             \
    do {                                                                       \
        if (CHECK_VALUE_ENABLE) {                                              \
            if ((actual) == (unexpected)) {                                    \
                infinite_throw(log_format, ##__VA_ARGS__);                     \
            }                                                                  \
        }                                                                      \
    } while (0)

/* Check the value */
#define check_value_equal(actual, expected, log_format, ...)                   \
    do {                                                                       \
        if (CHECK_VALUE_ENABLE) {                                              \
            if (!((actual) == (expected))) {                                   \
                infinite_throw(log_format, ##__VA_ARGS__);                     \
            }                                                                  \
        }                                                                      \
    } while (0)

void LogParam_Init(void);
void print_log(LogType logType);

#endif
