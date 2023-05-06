#ifndef __GBK_STRING_H__
#define __GBK_STRING_H__

char* gbk_test_string            = "一个测试字符串";
char  navigationBarString[4][10] = {
    "返回",
    "主页",
    "设置",
    "目录",
};

char* bookshelfHeadlineStr   = "书架";
char* dirListHeadlineStr     = "目录";
char* settingMenuHeadlineStr = "设置菜单";
char* fontNameTaStr          = "字体";
char fontNameStr[3][10] = {
    "宋体",
    "楷体",
    "黑体",
};
char* fontSizeTaStr          = "字号";
char* foreColorTaStr         = "前景色";
char* backColorTaStr         = "背景色";
char* fontSizeStr_HAO        = "号";
char* operationStr_Apply = "应用";
char* operationStr_Cancel = "取消";

char* testFilePath = "0:/BOOK/测试-平凡的世界.txt";

char* bookDirPath_GBK  = "0:\\BOOK\\";
char* bookDirPath_ANSI = "0:/BOOK/";

char* SDCardDetectErrorHint = "未检测到SD卡，请确认SD已插入";

char* Chapter_Str_DI      = "第";
char* Chapter_Str_ZHANG   = "章";
char* Chapter_Str_First_1 = "第一章";
char* Chapter_Str_First_2 = "第1章";

char* gbk_full_stop        = "。"; /* GBK中文句号 */
char* gbk_exclamatory_mark = "！"; /* GBK中文感叹号 */
char* gbk_question_mark    = "？"; /* GBK中文问号 */
#endif
