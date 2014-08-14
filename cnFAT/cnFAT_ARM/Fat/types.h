 /*
+FHDR------------------------------------------------------------------
Copyright (c),
Tony Yang –specialized in usb,fat firmware development
Contact:qq 292942278  e-mail:tony_yang123@sina.com.cn

Abstract:
$Id: fat.c,v 1.14 2007/05/11 03:00:55 design Exp $
-FHDR-------------------------------------------------------------------
*/ 
#define SUCC 0x0
#define FAIL 0Xff

typedef unsigned char  u8;         /* 无符号8位整型变量*/
typedef signed char  s8;           /* 有符号8位整型变量*/
typedef unsigned int u16;          /* 无符号16位整型变量*/
typedef signed int s16;            /* 有符号16位整型变量*/
typedef unsigned long u32;         /* 无符号32位整型变量*/
typedef signed long s32;           /* 有符号32位整型变量*/


/*
+FFTR--------------------------------------------------------------------
$Log: types.h,v $
Revision 1.3  2007/03/06 12:26:56  yangwenbin
函数cd_folder()完成， mode 0/mode1两种模式测试通过

Revision 1.2  2007/02/28 13:39:52  Design
函数open_file打开绝对路径+短文件名测试通过
使用C代码如下：
open_file("C:\\HELLO.TXT")

Revision 1.1.1.1  2007/02/26 14:01:12  Design
volume_inquire函数测试成功

Revision 1.1.1.1  2007/02/24 07:59:45  yangwenbin
增加FAT16文件系统基本代码

-FFTR--------------------------------------------------------------------
*/



