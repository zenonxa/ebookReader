/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "BSP/sdio/sdio_sdcard.h"

/* Definitions of physical drive number for each drive */
//#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
//#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
//#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

#define USE_EX_FLASH			0	// 是否对外部 FLASH 应用 FATFS	【0:禁用 1:启用】
#define USE_DYNAMIC_MEM_ALLOC	1	// 是否使用动态分配				【0:禁用 1:启用】

#if USE_EX_FLASH
	#include "BSP/w25qxx/w25qxx.h"
#endif


#if USE_DYNAMIC_MEM_ALLOC
	#include "malloc.h"
#endif

// physical drive number
#define SD_CARD		0

#if USE_EX_FLASH
	#define EX_FLASH 1	//外部flash,卷标为1
#endif

//前12M字节给fatfs用,12M字节后,用于存放字库,字库占用3.09M.	剩余部分,给客户自己用
#if USE_EX_FLASH
	#define FLASH_SECTOR_SIZE 	512
	uint16_t	    FLASH_SECTOR_COUNT=2048*12;	//W25Q1218,前12M字节给FATFS占用
	#define FLASH_BLOCK_SIZE   	8     	//每个BLOCK有8个扇区
#endif


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS result = 0;

	switch (pdrv) {
		case SD_CARD:
			result = SD_Init();
			break;
#if USE_EX_FLASH
		case EX_FLASH://外部flash
			W25QXX_Init();
			FLASH_SECTOR_COUNT=2048*12;//W25Q1218,前12M字节给FATFS占用 
 			break;
#endif
		default:
			result = 1;
	}
	if (result)
		return STA_NOINIT;
	else
		return 0;
}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	uint8_t res = 0;
	if (!count)
		return RES_PARERR;	//count不能等于0，否则返回参数错误		 

	switch(pdrv)
	{
		case SD_CARD://SD卡
			res=SD_ReadDisk(buff,sector,count);	 
			while(res)//读出错
			{
				SD_Init();	//重新初始化SD卡
				res=SD_ReadDisk(buff,sector,count);	
				//printf("sd rd error:%d\r\n",res);
			}
			break;
#if USE_EX_FLASH
		case EX_FLASH://外部flash
			for(;count>0;count--)
			{
				W25QXX_Read(buff,sector*FLASH_SECTOR_SIZE,FLASH_SECTOR_SIZE);
				sector++;
				buff+=FLASH_SECTOR_SIZE;
			}
			res=0;
			break;
#endif
		default:
			res=1; 
	}
	
	//处理返回值，将SPI_SD_driver.c的返回值转成ff.c的返回值
    if(res==0x00)
		return RES_OK;	 
    else
		return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	uint8_t res = 0;
	if (!count)return RES_PARERR;//count不能等于0，否则返回参数错误	

	switch(pdrv)
	{
		case SD_CARD://SD卡
			res=SD_WriteDisk((uint8_t*)buff,sector,count);
			while(res)//写出错
			{
				SD_Init();	//重新初始化SD卡
				res=SD_WriteDisk((uint8_t*)buff,sector,count);	
				//printf("sd wr error:%d\r\n",res);
			}
			break;
		#if USE_EX_FLASH
		case EX_FLASH://外部flash
			for(;count>0;count--)
			{										    
				W25QXX_Write((uint8_t*)buff,sector*FLASH_SECTOR_SIZE,FLASH_SECTOR_SIZE);
				sector++;
				buff+=FLASH_SECTOR_SIZE;
			}
			res=0;
			break;
		#endif
		default:
			res=1; 
	}

    //处理返回值，将SPI_SD_driver.c的返回值转成ff.c的返回值
    if(res == 0x00)
		return RES_OK;	 
    else return
		RES_ERROR;	
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	if(pdrv==SD_CARD)//SD卡
	{
		switch(cmd)
	    {
		    case CTRL_SYNC:
				res = RES_OK; 
		        break;	 
		    case GET_SECTOR_SIZE:
				*(DWORD*)buff = 512; 
		        res = RES_OK;
		        break;	 
		    case GET_BLOCK_SIZE:
				*(WORD*)buff = SDCardInfo.LogBlockSize;
		        res = RES_OK;
		        break;	 
		    case GET_SECTOR_COUNT:
		        *(DWORD*)buff = SDCardInfo.LogBlockNbr;
		        res = RES_OK;
		        break;
		    default:
		        res = RES_PARERR;
		        break;
	    }
	}
		#if USE_EX_FLASH
	else if(pdrv==EX_FLASH)	//外部FLASH  
	{
	    switch(cmd)
	    {
		    case CTRL_SYNC:
				res = RES_OK; 
		        break;	 
		    case GET_SECTOR_SIZE:
		        *(WORD*)buff = FLASH_SECTOR_SIZE;
		        res = RES_OK;
		        break;	 
		    case GET_BLOCK_SIZE:
		        *(WORD*)buff = FLASH_BLOCK_SIZE;
		        res = RES_OK;
		        break;	 
		    case GET_SECTOR_COUNT:
		        *(DWORD*)buff = FLASH_SECTOR_COUNT;
		        res = RES_OK;
		        break;
		    default:
		        res = RES_PARERR;
		        break;
	    }
	}
	#endif
	else res=RES_ERROR;//其他的不支持

	return res;;
}


DWORD get_fattime (void)
{				 
	return 0;
}

#if USE_DYNAMIC_MEM_ALLOC
//动态分配内存
void *ff_memalloc (UINT size)			
{
	return (void*)mymalloc(SRAMIN,size);
}


//释放内存
void ff_memfree (void* mf)		 
{
	myfree(SRAMIN,mf);
}
#endif
