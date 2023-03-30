#ifndef _SDMMC_SDCARD_H
#define _SDMMC_SDCARD_H
#include "SYSTEM/sys/sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F103开发板
//SD卡驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2019/9/19
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

#define SD_TIMEOUT 			((uint32_t)100000000)  	//超时时间
#define SD_TRANSFER_OK     	((uint8_t)0x00)
#define SD_TRANSFER_BUSY   	((uint8_t)0x01) 

extern SD_HandleTypeDef        SDCARD_Handler;     	//SD卡句柄
extern HAL_SD_CardInfoTypeDef  SDCardInfo;         	//SD卡信息结构体

uint8_t SD_Init(void);
uint8_t SD_GetCardInfo(HAL_SD_CardInfoTypeDef *cardinfo);
uint8_t SD_GetCardState(void);
uint8_t SD_ReadDisk(uint8_t* buf,uint32_t sector,uint32_t cnt);
uint8_t SD_WriteDisk(uint8_t *buf,uint32_t sector,uint32_t cnt);
uint8_t SD_ReadBlocks_DMA(uint32_t *buf,uint64_t sector,uint32_t blocksize,uint32_t cnt);
uint8_t SD_WriteBlocks_DMA(uint32_t *buf,uint64_t sector,uint32_t blocksize,uint32_t cnt);
#endif
