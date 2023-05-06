#include "main.h"

uint8_t*     lcd_buffer;
uint8_t*     page_buffer[PAGE_NUM];
unsigned int page_buffer_len[PAGE_NUM];

COLOR_DATTYPE color;

extern FIL*    main_file;  // file 1
extern FIL*    ftemp;      // file temp.
extern UINT    br, bw;     // the number of byte really read or written
extern FILINFO fileinfo;   // file information
extern DIR     dir;        // directory

uint8_t       fontNameSelect  = Font_SimSun;
uint8_t       fontSizeSelect  = PX16;
COLOR_DATTYPE foreColorSelect = RGB565_BLACK;
COLOR_DATTYPE backColorSelect = RGB565_WHITE;
uint8_t       needRerender    = 1;
uint16_t booknameIndex = 0; /* 数值上等价于读取的已读取过的bookname的数量 */

uint32_t* pageNumTBL;

/* 书架列表控件 */
List* bookshelf = NULL;
/* 全局导航栏控件 */
List*    navigationBarAtHome    = NULL;
List*    navigationBarOnReading = NULL;
List*    dirList                = NULL; /* 目录表 */
Button** chapterBtn             = NULL; /* 目录表的章节按钮 */
char**   chapterName            = NULL; /* 章节名字符串 */
List*    settingMenu            = NULL; /*设置菜单*/
uint16_t chapterNameIndex = 0; /* 数值上等价于读取的已读取过的章节的数量 */
Button*  buttonDir     = NULL;
Button** booknameBtn   = NULL;
Button*  buttonBack    = NULL;
Button*  buttonHome    = NULL;
Button*  buttonSetting = NULL;
/* 设置菜单 */
Textarea* fontNameTA  = NULL;
Textarea* fontSizeTA  = NULL;
Textarea* foreColorTA = NULL;
Textarea* backColorTA = NULL;

const uint16_t selectBoxBorderWidth    = 3;
const uint16_t selectBoxSpace          = 2;
const uint16_t selectBoxBorderColor    = RGB888toRGB565(0xcccccc);
const uint8_t  settingMenuForeColorCnt = 5;
const uint8_t  settingMenuBackColorCnt = 5;
const uint8_t  settingMenuOperationCnt = 2;
Textarea*      fontNameTa[Font_Name_Cnt];     /* SimSun KaiTi SimHei */
Textarea*      fontSizeTa[Font_Size_Cnt - 1]; /* PX16 PX24 PX32*/
char           fontSizeStr[3][6];
Textarea*      foreColorTa[settingMenuForeColorCnt];
Textarea*      backColorTa[settingMenuForeColorCnt];
COLOR_DATTYPE  fontNameTaBackColor[Font_Name_Cnt] = {
    RGB565_WHITE,
    RGB565_WHITE,
    RGB565_WHITE,
};
COLOR_DATTYPE fontSizeTaBackColor[Font_Size_Cnt - 1] = {
    RGB565_WHITE,
    RGB565_WHITE,
    RGB565_WHITE,
};
COLOR_DATTYPE foreColor[settingMenuForeColorCnt] = {
    /* 黑、红、绿、蓝、白 */
    RGB565_BLACK, RGB565_RED, RGB565_GREEN, RGB565_BLUE, RGB565_WHITE,
};
COLOR_DATTYPE backColor[settingMenuBackColorCnt] = {
    /* 黑、红、绿、蓝、白 */
    RGB888toRGB565(0x222222), RGB888toRGB565(0xf56c6c),
    RGB888toRGB565(0x67c23a), RGB888toRGB565(0x409eff),
    RGB888toRGB565(0xefefef),
};
/* 操作按钮
 *  0：应用
 *  1：取消 */
Button* operationBtn[settingMenuOperationCnt];
char    operationStr[settingMenuOperationCnt][6];
/* 阅读区域 */
Textarea* readingArea = NULL;

LinkedList* curTouchQueryQueue = NULL;
LinkedList  homeTouchQueryQueue; /* 存储widget指针的触摸查询队列 */
LinkedList  readingTouchQueryQueue;
LinkedList  emptyTouchQueryQueue;

uint16_t ROW_LIMIT_PX16;
uint16_t ROW_LIMIT_PX24;
uint16_t ROW_LIMIT_PX32;
uint16_t COLUMN_LIMIT_PX16;
uint16_t COLUMN_LIMIT_PX24;
uint16_t COLUMN_LIMIT_PX32;

/* 目录表的最大长度： 10000个章节 */
const uint32_t DIR_TABLE_MAX_SIZE = 100;
uint32_t*      dirTable           = NULL;
uint16_t       dirTableIndex      = 0; /* 目录表的索引指针 */
uint16_t       dirTableHead       = 0; /* 第一章 */
uint16_t       dirTableTail       = 0;

/* 章节页码表的最大长度： 100页 */
const uint32_t CHAPTER_PAGE_TABLE_MAX_SIZE = 100;
uint32_t       chapterPageTableBuf[2][CHAPTER_PAGE_TABLE_MAX_SIZE];
uint32_t* prevChapterPageTable = chapterPageTableBuf[0]; /* 上一章页码表 */
uint32_t* curChapterPageTable = chapterPageTableBuf[1]; /* 本章的页码表 */

int32_t curChapterPageTableIndex  = 0;  /* 本章的页码表的索引指针 */
int32_t curChapterPageTableTail   = -1; /* 本章的最后一页 */
int32_t prevChapterPageTableIndex = 0;  /* 上一章页码表的索引指针 */
int32_t prevChapterPageTableTail  = -1; /* 上一章的最后一页 */

// uint16_t curChapter = 0;
const uint8_t chapterNameLen = 70;

uint32_t curHeadOffset            = 0;
uint32_t curOffset                = 0;
uint32_t generatingDirTableOffset = 0;
// uint32_t offsetGeneratingCurChapterPageTable = 0;

bool needGenerateDirTable     = true;  /* 需要生成目录表的标志 */
bool generateDirTableFinished = false; /* 目录表生成完成的标志 */

/* 需要生成本章页码表的标志 */
bool needGenerateCurChapterPageTable = true;
/* 首次进入本章页码表生成过程 */
bool firstTimeIntoGeneratingCurChapterPageTable = true;
/* 本章页码表生成完成的标志 */
bool generateCurChapterPageTableFinished = false;

/* 需要生成上一章页码表的标志 */
bool needGeneratePrevChapterPageTable = false;
/* 首次进入上一章页码表生成过程 */
bool firstTimeIntoGeneratingPrevChapterPageTable = true;
/* 上一章页码表生成完成的标志 */
bool generatePrevChapterPageTableFinished = false;

bool noTouchEvent  = true;
bool isOverOnePage = false;

#define BOOKNAME_Buffer_Size 5
char**  booknameBuffer[BOOKNAME_Buffer_Size];
int16_t booknameBufferHead = -1;
int16_t booknameBufferTail = -1;
int16_t booknameBufferCur  = -1;
char**  bookname           = NULL;
#define nextBookNameBufferIndex(index) (((index) + 1) % (BOOKNAME_Buffer_Size))
#define prevBookNameBufferIndex(index)                                         \
    ((((index)-1) >= 0) ? ((index)-1) : (BOOKNAME_Buffer_Size - 1))

uint16_t textAreaWidth  = ATK_MD0700_LCD_WIDTH * 4 / 5;
uint16_t textAreaHeight = ATK_MD0700_LCD_HEIGHT * 4 / 5;

DeviceData g_deviceData = {
    .fontName  = Font_SimSun,
    .fontSize  = PX16,
    .foreColor = RGB888toRGB565(0x000000),
    .backColor = RGB888toRGB565(0xffffff),
};

EbookData g_ebookData = {
    .bookmark         = 0,
    .dirTableTail     = -1,
    .dirTableOffset   = 0,
    .dirTableFinished = false,
};

#define STRING_SIZE 60

#if 0
WCHAR utf16_string[STRING_SIZE];
char  gb2312_string[STRING_SIZE * 2];
#endif

#define readTxt(pFIL, offset, buff, byte)                                      \
    do {                                                                       \
        uint32_t tmpOffset = f_tell(pFIL);                                     \
        f_lseek(pFIL, offset);                                                 \
        f_read(pFIL, buff, byte, &br);                                         \
        buff[(br == byte) ? (byte - 1) : br] = 0;                              \
        log_n("offset[%d]: %s", offset, buff);                                 \
        f_lseek(pFIL, tmpOffset);                                              \
    } while (0)

