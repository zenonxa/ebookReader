#include "FATFS/exfuns/exfuns.h"
#include "BSP/W25QXX/w25qxx.h"
#include "FATFS/exfuns/fattester.h"
#include "FATFS/src/ff.h"
#include "MALLOC/malloc.h"
#include "SYSTEM/delay/delay.h"
#include "SYSTEM/usart/usart.h"
#include "font.h"
#include "log.h"
#include "string.h"
//////////////////////////////////////////////////////////////////////////////////
// 本程序只供学习使用，未经作者许可，不得用于其它任何用途
// ALIENTEK STM32开发板
// FATFS 扩展代码
// 正点原子@ALIENTEK
// 技术论坛:www.openedv.com
// 创建日期:2019/9/28
// 版本：V1.1
// 版权所有，盗版必究。
// Copyright(C) 广州市星翼电子科技有限公司 2014-2024
// All rights reserved
//********************************************************************************
// 升级说明
// V1.1
// 修正exf_copy函数,文件进度显示错误的bug
//////////////////////////////////////////////////////////////////////////////////

#define FILE_MAX_TYPE_NUM 7  // 最多FILE_MAX_TYPE_NUM个大类
#define FILE_MAX_SUBT_NUM 4  // 最多FILE_MAX_SUBT_NUM个小类

// 文件类型列表
u8* const FILE_TYPE_TBL[FILE_MAX_TYPE_NUM][FILE_MAX_SUBT_NUM] = {
    {"BIN"},                        // BIN文件
    {"LRC"},                        // LRC文件
    {"NES", "SMS"},                 // NES/SMS文件
    {"TXT", "C", "H"},              // 文本文件
    {"WAV", "MP3", "APE", "FLAC"},  // 支持的音乐文件
    {"BMP", "JPG", "JPEG", "GIF"},  // 图片文件
    {"AVI"},                        // 视频文件
};
///////////////////////////////公共文件区,使用malloc的时候////////////////////////////////////////////
/* Array of logic drivers for FATFS */
FATFS* fs[FF_VOLUMES] = {NULL};  // 逻辑磁盘工作区.
/* Buffer for FATFS */
u8* fatbuf = NULL;  // SD卡数据缓存区
/* A main and a vice file to be used, and a temperary file */
FIL* main_file = NULL;
FIL* vice_file = NULL;
FIL* temp_file = NULL;
/* the number of byte be read or writen */
UINT br, bw;
/* File and directory infomation structure */
FILINFO fileinfo;  // 文件信息
DIR     dir;       // 目录
/* Cache for flash when do reading or writing operation */
u8* flash_buffer = NULL;
u8* dzk;
extern uint32_t* pageIndex;
///////////////////////////////////////////////////////////////////////////////////////
// 为exfuns申请内存
// 返回值:0,成功
// 1,失败
u8 exfuns_init(void)
{
    const uint8_t MAX_DZK_SIZE =
        (getSize(Font_Size_Max) / 8 + ((getSize(Font_Size_Max) % 8) ? 1 : 0)) *
        (getSize(Font_Size_Max));
    u8 i, res = 0;
    for (i = 0; i < FF_VOLUMES; i++) {
        fs[i] =
            (FATFS*)mymalloc(SRAMIN, sizeof(FATFS));  // 为磁盘i工作区申请内存
        if (!fs[i]) {
            printf("Fail to malloc for FATFS fs[%d]\r\n", i);
            break;
        }
    }
    /* malloc for FIL type */
    main_file = (FIL*)mymalloc(SRAMIN, sizeof(FIL));
    vice_file = (FIL*)mymalloc(SRAMIN, sizeof(FIL));
    temp_file = (FIL*)mymalloc(SRAMIN, sizeof(FIL));
    /* nakkic fir FATFS buffer */
    fatbuf = (u8*)mymalloc(SRAMIN, 512);

    flash_buffer = (u8*)mymalloc(SRAMIN, FLASH_BUFFER_SIZE);
    logBuffer   = (u8*)mymalloc(SRAMIN, LOG_BUFFER_SIZE);

	dzk = (u8*)mymalloc(SRAMIN, MAX_DZK_SIZE);
    pageIndex = (uint32_t*)mymalloc(SRAMEX, PAGE_INDEX_SIZE*sizeof(uint32_t));

    if ((i == FF_VOLUMES) && main_file && vice_file && temp_file && fatbuf &&
        flash_buffer && logBuffer && dzk && pageIndex) {
        res = 0; /* All succeed. */
    } else {
        res = 1; /* Failure occur. */
    }
    return res;
}

