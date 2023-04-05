#include "font.h"
#include "BSP/W25QXX/w25qxx.h"
#include "SYSTEM/delay/delay.h"
#include "SYSTEM/sys/sys.h"
#include "string.h"

/* The size of one sector in ex-flash (Byte) */
#define ONE_SECTOR_SIZE (4 * 1024)

/* All the file concerning font, including FONT_HEADER, UNIGBK mapping table, 12
 * default font libraries */
const uint32_t FONT_OCCUPY_ALL =
    FONT_HEADER_SIZE + UNIGBK_SIZE + FONT_SIZE_ALL * Font_Name_Cnt;
/* The number of the sector used for font */
const uint16_t FONTSECSIZE = FONT_OCCUPY_ALL / ONE_SECTOR_SIZE +
                             ((FONT_OCCUPY_ALL % ONE_SECTOR_SIZE == 0) ? 0 : 1);

/* Font library name string */
const char* const path_prefixStr             = "0:FONT/";
const char* const prefixStr[Prefix_Cnt]      = {"UNIGBK", "GBK"};
const char* const underlineStr               = "_";
const char* const FontNameStr[Font_Name_Cnt] = {
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
const char* const dotStr                = ".";
const char* const suffixStr[Suffix_Cnt] = {"BIN", "DZK"};

/**
 * @description: Open to the external function to get the path string with the
 * give fontName and fontSize.
 * @param {char*} dest: the dist buffer to save the path string.
 * @param {FontName} fontName: the name of the font
 * @param {FontSize} fontSize: the size of the font
 * @return {char*} A pointer pointing to the path string, whose value is equal
 * to the given param dest
 */
char* getFontPath(char* dest, FontName fontName, FontSize fontSize)
{
    uint8_t condition;

    strcpy(dest, path_prefixStr);
    /* condition:
        true:	UNIGBK.bin
        false:	$FontName_$FontSize.DZK
    */
    condition = ((fontName == Font_Name_None) && (fontSize == Font_Size_None));
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

/**
 * @description: Open to the external function to get the address in ex-flash
 * with the give fontName and fontSize.
 * @param {FontName} fontName: the name of the font
 * @param {FontSize} fontSize: the size of the font
 * @return {uint32_t} the address of the font library
 */
uint32_t getFontAddr(FontName fontName, FontSize fontSize)
{
    if ((fontName == Font_Name_None) && (fontSize == Font_Size_None)) {
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


/**
 * @description: Open to the external function to get the address of the Unicode-GBK mapping table.
 * @return {uint32_t} The address of the Unicode-GBK mapping table.
 */
/**/
uint32_t getMappingTableAddr(void)
{
    return UNIGBK_ADDR;
}

/* Get path of [UNIGBK.BIN] */
/**
 * @description: Open to the external function to get the path string of the Unicode-GBK mappint table.
 * @param {uint8_t*} pathBuf: the buffer to save the path string of Unicode-GBK mappint table
 * @return {void}
 */
void getMappingTablePath(uint8_t* pathBuf)
{
    strcpy((char*)pathBuf, path_prefixStr);
    strcat((char*)pathBuf, prefixStr[UNIGBK]);
    strcat((char*)pathBuf, dotStr);
    strcat((char*)pathBuf, suffixStr[BIN]);
}


/**
 * @description: Convert the fontSize from Enum type to uint8_t.
 * @param {FontSize} fontSize: the size of the font.
 * @return {uint8_t} A interger of the value of fontsize
 */
uint8_t getSize(const FontSize fontSize)
{
    uint8_t size;
    switch (fontSize) {
        case PX12: size = 12; break;
        case PX16: size = 16; break;
        case PX24: size = 24; break;
        case PX32: size = 32; break;
        default: size = 12;
    }
    return size;
}


/**
 * @description: Check the font header in ex-flash once.
 * @return {uint8_t} 0: success  1: fail
 */
uint8_t check_font_header_once(void)
{
    uint8_t    res = 0;
    FontHeader fontHeader;
    W25QXX_Read((uint8_t*)&fontHeader, FONT_HEADER_ADDR,
                sizeof(fontHeader));  // 读出FontHeader结构体数据
    if (!(fontHeader.fontok == FLAG_OK) || !(fontHeader.ugbkok == FLAG_OK)) {
        res = 1;
    }
    return res;
}


/**
 * @description: Check the font header in ex-flash, trying for the given times.
 * @return {uint8_t} 0: success  1: fail
 */
uint8_t check_font_header(uint8_t tryTimes)
{
    uint8_t t = 0, res = 0;
    /* Try at least once. */
    if (tryTimes < 1) {
        tryTimes = 1;
    }
    //	W25QXX_Init();
    /* Try to read and check font flag in flash */
    while (t < tryTimes) {
        t++;
        res = check_font_header_once();
        if (!res) {
            break;
        }
        delay_ms(20);
    }
    /* Check font flag result:
     * 	0: success
     *	1: fail					*/
    return res;
}


/**
 * @description: Write font header to ex-flash
 * @param {FontHeader*} pFontHeader: A pointer pointering to a structure FontHeader
 * @param {uint8_t} tryTimes: the number of the time to try
 * @return {uint8_t} 0: success  1: fail
 */
uint8_t write_font_header(FontHeader* pFontHeader, uint8_t tryTimes)
{
    uint8_t i, res, ref_res_val;
    if (tryTimes < 1) {
        tryTimes = 1;
    }
    if ((pFontHeader->fontok == FLAG_OK) && (pFontHeader->ugbkok == FLAG_OK)) {
        ref_res_val =
            0; /* Check operation would succeed after HontHeader writing. */
        res = 1;
    } else {
        ref_res_val = 1;
        res         = 0;
    }
    /* write font header to flash manually */
    for (i = 0; i < tryTimes; ++i) {
        W25QXX_Write((uint8_t*)&fontHeader, FONT_HEADER_ADDR,
                     sizeof(fontHeader));
        res = check_font_header_once();
        if (res == ref_res_val) {
            break;
        }
    }
    return res;
}


#if 0
uint8_t getFontFileType(FontName fontName, FontSize fontSize)
{
    uint8_t fontFileType = Font_File_Type_Default;
    if ((fontName == Font_Name_None) && (fontSize == Font_Size_None)) {
        fontFileType = MAPPING_TABLE; /* Unicode to GBK mapping table */
    } else if (!((fontName >= Font_Name_Min) && (fontName <= Font_Name_Max) &&
                 (fontSize >= Font_Size_Min) && (fontSize <= Font_Size_Max))) {
        fontFileType = FONT; /* Font file */
    } else {
        fontFileType = Font_File_Type_None; /* Invalid type */
    }
    return fontFileType;
}
#endif