int main(void)
{
    bool onceTag = true;

    uint8_t    res;
    uint8_t    i = 0;
    uint8_t    tmp_buf[PAGE_SIZE];
    FontHeader fontHeader = {FLAG_OK, FLAG_OK};
    uint16_t   color      = ATK_MD0700_BLACK;
    uint16_t   startX;
    uint16_t   startY;
    uint16_t   size      = getSize(fontSizeSelect);
    uint16_t   lineSpace = getLineSpace(fontSizeSelect);

    // page_buffer[0] = tmp_buf;
    /* 执行必要的初始化操作 */
    InitForMain();

#if ACTION_ONCE
    excuteCommand();
#else
    /* Detect SD Card and mount FATFS for SD Card */
    uint8_t touchState = Touch_State_None;
    uint8_t flag;
    uint8_t slideDirestion;
    uint8_t touchEvent;
    int16_t dx, dy;
    Obj*    cur_target  = NULL;
    Obj*    prev_target = NULL;
    uint8_t listItemLimit;

    uint16_t preRenderX = ((Obj*)readingArea)->x;
    uint16_t preRenderY = ((Obj*)readingArea)->y;
    uint16_t preRenderXLimit =
        ((Obj*)readingArea)->x + ((Obj*)readingArea)->width - 1;
    uint16_t preRenderYLimit =
        ((Obj*)readingArea)->y + ((Obj*)readingArea)->height - 1;
    g_deviceData.foreColor = foreColor[0];
    g_deviceData.backColor = backColor[0];
    fontNameSelect         = g_deviceData.fontName;
    fontSizeSelect         = g_deviceData.fontSize;
    foreColorSelect        = g_deviceData.foreColor;
    backColorSelect        = g_deviceData.backColor;

    /* 展示开机Logo */
    show_logo(NULL, 500);
    atk_md0700_clear(BACKGROUND_COLOR);
    /* 阻塞等待SD卡插入 */
    waiting_for_SD_Card();
    /* 挂载SD卡 */
    mount_SD_Card();
    /* 检查 ex-flash 中的头部信息 */
    res = check_font_header(10);
    /* 若检查 ex-flash 失败，则更新BMP转换表及所有字库文件 */
    if (res) {
        res = update_font();
        check_value_equal(res, 0,
                          "Fail to load UNIGBK.BIN and font library to falsh.");
        fontHeader.fontok = FLAG_OK;
        fontHeader.ugbkok = FLAG_OK;
        W25QXX_Write((uint8_t*)&fontHeader, FONT_HEADER_ADDR,
                     sizeof(fontHeader));
    }
    clearTouchFlag(&flag);
    TouchEventInfo_Init();
    curTouchQueryQueue = &homeTouchQueryQueue;
    init_LinkedList(curTouchQueryQueue, NodeDataType_Obj);
    init_LinkedList(&emptyTouchQueryQueue, NodeDataType_Obj);

    /* 创建导航栏 */
    createNavigationBar();
    /* 创建书架列表 */
    createBookshelf();
    listItemLimit = bookshelf->itemList->size;
    createBooknameBuffer();
    /* 读取目录,并将文件名加载值书架列表的列表项中 */
    res = f_opendir(&dir, "0:/BOOK");
    check_value_equal(res, FR_OK, "Open dir fail");
    CopyBookname(&dir, listItemLimit, bookname, 1);
    refreshBtnName(bookshelf, booknameBtn, bookname,
                   DrawOption_Delay); /* 刷新书名 */
    renderHomePage();                 /* 渲染 Home 界面 */
    createReadingArea();
    createDirList();
    createSettingMenu();

    // f_open(main_file, "0:BOOK/1.TXT", FA_READ);
    // renderText(16375);
    // while (1)
    //     ;
    // char tmpPath[30];
    // strcpy(tmpPath, "0:BOOK/11111111111111111111121111111111111.txt");
    // f_stat(tmpPath, &fileinfo);
    // check_value_equal(res, FR_OK, "Fail to stat file [%s]", tmpPath);
    // log_n("%s%s", tmpPath, ARROW_STRING);
    // log_n("[altname]: %s [fname]: %s", fileinfo.altname, fileinfo.fname);
    // strcpy(tmpPath, "0:BOOK/11111111111111111111131111111111111.txt");
    // f_stat(tmpPath, &fileinfo);
    // check_value_equal(res, FR_OK, "Fail to stat file [%s]", tmpPath);
    // log_n("%s%s", tmpPath, ARROW_STRING);
    // log_n("[altname]: %s [fname]: %s", fileinfo.altname, fileinfo.fname);
    // while(1);
    while (1) {
        if (noTouchEvent == false) {
            /* 状态机设计：
             * - 每次循环更新一次触摸事件生命周期状态
             * - 根据当前状态进行必要的即时响应
             * - 在松手后，产生触摸事件的结果，根据此结果，进行对应的操作 */
            touchEventUpdate(&touchState, &flag); /* 更新触摸事件生命周期 */
            // log_n("[PEN] %d", HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_10));
            if (touchState == TouchState_OnRelease) {
                touchEvent = getTouchEvent(flag);
                /* Do things according the touch event */
                /* 松手后解除控件的即时响应 */
                if (cur_target) {
                    switch (cur_target->type) {
                        /* 恢复按钮的状态 */
                        case Obj_Type_Button:
                            if (((Button*)cur_target)->isPressed ==
                                BT_PRESSED) {
                                ((Button*)cur_target)->isPressed = BT_UNPRESSED;
                                ((Button*)cur_target)
                                    ->DrawButton((Button*)cur_target);
                            }
                            break;
                        default: break;
                    }
                    prev_target = cur_target;
                    cur_target  = NULL;
                }
                if (touchEvent == Touch_Event_Move) {
                    slideDirestion =
                        getSlideDirection(point_prev[0].x, point_prev[0].y,
                                          point_cur[0].x, point_cur[0].y);
                    if (slideDirestion == Slide_Up) {
                        LinkedNodeData data;
                        data.obj = (Obj*)settingMenu;
                        if ((curTouchQueryQueue == &homeTouchQueryQueue) &&
                            (find_data(curTouchQueryQueue, &data) ==
                             NULL)) { /* 在主页？ */
                            /* 书架界面上滑：下一页 */
                            if ((*fileinfo.fname) ||
                                (booknameBufferCur != booknameBufferTail)) {
                                CopyBookname(&dir, listItemLimit, bookname, 1);
                                for (i = 0; i < listItemLimit; ++i) {
                                    booknameBtn[i]->isPressed = BT_UNPRESSED;
                                }
                                refreshBtnName(bookshelf, booknameBtn, bookname,
                                               DrawOption_Immediately);
                            }
                        } else if (curTouchQueryQueue ==
                                   &readingTouchQueryQueue) {
                        }
                    } else if (slideDirestion == Slide_Down) {
                        if (curTouchQueryQueue == &homeTouchQueryQueue) {
                            /* 书架界面下滑：上一页 */
                            if (booknameIndex > 9) {
                                CopyBookname(&dir,
                                             listItemLimit +
                                                 getItemListSize(bookshelf),
                                             bookname, 0);
                                for (i = 0; i < listItemLimit; ++i) {
                                    booknameBtn[i]->isPressed = BT_UNPRESSED;
                                }
                                refreshBtnName(bookshelf, booknameBtn, bookname,
                                               DrawOption_Immediately);
                            }
                        }
                    } else if (slideDirestion == Slide_To_Left) {
                        if (curTouchQueryQueue == &readingTouchQueryQueue) {
                            /* 阅读界面：下一页 */
                            /* 在本章页码表生成完成后，并且电子书尚未读完时，才允许向下翻页
                             */
                            readingNextPage();
                        }
                    } else if (slideDirestion == Slide_To_Right) {
                        /* 阅读界面：上一页 */
                        if (curTouchQueryQueue == &readingTouchQueryQueue) {
                            /* 是否在本章的第一页？ */
                            readingPrevPage();
                        }
                    }
                } else if ((touchEvent == Touch_Event_ShortPress) ||
                           (touchEvent == Touch_Event_LongPress)) {
                    /* 位于主页时，若触摸点位于书架的图书按钮上，则进度阅读界面，并切换组件查询队列
                     */
                    // if (curTouchQueryQueue == &homeTouchQueryQueue)
                    if (prev_target != NULL) {
                        if (prev_target->type == Obj_Type_Button) {
                            ((Button*)prev_target)
                                ->OnClicked((Button*)prev_target);
                        } else if (prev_target->type == Obj_Type_Textarea) {
                            ((Textarea*)prev_target)
                                ->OnClicked((Textarea*)prev_target);
                        }
                        prev_target = NULL;
                    }
                    /* 存在展开的菜单时，若落点不在菜单内，则收回菜单 */
                    /* 阅读界面：发生短按或长按时，根据落点区域判断是否进行翻页以及翻页的方向
                     */
                }
                clearTouchFlag(&flag);
                touchState   = Touch_State_None;
                noTouchEvent = true;
                showIndex(true);
            } else if (touchState == TouchState_OnPress) {
                /* 查询控件队列，落点是否位于某个具体的控件上 */
                cur_target =
                    touchQueryForWidget(curTouchQueryQueue, &point_cur[0]);
                updateWidgetStateOnTouch(cur_target, TouchState_OnPress);
            } else if (touchState == TouchState_Moving) {
                if (cur_target) {
                    if (touchQueryForWidget(curTouchQueryQueue,
                                            &point_cur[0]) != cur_target) {
                        updateWidgetStateOnTouch(cur_target,
                                                 TouchState_OnRelease);
                        cur_target = NULL;
                    }
                }
            }
        } else {
            // preRender();
            /* 无触摸时，只在阅读界面进行后台加载 */
            if (curTouchQueryQueue ==
                &readingTouchQueryQueue) { /* 在阅读界面？ */
                /* 上一章章节页码表 */
                if ((needGeneratePrevChapterPageTable == true) &&
                    (generatePrevChapterPageTableFinished == false)) {
                    /* 上一章页码表 */
                    if (dirTable[dirTableTail] < curOffset) {
                        /* 若前置页码表未准备好，则优先进行前置页码表的生成
                         */
                        goto GENERATE_DIR_TBL;
                    }
                    if (dirTableIndex > 0) {
                        /* 当前不是第一章，则加载上一章页码表 */
                        handleGenerationOfPrevChapterPageTable();
                    }
                } else if ((needGenerateCurChapterPageTable == true) &&
                           (generateCurChapterPageTableFinished == false)) {
                    /* 本章页码表 */
                    if (dirTable[dirTableTail] < curOffset) {
                        /* 若前置页码表未准备好，则优先进行前置页码表的生成
                         */
                        goto GENERATE_DIR_TBL;
                    };
                    handleGenerationOfCurChapterPageTable();
                } else if ((needGenerateDirTable == true) &&
                           (generateDirTableFinished == false)) {
                GENERATE_DIR_TBL:
                    /* 目录表 */
                    /* 处理一个字符找到章节标题后记录到 dirTable 中 */

                    handleGenerationOfDirTable();
                }
            }
        }
    }
#endif /* Action Once */
    while (1) {
        log_n("Main work finished. In infinite loop now...");
        LED_Toggle();
        delay_ms(1500);
    }
}

void InitForMain(void)
{
    uint8_t res;
    /* Some initialization work */
    HAL_Init();                         /* HAL init */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* Set clock: 72Mhz */
    delay_init(72);                     /* delay init */
    led_init();                         /* LED init */
    key_init();                         /* KEY init */
    EXTI_Init();                        /* external interrupt init */
    usart_init(115200);                 /* Serial: 115200 */
    atk_md0700_init(ATK_MD0700_BLACK);  /* LCD panel init */
    SRAM_Init();
    W25QXX_Init();
    TIM3_Init(10000 - 1, 72 - 1);
    res = exfuns_init();
    for (uint8_t i = 0; i < PAGE_NUM; ++i) {
        page_buffer[i] = mymalloc(SRAMIN, PAGE_SIZE);
    }
    check_value_equal(res, 0, "Fail to do exfuns_init()");
    LogParam_Init();
}

/**
 * @description: 在主页底部创建导航栏
 * @return {void}
 */
void createNavigationBar(void)
{
    /* 创建导航栏 */
    setPublicBorder(RGB888toRGB565(0x000000), 3, BORDER_NULL);
    navigationBarAtHome =
        NewList(0, ATK_MD0700_LCD_HEIGHT / 10 * 9, ATK_MD0700_LCD_WIDTH,
                ATK_MD0700_LCD_HEIGHT / 10, &publicBorder, 0,
                ATK_MD0700_LCD_HEIGHT / 10, NULL, 0);
    publicElemData.obj = (Obj*)navigationBarAtHome;
    push_tail(curTouchQueryQueue, &publicElemData);
    /* 创建导航栏中的按钮 */
    setPublicFont(Font_SimSun, PX24, RGB888toRGB565(0x000000));
    setPublicBorder(RGB888toRGB565(0x000000), 5, BORDER_FLAG(BORDER_TOP));
    setPublicAlignType(AlignHorizonalType_CENTER, AlignVerticalType_MIDDLE);
    buttonBack         = NewButton(0, 0, 160, navigationBarAtHome->itemHeight,
                                   LocateType_Relative, &publicFont, &publicBorder,
                                   &publicAlignType, navigationBtnOnClicked);
    buttonHome         = NewButton(160, 0, 160, navigationBarAtHome->itemHeight,
                                   LocateType_Relative, &publicFont, &publicBorder,
                                   &publicAlignType, navigationBtnOnClicked);
    buttonSetting      = NewButton(320, 0, 160, navigationBarAtHome->itemHeight,
                                   LocateType_Relative, &publicFont, &publicBorder,
                                   &publicAlignType, navigationBtnOnClicked);
    buttonBack->str    = navigationBarString[0];
    buttonHome->str    = navigationBarString[1];
    buttonSetting->str = navigationBarString[2];
    AppendSubListItem(navigationBarAtHome, 0, (Obj*)buttonBack);
    AppendSubListItem(navigationBarAtHome, 0, (Obj*)buttonHome);
    AppendSubListItem(navigationBarAtHome, 0, (Obj*)buttonSetting);
}

/**
 * @description: 在主页创建书架列表
 * @return {void}
 */
void createBookshelf(void)
{
    /* 创建书架列表 */
    const uint16_t menuWidth_1  = 400;
    const uint16_t menuHeight_1 = 600;
    uint16_t       listItemLimit;
    setPublicBorder(RGB888toRGB565(0x000000), 3,
                    BORDER_FLAG(BORDER_TOP) | BORDER_FLAG(BORDER_BOTTOM));
    bookshelf =
        NewList((ATK_MD0700_LCD_WIDTH - menuWidth_1) / 2,
                (ATK_MD0700_LCD_HEIGHT - ATK_MD0700_LCD_HEIGHT / 10 / 2 -
                 menuHeight_1) /
                    2,
                menuWidth_1, menuHeight_1, &publicBorder, 70, 55, NULL, 1);
    listItemLimit      = bookshelf->itemList->size;
    publicElemData.obj = (Obj*)bookshelf;
    push_tail(curTouchQueryQueue, &publicElemData);
    /* 创建书架标题栏中的文本域 */
    setPublicFont(Font_SimSun, PX24, RGB888toRGB565(0x000000));
    setPublicBorder(RGB888toRGB565(0x000000), 3, BORDER_NULL);
    setPublicAlignType(AlignHorizonalType_CENTER, AlignVerticalType_MIDDLE);
    Textarea* bookshelfHeadlineTA =
        NewTextarea(0, 0, ((Obj*)bookshelf)->width, bookshelf->headlineHeight,
                    LocateType_Relative, &publicAlignType, &publicFont,
                    &publicBorder, RGB888toRGB565(0xcccccc), NULL);
    bookshelfHeadlineTA->str = bookshelfHeadlineStr;
    SetListHeadlineTextarea(bookshelf, bookshelfHeadlineTA, DrawOption_Delay);
    /* 创建按钮，并添加至书架列表中 */
    booknameBtn = mymalloc(GUI_MALLOC_SOURCE, sizeof(Button*) * listItemLimit);
    check_value_not_equal(booknameBtn, NULL,
                          "Failed to malloc for booknameBtn");
    bookname = (char**)mymalloc(SRAMIN, sizeof(char*) * listItemLimit);
    for (uint16_t i = 0; i < listItemLimit; i++) {
        bookname[i] = (char*)mymalloc(SRAMIN, sizeof(char) * STRING_SIZE * 2);
        memset(bookname[i], 0, sizeof(char) * STRING_SIZE);
    }
    memset(booknameBtn, 0, sizeof(Button*) * listItemLimit);
    setPublicAlignType(AlignHorizonalType_LEFT, AlignVerticalType_MIDDLE);
    setPublicBorder(RGB888toRGB565(0x000000), 3, BORDER_NULL);
    setPublicFont(Font_SimSun, PX24, RGB888toRGB565(0x000000));
    for (uint16_t i = 0; i < listItemLimit; ++i) {
        booknameBtn[i] =
            NewButton(0, 0, ((Obj*)bookshelf)->width, bookshelf->itemHeight,
                      LocateType_Relative, &publicFont, &publicBorder,
                      &publicAlignType, &bookshelfBtnOnClicked);
        check_value_not_equal(booknameBtn[i], NULL,
                              "Fail to malloc for bootnameBtn[%d]", i);
        booknameBtn[i]->str = bookname[i];
    }
}