// 将小写字母转为大写字母,如果是数字,则保持不变.
u8 char_upper(u8 c)
{
    if (c < 'A')
        return c;  // 数字,保持不变.
    if (c >= 'a')
        return c - 0x20;  // 变为大写.
    else
        return c;  // 大写,保持不变
}
// 报告文件的类型
// fname:文件名
// 返回值:0XFF,表示无法识别的文件类型编号.
//		 其他,高四位表示所属大类,低四位表示所属小类.
u8 f_typetell(u8* fname)
{
    u8  tbuf[5];
    u8* attr = '\0';  // 后缀名
    u8  i    = 0, j;
    while (i < 250) {
        i++;
        if (*fname == '\0')
            break;  // 偏移到了最后了.
        fname++;
    }
    if (i == 250)
        return 0XFF;         // 错误的字符串.
    for (i = 0; i < 5; i++)  // 得到后缀名
    {
        fname--;
        if (*fname == '.') {
            fname++;
            attr = fname;
            break;
        }
    }
    strcpy((char*)tbuf, (const char*)attr);  // copy
    for (i = 0; i < 4; i++)
        tbuf[i] = char_upper(tbuf[i]);       // 全部变为大写
    for (i = 0; i < FILE_MAX_TYPE_NUM; i++)  // 大类对比
    {
        for (j = 0; j < FILE_MAX_SUBT_NUM; j++)  // 子类对比
        {
            if (*FILE_TYPE_TBL[i][j] == 0)
                break;  // 此组已经没有可对比的成员了.
            if (strcmp((const char*)FILE_TYPE_TBL[i][j], (const char*)tbuf) ==
                0)  // 找到了
            {
                return (i << 4) | j;
            }
        }
    }
    return 0XFF;  // 没找到
}

