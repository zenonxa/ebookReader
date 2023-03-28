#include "font.h"
#include "SYSTEM/sys/sys.h"
#include "string.h"
#include "BSP/W25QXX/w25qxx.h"
#include "SYSTEM/delay/delay.h"

#define ONE_SECTOR_SIZE		(4*1024)

const uint32_t FONT_OCCUPY_ALL = FONT_HEADER_SIZE + UNIGBK_SIZE + FONT_SIZE_ALL*Font_Cnt;

const uint16_t FONTSECSIZE = FONT_OCCUPY_ALL/ONE_SECTOR_SIZE + ((FONT_OCCUPY_ALL%ONE_SECTOR_SIZE == 0) ? 0 : 1);

/* Font library name string */
const char* const path_prefixStr = "0:FONT/";
const char* const prefixStr[Prefix_Cnt] = {
	"UNIGBK",
	"GBK"
};
const char* const underlineStr = "_";
const char* const FontNameStr[Font_Cnt] = {
	"SimSun",
	"KaiTi",
	"SimHei",
};

const char* const FontSizeStr[Font_Size_Cnt] = {
	"12",
	"16",
	"24",
	"32",
};

const char* const dotStr = ".";
const char* const suffixStr[Suffix_Cnt] = {
	"BIN",
	"DZK"
};

void getFontPath(char* dest, FontName fontName, FontSize fontSize) {
	uint8_t condition;
	
	strcpy(dest, path_prefixStr);
	/* condition:
		true:	UNIGBK.bin
		false:	$FontName_$FontSize.DZK
	*/
	condition = ((fontName == Font_None) && (fontSize == Font_Size_None));
	strcat(dest, (condition ? prefixStr[UNIGBK] : prefixStr[GBK]));
	if (!condition) {
		strcat(dest, underlineStr);
		strcat(dest, FontNameStr[fontName]);
		strcat(dest, underlineStr);
		strcat(dest, FontSizeStr[fontSize]);
	}
	strcat(dest, dotStr);
	strcat(dest, (condition ? suffixStr[BIN] : suffixStr[DZK]));
}


uint32_t getFontAddr(FontName fontName, FontSize fontSize) {
	if ((fontName == Font_None) && (fontSize == Font_Size_None)) {
		return UNIGBK_ADDR;
	}
	uint32_t addr = FONT_LIB_BASE_ADDR + fontName * FONT_SIZE_ALL;
	if (fontSize > PX12) {
		addr += FONT_SIZE_PX12;
	}
	if (fontSize > PX16) {
		addr += FONT_SIZE_PX16;
	}
	if (fontSize > PX24) {
		addr += FONT_SIZE_PX24;
	}
	return addr;
}


uint8_t getSize(FontSize fontSize) {
	uint8_t size;
	switch (fontSize) {
		case PX12:
			size = 12;
			break;
		case PX16:
			size = 16;
			break;
		case PX24:
			size = 24;
			break;
		case PX32:
			size = 32;
			break;
		default:
			size = 12;
	}
	return size;
}


uint8_t check_font_header_once(void) {
	uint8_t res = 0;
	FontHeader fontHeader;
	W25QXX_Read((uint8_t*)&fontHeader, FONT_HEADER_ADDR, sizeof(fontHeader));//读出FontHeader结构体数据
	if(!(fontHeader.fontok == FLAG_OK) || !(fontHeader.ugbkok == FLAG_OK)) {
		res = 1;
	}
	return res;
}


//初始化字体
//返回值:0,字库完好.
//		 其他,字库丢失
uint8_t check_font_header(uint8_t tryTimes)
{
	uint8_t t = 0, res = 0;
	/* Try at least once. */
	if (tryTimes < 1) {
		tryTimes = 1;
	}
//	W25QXX_Init();
	/* Try to read and check font flag in flash */
	while(t < tryTimes)
	{
		t++;
		check_font_header_once();
		delay_ms(20);
	}
	/* Check font flag error! */
	if(!(fontHeader.fontok == FLAG_OK) || !(fontHeader.ugbkok == FLAG_OK)) {
		res = 1;
	}
	return res;    
}

uint8_t write_font_header(FontHeader* pFontHeader, uint8_t tryTimes) {
	uint8_t i, res, ref_res_val;
	if (tryTimes < 1) {
		tryTimes = 1;
	}
	if ((pFontHeader->fontok == FLAG_OK) && (pFontHeader->ugbkok == FLAG_OK)) {
		ref_res_val = 0;	/* Check operation would succeed after HontHeader writing. */
		res = 1;
	} else {
		ref_res_val = 1;
		res = 0;
	}
	/* write font header to flash manually */
	for (i = 0; i < tryTimes; ++i) {
		W25QXX_Write((uint8_t*)&fontHeader, FONT_HEADER_ADDR, sizeof(fontHeader));
		res = check_font_header_once();
		if (res == ref_res_val) {
			break;
		}
	}
	return res;
}
