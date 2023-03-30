#include "util.h"

void Progress_Init(Progress* progress, uint32_t cur_val, uint32_t dead_val) {
    progress->cur_val = cur_val;
    progress->dead_val = dead_val;
}

void Progress_Update(Progress* progress, uint32_t cur_val) {
    progress->cur_val = cur_val;
}

void ProgressWithInfo_Init(ProgressWithInfo* progressWithInfo, 
                uint32_t cur_val, uint32_t dead_val, char* info, char* src, char* dest) {
    Progress_Init(&progressWithInfo->progress, cur_val, dead_val);
    progressWithInfo->info = info;
    progressWithInfo->src = src;
    progressWithInfo->dest = dest;
}

void ProgressWithInfo_Update(ProgressWithInfo* progressWithInfo, uint32_t cur_val) {
    Progress_Update(&progressWithInfo->progress, cur_val);
}
