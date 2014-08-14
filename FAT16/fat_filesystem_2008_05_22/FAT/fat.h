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


extern u8 create_file(u8* filename);
extern u8 create_floder(u8* foldername);
extern u8 open_file(u8 filename[]);
extern u8 close_file(u8 FCB);
extern u8 rename_file(u8* oldfilename,u8* newfilename);
extern u8 folder_enumeration(u8*return_string , u8 mode,u8 *ATTR) ; 
extern u8 disk_enumeration(u8 *return_string,u8 mode,u8* ATTR);
extern u8 f_seek(u8 FCBsn, s32 offset, u8 origin);   
extern u16 write_file(u8 FCBsn,u8* buffer, u16 length);
extern u16 read_file(u8 FCBsn,u8* buffer, u16 length);
extern u8 delete_file( u8* filename);
extern u8 delete_folder(u8* foldername);
extern u8 volume_inquiry(u8  partition_id,u32 *volume_capacity, u32 *volume_free_space);
extern u8 find_file(u8 * filename,u8 mode,u8* foldername);
extern u8 cd_folder(u8 * foldername,u8 mode);
extern u8 FAT_filesystem_initialiation();
extern u8 FAT_filesystem_autoformat(u8 disk_,u8 filesystem_type,u32 disk_capacity);
