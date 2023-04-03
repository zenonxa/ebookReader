#include "main.h"

uint8_t*     lcd_buffer;
uint8_t*     page_buffer[PAGE_NUM];
unsigned int page_buffer_len[PAGE_NUM];

extern FIL*    main_file;  // file 1
extern FIL*    ftemp;      // file temp.
extern UINT    br, bw;     // the number of byte really read or written
extern FILINFO fileinfo;   // file information
extern DIR     dir;        // directory

uint8_t fontNameSelect = Font_Name_Min;
uint8_t fontSizeSelect = Font_Size_Min;
uint8_t needRerender   = 1;

int main(void)
{
    uint8_t    res;
    uint8_t    i;
    uint8_t    tmp_buf[PAGE_SIZE];
    FontHeader fontHeader = {FLAG_OK, FLAG_OK};
    uint16_t   color      = ATK_MD0700_BLACK;

    page_buffer[0] = tmp_buf;

    /* Some initialization work */
    HAL_Init();                         /* HAL init */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* Set clock: 72Mhz */
    delay_init(72);                     /* delay init */
    led_init();                         /* LED init */
    key_init();                         /* KEY init */
    EXTI_Init();                        /* external interrupt init */
    usart_init(115200);                 /* Serial: 115200 */
    atk_md0700_init(&color);            /* LCD panel init */
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
            FontName fontName = Font_SimHei;
            uint8_t  i;
            char     fontPath[60];
            waiting_for_SD_Card();
            mount_SD_Card();
            res = update_fontx(Font_KaiTi, PX16);
            if (res) {
                infinite_throw("Fail to load [%s:%s] to ex-Flash", Font_KaiTi,
                               PX16);
            }
            // for (i = Font_Size_Min; i <= Font_Size_Max; i++) {
            //     res = update_fontx(fontName, (FontSize)i);
            //     if (res) {
            //         infinite_throw("Fail to load [%s:%s] to ex-Flash",
            //                        FontNameStr[fontName], FontSizeStr[i]);
            //     }
            // }
            infinite_throw("Write font library to ex-Flash done.");
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
    FIL      my_file;
    uint8_t* pc;
    uint16_t textAreaWidth  = ATK_MD0700_LCD_WIDTH * 4 / 5;
    uint16_t textAreaHeight = ATK_MD0700_LCD_HEIGHT * 4 / 5;
    /* Show Logo */
    show_logo(NULL, 3000);
    atk_md0700_fill(0, 0, ATK_MD0700_LCD_WIDTH - 1, ATK_MD0700_LCD_HEIGHT - 1,
                    &BACKGROUND_COLOR, SINGLE_COLOR_BLOCK);
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
    page_buffer[0][br] = '\0';
    if (res != FR_OK) {
        infinite_throw("Read test file fail.");
    }
    /* Test to show a string with Chinese character */
    // BACKGROUND_COLOR = ATK_MD0700_GREEN;
    // Show_Str((ATK_MD0700_LCD_WIDTH - textAreaWidth) / 2,
    //               (ATK_MD0700_LCD_HEIGHT - textAreaHeight) / 2,
    //               textAreaWidth, textAreaHeight, page_buffer[0], Font_SimSun,
    //               PX12, 1);
    // log_n("br: %d", br);
    f_close(&my_file);
    /* Unmount SD Card volume */
    f_mount(NULL, "0:", 1);

    uint8_t touchState = Touch_State_None;
    uint8_t flag;
    uint8_t slideDirestion;
    int16_t dx, dy;
    clearTouchFlag(&flag);

    while (1) {
#    if 1
        touchEventUpdate(&touchState, &flag);
        if (touchState == OnRelease) {
            // dx = point_cur[0].x - point_prev[0].x;
            // dy = point_cur[0].y - point_prev[0].y;
            // slideDirestion = getSlideDirection(point_prev[0].x, point_prev[0].y, point_cur[0].x, point_cur[0].y);
            // log_n("TouchEvent: %d:%s", slideDirestion, SlideDirectionStr[slideDirestion]);
            log_n("point_prev[0].x: %d, point_prev[0].y: %d", point_prev[0].x, point_prev[0].y);
            log_n("point_cur[0].x: %d, point_cur[0].y: %d", point_cur[0].x, point_cur[0].y);
            dy = point_cur[0].y - point_prev[0].y;
            dx = point_cur[0].x - point_prev[0].x;
            log_n("Touch state flag: %x", flag);
            log_n("Angel: %.2f", getSlideAngle(dy, dx));
            slideDirestion = getSlideDirection(point_prev[0].x, point_prev[0].y, point_cur[0].x, point_cur[0].y);
            log_n("Direction ===> %d:%s", slideDirestion, SlideDirectionStr[slideDirestion]);
            log_n("TouchEvent: %d", getTouchEvent(flag));
            log_n("");
            clearTouchFlag(&flag);
			touchState = Touch_State_None;
    }
#    else
        if (atk_md0700_touch_scan(point_cur, ATK_MD0700_TOUCH_TP_ENABLE_CNT) >
            0) {
            log_n("X: %d, Y:%d", point_cur[0].x, point_cur[0].y);
            delay_ms(1000);
        }
#    endif
#    if 0
        if (needRerender) {
            log_n("Rerender LCD panel.");
            atk_md0700_fill((ATK_MD0700_LCD_WIDTH - textAreaWidth) / 2,
                            (ATK_MD0700_LCD_HEIGHT - textAreaHeight) / 2,
                            (ATK_MD0700_LCD_WIDTH + textAreaWidth) / 2,
                            (ATK_MD0700_LCD_HEIGHT + textAreaHeight) / 2,
                            &BACKGROUND_COLOR, SINGLE_COLOR_BLOCK);
            delay_ms(500);
            Show_Str((ATK_MD0700_LCD_WIDTH - textAreaWidth) / 2,
                     (ATK_MD0700_LCD_HEIGHT - textAreaHeight) / 2,
                     textAreaWidth, textAreaHeight, page_buffer[0],
                     fontNameSelect, fontSizeSelect, 1);
            needRerender = 0;
        }
#    endif
    }
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

void show_logo(uint8_t* logoPicture, uint16_t delayTime_ms)
{
    uint16_t time = 0;
    uint8_t  size = 32;
    /* Show Logo */
    if (logoPicture == NULL) {
        atk_md0700_show_string(
            (ATK_MD0700_LCD_WIDTH - size) / 2, (ATK_MD0700_LCD_HEIGHT) / 2,
            size * 2, size, "LOGO", mapping_font_size(size), ATK_MD0700_WHITE);
    }
    /* Delay for Logo */
    delay_ms(delayTime_ms);
}
