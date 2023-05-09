#ifndef __MAIN_H__
#define __MAIN_H__

// #include "math.h"
#include <stm32f1xx.h>
#include <string.h>
#include <wchar.h>

/********************************************************/
/* 				include for BSP							*/
/********************************************************/
#include "BSP/ATK_MD0700/atk_md0700.h"
#include "BSP/ATK_MD0700/atk_md0700_touch.h"
#include "BSP/EXTI/exti.h"
#include "BSP/KEY/key.h"
#include "BSP/LED/led.h"
#include "BSP/SDIO/sdio_sdcard.h"
#include "BSP/SRAM/sram.h"
#include "BSP/TIMER/timer.h"
#include "BSP/W25QXX/w25qxx.h"
/********************************************************/

/********************************************************/
/* 				include for SYSTEM						*/
/********************************************************/
#include "SYSTEM/delay/delay.h"
#include "SYSTEM/sys/sys.h"
#include "SYSTEM/usart/usart.h"
/********************************************************/

/********************************************************/
/*				include for Middlewares					*/
/********************************************************/
#include "FATFS/exfuns/exfuns.h"
#include "FATFS/src/ff.h"
#include "MALLOC/malloc.h"
#include "TEXT/fontupd.h"
#include "font.h"
#include "gui.h"
#include "log.h"
#include "text.h"
#include "widget/inc/button.h"
#include "widget/inc/list.h"
#include "widget/inc/textarea.h"
/********************************************************/

/********************************************************/
/*				include for Test						*/
/********************************************************/
#include "Test/test_atk_md0700.h"
#include "gbk_string.h"
/********************************************************/

/********************************************************/
/*					Action command						*/
/********************************************************/
#define ACTION_ONCE 0
#define ACTION_COMMAND SingleTest
typedef enum {
    WriteFontHeader = 1, /* write FontHeader to Flash */
    EraseFontHeader,     /* write OK flags of FontHeader in Flash */
    WriteFontLib,        /* Rewrite a font library to Flash */
    LoadFileToFlash,     /* Load a specified file to the flash */
    SingleTest,          /* Set for a single test */
} ActionCommand;
/********************************************************/

/********************************************************/
/*						Value							*/
/********************************************************/
typedef enum {
    FAT_DRV_SDCARD = 0,
} FatfsLogicDriverNumber;

typedef enum {
    ReadWriteType_Read,
    ReadWriteType_Write,
    ReadWriteType_Min     = ReadWriteType_Read,
    ReadWriteType_Max     = ReadWriteType_Write,
    ReadWriteType_Default = ReadWriteType_Min,
    ReadWriteType_Cnt     = ReadWriteType_Max + 1,
    ReadWriteType_None    = ReadWriteType_Cnt,
} ReadWriteType;

typedef struct
{
    uint8_t data;
} EbookDataHeader;

typedef enum {
    EbookDataType_BookValidFlag = 0,
    EbookDataType_BookName,
    EbookDataType_BookSize,
    EbookDataType_Bookmark,
    EbookDataType_ChapterTableOffset,
    EbookDataType_ChapterTableHead,
    EbookDataType_ChapterTableTail,
    EbookDataType_ChapterTable,
    EbookDataType_Min     = EbookDataType_BookValidFlag,
    EbookDataType_Max     = EbookDataType_ChapterTable,
    EbookDataType_Default = EbookDataType_Min,
    EbookDataType_Cnt     = EbookDataType_Max + 1,
    EbookDataType_None    = EbookDataType_Cnt,
} EbookDataType;

/* 公共设备配置 */
typedef struct
{
    uint8_t       fontName;
    uint8_t       fontSize;
    COLOR_DATTYPE foreColor;
    COLOR_DATTYPE backColor;
} DeviceData;

/* 图书配置 */
typedef struct
{
    uint8_t   bookVaildFlag;
    char*     bookname;
    uint32_t  bookSize;
    uint32_t  bookmark;
    uint32_t  chapterTableOffset;
    uint16_t  chapterHead;
    uint16_t  chapterTail;
    uint32_t* chpterTable;
} EbookData;
/********************************************************/

/********************************************************************************************/
/*									Macro
 */
/********************************************************************************************/
#define PAGE_NUM 1      // the number of cache page
#define PAGE_SIZE 1700  // the size of one cache page

#define SRAMEX_USER_BASE_ADDR                                                  \
    (0x68000000 + (MY_DISP_HOR_RES) * (MY_DISP_VER_RES)*2)

