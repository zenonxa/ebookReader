#ifndef __MALLOC_H
#define __MALLOC_H
#include "SYSTEM/sys/sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板V3
//内存管理 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2019/9/19
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
#define USE_DYNAMIC_SRAMIN	0
#define USE_DYNAMIC_SRAMEX	1

#ifndef NULL
#define NULL 0
#endif

// 定义内存池
#if USE_DYNAMIC_SRAMIN
	#define SRAMIN	 0		//内部内存池
#endif
#if USE_DYNAMIC_SRAMEX
	#if (USE_DYNAMIC_SRAMIN == 0)
		#define SRAMEX	0		//外部内存池 
	#else
		#define SRAMEX	1
	#endif
#endif

// 计算BANK数
#if (USE_DYNAMIC_SRAMIN == 0) && (USE_DYNAMIC_SRAMEX == 0)
	#define SRAMBANK	0
#endif

#if USE_DYNAMIC_SRAMIN
	#ifndef SRAMBANK
		#define SRAMBANK	1
	#else 
		#undef SRAMBANK
		#define SRAMBANK	1
	#endif
#endif

#if USE_DYNAMIC_SRAMEX
	#ifndef SRAMBANK
		#define SRAMBANK	2
	#else 
		#undef SRAMBANK
		#define SRAMBANK	2
	#endif
#endif

#if USE_DYNAMIC_SRAMIN
	//mem1内存参数设定.mem1完全处于内部SRAM里面.
	#define MEM1_BLOCK_SIZE			32  	  						//内存块大小为32字节
	#define MEM1_MAX_SIZE			10*1024  						//最大管理内存 40K
	#define MEM1_ALLOC_TABLE_SIZE	MEM1_MAX_SIZE/MEM1_BLOCK_SIZE 	//内存表大小
#endif

#if USE_DYNAMIC_SRAMEX
	//mem2内存参数设定.mem2的内存池处于外部SRAM里面
	#define MEM2_BLOCK_SIZE			32  	  						//内存块大小为32字节
	#define MEM2_MAX_SIZE			50 *1024  						//最大管理内存960K
	#define MEM2_ALLOC_TABLE_SIZE	MEM2_MAX_SIZE/MEM2_BLOCK_SIZE 	//内存表大小 
#endif	 
 
 #if (USE_DYNAMIC_SRAMIN != 0) || (USE_DYNAMIC_SRAMEX != 0)
//内存管理控制器
struct _m_mallco_dev
{
	void (*init)(uint8_t);					//初始化
	uint8_t (*perused)(uint8_t);		  	    	//内存使用率
	uint8_t 	*membase[SRAMBANK];				//内存池 管理SRAMBANK个区域的内存
	uint16_t *memmap[SRAMBANK]; 				//内存管理状态表
	uint8_t  memrdy[SRAMBANK]; 				//内存管理是否就绪
};
extern struct _m_mallco_dev mallco_dev;	 //在mallco.c里面定义

void mymemset(void *s,uint8_t c,uint32_t count);	//设置内存
void mymemcpy(void *des,void *src,uint32_t n);//复制内存     
void my_mem_init(uint8_t memx);				//内存管理初始化函数(外/内部调用)
uint32_t my_mem_malloc(uint8_t memx,uint32_t size);	//内存分配(内部调用)
uint8_t my_mem_free(uint8_t memx,uint32_t offset);		//内存释放(内部调用)
uint8_t my_mem_perused(uint8_t memx);				//获得内存使用率(外/内部调用) 
////////////////////////////////////////////////////////////////////////////////
//用户调用函数
void myfree(uint8_t memx,void *ptr);  			//内存释放(外部调用)
void *mymalloc(uint8_t memx,uint32_t size);			//内存分配(外部调用)
void *myrealloc(uint8_t memx,void *ptr,uint32_t size);//重新分配内存(外部调用)
#endif /* (USE_DYNAMIC_SRAMIN != 0) || (USE_DYNAMIC_SRAMEX != 0) */

#endif
