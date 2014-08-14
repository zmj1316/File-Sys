/*+FHDR------------------------------------------------------------------
��Ȩ����:

���ı�-רעUSB��FAT�ļ�ϵͳ�Ĺ̼��о�
��ϵ��ʽ:qq 292942278  ����:tony_yang123@sina.com.cn

����FAT16����Ѵ���,����Բ���,������о���
������FAT32����,�շѵİ汾,�����������ϵ������

Copyright (c),
Tony Yang �CSpecialized in the USB and FAT's firmware research and design
Contact method:qq 292942278  e-mail:tony_yang123@sina.com.cn

This code of FAT16 is free code, you can test, design, research of it 
as your freedom, also the code with FAT32 code vision is for charge version
pls contact with author when you want it to buy of it.

Abstract:
$Id: main.C,v 1.1.1.1 2007/01/01 10:35:32 tony Exp $
-FHDR-------------------------------------------------------------------*/

#include "stdio.h"
#include "string.h"
#include "include\types.h"
#include "fat\fat.h"
FILE *file1;
/*===============================================================================
����
��ȡ�����ϵ�һ������
��ڣ�u8 *buf:���������׵�ַ,u32 sector:�����������
���ڣ�SUCC
===============================================================================*/
u8 read_flash_sector(u8 *buf,u32 sector)
{
  fseek(file1,sector * 512,0);
  fread(buf,1,512,file1);
  return(SUCC);
}

/*===============================================================================
����
�����ϵ�һ������д��
��ڣ�u8 *buf:���������׵�ַ,u32 sector:�����������
���ڣ�SUCC
===============================================================================*/
u8 write_flash_sector(u8 *buf,u32 sector)
{
  fseek(file1,sector * 512,0);
  fwrite(buf,1,512,file1);
  return(SUCC);
}

/*===============================================================================
����
��ʼ��ģ����̵�����
��ڣ���
���ڣ���
===============================================================================*/
u8 flash_management_sysinit()
{u32 i;
 u8 buf[512];
 if ((file1 = fopen("fat16.img","rb+")) == NULL)
 {     /*�Զ�����һ��IMG�ļ�*/
	   file1 = fopen("fat16.img","wb+");
       printf("make fat16.img in progress!\n");
	   for(i = 0;i<483328;i++)
	   {   if((i % 1000) == 0)
	           printf("#");
           fwrite(buf,1,512,file1);
	   }
       printf("\nmake fat16.img complete\n");
	   return(SUCC);
     }
     return(SUCC);
}

