#include "string.h"
#include <stm32f1xx.h>
#include "SYSTEM/usart/usart.h"
#include "log.h"

uint8_t* log_buffer;
LogParam logParam;

void print_log_normal(char* normal_log);
void print_log_erase_flash(Progress* progress);
void print_read_flash(ProgressWithInfo* progressWithInfo);
void print_write_flash(ProgressWithInfo* progressWithInfo);
uint8_t print_log_progress(Progress* progress, char* log);
void print_log_progress_base(uint32_t* record, Progress* progress, char* log);


void print_log(LogType logType, LogParam* logParam) {
	while(__HAL_USART_GET_FLAG(&g_uart1_handle, USART_FLAG_TC) == RESET);
	switch(logType) {
		case Normal_Log:
			print_log_normal(logParam->logInfo);
			break;
		case Flash_Erase_Log:
			print_log_erase_flash(&logParam->progress);
			break;
		case Flash_Read_Log:
			/* Maybe don't need to do anything */
			break;
		case Flash_Write_Log:
			print_write_flash(&logParam->progressWithInfo);
			break;
		default:
			printf("Invalid LogType\r\n");
	}
}


void print_log_normal(char* normal_log) {
	log_n("%s", normal_log);
}


void print_log_erase_flash(Progress* progress) {
	static uint8_t no_log = 1;
	if (progress->cur_val == 0) {
		sprintf((char*)log_buffer, "%s", "Erasing flash");
	}
	print_log_progress(progress, (char*)log_buffer);
}


void print_read_flash(ProgressWithInfo* progressWithInfo) {
	/* Maybe don't need to do anything */
}

void print_write_flash(ProgressWithInfo* progressWithInfo) {
	if (progressWithInfo->progress.cur_val == 0) {
		sprintf((char*)log_buffer, "Loading [%s]", progressWithInfo->info);
		if (progressWithInfo->src != NULL) {
			sprintf((char*)log_buffer+strlen((char*)log_buffer), 
				" from [%s]", progressWithInfo->src);
		}
		if (progressWithInfo->dest != NULL) {
			sprintf((char*)log_buffer+strlen((char*)log_buffer), 
				" to [%s]", progressWithInfo->dest);
		}
		strcat((char*)log_buffer, ".");
		progressWithInfo->log = (char*)log_buffer;
	}
	print_log_progress(&progressWithInfo->progress, progressWithInfo->log);
}


uint8_t print_log_progress(Progress* progress, char* log) {
	uint8_t no_log = 1;
	static uint32_t val_record;
	static uint32_t dead_percent_5;
	if (progress->cur_val == 0) {
		val_record = 0;
		dead_percent_5 = progress->dead_val / 20;
	}
	/* print log information when progress is 0% or 100%, and it would also 
	 * refresh log when the progress having increasing by 5%.				*/
	if ((progress->cur_val == 0) || ((progress->cur_val - val_record) > dead_percent_5) || 
		(progress->cur_val == progress->dead_val)) {
		print_log_progress_base(&val_record, progress, log);
	}
	return no_log;
}


void print_log_progress_base(uint32_t* record, Progress* progress, char* log) {
	/* Update record */
	if (progress->cur_val == progress->dead_val) {
		*record = 0;
	} else {
		*record = progress->cur_val;
	}
	/* print progress */
	log_n("%s || %7d/%7d || %3.2f%%", log, progress->cur_val, progress->dead_val, 
			100.0 * progress->cur_val / progress->dead_val);
}
