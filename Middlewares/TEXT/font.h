#ifndef __FONT_H__
#define __FONT_H__

#include "SYSTEM/sys/sys.h"

/* The type of the font */
typedef enum {
	Font_SimSun = 0,
	Font_KaiTi,
	Font_SimHei,
	Font_Min_Value = Font_SimSun,		/* Min */
	Font_Max_Value = Font_SimHei,		/* Max */
	Font_Default = Font_Min_Value,		/* Default */
	Font_Cnt = Font_Max_Value + 1,		/* The number of all */
	Font_None = 0xff,					/* Invalid value */
} FontName;

typedef enum {
	PX12 = 0,
	PX16,
	PX24,
	PX32,
	Font_Min_Size = PX12,					/* Min */
	Font_Max_Size = PX32,					/* Max */
	FONT_Default_size = Font_Min_Size,		/* Default */
	Font_Size_Cnt = Font_Max_Size + 1,		/* The number of all size */
	Font_Size_None = 0xff,					/* Invalid value */
} FontSize;


typedef enum {
	UNIGBK = 0,
	GBK,
	Prefix_Min_Value = UNIGBK,
	Prefix_Max_Value = GBK,
	Prefix_Default_Value = Prefix_Min_Value,
	Prefix_Cnt = Prefix_Max_Value + 1,
	Prefix_None = 0xff,
} Prefix;


typedef enum {
	BIN = 0,
	DZK,
	Suffix_Min_Value = BIN,
	Suffix_Max_Value = DZK,
	Suffix_Default_Value = Suffix_Min_Value,
	Suffix_Cnt = Suffix_Max_Value + 1,
	Suffix_None = 0xff,
} Suffix;

__packed typedef struct 
{
	/* font flag ==> normal:0xAA; abnormal:!0xAA */
	uint8_t ugbkok;
	/* flag for UniBGK.bin ==> normal:0xAA; abnormal:!0xAA */
	uint8_t fontok;
} FontHeader;

#define FLAG_OK				0xAA

#define FONT_HEADER_SIZE	64
#define UNIGBK_SIZE			174344

/* Font library size (Byte) */
#define FONT_SIZE_PX12		212064
#define FONT_SIZE_PX16		282752
#define FONT_SIZE_PX24		636192
#define FONT_SIZE_PX32		1131008

/* The number of the different size of one font */
//#define FONT_SIZE_CNT		4

/* The occupation of all different size of one font */
#define FONT_SIZE_ALL		(FONT_SIZE_PX12 + FONT_SIZE_PX16 + FONT_SIZE_PX24 + FONT_SIZE_PX32)

/* Address of header, UniGBK.bin, font_lib. 
 * Start from address 9 MB in flash. 		*/
#define FONT_BASE_ADDR		(9*1024*1024)
#define FONT_HEADER_ADDR	(FONT_BASE_ADDR + 0)
#define UNIGBK_ADDR			(FONT_BASE_ADDR + FONT_HEADER_SIZE)
#define FONT_LIB_BASE_ADDR	(FONT_BASE_ADDR + FONT_HEADER_SIZE + UNIGBK_SIZE)

/* Calculate the address with the given font_num of enum type */
//#define FONT_ADDR(font)		(FONT_LIB_BASE_ADDR + (font)*FONT_SIZE_ALL)

/* Occupation of all of the FONT, including header, UniGBK.bin, and all kinds of font library. */
extern const uint32_t FONT_OCCUPY_ALL;

/* Sector occupation of all of the font information. */
extern const uint16_t FONTSECSIZE;

/* Font Header */
extern FontHeader fontHeader;

/* Font library name string */
extern const char* const path_prefixStr;
extern const char* const prefixStr[];
extern const char* const underlineStr;
extern const char* const FontNameStr[];
extern const char* const FontSizeStr[];
extern const char* const dotStr;
extern const char* const suffixStr[];

/* Function Declaration */
void getFontPath(char* buffer, FontName fontName, FontSize fontSize);
uint32_t getFontAddr(FontName fontName, FontSize fontSize);
uint8_t getSize(FontSize fontSize);
uint8_t check_font_header_once(void);
uint8_t check_font_header(uint8_t tryTimes);
uint8_t write_font_header(FontHeader* pFontHeader, uint8_t tryTimes);
#endif