void createBooknameBuffer(void)
{
    bool     res_ok = 1;
    uint16_t i, j;
    /* 创建bookname环形缓冲区 */
    uint16_t listItemLimit = bookshelf->itemList->size;
    for (i = 0; i < BOOKNAME_Buffer_Size; ++i) {
        booknameBuffer[i] =
            (char**)mymalloc(SRAMIN, sizeof(char*) * listItemLimit);
        if (booknameBuffer == NULL) {
            res_ok = 0;
            break;
        }
    }
    check_value_not_equal(res_ok, 0, "Failed to malloc for booknameBuffer[%d]",
                          i);
    for (i = 0; i < BOOKNAME_Buffer_Size; ++i) {
        for (j = 0; j < listItemLimit; ++j) {
            booknameBuffer[i][j] = (char*)mymalloc(
                GUI_MALLOC_SOURCE, sizeof(char) * STRING_SIZE * 2);
            if (booknameBuffer[i][j] == NULL) {
                res_ok = 0;
                break;
            }
            memset(booknameBuffer[i][j], 0, sizeof(char) * STRING_SIZE * 2);
        }
        if (res_ok == 0) {
            break;
        }
    }
    check_value_not_equal(res_ok, 0,
                          "Failed to malloc for booknameBuffer[%d][%d]", i, j);
}

void createDirList(void)
{
    const uint16_t dirListWidth  = 300;
    const uint16_t dirListHeight = 550;
    uint16_t       listItemLimit;
    uint16_t       posY = ((Obj*)navigationBarOnReading)->y - 1 -
                    buttonDir->border.borderWidth - dirListHeight - 3 - 5;
    setPublicBorder(RGB888toRGB565(0x000000), 3,
                    BORDER_FLAG(BORDER_TOP) | BORDER_FLAG(BORDER_BOTTOM) |
                        BORDER_FLAG(BORDER_RIGHT));
    dirList = NewList(0, posY, dirListWidth, dirListHeight, &publicBorder, 70,
                      50, NULL, 1);
    listItemLimit = dirList->itemList->size;
    /* 创建目录列表标题栏中的文本域 */
    setPublicFont(Font_SimSun, PX24, RGB888toRGB565(0x000000));
    setPublicBorder(RGB888toRGB565(0x000000), 0, BORDER_NULL);
    setPublicAlignType(AlignHorizonalType_CENTER, AlignVerticalType_MIDDLE);
    Textarea* dirListHeadlineTA =
        NewTextarea(0, 0, ((Obj*)dirList)->width, dirList->headlineHeight,
                    LocateType_Relative, &publicAlignType, &publicFont,
                    &publicBorder, RGB888toRGB565(0xcccccc), NULL);
    dirListHeadlineTA->str = dirListHeadlineStr;
    SetListHeadlineTextarea(dirList, dirListHeadlineTA, DrawOption_Delay);
    /* 为目录列表的章节表项申请字符串控件 */
    chapterName = (char**)mymalloc(SRAMIN, sizeof(char*) * listItemLimit);
    check_value_not_equal(chapterName, NULL,
                          "Failed to malloc for chapterName");
    for (uint16_t i = 0; i < listItemLimit; ++i) {
        chapterName[i] = (char*)mymalloc(SRAMIN, sizeof(char) * chapterNameLen);
        memset(chapterName[i], 0, sizeof(char) * chapterNameLen);
    }
    /* 创建章节按钮，添加至目录列表中 */
    setPublicAlignType(AlignHorizonalType_LEFT, AlignVerticalType_MIDDLE);
    setPublicBorder(RGB888toRGB565(0x000000), 0, BORDER_NULL);
    setPublicFont(Font_SimSun, PX16, RGB888toRGB565(0x000000));
    chapterBtn = mymalloc(GUI_MALLOC_SOURCE, sizeof(Button*) * listItemLimit);
    check_value_not_equal(chapterBtn, NULL, "Failed to malloc for chapterBtn");
    memset(chapterBtn, 0, sizeof(Button*) * listItemLimit);
    for (uint16_t i = 0; i < listItemLimit; ++i) {
        chapterBtn[i] =
            NewButton(0, 0, ((Obj*)dirList)->width, dirList->itemHeight,
                      LocateType_Relative, &publicFont, &publicBorder,
                      &publicAlignType, &chapterBtnOnClicked);
        chapterBtn[i]->str = chapterName[i];
    }
}

void createSettingMenu(void)
{
    const uint16_t settingMenuWidth  = 480;
    const uint16_t settingMenuHeight = 370;
    const uint16_t taWidth           = 100;
    const uint16_t btnWidth          = 50;
    const uint16_t btnHeight         = 40;
    uint16_t       restWidth         = settingMenuWidth - taWidth - 30;
    uint16_t       btnCnt            = 0;
    uint16_t       space             = 0;
    uint16_t       startX            = taWidth + 30;
    uint16_t       x                 = 0;
    uint16_t       listItemLimit;
    uint16_t       posY = ((Obj*)navigationBarOnReading)->y - 1 -
                    buttonDir->border.borderWidth - settingMenuHeight;
    setPublicBorder(RGB888toRGB565(0x000000), 3, BORDER_FLAG(BORDER_TOP));
    settingMenu   = NewList(0, posY, settingMenuWidth, settingMenuHeight,
                            &publicBorder, 70, btnHeight + 20, NULL, 0);
    listItemLimit = settingMenu->itemList->size;
    /* 创建目录列表标题栏中的文本域 */
    setPublicFont(Font_SimSun, PX24, RGB888toRGB565(0x000000));
    setPublicBorder(RGB888toRGB565(0x000000), 0, BORDER_NULL);
    setPublicAlignType(AlignHorizonalType_CENTER, AlignVerticalType_MIDDLE);
    Textarea* settingMenuHeadlineTA = NewTextarea(
        0, 0, ((Obj*)settingMenu)->width, settingMenu->headlineHeight,
        LocateType_Relative, &publicAlignType, &publicFont, &publicBorder,
        RGB888toRGB565(0xcccccc), NULL);
    settingMenuHeadlineTA->str = settingMenuHeadlineStr;
    SetListHeadlineTextarea(settingMenu, settingMenuHeadlineTA,
                            DrawOption_Delay);
    /* 创建设置菜单文本域 */
    setPublicFont(Font_SimSun, PX16, RGB888toRGB565(0x000000));
    setPublicBorder(RGB888toRGB565(0x000000), 1, BORDER_NULL);
    setPublicAlignType(AlignHorizonalType_CENTER, AlignVerticalType_MIDDLE);
    fontNameTA  = NewTextarea(0, 0, taWidth, settingMenu->itemHeight,
                              LocateType_Relative, &publicAlignType, &publicFont,
                              &publicBorder, GUI_getBackColor(), NULL);
    fontSizeTA  = NewTextarea(0, 0, taWidth, settingMenu->itemHeight,
                              LocateType_Relative, &publicAlignType, &publicFont,
                              &publicBorder, GUI_getBackColor(), NULL);
    foreColorTA = NewTextarea(
        0, 0, taWidth, settingMenu->itemHeight, LocateType_Relative,
        &publicAlignType, &publicFont, &publicBorder, GUI_getBackColor(), NULL);
    backColorTA = NewTextarea(
        0, 0, taWidth, settingMenu->itemHeight, LocateType_Relative,
        &publicAlignType, &publicFont, &publicBorder, GUI_getBackColor(), NULL);
    fontNameTA->str  = fontNameTaStr;
    fontSizeTA->str  = fontSizeTaStr;
    foreColorTA->str = foreColorTaStr;
    backColorTA->str = backColorTaStr;
    AppendSubListItem(settingMenu, 0, (Obj*)fontNameTA);
    AppendSubListItem(settingMenu, 1, (Obj*)fontSizeTA);
    AppendSubListItem(settingMenu, 2, (Obj*)foreColorTA);
    AppendSubListItem(settingMenu, 3, (Obj*)backColorTA);
    /* 创建设置菜单按钮 */
    createSettingMenuTa(fontNameTa, Font_Name_Cnt, fontNameTaBackColor, startX,
                        restWidth, btnWidth, btnHeight, 0);
    // createSettingMenuBtn(fontNameTa, Font_Name_Cnt, startX, restWidth,
    // btnWidth,
    //                      btnHeight, 0);
    for (uint8_t i = Font_Name_Min; i < Font_Name_Cnt; ++i) {
        fontNameTa[i]->str = fontNameStr[i];
    }

    // createSettingMenuBtn(fontSizeTa, Font_Size_Cnt - 1, startX, restWidth,
    //                      btnWidth, btnHeight, 1);
    createSettingMenuTa(fontSizeTa, Font_Size_Cnt - 1, fontSizeTaBackColor,
                        startX, restWidth, btnWidth, btnHeight, 1);
    for (uint8_t i = PX16 - 1; i < Font_Size_Cnt - 1; ++i) {
        sprintf(fontSizeStr[i], "%d%s", getSize(i + 1), fontSizeStr_HAO);
        fontSizeTa[i]->str = fontSizeStr[i];
    }
    createSettingMenuTa(foreColorTa, settingMenuForeColorCnt, foreColor, startX,
                        restWidth, btnWidth * 9 / 10, (btnHeight - 5) * 9 / 10,
                        2);
    createSettingMenuTa(backColorTa, settingMenuForeColorCnt, backColor, startX,
                        restWidth, btnWidth * 9 / 10, (btnHeight - 5) * 9 / 10,
                        3);
    // createSettingMenuBtn(foreColorTa, settingMenuForeColorCnt, startX,
    //                      restWidth, btnWidth * 9 / 10, (btnHeight - 5) * 9 /
    //                      10, 2);
    // createSettingMenuBtn(backColorTa, settingMenuBackColorCnt, startX,
    //                      restWidth, btnWidth * 9 / 10, (btnHeight - 5) * 9 /
    //                      10, 3);
    startX    = 70;
    restWidth = ((Obj*)settingMenu)->width - startX * 2;
    createSettingMenuBtn(operationBtn, settingMenuOperationCnt, startX,
                         restWidth, btnWidth + 20, btnHeight, 4);
    sprintf(operationStr[0], "%s", operationStr_Apply);
    operationBtn[0]->str = operationStr[0];
    sprintf(operationStr[1], "%s", operationStr_Cancel);
    operationBtn[1]->str = operationStr[1];
}

void createSettingMenuBtn(Button** btnArr,
                          uint16_t btnCnt,
                          uint16_t startX,
                          uint16_t restWidth,
                          uint16_t btnWidth,
                          uint16_t btnHeight,
                          uint16_t lineIndex)
{
    uint16_t x     = startX;
    uint16_t space = (restWidth - btnCnt * btnWidth) / (btnCnt + 1);
    setPublicFont(Font_SimSun, PX16, GUI_getForeColor());
    setPublicBorder(GUI_getForeColor(), 3, BORDER_ALL);
    setPublicAlignType(AlignHorizonalType_CENTER, AlignVerticalType_MIDDLE);
    for (uint8_t i = 0; i < btnCnt; ++i) {
        x += space;
        btnArr[i] = NewButton(x, (settingMenu->itemHeight - btnHeight) / 2,
                              btnWidth, btnHeight, LocateType_Relative,
                              &publicFont, &publicBorder, &publicAlignType,
                              &settingMenuBtnOnClicked);
        AppendSubListItem(settingMenu, lineIndex, (Obj*)btnArr[i]);
        x += btnWidth;
    }
}

