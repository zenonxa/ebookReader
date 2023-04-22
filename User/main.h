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
#define ACTION_COMMAND WriteFontLib
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
/********************************************************/

/********************************************************************************************/
/*									Macro
 */
/********************************************************************************************/
#define PAGE_NUM 3      // the number of cache page
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
void   show_logo(uint8_t* logoPicture, uint16_t delayTime_ms);
void   fillMainArea(void);
Obj*   touchQuery(LinkedList* queryQueue, Position* pos);
Obj*   touchSubQuery(LinkedList* querySubQueue, Position* pos);
Obj*   touchQueryForWidget(LinkedList* touchQueryQueue, Position* pos);
void   updateWidgetStateOnTouch(Obj* obj, TouchState state);
void   refreshBookname(List*      list,
                       Button**   booknameBtn,
                       char**     bookname,
                       DrawOption drawOption);
void   CopyBookname(DIR* dir, uint8_t limit, char** bookname, bool forward);
void   readDirRevese(DIR* dir, uint8_t limit);
void   bookshelfBtnOnClicked(Button* bookBtn);
void   navigationBtnOnClicked(Button* button);
WCHAR* wchncpy(WCHAR* dest, WCHAR* src, int cnt);
WCHAR* convert_GB2312_to_Unicode(WCHAR* pUnicode, char* pGB2312);
char*  convert_Unicode_to_GB2312(char* pGB2312, WCHAR* pUnicode);
void   renderHomePage(void);
void   InitForMain(void);
void   createNavigationBar(void);
void   createBookshelf(void);
void   createReadingArea(void);
void   createBooknameBuffer(void);
void   CreatePageIndex(char* filePath);
void   clearReadingArea(void);
void   renderText(uint32_t offset);
void   handleGenerationOfCurChapterPageTable(void);
void   handleGenerationOfPrevChapterPageTable(void);
void   handleGenerationOfChapterPageTable(uint16_t* x,
                                          uint16_t* y,
                                          uint32_t* pOffset,
                                          uint32_t  offsetLimit,
                                          bool*     finishedFlag,
                                          uint32_t* pageTable,
                                          uint32_t* pageTableIndex);
void   handleGenerationOfDirTable(void);
#if ACTION_ONCE
void excuteCommand(void);
#endif
/********************************************************************************************/

#endif
