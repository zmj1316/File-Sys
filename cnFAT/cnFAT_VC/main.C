/*
+FHDR------------------------------------------------------------------
Copyright (c),
Tony Yang –specialized in fat usb firmware development  
Contact:qq 292942278  e-mail:tony_yang123@sina.com.cn
;;;;;;;;;;
Abstract:
$Id: main.C,v 1.12 2007/05/10 11:13:14 design Exp $
-FHDR-------------------------------------------------------------------
*/
#include "stdio.h"
#include "include\types.h"
#include "fat\fat.h"
#include "Flash_Management\Flash_Management.h" 
#include "include\FAT_cfg.h" 

/*
===============================================================================
函数
main();
入口：无
出口：无
===============================================================================
*/ 
static FILE *file2;
static u8 Fl;
void main(void)
{


#if 1

  u32 cc,bb;
  u16 COUNT,i;  
  u8 a,ATTR,j,b,mode;
  u8 HANDLE1,HANDLE2;   
  u8 buf[65536];
  u8 ddd[] = "c:\\ok\\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa我们aaaaaaaaaaaAAAAAAAAAAAaaaaaaaaa.rar";
  flash_management_sysinit();

  FAT_filesystem_autoformat('C',FAT32,270336*2*2);
  FAT_filesystem_autoformat('D',FAT16,270336);
  FAT_filesystem_initialization();
  volume_inquiry('c',&cc,&bb);
 
  printf("Volume C Capacity: %ld\n",cc);
  printf("Volume C FreeSpace: %ld\n",bb);
  volume_inquiry('d',&cc,&bb);
 
  printf("Volume D Capacity: %ld\n",cc);
  printf("Volume D FreeSpace: %ld\n",bb);
  scanf("%c",&a);  


/*  建立一个目录CREATE_FOLDER_TEST，以测试建立目录函数create_floder()  */
  create_folder("c:\\测试中文目录");
  create_file("测试中文目录\\测试中文文件名.txt");
  create_folder("aaaaaaaaaaA");
  create_folder("aaaaaaaaaaA");
  create_folder("C:\\AAAAAAAAAAAaaaaaa");
  create_folder("D:\\AAAAAAAAAAAaaaaaa");
  create_folder("c:\\CREATE_FOLDER_TEST");
  create_folder("D:\\CREATE_FOLDER_TEST");

  scanf("%c",&a);

/*  在目录CREATE_FOLDER_TEST下建立一个文件，以测试建立目录函数create_file()  */
 create_file("C:\\CREATE_FOLDER_TEST\\created_file.txt");
 create_file("D:\\CREATE_FOLDER_TEST\\created_file.txt");
 create_file("C:\\CREATE_FOLDER_TEST\\AAAAAAAAAAB");
 create_file("D:\\CREATE_FOLDER_TEST\\AAAAAAAAAABaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
 create_file("D:\\CREATE_FOLDER_TEST\\aa.rarc.rar.rar");
/* 重命令文件CREATE_FOLDER_TEST\\created_file.txt为"DFDFDFDFDFDFDFDFSDFSDTONY.TXT"
   ，以测试重命名文件函数rename_file() */

 //rename_file("ok\\bAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA.RAR","DFDFDFDFDFDFDFDFSDFSDTONY.TXT");
 //scanf("%c",&a);	
/* 删除文件CREATE_FOLDER_TEST\\created_file.txt */
  //delete_file("ok\\ZZZZZZZZZZZZZZZUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA.RAR");

/* 删除目录CREATE_FOLDER_TEST */
  //delete_folder("CREATE_FOLDER_TEST");

/*  列举CREATE_FOLDER_TEST目录下面的所有文件，打印到屏幕  */
  cd_folder("c:\\CREATE_FOLDER_TEST",0);

   a = 0;
  while(folder_enumeration(buf,a,&ATTR) == SUCC)
  { 
     printf("\nreaded entry=%s Attr = %x",buf,ATTR);

     if(a == 0)
	   a = 1;
  }
scanf("%c",&a);	

 cd_folder(" ",1);

/*  列举根目录的文件，打印去屏幕  */
 
   a = 0;
  while(folder_enumeration(buf,a,&ATTR) == SUCC)
  { 
     printf("\nreaded entry=%s Attr = %x",buf,ATTR);

     if(a == 0)
	   a = 1;
  }
scanf("%c",&a);	

#if 1
/* 在OK目录下连续写文件，readed.rar，直到磁盘被写满 */ 
   create_folder("c:\\ok");
 j = 0;
 COUNT = 0;
do{
 for(a = 0;a < 25; a++)
 { 
  COUNT++;	 
 if(create_file(ddd) == SUCC)
 { 
   printf("Create file %s successfully\n",ddd);
  
   HANDLE1 = open_file(ddd);
   if((file2 = fopen("readed.rar","rb+")) == NULL)
     {
      return;
     }
	if(HANDLE1 != FAIL)
	{   
		   
		printf("\nOpen File %s successfully",ddd);
		do{
	
        cc = fread(buf,1,40000,file2);
		//printf("\nreaded chars = %ld ",cc); 
	    write_file(HANDLE1,buf, cc);
		if(cc < 40000)
			break;
		}while(1);
		close_file(HANDLE1);
		fclose(file2);
	}
	else
       printf("\nOpen File %s failed",ddd);

 }
 else
   printf("Create file %s failed\n",ddd);
  ddd[6+j] ++; 
 }
  j++;
}while(j< 20);

scanf("%c",&a);	

/*  分离磁盘0中的所有文件  */
  a = 0;
  while(disk_enumeration(0,buf,a,&ATTR) == SUCC)
  { 
     printf("\nreaded entry=%s Attr = %x",buf,ATTR);
	 
     if(a == 0)
	   a = 1;


	 if(ATTR & ATTR_DIRECTORY)
	 { 
	   mkdir(buf+3);
	   continue;
	 }
	 else
     {
		 if ((file2 = fopen(buf+3,"wb+")) == NULL)
		 {  
           return;

		 }    
	     else 
		   HANDLE1 = open_file(buf);
	 }
	 
	if(HANDLE1 != FAIL)
		do{	     
	    cc = read_file(HANDLE1,buf+400,50000);
		printf("\nreaded chars = %ld ",cc);
        fwrite(buf+400,1,cc,file2);
		if(cc != 50000)
		{ 
			close_file(HANDLE1);

			
		break;   
		}   
	
	}while(1);
    else{
	printf("Openfile failed!");
	}
    fclose(file2);	 
  }


scanf("%c",&a);	
#endif 

/* DISK0  拷贝至DISK1 */
  mode = 0;      //设置disk_enumeration列举mode(0)-复位至第一个文件项或目录项开始枚举
  while(disk_enumeration(0,buf,mode,&ATTR) == SUCC)          //枚举一个目录项或目录项
  { 
     printf("\nreaded entry=%s Attr = %x",buf,ATTR); //打印被枚举的一个目录项或目录项
	
     if(mode == 0) 
	   mode = 1;    //设置disk_enumeration列举mode(1)- 继续上一枚举后的目录项或目录项
	 if(ATTR & ATTR_DIRECTORY)
	 { 
	   buf[0]++;
	   create_folder(buf);                              //建一个disk_enumeration的目录
	   continue;
	 }
	 else                                                                   //复制文件
     { HANDLE1 = open_file(buf);                                            //打开文件
	   buf[0]++;
	   create_file(buf);                                  //在DISK1上建立文件
	       
	 }
	if(HANDLE1 != FAIL)
	{ HANDLE2 = open_file(buf);                                //打开DISK1上建立的文件
	  if(HANDLE2 == FAIL)
		  break;
	do{                                                                     //复制文件
	    cc = read_file(HANDLE1,buf+400,50000);                                //读文件
		printf("\nreaded chars = %ld ",cc);                   //打印读文件读取的字节数
        cc = write_file(HANDLE2,buf+400,cc);                //将读取的字节写去DISK1上的文件
		if(cc != 50000)                                 //检查读取的字节数，确认文件尾
		{ 
		 close_file(HANDLE1);                                 //文件复制结束，关闭文件
		 close_file(HANDLE2);
	 	 break;   
		} 
	  }while(1);
	}
    else
	{
	  printf("Openfile failed!");
	}
  }



scanf("%c",&a);		



#else
	u8 ret,buf[1024];
	u32 i;
    struct attribute ATTRIBUTE;

	ret = flash_management_sysinit();

	ret = FAT_filesystem_autoformat('C',FAT32,270336*2*2);
	FAT_filesystem_autoformat('D',FAT16,270336);

	ret = FAT_filesystem_initialization();


	ret = create_folder("C:\\newFolder_0");
	if (ret == 0)
	{
		printf("newFolder_0 创建成功！\n");
	}
	
	ret = create_file("C:\\newFolder_0\\newFile_1");
	if (ret == 0)
	{
		printf("newFolder_0\\newFile_1 创建成功！\n");
	}
	ret = create_file("C:\\newFolder_0\\newFile_2");
	if (ret == 0)
	{
		printf("newFolder_0\\newFile_2 创建成功！\n");
	}
		
	/*ret = delete_file("C:\\newFolder_0\\newFile_1");
	if (ret == 0)
	{
		printf("newFolder_0\\newFile_1 删除成功！\n");
	}
	ret = delete_file("C:\\newFolder_0\\newFile_2");
	if (ret == 0)
	{
		printf("newFolder_0\\newFile_2 删除成功！\n");
	}
	cd_folder("C:\\newFolder_0",1);	

	ret = delete_folder("C:\\newFolder_0");//成功
	if (ret == 0)
	{
		printf("newFolder_0 删除成功！\n\n");
	}
	else
		printf("newFolder_0 删除失败！\n\n");
*/

  //获取文件的访问,建立,修改时间	
  create_file("test.txt");
  if(get_file_attribute("test.txt", The_FILE, &ATTRIBUTE)== SUCC)                                                                                                                                
  {
	printf("\n文件:test.txt\naccess time: %d-%d-%d",ATTRIBUTE.access_time_year,ATTRIBUTE.access_time_month,ATTRIBUTE.access_time_day);
    printf("\ncreate time: %d-%d-%d",ATTRIBUTE.create_time_year,ATTRIBUTE.create_time_month,ATTRIBUTE.create_time_day);
    printf("\ncreate time: %d:%d:%d",ATTRIBUTE.create_time_hour,ATTRIBUTE.create_time_minute,ATTRIBUTE.create_time_second);
    printf("\nmodify time: %d-%d-%d",ATTRIBUTE.modify_time_year,ATTRIBUTE.modify_time_month,ATTRIBUTE.modify_time_day);
    printf("\nmodify time: %d:%d:%d",ATTRIBUTE.modify_time_hour,ATTRIBUTE.modify_time_minute,ATTRIBUTE.modify_time_second);
    printf("\nattr=%x",ATTRIBUTE.attr);    
  }
  else
  {
    printf("FAIL");
  }

   //获取目录的访问,建立,修改时间
  create_folder("test");
  if(get_file_attribute("test", The_DIRECTORY, &ATTRIBUTE)== SUCC)                                                                                                                                
  {
	printf("\n目录:test\naccess time: %d-%d-%d",ATTRIBUTE.access_time_year,ATTRIBUTE.access_time_month,ATTRIBUTE.access_time_day);
    printf("\ncreate time: %d-%d-%d",ATTRIBUTE.create_time_year,ATTRIBUTE.create_time_month,ATTRIBUTE.create_time_day);
    printf("\ncreate time: %d:%d:%d",ATTRIBUTE.create_time_hour,ATTRIBUTE.create_time_minute,ATTRIBUTE.create_time_second);
    printf("\nmodify time: %d-%d-%d",ATTRIBUTE.modify_time_year,ATTRIBUTE.modify_time_month,ATTRIBUTE.modify_time_day);
    printf("\nmodify time: %d:%d:%d",ATTRIBUTE.modify_time_hour,ATTRIBUTE.modify_time_minute,ATTRIBUTE.modify_time_second);
    printf("\nattr=%x",ATTRIBUTE.attr);    
  }
  else
  {
    printf("FAIL");
  }

	scanf("%c",&ret);
#endif
}
   
/*
+FFTR--------------------------------------------------------------------
$Log: main.C,v $

-FFTR--------------------------------------------------------------------
*/