void createSettingMenuTa(Textarea**     taArr,
                         uint16_t       taCnt,
                         COLOR_DATTYPE* colorArr,
                         uint16_t       startX,
                         uint16_t       restWidth,
                         uint16_t       taWidth,
                         uint16_t       taHeight,
                         uint16_t       lineIndex)
{
    uint16_t x     = startX;
    uint16_t space = (restWidth - taCnt * taWidth) / (taCnt + 1);
    setPublicFont(Font_SimSun, PX16, GUI_getForeColor());
    setPublicBorder(GUI_getForeColor(), 1, BORDER_ALL);
    setPublicAlignType(AlignHorizonalType_CENTER, AlignVerticalType_MIDDLE);
    for (uint8_t i = 0; i < taCnt; ++i) {
        x += space;
        taArr[i] = NewTextarea(x, (settingMenu->itemHeight - taHeight) / 2,
                               taWidth, taHeight, LocateType_Relative,
                               &publicAlignType, &publicFont, &publicBorder,
                               colorArr[i], &settingMenuTaOnClicked);
        AppendSubListItem(settingMenu, lineIndex, (Obj*)taArr[i]);
        x += taWidth;
    }
}

void chapterBtnOnClicked(Button* button)
{
    log_n("chapterNameIndex: %d, getItemListSize(dirList): %d",
          chapterNameIndex, getItemListSize(dirList));
    log_n("dirTable[Tail]: %d, f_size: %d", dirTable[dirTableTail],
          f_size(main_file));
    uint32_t chapterNum =
        chapterNameIndex -
        ((dirTable[dirTableTail] == f_size(main_file)) ? 1 : 0);
    chapterNum -= getItemListSize(dirList);
    for (int i = 0; i < dirList->itemList->size; ++i) {
        if (button == chapterBtn[i]) {
            chapterNum += i;
            break;
        }
    }
    if (chapterNum <= dirTableTail) {
        uint16_t       chapter_old = dirTableIndex;
        LinkedNodeData data;
        data.obj = (Obj*)dirList;
        deleteNode(curTouchQueryQueue, find_data(curTouchQueryQueue, &data));
        fillMainArea(GUI_getBackColor());
        log_n("Chapter jump. chapterNum: %d", chapterNum);
        curOffset = dirTable[chapterNum + dirTableHead];
        LED0(1);
        renderText(curOffset);
        /* 根据原来的章节序号，确定是否需要更新本章页码表以及上一章页码表 */
        dirTableIndex = getDirTableIndex();
        if (dirTableIndex > chapter_old) {
            /* 如果是向后跳转，必须重新生成本章页码表 */
            generateCurChapterPageTableFinished        = false;
            firstTimeIntoGeneratingCurChapterPageTable = true;
            needGenerateCurChapterPageTable            = true;
            /* 如果是向后跳转到下一个章节，且本章页码表已生成完成，则将本章页码表传递给上一章页码表
             */
            /* 否则，要求跳转后重新生成上章页码表 */
            if ((dirTableIndex == chapter_old + 1) &&
                (generateCurChapterPageTableFinished == true)) {
                passChapterPageTableFromCurToPrev();
            } else {
                generatePrevChapterPageTableFinished        = false;
                firstTimeIntoGeneratingPrevChapterPageTable = true;
            }
        } else if (dirTableIndex < chapter_old) {
            /* 如果是向前跳转到上一个章节，切上章页码表已生成完成，则将上章页码表传递给本章页码表
             */
            /* 否则，要求跳转后重新生成本章页码表 */
            /* 如果是向前跳转，只要跳转后不是第一章，则要求生成上章页码表 */
            if (dirTableIndex > dirTableHead) {
                generatePrevChapterPageTableFinished        = false;
                firstTimeIntoGeneratingPrevChapterPageTable = true;
            }
            if ((dirTableIndex == chapter_old - 1) &&
                (generatePrevChapterPageTableFinished == true)) {
                passChapterPageTableFromPrevToCur();
                curChapterPageTableIndex = 0;
            } else {
                needGenerateCurChapterPageTable            = true;
                firstTimeIntoGeneratingCurChapterPageTable = true;
                generateCurChapterPageTableFinished        = false;
            }
            /* 由于need信号在本章页码表生成算法结尾执行，故要考虑设置信号的时机
             */
            if (generateCurChapterPageTableFinished == true) {
                needGeneratePrevChapterPageTable = true;
            }
        }
    }
}

void settingMenuBtnOnClicked(Button* button)
{
    Button* curSelectBtn = NULL;
#if 0
    curSelectBtn         = fontNameTa[fontNameSelect];
    for (uint8_t i = Font_Name_Min; i < Font_Name_Cnt; ++i) {
        if ((fontNameTa[i] == button) && (curSelectBtn != button)) {
            clearSelectBox((Obj*)curSelectBtn, selectBoxBorderWidth,
                           selectBoxSpace);
            drawSelectBox((Obj*)fontNameTa[i], selectBoxBorderWidth,
                          selectBoxSpace, selectBoxBorderColor);
            log_n("Set [fontNameSelect] from %d to %d", fontNameSelect, i);
            fontNameSelect = i;
            return;
        }
    }
    curSelectBtn = fontSizeTa[fontSizeSelect - 1];
    for (uint8_t i = PX16 - 1; i < Font_Size_Cnt - 1; ++i) {
        if ((fontSizeTa[i] == button) && (curSelectBtn != button)) {
            clearSelectBox((Obj*)curSelectBtn, selectBoxBorderWidth,
                           selectBoxSpace);
            drawSelectBox((Obj*)fontSizeTa[i], selectBoxBorderWidth,
                          selectBoxSpace, selectBoxBorderColor);
            log_n("Set [fontSizeSelect] from %d to %d", fontSizeSelect, i + 1);
            fontSizeSelect = i + 1;
            return;
        }
    }
#endif
    if (button == operationBtn[0]) {
        LinkedNode* node = NULL;
        bool        isFontNameChanged =
            (g_deviceData.fontName != fontNameSelect) ? true : false;
        bool isFontSizeChanged =
            (g_deviceData.fontSize != fontSizeSelect) ? true : false;
        bool isColorChanged = ((g_deviceData.foreColor != foreColorSelect) ||
                               (g_deviceData.backColor != backColorSelect)) ?
                                  true :
                                  false;
        /* “应用”按钮 */
        log_n("[Apply]%s", ARROW_STRING);
        if (isFontNameChanged == true) {
            g_deviceData.fontName      = fontNameSelect;
            readingArea->font.fontName = fontNameSelect;
        }
        /* 若字号发生了修改，则需要重新生成本章、上章页码表 */
        if (isFontSizeChanged == true) {
            g_deviceData.fontSize = fontSizeSelect;
            /* 要求生成本章页码表 */
            needGenerateCurChapterPageTable            = true;
            firstTimeIntoGeneratingCurChapterPageTable = true;
            generateCurChapterPageTableFinished        = false;
            /* 要求生成上章页码表 */
            firstTimeIntoGeneratingPrevChapterPageTable = false;
            generatePrevChapterPageTableFinished        = false;
            readingArea->font.fontSize                  = fontSizeSelect;
            LED0(1);
        }
        if (isColorChanged == true) {
            g_deviceData.foreColor = foreColorSelect;
            g_deviceData.backColor = backColorSelect;
            GUI_setForeColor(foreColorSelect);
            GUI_setBackColor(backColorSelect);
            /* 更新阅读界面队列中的控件的前景色、背景色 */
            node = curTouchQueryQueue->head;
            while (node) {
                updateWidgetColor(node->nodeData.obj);
                node = node->next;
            }
            /* 更新阅读界面队列中的控件的前景色、背景色 */
            node = readingTouchQueryQueue.head;
            while (node) {
                updateWidgetColor(node->nodeData.obj);
                node = node->next;
            }
            // updateWidgetColor((Obj*)readingArea);
            updateWidgetColor((Obj*)dirList);
            /* 阅读界面下重绘底部导航栏 */
            // if (curTouchQueryQueue == &readingTouchQueryQueue) {
            //     node =
            //     navigationBarOnReading->itemList->head->nodeData.subList
            //                .head;
            //     while (node) {
            //         draw_widget(node->nodeData.obj);
            //         node = node->next;
            //     }
            // }
        }
        navigationBtnOnClicked(buttonSetting);
        if ((isColorChanged == true) &&
            (curTouchQueryQueue == &readingTouchQueryQueue)) {
            draw_widget((Obj*)navigationBarOnReading);
        }
    } else if (button == operationBtn[1]) {
        /* “取消”按钮 */
        log_n("[Cancel]%s", ARROW_STRING);
        fontNameSelect  = g_deviceData.fontName;
        fontSizeSelect  = g_deviceData.fontSize;
        foreColorSelect = g_deviceData.foreColor;
        backColorSelect = g_deviceData.backColor;
        navigationBtnOnClicked(buttonSetting);
    }
}

void settingMenuTaOnClicked(Textarea* textarea)
{
    Textarea* curSelectTa = NULL;
    curSelectTa           = fontNameTa[fontNameSelect];
    for (uint8_t i = Font_Name_Min; i < Font_Name_Cnt; ++i) {
        if ((fontNameTa[i] == textarea) && (curSelectTa != textarea)) {
            clearSelectBox((Obj*)curSelectTa, selectBoxBorderWidth,
                           selectBoxSpace);
            drawSelectBox((Obj*)fontNameTa[i], selectBoxBorderWidth,
                          selectBoxSpace, selectBoxBorderColor);
            log_n("Set [fontNameSelect] from %d to %d", fontNameSelect, i);
            fontNameSelect = i;
            return;
        }
    }

    curSelectTa = fontSizeTa[fontSizeSelect - 1];
    for (uint8_t i = PX16 - 1; i < Font_Size_Cnt - 1; ++i) {
        if ((fontSizeTa[i] == textarea) && (curSelectTa != textarea)) {
            clearSelectBox((Obj*)curSelectTa, selectBoxBorderWidth,
                           selectBoxSpace);
            drawSelectBox((Obj*)fontSizeTa[i], selectBoxBorderWidth,
                          selectBoxSpace, selectBoxBorderColor);
            log_n("Set [fontSizeSelect] from %d to %d", fontSizeSelect, i + 1);
            fontSizeSelect = i + 1;
            return;
        }
    }

    curSelectTa = foreColorTa[getColorIndex(foreColor, settingMenuForeColorCnt,
                                            foreColorSelect)];
    for (uint8_t i = 0; i < settingMenuForeColorCnt; ++i) {
        if ((foreColorTa[i] == textarea) && (curSelectTa != textarea)) {
            clearSelectBox((Obj*)curSelectTa, selectBoxBorderWidth,
                           selectBoxSpace);
            drawSelectBox((Obj*)foreColorTa[i], selectBoxBorderWidth,
                          selectBoxSpace, selectBoxBorderColor);
            log_n("Set [foreColorSelect] from 0X%04X to 0X%04X",
                  foreColorSelect, foreColor[i]);
            foreColorSelect = foreColor[i];
            return;
        }
    }

    curSelectTa = backColorTa[getColorIndex(backColor, settingMenuBackColorCnt,
                                            backColorSelect)];
    for (uint8_t i = 0; i < settingMenuBackColorCnt; ++i) {
        if ((backColorTa[i] == textarea) && (curSelectTa != textarea)) {
            clearSelectBox((Obj*)curSelectTa, selectBoxBorderWidth,
                           selectBoxSpace);
            drawSelectBox((Obj*)backColorTa[i], selectBoxBorderWidth,
                          selectBoxSpace, selectBoxBorderColor);
            log_n("Set [backColorSelect] from 0X%04X to 0X%04X",
                  backColorSelect, backColor[i]);
            backColorSelect = backColor[i];
            return;
        }
    }
}

void updateWidgetColor(Obj* obj)
{
    if (obj->type == Obj_Type_Button) {
        Button* button             = (Button*)obj;
        button->font.fontColor     = GUI_getForeColor();
        button->border.borderColor = GUI_getForeColor();
    } else if (obj->type == Obj_Type_Textarea) {
        Textarea* textarea           = (Textarea*)obj;
        textarea->border.borderColor = GUI_getForeColor();
        if (textarea->str != NULL) {
            textarea->font.fontColor = GUI_getForeColor();
            textarea->backColor      = GUI_getBackColor();
        }
    } else if (obj->type == Obj_Type_List) {
        List*       list    = (List*)obj;
        LinkedNode* subList = list->itemList->head;
        while (subList) {
            LinkedNode* node = subList->nodeData.subList.head;
            while (node) {
                updateWidgetColor(node->nodeData.obj);
                node = node->next;
            }
            subList = subList->next;
        }
    }
}

