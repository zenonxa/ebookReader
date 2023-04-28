#include "util.h"
#include "log.h"

Progress         g_progress;
ProgressWithInfo g_progressWithInfo;
TouchEventInfo   g_touchEventInfo;

void Progress_Init(uint32_t cur_val, uint32_t dead_val)
{
    Progress* progress = logParam.progress;
    progress->cur_val  = cur_val;
    progress->dead_val = dead_val;
}

void Progress_Update(uint32_t cur_val)
{
    Progress* progress = logParam.progress;
    progress->cur_val  = cur_val;
}

void ProgressWithInfo_Init(uint32_t cur_val,
                           uint32_t dead_val,
                           char*    info,
                           char*    src,
                           char*    dest)
{
    ProgressWithInfo* progressWithInfo = logParam.progressWithInfo;
    Progress_Init(cur_val, dead_val);
    progressWithInfo->info = info;
    progressWithInfo->src  = src;
    progressWithInfo->dest = dest;
}

void ProgressWithInfo_Update(uint32_t cur_val)
{
    // ProgressWithInfo* progressWithInfo = logParam.progressWithInfo;
    Progress_Update(cur_val);
}

void TouchEventInfo_Init()
{
    // TouchEventInfo* touchEventInfo = logParam.touchEventInfo;
    logParam.touchEventInfo->startPos = point_prev;
    logParam.touchEventInfo->endPos   = point_cur;
    clearTouchFlag(&logParam.touchEventInfo->flag);
    logParam.touchEventInfo->angle          = 0.0;
    logParam.touchEventInfo->slideDirection = Slide_Direction_None;
    logParam.touchEventInfo->touchEvent     = Touch_Event_NoEvent;
}

void TouchEventInfo_Update(uint8_t        flag,
                           float          angle,
                           SlideDirection slideDirection,
                           TouchEvent     touchEvent)
{
    TouchEventInfo* touchEventInfo = logParam.touchEventInfo;
    touchEventInfo->flag           = flag;
    touchEventInfo->angle          = angle;
    touchEventInfo->slideDirection = slideDirection;
    touchEventInfo->touchEvent     = touchEvent;
}

#define SWAP_VAL(pA, pB, buf, type)                                            \
    do {                                                                       \
        *((type*)(buf)) = *((type*)(pA));                                      \
        *((type*)(pA))  = *((type*)(pB));                                      \
        *((type*)(pB))  = *((type*)(buf));                                     \
    } while (0)

void swapVal(void* a, void* b, ValType type)
{
    char tmpBuf[10];
    switch (type) {
        case ValType_UINT8: SWAP_VAL(a, b, tmpBuf, uint8_t); break;
        case ValType_UINT16: SWAP_VAL(a, b, tmpBuf, uint16_t); break;
        case ValType_UINT32: SWAP_VAL(a, b, tmpBuf, uint32_t); break;
        case ValType_UINT8_POINTER: SWAP_VAL(a, b, tmpBuf, uint8_t*); break;
        case ValType_UINT16_POINTER: SWAP_VAL(a, b, tmpBuf, uint16_t*); break;
        case ValType_UINT32_POINTER: SWAP_VAL(a, b, tmpBuf, uint32_t*); break;
        default:; break;
    }
}