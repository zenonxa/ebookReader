#include "main.h"

uint8_t*     lcd_buffer;
uint8_t*     page_buffer[PAGE_NUM];
unsigned int page_buffer_len[PAGE_NUM];

extern FIL*    main_file;  // file 1
extern FIL*    ftemp;      // file temp.
extern UINT    br, bw;     // the number of byte really read or written
extern FILINFO fileinfo;   // file information
extern DIR     dir;        // directory

int main(void)
{
    uint8_t    res;
    uint8_t    i;
    uint8_t    tmp_buf[PAGE_SIZE];
    FontHeader fontHeader = {FLAG_OK, FLAG_OK};

    page_buffer[0] = tmp_buf;

    /* Some initialization work */
    HAL_Init();                         /* HAL init */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* Set clock: 72Mhz */
    delay_init(72);                     /* delay init */
    led_init();                         /* LED init */
    //	key_init();                         /* KEY init */
    usart_init(115200); /* Serial: 115200 */
    atk_md0700_init();  /* LCD panel init */
    SRAM_Init();
    W25QXX_Init();
    TIM3_Init(1000 - 1, 72 - 1);
    res = exfuns_init();
    if (res) {
        infinite_throw("Fail to do exfuns_init()");
    }
#if ACTION_ONCE
    ActionCommand actionCommand = ACTION_COMMAND;
    switch (actionCommand) {
        case WriteFontHeader:
            fontHeader.ugbkok = FLAG_OK;
            fontHeader.fontok = FLAG_OK;
            res               = write_font_header(&fontHeader, 10);
            infinite_throw("Write FontHeader done.");
            break;
        case EraseFontHeader:
            fontHeader.ugbkok = 0x00;
            fontHeader.fontok = 0x00;
            res               = write_font_header(&fontHeader, 10);
            if (res) {
                infinite_throw("Erase FontHeader done.");
            }
            break;
        case WriteFontLib: {
            FontName fontName   = Font_SimSun;
            FontSize fontSize_1 = PX12;
            FontSize fontSize_2 = PX16;
            waiting_for_SD_Card();
            mount_SD_Card();
            res = update_fontx(fontName, fontSize_1);
            if (res) {
                log_n("Fail to write font library to ex-flash",
                      FontNameStr[fontName], FontSizeStr[fontSize_1]);
            }
            res = update_fontx(fontName, fontSize_2);
            if (res) {
                log_n("Fail to write font[%s:%s] library to ex-flash",
                      FontNameStr[fontName], FontSizeStr[fontSize_2]);
            }
            infinite_throw("Writing font library to ex-flash done.");
            break;
        }
        case LoadFileToFlash: {
            char*    fileName = "";
            uint32_t address  = getFontAddr(Font_SimSun, PX12);
            // load_file_to_flash(fileName, address);
            break;
        }
        case SingleTest: {
            uint8_t i, j;
            char    font_update_log_buf[100];
            char    filePath[32];
            for (i = Font_Name_Min; i <= Font_Name_Max; ++i) {
                for (j = Font_Size_Min; j <= Font_Size_Max; ++j) {
                    log_n("File path: %s, address in flash: %d",
                          getFontPath(filePath, (FontName)i, (FontSize)j),
                          getFontAddr(i, j));
                    delay_ms(10);
                }
            }
            break;
        }
        default: infinite_throw("Action once. In default of switch..."); break;
    }
    LED_flashing(1000);
#else
    /* Detect SD Card and mount FATFS for SD Card */
    FIL my_file;
    waiting_for_SD_Card();
    mount_SD_Card();
    /* Check flag in Flash */
    res = check_font_header(10);

    /* Copy font file from SD Card to Flash */
    if (res) {
        res = update_font();
        if (res) {
            infinite_throw(
                "Fail to load UNIGBK.BIN and font library to falsh.");
        }
        fontHeader.fontok = FLAG_OK;
        fontHeader.ugbkok = FLAG_OK;
        W25QXX_Write((uint8_t*)&fontHeader, FONT_HEADER_ADDR,
                     sizeof(fontHeader));
    }

    /* Open test txt fle in SD Card */
    res = f_open(&my_file, "0:BOOK/test.txt", FA_READ);
    if (res != FR_OK) {
        infinite_throw("Open test file fail.");
    }
    f_read(&my_file, page_buffer[0], PAGE_SIZE, &br);
    if (res != FR_OK) {
        infinite_throw("Read test file fail.");
    }
    /* Test to show a string with Chinese character */
    Show_Str(30, 30, ATK_MD0700_LCD_WIDTH/5*4, ATK_MD0700_LCD_HEIGHT/5*4, page_buffer[0], Font_SimSun, PX16, 1);
    f_close(&my_file);
    /* Unmount SD Card volume */
    f_mount(NULL, "0:", 1);
#endif
    while (1) {
        log_n("Main work finished. In infinite loop now...");
        LED_Toggle();
        delay_ms(1500);
    }
}

void LED_Toggle(void)
{
    static uint8_t i = 0;
    if (i % 2 == 0) {
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
void LED_flashing(uint16_t time_span_ms)
{
    while (1) {
        log_n(ACTION_ONCE_DONE_MSG);
        LED_Toggle();
        delay_ms(time_span_ms);
    }
}

void waiting_for_SD_Card(void)
{
    while (SD_Init()) {
        log_n("SD Card Error! Please check!");
        HAL_Delay(500);
    }
}

void mount_SD_Card(void)
{
    if (f_mount(fs[FAT_DRV_SDCARD], "0:", 1) != FR_OK) {
        infinite_throw("Fail to mount SD Card.");
    }
}