/**
 * @description:
 * @return {void}
 */
void passChapterPageTableFromPrevToCur(void)
{
    swapVal(&curChapterPageTable, &prevChapterPageTable,
            ValType_UINT32_POINTER);
    curChapterPageTableIndex = curChapterPageTableTail =
        prevChapterPageTableTail;
    prevChapterPageTableTail            = -1;
    prevChapterPageTableIndex           = 0;
    needGenerateCurChapterPageTable     = false;
    generateCurChapterPageTableFinished = true;
    if (dirTableIndex > dirTableHead) {
        /* 如果不是第一章，则需重新生成上章页码表 */
        needGeneratePrevChapterPageTable            = true;
        firstTimeIntoGeneratingPrevChapterPageTable = true;
        generatePrevChapterPageTableFinished        = false;
        LED0(1);
    }
}

/**
 * @description:
 * @return {void}
 */
void passChapterPageTableFromCurToPrev(void)
{
    prevChapterPageTableTail  = curChapterPageTableTail;
    prevChapterPageTableIndex = prevChapterPageTableTail;
    curChapterPageTableTail   = -1;
    curChapterPageTableIndex  = 0;
    swapVal(&curChapterPageTable, &prevChapterPageTable,
            ValType_UINT32_POINTER);
    /* 不需要重新生成上一章页码表 */
    needGeneratePrevChapterPageTable     = false;
    generatePrevChapterPageTableFinished = true;
    /* 需要重新生成本章页码表 */
    needGenerateCurChapterPageTable            = true;
    firstTimeIntoGeneratingCurChapterPageTable = true;
    generateCurChapterPageTableFinished        = false;
    LED0(1);
}

