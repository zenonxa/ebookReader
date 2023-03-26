#include "SYSTEM/sys/sys.h"
#include "SYSTEM/delay/delay.h"
#include "SYSTEM/usart/usart.h"
#include "BSP/LED/led.h"
#include "BSP/KEY/key.h"
#include "BSP/TIMER/timer.h"
#include "BSP/SRAM/sram.h"
#include "BSP/ATK_MD0700/atk_md0700.h"
#include "BSP/W25QXX/w25qxx.h"
#include "malloc.h"
#include "lvgl.h"
#include "examples/lv_examples.h"
#include "examples/porting/lv_port_disp.h"
#include "examples/porting/lv_port_indev.h"
#include "Test/test_atk_md0700.h"
#include "FATFS/src/ff.h"
#include "FATFS/exfuns/exfuns.h"
#include "font/myfont.h"
#include <stm32f1xx.h>
#include "BSP/SDIO/sdio_sdcard.h"
#include "main.h"

#define PAGE_NUM	3		// the number of cache page
#define PAGE_SIZE	500	// the size of one cache page

#define ACTION_ONCE	0
#define FLAG_VALUE	0xA55A

uint8_t lcd_buffer;
uint8_t* page_buffer[PAGE_NUM];
unsigned int page_buffer_len[PAGE_NUM];

extern FIL *file;	  		// file 1
extern FIL *ftemp;	  		// fiel temp.
extern UINT br,bw;			// the number of byte really read or written
extern FILINFO fileinfo;	// file information
extern DIR dir;  			// directory

lv_style_t style_font;

int main(void)
{
	uint8_t res;
	uint16_t i;
	uint16_t flag;
	uint8_t tmp_buf[PAGE_SIZE];
	
    HAL_Init();                         /* HAL init */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* Set clock: 72Mhz */
    delay_init(72);                     /* delay init */
    led_init();                         /* LED init */
    key_init();                         /* KEY init */
    usart_init(115200);                 /* Serial: 115200 */
//	atk_md0700_init();
	SRAM_Init();
	W25QXX_Init();
#if ACTION_ONCE
	{
		flag = FLAG_VALUE;
		W25QXX_Write((uint8_t*)&flag, 0, sizeof(flag));
		for (i = 0; i < 10; ++i) {
			PRINT_MSG("Write flag done.");
			delay_ms(5);
		}
	}
	LED_flashing(1000);
#else
	/* Some initialization work */
	TIM3_Init(1000-1,72-1);
	lv_init();
	lv_port_disp_init();
	lv_port_indev_init();
/*
	my_mem_init(SRAMIN);
	my_mem_init(SRAMEX);
	res = exfuns_init();
	if (res) {
		delay_ms(3000);
		printf("file: %d\r\n", (file == NULL) ? NULL : 1);
		printf("ftemp: %d\r\n", (ftemp == NULL)? NULL : 1);
		printf("fatbuf:%d\r\n", (fatbuf == NULL) ? NULL : 1);
		ERROR_THROW("exfuns_init() fail!");
	} else {
		printf("Address of file: %x\r\n", (int)file);
		printf("Address of ftemp: %x\r\n", (int)ftemp);
		printf("Address of fatbuf: %x\r\n", (int)fatbuf);
		printf("\r\n");
		delay_ms(100);
	}
	while(1) {
		printf("Size of FATFS: %d\r\n", sizeof(FATFS));
		printf("Size of FIL: %d\r\n", sizeof(FIL));
		LED_Toggle();
		delay_ms(500);
	}
*/

	/* Detect SD Card and mount FATFS for SD Card */
	FATFS my_fs;
	FIL my_file;
	while(SD_Init()) {
		printf("SD Card Error! Please check!\r\n");
		HAL_Delay(200);
	}
	res = f_mount(&my_fs, "0:", 1);
	if (res != FR_OK) {
		ERROR_THROW("Fail to mount SD Card.");
	}
	/* Check flag in Flash */
	for (i = 0; i < 10; ++i) {
		W25QXX_Read((uint8_t*)&flag, 0, sizeof(flag));
		if (flag == FLAG_VALUE)
			break;
	}
	/* Copy font file from SD Card to Flash */
	if (flag != FLAG_VALUE) {
		res = load_file_to_flash(MYFONT_MSYH_32_PATH, MYFONT_MSYH_32_ADDR);
		if (res) {
			ERROR_THROW("load_file_to_flash error!\r\n");
		}
		flag = FLAG_VALUE;
		W25QXX_Write((uint8_t*)&flag, 0, sizeof(flag));
	}
	page_buffer[0] = tmp_buf;
	res = f_open(&my_file, "0:BOOK/test.txt", FA_READ);
	if (res != FR_OK) {
		ERROR_THROW("Open test file fail.");
	}
	f_read(&my_file, page_buffer[0], PAGE_SIZE, &br);
	if (res != FR_OK) {
		ERROR_THROW("Read test file fail.");
	}
	
//	test_atk_md0700();
	/* Test textarea widget of LVGL with custom font */
	lv_style_init(&style_font);
	lv_style_set_text_font(&style_font, &myfont_msyh_32);
	lv_style_set_text_color(&style_font, lv_palette_main(LV_PALETTE_GREEN));
	lv_obj_t * ta = lv_textarea_create(lv_scr_act());
	lv_obj_add_style(ta, &style_font, LV_STATE_DEFAULT);
	lv_obj_set_size(ta, 480, 800);
	lv_textarea_add_text(ta, (const char*)page_buffer[0]);
	
	i = 0;
	while (1) {
		lv_timer_handler();
		delay_ms(1);
		if (i >= 1000) {
			printf("In LVGL infinite loop.\r\n");
			LED_Toggle();
			i = 0;
		} else {
			i++;
		}
	}
#endif
}


void LED_Toggle(void) {
	static uint8_t i = 0;
	if (i%2 == 0) {
		LED0(0);
		LED1(1);
		++i;
	} else {
		LED0(1);
		LED1(0);
		--i;
	}
}

/* Toggle state for LED0 and LED1  */
void LED_flashing(uint16_t time_span_ms) {
	while(1) {
		PRINT_MSG(ACTION_ONCE_DONE_MSG);
		LED_Toggle();
		delay_ms(time_span_ms);
	}
}
