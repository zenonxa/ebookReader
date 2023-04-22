#ifndef __UTIL_H__
#define __UTIL_H__

#include "BSP/ATK_MD0700/atk_md0700_touch.h"
#include "SYSTEM/sys/sys.h"
#include "float.h"
#include "math.h"

#define isFloatEqual(a, b) (fabs((a) - (b)) < FLT_EPSILON)
#define isFloatZero(a) (fabs(a) < FLT_EPSILON)

typedef struct
{
    uint32_t cur_val;  /* current value, start from 0*/
    uint32_t dead_val; /* target value */
} Progress;

typedef struct
{
    Progress progress;
    char*    info; /* For data/file name */
    char*    log;
    char*    src;
    char*    dest;
    //	char* info_first;	/* For data/file name */
    //	char* info_second;	/* For src */
    //	char* info_third;	/* For dest */
} ProgressWithInfo;

typedef struct
{
    Position* startPos;
    Position* endPos;
    uint8_t   flag;
    float     angle;
    uint8_t   slideDirection;
    uint8_t   touchEvent;
} TouchEventInfo;

typedef enum {
    ValType_UINT8 = 0,
    ValType_UINT16,
    ValType_UINT32,
    ValType_UINT8_POINTER,
    ValType_UINT16_POINTER,
    ValType_UINT32_POINTER,
    ValType_Min     = ValType_UINT8,
    ValType_Max     = ValType_UINT32_POINTER, /* Max */
    ValType_Default = ValType_Min,            /* Default */
    ValType_Cnt     = ValType_Max + 1,        /* The number of all */
    ValType_None    = ValType_Cnt,            /* Invalid value */
} ValType;

extern Progress         g_progress;
extern ProgressWithInfo g_progressWithInfo;
extern TouchEventInfo   g_touchEventInfo;

/* Init the param to prepare for the log function */
void Progress_Init(uint32_t cur_val, uint32_t dead_val);
void ProgressWithInfo_Init(uint32_t cur_val,
                           uint32_t dead_val,
                           char*    info,
                           char*    src,
                           char*    dest);

/* Update the param to pass to log function  */
void Progress_Update(uint32_t cur_val);
void ProgressWithInfo_Update(uint32_t cur_val);

void TouchEventInfo_Init(void);
void TouchEventInfo_Update(uint8_t        flag,
                           float          angle,
                           SlideDirection slideDirection,
                           TouchEvent     touchEvent);
void swapVal(void* a, void* b, ValType type);

#endif