void showIndex(bool showFlag)
{
#if 0
    /* 查看当前dir指针偏移 */
    char           buf[30]     = {0};
    const uint16_t indexWidth  = 150;
    const uint16_t indexHeight = 30;
    fillArea(0, 0, indexWidth, indexHeight, GUI_getBackColor());
    fillArea(ATK_MD0700_LCD_WIDTH - indexWidth, 0, indexWidth, indexHeight,
             GUI_getBackColor());
    if (showFlag == true) {
        sprintf(buf, "Chapter: %2d", getDirTableIndex());
        Show_Str(0, 0, indexWidth, indexHeight, (uint8_t*)buf, strlen(buf),
                 Font_SimSun, PX24, 1, &isOverOnePage);
        sprintf(buf, "Page: %2d", getCurPageIndex());
        Show_Str(ATK_MD0700_LCD_WIDTH - indexWidth, 0, indexWidth, indexHeight,
                 (uint8_t*)buf, strlen(buf), Font_SimSun, PX24, 1,
                 &isOverOnePage);
    }
#endif
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

/**
 * @description: 等待插入SD卡
 * @return {void}
 */
void waiting_for_SD_Card(void)
{
    uint8_t onceFlag = true;
    while (SD_Init()) {
        if (onceFlag == true) {
            Show_Str_Mid(0, 0, ATK_MD0700_LCD_WIDTH - 1,
                         ATK_MD0700_LCD_HEIGHT - 1, SDCardDetectErrorHint,
                         Font_SimSun, PX24, 16, 0);
            onceFlag = false;
        }
        log_n("SD Card Error! Please check!");
        HAL_Delay(500);
    }
    atk_md0700_clear(ATK_MD0700_WHITE);
}

/**
 * @description: 挂载SD卡
 * @return {void}
 */
void mount_SD_Card(void)
{
    if (f_mount(fs[FAT_DRV_SDCARD], "0:", 1) != FR_OK) {
        infinite_throw("Fail to mount SD Card.");
    }
}

/**
 * @description: 显示Logo
 * @param {uint8_t*} logoPicture 图片指针
 * @param {uint16_t} delayTime_ms 延时展示的时长 (ms)
 * @return {void}
 */
void show_logo(uint8_t* logoPicture, uint16_t delayTime_ms)
{
    uint16_t time    = 0;
    uint8_t  size    = 32;
    char     str[20] = "EbookReader";
    uint16_t len     = strlen(str);
    /* Show Logo */
    if (logoPicture == NULL) {
        atk_md0700_show_string((ATK_MD0700_LCD_WIDTH - size / 2 * len) / 2,
                               (ATK_MD0700_LCD_HEIGHT - size) / 2, size * len,
                               size, str, mapping_font_size(size),
                               ATK_MD0700_WHITE);
    }
    /* Delay for Logo */
    delay_ms(delayTime_ms);
}

/**
 * @description: 按给定的颜色填充指定区域
 * @return {void}
 */
void fillMainArea(COLOR_DATTYPE color)
{
    fillArea(0, 0, ATK_MD0700_LCD_WIDTH,
             ATK_MD0700_LCD_HEIGHT - ((Obj*)buttonHome)->height -
                 buttonHome->border.borderWidth,
             color);
}

void clearReadingArea(void)
{
    fillArea(((Obj*)readingArea)->x, ((Obj*)readingArea)->y,
             ((Obj*)readingArea)->width, ((Obj*)readingArea)->height,
             GUI_getBackColor());
}

void addWidgetToQuery() {}

/**
 * @description: 查询触摸点是否落在某个控件上，不具体到某个子控件
 * @param {LinkedList*} queryQueue 父控件链表
 * @param {Position*} pos 触摸点坐标
 * @return {Obj*} 指向目标父控件的指针
 */
Obj* touchQuery(LinkedList* queryQueue, Position* pos)
{
    Obj*        target = NULL;
    Obj*        obj;
    LinkedNode* node;
    if (queryQueue->nodeDataType == NodeDataType_Obj) {
        node = queryQueue->head;
        while (node) {
            obj = node->nodeData.obj;
            if (obj && GUI_isTarget(obj, pos)) {
                target = obj;
                break;
            }
            node = node->next;
        }
    }
    return target;
}

/**
 * @description: 查询本次触摸是否落在菜单中的子控件上，若是则将其返回
 * @param {LinkedList*} querySubQueue 菜单的控件链表
 * @param {Position*} pos 触摸点坐标
 * @return {Obj*} 指向触摸事件的目标子控件的指针
 */
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

/**
 * @description: 查询触摸操作的对象，返回具体的子控件对象
 * @param {LinkedList*} touchQueryQueue 进行触摸查询的控件队列
 * @param {Position*} pos 触摸点坐标
 * @param {TouchState} state 当前触摸事件的生命周期状态
 * @return {Obj*} 指向触摸点所在的具体控件的指针
 */
Obj* touchQueryForWidget(LinkedList* touchQueryQueue, Position* pos)
{
    Obj*    obj        = NULL;
    uint8_t BT_pressed = UNPRESSED;
    obj                = touchQuery(touchQueryQueue, pos);
    if (obj && (obj->type == Obj_Type_List)) {
        obj = touchSubQuery(((List*)obj)->itemList, pos);
    }
    return obj;
}

/**
 * @description: 根据此时的触摸事件生命周期的状态，改变控件的状态
 * @param {Obj*} obj 触摸事件操作的对象
 * @param {TouchState} state 当前的触摸生命周期的状态
 * @return {void}
 */
void updateWidgetStateOnTouch(Obj* obj, TouchState state)
{
    if (obj) {
        switch (obj->type) {
            case Obj_Type_Button:
                if (state == TouchState_OnPress) {
                    ((Button*)obj)->isPressed = BT_PRESSED;
                    // clearBorder(obj);
                } else if (state == TouchState_OnRelease) {
                    ((Button*)obj)->isPressed = BT_UNPRESSED;
                    // drawBorder(obj);
                }
                ((Button*)obj)->DrawButton((Button*)obj);
                break;
            default: break;
        }
    }
}

/**
 * @description: 将目录指针回滚指定步数
 * @param {DIR*} dir 目录
 * @param {uint8_t} limit 回滚步数
 * @return {void}
 */
void readDirRevese(DIR* dir, uint8_t limit)
{
    // uint16_t bookCntInCurPage = getItemListSize(bookshelf);
    uint8_t res;
    booknameIndex -= limit;
    res = f_readdir(dir, NULL);
    check_value_equal(res, FR_OK, "Failed to reverse to head.");
    /* 已回退至缓冲区首部，则需要借助f_readdir接口进行回滚，相当于重新打开 */
    for (int i = 0; i < booknameIndex; ++i) {
        res = f_readdir(dir, &fileinfo);
        check_value_equal(res, FR_OK, "Failed to readdir [%d].", i);
    }
}

/**
 * @description: 遍历目录，将指定数量的文件名
 * @param {DIR*} dir 目录
 * @param {uint8_t} limit 记录的文件名的数量
 * @param {char**} bookname 存储目录名的二维数组
 * @return {void}
 */
void CopyBookname(DIR* dir, uint8_t limit, char** bookname, bool forward)
{
    uint8_t     i                  = 0;
    uint8_t     res                = FR_OK;
    static bool readFromBufferFlag = false;
    /* 第一次拷贝，初始化缓冲区index */
    if (forward) {
        /* 前进式拷贝，进行index的处理 */
        if (booknameBufferCur == -1) {
            /* 首次前进式拷贝，需要readir */
            booknameBufferCur = booknameBufferTail = booknameBufferHead = 0;
        } else {
            /* 非首次拷贝，需要readdir */
            /* cur已在最后一个buffer，则除了cur自增，还要让tail自增 */
            // 未发生回退，或者回退已完全恢复 */
            /* 不管是否发生过回退，都需要移动cur */
            if (booknameBufferCur == booknameBufferTail) {
                readFromBufferFlag = false;
            }
            booknameBufferCur = nextBookNameBufferIndex(booknameBufferCur);
            /* 如果（未发生过回退，或回退已恢复）或（发生过回滚），则需要readdir
             */
            if ((booknameBufferCur == booknameBufferTail) ||
                (readFromBufferFlag == false)) {
                if (nextBookNameBufferIndex(booknameBufferTail) ==
                    booknameBufferCur) {
                    booknameBufferTail =
                        nextBookNameBufferIndex(booknameBufferTail);
                }
                if (booknameBufferTail == booknameBufferHead) {
                    booknameBufferHead =
                        nextBookNameBufferIndex(booknameBufferHead);
                }
            }
        }
        if (readFromBufferFlag == false) { /* 需要raeddir？ */
            /* 通过多次调用readdir遍历指定数量的文件名 */
            while (res == FR_OK) {
                res = f_readdir(dir, &fileinfo);
                // log_n("In copy bookname. [%d] dir->fn: [%8s]", i, dir->fn);
                /* 若已遍历整个目录, 则退出 */
                if (*fileinfo.fname == 0) {
                    break;
                }
                booknameIndex++; /* 下一个会被读取的文件项的编号, 自0开始 */
                check_value_equal(res, FR_OK, "read dir fail");
                strcpy(booknameBuffer[booknameBufferCur][i], fileinfo.fname);
                ++i;
                /* 读至足够 limit 项则退出 */
                if (i >= limit) {
                    break;
                }
            }
            /* 若目录剩余项不足 limit, 将未获取到字符串的数组置为空字符串 */
            if (i < limit) {
                for (; i < limit; ++i) {
                    booknameBuffer[booknameBufferCur][i][0] = 0;
                }
            }
        }
        for (uint8_t j = 0; j < limit; ++j) {
            strcpy(bookname[j], booknameBuffer[booknameBufferCur][j]);
            if ((readFromBufferFlag == true) && (bookname[j][0])) {
                ++booknameIndex;
            }
        }
    } else {
        /* 回退式拷贝 */
        /* 刷新上一页[1] or 重刷本页？ */
        if (limit >= bookshelf->itemList->size) { /* 刷新上一页？ */
            if (booknameBufferCur != booknameBufferHead) {
                booknameBufferCur  = prevBookNameBufferIndex(booknameBufferCur);
                readFromBufferFlag = true;
                booknameIndex -= getItemListSize(bookshelf);
            } else {
                readFromBufferFlag = false;
                /* readdir指针回滚，再拷贝到buff[cur]中 */
                readDirRevese(dir, bookshelf->itemList->size +
                                       getItemListSize(bookshelf));
                for (int i = 0; i < bookshelf->itemList->size; ++i) {
                    res = f_readdir(dir, &fileinfo);
                    check_value_equal(res, FR_OK, "Failed to readdir [%d].", i);
                    strcpy(booknameBuffer[booknameBufferCur][i],
                           fileinfo.fname);
                    ++booknameIndex;
                }
            }
        } else { /* 刷新本页 */
            // booknameIndex -= getItemListSize(bookshelf);
        }
    }
    /* 将buff[cur]中的bookname拷贝出来（刷新上页、重刷本页） */
    for (int i = 0; i < bookshelf->itemList->size; ++i) {
        strcpy(bookname[i], booknameBuffer[booknameBufferCur][i]);
    }
}

/**
 * @description: 刷新书架列表的书名
 * @param {List*     } list 书架列表
 * @param {Button**  } booknameBtn 图书按钮数组
 * @param {char**    } bookname 书名数组
 * @param {DrawOption} drawOption 绘制选项（立即绘制，或稍后手动绘制）
 * @return {void}
 */
void refreshBtnName(List*      list,
                    Button**   nameBtnArr,
                    char**     nameArr,
                    DrawOption drawOption)
{
    uint8_t i = 0;
    for (i = 0; i < list->itemList->size; i++) {
        // 若无法搜索到书名，后续内容不渲染
        if (nameArr[i][0] == 0) {
            break;
        }
        // 如果按钮中有书名，但按钮曾被移出链表，则重新添加回来
        LinkedNodeData data;
        data.obj = (Obj*)nameBtnArr[i];
        if (find_data(getSubList(list, i), &data) == NULL) {
            AppendSubListItem(list, i, (Obj*)nameBtnArr[i]);
        }
    }
    // 若存在空字符串的按钮，则移出链表
    for (; i < list->itemList->size; i++) {
        LinkedNodeData data;
        data.obj = (Obj*)nameBtnArr[i];
        nameBtnArr[i]->DrawButton(nameBtnArr[i]);
        deleteNode(getSubList(list, i), find_data(getSubList(list, i), &data));
    }
    /* 根据参数决定是否立即重新绘制 */
    if (drawOption == DrawOption_Immediately) {
        redrawListItem(list);
    }
}

void copyChapterName(void)
{
    uint16_t        i;
    static uint16_t chapterCnt = 0;
    uint32_t        tmpOffset  = f_tell(main_file);
    if (dirTableTail > 0) {
        chapterCnt = dirTableTail - dirTableHead + 1;
    } else {
        return;
    }
    for (i = 0; i < dirList->itemList->size; ++i) {
        if (chapterNameIndex >= chapterCnt) {
            break;
        }
        /* 读章节名 */
        uint16_t pos = 0;
        f_lseek(main_file, dirTable[chapterNameIndex + dirTableHead]);
        f_read(main_file, chapterName[i], chapterNameLen, &br);
        bool     isHz  = false;
        uint16_t chPos = 0;
        uint16_t j;
        for (j = 0; j < br; ++j) {
            if (isHz == false) {
                if (chapterName[i][j] & 0x80) {
                    isHz  = true;
                    chPos = j;
                    continue;
                }
                chPos = j;
                if ((chapterName[i][j] == '.') || (chapterName[i][j] == '?') ||
                    (chapterName[i][j] == '!') || (chapterName[i][j] == '\n') ||
                    (chapterName[i][j] == '\r')) {
                    break;
                }
            } else {
                isHz = false;
                if ((strncmp(chapterName[i] + j, gbk_full_stop, 2) == 0) ||
                    (strncmp(chapterName[i] + j, gbk_exclamatory_mark, 2) ==
                     0) ||
                    (strncmp(chapterName[i] + j, gbk_question_mark, 2) == 0)) {
                    break;
                }
            }
        }
        // if ((j + ((isHz == true) ? 1 : 0)) >= br) {
        //     chapterName[j] = 0;
        // }
        chapterName[i][j] = 0;
        ++chapterNameIndex;
    }
    for (; i < dirList->itemList->size; ++i) {
        chapterName[i][0] = 0;
    }
    f_lseek(main_file, tmpOffset);
}

/**
 * @description: 刷新主页
 * @return {void}
 */
void renderHomePage(void)
{
    /* 刷新主界面书架、导航栏 */
    atk_md0700_clear(GUI_getBackColor());
    bookshelf->DrawList(bookshelf);
    navigationBarAtHome->DrawList(navigationBarAtHome);
}

void createReadingArea(void)
{
    const uint16_t textAreaWidth  = ATK_MD0700_LCD_WIDTH * 4 / 5;
    const uint16_t textAreaHeight = ATK_MD0700_LCD_HEIGHT * 9 / 10 - 100;
    init_LinkedList(&readingTouchQueryQueue, NodeDataType_Obj);
    setPublicFont((FontName)g_deviceData.fontName,
                  (FontSize)g_deviceData.fontSize, g_deviceData.foreColor);
    setPublicBorder(RGB888_BLACK, 3, BORDER_ALL);
    // readingArea =
    //     NewTextarea((ATK_MD0700_LCD_WIDTH - textAreaWidth) / 2,
    //                 (ATK_MD0700_LCD_HEIGHT - textAreaHeight) / 2,
    //                 textAreaWidth, textAreaHeight, LocateType_Absolute,
    //                 &publicFont, &publicBorder, RGB888toRGB565(0xffffff));
    setPublicAlignType(AlignHorizonalType_LEFT, AlignVerticalType_BOTTOM);
    readingArea = NewTextarea(
        (ATK_MD0700_LCD_WIDTH - textAreaWidth) / 2, 80, textAreaWidth,
        textAreaHeight, LocateType_Absolute, &publicAlignType, &publicFont,
        &publicBorder, GUI_getBackColor(), NULL);
    readingArea->str   = page_buffer[0];
    publicElemData.obj = (Obj*)readingArea;
    push_tail(&readingTouchQueryQueue, &publicElemData);
    /* 建立阅读界面的导航栏 */
    setPublicBorder(RGB888toRGB565(0x000000), 3, BORDER_NULL);
    navigationBarOnReading =
        NewList(0, ATK_MD0700_LCD_HEIGHT / 10 * 9, ATK_MD0700_LCD_WIDTH,
                ATK_MD0700_LCD_HEIGHT / 10, &publicBorder, 0,
                ATK_MD0700_LCD_HEIGHT / 10, NULL, 0);
    setPublicFont(Font_SimSun, PX24, RGB888toRGB565(0x000000));
    setPublicBorder(RGB888toRGB565(0x000000), 5, BORDER_FLAG(BORDER_TOP));
    setPublicAlignType(AlignHorizonalType_CENTER, AlignVerticalType_MIDDLE);
    buttonDir      = NewButton(0, 0, 160, navigationBarOnReading->itemHeight,
                               LocateType_Relative, &publicFont, &publicBorder,
                               &publicAlignType, navigationBtnOnClicked);
    buttonDir->str = navigationBarString[3];
    AppendSubListItem(navigationBarOnReading, 0, (Obj*)buttonDir);
    AppendSubListItem(navigationBarOnReading, 0, (Obj*)buttonHome);
    AppendSubListItem(navigationBarOnReading, 0, (Obj*)buttonSetting);
    /* 把阅读界面导航栏加入阅读界面 */
    publicElemData.obj = (Obj*)navigationBarOnReading;
    push_tail(&readingTouchQueryQueue, &publicElemData);
    COLUMN_LIMIT_PX16 = textAreaWidth / (getSize(PX16) / 2);
    COLUMN_LIMIT_PX24 = textAreaWidth / (getSize(PX24) / 2);
    COLUMN_LIMIT_PX32 = textAreaWidth / (getSize(PX32) / 2);
    ROW_LIMIT_PX16    = textAreaHeight / (getSize(PX16) + getLineSpace(PX16));
    ROW_LIMIT_PX24    = textAreaHeight / (getSize(PX24) + getLineSpace(PX24));
    ROW_LIMIT_PX32    = textAreaHeight / (getSize(PX32) + getLineSpace(PX32));
    log_n("[ReadingArea] x: %d", ((Obj*)readingArea)->x);
    log_n("[ReadingArea] y: %d", ((Obj*)readingArea)->y);
    log_n("[ReadingArea] width: %d", ((Obj*)readingArea)->width);
    log_n("[ReadingArea] height: %d", ((Obj*)readingArea)->height);
    // log_n("[ReadingArea] COLUMN_LIMIT_PX16: %d", COLUMN_LIMIT_PX16);
    // log_n("[ReadingArea] COLUMN_LIMIT_PX24: %d", COLUMN_LIMIT_PX24);
    // log_n("[ReadingArea] COLUMN_LIMIT_PX32: %d", COLUMN_LIMIT_PX32);
    // log_n("[ReadingArea] ROW_LIMIT_PX16: %d", ROW_LIMIT_PX16);
    // log_n("[ReadingArea] ROW_LIMIT_PX24: %d", ROW_LIMIT_PX24);
    // log_n("[ReadingArea] ROW_LIMIT_PX32: %d", ROW_LIMIT_PX32);
    // log_n("[ReadingArea] AREA_BYTE_LIMIT_PX16: %d",
    //       ROW_LIMIT_PX16 * COLUMN_LIMIT_PX16);
    // log_n("[ReadingArea] AREA_BYTE_LIMIT_PX24: %d",
    //       ROW_LIMIT_PX24 * COLUMN_LIMIT_PX24);
    // log_n("[ReadingArea] AREA_BYTE_LIMIT_PX32: %d",
    //       ROW_LIMIT_PX32 * COLUMN_LIMIT_PX32);
}

/* 要做到上一页，需要知道页起始字节在文件中的偏移，然后移动指针至此偏移，再刷新
 * 开始渲染文本内容时，open一本书，从头开始读。每次读取一个page_buffer，大小为PAGE_SIZE。
 * 之后渲染一页内容，记录下一页的起始文本在page_buffer中的偏移，即可得知page_buffer中还剩余多少个字节
 */
/* 按照满屏的字符数量去读一个buffer，然后遍历这个buffer， */
void CreatePageIndex(char* filePath)
{
    uint8_t     res;
    uint16_t    pageNumTBL_Index = 0;
    uint32_t    pageBufferIndex  = 0;
    uint32_t    offset           = 0;
    uint16_t    startX           = ((Obj*)readingArea)->x;
    uint16_t    startY           = ((Obj*)readingArea)->y;
    uint16_t    areaWidth        = ((Obj*)readingArea)->width;
    uint16_t    areaHeight       = ((Obj*)readingArea)->height;
    uint16_t    curX             = startX;
    uint16_t    curY             = startY;
    uint8_t     bHz              = 0; /* 0: ASCII字符, 1: 汉字字符 */
    uint8_t     size             = getSize(fontSizeSelect);
    uint8_t     lineSpace        = getLineSpace(fontSizeSelect);
    const int   bufferSize       = 4096;
    static char strBuffer[bufferSize];
    char*       str            = strBuffer;
    char*       strPrev        = str;
    bool        needClearPanel = true;
    uint16_t    row, column;
    uint16_t    row_limit;
    /* 若有已被打开的文件，先将其关闭 */
    // atk_md0700_clear(ATK_MD0700_WHITE);
    if (main_file->obj.fs != 0) {
        f_close(main_file);
    }
    log_n("%sRead file [%s] start.", ARROW_STRING, filePath);
    res = f_open(main_file, filePath, FA_READ);
    check_value_equal(res, FR_OK, "Failed to open file [%s]", filePath);
    memset(pageNumTBL, 0, PAGE_INDEX_SIZE * sizeof(uint32_t));
    pageNumTBL[pageNumTBL_Index++] = 0; /* 写入起始页的页码：0 */
    res = f_read(main_file, strBuffer, bufferSize, &br);
    check_value_equal(res, FR_OK, "Failed to read from file [%s]", filePath);
    /* 每次读一页 */
    while (1) {
        /* 每次读一段 */
        ;
    }
#if 0
    while (1) {
        if (needClearPanel == true) {
            atk_md0700_clear(ATK_MD0700_WHITE);
        }
        /* 按page_buffer的大小进行一次读取 */
        /* 渲染一个buffer的内容 */
        strPrev = str;
        // log_n("Before rendering. [str]: %d", str);
        /* 渲染一页内容，返回下一次渲染的字符地址 */
        str = renderString(startX, startY, areaWidth, areaHeight, &curX, &curY,
                           str, br - pageBufferIndex, fontNameSelect,
                           fontSizeSelect, 0, 0);
        // log_n("After rendering. [str]: %d", str);
        // log_n("[strPrev]: %d", strPrev);
        // log_n("[str-strPrev]: %d", str - strPrev);
        pageBufferIndex += (str - strPrev);
        offset += (str - strPrev);
        // log_n("[offset]: %d", offset);
        // if (needClearPanel == true) {
        //     // delay_ms(500);
        // }
        needClearPanel = true;
        /* 如果buffer读完了，则再从文件读取一个buffer */
        if (pageBufferIndex >= bufferSize) {
            res = f_read(main_file, strBuffer, bufferSize, &br);
            check_value_equal(res, FR_OK, "Failed to read from file [%s]",
                              filePath);
            str             = strBuffer;
            pageBufferIndex = 0;
            needClearPanel  = false;
        }
        /* 如果现在的坐标已经 */
        if (curY > (startY + areaHeight - size)) {
            curX                           = startX;
            curY                           = startY;
            pageNumTBL[pageNumTBL_Index++] = offset;
            // log_n("%spageNumTBL[pageNumTBL_Index] = %d", ARROW_STRING,
            // pageNumTBL[pageNumTBL_Index-1]);
        }
        if (f_eof(main_file) && (pageBufferIndex == br)) {
            break;
        }
    }
#endif
    log_n("%sRead file [%s] finish.", ARROW_STRING, filePath);
    for (int i = 0; i < pageNumTBL_Index; ++i) {
        log_n("pageNum[%d]: %d", i, pageNumTBL[i]);
    }
}

void preRender(void) {}

void bookshelfBtnOnClicked(Button* bookBtn)
{
    char    path[30];
    uint8_t res;

    curTouchQueryQueue = &readingTouchQueryQueue;
    strcpy(path, bookDirPath_GBK);
    strcat(path, bookBtn->str);
    res = f_open(main_file, path, FA_READ);
    check_value_equal(res, FR_OK, "Failed to open file [%s], res [%d]", path,
                      res);
    // curOffset = 0; /* 读取书签位置，暂定为0 */
    // res = f_read(main_file, page_buffer[0], PAGE_SIZE, &br);
    // check_value_equal(res, FR_OK, "Failed to read from file [%s]", path);
    // page_buffer[0][br] = '\0';
    /* 一些初始化 */
    // curChapterPageTableIndex  = 0;  /* 本章的页码表的索引指针 */
    // curChapterPageTableTail   = -1; /* 本章的最后一页 */
    // prevChapterPageTableIndex = 0;  /* 上一章页码表的索引指针 */
    // prevChapterPageTableTail  = -1; /* 上一章的最后一页 */
    // dirTableIndex = dirTableHead;
    fillArea(((Obj*)bookshelf)->x - 10, ((Obj*)bookshelf)->y - 10,
             ((Obj*)bookshelf)->width + 20, ((Obj*)bookshelf)->height + 20,
             GUI_getBackColor());
    log_n("Rerender LCD panel.");
    navigationBarOnReading->DrawList(navigationBarOnReading);
    renderText(curOffset);
    if (dirTableTail < 0) { /* 暂时设置为每次打开都需要读取目录 */
        needGenerateDirTable     = true;
        generateDirTableFinished = false;
        /* 进入后设置为需要读取本章，且为首次进入，读取未完成 */
        needGenerateCurChapterPageTable            = true;
        firstTimeIntoGeneratingCurChapterPageTable = true;
        generateCurChapterPageTableFinished        = false;
        /* 进入后设置为暂不需要读取上一章（本章读取完成后再使能），且为首次进入，读取未完成
         */
        needGeneratePrevChapterPageTable            = false;
        firstTimeIntoGeneratingPrevChapterPageTable = true;
        generatePrevChapterPageTableFinished        = false;
    }
    /* 本章已准备好，则亮灯提醒用户可翻页 */
    if (generateCurChapterPageTableFinished == true) {
        LED0(0);
    }
}

void renderText(uint32_t offset)
{
    uint8_t        res;
    char*          str;
    const uint16_t BUF_SIZE     = 1700;
    uint16_t       chapter      = 0;
    uint32_t       textLenLimit = 4096;
    uint16_t       foreColor    = GUI_getForeColor();
    // uint16_t       backColor    = GUI_getBackColor();
    // char*          buf      = mymalloc(SRAMEX, BUF_SIZE * sizeof(uint16_t));
    char* buf = readingArea->str;
    f_lseek(main_file, offset);
    res = f_read(main_file, buf, BUF_SIZE, &br);
    if (br != BUF_SIZE) {
        buf[br] = 0;
    } else {
        buf[BUF_SIZE - 1] = 0;
    }
    fillArea(((Obj*)readingArea)->x, ((Obj*)readingArea)->y,
             ((Obj*)readingArea)->width, ((Obj*)readingArea)->height,
             GUI_getBackColor());
    if ((dirTableTail > 0) && (dirTableIndex > 0) &&
        (dirTableTail > dirTableIndex)) {
        textLenLimit = dirTable[dirTableIndex + 1] - curOffset;
    } else {
        textLenLimit = 4096;
    }
    GUI_setForeColor(readingArea->font.fontColor);
    // GUI_setBackColor(RGB888toRGB565(0x000000));
    // fillMainArea(GUI_getBackColor());
    str = Show_Str(((Obj*)readingArea)->x, ((Obj*)readingArea)->y,
                   ((Obj*)readingArea)->width, ((Obj*)readingArea)->height,
                   (uint8_t*)readingArea->str, textLenLimit,
                   (FontName)readingArea->font.fontName,
                   (FontSize)readingArea->font.fontSize, 1, &isOverOnePage);
    GUI_setForeColor(foreColor);
    // GUI_setBackColor(backColor);
    curHeadOffset = curOffset;
    curOffset += (str - buf);
}

/**
 * @description: 根据本页的首字节偏移，获取当前章节序号
 * @return {uint16_t} 当前章节序号
 */
uint16_t getDirTableIndex(void)
{
    uint16_t chapter = 0;
    for (uint16_t i = 0; i <= dirTableTail; ++i) {
        if (dirTable[i] > curHeadOffset) {
            chapter = i - 1;
            break;
        }
    }
    return chapter;
}

uint16_t getCurPageIndex(void)
{
    uint16_t retIndex = 0;
    if (generateCurChapterPageTableFinished == true) {
        for (uint16_t i = 0; i <= curChapterPageTableTail; ++i) {
            if (curChapterPageTable[i] > curHeadOffset) {
                retIndex = i - 1;
                break;
            }
        }
    }
    return retIndex;
}

/**
 * @description: 点击导航栏按钮时执行对应的操作
 * @param {Button*} button 指向被点击的导航栏按钮的指针
 * @return {void}
 */
void navigationBtnOnClicked(Button* button)
{
    if (button == buttonDir) {
        LinkedNodeData data;
        data.obj             = (Obj*)dirList;
        LinkedNode* node_res = NULL;
        if ((node_res = find_data(curTouchQueryQueue, &data)) == NULL) {
            push_head(curTouchQueryQueue, &data);
            chapterNameIndex = 0;
            fillArea(((Obj*)dirList)->x, ((Obj*)dirList)->y,
                     ((Obj*)dirList)->width, ((Obj*)dirList)->height,
                     GUI_getBackColor());
            copyChapterName();
            refreshBtnName(dirList, chapterBtn, chapterName, DrawOption_Delay);
            dirList->DrawList(dirList);
        } else {
            deleteNode(curTouchQueryQueue, node_res);
            fillMainArea(GUI_getBackColor());
            curOffset = curHeadOffset;
            renderText(curOffset);
        }
    } else if (button == buttonHome) {
        LinkedNodeData data;
        LinkedNode*    res_node = NULL;
        LED0(1);
        if (curTouchQueryQueue == &readingTouchQueryQueue) {
            /* 检查阅读界面下的目录列表组件的出队 */
            data.obj = (Obj*)dirList;
            if ((res_node = find_data(curTouchQueryQueue, &data)) != NULL) {
                deleteNode(curTouchQueryQueue, res_node);
            }
        }
        /* 主界面和阅读界面都有打开设置菜单的通道，都需要检查组件出队 */
        data.obj = (Obj*)settingMenu;
        if ((res_node = find_data(curTouchQueryQueue, &data)) != NULL) {
            deleteNode(curTouchQueryQueue, res_node);
        }
        curOffset = curHeadOffset;
        renderHomePage();
        curTouchQueryQueue = &homeTouchQueryQueue;
    } else if (button == buttonSetting) {
        LinkedNodeData data;
        data.obj             = (Obj*)settingMenu;
        LinkedNode* node_res = NULL;
        if ((node_res = find_data(curTouchQueryQueue, &data)) == NULL) {
            push_head(curTouchQueryQueue, &data);
            chapterNameIndex = 0;
            fillArea(((Obj*)settingMenu)->x, ((Obj*)settingMenu)->y,
                     ((Obj*)settingMenu)->width, ((Obj*)settingMenu)->height,
                     GUI_getBackColor());
            // copyChapterName();
            // refreshBtnName(dirList, chapterBtn, chapterName,
            // DrawOption_Delay);
            settingMenu->DrawList(settingMenu);
            drawSelectBox((Obj*)fontNameTa[fontNameSelect],
                          selectBoxBorderWidth, selectBoxSpace,
                          selectBoxBorderColor);
            drawSelectBox((Obj*)fontSizeTa[fontSizeSelect - 1],
                          selectBoxBorderWidth, selectBoxSpace,
                          selectBoxBorderColor);
            drawSelectBox(
                (Obj*)foreColorTa[getColorIndex(
                    foreColor, settingMenuForeColorCnt, foreColorSelect)],
                selectBoxBorderWidth, selectBoxSpace, selectBoxBorderColor);
            drawSelectBox(
                (Obj*)backColorTa[getColorIndex(
                    backColor, settingMenuBackColorCnt, backColorSelect)],
                selectBoxBorderWidth, selectBoxSpace, selectBoxBorderColor);
        } else {
            deleteNode(curTouchQueryQueue, node_res);
            fillMainArea(GUI_getBackColor());
            if (curTouchQueryQueue == &readingTouchQueryQueue) {
                curOffset = curHeadOffset;
                renderText(curOffset);
            } else {
                renderHomePage();
            }
        }
    }
}

void ShieldHomePageWidget(void) {}

void readingNextPage(void)
{
    if ((curOffset < f_size(main_file)) &&
        (generateCurChapterPageTableFinished == true)) {
        ++curChapterPageTableIndex;
        if (curChapterPageTableIndex > curChapterPageTableTail) {
            ++dirTableIndex; /* 下一章 */
            passChapterPageTableFromCurToPrev();
            curOffset = dirTable[dirTableIndex];
        }
        log_n("Before paging down. Tail offset of the "
              "page: %d",
              curOffset);
        renderText(curOffset);
        log_n("After paging down. Tail offset of the "
              "page: %d",
              curOffset);
    }
}

void readingPrevPage(void)
{
    if (curChapterPageTableIndex > 0) {
        curOffset = curChapterPageTable[--curChapterPageTableIndex];
        renderText(curOffset);
    } else {
        /* 上一章页码表已生成，且不在首章首章页？ */
        if ((generatePrevChapterPageTableFinished == true) &&
            !((dirTableIndex == dirTableHead) &&
              (curChapterPageTableIndex == 0))) {
            --dirTableIndex;
            passChapterPageTableFromPrevToCur();
            curOffset = curChapterPageTable[curChapterPageTableTail];
            log_n("Before paging up. Head offset of "
                  "the page: %d",
                  curHeadOffset);
            renderText(curOffset);
            log_n("After paging up. Head offset of the "
                  "page: %d",
                  curHeadOffset);
        }
    }
}

void handleGenerationOfCurChapterPageTable(void)
{
    static uint16_t x                                   = 0;
    static uint16_t y                                   = 0;
    static uint32_t generatingCurChapterPageTableOffset = 0;
    if (generateCurChapterPageTableFinished == false) {
        if (firstTimeIntoGeneratingCurChapterPageTable == true) {
            firstTimeIntoGeneratingCurChapterPageTable = false;
            x                                          = ((Obj*)readingArea)->x;
            y                                          = ((Obj*)readingArea)->y;
            curChapterPageTableTail                    = -1;
            curChapterPageTableIndex                   = 0;
            /* 更新当前章序号 */
            dirTableIndex = getDirTableIndex();
            curChapterPageTable[++curChapterPageTableTail] =
                generatingCurChapterPageTableOffset = dirTable[dirTableIndex];
        }
        handleGenerationOfChapterPageTable(
            &x, &y, &generatingCurChapterPageTableOffset,
            dirTable[dirTableIndex + 1], &generateCurChapterPageTableFinished,
            curChapterPageTable, &curChapterPageTableTail);
    }
    /* 本章页码表生成完成后，若上章页码表未生成完成，且此时不在首章，则设置需要生成上章页码表的信号
     */
    curChapterPageTableIndex = getCurPageIndex();
    if (generateCurChapterPageTableFinished == true) {
        showIndex(true);
        if ((generatePrevChapterPageTableFinished == false) &&
            (dirTableIndex != dirTableHead)
            /*&& (needGeneratePrevChapterPageTable == false)*/) {
            needGeneratePrevChapterPageTable = true;
        }
    }
}

void handleGenerationOfPrevChapterPageTable(void)
{
    static uint16_t x                                    = 0;
    static uint16_t y                                    = 0;
    static uint32_t generatingPrevChapterPageTableOffset = 0;
    if (firstTimeIntoGeneratingPrevChapterPageTable == true) {
        firstTimeIntoGeneratingPrevChapterPageTable = false;
        x                                           = ((Obj*)readingArea)->x;
        y                                           = ((Obj*)readingArea)->y;
        prevChapterPageTableTail                    = -1;
        dirTableIndex                               = getDirTableIndex();
        prevChapterPageTable[++prevChapterPageTableTail] =
            generatingPrevChapterPageTableOffset = dirTable[dirTableIndex - 1];
    }
    handleGenerationOfChapterPageTable(
        &x, &y, &generatingPrevChapterPageTableOffset, dirTable[dirTableIndex],
        &generatePrevChapterPageTableFinished, prevChapterPageTable,
        &prevChapterPageTableTail);
    // if (generatePrevChapterPageTableFinished == true) {
    //     prevChapterPageTableIndex = prevChapterPageTableTail;
    // }
}

/**
 * @description: 章节页码表生成算法
 * @param {uint16_t*} x 历史X坐标
 * @param {uint16_t*} y 历史X坐标
 * @param {uint32_t*} 历史偏移量
 * @param {uint32_t } 偏移量限制，即 *pOffset<offsetLimit
 * @param {bool*    } finishedFlag 生成完成标志
 * @param {uint32_t*} pageTable 页码表
 * @param {uint32_t*} pageTableIndex 当前
 * @return {*}
 */
#ifndef LOG_handleGenerationOfChapterPageTable
#    define LOG_handleGenerationOfChapterPageTable 1
#endif
void handleGenerationOfChapterPageTable(uint16_t* x,
                                        uint16_t* y,
                                        uint32_t* pOffset,
                                        uint32_t  offsetLimit,
                                        bool*     finishedFlag,
                                        uint32_t* pageTable,
                                        uint32_t* pageTableIndex)
{
    uint8_t        res;
    static char    preRenderCh[2];
    static uint8_t preRenderChOffset = 0;
    static bool    isCompleteChar    = true;
    uint16_t       width             = ((Obj*)readingArea)->width;
    uint16_t       height            = ((Obj*)readingArea)->height;
    uint32_t       tmpOffset = f_tell(main_file); /* 保存当前偏移 */
    uint8_t        size      = getSize(readingArea->font.fontSize);
    uint8_t        lineSpace = getLineSpace(readingArea->font.fontSize);
    uint16_t       startX    = ((Obj*)readingArea)->x;
    uint16_t       startY    = ((Obj*)readingArea)->y;
    uint16_t       limitX = ((Obj*)readingArea)->x + ((Obj*)readingArea)->width;
    uint16_t limitY = ((Obj*)readingArea)->y + ((Obj*)readingArea)->height;
    bool     isHz   = false; /* 是否是汉字？ */
    bool     isCR   = false; /* 是否为回车？ */

    f_lseek(main_file, *pOffset);
    if (f_eof(main_file) || (*pOffset >= offsetLimit)) {
        /* 若文件已读完，或偏移等于下一章的首字节偏移，则表示读完了 */
        *finishedFlag = true;
        /* 若加载的是本章 */
        LED0(0);
#if LOG_handleGenerationOfChapterPageTable
        if (pageTable == curChapterPageTable) {
            log_n("%s Cur Chapter Page Table. [index]: %d", ARROW_STRING,
                  *pageTableIndex);
        } else if (pageTable == prevChapterPageTable) {
            log_n("%s Prev Chapter Page Table. [index]: %d", ARROW_STRING,
                  *pageTableIndex);
        }
        for (int i = 0; i <= *pageTableIndex; ++i) {
            char tmpBuf[21];
            readTxt(main_file, pageTable[i], tmpBuf, 20);
        }
#endif
    } else {
        /* 读取一个完整的字符（汉字字符或ASCII字符） */
        res = f_read(main_file, preRenderCh + preRenderChOffset, 1, &br);
        check_value_equal(res, FR_OK, "read file error");
        *pOffset += br;
        if ((isCompleteChar == true) &&
            (preRenderCh[preRenderChOffset] & 0x80)) {
            isCompleteChar = false;
            preRenderChOffset++;
        } else if ((isCompleteChar == false) && (preRenderChOffset != 0)) {
            isCompleteChar = true;
        }
        /* 读到一个完整的字符后，才执行下面操作 */
        if (isCompleteChar == true) {
            isHz = (preRenderChOffset != 0) ? true : false;
            if (isHz == true) { /* 汉字？ */
                *x += size;
            } else {            /* ASCII */
                isCR = (preRenderCh[0] == '\r') ? true : false;
                if (isCR == true) {
                    /* 回车？*/
                    *x = startX;
                    *y += size;
                    *y += lineSpace;
                    *pOffset += 1;
                } else { /* 普通ASCII字符*/
                    *x += (size / 2);
                }
            }
            if (*x >= limitX) { /* 宽度越界则换行 */
                *x = startX;
                if (isHz == true) {
                    *x += size;
                } else if (isCR == false) {
                    *x += (size / 2);
                }
                *y += size;
                *y += lineSpace;
            }
            if ((*y + size) >= limitY) { /* 高度越界则记录一页 */
                *y = startY;
                if (*pOffset < offsetLimit) {
                    pageTable[++(*pageTableIndex)] =
                        *pOffset - (preRenderChOffset + 1);
                }
#if LOG_handleGenerationOfChapterPageTable
                /* 打印此章节每一页的首字节偏移量 */
                //                if (pOffset ==
                //                &generatingCurChapterPageTableOffset) {
                //                    log_n("[curChapterPageTableTail]: %d",
                //                          curChapterPageTableTail);
                //                } else if (pOffset ==
                //                &generatingPrevChapterPageTableOffset) {
                //                    log_n("[prevChapterPageTableTail]: %d",
                //                          prevChapterPageTableTail);
                //                }
                log_n("[ChapterPageTableTail]: %d", *pageTableIndex);
#endif
            }
            preRenderChOffset = 0;
        }
    }
END_OF_HANDLE_GENERATION_OF_CHAPTER_PAGE_TABLE:
    f_lseek(main_file, tmpOffset);
}

/**
 * @description: 目录生成算法
 * @return {void}
 */
void handleGenerationOfDirTable(void)
{
    static bool     isCompleteChar = true;
    static char     preRenderCh[2];
    static uint8_t  preRenderChOffset = 0;
    static bool     gotDI             = false;
    static uint32_t offset_DI         = 0;
    static uint32_t offset_ZHANG      = 0;
    static uint8_t  charCnt           = 0;
    static bool     gotZHANG          = false;
    uint32_t        tmpOffset         = f_tell(main_file);
    uint8_t         res;
    f_lseek(main_file, generatingDirTableOffset);
    if (!f_eof(main_file)) {
#if 1
        res = f_read(main_file, preRenderCh + preRenderChOffset, 1, &br);
        /* 读一个完整的字符 */
        check_value_equal(res, FR_OK, "read file error");
        generatingDirTableOffset += br;
        if ((isCompleteChar == true) &&
            (preRenderCh[preRenderChOffset] & 0x80)) {
            isCompleteChar = false;
            preRenderChOffset++;
        } else if ((isCompleteChar == false) && (preRenderChOffset != 0)) {
            isCompleteChar = true;
        }
        if (isCompleteChar == true) {
            if (gotZHANG == true) {
                /* 若已发现“章”，则找后续的至多25字符（50
                 * Bytes）作为章节标题，遇到换行符则提前终止 */
                if (((preRenderChOffset == 0) &&
                     ((preRenderCh[preRenderChOffset] == '\n') ||
                      (preRenderCh[preRenderChOffset] == '\r'))) ||
                    (charCnt > 50)) {
                    // charCnt += (preRenderChOffset + 1);
                    gotDI       = false;
                    gotZHANG    = false;
                    int tmpSeek = f_tell(main_file);
                    f_lseek(main_file, offset_DI);
                    char chapter[100];
                    f_read(main_file, chapter,
                           offset_ZHANG - offset_DI + 2 + charCnt, &br);
                    chapter[br] = 0;
                    charCnt     = 0;
                    f_lseek(main_file, tmpSeek);
                    dirTable[++dirTableTail] = offset_DI;
                    /* 如果是第一章 */
                    if ((strncmp(chapter, Chapter_Str_First_1,
                                 strlen(Chapter_Str_First_1)) == 0) ||
                        (strncmp(chapter, Chapter_Str_First_2,
                                 strlen(Chapter_Str_First_2)) == 0)) {
                        dirTableIndex = dirTableHead = dirTableTail;
                    }
                    log_n("[%d]%s offset: %d || [Ch_1st]: %d", dirTableTail,
                          chapter, offset_DI, dirTableHead);
                    // continue;
                    goto END_OF_HANDLE_GENERATION_OF_DIR_TABLE;
                }
                charCnt += (preRenderChOffset + 1);
            }
            if (gotDI == true) {
                /* 若已发现“第”字，且未发现“章”字，则开始检测“章”字 */
                charCnt += (preRenderChOffset + 1);
                if (charCnt > 18) {
                    gotDI = false;
                    f_lseek(main_file, offset_DI + 2);
                    charCnt = 0;
                }
                if ((preRenderChOffset != 0) && (gotZHANG == false) &&
                    (strncmp(preRenderCh, Chapter_Str_ZHANG, 2) == 0)) {
                    gotZHANG     = true;
                    offset_ZHANG = f_tell(main_file) - 2;
                    charCnt      = 0;
                }
            }
            /* 若是汉字，且未发现“第”字，则进行“第”字符串匹配 */
            if ((preRenderChOffset != 0) && (gotDI == false) &&
                (strncmp(preRenderCh, Chapter_Str_DI, 2) == 0)) {
                gotDI     = true;
                offset_DI = f_tell(main_file) - 2;
                charCnt   = 0;
            }
            preRenderChOffset = 0;
        }
#endif
    } else {
        dirTable[++dirTableTail] =
            f_tell(main_file); /* 目录表项的最大一项表示文件的最大偏移+1 */
        generateDirTableFinished = true;
    }
END_OF_HANDLE_GENERATION_OF_DIR_TABLE:
    f_lseek(main_file, tmpOffset);
}

uint16_t
getColorIndex(COLOR_DATTYPE* colorArr, uint16_t colorCnt, uint16_t color)
{
    uint16_t index = 0;
    for (uint16_t i = 0; i < colorCnt; ++i) {
        if (colorArr[i] == color) {
            index = i;
            break;
        }
    }
    return index;
}

#if 0
WCHAR* convert_GB2312_to_Unicode(WCHAR* pUnicode, char* pGB2312)
{
    WCHAR  tmp;
    WCHAR* origin = pUnicode;
    while (*pGB2312) {
        tmp = *pGB2312++;
        if (tmp & 0x80) {
            tmp <<= 8;
            tmp |= (*pGB2312++);
        }
        *pUnicode = ff_oem2uni(tmp, 936);
        pUnicode++;
    }
    *pUnicode = 0;
    return origin;
}

char* convert_Unicode_to_GB2312(char* pGB2312, WCHAR* pUnicode)
{
    WCHAR tmp;
    char* origin = pGB2312;
    while (*pUnicode) {
        tmp = ff_uni2oem(*pUnicode, 936);
        if (tmp & (~(0xff))) {
            *pGB2312++ = (tmp >> 8);
        }
        *pGB2312++ = tmp;
        pUnicode++;
    }
    *pGB2312 = 0;
    return origin;
}

WCHAR* wchncpy(WCHAR* dest, WCHAR* src, int cnt)
{
    int    i      = 0;
    WCHAR* origin = dest;
    for (i = 0; i < cnt; ++i) {
        *dest = *src;
        if (*src == 0) {
            break;
        }
        dest++;
        src++;
    }
    return origin;
}
#endif

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
