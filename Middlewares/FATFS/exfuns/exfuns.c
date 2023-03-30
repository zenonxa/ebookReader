#include "string.h"
#include "FATFS/exfuns/exfuns.h"
#include "FATFS/exfuns/fattester.h"	
#include "malloc.h"
#include "SYSTEM/usart/usart.h"
#include "SYSTEM/delay/delay.h"
#include "FATFS/src/ff.h"
#include "BSP/W25QXX/w25qxx.h"
#include "log.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//FATFS ��չ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2019/9/28
//�汾��V1.1
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved	
//********************************************************************************
//����˵��
//V1.1
//����exf_copy����,�ļ�������ʾ�����bug
////////////////////////////////////////////////////////////////////////////////// 	


#define FILE_MAX_TYPE_NUM		7	//���FILE_MAX_TYPE_NUM������
#define FILE_MAX_SUBT_NUM		4	//���FILE_MAX_SUBT_NUM��С��


 //�ļ������б�
u8*const FILE_TYPE_TBL[FILE_MAX_TYPE_NUM][FILE_MAX_SUBT_NUM]=
{
	{"BIN"},							//BIN�ļ�
	{"LRC"},							//LRC�ļ�
	{"NES", "SMS"},						//NES/SMS�ļ�
	{"TXT", "C",	"H"},				//�ı��ļ�
	{"WAV", "MP3",	"APE",	"FLAC"},	//֧�ֵ������ļ�
	{"BMP", "JPG",	"JPEG",	"GIF"},		//ͼƬ�ļ�
	{"AVI"},							//��Ƶ�ļ�
};
///////////////////////////////�����ļ���,ʹ��malloc��ʱ��////////////////////////////////////////////
/* Array of logic drivers for FATFS */
FATFS *fs[FF_VOLUMES]= {NULL};//�߼����̹�����.
/* Buffer for FATFS */
u8 *fatbuf = NULL;			//SD�����ݻ�����
/* A main and a vice file to be used, and a temperary file */
FIL *main_file = NULL;
FIL *vice_file = NULL;
FIL *temp_file = NULL;
/* the number of byte be read or writen */
UINT br,bw;
/* File and directory infomation structure */
FILINFO fileinfo;	// �ļ���Ϣ
DIR dir;  			// Ŀ¼
/* Cache for flash when do reading or writing operation */
u8* flash_buffer = NULL;
///////////////////////////////////////////////////////////////////////////////////////
//Ϊexfuns�����ڴ�
//����ֵ:0,�ɹ�
//1,ʧ��
u8 exfuns_init(void)
{
	u8 i, res = 0;
	for(i=0; i<FF_VOLUMES; i++)
	{
		fs[i] = (FATFS*)mymalloc(SRAMIN,sizeof(FATFS));	//Ϊ����i�����������ڴ�
		if(!fs[i]) {
			printf("Fail to malloc for FATFS fs[%d]\r\n", i);
			break;
		}
	}
	/* malloc for FIL type */
	main_file		= (FIL*)mymalloc(SRAMIN, sizeof(FIL));
	vice_file 		= (FIL*)mymalloc(SRAMIN, sizeof(FIL));
	temp_file		= (FIL*)mymalloc(SRAMIN, sizeof(FIL));
	/* nakkic fir FATFS buffer */
	fatbuf 			= (u8*)	mymalloc(SRAMIN, 512);
	
	flash_buffer	= (u8*) mymalloc(SRAMIN, FLASH_BUFFER_SIZE);
	log_buffer		= (u8*) mymalloc(SRAMIN, LOG_BUFFER_SIZE);
	if((i==FF_VOLUMES) && main_file && vice_file && temp_file && 
			fatbuf && flash_buffer && log_buffer) {
		res = 0;		/* All succeed. */
	} else {
		res = 1;		/* Failure occur. */
	}
	return res;
}

