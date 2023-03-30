#ifndef __UTIL_H__
#define __UTIL_H__

#include "SYSTEM/sys/sys.h"
#include "math.h"
#include "float.h"

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

/* Init the param to prepare for the log function */
void Progress_Init(Progress* progress, uint32_t cur_val, uint32_t dead_val);
void ProgressWithInfo_Init(ProgressWithInfo* progressWithInfo, 
                uint32_t cur_val, uint32_t dead_val, char* info, char* src, char* dest);

/* Update the param to pass to log function  */
void Progress_Update(Progress* progress, uint32_t cur_val);
void ProgressWithInfo_Update(ProgressWithInfo* progressWithInfo, uint32_t cur_val);

#endif
