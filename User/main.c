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
    uint16_t   startX;
    uint16_t   startY;

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
    TIM3_Init(10000 - 1, 72 - 1);
    res = exfuns_init();
    if (res) {
        infinite_throw("Fail to do exfuns_init()");
    }
    LogParam_Init();
#if ACTION_ONCE
    excuteCommand();
#else
    /* Detect SD Card and mount FATFS for SD Card */
    FIL      my_file;
    uint8_t* pc;
    uint16_t textAreaWidth  = ATK_MD0700_LCD_WIDTH * 4 / 5;
    uint16_t textAreaHeight = ATK_MD0700_LCD_HEIGHT * 4 / 5;
    /* Show Logo */
    show_logo(NULL, 500);
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
    // f_mount(NULL, "0:", 1);

    uint8_t touchState = Touch_State_None;
    uint8_t flag;
    uint8_t slideDirestion;
    uint8_t touchEvent;
    int16_t dx, dy;
    clearTouchFlag(&flag);
    TouchEventInfo_Init();
    //__HAL_TIM_DISABLE(&TIM3_Handler);
    // Button* pBtn =
    //     NewButton(100, 100, 120, 120, RGB888toRGB565(0x000000), 3,
    //     BORDER_ALL);
    // pBtn->str = "Hello world";
    // pBtn->DrawButton(pBtn);
    LinkedList touchQueryQueue; /* Queue for querying of touch operation */
    init_LinkedList(&touchQueryQueue, NodeDataType_Obj);

    const uint16_t menuWidth_1   = 400;
    const uint16_t menuHeight_1  = 600;
    const uint8_t  listItemLimit = 4;
    Border         listBorder    = {
                   .borderColor = RGB888toRGB565(0x000000),
                   .borderWidth = 3,
                   .borderFlag = BORDER_FLAG(BORDER_TOP) | BORDER_FLAG(BORDER_BOTTOM),
    };
    uint16_t buttonFontColor = RGB888toRGB565(0x000000);
    Border   buttonBorder    = {
             .borderColor = RGB888toRGB565(0x000000),
             .borderWidth = 3,
             .borderFlag  = BORDER_NULL,
    };
    Font buttonFont = {
        .fontName  = Font_SimSun,
        .fontSize  = PX24,
        .fontColor = RGB888toRGB565(0x000000),
    };
    List* bookshelf =
        NewList((ATK_MD0700_LCD_WIDTH - menuWidth_1) / 2,
                (ATK_MD0700_LCD_HEIGHT - ATK_MD0700_LCD_HEIGHT / 10 / 2 -
                 menuHeight_1) /
                    2,
                menuWidth_1, menuHeight_1, &listBorder, 70, 55, NULL, 1);
    publicElemData.obj = (Obj*)bookshelf;
    push_tail(&touchQueryQueue, &publicElemData);
    setPublicFont(Font_SimSun, PX24, RGB888toRGB565(0x000000));
    setPublicBorder(RGB888toRGB565(0x000000), 3, BORDER_NULL);
    Textarea* bookshelfHeadline =
        NewTextarea(0, 0, ((Obj*)bookshelf)->width, bookshelf->headlineHeight,
                    &publicFont, &publicBorder, RGB888toRGB565(0xcccccc));
    bookshelfHeadline->str      = bookshelfHeadlineStr;
    bookshelf->headlineTextarea = bookshelfHeadline;
    /**********************/
    /* Navigation Bar */
    setPublicBorder(RGB888toRGB565(0x000000), 3, BORDER_NULL);
    List* navigationBar =
        NewList(0, ATK_MD0700_LCD_HEIGHT / 10 * 9, ATK_MD0700_LCD_WIDTH,
                ATK_MD0700_LCD_HEIGHT / 10, &publicBorder, 0,
                ATK_MD0700_LCD_HEIGHT / 10, NULL, 0);
    publicElemData.obj = (Obj*)navigationBar;
    push_tail(&touchQueryQueue, &publicElemData);
    setPublicFont(Font_SimSun, PX24, RGB888toRGB565(0x000000));
    setPublicBorder(RGB888toRGB565(0x000000), 3, BORDER_NULL);
    setPublicAlignType(AlignHorizonalType_CENTER, AlignVerticalType_MIDDLE);
    Button* buttonBack =
        NewButton(0, 0, 160, navigationBar->itemHeight, &publicFont,
                  &publicBorder, &publicAlignType);
    Button* buttonHome =
        NewButton(160, 0, 160, navigationBar->itemHeight, &publicFont,
                  &publicBorder, &publicAlignType);
    Button* buttonSetting =
        NewButton(320, 0, 160, navigationBar->itemHeight, &publicFont,
                  &publicBorder, &publicAlignType);
    buttonBack->str    = navigationBarString[0];
    buttonHome->str    = navigationBarString[1];
    buttonSetting->str = navigationBarString[2];
    AppendSubListItem(navigationBar, 0, (Obj*)buttonBack);
    AppendSubListItem(navigationBar, 0, (Obj*)buttonHome);
    AppendSubListItem(navigationBar, 0, (Obj*)buttonSetting);
    navigationBar->DrawList(navigationBar);
    /* Navigation Bar */
    /**********************/
    Button* btn[listItemLimit];
    char    str[listItemLimit][30];
    /* Create button */
    setPublicAlignType(AlignHorizonalType_LEFT, AlignVerticalType_MIDDLE);
    for (i = 0; i < listItemLimit; ++i) {
        btn[i] =
            NewButton(0, 0, ((Obj*)bookshelf)->width, bookshelf->itemHeight,
                      &buttonFont, &buttonBorder, &publicAlignType);
    }
    /* Append button to sublist */
    for (i = 0; i < listItemLimit; ++i) {
        AppendSubListItem(bookshelf, i, (Obj*)btn[i]);
    }
    res = f_opendir(&dir, "0:/BOOK");
    check_value_equal(res, FR_OK, "Open dir fail");
    i = 0;
    /* Read the directory and copy the fileName path string to the char array
     * pointed by button[i]*/
    while (res == FR_OK) {
        res = f_readdir(&dir, &fileinfo);
        if ((i >= listItemLimit) || (*fileinfo.fname == 0)) {
            break;
        }
        check_value_equal(res, FR_OK, "read dir fail");
        log_n("File name: %s, altname: %s", fileinfo.fname, fileinfo.altname);
        strcpy(str[i], fileinfo.fname);
        btn[i]->str = str[i];
        ++i;
    }
    bookshelf->DrawList(bookshelf);
    startX = 0;
    startY = ATK_MD0700_LCD_HEIGHT / 10 * 9;
    color  = RGB888toRGB565(0XCCCCCC);
    atk_md0700_fill(startX, startY, ATK_MD0700_LCD_WIDTH - 1,
                    startY + LINE_WIDTH_DEFAULT, &color, SINGLE_COLOR_BLOCK);
    BORDER_ALL;
    Obj* cur_target = NULL;
    // log_n("     %sCnt of queryQueue %d", ARROW_STRING, touchQueryQueue.size);
    while (1) {
#    if 1
        Obj* obj = NULL;
        touchEventUpdate(&touchState, &flag);
        if (touchState == OnRelease) {
            touchEvent = getTouchEvent(flag);
            /* Do things according the touch event */
            switch (cur_target->type) {
                case Obj_Type_Button:
                    if (((Button*)cur_target)->ispressed == BT_PRESSED) {
                        ((Button*)cur_target)->ispressed = BT_UNPRESSED;
                        ((Button*)cur_target)->DrawButton((Button*)cur_target);
                    }
                    break;
                default: break;
            }
            cur_target = NULL;
            // if (pBtn->ispressed) {
            //     pBtn->ispressed = BT_UNPRESSED;
            //     pBtn->DrawButton(pBtn);
            // }
            clearTouchFlag(&flag);
            touchState = Touch_State_None;
        } else if (touchState == OnPress) {
            cur_target =
                touchQueryForWidget(&touchQueryQueue, &point_cur[0], OnPress);
            // obj = touchQuery(&touchQueryQueue, &point_cur[0]);
            // if (obj && (obj->type == Obj_Type_List)) {
            //     obj = touchSubQuery(((List*)obj)->itemList, &point_cur[0]);
            //     if (obj) {
            //         switch (obj->type) {
            //             case Obj_Type_Button:
            //                 ((Button*)obj)->ispressed = BT_PRESSED;
            //                 ((Button*)obj)->DrawButton((Button*)obj);
            //                 break;
            //             default: break;
            //         }
            //     }
            // }
            // if (GUI_isTarget((Obj*)pBtn, &point_cur[0])) {
            //     pBtn->ispressed = BT_PRESSED;
            //     pBtn->DrawButton(pBtn);
            // }
        }
        obj = NULL;
#    else
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
#endif /* Action Once */
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

void fillMainArea(void)
{
    fillArea(0, 0, ATK_MD0700_LCD_WIDTH, ATK_MD0700_LCD_HEIGHT / 10 * 9 - 1,
             GUI_getBackColor());
}

Obj* touchQuery(LinkedList* queryQueue, Position* pos)
{
    Obj*        target = NULL;
    Obj*        obj;
    LinkedNode* node;
    if (queryQueue->nodeDataType == NodeDataType_Obj) {
        node = queryQueue->head;
        while (node) {
            obj = node->nodeData.obj;
            if (GUI_isTarget(obj, pos)) {
                target = obj;
                break;
            }
            node = node->next;
        }
    }
    return target;
}

Obj* touchSubQuery(LinkedList* querySubQueue, Position* pos)
{
    LinkedNode* lineNode;
    LinkedNode* node;
    Obj*        obj    = NULL;
    Obj*        target = NULL;
    lineNode           = querySubQueue->head;
    while (lineNode) {
        node = lineNode->nodeData.subList.head;
        while (node) {
            obj = node->nodeData.obj;
            if (GUI_isTarget(obj, pos)) {
                target = obj;
                break;
            }
            node = node->next;
        }
        lineNode = lineNode->next;
    }
    return target;
}

Obj* touchQueryForWidget(LinkedList* touchQueryQueue,
                         Position*   pos,
                         TouchState  state)
{
    Obj*    obj        = NULL;
    uint8_t BT_pressed = UNPRESSED;
    obj                = touchQuery(touchQueryQueue, pos);
    if (obj && (obj->type == Obj_Type_List)) {
        obj = touchSubQuery(((List*)obj)->itemList, pos);
        if (obj) {
            if (state == OnPress) {
                BT_pressed = BT_UNPRESSED;
            } else if (state == OnRelease) {
                BT_pressed = BT_UNPRESSED;
            }
            switch (obj->type) {
                case Obj_Type_Button:
                    ((Button*)obj)->ispressed = BT_PRESSED;
                    ((Button*)obj)->DrawButton((Button*)obj);
                    break;
                default: break;
            }
        }
    }
    return obj;
}

#if ACTION_ONCE
void excuteCommand(void)
{
    uint8_t       res           = 0;
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
}
#endif
