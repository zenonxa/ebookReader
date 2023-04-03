#ifndef __LOG_H__
#define __LOG_H__

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

#define log(format, ...)                                                       \
    do {                                                                       \
        while (__HAL_USART_GET_FLAG(&g_uart1_handle, USART_FLAG_TC) == RESET)  \
            ;                                                                  \
        printf(format, ##__VA_ARGS__);                                         \
    } while (0)

#define log_n(format, ...)                                                     \
    do {                                                                       \
        log(format, ##__VA_ARGS__);                                            \
        printf("\r\n");                                                        \
    } while (0)

#define logBufferAppend(format, ...)                                           \
    do {                                                                       \
        sprintf(logParam.logBuffer + strlen(logParam.logBuffer), format,       \
                ##__VA_ARGS__);                                                \
    } while (0)

#define TIME_SPAN_IN_INFINITE_LOOP 1000 /* ms */

#define ARROW_STRING " ========> "

#define infinite_throw(format, ...)                                            \
    do {                                                                       \
        log_n(format, ##__VA_ARGS__);                                          \
        delay_ms(TIME_SPAN_IN_INFINITE_LOOP);                                  \
    } while (1)

void LogParam_Init(void);
void print_log(LogType logType);

#endif