// 得到磁盘剩余容量
// drv:磁盘编号("0:"/"1:")
// total:总容量	 （单位KB）
// free:剩余容量	 （单位KB）
// 返回值:0,正常.其他,错误代码
u8 exf_getfree(u8* drv, u32* total, u32* free)
{
    FATFS* fs1;
    u8     res;
    u32    fre_clust = 0, fre_sect = 0, tot_sect = 0;
    // 得到磁盘信息及空闲簇数量
    res = (u32)f_getfree((const TCHAR*)drv, (DWORD*)&fre_clust, &fs1);
    if (res == 0) {
        tot_sect = (fs1->n_fatent - 2) * fs1->csize;  // 得到总扇区数
        fre_sect = fre_clust * fs1->csize;            // 得到空闲扇区数
#if FF_MIN_SS != FF_MAX_SS
#    if _MAX_SS != 512  // 扇区大小不是512字节,则转换为512字节
        tot_sect *= fs1->ssize / 512;
        fre_sect *= fs1->ssize / 512;
#    endif
#endif
        *total = tot_sect >> 1;  // 单位为KB
        *free  = fre_sect >> 1;  // 单位为KB
    }
    return res;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 文件复制
// 注意文件大小不要超过4GB.
// 将psrc文件,copy到pdst.
// fcpymsg,函数指针,用于实现拷贝时的信息显示
//         pname:文件/文件夹名
//         pct:百分比
//         mode:
//			[0]:更新文件名
//			[1]:更新百分比pct
//			[2]:更新文件夹
//			[3~7]:保留
// psrc,pdst:源文件和目标文件
// totsize:总大小(当totsize为0的时候,表示仅仅为单个文件拷贝)
// cpdsize:已复制了的大小.
// fwmode:文件写入模式
// 0:不覆盖原有的文件
// 1:覆盖原有的文件
// 返回值:0,正常
//     其他,错误,0XFF,强制退出
u8 exf_copy(u8 (*fcpymsg)(u8* pname, u8 pct, u8 mode),
            u8* psrc,
            u8* pdst,
            u32 totsize,
            u32 cpdsize,
            u8  fwmode)
{
    u8                 res;
    u16                br       = 0;
    u16                bw       = 0;
    FIL*               fsrc     = 0;
    FIL*               fdst     = 0;
    u8*                fbuf     = 0;
    u8                 curpct   = 0;
    unsigned long long lcpdsize = cpdsize;
    fsrc = (FIL*)mymalloc(SRAMIN, sizeof(FIL));  // 申请内存
    fdst = (FIL*)mymalloc(SRAMIN, sizeof(FIL));
    fbuf = (u8*)mymalloc(SRAMIN, 8192);
    if (fsrc == NULL || fdst == NULL || fbuf == NULL)
        res = 100;  // 前面的值留给fatfs
    else {
        if (fwmode == 0)
            fwmode = FA_CREATE_NEW;  // 不覆盖
        else
            fwmode = FA_CREATE_ALWAYS;  // 覆盖存在的文件

        res = f_open(fsrc, (const TCHAR*)psrc,
                     FA_READ | FA_OPEN_EXISTING);  // 打开只读文件
        if (res == 0)
            res = f_open(fdst, (const TCHAR*)pdst,
                         FA_WRITE | fwmode);  // 第一个打开成功,才开始打开第二个
        if (res == 0)                         // 两个都打开成功了
        {
            if (totsize == 0)  // 仅仅是单个文件复制
            {
                totsize  = fsrc->obj.objsize;
                lcpdsize = 0;
                curpct   = 0;
            } else
                curpct = (lcpdsize * 100) / totsize;  // 得到新百分比
            fcpymsg(psrc, curpct, 0X02);              // 更新百分比
            while (res == 0)                          // 开始复制
            {
                res = f_read(fsrc, fbuf, 8192, (UINT*)&br);  // 源头读出512字节
                if (res || br == 0)
                    break;
                res =
                    f_write(fdst, fbuf, (UINT)br, (UINT*)&bw);  // 写入目的文件
                lcpdsize += bw;
                if (curpct != (lcpdsize * 100) / totsize)  // 是否需要更新百分比
                {
                    curpct = (lcpdsize * 100) / totsize;
                    if (fcpymsg(psrc, curpct, 0X02))  // 更新百分比
                    {
                        res = 0XFF;  // 强制退出
                        break;
                    }
                }
                if (res || bw < br)
                    break;
            }
            f_close(fsrc);
            f_close(fdst);
        }
    }
    myfree(SRAMIN, fsrc);  // 释放内存
    myfree(SRAMIN, fdst);
    myfree(SRAMIN, fbuf);
    return res;
}

// 得到路径下的文件夹
// 返回值:0,路径就是个卷标号.
//     其他,文件夹名字首地址
u8* exf_get_src_dname(u8* dpfn)
{
    u16 temp = 0;
    while (*dpfn != 0) {
        dpfn++;
        temp++;
    }
    if (temp < 4)
        return 0;
    while ((*dpfn != 0x5c) && (*dpfn != 0x2f))
        dpfn--;  // 追述到倒数第一个"\"或者"/"处
    return ++dpfn;
}
// 得到文件夹大小
// 注意文件夹大小不要超过4GB.
// 返回值:0,文件夹大小为0,或者读取过程中发生了错误.
//     其他,文件夹大小.
u32 exf_fdsize(u8* fdname)
{
#define MAX_PATHNAME_DEPTH 512 + 1  // 最大目标文件路径+文件名深度
    u8       res      = 0;
    DIR*     fddir    = 0;  // 目录
    FILINFO* finfo    = 0;  // 文件信息
    u8*      pathname = 0;  // 目标文件夹路径+文件名
    u16      pathlen  = 0;  // 目标路径长度
    u32      fdsize   = 0;

    fddir = (DIR*)mymalloc(SRAMIN, sizeof(DIR));  // 申请内存
    finfo = (FILINFO*)mymalloc(SRAMIN, sizeof(FILINFO));
    if (fddir == NULL || finfo == NULL)
        res = 100;
    if (res == 0) {
        pathname = mymalloc(SRAMIN, MAX_PATHNAME_DEPTH);
        if (pathname == NULL)
            res = 101;
        if (res == 0) {
            pathname[0] = 0;
            strcat((char*)pathname, (const char*)fdname);  // 复制路径
            res = f_opendir(fddir, (const TCHAR*)fdname);  // 打开源目录
            if (res == 0)                                  // 打开目录成功
            {
                while (res == 0)  // 开始复制文件夹里面的东东
                {
                    res = f_readdir(fddir, finfo);  // 读取目录下的一个文件
                    if (res != FR_OK || finfo->fname[0] == 0)
                        break;  // 错误了/到末尾了,退出
                    if (finfo->fname[0] == '.')
                        continue;  // 忽略上级目录
                    if (finfo->fattrib &
                        0X10)  // 是子目录(文件属性,0X20,归档文件;0X10,子目录;)
                    {
                        pathlen = strlen(
                            (const char*)pathname);  // 得到当前路径的长度
                        strcat((char*)pathname, (const char*)"/");  // 加斜杠
                        strcat(
                            (char*)pathname,
                            (const char*)finfo->fname);  // 源路径加上子目录名字
                        // printf("\r\nsub folder:%s\r\n",pathname);
                        // //打印子目录名
                        fdsize +=
                            exf_fdsize(pathname);  // 得到子目录大小,递归调用
                        pathname[pathlen] = 0;  // 加入结束符
                    } else
                        fdsize += finfo->fsize;  // 非目录,直接加上文件的大小
                }
            }
            myfree(SRAMIN, pathname);
        }
    }
    myfree(SRAMIN, fddir);
    myfree(SRAMIN, finfo);
    if (res)
        return 0;
    else
        return fdsize;
}
// 文件夹复制
// 注意文件夹大小不要超过4GB.
// 将psrc文件夹,copy到pdst文件夹.
// pdst:必须形如"X:"/"X:XX"/"X:XX/XX"之类的.而且要实现确认上一级文件夹存在
// fcpymsg,函数指针,用于实现拷贝时的信息显示
//         pname:文件/文件夹名
//         pct:百分比
//         mode:
//			[0]:更新文件名
//			[1]:更新百分比pct
//			[2]:更新文件夹
//			[3~7]:保留
// psrc,pdst:源文件夹和目标文件夹
// totsize:总大小(当totsize为0的时候,表示仅仅为单个文件拷贝)
// cpdsize:已复制了的大小.
// fwmode:文件写入模式
// 0:不覆盖原有的文件
// 1:覆盖原有的文件
// 返回值:0,成功
//     其他,错误代码;0XFF,强制退出
u8 exf_fdcopy(u8 (*fcpymsg)(u8* pname, u8 pct, u8 mode),
              u8*  psrc,
              u8*  pdst,
              u32* totsize,
              u32* cpdsize,
              u8   fwmode)
{
#define MAX_PATHNAME_DEPTH 512 + 1  // 最大目标文件路径+文件名深度
    u8       res    = 0;
    DIR*     srcdir = 0;  // 源目录
    DIR*     dstdir = 0;  // 源目录
    FILINFO* finfo  = 0;  // 文件信息
    u8*      fn     = 0;  // 长文件名

    u8* dstpathname = 0;  // 目标文件夹路径+文件名
    u8* srcpathname = 0;  // 源文件夹路径+文件名

    u16 dstpathlen = 0;  // 目标路径长度
    u16 srcpathlen = 0;  // 源路径长度

    srcdir = (DIR*)mymalloc(SRAMIN, sizeof(DIR));  // 申请内存
    dstdir = (DIR*)mymalloc(SRAMIN, sizeof(DIR));
    finfo  = (FILINFO*)mymalloc(SRAMIN, sizeof(FILINFO));

    if (srcdir == NULL || dstdir == NULL || finfo == NULL)
        res = 100;
    if (res == 0) {
        dstpathname = mymalloc(SRAMIN, MAX_PATHNAME_DEPTH);
        srcpathname = mymalloc(SRAMIN, MAX_PATHNAME_DEPTH);
        if (dstpathname == NULL || srcpathname == NULL)
            res = 101;
        if (res == 0) {
            dstpathname[0] = 0;
            srcpathname[0] = 0;
            strcat((char*)srcpathname,
                   (const char*)psrc);  // 复制原始源文件路径
            strcat((char*)dstpathname,
                   (const char*)pdst);  // 复制原始目标文件路径
            res = f_opendir(srcdir, (const TCHAR*)psrc);  // 打开源目录
            if (res == 0)                                 // 打开目录成功
            {
                strcat((char*)dstpathname, (const char*)"/");  // 加入斜杠
                fn = exf_get_src_dname(psrc);
                if (fn == 0)  // 卷标拷贝
                {
                    dstpathlen              = strlen((const char*)dstpathname);
                    dstpathname[dstpathlen] = psrc[0];  // 记录卷标
                    dstpathname[dstpathlen + 1] = 0;    // 结束符
                } else
                    strcat((char*)dstpathname, (const char*)fn);  // 加文件名
                fcpymsg(fn, 0, 0X04);  // 更新文件夹名
                res = f_mkdir(
                    (const TCHAR*)
                        dstpathname);  // 如果文件夹已经存在,就不创建.如果不存在就创建新的文件夹.
                if (res == FR_EXIST)
                    res = 0;
                while (res == 0)  // 开始复制文件夹里面的东东
                {
                    res = f_readdir(srcdir, finfo);  // 读取目录下的一个文件
                    if (res != FR_OK || finfo->fname[0] == 0)
                        break;  // 错误了/到末尾了,退出
                    if (finfo->fname[0] == '.')
                        continue;                    // 忽略上级目录
                    fn         = (u8*)finfo->fname;  // 得到文件名
                    dstpathlen = strlen(
                        (const char*)dstpathname);  // 得到当前目标路径的长度
                    srcpathlen =
                        strlen((const char*)srcpathname);  // 得到源路径长度

                    strcat((char*)srcpathname,
                           (const char*)"/");  // 源路径加斜杠
                    if (finfo->fattrib &
                        0X10)  // 是子目录(文件属性,0X20,归档文件;0X10,子目录;)
                    {
                        strcat((char*)srcpathname,
                               (const char*)fn);  // 源路径加上子目录名字
                        res =
                            exf_fdcopy(fcpymsg, srcpathname, dstpathname,
                                       totsize, cpdsize, fwmode);  // 拷贝文件夹
                    } else                                         // 非目录
                    {
                        strcat((char*)dstpathname,
                               (const char*)"/");  // 目标路径加斜杠
                        strcat((char*)dstpathname,
                               (const char*)fn);  // 目标路径加文件名
                        strcat((char*)srcpathname,
                               (const char*)fn);  // 源路径加文件名
                        fcpymsg(fn, 0, 0X01);     // 更新文件名
                        res = exf_copy(fcpymsg, srcpathname, dstpathname,
                                       *totsize, *cpdsize, fwmode);  // 复制文件
                        *cpdsize += finfo->fsize;  // 增加一个文件大小
                    }
                    srcpathname[srcpathlen] = 0;  // 加入结束符
                    dstpathname[dstpathlen] = 0;  // 加入结束符
                }
            }
            myfree(SRAMIN, dstpathname);
            myfree(SRAMIN, srcpathname);
        }
    }
    myfree(SRAMIN, srcdir);
    myfree(SRAMIN, dstdir);
    myfree(SRAMIN, finfo);
    return res;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Loading file from SD Card to ex-flash */
uint8_t load_file_to_flash(char* fname, uint32_t flash_addr)
{
    uint8_t res = 0;
    UINT    br;
    //	uint8_t* tempbuf = NULL;
    uint32_t offset = 0;
    if (temp_file == NULL) {
        infinite_throw("temp_file is NULL");
    }
    res = f_open(temp_file, (TCHAR*)fname, FA_READ);
    if (res != FR_OK) {
        infinite_throw("Fail to open file: %s", fname);
    }
    //	tempbuf = mymalloc(SRAMEX, 4096);
    //	if (tempbuf == NULL) {
    //		ERROR_THROW("open and malloc succeed?");
    //		return 1;
    //	}
    ProgressWithInfo_Init(offset, f_size(temp_file),
                          fname, "SD Card", "Flash");
    log_n("%sLoading [%s] start.", ARROW_STRING, fname);
    print_log(Flash_Write_Log);
    // 死循环执行
    while (res == FR_OK) {
        res = f_read(temp_file, flash_buffer, FLASH_BUFFER_SIZE,
                     &br);  // 读取数据
        if (res != FR_OK) {
            break;  // 执行错误
        }
        //		if (offset/(5*FLASH_BUFFER_SIZE) == 0) {
        //			/* Show the progress of the update */
        //			//print_loading_log(fname, offset, f_size(temp_file));
        //		}
        W25QXX_Write(flash_buffer, flash_addr + offset,
                     FLASH_BUFFER_SIZE);  // 从 0 开始写 4096 个数据
        offset += br;
        /* Show the update progress */
        ProgressWithInfo_Update(offset);
        print_log(Flash_Write_Log);
        if (br != FLASH_BUFFER_SIZE) {
            break; /* Finish loading */
        }
        delay_ms(10);
    }

    //	print_loading_log(fname, offset, f_size(temp_file));
    log_n("%sLoading [%s] finished.", ARROW_STRING, fname);
    delay_ms(10);
    f_close(temp_file);
    return 0;
}

#if 0
void print_loading_log(char* fileName, uint32_t offset, uint32_t fileSize) {
	static float percent_record = 0;
	float percent = 100.0 * offset / fileSize;
	"Loading file [%s] to flash || ";
	if ((percent == 0) || (percent == 100) || ((percent - percent_record) > 5.0)) {
		sprintf((char*)log_buffer, "%7d/%7d || %.2f%%", 
						fileName, offset, fileSize, percent);
		log_n(log_buffer);
		delay_ms(10);
		if (percent != 0) {
			percent_record = percent;
		}
	}
	if (percent == 100) {
		percent_record = 0;
	}
}

void print_progress_log(uint32_t cur_val, uint32_t end_val) {
	"Loading ";
}
#endif
