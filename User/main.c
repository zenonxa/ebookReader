#include "main.h"

uint8_t*     lcd_buffer;
uint8_t*     page_buffer[PAGE_NUM];
unsigned int page_buffer_len[PAGE_NUM];

extern FIL*    main_file;  // file 1
extern FIL*    ftemp;      // file temp.
extern UINT    br, bw;     // the number of byte really read or written
extern FILINFO fileinfo;   // file information
extern DIR     dir;        // directory

uint8_t  fontNameSelect = Font_SimSun;
uint8_t  fontSizeSelect = PX24;
uint8_t  needRerender   = 1;
uint16_t booknameIndex  = 0;

/* 书架列表控件 */
List* bookshelf = NULL;
/* 全局导航栏控件 */
List*    navigationBar = NULL;
Button** booknameBtn   = NULL;
Button*  buttonBack    = NULL;
Button*  buttonHome    = NULL;
Button*  buttonSetting = NULL;

LinkedList* curTouchQueryQueue;
LinkedList  homeTouchQueryQueue; /* 存储widget指针的触摸查询队列 */
LinkedList  emptyTouchQueryQueue;

char** bookname = NULL;

uint16_t textAreaWidth  = ATK_MD0700_LCD_WIDTH * 4 / 5;
uint16_t textAreaHeight = ATK_MD0700_LCD_HEIGHT * 4 / 5;

#define STRING_SIZE 60

