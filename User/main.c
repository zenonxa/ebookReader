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

uint8_t fontNameSelect = Font_SimSun;
uint8_t fontSizeSelect = PX24;
uint8_t needRerender   = 1;
uint16_t booknameIndex = 0; /* 数值上等价于读取的已读取过的bookname的数量 */

uint32_t* pageNumTBL;

/* 书架列表控件 */
List* bookshelf = NULL;
/* 全局导航栏控件 */
List*    navigationBar = NULL;
Button** booknameBtn   = NULL;
Button*  buttonBack    = NULL;
Button*  buttonHome    = NULL;
Button*  buttonSetting = NULL;
/* 阅读区域 */
Textarea* readingArea = NULL;

LinkedList* curTouchQueryQueue = NULL;
LinkedList  homeTouchQueryQueue; /* 存储widget指针的触摸查询队列 */
LinkedList  emptyTouchQueryQueue;

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

#define STRING_SIZE 60

WCHAR utf16_string[STRING_SIZE];
char  gb2312_string[STRING_SIZE * 2];

const char tmpPath[] = "0:BOOK/";
#define readdir(forward)                                                       \
    do {                                                                       \
        FILINFO* pFNO;                                                         \
        if (forward) {                                                         \
            pFNO = &fileinfo;                                                  \
        } else {                                                               \
            pFNO = NULL;                                                       \
        }                                                                      \
        uint8_t res = f_readdir(&dir, pFNO);                                   \
        check_value_equal(res, FR_OK, "Fail to read dir %s", tmpPath);         \
        if ((pFNO) != (NULL)) {                                                \
            log_n("filename: [%s]", fileinfo.fname);                           \
        } else {                                                               \
            log_n("Reverse.");                                                 \
        }                                                                      \
        log_n("dir.dptr: %d", dir.dptr);                                       \
    } while (0)

