/*+FHDR------------------------------------------------------------------
版权所有:

杨文斌-专注USB与FAT文件系统的固件研究
联系方式:qq 292942278  电邮:tony_yang123@sina.com.cn

代码FAT16是免费代码,你可以测试,设计与研究它
我们有FAT32代码,收费的版本,你可与作者联系并购买

Copyright (c),
Tony Yang –Specialized in the USB and FAT's firmware research and design
Contact method:qq 292942278  e-mail:tony_yang123@sina.com.cn

This code of FAT16 is free code, you can test, design, research of it 
as your freedom, also the code with FAT32 code vision is for charge version
pls contact with author when you want it to buy of it.

Abstract:
$Id: main.C,v 1.1.1.1 2007/01/01 10:35:32 tony Exp $
-FHDR-------------------------------------------------------------------*/

#include <stdio.h>

#include "include\types.h"

#include "fat\fat.h"

#include "Flash_Management\Flash_Management.h" 

#include "include\FAT_cfg.h" 

/*===============================================================================
函数
main()
用户可以通过修改其中的代码，来达到测试文件系统的功能的目的
入口：无
出口：无
===============================================================================*/ 
FILE *file2;
void main(void)
{
  u32 cc,bb;
  u16 COUNT;
  u8 a,ATTR,j;
  u8 HANDLE1; 
  u8 buf[65535];
  u8 ddd[] = "c:\\ok\\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaAAAAAAAAAAAAAAAaaaaaaaaa.rar";
  
  //初始化虚拟磁盘的驱动
  flash_management_sysinit();
  FAT_filesystem_autoformat(0,FAT16,483328);
  FAT_filesystem_initialiation();

  //调用函数volume_inquiry,查询分区容量和剩余空间
  volume_inquiry('c',&cc,&bb);

  //打印虚拟磁盘分区容量和剩余空间
  printf("Volume Capacity: %ld\n",cc);
  printf("Volume FreeSpace: %ld\n",bb);
  scanf("%c",&a);

  /*建立目录CREATE_FOLDER_TEST，用于测试建立目录函数create_floder()*/

  create_floder("CREATE_FOLDER_TEST");
  create_floder("CREATE_FOLDER_TEST");//目录再建失败, 不可以建重复的目录


  /*目录建立测试*/
  create_floder("a");
  create_floder("a\\b");
  create_floder("a\\b\\c");
  create_floder("a\\b\\c\\d");


  /*建文件在目录CREATE_FOLDER_TEST下，用于测试建立文件函数create_file()*/
  
  create_file("C:\\CREATE_FOLDER_TEST\\created_file.txt");

  create_file("C:\\CREATE_FOLDER_TEST\\created_file.txt");//文件再建失败,不可以建重复的文件


  /* 重命名文件CREATE_FOLDER_TEST\\created_file.txt为"created_file_new.txt",用于测试文件重命名函数rename_file() */
  rename_file("C:\\CREATE_FOLDER_TEST\\created_file.txt","created_file_new.txt");
	
  /*  读取目录CREATE_FOLDER_TEST下面的所有ENTRY，打印出来到屏幕*/
  cd_folder("CREATE_FOLDER_TEST",0);
 
  a = 0; //枚举模式初始设定
  while(folder_enumeration(buf,a,&ATTR) == SUCC)
  { 
      //打印读取的Directory ENTRY及其attribute
	  printf("\nreaded entry=%s Attr = %x",buf,ATTR);

     if(a == 0)
	   a = 1;//枚举模式修改为枚举继续
  }

  scanf("%c",&a);	

  cd_folder(" ",1); //返回到根目录

  /*  读取根目录的所有ENTRY，打印出来到屏幕  */

  a = 0;//枚举模式初始设定
  while(folder_enumeration(buf,a,&ATTR) == SUCC)
  { 
	  //打印读取的Directory ENTRY及其attribute
     printf("\nreaded entry=%s Attr = %x",buf,ATTR);

     if(a == 0)
	   a = 1;//枚举模式修改为枚举继续
  }

  scanf("%c",&a);	 

  /* 建立一个OK目录,之后向其写文件readed.rar，直到虚拟盘被写满 */ 
  create_floder("OK");

  j = 0;
  COUNT = 0;
  do{
  for(a = 0;a<25;a++)
  {
  COUNT++;
  //调用create_file()往磁盘上建立文件 
  if(create_file(ddd) == SUCC)
  {
   printf("Create file %s successfully\n",ddd);
   
   HANDLE1 = open_file(ddd);//打开刚刚完成建立的文件

   //打开readed.rar,作为写入源文件
   if ((file2 = fopen("readed.rar","rb+")) == NULL)
     {
      return;
     }
   if(HANDLE1 != FAIL)
	{   
		//将readed.rar拷入刚刚建立的文件   
		printf("\nOpen File %s successfully",ddd);
		do{
            printf("x");
			//从readed.rar里读数据
            cc = fread(buf,1,40000,file2);  
	        //读数据后拷入刚刚建立的文件 
			write_file(HANDLE1,buf, cc);
		    if(cc < 40000)
			    break;
		}while(1);
		//拷贝整个文件到结束,之后关闭文件
		close_file(HANDLE1);   
		fclose(file2);
	}

  }
  else
   printf("Create file %s failed\n",ddd);
   ddd[6+j] ++; 
  }
  j++;
 }while(j< 40);



  /*把虚拟磁盘上的所有的文件与目录分离到ＷＩＮＤＯＷＳ下*/
 
  a = 0;//枚举模式初始设定
  while(disk_enumeration(buf,a,&ATTR) == SUCC)
  { 
     printf("\nreaded entry=%s Attr = %x",buf,ATTR);
     
     if(a == 0)
	   a = 1;	//枚举模式修改为枚举继续

	 if(ATTR & ATTR_DIRECTORY)
	 { 
	   mkdir(buf+3);
	   continue;
	 }
	 else
     {   //在windows当前目录下建立一个文件,文件名与当前枚举的文件名一致*/
		 if ((file2 = fopen(buf+3,"wb+")) == NULL)
		 {  
            return;
		 }    
	     else 
		     HANDLE1 = open_file(buf);
	 }//读数据后拷入刚刚建立的文件 
	if(HANDLE1 != FAIL)
		do{	
	    //从当前的文件读取数据
	    cc = read_file(HANDLE1,buf+400,50000);
		printf("\nreaded chars = %ld ",cc);
        //读数据后拷入刚刚建立的文件
		fwrite(buf+400,1,cc,file2);
		if(cc != 50000)
		{ 
			close_file(HANDLE1);
		    break;   
		}   
	
	}while(1);
    else{
	}
    fclose(file2);	 
  }

  scanf("%c",&a);	
}
   


