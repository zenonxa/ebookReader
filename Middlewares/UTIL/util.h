#ifndef __UTIL_H__
#define __UTIL_H__

#include "SYSTEM/sys/sys.h"
#include "math.h"
#include "float.h"
#include "BSP/ATK_MD0700/atk_md0700_touch.h"

#define isFloatEqual(a, b) 	(fabs((a)-(b)) < 1e-6)
#define isFloatZero(a) 		(fabs(a) < FLT_EPSILON)

typedef struct {
	uint32_t cur_val;	/* current value, start from 0*/
	uint32_t dead_val;	/* target value */
} Progress;

typedef struct {
	Progress progress;
	char* info;			/* For data/file name */
	char* log;
	char* src;
	char* dest;
//	char* info_first;	/* For data/file name */
//	char* info_second;	/* For src */
//	char* info_third;	/* For dest */
} ProgressWithInfo;

typedef struct {
	Position* startPos;
	Position* endPos;
	uint8_t flag;
	float angle;
	uint8_t slideDirection;
	uint8_t touchEvent;
} TouchEventInfo;

extern Progress g_progress;
extern ProgressWithInfo g_progressWithInfo;
extern TouchEventInfo g_touchEventInfo;

/* Init the param to prepare for the log function */
void Progress_Init(uint32_t cur_val, uint32_t dead_val);
void ProgressWithInfo_Init(uint32_t cur_val, uint32_t dead_val, char* info, char* src, char* dest);

/* Update the param to pass to log function  */
void Progress_Update(uint32_t cur_val);
void ProgressWithInfo_Update(uint32_t cur_val);

void TouchEventInfo_Init(void);
void TouchEventInfo_Update(uint8_t flag, float angle, SlideDirection slideDirection, TouchEvent touchEvent);

#endif
