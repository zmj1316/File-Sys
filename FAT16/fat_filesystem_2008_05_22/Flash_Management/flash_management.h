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

/*===============================================================================
函数
读取磁盘上的一个扇区
入口：u8 *buf:缓冲区的首地址,u32 sector:物理的扇区号
出口：SUCC
===============================================================================*/
extern u8 read_flash_sector(u8 *buf,u32 sector);

/*===============================================================================
函数
磁盘上的一个扇区写入
入口：u8 *buf:缓冲区的首地址,u32 sector:物理的扇区号
出口：SUCC
===============================================================================*/
extern u8 write_flash_sector(u8 *buf,u32 sector);

/*===============================================================================
函数
初始化模拟磁盘的驱动
入口：无
出口：无
===============================================================================*/
extern u8 flash_management_sysinit();
