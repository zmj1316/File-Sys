/*
+FHDR------------------------------------------------------------------
Copyright (c),
Tony Yang ¨C51,AVR,ARM firmware developer  
Contact:qq 292942278  e-mail:tony_yang123@sina.com.cn

Abstract:
$Id: fat.h,v 1.3 2007/03/11 10:23:44 yangwenbin Exp $
-FHDR-------------------------------------------------------------------
*/
extern u8 create_file(u8* filename);
extern u8 create_floder(u8* foldername);
extern u8 open_file(u8 filename[]);
extern u8 close_file(u8 FCB);
extern u8 rename_file(u8* oldfilename,u8* newfilename);
extern u8 folder_enumeration(u8*return_string , u8 mode,u8 *ATTR) ; 
extern u8 disk_enumeration(u8 disk_,u8 *return_string,u8 mode,u8* ATTR);
extern u8 f_seek(u8 FCBsn, s32 offset, u8 origin);   
extern u16 write_file(u8 FCBsn,u8* buffer, u16 length);
extern u16 read_file(u8 FCBsn,u8* buffer, u16 length);
extern u8 delete_file( u8* filename);
extern u8 delete_folder(u8* foldername);
extern u8 volume_inquiry(u8  partition_id,u32 *volume_capacity, u32 *volume_free_space);
extern u8 find_file(u8 * filename,u8 mode,u8* foldername);
extern u8 cd_folder(u8 * foldername,u8 mode);
extern u8 FAT_filesystem_initialiation(void);
extern u8 FAT16_filesystem_autoformat(u8 disk_);
