#include "main.h"

uint8_t lcd_buffer;
uint8_t* page_buffer[PAGE_NUM];
unsigned int page_buffer_len[PAGE_NUM];

extern FIL *main_file;	  	// file 1
extern FIL *ftemp;	  		// fiel temp.
extern UINT br,bw;			// the number of byte really read or written
extern FILINFO fileinfo;	// file information
extern DIR dir;  			// directory

int main(void)
{
	uint8_t res;
	uint16_t i;
	uint8_t tmp_buf[PAGE_SIZE];
	FontHeader fontHeader = {FLAG_OK, FLAG_OK};
	
	page_buffer[0] = tmp_buf;
	
	/* Some initialization work */
    HAL_Init();                         /* HAL init */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* Set clock: 72Mhz */
    delay_init(72);                     /* delay init */
    led_init();                         /* LED init */
//	key_init();                         /* KEY init */
    usart_init(115200);                 /* Serial: 115200 */
	atk_md0700_init();					/* LCD panel init */
	SRAM_Init();
	W25QXX_Init();
//	TIM3_Init(1000-1,72-1);
	res = exfuns_init();
	if (res) {
		ERROR_THROW("Fail to do exfuns_init()");
	}
#if ACTION_ONCE
	ActionCommand actionCommand = ACTION_COMMAND;
	switch (actionCommand) {
		case WriteFontHeader:
			fontHeader.ugbkok = FLAG_OK;
			fontHeader.fontok = FLAG_OK;
			res = write_font_header(&fontHeader, 10);
			if (res)
				ERROR_THROW("Fail to write FontHeader.");
			break;
		case EraseFontHeader:
			fontHeader.ugbkok = 0x00;
			fontHeader.fontok = 0x00;
			res = write_font_header(&fontHeader, 10);
			if (res)
				ERROR_THROW("Fail to erase FontHeader.");
			break;
		case SingleTest: {
			char font_update_log_buf[100];
				strcpy(font_update_log_buf, "Update UNIGBK.bin ==> ");
				sprintf(font_update_log_buf+strlen(font_update_log_buf), "%s:%s || %d/%d||\r\n", FontNameStr[Font_KaiTi], FontSizeStr[PX16], 1, 100);
				ERROR_THROW(font_update_log_buf);
			}
			break;
		default:
			ERROR_THROW("Action once. In default of switch...");
			break;
	}
	LED_flashing(1000);
#else
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
	res = check_font_header(10);
	
// Test
	if (!res) {
		ERROR_THROW("Succeed to detect OK flags in FontHeader\r\n");
	}
	
	/* Copy font file from SD Card to Flash */
	if (res) {
		res = update_font();
//		res = load_file_to_flash(MYFONT_MSYH_32_PATH, MYFONT_MSYH_32_ADDR);
//		res = 1;
		if (res) {
//			ERROR_THROW("load_file_to_flash error!\r\n");
			ERROR_THROW("Fail to load UNIGBK.BIN and font library to falsh.");
		}
		fontHeader.fontok = FLAG_OK;
		fontHeader.ugbkok = FLAG_OK;
		W25QXX_Write((uint8_t*)&fontHeader, FONT_HEADER_SIZE, sizeof(fontHeader));
	}
	
	/* Open test txt fle in SD Card */
	res = f_open(&my_file, "0:BOOK/test.txt", FA_READ);
	if (res != FR_OK) {
		ERROR_THROW("Open test file fail.");
	}
	f_read(&my_file, page_buffer[0], PAGE_SIZE, &br);
	if (res != FR_OK) {
		ERROR_THROW("Read test file fail.");
	}
	/* Test to show a string with Chinese character */
	Show_Str_Mid(30, 30, (uint8_t*)"ÄãºÃ£¬ÊÀ½ç£¡", Font_SimSun, PX12, 20);
#endif
	while (1) {
		printf("Not once action. In infinite loop now...");
		delay_ms(1000);
	}
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
