#ifndef __MAIN_H__
#define __MAIN_H__

#include "SYSTEM/sys/sys.h"

#define SRAMEX_USER_BASE_ADDR	(0x68000000+(MY_DISP_HOR_RES)*(MY_DISP_VER_RES)*2)

#define ERROR_LED_FLASH_SPAN_MS	500
#define ERROR_THROW(msg)						\
do {											\
	while(1) {									\
		LED_Toggle();							\
		printf("%s\r\n", msg);					\
		delay_ms(ERROR_LED_FLASH_SPAN_MS);		\
	}											\
}while(0)

#define PRINT_MSG(MSG)		\
do {						\
	printf("%s\r\n", MSG);	\
}while(0)

#define ACTION_ONCE_DONE_MSG	"Action once done. Now in LED flashing infinite loop."

/* Toggle state for LED0 and LED1  */
void LED_Toggle(void);
void LED_flashing(uint16_t time_span_ms);	/* LED0 and LED1, flashing as the given time span */

#endif