//��Сд��ĸתΪ��д��ĸ,���������,�򱣳ֲ���.
u8 char_upper(u8 c)
{
	if(c<'A')return c;//����,���ֲ���.
	if(c>='a')return c-0x20;//��Ϊ��д.
	else return c;//��д,���ֲ���
}	      
//�����ļ�������
//fname:�ļ���
//����ֵ:0XFF,��ʾ�޷�ʶ����ļ����ͱ��.
//		 ����,����λ��ʾ��������,����λ��ʾ����С��.
u8 f_typetell(u8 *fname)
{
	u8 tbuf[5];
	u8 *attr='\0';//��׺��
	u8 i=0,j;
	while(i<250)
	{
		i++;
		if(*fname=='\0')break;//ƫ�Ƶ��������.
		fname++;
	}
	if(i==250)return 0XFF;//������ַ���.
 	for(i=0;i<5;i++)//�õ���׺��
	{
		fname--;
		if(*fname=='.')
		{
			fname++;
			attr=fname;
			break;
		}
  	}
	strcpy((char *)tbuf,(const char*)attr);//copy
 	for(i=0;i<4;i++)tbuf[i]=char_upper(tbuf[i]);//ȫ����Ϊ��д 
	for(i=0;i<FILE_MAX_TYPE_NUM;i++)	//����Ա�
	{
		for(j=0;j<FILE_MAX_SUBT_NUM;j++)//����Ա�
		{
			if(*FILE_TYPE_TBL[i][j]==0)break;//�����Ѿ�û�пɶԱȵĳ�Ա��.
			if(strcmp((const char *)FILE_TYPE_TBL[i][j],(const char *)tbuf)==0)//�ҵ���
			{
				return (i<<4)|j;
			}
		}
	}
	return 0XFF;//û�ҵ�		 			   
}

//�õ�����ʣ������
//drv:���̱��("0:"/"1:")
//total:������	 ����λKB��
//free:ʣ������	 ����λKB��
//����ֵ:0,����.����,�������
u8 exf_getfree(u8 *drv,u32 *total,u32 *free)
{
	FATFS *fs1;
	u8 res;
    u32 fre_clust=0, fre_sect=0, tot_sect=0;
    //�õ�������Ϣ�����д�����
    res =(u32)f_getfree((const TCHAR*)drv, (DWORD*)&fre_clust, &fs1);
    if(res==0)
	{											   
	    tot_sect=(fs1->n_fatent-2)*fs1->csize;	//�õ���������
	    fre_sect=fre_clust*fs1->csize;			//�õ�����������	   
#if FF_MIN_SS != FF_MAX_SS
#if _MAX_SS!=512				  				//������С����512�ֽ�,��ת��Ϊ512�ֽ�
		tot_sect*=fs1->ssize/512;
		fre_sect*=fs1->ssize/512;
#endif	  
#endif
		*total=tot_sect>>1;	//��λΪKB
		*free=fre_sect>>1;	//��λΪKB 
 	}
	return res;
}		   
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//�ļ�����
//ע���ļ���С��Ҫ����4GB.
//��psrc�ļ�,copy��pdst.
//fcpymsg,����ָ��,����ʵ�ֿ���ʱ����Ϣ��ʾ
//        pname:�ļ�/�ļ�����
//        pct:�ٷֱ�
//        mode:
//			[0]:�����ļ���
//			[1]:���°ٷֱ�pct
//			[2]:�����ļ���
//			[3~7]:����
//psrc,pdst:Դ�ļ���Ŀ���ļ�
//totsize:�ܴ�С(��totsizeΪ0��ʱ��,��ʾ����Ϊ�����ļ�����)
//cpdsize:�Ѹ����˵Ĵ�С.
//fwmode:�ļ�д��ģʽ
//0:������ԭ�е��ļ�
//1:����ԭ�е��ļ�
//����ֵ:0,����
//    ����,����,0XFF,ǿ���˳�
u8 exf_copy(u8(*fcpymsg)(u8*pname,u8 pct,u8 mode),u8 *psrc,u8 *pdst,u32 totsize,u32 cpdsize,u8 fwmode)
{
	u8 res;
    u16 br=0;
	u16 bw=0;
	FIL *fsrc=0;
	FIL *fdst=0;
	u8 *fbuf=0;
	u8 curpct=0;
	unsigned long long lcpdsize=cpdsize; 
 	fsrc=(FIL*)mymalloc(SRAMIN,sizeof(FIL));//�����ڴ�
 	fdst=(FIL*)mymalloc(SRAMIN,sizeof(FIL));
	fbuf=(u8*)mymalloc(SRAMIN,8192);
  	if(fsrc==NULL||fdst==NULL||fbuf==NULL)res=100;//ǰ���ֵ����fatfs
	else
	{   
		if(fwmode==0)fwmode=FA_CREATE_NEW;//������
		else fwmode=FA_CREATE_ALWAYS;	  //���Ǵ��ڵ��ļ�
		 
	 	res=f_open(fsrc,(const TCHAR*)psrc,FA_READ|FA_OPEN_EXISTING);	//��ֻ���ļ�
	 	if(res==0)res=f_open(fdst,(const TCHAR*)pdst,FA_WRITE|fwmode); 	//��һ���򿪳ɹ�,�ſ�ʼ�򿪵ڶ���
		if(res==0)//�������򿪳ɹ���
		{
			if(totsize==0)//�����ǵ����ļ�����
			{
				totsize=fsrc->obj.objsize;
				lcpdsize=0;
				curpct=0;
		 	}else curpct=(lcpdsize*100)/totsize;	//�õ��°ٷֱ�
			fcpymsg(psrc,curpct,0X02);			//���°ٷֱ�
			while(res==0)//��ʼ����
			{
				res=f_read(fsrc,fbuf,8192,(UINT*)&br);	//Դͷ����512�ֽ�
				if(res||br==0)break;
				res=f_write(fdst,fbuf,(UINT)br,(UINT*)&bw);	//д��Ŀ���ļ�
				lcpdsize+=bw;
				if(curpct!=(lcpdsize*100)/totsize)//�Ƿ���Ҫ���°ٷֱ�
				{
					curpct=(lcpdsize*100)/totsize;
					if(fcpymsg(psrc,curpct,0X02))//���°ٷֱ�
					{
						res=0XFF;//ǿ���˳�
						break;
					}
				}			     
				if(res||bw<br)break;       
			}
		    f_close(fsrc);
		    f_close(fdst);
		}
	}
	myfree(SRAMIN,fsrc);//�ͷ��ڴ�
	myfree(SRAMIN,fdst);
	myfree(SRAMIN,fbuf);
	return res;
}

