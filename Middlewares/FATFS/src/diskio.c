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

#define USE_EX_FLASH			0	// �Ƿ���ⲿ FLASH Ӧ�� FATFS	��0:���� 1:���á�
#define USE_DYNAMIC_MEM_ALLOC	1	// �Ƿ�ʹ�ö�̬����				��0:���� 1:���á�

#if USE_EX_FLASH
	#include "BSP/w25qxx/w25qxx.h"
#endif


#if USE_DYNAMIC_MEM_ALLOC
	#include "malloc.h"
#endif

// physical drive number
#define SD_CARD		0

#if USE_EX_FLASH
	#define EX_FLASH 1	//�ⲿflash,���Ϊ1
#endif

//ǰ12M�ֽڸ�fatfs��,12M�ֽں�,���ڴ���ֿ�,�ֿ�ռ��3.09M.	ʣ�ಿ��,���ͻ��Լ���
#if USE_EX_FLASH
	#define FLASH_SECTOR_SIZE 	512
	uint16_t	    FLASH_SECTOR_COUNT=2048*12;	//W25Q1218,ǰ12M�ֽڸ�FATFSռ��
	#define FLASH_BLOCK_SIZE   	8     	//ÿ��BLOCK��8������
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
		case EX_FLASH://�ⲿflash
			W25QXX_Init();
			FLASH_SECTOR_COUNT=2048*12;//W25Q1218,ǰ12M�ֽڸ�FATFSռ�� 
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
		return RES_PARERR;	//count���ܵ���0�����򷵻ز�������		 

	switch(pdrv)
	{
		case SD_CARD://SD��
			res=SD_ReadDisk(buff,sector,count);	 
			while(res)//������
			{
				SD_Init();	//���³�ʼ��SD��
				res=SD_ReadDisk(buff,sector,count);	
				//printf("sd rd error:%d\r\n",res);
			}
			break;
#if USE_EX_FLASH
		case EX_FLASH://�ⲿflash
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
	
	//������ֵ����SPI_SD_driver.c�ķ���ֵת��ff.c�ķ���ֵ
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
	if (!count)return RES_PARERR;//count���ܵ���0�����򷵻ز�������	

	switch(pdrv)
	{
		case SD_CARD://SD��
			res=SD_WriteDisk((uint8_t*)buff,sector,count);
			while(res)//д����
			{
				SD_Init();	//���³�ʼ��SD��
				res=SD_WriteDisk((uint8_t*)buff,sector,count);	
				//printf("sd wr error:%d\r\n",res);
			}
			break;
		#if USE_EX_FLASH
		case EX_FLASH://�ⲿflash
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

    //������ֵ����SPI_SD_driver.c�ķ���ֵת��ff.c�ķ���ֵ
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
	if(pdrv==SD_CARD)//SD��
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
	else if(pdrv==EX_FLASH)	//�ⲿFLASH  
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
	else res=RES_ERROR;//�����Ĳ�֧��

	return res;;
}


DWORD get_fattime (void)
{				 
	return 0;
}

#if USE_DYNAMIC_MEM_ALLOC
//��̬�����ڴ�
void *ff_memalloc (UINT size)			
{
	return (void*)mymalloc(SRAMIN,size);
}


//�ͷ��ڴ�
void ff_memfree (void* mf)		 
{
	myfree(SRAMIN,mf);
}
#endif