int main(void)
{
    uint8_t    res;
    uint8_t    i = 0;
    uint8_t    tmp_buf[PAGE_SIZE];
    FontHeader fontHeader = {FLAG_OK, FLAG_OK};
    uint16_t   color      = ATK_MD0700_BLACK;
    uint16_t   startX;
    uint16_t   startY;

    page_buffer[0] = tmp_buf;
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
        check_value_not_equal(
            res, 0, "Fail to load UNIGBK.BIN and font library to falsh.");
        fontHeader.fontok = FLAG_OK;
        fontHeader.ugbkok = FLAG_OK;
        W25QXX_Write((uint8_t*)&fontHeader, FONT_HEADER_ADDR,
                     sizeof(fontHeader));
    }
    // char tmpPath[] = "0:BOOK/";
    // res = f_opendir(&dir, tmpPath);
    // check_value_equal(res, FR_OK, "Fail to open dir %s", tmpPath);
    // while (1)
    //     ;

    // res = f_readdir(&dir, &fileinfo);

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
    refreshBookname(bookshelf, booknameBtn, bookname,
                    DrawOption_Delay); /* 刷新书名 */
    renderHomePage();                  /* 渲染 Home 界面 */
    createReadingArea();
    CreatePageIndex("0:BOOK/1.txt");
    while (1) {
        /* 状态机设计：
         * - 每次循环更新一次触摸事件生命周期状态
         * - 根据当前状态进行必要的即时响应
         * - 在松手后，产生触摸事件的结果，根据此结果，进行对应的操作 */
        touchEventUpdate(&touchState, &flag); /* 更新触摸事件生命周期 */
        if (touchState == TouchState_OnRelease) {
            touchEvent = getTouchEvent(flag);
            /* Do things according the touch event */
            /* 松手后解除控件的即时响应 */
            if (cur_target) {
                switch (cur_target->type) {
                    /* 恢复按钮的状态 */
                    case Obj_Type_Button:
                        if (((Button*)cur_target)->isPressed == BT_PRESSED) {
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
                    if (curTouchQueryQueue == &homeTouchQueryQueue) {
                        /* 书架界面上滑：下一页 */
                        if ((*fileinfo.fname) ||
                            (booknameBufferCur != booknameBufferTail)) {
                            CopyBookname(&dir, listItemLimit, bookname, 1);
                            for (i = 0; i < listItemLimit; ++i) {
                                booknameBtn[i]->isPressed = BT_UNPRESSED;
                            }
                            refreshBookname(bookshelf, booknameBtn, bookname,
                                            DrawOption_Immediately);
                        }
                    }
                } else if (slideDirestion == Slide_Down) {
                    if (curTouchQueryQueue == &homeTouchQueryQueue) {
                        /* 书架界面下滑：上一页 */
                        if (booknameIndex > 9) {
                            // readDirRevese(&dir, getItemListSize(bookshelf) -
                            // 1); readDirRevese(&dir, listItemLimit);
                            CopyBookname(&dir,
                                         listItemLimit +
                                             getItemListSize(bookshelf),
                                         bookname, 0);
                            for (i = 0; i < listItemLimit; ++i) {
                                booknameBtn[i]->isPressed = BT_UNPRESSED;
                            }
                            refreshBookname(bookshelf, booknameBtn, bookname,
                                            DrawOption_Immediately);
                        }
                    }
                } else if (slideDirestion == Slide_To_Left) {
                    /* 阅读界面：向左翻页 */
                } else if (slideDirestion == Slide_To_Right) {
                    /* 阅读界面：向右翻页 */
                }
            } else if ((touchEvent == Touch_Event_ShortPress) ||
                       (touchEvent == Touch_Event_LongPress)) {
                /* 位于主页时，若触摸点位于书架的图书按钮上，则进度阅读界面，并切换组件查询队列
                 */
                if (curTouchQueryQueue == &homeTouchQueryQueue) {
                    if ((prev_target) &&
                        (prev_target->type == Obj_Type_Button)) {
                        ((Button*)prev_target)->OnClicked((Button*)prev_target);
                    }
                    prev_target = NULL;
                }
                /* 存在展开的菜单时，若落点不在菜单内，则收回菜单 */
                /* 阅读界面：发生短按或长按时，根据落点区域判断是否进行翻页以及翻页的方向
                 */
            }
            clearTouchFlag(&flag);
            touchState = Touch_State_None;
#    if 1
            /* 查看当前dir指针偏移 */
            char buf[30] = {0};
            sprintf(buf, "%2d", booknameIndex);
            fillArea(0, 0, 50, 30, RGB888toRGB565(0xffffff));
            Show_Str(0, 0, 50, 30, (uint8_t*)buf, Font_SimSun, PX24, 0);
#    endif
        } else if (touchState == TouchState_OnPress) {
            /* 查询控件队列，落点是否位于某个具体的控件上 */
            cur_target = touchQueryForWidget(curTouchQueryQueue, &point_cur[0]);
            updateWidgetStateOnTouch(cur_target, TouchState_OnPress);
        } else if (touchState == TouchState_Moving) {
            if (cur_target) {
                if (touchQueryForWidget(curTouchQueryQueue, &point_cur[0]) !=
                    cur_target) {
                    updateWidgetStateOnTouch(cur_target, TouchState_OnRelease);
                    cur_target = NULL;
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
    navigationBar =
        NewList(0, ATK_MD0700_LCD_HEIGHT / 10 * 9, ATK_MD0700_LCD_WIDTH,
                ATK_MD0700_LCD_HEIGHT / 10, &publicBorder, 0,
                ATK_MD0700_LCD_HEIGHT / 10, NULL, 0);
    publicElemData.obj = (Obj*)navigationBar;
    push_tail(curTouchQueryQueue, &publicElemData);
    /* 创建导航栏中的按钮 */
    setPublicFont(Font_SimSun, PX24, RGB888toRGB565(0x000000));
    setPublicBorder(RGB888toRGB565(0xcccccc), 5, BORDER_FLAG(BORDER_TOP));
    setPublicAlignType(AlignHorizonalType_CENTER, AlignVerticalType_MIDDLE);
    buttonBack         = NewButton(0, 0, 160, navigationBar->itemHeight,
                                   LocateType_Relative, &publicFont, &publicBorder,
                                   &publicAlignType, navigationBtnOnClicked);
    buttonHome         = NewButton(160, 0, 160, navigationBar->itemHeight,
                                   LocateType_Relative, &publicFont, &publicBorder,
                                   &publicAlignType, navigationBtnOnClicked);
    buttonSetting      = NewButton(320, 0, 160, navigationBar->itemHeight,
                                   LocateType_Relative, &publicFont, &publicBorder,
                                   &publicAlignType, navigationBtnOnClicked);
    buttonBack->str    = navigationBarString[0];
    buttonHome->str    = navigationBarString[1];
    buttonSetting->str = navigationBarString[2];
    AppendSubListItem(navigationBar, 0, (Obj*)buttonBack);
    AppendSubListItem(navigationBar, 0, (Obj*)buttonHome);
    AppendSubListItem(navigationBar, 0, (Obj*)buttonSetting);
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
    Textarea* bookshelfHeadlineTA =
        NewTextarea(0, 0, ((Obj*)bookshelf)->width, bookshelf->headlineHeight,
                    LocateType_Relative, &publicFont, &publicBorder,
                    RGB888toRGB565(0xcccccc));
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
    while (SD_Init()) {
        log_n("SD Card Error! Please check!");
        HAL_Delay(500);
    }
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

/**
 * @description: 按给定的颜色填充指定区域
 * @return {void}
 */
void fillMainArea(void)
{
    fillArea(0, 0, ATK_MD0700_LCD_WIDTH, ATK_MD0700_LCD_HEIGHT / 10 * 9 - 1,
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
                } else if (state == TouchState_OnRelease) {
                    ((Button*)obj)->isPressed = BT_UNPRESSED;
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
            // if ((booknameBufferCur != booknameBufferTail) &&
            // (readFromBufferFlag == true)) { /* 发生过回退，但未发生回滚 */
            //     booknameBufferCur =
            //     nextBookNameBufferIndex(booknameBufferCur);
            // } else if (booknameBufferCur == booknameBufferTail) { /*
            // 未发生回退，或者回退已完全恢复 */
            //     readFromBufferFlag = false;
            // }
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
                log_n("In copy bookname. [%d] dir->fn: [%8s]", i, dir->fn);
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
            // if (booknameBufferCur ==
            //     prevBookNameBufferIndex(
            //         booknameBufferHead)) { /*
            //         若上一页不在缓存中，则cur自减后，还需要回滚
            //                                 */
            //     booknameBufferHead =
            //         prevBookNameBufferIndex(booknameBufferHead);
            //     booknameBufferTail =
            //         prevBookNameBufferIndex(booknameBufferTail);
            // if (readFromBufferFlag == false) {

            // }
        } else { /* 刷新本页 */
            // booknameIndex -= getItemListSize(bookshelf);
        }
    }
    /* 将buff[cur]中的bookname拷贝出来（刷新上页、重刷本页） */
    for (int i = 0; i < bookshelf->itemList->size; ++i) {
        strcpy(bookname[i], booknameBuffer[booknameBufferCur][i]);
    }

#if 0
    if (booknameBufferCur == -1) {
        booknameBufferCur = booknameBufferTail = booknameBufferHead = 0;
        /* 第一次前进式拷贝，需要readir */
    } else {
        /* 如果当前使用的缓冲区就是最后一个，则tail应当取模自增1 */
        if (booknameBufferCur == booknameBufferTail) {
            booknameBufferTail = nextBookNameBufferIndex(booknameBufferTail);
        }
        /* cur正常取模自增1。若cur取模自增后等于head，则head也应取模自增1 */
        booknameBufferCur = nextBookNameBufferIndex(booknameBufferCur);
        if (booknameBufferCur == booknameBufferHead) {
            booknameBufferHead = nextBookNameBufferIndex(booknameBufferHead);
        }
    }
    while (res == FR_OK) {
        res = f_readdir(dir, &fileinfo);
        log_n("In copy bookname. [%d] dir->fn: [%8s]", i, dir->fn);
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
    for (uint8_t j = 0; j < limit; ++j) {
        strcpy(bookname[j], booknameBuffer[booknameBufferCur][j]);
    }
#endif
}

/**
 * @description: 刷新书架列表的书名
 * @param {List*     } list 书架列表
 * @param {Button**  } booknameBtn 图书按钮数组
 * @param {char**    } bookname 书名数组
 * @param {DrawOption} drawOption 绘制选项（立即绘制，或稍后手动绘制）
 * @return {void}
 */
void refreshBookname(List*      list,
                     Button**   booknameBtn,
                     char**     bookname,
                     DrawOption drawOption)
{
    uint8_t i = 0;
    for (i = 0; i < list->itemList->size; i++) {
        // 若无法搜索到书名，后续内容不渲染
        if (bookname[i][0] == 0) {
            break;
        }
        // 如果按钮中有书名，但按钮曾被移出链表，则重新添加回来
        LinkedNodeData data;
        data.obj = (Obj*)booknameBtn[i];
        if (find_data(getSubList(list, i), &data) == NULL) {
            AppendSubListItem(list, i, (Obj*)booknameBtn[i]);
        }
    }
    // 若存在空字符串的按钮，则移出链表
    for (; i < list->itemList->size; i++) {
        LinkedNodeData data;
        data.obj = (Obj*)booknameBtn[i];
        booknameBtn[i]->DrawButton(booknameBtn[i]);
        deleteNode(getSubList(list, i), find_data(getSubList(list, i), &data));
    }
    /* 根据参数决定是否立即重新绘制 */
    if (drawOption == DrawOption_Immediately) {
        redrawListItem(list);
    }
}

/**
 * @description: 刷新主页
 * @return {void}
 */
void renderHomePage(void)
{
    if (curTouchQueryQueue != &homeTouchQueryQueue) {
        readDirRevese(&dir, getItemListSize(bookshelf));
    }
    /* 刷新主界面书架、导航栏 */
    // refreshBookname(bookshelf, booknameBtn, bookname, DrawOption_Delay);
    atk_md0700_clear(BACKGROUND_COLOR);
    bookshelf->DrawList(bookshelf);
    navigationBar->DrawList(navigationBar);
}

void createReadingArea(void)
{
    const uint16_t textAreaWidth  = ATK_MD0700_LCD_WIDTH * 4 / 5;
    const uint16_t textAreaHeight = ATK_MD0700_LCD_HEIGHT * 4 / 5;
    setPublicFont((FontName)fontNameSelect, (FontSize)fontSizeSelect,
                  RGB888toRGB565(0x000000));
    readingArea =
        NewTextarea((ATK_MD0700_LCD_WIDTH - textAreaWidth) / 2,
                    (ATK_MD0700_LCD_HEIGHT - textAreaHeight) / 2, textAreaWidth,
                    textAreaHeight, LocateType_Absolute, &publicFont,
                    &publicBorder, RGB888toRGB565(0xffffff));
    readingArea->str = NULL;
    log_n("[ReadingArea] x: %d", ((Obj*)readingArea)->x);
    log_n("[ReadingArea] y: %d", ((Obj*)readingArea)->y);
    log_n("[ReadingArea] width: %d", ((Obj*)readingArea)->width);
    log_n("[ReadingArea] height: %d", ((Obj*)readingArea)->height);
}

/* 要做到上一页，需要知道页起始字节在文件中的偏移，然后移动指针至此偏移，再刷新
 * 开始渲染文本内容时，open一本书，从头开始读。每次读取一个page_buffer，大小为PAGE_SIZE。
 * 之后渲染一页内容，记录下一页的起始文本在page_buffer中的偏移，即可得知page_buffer中还剩余多少个字节
 */
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
    char*       str     = strBuffer;
    char*       strPrev = str;
    /* 若有已被打开的文件，先将其关闭 */
    if (main_file->obj.fs != 0) {
        f_close(main_file);
    }
    res = f_open(main_file, filePath, FA_READ);
    check_value_equal(res, FR_OK, "Failed to open file [%s]", filePath);
    memset(pageNumTBL, 0, PAGE_INDEX_SIZE * sizeof(uint32_t));
    pageNumTBL[pageNumTBL_Index++] = 0;
    res = f_read(main_file, strBuffer, bufferSize, &br);
    check_value_equal(res, FR_OK, "Failed to read from file [%s]", filePath);
    // for (int i = 0; i < 20; ++i) {
    //     log("0x%2x ", strBuffer[i]);
    //     if ((i + 1) % 5 == 0) {
    //         log_n("");
    //     }
    // }
    // while(1);
    while (1) {
        /* 按page_buffer的大小进行一次读取 */
        /* 渲染一个buffer的内容 */
        /* 渲染一页内容 */
        strPrev = str;
        log_n("Before rendering. [str]: %d", str);
        str = renderString(startX, startY, areaWidth, areaHeight, &curX, &curY,
                           str + pageBufferIndex, bufferSize - pageBufferIndex,
                           fontNameSelect, fontSizeSelect, 0, 0);
        log_n("After rendering. [str]: %d", str);
        log_n("[strPrev]: %d", strPrev);
        log_n("[str-strPrev]: %d", str - strPrev);
        pageBufferIndex += (str - strPrev);
        offset += (str - strPrev);
        /* 如果buffer读完了，则再从文件读取一个buffer */
        if (pageBufferIndex >= PAGE_INDEX_SIZE) {
            res = f_read(main_file, strBuffer, bufferSize, &br);
            check_value_equal(res, FR_OK, "Failed to read from file [%s]",
                              filePath);
            pageBufferIndex = 0;
        }
        /* 如果现在的坐标已经 */
        if (curY > (startY + areaHeight - size)) {
            curX                           = startX;
            curY                           = startY;
            pageNumTBL[pageNumTBL_Index++] = offset;
        }
        if (f_eof(main_file)) {
            break;
        }
#if 0
        while (*str) {
            if (!bHz) {            /* 处理第一个字符 */
                if (*str & 0x80) { /* 第一个字符是汉字？ */
                    bHz = 1;
                } else { /* 第一个字符不是汉字 */
                    /* 若剩余空间已无法再绘制一个ASCII字符，则换行 */
                    if (x > (xStart + width - size / 2)) {
                        /* 换行操作包括：X重置、Y增加行高、Y增加行距 */
                        moveCursorToNextLine(&x, &y, size, lineSpace, xStart,
                                             &ls_y);
                    }
                    /* 若剩余高度不足以再再渲染一行文字，则结束渲染 */
                    if (y > (yStart + height - size)) {
                        break;  // 越界返回
                    }
                    /* 绘制ASCII字符。若遇到换行符号，则改为另起一行，并将指针后后移
                     */
                    if (*str == 13) {
                        moveCursorToNextLine(&x, &y, size, lineSpace, xStart,
                                             &ls_y);
                        str++;
                    } else {
                        atk_md0700_show_char(x, y, *(char*)str,
                                             (atk_md0700_lcd_font_t)size,
                                             FOREGROUND_COLOR);  // 有效部分写入
                        str++;
                        x += size / 2;  // 字符,为全字的一半
                    }
                }
            } else {     /* 处理中文字符 */
                bHz = 0; /* 清除汉字标志 */
                /* 若本行剩余空间无法再绘制汉字，则换行 */
                if (x > (xStart + width - size)) {
                    /* The space left in this line is not enough */
                    moveCursorToNextLine(&x, &y, size, lineSpace, xStart,
                                         &ls_y);
                }
                /* 若剩余高度无法再绘制汉字，则结束绘制 */
                if (y > (yStart + height - size)) {
                    break;  // 越界返回
                }
                Show_Font(x, y, str, fontName, fontSize,
                          mode);  // 显示这个汉字,空心显示
                str += 2;
                x += size;  // 下一个汉字偏移
            }
        }
        log_n("[%d] read from [%s]: %d byte", i++, filePath, br);
        /* 遍历page_buffer中的内容，找到每一页内容的起始偏移 */
        // pCh = Show_Str(((Obj*)readingArea)->x, ((Obj*)readingArea)->y,
        //                ((Obj*)readingArea)->width,
        //                ((Obj*)readingArea)->height, page_buffer[0],
        //                readingArea->font.fontName,
        //                readingArea->font.fontSize, 1, DrawOption_Delay);
#endif
    }
    log_n("%sRead file [%s] finish.", ARROW_STRING, filePath);
    for (int i = 0; i < pageNumTBL_Index; ++i) {
        log_n("pageNum[%d]: %d", i, pageNumTBL[i]);
    }
}

void bookshelfBtnOnClicked(Button* bookBtn)
{
    char    path[30];
    uint8_t res;

    curTouchQueryQueue = &emptyTouchQueryQueue;
    strcpy(path, bookDirPath_GBK);
    strcat(path, bookBtn->str);
    res = f_open(main_file, path, FA_READ);
    check_value_equal(res, FR_OK, "Failed to open file [%s], res [%d]", path,
                      res);
    res = f_read(main_file, page_buffer[0], PAGE_SIZE, &br);
    check_value_equal(res, FR_OK, "Failed to read from file [%s]", path);
    page_buffer[0][br] = '\0';
    log_n("Rerender LCD panel.");
    /* 整个界面清空后，开始渲染文本 */
    atk_md0700_clear(ATK_MD0700_WHITE);
    // atk_md0700_fill((ATK_MD0700_LCD_WIDTH - textAreaWidth) / 2,
    //                 (ATK_MD0700_LCD_HEIGHT - textAreaHeight) / 2,
    //                 (ATK_MD0700_LCD_WIDTH + textAreaWidth) / 2,
    //                 (ATK_MD0700_LCD_HEIGHT + textAreaHeight) / 2,
    //                 &BACKGROUND_COLOR, SINGLE_COLOR_BLOCK);
    Show_Str((ATK_MD0700_LCD_WIDTH - textAreaWidth) / 2,
             (ATK_MD0700_LCD_HEIGHT - textAreaHeight) / 2, textAreaWidth,
             textAreaHeight, page_buffer[0], (FontName)fontNameSelect,
             (FontSize)fontSizeSelect, 1);
}

/**
 * @description: 点击导航栏按钮时执行对应的操作
 * @param {Button*} button 指向被点击的导航栏按钮的指针
 * @return {void}
 */
void navigationBtnOnClicked(Button* button)
{
    if (button == buttonBack) {
    } else if (button == buttonHome) {
        renderHomePage();
        curTouchQueryQueue = &homeTouchQueryQueue;
    } else if (button == buttonSetting) {
    }
}

void ShieldHomePageWidget(void) {}

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