//�õ�·���µ��ļ���
//����ֵ:0,·�����Ǹ�����.
//    ����,�ļ��������׵�ַ
u8* exf_get_src_dname(u8* dpfn)
{
	u16 temp=0;
 	while(*dpfn!=0)
	{
		dpfn++;
		temp++;	
	}
	if(temp<4)return 0; 
	while((*dpfn!=0x5c)&&(*dpfn!=0x2f))dpfn--;	//׷����������һ��"\"����"/"�� 
	return ++dpfn;
}
//�õ��ļ��д�С
//ע���ļ��д�С��Ҫ����4GB.
//����ֵ:0,�ļ��д�СΪ0,���߶�ȡ�����з����˴���.
//    ����,�ļ��д�С.
u32 exf_fdsize(u8 *fdname)
{
#define MAX_PATHNAME_DEPTH	512+1	//���Ŀ���ļ�·��+�ļ������
	u8 res=0;	  
    DIR *fddir=0;		//Ŀ¼
	FILINFO *finfo=0;	//�ļ���Ϣ
	u8 * pathname=0;	//Ŀ���ļ���·��+�ļ���
 	u16 pathlen=0;		//Ŀ��·������
	u32 fdsize=0;

	fddir=(DIR*)mymalloc(SRAMIN,sizeof(DIR));//�����ڴ�
 	finfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));
   	if(fddir==NULL||finfo==NULL)res=100;
	if(res==0)
	{ 
 		pathname=mymalloc(SRAMIN,MAX_PATHNAME_DEPTH);	    
 		if(pathname==NULL)res=101;	   
 		if(res==0)
		{
			pathname[0]=0;	    
			strcat((char*)pathname,(const char*)fdname); //����·��	
		    res=f_opendir(fddir,(const TCHAR*)fdname); 		//��ԴĿ¼
		    if(res==0)//��Ŀ¼�ɹ� 
			{														   
				while(res==0)//��ʼ�����ļ�������Ķ���
				{
			        res=f_readdir(fddir,finfo);						//��ȡĿ¼�µ�һ���ļ�
			        if(res!=FR_OK||finfo->fname[0]==0)break;		//������/��ĩβ��,�˳�
			        if(finfo->fname[0]=='.')continue;     			//�����ϼ�Ŀ¼
					if(finfo->fattrib&0X10)//����Ŀ¼(�ļ�����,0X20,�鵵�ļ�;0X10,��Ŀ¼;)
					{
 						pathlen=strlen((const char*)pathname);		//�õ���ǰ·���ĳ���
						strcat((char*)pathname,(const char*)"/");	//��б��
						strcat((char*)pathname,(const char*)finfo->fname);	//Դ·��������Ŀ¼����
 						//printf("\r\nsub folder:%s\r\n",pathname);	//��ӡ��Ŀ¼��
						fdsize+=exf_fdsize(pathname);				//�õ���Ŀ¼��С,�ݹ����
						pathname[pathlen]=0;						//���������
					}else fdsize+=finfo->fsize;						//��Ŀ¼,ֱ�Ӽ����ļ��Ĵ�С
						
				} 
		    }	  
  			myfree(SRAMIN,pathname);	     
		}
 	}
	myfree(SRAMIN,fddir);    
	myfree(SRAMIN,finfo);
	if(res)return 0;
	else return fdsize;
}	  
//�ļ��и���
//ע���ļ��д�С��Ҫ����4GB.
//��psrc�ļ���,copy��pdst�ļ���.
//pdst:��������"X:"/"X:XX"/"X:XX/XX"֮���.����Ҫʵ��ȷ����һ���ļ��д���
//fcpymsg,����ָ��,����ʵ�ֿ���ʱ����Ϣ��ʾ
//        pname:�ļ�/�ļ�����
//        pct:�ٷֱ�
//        mode:
//			[0]:�����ļ���
//			[1]:���°ٷֱ�pct
//			[2]:�����ļ���
//			[3~7]:����
//psrc,pdst:Դ�ļ��к�Ŀ���ļ���
//totsize:�ܴ�С(��totsizeΪ0��ʱ��,��ʾ����Ϊ�����ļ�����)
//cpdsize:�Ѹ����˵Ĵ�С.
//fwmode:�ļ�д��ģʽ
//0:������ԭ�е��ļ�
//1:����ԭ�е��ļ�
//����ֵ:0,�ɹ�
//    ����,�������;0XFF,ǿ���˳�
u8 exf_fdcopy(u8(*fcpymsg)(u8*pname,u8 pct,u8 mode),u8 *psrc,u8 *pdst,u32 *totsize,u32 *cpdsize,u8 fwmode)
{
#define MAX_PATHNAME_DEPTH	512+1	//���Ŀ���ļ�·��+�ļ������
	u8 res=0;	  
    DIR *srcdir=0;		//ԴĿ¼
	DIR *dstdir=0;		//ԴĿ¼
	FILINFO *finfo=0;	//�ļ���Ϣ
	u8 *fn=0;   		//���ļ���

	u8 * dstpathname=0;	//Ŀ���ļ���·��+�ļ���
	u8 * srcpathname=0;	//Դ�ļ���·��+�ļ���
	
 	u16 dstpathlen=0;	//Ŀ��·������
 	u16 srcpathlen=0;	//Դ·������

  
	srcdir=(DIR*)mymalloc(SRAMIN,sizeof(DIR));//�����ڴ�
 	dstdir=(DIR*)mymalloc(SRAMIN,sizeof(DIR));
	finfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));

   	if(srcdir==NULL||dstdir==NULL||finfo==NULL)res=100;
	if(res==0)
	{ 
 		dstpathname=mymalloc(SRAMIN,MAX_PATHNAME_DEPTH);
		srcpathname=mymalloc(SRAMIN,MAX_PATHNAME_DEPTH);
 		if(dstpathname==NULL||srcpathname==NULL)res=101;	   
 		if(res==0)
		{
			dstpathname[0]=0;
			srcpathname[0]=0;
			strcat((char*)srcpathname,(const char*)psrc); 	//����ԭʼԴ�ļ�·��	
			strcat((char*)dstpathname,(const char*)pdst); 	//����ԭʼĿ���ļ�·��	
		    res=f_opendir(srcdir,(const TCHAR*)psrc); 		//��ԴĿ¼
		    if(res==0)//��Ŀ¼�ɹ� 
			{
  				strcat((char*)dstpathname,(const char*)"/");//����б��
 				fn=exf_get_src_dname(psrc);
				if(fn==0)//��꿽��
				{
					dstpathlen=strlen((const char*)dstpathname);
					dstpathname[dstpathlen]=psrc[0];	//��¼���
					dstpathname[dstpathlen+1]=0;		//������ 
				}else strcat((char*)dstpathname,(const char*)fn);//���ļ���		
 				fcpymsg(fn,0,0X04);//�����ļ�����
				res=f_mkdir((const TCHAR*)dstpathname);//����ļ����Ѿ�����,�Ͳ�����.��������ھʹ����µ��ļ���.
				if(res==FR_EXIST)res=0;
				while(res==0)//��ʼ�����ļ�������Ķ���
				{
			        res=f_readdir(srcdir,finfo);					//��ȡĿ¼�µ�һ���ļ�
			        if(res!=FR_OK||finfo->fname[0]==0)break;		//������/��ĩβ��,�˳�
			        if(finfo->fname[0]=='.')continue;     			//�����ϼ�Ŀ¼
					fn=(u8*)finfo->fname; 							//�õ��ļ���
					dstpathlen=strlen((const char*)dstpathname);	//�õ���ǰĿ��·���ĳ���
					srcpathlen=strlen((const char*)srcpathname);	//�õ�Դ·������

					strcat((char*)srcpathname,(const char*)"/");//Դ·����б��
 					if(finfo->fattrib&0X10)//����Ŀ¼(�ļ�����,0X20,�鵵�ļ�;0X10,��Ŀ¼;)
					{
						strcat((char*)srcpathname,(const char*)fn);		//Դ·��������Ŀ¼����
						res=exf_fdcopy(fcpymsg,srcpathname,dstpathname,totsize,cpdsize,fwmode);	//�����ļ���
					}else //��Ŀ¼
					{
						strcat((char*)dstpathname,(const char*)"/");//Ŀ��·����б��
						strcat((char*)dstpathname,(const char*)fn);	//Ŀ��·�����ļ���
						strcat((char*)srcpathname,(const char*)fn);	//Դ·�����ļ���
 						fcpymsg(fn,0,0X01);//�����ļ���
						res=exf_copy(fcpymsg,srcpathname,dstpathname,*totsize,*cpdsize,fwmode);//�����ļ�
						*cpdsize+=finfo->fsize;//����һ���ļ���С
					}
					srcpathname[srcpathlen]=0;//���������
					dstpathname[dstpathlen]=0;//���������	    
				} 
		    }	  
  			myfree(SRAMIN,dstpathname);
 			myfree(SRAMIN,srcpathname); 
		}
 	}
	myfree(SRAMIN,srcdir);
	myfree(SRAMIN,dstdir);
	myfree(SRAMIN,finfo);
    return res;	  
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Loading file from SD Card to ex-flash */
uint8_t load_file_to_flash(char* fname, uint32_t flash_addr) {
	uint8_t res = 0;
	UINT br;
//	uint8_t* tempbuf = NULL;
	uint32_t offset = 0;
	if (temp_file == NULL) {
		infinite_throw("temp_file is NULL");
	}
	res = f_open(temp_file, fname, FA_READ);
	if (res != FR_OK) {
		infinite_throw("Fail to open file: %s", fname);
	}
//	tempbuf = mymalloc(SRAMEX, 4096);
//	if (tempbuf == NULL) {
//		ERROR_THROW("open and malloc succeed?");
//		return 1;
//	}
	ProgressWithInfo_Init(&logParam.progressWithInfo, offset, f_size(temp_file), 
							fname, "SD Card", "Flash");
	print_log(Flash_Write_Log, &logParam);
	//��ѭ��ִ��
	while(res == FR_OK) {
		res = f_read(temp_file, flash_buffer, FLASH_BUFFER_SIZE, &br); //��ȡ����
		if(res != FR_OK) {
			break; //ִ�д���
		}
//		if (offset/(5*FLASH_BUFFER_SIZE) == 0) {
//			/* Show the progress of the update */
//			//print_loading_log(fname, offset, f_size(temp_file));
//		}
		W25QXX_Write(flash_buffer, flash_addr+offset, FLASH_BUFFER_SIZE); //�� 0 ��ʼд 4096 ������
		offset += br;
		/* Show the update progress */
		ProgressWithInfo_Update(&logParam.progressWithInfo, offset);
		print_log(Flash_Write_Log, &logParam);
		if(br != FLASH_BUFFER_SIZE) {
			break;		/* Finish loading */
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