#if 0
#    define ERROR_LED_FLASH_SPAN_MS 500
#    define ERROR_THROW(msg)                                                   \
        do {                                                                   \
            while (1) {                                                        \
                LED_Toggle();                                                  \
                printf("%s\r\n", msg);                                         \
                delay_ms(ERROR_LED_FLASH_SPAN_MS);                             \
            }                                                                  \
        } while (0)
#endif

#define ACTION_ONCE_DONE_MSG                                                   \
    "Action once done. Now in LED flashing infinite loop."
/********************************************************************************************/

/********************************************************************************************/
/*									Function Declaration
 */
/********************************************************************************************/
/* Toggle state for LED0 and LED1  */
void waiting_for_SD_Card(void);
void mount_SD_Card(void);
void LED_Toggle(void);
void LED_flashing(
    uint16_t time_span_ms); /* LED0 and LED1, flashing as the given time span */
void     show_logo(uint8_t* logoPicture, uint16_t delayTime_ms);
void     fillMainArea(COLOR_DATTYPE color);
Obj*     touchQuery(LinkedList* queryQueue, Position* pos);
Obj*     touchSubQuery(LinkedList* querySubQueue, Position* pos);
Obj*     touchQueryForWidget(LinkedList* touchQueryQueue, Position* pos);
void     updateWidgetStateOnTouch(Obj* obj, TouchState state);
void     refreshBtnName(List*      list,
                        Button**   booknameBtn,
                        char**     bookname,
                        DrawOption drawOption);
void     CopyBookname(DIR* dir, uint8_t limit, char** bookname, bool forward);
void     readDirRevese(DIR* dir, uint8_t limit);
void     bookshelfBtnOnClicked(Button* bookBtn);
void     navigationBtnOnClicked(Button* button);
WCHAR*   wchncpy(WCHAR* dest, WCHAR* src, int cnt);
WCHAR*   convert_GB2312_to_Unicode(WCHAR* pUnicode, char* pGB2312);
char*    convert_Unicode_to_GB2312(char* pGB2312, WCHAR* pUnicode);
void     renderHomePage(void);
void     InitForMain(void);
void     createNavigationBar(void);
void     createBookshelf(void);
void     createReadingArea(void);
void     createBooknameBuffer(void);
void     CreatePageIndex(char* filePath);
void     clearReadingArea(void);
void     createSettingMenu(void);
void     renderText(uint32_t offset);
void     handleGenerationOfCurChapterPageTable(void);
void     handleGenerationOfPrevChapterPageTable(void);
void     handleGenerationOfChapterPageTable(uint16_t* x,
                                            uint16_t* y,
                                            uint32_t* pOffset,
                                            uint32_t  offsetLimit,
                                            bool*     finishedFlag,
                                            uint32_t* pageTable,
                                            int32_t*  pageTableIndex);
void     handleGenerationOfDirTable(void);
void     createDirList(void);
void     chapterBtnOnClicked(Button* button);
void     copyChapterName(void);
void     passChapterPageTableFromPrevToCur(void);
void     passChapterPageTableFromCurToPrev(void);
uint16_t getDirTableIndex(void);
void     settingMenuBtnOnClicked(Button* button);
void     createSettingMenuBtn(Button** btnArr,
                              uint16_t btnCnt,
                              uint16_t startX,
                              uint16_t restWidth,
                              uint16_t btnWidth,
                              uint16_t btnHeight,
                              uint16_t lineIndex);
uint16_t
     getColorIndex(COLOR_DATTYPE* colorArr, uint16_t colorCnt, uint16_t color);
void createSettingMenuTa(Textarea**     taArr,
                         uint16_t       taCnt,
                         COLOR_DATTYPE* colorArr,
                         uint16_t       startX,
                         uint16_t       restWidth,
                         uint16_t       taWidth,
                         uint16_t       taHeight,
                         uint16_t       lineIndex);
void settingMenuTaOnClicked(Textarea* textarea);
void updateWidgetColor(Obj* obj);
void readingNextPage(void);
void readingPrevPage(void);
uint16_t     getCurPageIndex(void);
void         showIndex(bool showFlag);
uint32_t     hash(const char* key);
unsigned int BKDR_hash(const char* key);
int          findBookID(const char* bookname, bool* foundFlag);
void         loadBookData(ReadWriteType rw_type,
                          EbookDataType ebookDataType,
                          uint32_t      id,
                          uint8_t*      buff);
void         readEbookData(int id);
void         writeEbookData(int id);
#if ACTION_ONCE
void excuteCommand(void);
#endif
/********************************************************************************************/

#endif