WCHAR utf16_string[STRING_SIZE];
char  gb2312_string[STRING_SIZE * 2];

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
    // memset(utf16_string, 0, sizeof(WCHAR) * 60);
    // utf16_string[0] = 0x4b6d;
    // utf16_string[1] = 0xd58b;
    // utf16_string[2] = 0x2e00;
    // utf16_string[3] = 0x7400;
    // utf16_string[4] = 0x7800;
    // utf16_string[5] = 0x7400;
    // res = f_open(&my_file, L"测试.txt", FA_READ);
    // res = f_open(&my_file, L"测试.txt", FA_CREATE_NEW);
    // if (res != FR_OK) {
    //     char tmp[30];
    //     sprintf(tmp, "%d", res);
    //     Show_Str(0, 0, 50, 30, tmp, Font_SimSun, PX24, 0);
    //     LED_flashing(500);
    //     infinite_throw("Open test file fail. ==> %d", res);
    // }
    // res = f_close(&my_file);
    // f_mount(NULL, L"0:", 1);
    // LED_flashing(3000);

    /* Open test txt fle in SD Card */
    // res = f_open(&my_file, "0:BOOK/test.txt", FA_READ);
    // if (res != FR_OK) {
    //     infinite_throw("Open test file fail.");
    // }
    // f_read(&my_file, page_buffer[0], PAGE_SIZE, &br);
    // page_buffer[0][br] = '\0';
    // if (res != FR_OK) {
    //     infinite_throw("Read test file fail.");
    // }
    // f_close(&my_file);
    /* Test to show a string with Chinese character */
    // BACKGROUND_COLOR = ATK_MD0700_GREEN;
    // Show_Str((ATK_MD0700_LCD_WIDTH - textAreaWidth) / 2,
    //               (ATK_MD0700_LCD_HEIGHT - textAreaHeight) / 2,
    //               textAreaWidth, textAreaHeight, page_buffer[0], Font_SimSun,
    //               PX12, 1);
    // log_n("br: %d", br);
    /* Unmount SD Card volume */
    // f_mount(NULL, "0:", 1);

    uint8_t touchState = Touch_State_None;
    uint8_t flag;
    uint8_t slideDirestion;
    uint8_t touchEvent;
    int16_t dx, dy;
    clearTouchFlag(&flag);
    TouchEventInfo_Init();
    curTouchQueryQueue = &homeTouchQueryQueue;
    init_LinkedList(curTouchQueryQueue, NodeDataType_Obj);
    init_LinkedList(&emptyTouchQueryQueue, NodeDataType_Obj);

    const uint16_t menuWidth_1  = 400;
    const uint16_t menuHeight_1 = 600;
    uint8_t        listItemLimit;
    /* 创建书架列表 */
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
    setPublicFont(Font_SimSun, PX24, RGB888toRGB565(0x000000));
    setPublicBorder(RGB888toRGB565(0x000000), 3, BORDER_NULL);
    Textarea* bookshelfHeadline =
        NewTextarea(0, 0, ((Obj*)bookshelf)->width, bookshelf->headlineHeight,
                    &publicFont, &publicBorder, RGB888toRGB565(0xcccccc));
    bookshelfHeadline->str      = bookshelfHeadlineStr;
    bookshelf->headlineTextarea = bookshelfHeadline;
    /* 创建导航栏 */
    setPublicBorder(RGB888toRGB565(0x000000), 3, BORDER_NULL);
    navigationBar =
        NewList(0, ATK_MD0700_LCD_HEIGHT / 10 * 9, ATK_MD0700_LCD_WIDTH,
                ATK_MD0700_LCD_HEIGHT / 10, &publicBorder, 0,
                ATK_MD0700_LCD_HEIGHT / 10, NULL, 0);
    publicElemData.obj = (Obj*)navigationBar;
    push_tail(curTouchQueryQueue, &publicElemData);
    setPublicFont(Font_SimSun, PX24, RGB888toRGB565(0x000000));
    setPublicBorder(RGB888toRGB565(0xcccccc), 5, BORDER_FLAG(BORDER_TOP));
    setPublicAlignType(AlignHorizonalType_CENTER, AlignVerticalType_MIDDLE);
    buttonBack =
        NewButton(0, 0, 160, navigationBar->itemHeight, &publicFont,
                  &publicBorder, &publicAlignType, navigationBtnOnClicked);
    buttonHome =
        NewButton(160, 0, 160, navigationBar->itemHeight, &publicFont,
                  &publicBorder, &publicAlignType, navigationBtnOnClicked);
    buttonSetting =
        NewButton(320, 0, 160, navigationBar->itemHeight, &publicFont,
                  &publicBorder, &publicAlignType, navigationBtnOnClicked);
    buttonBack->str    = navigationBarString[0];
    buttonHome->str    = navigationBarString[1];
    buttonSetting->str = navigationBarString[2];
    AppendSubListItem(navigationBar, 0, (Obj*)buttonBack);
    AppendSubListItem(navigationBar, 0, (Obj*)buttonHome);
    AppendSubListItem(navigationBar, 0, (Obj*)buttonSetting);
    /* 绘制导航栏 */
    // navigationBar->DrawList(navigationBar);
    /* 创建按钮，并添加至书架列表中 */
    booknameBtn = mymalloc(GUI_MALLOC_SOURCE, sizeof(Button*) * listItemLimit);
    check_value_not_equal(booknameBtn, NULL,
                          "Failed to malloc for booknameBtn");
    // Button* booknameBtn[listItemLimit];
    // char bookname[listItemLimit][30];
    bookname = (char**)mymalloc(SRAMIN, sizeof(char*) * listItemLimit);
    for (int i = 0; i < listItemLimit; i++) {
        bookname[i] = (char*)mymalloc(SRAMIN, sizeof(char) * STRING_SIZE * 2);
        memset(bookname[i], 0, sizeof(char) * STRING_SIZE);
    }
    memset(booknameBtn, 0, sizeof(Button*) * listItemLimit);
    // memset(bookname, 0, sizeof(char) * listItemLimit * 30);
    setPublicAlignType(AlignHorizonalType_LEFT, AlignVerticalType_MIDDLE);
    setPublicBorder(RGB888toRGB565(0x000000), 3, BORDER_NULL);
    setPublicFont(Font_SimSun, PX24, RGB888toRGB565(0x000000));
    for (i = 0; i < listItemLimit; ++i) {
        booknameBtn[i] = NewButton(
            0, 0, ((Obj*)bookshelf)->width, bookshelf->itemHeight, &publicFont,
            &publicBorder, &publicAlignType, &bookshelfBtnOnClicked);
        check_value_not_equal(booknameBtn[i], NULL,
                              "Fail to malloc for bootnameBtn[%d]", i);
        booknameBtn[i]->str = bookname[i];
    }
    /* 读取目录,并将文件名加载值书架列表的列表项中 */
    res = f_opendir(&dir, "0:/BOOK");
    check_value_equal(res, FR_OK, "Open dir fail");
    i = 0;
    CopyBookname(&dir, listItemLimit, bookname);
    refreshBookname(bookshelf, booknameBtn, bookname, DrawOption_Delay);
    /* 绘制书架列表 */
    // bookshelf->DrawList(bookshelf);
    renderHomePage();
    Obj* cur_target  = NULL;
    Obj* prev_target = NULL;
    while (1) {
#    if 1
        touchEventUpdate(&touchState, &flag); /* 更新触摸事件生命周期 */
        if (touchState == TouchState_OnRelease) {
            touchEvent = getTouchEvent(flag);
            /* Do things according the touch event */
            if (cur_target) {
                switch (cur_target->type) {
                    case Obj_Type_Button:
                        if (((Button*)cur_target)->ispressed == BT_PRESSED) {
                            ((Button*)cur_target)->ispressed = BT_UNPRESSED;
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
                        if (*fileinfo.fname) {
                            CopyBookname(&dir, listItemLimit, bookname);
                            for (i = 0; i < listItemLimit; ++i) {
                                booknameBtn[i]->ispressed = BT_UNPRESSED;
                            }
                            refreshBookname(bookshelf, booknameBtn, bookname,
                                            DrawOption_Immediately);
                        }
                    }
                } else if (slideDirestion == Slide_Down) {
                    if (curTouchQueryQueue == &homeTouchQueryQueue) {
                        /* 书架界面下滑：上一页 */
                        if (booknameIndex > 9) {
                            readDirRevese(&dir, getItemListSize(bookshelf));
                            readDirRevese(&dir, listItemLimit);
                            CopyBookname(&dir, listItemLimit, bookname);
                            for (i = 0; i < listItemLimit; ++i) {
                                booknameBtn[i]->ispressed = BT_UNPRESSED;
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
                // cur_target;
                /* 主页时，把 */
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
            /* 查看当前dir指针偏移 */
            // char buf[30] = {0};
            // sprintf(buf, "%2d", booknameIndex);
            // fillArea(0, 0, 50, 30, RGB888toRGB565(0xffffff));
            // Show_Str(0, 0, 50, 30, buf, Font_SimSun, PX24, 0);
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
                    ((Button*)obj)->ispressed = BT_PRESSED;
                } else if (state == TouchState_OnRelease) {
                    ((Button*)obj)->ispressed = BT_UNPRESSED;
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
    uint8_t i = 0;
    uint8_t res;
    for (i = 0; i < limit - 1; ++i) {
        res = f_readdir(dir, NULL);
        booknameIndex--;
    }
}

/**
 * @description: 遍历目录，将指定数量的文件名
 * @param {DIR*} dir 目录
 * @param {uint8_t} limit 记录的文件名的数量
 * @param {char**} bookname 存储目录名的二维数组
 * @return {void}
 */
void CopyBookname(DIR* dir, uint8_t limit, char** bookname)
{
    uint8_t i   = 0;
    uint8_t res = FR_OK;
    while (res == FR_OK) {
        res = f_readdir(dir, &fileinfo);
        /* 若已遍历整个目录, 则退出 */
        if (*fileinfo.fname == 0) {
            break;
        }
        booknameIndex++; /* 下一个会被读取的文件项的编号, 自0开始 */
        check_value_equal(res, FR_OK, "read dir fail");
        strcpy(bookname[i], fileinfo.fname);
        ++i;
        /* 读够 limit 项则退出 */
        if (i >= limit) {
            break;
        }
    }
    /* 若目录剩余项不足 limit, 将未获取到字符串的数组置为空字符串 */
    if (i < limit) {
        for (; i < limit; ++i) {
            bookname[i][0] = 0;
        }
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
    uint16_t limit = getItemListSize(bookshelf);
    readDirRevese(&dir, limit);
    /* 刷新主界面书架、导航栏 */
    // refreshBookname(bookshelf, booknameBtn, bookname, DrawOption_Delay);
    atk_md0700_clear(ATK_MD0700_WHITE);
    bookshelf->DrawList(bookshelf);
    navigationBar->DrawList(navigationBar);
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
    atk_md0700_fill((ATK_MD0700_LCD_WIDTH - textAreaWidth) / 2,
                    (ATK_MD0700_LCD_HEIGHT - textAreaHeight) / 2,
                    (ATK_MD0700_LCD_WIDTH + textAreaWidth) / 2,
                    (ATK_MD0700_LCD_HEIGHT + textAreaHeight) / 2,
                    &BACKGROUND_COLOR, SINGLE_COLOR_BLOCK);
    Show_Str((ATK_MD0700_LCD_WIDTH - textAreaWidth) / 2,
             (ATK_MD0700_LCD_HEIGHT - textAreaHeight) / 2, textAreaWidth,
             textAreaHeight, page_buffer[0], fontNameSelect, fontSizeSelect, 1);
}

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
