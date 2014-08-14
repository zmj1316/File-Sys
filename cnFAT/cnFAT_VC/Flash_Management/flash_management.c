/*
+FHDR------------------------------------------------------------------
Copyright (c),
Tony Yang �Cspecialized in fat usb firmware development  
Contact:qq 292942278  e-mail:tony_yang123@sina.com.cn
;;;;;;;;;;
Abstract:
Sector Read/Write Driver
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
$Id: flash_management.c,v 1.1.1.1 2007/02/26 14:01:12 Design Exp $
-FHDR-------------------------------------------------------------------
*/

#include "stdio.h"
#include "string.h"
#include <include\types.h>
static FILE *file1,*file2;
u32 Readnumbers = 0;
u32 Writenumbers = 0;
/*
===============================================================================
����
Read flash sector(512bytes)
��ڣ�u8 *buf:�������׵�ַ,u32 sector:����������
���ڣ�SUCC
===============================================================================
*/
u8 read_flash_sector(u8 *buf,u32 sector,u8 disk_id)
{  
	switch(disk_id)
	{
	  case 0 : {Readnumbers++;
                 fseek(file1,sector * 512,0);
                 fread(buf, 1,512,file1);
                 break;
			   }
      case 1 : {
                 fseek(file2,sector * 512,0);
                 fread(buf, 1,512,file2);
                 break;
			   }
      default: return(FAIL);
	}
return(SUCC);
}

/*
===============================================================================
����
Write flash sector(512bytes)
��ڣ�u8 *buf:�������׵�ַ,u32 sector:����������
���ڣ�SUCC
===============================================================================
*/  
u8 write_flash_sector(u8 *buf,u32 sector, u8 disk_id)
{
  switch(disk_id)
  {
  case 0 : {    Writenumbers++;
                fseek(file1,sector * 512,0);
                fwrite(buf, 1,512,file1);
				fflush(file1);
                return(SUCC);
			  }
     case 1 : {
                fseek(file2,sector * 512,0);
                fwrite(buf, 1,512,file2);
				fflush(file2);
                return(SUCC);
			 }
     default: return(FAIL);    
  }
return(SUCC);
}

/*
===============================================================================
����
��ģ�����IMG�ļ�fat16.img
��ڣ���
���ڣ���
===============================================================================
*/
u8 flash_management_sysinit()
{ u32 i;
  u8 buf[512];
  if ((file1 = fopen("fat32.img","rb+")) == NULL)
     {
	  printf("make fat32.img in progress!\n");
	  file1 = fopen("fat32.img","wb+");
      for(i = 0;i<(270336*2*2);i++)
	  {
	   if((i%1000) == 0)
		   printf("#");
	   fwrite(buf, 1,512,file1);
	  
	  }
     }
  if ((file2 = fopen("fat16.img","rb+")) == NULL)
     {printf("make fat16.img in progress!\n");
	  file2 = fopen("fat16.img","wb+");
      for(i = 0;i<270336;i++)
	  {
	   if((i%1000) == 0)
		   printf("#");
	   fwrite(buf, 1,512,file2);
	  
	  }
     }
  return(SUCC);  
    
}

/*
+FFTR--------------------------------------------------------------------
$Log: flash_management.c,v $


-FFTR--------------------------------------------------------------------
*/