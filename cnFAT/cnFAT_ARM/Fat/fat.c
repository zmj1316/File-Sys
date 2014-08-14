
 /*
+FHDR------------------------------------------------------------------
Copyright (c),
Tony Yang –specialized in usb,fat firmware development
Contact:qq 292942278  e-mail:tony_yang123@sina.com.cn

Abstract:
$Id: fat.c,v 1.14 2007/05/11 03:00:55 design Exp $
-FHDR-------------------------------------------------------------------
*/      

#include"types.h"

#include"FAT_cfg.h"
#include"time.h"
#include"gb2312_to_unicode_table.h"

#include "..\config.h"
OS_EVENT *fat_filesystem_Mutex;
u8 err;

//互锁宏定义
#define enter_Mutex	  OSSemPend(fat_filesystem_Mutex,0,&err)
#define release_Mutex OSSemPost(fat_filesystem_Mutex)
//Current Directory Entry 
//static struct Directory_Entry_ Directory_Entry;
//CORE of FAT filesystem
static struct core_ CORE[maximum_disks];
//BPB
static struct partition_bpb BPB[maximum_disks]; 
//Define FCBs for FileRead/Write  
struct FileControlBlock FCB[MaximumFCB];
u8 disk_id;
extern USB_INT8U rbcWrite10 (USB_INT16U usInterface,
                      USB_INT8U  ucLunIndex,
                      USB_INT32U uiLba,                                 /*  LBA,即逻辑块地址            */
                      USB_INT32U uiTranLen,                             /*  要读取的数据长度            */
                      USB_INT8U *pucData);                               /*  接收数据缓冲区              */
extern USB_INT8U rbcRead10 (USB_INT16U usInterface,
                     USB_INT8U  ucLunIndex,
                     USB_INT32U uiLba,                                  /*  LBA,即逻辑块地址            */
                     USB_INT32U uiTranLen,                              /*  要读取的数据长度(块数)      */
                     USB_INT8U *pucData);                                /*  接收数据缓冲区              */

//driver transition function
u8 read_flash_sector(u8 *buf,u32 sector,u8 disk_)
{
    rbcRead10(0,0,sector,1,buf);
	return(SUCC);
}


//driver transition function
u8 write_flash_sector(u8 *buf,u32 sector,u8 disk_)
{
	rbcWrite10(0,0,sector,1,buf);
	return(SUCC);

}
/*
===============================================================================
函数
字符串转换为大写
入口：*string:字符串首地址
出口：SUCC
===============================================================================
*/ 
static u8 UPCASE(u8* string) 
{ 
 while(*string) 
 { 
     if(*string >='a' && *string <= 'z')
      {
  	   *string -= 32;
 	   
      }
	 string++;
   }    
 return(SUCC);
}
/*
===============================================================================
函数
测试字符串长度
入口：*string:字符串首地址
出口：字符串长度
===============================================================================
*/ 
static u16 LengthofString(u8 * string)
{ 
 u16 i;
 i = 0;
 while(*string)
  {
    i++;
    string++;
  }
 return(i);
} 
/*
===============================================================================
函数
连接字符串2到字符串1之后,连接后字符串2无变化
入口：*string1:字符串1首地址,*string2:字符串2首地址
出口：SUCC,FAIL
===============================================================================
*/ 
static u8 concanenateString(u8 *string1,u8 *string2)
{
  u8 len,i;
  len = LengthofString(string1);
  i = 0;
  while(string2[i])
  {
    string1[len] = string2[i];  
    len++;
	i++;
  }
  string1[len] = 0;
  return(SUCC);
}

/*
===============================================================================
函数
字符串copy
入口：*string1:源字符串首地址；*string2:目标字符串首地址
出口：SUCC
===============================================================================
*/ 
static u8 stringcpy(u8 *string1,u8 *string2)
{
 while(*string1) 
   { 
     *string2 = *string1;
     string1++;
     string2++;
   }  
 *string2 = 0;
 return(SUCC);
}
/*
===============================================================================
函数
字符串比较(不区分大小写)
入口：*string1:字符串1首地址；*string2:字符串2首地址
出口：SUCC,FAIL
===============================================================================
*/ 
static u8 stringcmp(u8 *string1,u8 *string2)
{
 UPCASE(string1);
 UPCASE(string2);  
 while((*string1) && (*string2)) 
   {
     if((*string1) == (*string2))
      {
         string1++;
         string2++;
      }
     else
       return(FAIL);
   }   
   
 if( ((*string1) == 0) && ((*string2) == 0))
 {
     return(SUCC);
 }
 else
     return(FAIL);
}

/*
===============================================================================
函数
据FCBsn写time stamp
入口： FCBsn:写的FCB的序号, flag:ACCESS_TIME,MODIFY_TIME,CREATE_TIME
出口：SUCC,FAIL
===============================================================================
*/ 
#if enable_time_stamp_transaction
u8 write_time_stamp(u8 FCBsn,u8 flag)
{
u8 buf[512];
u16 date_time[2];
u16 offset;
if(FCB[FCBsn].file_openned_flag == UsedFlag)
  return(FAIL);
if(flag == 0)
  return(SUCC);
get_time(date_time);
read_flash_sector(buf,FCB[FCBsn].Entry_Storedin_Sector,FCB[FCBsn].disk);
offset = FCB[FCBsn].Entry_Storedin_Sector_Offset;
if(flag & MODIFY_TIME)
{
  buf[offset + 22] = date_time[1] & 0xff;
  buf[offset + 23] = (date_time[1] >> 8) & 0xff;
  buf[offset + 24] = date_time[0] & 0xff;
  buf[offset + 25] = (date_time[0] >> 8) & 0xff;
}

if(flag & ACCESS_TIME)
{
  buf[offset + 18] = date_time[0] & 0xff;
  buf[offset + 19] = (date_time[0] >> 8) & 0xff;
}
if(flag & CREATE_TIME)
{
  buf[offset + 13] = 0;
  buf[offset + 14] = date_time[1] & 0xff;
  buf[offset + 15] = (date_time[1] >> 8) & 0xff;
  buf[offset + 16] = date_time[0] & 0xff;
  buf[offset + 17] = (date_time[0] >> 8) & 0xff;
}
write_flash_sector(buf,FCB[FCBsn].Entry_Storedin_Sector,FCB[FCBsn].disk);
 return(SUCC);
}
#endif
/*
===============================================================================
函数
据Entry_Storedin_Sector,offset写time stamp
入口： Entry_Storedin_Sector:ENTRY所在SECTOR，
       offset：ENTRY所在offset，
	   flag:ACCESS_TIME,MODIFY_TIME,CREATE_TIME
出口：SUCC,FAIL
===============================================================================
*/ 
#if enable_time_stamp_transaction
u8 write_time_stamp_direct(u32 Entry_Storedin_Sector,u16 offset,u8 flag)
{
u8 buf[512];
u16 date_time[2];
if(flag == 0)
  return(SUCC);
get_time(date_time);
read_flash_sector(buf,Entry_Storedin_Sector,disk_id);
if(flag & MODIFY_TIME)
{
  buf[offset + 22] = date_time[1] & 0xff;
  buf[offset + 23] = (date_time[1] >> 8) & 0xff;
  buf[offset + 24] = date_time[0] & 0xff;
  buf[offset + 25] = (date_time[0] >> 8) & 0xff;
}

if(flag & ACCESS_TIME)
{
  buf[offset + 18] = date_time[0] & 0xff;
  buf[offset + 19] = (date_time[0] >> 8) & 0xff;
}

if(flag & CREATE_TIME)
{
  buf[offset + 13] = 0;
  buf[offset + 14] = date_time[1] & 0xff;
  buf[offset + 15] = (date_time[1] >> 8) & 0xff;
  buf[offset + 16] = date_time[0] & 0xff;
  buf[offset + 17] = (date_time[0] >> 8) & 0xff;
}
write_flash_sector(buf,Entry_Storedin_Sector,disk_id);
  return(SUCC);
}
#endif

/*
===============================================================================
函数
在簇链里找当前簇的下一簇
入口：Cluster:当前簇号
出口: 返回下一簇号
===============================================================================
*/ 
static u32 Get_Next_Cluster_From_Current_Cluster(u32 Cluster)
{
   u8 buf[512]; 
   u32 ThisFATSecNum,ThisFATEntOffset;
   //FAT16
   if(CORE[disk_id].fs_type == FAT16)
     ThisFATEntOffset = Cluster * 2;
   else
	 ThisFATEntOffset = Cluster * 4;
   ThisFATSecNum = CORE[disk_id].relative_sector + BPB[disk_id].reserved_sector 
                   + (ThisFATEntOffset / BPB[disk_id].bytes_per_sector);
   ThisFATEntOffset = ThisFATEntOffset % BPB[disk_id].bytes_per_sector; 
   read_flash_sector(buf,ThisFATSecNum,disk_id);
   if(CORE[disk_id].fs_type == FAT16)
     return((u32)buf[ThisFATEntOffset] + ((u32)buf[ThisFATEntOffset + 1]) * 256);
   else
     return((u32)buf[ThisFATEntOffset] + ((u32)buf[ThisFATEntOffset + 1]) * 256
	 +(u32)buf[ThisFATEntOffset+2]*256*256 + ((u32)buf[ThisFATEntOffset + 3]) * 256*256*256);
}
/*
===============================================================================
函数
在簇链里找当前簇之前的簇号
入口：Cluster:当前簇号
出口: 返回当前簇之前的簇号
===============================================================================
*/ 
static u32 Get_Previous_Cluster_From_Current_Cluster(u32 Cluster)
{
  u8 buf[512];
  u32 CUR_FATSecNum,CUR_FATEntOffset;
  u32 CUR_Checking_Cluster_No;
  CUR_Checking_Cluster_No = 0;
  CUR_FATEntOffset = 0;
  CUR_FATSecNum = CORE[disk_id].FirstSectorofFAT1; 
  read_flash_sector(buf,CUR_FATSecNum,disk_id); 
  do{ 
	 if( CUR_Checking_Cluster_No == (CORE[disk_id].CountofClusters + 2))
         {
          return(FAIL);
         }
	 if(CORE[disk_id].fs_type == FAT16)
       if( (u16)(buf[CUR_FATEntOffset] + buf[CUR_FATEntOffset + 1] * 256) == (u16)Cluster)
          return(CUR_Checking_Cluster_No);
     else
       if((u32) (buf[CUR_FATEntOffset] + buf[CUR_FATEntOffset + 1] * 256
		   + buf[CUR_FATEntOffset + 2] * 256*256 + + buf[CUR_FATEntOffset + 3] * 256 * 256 *256
		   ) == Cluster)
          return(CUR_Checking_Cluster_No);	   
	   CUR_Checking_Cluster_No++;
     
     if(CORE[disk_id].fs_type == FAT32) 
		 CUR_FATEntOffset += 4;
	 else
         CUR_FATEntOffset += 2;
     if(CUR_FATEntOffset >= 512)
      {
        CUR_FATSecNum++;

        read_flash_sector(buf,CUR_FATSecNum,disk_id); 
        CUR_FATEntOffset = 0;     
      }
  }while(1);
}
/*
===============================================================================
函数
删除整个簇链，返回文件系统分配使用
入口：Cluster:首簇号
出口: SUCC,FAIL
===============================================================================
*/ 
static u8 FreeClusterChain(u32 Cluster)
{
 u8 buf[512],i,flag; 
 u32 ThisFAT1SecNum,ThisFATEntOffset,ThisFAT2SecNum,sector_in_buf; 
 sector_in_buf = 0;
 flag = 0;
 do{
   if(Cluster < CORE[disk_id].min_cluster_for_allocat)
	   CORE[disk_id].min_cluster_for_allocat = Cluster;
   if(CORE[disk_id].fs_type == FAT16) 
      ThisFATEntOffset = Cluster * 2;
   else
      ThisFATEntOffset = Cluster * 4;
   ThisFAT1SecNum = CORE[disk_id].relative_sector + BPB[disk_id].reserved_sector 
                   + (ThisFATEntOffset / BPB[disk_id].bytes_per_sector);
   ThisFAT2SecNum = ThisFAT1SecNum + BPB[disk_id].sectors_per_FAT;
   ThisFATEntOffset = (ThisFATEntOffset) % BPB[disk_id].bytes_per_sector; 
   if(ThisFAT1SecNum != sector_in_buf)
   {
	  if(flag ==1)
	  {
	    write_flash_sector(buf,sector_in_buf,disk_id); 
		sector_in_buf += BPB[disk_id].sectors_per_FAT;
         for(i = 1;i < BPB[disk_id].numbers_of_FAT;i++) 
		 {
             write_flash_sector(buf,sector_in_buf,disk_id);
             sector_in_buf += BPB[disk_id].sectors_per_FAT;
		 }
	  
	  }
      sector_in_buf = ThisFAT1SecNum;
      read_flash_sector(buf,ThisFAT1SecNum,disk_id);
	  flag = 1;
   }
   
   
   if(CORE[disk_id].fs_type == FAT16) 
   {
    Cluster = (u32)buf[ThisFATEntOffset]+ (u32)buf[ThisFATEntOffset + 1] * 256;  
    buf[ThisFATEntOffset] = 0x0;       //free Cluster
    buf[ThisFATEntOffset + 1] = 0x0;
   }
   else
   {
    Cluster = (u32)buf[ThisFATEntOffset]+ (u32)buf[ThisFATEntOffset + 1] * 256
		      + (u32)buf[ThisFATEntOffset + 2] * 256*256 + (u32)buf[ThisFATEntOffset + 3] * 256*256*256;  
    buf[ThisFATEntOffset] = 0x0;       //free Cluster
    buf[ThisFATEntOffset + 1] = 0x0;   
    buf[ThisFATEntOffset + 2] = 0x0;       //free Cluster
    buf[ThisFATEntOffset + 3] = 0x0;     
   }
   if(CORE[disk_id].fs_type == FAT16)    
   {
	 if(Cluster >= 0xfff6)
	 {	 write_flash_sector(buf,ThisFAT1SecNum,disk_id); 
         for(i = 1;i < BPB[disk_id].numbers_of_FAT;i++) 
		 {
             write_flash_sector(buf,ThisFAT2SecNum,disk_id);
             ThisFAT2SecNum = ThisFAT2SecNum + BPB[disk_id].sectors_per_FAT;
		 }
		 return(SUCC);
	 
	 }   
   }
   else
   {
 	 if(Cluster >= 0xfffffff)
     {   write_flash_sector(buf,ThisFAT1SecNum,disk_id); 
         for(i = 1;i < BPB[disk_id].numbers_of_FAT;i++) 
		 {
             write_flash_sector(buf,ThisFAT2SecNum,disk_id);
             ThisFAT2SecNum = ThisFAT2SecNum + BPB[disk_id].sectors_per_FAT;
		 }
		 return(SUCC);
	 
	 }   
   }

  }while(1);  
}
/*
===============================================================================
函数
Given any valid data cluster number N,
Return the sector number of the first sector of that cluster
入口：u32 N:data cluster number N
出口: RETURN first sector of that cluster
===============================================================================
*/  
static u32 FirstSectorofCluster(u32 N)
{
 return((N - 2) * BPB[disk_id].sector_per_cluster + CORE[disk_id].FirstDataSector);
}
/*
===============================================================================
函数
从FAT中分配一个空簇加到簇链当前簇之后
入口：Cluster:当前簇号
出口: 加入的簇号
===============================================================================
*/ 
u8 Allocate_EMPTY_CLUSTER_TO_CUR_CLUSTER_CHAIN(u32 Cluster,u32 *Added_Cluster,u8 *buf,u8 CLS)
{
  u32 temp,EMPTY_CLUSTER,ThisFAT1SecNum,ThisFATEntOffset,ThisFAT2SecNum;
  u16 i,fat_counter; 
  EMPTY_CLUSTER = CORE[disk_id].min_cluster_for_allocat;
  ThisFAT1SecNum = CORE[disk_id].relative_sector + BPB[disk_id].reserved_sector;
  if(CORE[disk_id].fs_type == FAT16)   
  {
	  ThisFAT1SecNum += (CORE[disk_id].min_cluster_for_allocat * 2) / 512;
      ThisFAT2SecNum = ThisFAT1SecNum + BPB[disk_id].sectors_per_FAT;
	  i = (CORE[disk_id].min_cluster_for_allocat) % 256; 
	  ThisFATEntOffset = (CORE[disk_id].min_cluster_for_allocat * 2) % 512;
	  fat_counter = 256;
  }
  else
  {	  ThisFAT1SecNum += (CORE[disk_id].min_cluster_for_allocat * 4) / 512;
      ThisFAT2SecNum = ThisFAT1SecNum + BPB[disk_id].sectors_per_FAT;
	  i = (CORE[disk_id].min_cluster_for_allocat) % 128; 
	  ThisFATEntOffset = (CORE[disk_id].min_cluster_for_allocat * 4) % 512;
	  fat_counter = 128;
  }
  do{      
  
  read_flash_sector(buf,ThisFAT1SecNum,disk_id);   
  for(;i < fat_counter ;i++)
   {
     if((buf[ThisFATEntOffset] == 0 && buf[ThisFATEntOffset + 1] == 0 && CORE[disk_id].fs_type == FAT16)
      || (buf[ThisFATEntOffset] == 0 && buf[ThisFATEntOffset + 1] == 0 
	  && buf[ThisFATEntOffset + 2] == 0 && buf[ThisFATEntOffset + 3] == 0 
	  && CORE[disk_id].fs_type == FAT32))
      {
	   if( EMPTY_CLUSTER == (CORE[disk_id].CountofClusters + 2))
         {
          return(FAIL);
         }  
       temp = Get_Next_Cluster_From_Current_Cluster(Cluster);
       buf[ThisFATEntOffset] = (u8)(temp & 0xff);   //SET to Allocated Cluster
       buf[ThisFATEntOffset + 1] = (u8)((temp >> 8) & 0xff);
	   if(CORE[disk_id].fs_type == FAT32) 
	   {
	     buf[ThisFATEntOffset + 2] = (u8)((temp >> 16) & 0xff);   //SET to Allocated Cluster
         buf[ThisFATEntOffset + 3] = (u8)((temp >> 24) & 0xff);
	   }   
       write_flash_sector(buf,ThisFAT1SecNum,disk_id);  
       temp = ThisFAT2SecNum;
       for(i = 1;i < BPB[disk_id].numbers_of_FAT;i++)  //update backup FAT2,etc..
         {
          write_flash_sector(buf,temp,disk_id);
          temp +=  BPB[disk_id].sectors_per_FAT;
         }  
       temp = Get_Next_Cluster_From_Current_Cluster(Cluster);
	   if(CORE[disk_id].fs_type == FAT16) 
         ThisFATEntOffset = Cluster * 2;
       else
         ThisFATEntOffset = Cluster * 4;
	   ThisFAT1SecNum = CORE[disk_id].relative_sector + BPB[disk_id].reserved_sector 
                   + (ThisFATEntOffset / BPB[disk_id].bytes_per_sector);
       ThisFATEntOffset = ThisFATEntOffset % BPB[disk_id].bytes_per_sector;
       ThisFAT2SecNum = ThisFAT1SecNum + BPB[disk_id].sectors_per_FAT; 
       read_flash_sector(buf,ThisFAT1SecNum,disk_id);
       buf[ThisFATEntOffset] = (u8)(EMPTY_CLUSTER & 0xff);//Add to cluster chain
       buf[ThisFATEntOffset + 1] = (u8)((EMPTY_CLUSTER >> 8) & 0xff); 
	   if(CORE[disk_id].fs_type == FAT32)
	   {
	      buf[ThisFATEntOffset + 2] = (u8)((EMPTY_CLUSTER >> 16) & 0xff); 
		  buf[ThisFATEntOffset + 3] = (u8)((EMPTY_CLUSTER >> 24) & 0xff); 
	   }
	   CORE[disk_id].min_cluster_for_allocat = EMPTY_CLUSTER + 1;
       write_flash_sector(buf,ThisFAT1SecNum,disk_id);  
       temp = ThisFAT2SecNum;
       for(i = 1;i < BPB[disk_id].numbers_of_FAT;i++)  //update backup FAT2,etc..
         {          
		   read_flash_sector(buf,temp,disk_id);
           buf[ThisFATEntOffset] = (u8)(EMPTY_CLUSTER & 0xff);//Add to cluster chain
           buf[ThisFATEntOffset + 1] = (u8)((EMPTY_CLUSTER >> 8) & 0xff); 
	       if(CORE[disk_id].fs_type == FAT32)
		   {
	          buf[ThisFATEntOffset + 2] = (u8)((EMPTY_CLUSTER >> 16) & 0xff); 
		      buf[ThisFATEntOffset + 3] = (u8)((EMPTY_CLUSTER >> 24) & 0xff); 
		   }
           write_flash_sector(buf,temp,disk_id);
           temp +=  BPB[disk_id].sectors_per_FAT;
         }
	    //将簇中数据清空
	   if(CLS)
	   {
		 for(i = 0;i< 512;i++)
		   buf[i] = 0;
         ThisFAT1SecNum = FirstSectorofCluster(EMPTY_CLUSTER);
	     for(i = 0;i < BPB[disk_id].sector_per_cluster;i++)
           write_flash_sector(buf,ThisFAT1SecNum + i,disk_id);
       }
	   *Added_Cluster = EMPTY_CLUSTER;
       return(SUCC);
      }
     else
	 {  if(CORE[disk_id].fs_type == FAT16)
          ThisFATEntOffset += 2;
	    else
          ThisFATEntOffset += 4;
        EMPTY_CLUSTER ++;  
        if( EMPTY_CLUSTER == (CORE[disk_id].CountofClusters + 2))
         {
          return(FAIL);
         } 
      }
   }
  ThisFAT1SecNum ++;
  ThisFAT2SecNum ++;
  ThisFATEntOffset = 0; 
  i = 0;
  }while(1);     
}

/*
===============================================================================
函数
从FAT中分配一个空簇
入口：cluster--成功分配的簇号
出口: SUCC,FAIL
===============================================================================
*/ 
u8 Allocate_An_Empty_cluster(u32 * cluster,u8 * buf,u8 cls)
{
  u32 temp,EMPTY_CLUSTER,ThisFAT1SecNum,ThisFATEntOffset,ThisFAT2SecNum;
  u16 i,fat_counter; 
  EMPTY_CLUSTER = CORE[disk_id].min_cluster_for_allocat;
  ThisFAT1SecNum = CORE[disk_id].relative_sector + BPB[disk_id].reserved_sector;
  ThisFAT2SecNum = ThisFAT1SecNum + BPB[disk_id].sectors_per_FAT; 
  if(CORE[disk_id].fs_type == FAT16)   
  {
	  ThisFAT1SecNum += (CORE[disk_id].min_cluster_for_allocat * 2) / 512;
      ThisFAT2SecNum = ThisFAT1SecNum + BPB[disk_id].sectors_per_FAT;
	  i = (CORE[disk_id].min_cluster_for_allocat) % 256; 
	  ThisFATEntOffset = (CORE[disk_id].min_cluster_for_allocat * 2) % 512;
	  fat_counter = 256;
  }
  else
  {	  ThisFAT1SecNum += (CORE[disk_id].min_cluster_for_allocat * 4) / 512;
      ThisFAT2SecNum = ThisFAT1SecNum + BPB[disk_id].sectors_per_FAT;
	  i = (CORE[disk_id].min_cluster_for_allocat) % 128; 
	  ThisFATEntOffset = (CORE[disk_id].min_cluster_for_allocat * 4) % 512;
	  fat_counter = 128;
  }

  do{      
  
  read_flash_sector(buf,ThisFAT1SecNum,disk_id);   
  for(;i < fat_counter;i++)
   {
     if((buf[ThisFATEntOffset] == 0 && buf[ThisFATEntOffset + 1] == 0 && CORE[disk_id].fs_type == FAT16)
      || (buf[ThisFATEntOffset] == 0 && buf[ThisFATEntOffset + 1] == 0 
	  && buf[ThisFATEntOffset + 2] == 0 && buf[ThisFATEntOffset + 3] == 0 
	  && CORE[disk_id].fs_type == FAT32))
      {
	   if( EMPTY_CLUSTER == (CORE[disk_id].CountofClusters + 2))
         {
          return(FAIL);
         }     
       buf[ThisFATEntOffset] = 0xff;   //SET to Allocated Cluster
       buf[ThisFATEntOffset + 1] = 0xff;
       if(CORE[disk_id].fs_type == FAT32)
	   {
	      buf[ThisFATEntOffset + 2] = 0xff;
		  buf[ThisFATEntOffset + 3] = 0xff;
	   }
	   write_flash_sector(buf,ThisFAT1SecNum,disk_id);  
       temp = ThisFAT2SecNum;
       for(i = 1;i < BPB[disk_id].numbers_of_FAT;i++)  //update backup FAT2,etc..
         {
          write_flash_sector(buf,temp,disk_id);
          temp +=  BPB[disk_id].sectors_per_FAT;
         }  
	   //将簇中数据清空
	   if(cls)
	   {
	    for(i = 0;i< 512;i++)
		    buf[i] = 0;
        ThisFAT1SecNum = FirstSectorofCluster(EMPTY_CLUSTER);
	    for(i = 0; i < BPB[disk_id].sector_per_cluster;i++)
            write_flash_sector(buf,ThisFAT1SecNum + i,disk_id);
       }
	   *cluster = EMPTY_CLUSTER; 
	   CORE[disk_id].min_cluster_for_allocat = EMPTY_CLUSTER + 1;
       return(SUCC);
      }
     else
      {
        if(CORE[disk_id].fs_type == FAT16)
          ThisFATEntOffset += 2;
	    else
          ThisFATEntOffset += 4;
        EMPTY_CLUSTER ++;  
        if( EMPTY_CLUSTER == (CORE[disk_id].CountofClusters + 2))
         {
          return(FAIL);
         } 
      }
   }
  ThisFAT1SecNum ++;
  ThisFAT2SecNum ++;
  ThisFATEntOffset = 0;
  i = 0;
  }while(1);     

}

/*
===============================================================================
函数(CORE[disk_id]_INIT_1)
根据boot sector分区表计算得出分区partition_ID的relative_sector,total_sector等
入口：partition_ID(支持4个分区0,1,2,3)
出口: 无
===============================================================================
*/
//static void BPB_INIT_1(u8 partition_ID,u8 *buf)
//{
  //read_flash_sector(buf,0); //read MBR 
//  CORE[disk_id].relative_sector = 0;
//  CORE[disk_id].total_sector =  buf[32]+buf[33]*256+buf[34]*256*256+buf[35]*256*256*256;
//  CORE[disk_id].system_id = buf[0x1c2]; //Partition Type 0C-FAT32,06-FAT16 ect..
//  CORE[disk_id].PartitionID= 'C' + partition_ID; //从C开始到Z结束      
//}  
/*
===============================================================================
函数(CORE[disk_id]_INIT_1)
从root sector BPB[disk_id]计算FirstDataSector,FirstRootDirSecNum,etc..
入口：无
出口: 无
===============================================================================
*/
static void BPB_INIT_2(void)
{  
  CORE[disk_id].RootDirSectors = (BPB[disk_id].boot_entries * 32 + (BPB[disk_id].bytes_per_sector - 1)) / BPB[disk_id].bytes_per_sector;                             
  //  The start of the data region, the first sector of cluster 2                             
  CORE[disk_id].FirstDataSector = CORE[disk_id].relative_sector + BPB[disk_id].reserved_sector + BPB[disk_id].sectors_per_FAT
                               * BPB[disk_id].numbers_of_FAT + CORE[disk_id].RootDirSectors;
  CORE[disk_id].FirstRootDirSecNum = CORE[disk_id].relative_sector + BPB[disk_id].reserved_sector+ BPB[disk_id].sectors_per_FAT
                               * BPB[disk_id].numbers_of_FAT;
  CORE[disk_id].DataSec = CORE[disk_id].total_sector - BPB[disk_id].reserved_sector - CORE[disk_id].RootDirSectors
                 - BPB[disk_id].sectors_per_FAT * BPB[disk_id].numbers_of_FAT;
  CORE[disk_id].CountofClusters = CORE[disk_id].DataSec / BPB[disk_id].sector_per_cluster;
  if(CORE[disk_id].CountofClusters>= 65525)
  {   CORE[disk_id].RootClus = BPB[disk_id].RootClus;
      CORE[disk_id].fs_type = FAT32;
  }
  else
  {  
	  CORE[disk_id].RootClus = 0;
	  CORE[disk_id].fs_type = FAT16;
  }
  CORE[disk_id].FirstSectorofFAT1 = CORE[disk_id].relative_sector + BPB[disk_id].reserved_sector;
  CORE[disk_id].FirstSectorofFAT2 = CORE[disk_id].relative_sector + BPB[disk_id].reserved_sector + BPB[disk_id].sectors_per_FAT;
}
/*
===============================================================================
函数
Read Partition PBP
入口：Partition ID
出口：无
===============================================================================
*/ 
static u8 Read_partition_PBP(u8 partition_ID)
{  
  //u8 *ptr1,*ptr2; 
  //u16 i; 
  u8 buf[512]; 
  //////printf("\ndisk id = %d\n",partition_ID );
   //if ((CORE[partition_ID].PartitionID ) ==  partition_ID) 
   // {  
	  //Specific BPB[partition_ID] is already readed in the buffer
     // return(SUCC);
//    } 
   disk_id = partition_ID;
   read_flash_sector(buf,0,disk_id); //read MBR 
   if ( buf[0x1be] == 0x00 || buf[0x1be] == 0x80) // check boot indicator 00 or 0x80
   {  
   	   CORE[partition_ID].relative_sector =  *((u32*) (buf+454));
	   read_flash_sector(buf,CORE[partition_ID].relative_sector,disk_id); //read MBR 
   }
   else
     {
	   CORE[partition_ID].relative_sector = 0;
	  } 
       CORE[partition_ID].total_sector =  *((u32*) (buf+32));
       CORE[partition_ID].system_id = buf[0x1c2]; //Partition Type 0C-FAT32,06-FAT16 ect..
       CORE[partition_ID].PartitionID= 'C' + partition_ID; //从C开始到Z结束 
	   if ( buf[510] == 0x55 && buf[511] == 0xaa)
       {
CORE[partition_ID].total_sector =  buf[32]+buf[33]*256+buf[34]*256*256+buf[35]*256*256*256;
//////printf("\ntotal_sector =%d ",CORE[partition_ID].total_sector );
  //u16 bytes_per_sector;//每扇区字节数
BPB[partition_ID].bytes_per_sector = buf[0xb] + buf[0xc] * 256;
//////printf("\nbytes_per_sector =%d ",BPB[partition_ID].bytes_per_sector);
//  u8 sector_per_cluster; //每簇扇区数
BPB[partition_ID].sector_per_cluster = buf[0xd];
//////printf("\nsector_per_cluster=%d ",BPB[partition_ID].sector_per_cluster);
//  u16 reserved_sector;  //保留扇区数
BPB[partition_ID].reserved_sector = buf[14] + buf[15] * 256;
//////printf("\nreserved_sector=%d ",BPB[partition_ID].reserved_sector);
//  u8 numbers_of_FAT;//FAT副本数
BPB[partition_ID].numbers_of_FAT = buf[16];
//////printf("\nnumbers_of_FAT=%d ",BPB[partition_ID].numbers_of_FAT);
//  u16 boot_entries;//根目录项数，供FAT12/16使用
BPB[partition_ID].boot_entries = buf[17] + buf[18] * 256;
//////printf("\nboot_entries=%d ",BPB[partition_ID].boot_entries);

//  u16 TotSec16; //This field is the old 16-bit total count of sectors on the volume.
BPB[partition_ID].TotSec16 = buf[19] + buf[20] * 256;
if(CORE[partition_ID].total_sector  == 0)
  CORE[partition_ID].total_sector = BPB[partition_ID].TotSec16;
//BPB[partition_ID].TotSec16 = 0xffff;
//////printf("\nTotSec16=%d ",BPB[partition_ID].TotSec16);

//  u8 media_descriptor; //媒体描述符
BPB[partition_ID].media_descriptor = buf[21];
//  u16 sectors_per_FAT; //每个FAT表占用的扇区数，供FAT12/16使用
BPB[partition_ID].sectors_per_FAT =  buf[22] + buf[23] * 256;

if(BPB[partition_ID].sectors_per_FAT == 0)
  BPB[partition_ID].sectors_per_FAT =  buf[36] + buf[37] * 256 + buf[38] * 256*256 + buf[39] * 256*256*256;
//////printf("\nsectors_per_FAT=%ld",BPB[partition_ID].sectors_per_FAT);
//  u16 sectors_per_track; //每道扇区数
//  u16 number_of_head; //磁头数
//  u32 BPB[disk_id]_HiddSec; //隐藏扇区数
//  u32 BPB[disk_id]_TotSec32;//总扇区数，包含FAT32总扇区数
//  u8 BS_DrvNum;
//  u8 BS_Reserved1;
//  u8 BS_BootSig;
//  u32 BS_VolID;
//  u8 BS_VolLab[11];
//  u8 BS_FilSysType[8];
CORE[partition_ID].system_id = 0x6; //Partition Type 0C-FAT32,06-FAT16 ect..
BPB[partition_ID].RootClus = buf[44]+buf[45]*256+buf[46]*256*256+buf[47]*256*256*256;

        BPB_INIT_2();
        return(SUCC);
       } 

    
  return(FAIL);
}  

/*
===============================================================================
函数
从路径中读一个entry
入口：
出口：SUCC,FAIL
===============================================================================
*/
static u8 SplitNameFromPath(u8 *Path,u8 *Return_Entry_Name,u8 *FileExtension,u8 *RemovedCharsFromPath)
{  
  u8 i,flag,j;
  flag = FILE_NAME;
  *RemovedCharsFromPath = 0; 
  CORE[disk_id].CurPathType = DirectoryPath; 
  i = 0; 
  j = 0;
  do{           
     if( ( * Path) != 0 && ( * Path ) != '\\') //Path分离得到Entry name and file extension 
       { 
        (*RemovedCharsFromPath)++; 
        if( flag == FILE_NAME)
         {            
          if(*Path == '.')
             {
              flag  = FILE_EXTENSION;
              Path ++; 
             }  
          else
            {
             Return_Entry_Name[i] =  *Path;
             i++; 
             Path++;
            }
          }
        else if( flag  == FILE_EXTENSION)
         {
          if( j >= 20 )
            return(FAIL);
          FileExtension[j] =  *Path;
          j++;
          Path++;
         } 
      }
    else
      {
       if(!( * Path))
        {
          if(CORE[0].FullPathType == FilePath)
		  {
		  CORE[disk_id].CurPathType = FilePath;
		  }
          FileExtension[j] = 0;
          Return_Entry_Name[i] = 0;
          return(LastSplitedNameofPath);  
        } 
       (*RemovedCharsFromPath)++; 
       FileExtension[j] = 0;
       Return_Entry_Name[i] = 0;	   
       break;
      }
 }while(1); 
 return(SUCC);
} 
/*
===============================================================================
函数
Directory Entry offset+32 
入口：buf--Current Sector Buffer
出口：SUCC,FAIL
===============================================================================
*/             
static u8 CORE_offset_add_32(u8 *buf)
{
  CORE[disk_id].offset += 32;
  if (CORE[disk_id].offset >= 512)
  {
  if (CORE[disk_id].DirectoryType == RootDirectory && CORE[disk_id].fs_type == FAT16)
   {
        if (CORE[disk_id].SectorNum < ( CORE[disk_id].RootDirSectors +  CORE[disk_id].FirstRootDirSecNum))
         {
           CORE[disk_id].SectorNum++;
           CORE[disk_id].offset = 0; 
           read_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
		   if(buf[CORE[disk_id].offset] == 0 && buf[CORE[disk_id].offset+1] == 0)  //End of the directory
		     return(FAIL);
           return(SUCC);
         }
        else
           return(FAIL);
     }
    else  
     {
        if( (CORE[disk_id].SectorNum - FirstSectorofCluster(CORE[disk_id].ClusterNum) + 1) >= BPB[disk_id].sector_per_cluster)
         {
           CORE[disk_id].ClusterNum = Get_Next_Cluster_From_Current_Cluster(CORE[disk_id].ClusterNum);
           if(( CORE[disk_id].ClusterNum >= 2 && CORE[disk_id].ClusterNum <= 0xfff7 && CORE[disk_id].fs_type == FAT16)
           || ( CORE[disk_id].ClusterNum >= 2 && CORE[disk_id].ClusterNum <= 0xfFFFFff && CORE[disk_id].fs_type == FAT32) )
		   {
               CORE[disk_id].SectorNum = FirstSectorofCluster(CORE[disk_id].ClusterNum); 
               CORE[disk_id].offset = 0;
               read_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
			   if(buf[CORE[disk_id].offset] == 0 && buf[CORE[disk_id].offset+1] == 0)  //End of the directory
                   return(FAIL);
               return(SUCC);
            }
           else
                return(FAIL);
         }
        else
         {
            CORE[disk_id].SectorNum++; 
            CORE[disk_id].offset = 0;
            read_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
			if(buf[CORE[disk_id].offset] == 0 && buf[CORE[disk_id].offset+1] == 0)  //End of the directory
                return(FAIL);
            return(SUCC);
         }
     }
  }
  if(buf[CORE[disk_id].offset] == 0 && buf[CORE[disk_id].offset+1] == 0)  //End of the directory
       return(FAIL);
  return(SUCC);
}
/*
===============================================================================
函数
从目录读一个EntryWith 8.3 Name
入口：
出口：SUCC,FAIL
===============================================================================
*/ 
static u8 GetEntryWith8_3Name(u8 *buf,u8* EntryName,u8 *Extension)
{
  u8 j;
  struct Directory_Entry_  *Directory_Entry_Local;  
  u8 *pointer;
  pointer = buf;
  Directory_Entry_Local = (struct Directory_Entry_ *) (pointer + CORE[disk_id].offset);
  for(j = 0;j < 8;j++)
   {
    if(Directory_Entry_Local->filename[j] == 0x20)
      break;
    EntryName[j] = Directory_Entry_Local->filename[j];
   }   
  EntryName[j] = 0; 
  for(j = 0;j < 3;j++)
   {  
    if(Directory_Entry_Local->file_extention[j] == 0x20)
        break;
    Extension[j] = Directory_Entry_Local->file_extention[j]; 
   }
  Extension[j] = 0;

  if(CORE[0].FullPathType == FilePath && CORE[disk_id].CurPathType == FilePath)
     CORE[disk_id].FileSize = *(u32*)Directory_Entry_Local->file_length;
  Directory_Entry_Local->file_length[0] = 0;
  Directory_Entry_Local->file_length[0] = 0;
  CORE[disk_id].ClusterOfDirectoryEntry = (Directory_Entry_Local->first_cluster_number_low2bytes[0]) + 
	  (Directory_Entry_Local->first_cluster_number_low2bytes[1]) * 256
	  + (Directory_Entry_Local->first_cluster_number_high2bytes[0]) * 256 * 256 +
	  (Directory_Entry_Local->first_cluster_number_high2bytes[1]) * 256 * 256 * 256;
 
  CORE[disk_id].PreEntrySectorNum = CORE[disk_id].SectorNum;
  CORE[disk_id].PreEntryoffset = CORE[disk_id].offset;
  CORE_offset_add_32(buf);//Directory Entry offset + 32 
  return(SUCC);
}

/*
===============================================================================
函数 
根据UNICODE编码求GB2312
入口：
出口：SUCC,FAIL
===============================================================================
*/
u16 get_gb2312_from_unicode(u16 unicode)
{
    u16 i;
/*	u16 front,end;
	front = 0;
	end = 21790;
	
	do
	{
	  if(unicode == unicode_to_gb2312_table[2*((front + end)/2)] )
	  {
		  return(unicode_to_gb2312_table[2*((front + end)/2)+1]);
	  }
	  else
	  {
	    if(unicode > unicode_to_gb2312_table[2*((front + end)/2)])
			front = (front + end)/2;
		else
			end =  (front + end)/2;
	  }
	}while(front < end); */
	for(i = 0;i<sizeof(gb2312_to_unicode_table);i+=2)
	{
	 if(gb2312_to_unicode_table[i+1] == unicode)
	  {
	   return(gb2312_to_unicode_table[i]);
	  }
	
	}
    return(0xffff);
}

/*
===============================================================================
函数
从目录读一个EntryWithLongFileName
入口：
出口：SUCC,FAIL
===============================================================================
*/ 
static u8 GetEntryWithLongFileName(u8 *buf,u8* longFileName,u8 *Extension)
{
 u8 j,FileNameOffset;
 u8 directory_entry_name[260];                
// u8 flag;
 u16 len,gb2312_,i,k,pos;
 struct LongNameDirectoryEntry *LongNameDirectoryEntry_Local;
 *Extension = 0; 
 FileNameOffset = 0;
 pos = 258;
 directory_entry_name[pos] = 0;
 LongNameDirectoryEntry_Local = (struct LongNameDirectoryEntry *) (buf + CORE[disk_id].offset);
 do{
//	 flag = FILE_NAME;
	 k = 0;//FileNameOffset;
     for(j = 1;j < 10;j+=2)
      {        
       if (LongNameDirectoryEntry_Local->dir_lname1[j] == 0)
          break;   
	   if(LongNameDirectoryEntry_Local->dir_lname1[j+1] != 0)
	   {  //unicode gb2312 convert
	      gb2312_ = get_gb2312_from_unicode(LongNameDirectoryEntry_Local->dir_lname1[j+1]*256+LongNameDirectoryEntry_Local->dir_lname1[j]);
	      longFileName[k] = gb2312_ >> 8;
          longFileName[k+1] = gb2312_;
		  k+=2;
	   }
	   else
	   {
         longFileName[k] = LongNameDirectoryEntry_Local->dir_lname1[j];
         k ++;
	   }
      } 
	 longFileName[k] = 0;
       if(j >= 10)
         { 
           for(j = 0;j < 12;j += 2)
            {  
              if (LongNameDirectoryEntry_Local->dir_lname2[j] == 0)
                 break;
	          if(LongNameDirectoryEntry_Local->dir_lname2[j+1] != 0)
			  {
	            gb2312_ = get_gb2312_from_unicode(LongNameDirectoryEntry_Local->dir_lname2[j+1]*256+LongNameDirectoryEntry_Local->dir_lname2[j]);
	            longFileName[k] = gb2312_>>8;
                longFileName[k+1] = gb2312_;
		        k+=2;
			  }
			  else
			  {  longFileName[k] = LongNameDirectoryEntry_Local->dir_lname2[j];
                 k++;    
			  }
            }
           if(j >= 12)   
                for(j = 0;j < 4;j += 2)
                 { 
                  if (LongNameDirectoryEntry_Local->dir_lname3[j] == 0)
                     break; 
				  if(LongNameDirectoryEntry_Local->dir_lname3[j+1] != 0)
				  {
	                gb2312_ = get_gb2312_from_unicode(LongNameDirectoryEntry_Local->dir_lname3[j+1]*256+LongNameDirectoryEntry_Local->dir_lname3[j]);
	                longFileName[k] = gb2312_>>8;
                    longFileName[k+1] = gb2312_;
		            k+=2;
				  }
			      else
				  {
                    longFileName[k] = LongNameDirectoryEntry_Local->dir_lname3[j];
                    k ++; 
				  }
                 }
          }
	 if(k > 0)
	 {
		 longFileName[k] = 0;
		 len = LengthofString(longFileName);
         i = 0;
		 while(longFileName[i]){
		   directory_entry_name[pos- len + i] = longFileName[i];
		   i++;
		 }
		 pos-=len;
	 }
	CORE[disk_id].PreEntrySectorNum = CORE[disk_id].SectorNum;
    CORE[disk_id].PreEntryoffset = CORE[disk_id].offset;
	if(CORE_offset_add_32(buf) == FAIL) //Directory Entry offset + 32 
       return(FAIL);
    FileNameOffset = 0;
    k = FileNameOffset;
    LongNameDirectoryEntry_Local = (struct LongNameDirectoryEntry *) (buf + CORE[disk_id].offset);
    if(LongNameDirectoryEntry_Local->dir_attr != ATTR_LONG_NAME)
     { 
           if ( ! (LongNameDirectoryEntry_Local->dir_attr & ATTR_VOLUME_ID)) 
           {
			CORE[disk_id].ClusterOfDirectoryEntry = LongNameDirectoryEntry_Local->dir_first[0]+
				LongNameDirectoryEntry_Local->dir_first[1] * 256
				+ buf[CORE[disk_id].offset + 20] * 256 * 256 + buf[CORE[disk_id].offset + 21] * 256 * 256 * 256;
            CORE[disk_id].FileSize = *((u32*)LongNameDirectoryEntry_Local->dir_lname3);
			stringcpy(directory_entry_name+pos,longFileName);
			len =  LengthofString(longFileName);
			len --;
			i = 0;
			do{
				if(longFileName[len] == '.')
				{
				  longFileName[len] = 0;
                  stringcpy(longFileName + len + 1,Extension);
				  break;
				}
			   len--;
			   i++;
			   if(i >= 4 || len == 0 )
				 break;
			}while(1);

            break;
           }
//       flag = FILE_NAME;

       FileNameOffset = 256 - 13;
       k = FileNameOffset; 
	   do{
		  CORE[disk_id].PreEntrySectorNum = CORE[disk_id].SectorNum;
          CORE[disk_id].PreEntryoffset = CORE[disk_id].offset;
          if(CORE_offset_add_32(buf) == FAIL) //Directory Entry offset + 32 
            return(FAIL); 
          LongNameDirectoryEntry_Local = (struct LongNameDirectoryEntry *) (buf + CORE[disk_id].offset);
	      if(LongNameDirectoryEntry_Local->dir_lname1[0] == 0xe5)
             continue;
	      if(LongNameDirectoryEntry_Local->dir_attr != ATTR_LONG_NAME)
		  {
	       if ( ! (LongNameDirectoryEntry_Local->dir_attr & ATTR_VOLUME_ID)) 
            return(GetEntryWith8_3Name(buf,longFileName,Extension));
		   else
			 continue;
		  }
	      else
		   break;
	   } while(1);
     } 
  }while(1); 
  return(SUCC);
 }

              
/*
===============================================================================
函数
从目录读一个Entry
入口：mode = 0：返回所有directory entry
出口：SUCC,FAIL
===============================================================================
*/     
static u8 GetEntryFromDirectory(u8 *EntryName, u8 *Extension,u8 mode)
{ 
struct Directory_Entry_  *Directory_Entry_Local; 
struct LongNameDirectoryEntry *LongNameDirectoryEntry_Local; 
//u8 flag; 
u8 buf[512];

read_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
do{  
//  flag = FILE_NAME;  //or = FILE_EXTENSION 0xfe 
  Directory_Entry_Local = (struct Directory_Entry_ *) (buf + CORE[disk_id].offset);
  if(Directory_Entry_Local->filename[0] == 0x0)
	  return(FAIL);
  if(Directory_Entry_Local->filename[0] == 0xe5)
  {
   CORE[disk_id].PreEntrySectorNum = CORE[disk_id].SectorNum;
   CORE[disk_id].PreEntryoffset = CORE[disk_id].offset;
   if(CORE_offset_add_32(buf) == FAIL) //Directory Entry offset + 32 
     return(FAIL);
   continue;
  }  

  switch(Directory_Entry_Local->file_attribute) 
    {
       case ATTR_LONG_NAME:{
          if(GetEntryWithLongFileName(buf,EntryName,Extension) == SUCC)
          {	
		   read_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
           LongNameDirectoryEntry_Local = (struct LongNameDirectoryEntry *)(buf + CORE[disk_id].offset);
		   CORE[0].Entry_Attr = LongNameDirectoryEntry_Local->dir_attr;
		   
		   if(mode == Get_Selected_ENTRIES)
		   {
             if(CORE[disk_id].CurPathType == DirectoryPath && 
               (LongNameDirectoryEntry_Local->dir_attr & ATTR_DIRECTORY))
			 { 
			  
               CORE[disk_id].ClusterOfDirectoryEntry = *(u16*)LongNameDirectoryEntry_Local->dir_first
               + buf[CORE[disk_id].offset + 20] * 256 * 256 + buf[CORE[disk_id].offset + 21] * 256 * 256 * 256;
			   CORE[disk_id].PreEntrySectorNum = CORE[disk_id].SectorNum;
               CORE[disk_id].PreEntryoffset = CORE[disk_id].offset;
			   CORE_offset_add_32(buf);//Directory Entry offset + 32 
               return(SUCC);
			 }
             else if ( ! (LongNameDirectoryEntry_Local->dir_attr & ATTR_VOLUME_ID)) 
			 {
              CORE[disk_id].PreEntrySectorNum = CORE[disk_id].SectorNum;
             CORE[disk_id].PreEntryoffset = CORE[disk_id].offset;
			 CORE_offset_add_32(buf);//Directory Entry offset + 32 
             return(SUCC);
             }	   
		   }
		   else
		   {
			CORE[disk_id].PreEntrySectorNum = CORE[disk_id].SectorNum;
            CORE[disk_id].PreEntryoffset = CORE[disk_id].offset;
		    CORE[disk_id].ClusterOfDirectoryEntry = *(u16*)LongNameDirectoryEntry_Local->dir_first +
            buf[CORE[disk_id].offset + 20] * 256 * 256 + buf[CORE[disk_id].offset + 21] * 256 * 256 * 256;
			CORE_offset_add_32(buf);//Directory Entry offset + 32 
            
			return(SUCC);
		   }
          }
          break;
        }
       case ATTR_DIRECTORY:{
		  CORE[0].Entry_Attr = Directory_Entry_Local->file_attribute;
		  if(mode == Get_Selected_ENTRIES)
           if(CORE[0].FullPathType == FilePath && CORE[disk_id].CurPathType == FilePath)
             break;
          if(GetEntryWith8_3Name(buf,EntryName,Extension)  == SUCC)
             return(SUCC);       
          break;
       }
       case ATTR_VOLUME_ID:CORE[0].Entry_Attr = Directory_Entry_Local->file_attribute;
       //case 0:break;
       default:
        {
		 CORE[0].Entry_Attr = Directory_Entry_Local->file_attribute;
	     if(mode == Get_Selected_ENTRIES)
          if(CORE[0].FullPathType == DirectoryPath)
             break;
          return(GetEntryWith8_3Name(buf,EntryName,Extension)); 
        }
     } 
  CORE[disk_id].PreEntrySectorNum = CORE[disk_id].SectorNum;
  CORE[disk_id].PreEntryoffset = CORE[disk_id].offset;
  if(CORE_offset_add_32(buf) == FAIL) //Directory Entry offset + 32 
    return(FAIL);
 }while(1); 
//return(SUCC);
}  
/*
===============================================================================
函数
从目录中找一个Entry
入口：
出口：SUCC,FAIL
===============================================================================
*/
static u8 FindEntryStruct(u8 *floder_name,u8 *file_extension)
{  
   u8 EntryName[256],Extension[20]; 
   u8 Name_Compare_OK;
   //Extension_Compare_OK;   
   do{		   
          if(GetEntryFromDirectory(EntryName,Extension,Get_Selected_ENTRIES) != SUCC)
		  {
	       
            return(FAIL);
		  }
	   Name_Compare_OK = OK;
       if(stringcmp(EntryName,floder_name) != SUCC)
             Name_Compare_OK = unOK;        
       if(Name_Compare_OK == OK)  //检查文件扩展名
         {      
           if(CORE[0].FullPathType == FilePath && CORE[disk_id].CurPathType == FilePath)
              { 
					
//                  Extension_Compare_OK = OK;     
                  if(stringcmp(Extension,file_extension) == SUCC)
//                     Extension_Compare_OK = unOK;
//                  else
                     break;  
              }
            else
			{  	
                if(Extension[0] == 0) 
                 break;

              }  
          }
     }while(1);
   return(SUCC);
}
/*
===============================================================================
函数
Relative Path converts To Sector,SectorOffset,Cluster
入口：u8 *filename
出口：SUCC,FAIL
===============================================================================
*/
static u8 RelativePathToSectorCluster(u8 *RelativePath)
{ 
  u8 floder_name[256],file_extension[20]; 
  u8 Splited_Count;
  u8 Splited_Status;
  Splited_Status = SplitNameFromPath(RelativePath,floder_name,file_extension,&Splited_Count);
  if(Splited_Status == FAIL)
      return(FAIL);
  RelativePath += Splited_Count;  
  if(FindEntryStruct(floder_name,file_extension) != SUCC)
  {
   return(FAIL); 
  }
   if(CORE[disk_id].CurPathType == DirectoryPath)
     if(CORE[disk_id].DirectoryType == RootDirectory)
	  {
	  CORE[disk_id].DirectoryType = NoneRootDirectory; 
	  }
      
  if(Splited_Status == LastSplitedNameofPath)
  {
   return(SUCC); 
  }    
  do{ 
     Splited_Status = SplitNameFromPath(RelativePath,floder_name,file_extension,&Splited_Count);
     if(Splited_Status == FAIL)
	  return(FAIL);    
     else 
       { 
         CORE[disk_id].ClusterNum = CORE[disk_id].ClusterOfDirectoryEntry;
         CORE[disk_id].SectorNum = FirstSectorofCluster(CORE[disk_id].ClusterNum);
         CORE[disk_id].offset = 0;  
       }
     RelativePath += Splited_Count;
     if(CORE[disk_id].CurPathType == DirectoryPath)
      if(CORE[disk_id].DirectoryType == RootDirectory)
	  {
	  CORE[disk_id].DirectoryType = NoneRootDirectory; 
	  }
     if(FindEntryStruct(floder_name,file_extension) != SUCC)
	 {
       return(FAIL); 
	 }
     else if(Splited_Status == LastSplitedNameofPath)
	 {
      return(SUCC);
	 }  

    }while(1);
//    return(SUCC);
}
/*
===============================================================================
函数
Full Path converts To Sector,SectorOffset,Cluster
入口：u8 *filename
出口：SUCC,FAIL(u32 *cluster_no,u32 *sector,u16 *offset)
===============================================================================
*/
static u8 FullPathToSectorCluster(u8 * filename1)
{
   u8 buf[260],* filename;
   stringcpy(filename1,buf);
   filename = buf;
   UPCASE(filename);
   if( ((* filename) >= 'A' && ( * filename ) <= 'Z') )  //从指定盘符根目录开始寻址
       {
         if(( * (filename + 1)) == ':')
          {                          
           if( *( filename + 2 ) == '\\')
            { 

			  disk_id = (* filename ) - 'C';
              if(LengthofString(filename) > Maximum_File_Path_Name)
			      return(EpathLengthsOVERFLOW);
              filename += 3;
			  if(CORE[disk_id].fs_type == FAT16)
                CORE[disk_id].SectorNum = CORE[disk_id].FirstRootDirSecNum; 
			  else
			  {
				  CORE[disk_id].SectorNum = FirstSectorofCluster(CORE[disk_id].RootClus); 
			      CORE[disk_id].ClusterNum = CORE[disk_id].RootClus;
			  }
              CORE[disk_id].DirectoryType =  RootDirectory;
              CORE[disk_id].offset = 0;    
             }
          }
         else 
		 {   UPCASE(CORE[0].current_folder);
			 disk_id = (* CORE[0].current_folder ) - 'C';
             if((LengthofString(filename) + LengthofString(CORE[0].current_folder)) > Maximum_File_Path_Name)
                   return(EpathLengthsOVERFLOW);
             if(CORE[0].CurrentDirectoryType ==  RootDirectory)
			 {
			  if(CORE[disk_id].fs_type == FAT16)
                CORE[disk_id].SectorNum = CORE[disk_id].FirstRootDirSecNum; 
			  else
			  {
				  CORE[disk_id].SectorNum = FirstSectorofCluster(CORE[disk_id].RootClus); 
			      CORE[disk_id].ClusterNum = CORE[disk_id].RootClus;
			  }
			 }  
             else
              {
                 CORE[disk_id].ClusterNum = CORE[disk_id].ClusterNOofCurrentFolder;
                 CORE[disk_id].SectorNum = FirstSectorofCluster(CORE[disk_id].ClusterNum); 
              }
             CORE[disk_id].DirectoryType = CORE[0].CurrentDirectoryType;
             CORE[disk_id].offset = 0;	 

          }  
       }
       else if((* filename) == '\\')
            {UPCASE(CORE[0].current_folder);
		     disk_id = (* CORE[0].current_folder ) - 'C';
             if((LengthofString(filename) + 1) > Maximum_File_Path_Name)
                   return(EpathLengthsOVERFLOW); 

             filename ++;    //从当前盘符，根目录开始寻址
			  if(CORE[disk_id].fs_type == FAT16)
                CORE[disk_id].SectorNum = CORE[disk_id].FirstRootDirSecNum; 
			  else
			  {
				  CORE[disk_id].SectorNum = FirstSectorofCluster(CORE[disk_id].RootClus); 
			      CORE[disk_id].ClusterNum = CORE[disk_id].RootClus;
			  }
			 CORE[disk_id].DirectoryType = RootDirectory;
             CORE[disk_id].offset = 0;

            } 
  if(*filename)
     return(RelativePathToSectorCluster(filename));
  else
	return(SUCC);
   
}
/*
===============================================================================
函数
检查文件打开？
入口：u8 FCBsn
出口：FileAlreadyopenedByOtherHandle,FileUnopenedByOtherHandle
===============================================================================
*/ 
u8 Check_FileOpened_Status(u32 FirstClusterOfFile,u16 j, u8 disk_)
{
  u8 i;
  for(i = 0;i < MaximumFCB;i++)
  {
    if(i == j)
		continue;
   if(FCB[i].file_openned_flag == UsedFlag)
   {
     if(FCB[i].FirstClusterOfFile == FirstClusterOfFile && FCB[i].disk == disk_)
        return(FileAlreadyopenedByOtherHandle);
   }
  }
  return(FileUnopenedByOtherHandle);
} 
/*
===============================================================================
函数
将FCB file buffer回写入磁盘
入口：u8 FCBsn
出口：SUCC,FAIL
===============================================================================
*/ 
#if EnableFileBuf
u8 Writeback_FCB_file_buffer(u8 FCBsn)
{ 
  u16 i;
  u16 ClusterQTY,BUFoffset;
  u32 FCBbufSize,sector,NEXTCluster,qty,TEMP; 
  u8  buf[512];
  u32 FILESIZE;
  u16 wrote_sectors_count;
  //未分配文件首簇号，则执行分配

  if(FCB[FCBsn].FirstClusterOfFile == 0)
  { 
	if( Allocate_An_Empty_cluster(&FCB[FCBsn].FirstClusterOfFile,buf,NoRequiredCls)== FAIL)
	  return(FAIL);
	read_flash_sector(buf,FCB[FCBsn].Entry_Storedin_Sector,disk_id);
    i = FCB[FCBsn].Entry_Storedin_Sector_Offset + 26;
	buf[i] = (u8)(FCB[FCBsn].FirstClusterOfFile & 0xff);
	buf[i+1] = (u8)((FCB[FCBsn].FirstClusterOfFile >> 8) & 0xff);
    i = FCB[FCBsn].Entry_Storedin_Sector_Offset + 20;
    buf[i] = (u8)((FCB[FCBsn].FirstClusterOfFile >> 16) & 0xff);
	buf[i+1] = (u8)((FCB[FCBsn].FirstClusterOfFile >> 24) & 0xff);
	write_flash_sector(buf,FCB[FCBsn].Entry_Storedin_Sector,disk_id);
	FCB[FCBsn].CurClusterInBUF = FCB[FCBsn].FirstClusterOfFile;
  }
  ClusterQTY = FCB[FCBsn].CurBlockInBUF / BPB[disk_id].sector_per_cluster;
  qty = ClusterQTY;
  if(FCB[FCBsn].ClusterSNInBUF <= ClusterQTY)
  {
    ClusterQTY -= FCB[FCBsn].ClusterSNInBUF;
    NEXTCluster = FCB[FCBsn].CurClusterInBUF;
  }
  else
  {
    NEXTCluster = FCB[FCBsn].FirstClusterOfFile; 
  }
  //printf("ClusterQTY = %d",ClusterQTY);
  //费时的地方 004
  while(ClusterQTY)
  {TEMP = NEXTCluster;
   NEXTCluster =  Get_Next_Cluster_From_Current_Cluster(NEXTCluster);
   ClusterQTY--;
   if(CORE[disk_id].fs_type == FAT16)
   {
	 if(NEXTCluster >= 0xfff4 && NEXTCluster <= 0xffff)
	   {
	    if (Allocate_EMPTY_CLUSTER_TO_CUR_CLUSTER_CHAIN(TEMP,&NEXTCluster,buf,NoRequiredCls) == FAIL)
			return(FAIL);
	   }
   }
   else
   {
     if(NEXTCluster >= 0xfffffff && NEXTCluster <= 0xffffffff)
	   {
	    if (Allocate_EMPTY_CLUSTER_TO_CUR_CLUSTER_CHAIN(TEMP,&NEXTCluster,buf,NoRequiredCls) == FAIL)
			return(FAIL);
	   }
   
   }
  } 
  FCB[FCBsn].CurClusterInBUF = NEXTCluster;
  FCB[FCBsn].ClusterSNInBUF = qty; 

  //Wirteback FCB file buffer to physical sector
  FCBbufSize = FileBUFSize * TotalFileBUFsQTYeachFCB; 
  BUFoffset = 0;
  i = FCB[FCBsn].CurBlockInBUF % BPB[disk_id].sector_per_cluster;
  sector = FirstSectorofCluster(NEXTCluster);
  wrote_sectors_count = 0;
  //费时的地方 005 
  do{ 
     write_flash_sector(FCB[FCBsn].FileBUF + BUFoffset,sector + i,disk_id); 
     wrote_sectors_count++;
	 i++;
	 if(((wrote_sectors_count + FCB[FCBsn].CurBlockInBUF) * 512) >= FCB[FCBsn].FileSize)
	 {
	 break;
	 }
     BUFoffset += FileBUFSize;
     FCBbufSize -= FileBUFSize;
     if(FCBbufSize == 0)
	 {
	 break;
	 }
     if(i >= BPB[disk_id].sector_per_cluster)
	 {
	   TEMP = NEXTCluster; 
       NEXTCluster = Get_Next_Cluster_From_Current_Cluster(NEXTCluster);

       if(CORE[disk_id].fs_type == FAT16)
	   {
	     if(NEXTCluster >= 0xfff4 && NEXTCluster <= 0xffff)
		 {
	       if (Allocate_EMPTY_CLUSTER_TO_CUR_CLUSTER_CHAIN(TEMP,&NEXTCluster,buf,NoRequiredCls) == FAIL)
		   	  return(FAIL);
		 }
	   }
      else
	  {
         if(NEXTCluster >= 0xfffffff && NEXTCluster <= 0xffffffff)
		 {
	       if (Allocate_EMPTY_CLUSTER_TO_CUR_CLUSTER_CHAIN(TEMP,&NEXTCluster,buf,NoRequiredCls) == FAIL)
		  	return(FAIL);
		 }
   
	  }
	   sector = FirstSectorofCluster(NEXTCluster);
       i = 0;
      }    
  }while(1);
  FCB[FCBsn].Modified_Flag = 0;
  //Read File Directory entry
  read_flash_sector(buf,FCB[FCBsn].Entry_Storedin_Sector,disk_id);
  i = FCB[FCBsn].Entry_Storedin_Sector_Offset + 28;
  FILESIZE = buf[i] + buf[i+1] * 256 + buf[i+2] * 256 * 256+ buf[i+3]  * 256 * 256 *256;
  //check FileSize is modified?
  if(FILESIZE != FCB[FCBsn].FileSize) 
  { //if filesize was increased,need to update filesize of the File Directory entry
	FILESIZE = FCB[FCBsn].FileSize;
	buf[i] = (u8)(FILESIZE & 0xff);
	buf[i+1] = (u8)((FILESIZE >> 8) & 0xff);
	buf[i+2] = (u8)((FILESIZE >> 16) & 0xff);
	buf[i+3] = (u8)((FILESIZE >> 24) & 0xff);
    write_flash_sector(buf,FCB[FCBsn].Entry_Storedin_Sector,disk_id);
  }
  return(SUCC);
}
#endif 

/*
===============================================================================
函数
根据文件的当前位置，更新FCB文件缓冲区
入口：u8 FCBsn
出口：SUCC,FAIL
===============================================================================
*/ 
#if EnableFileBuf
u8 Update_FCB_file_buffer(u8 FCBsn)
{ 
  u16 i;
  u16 ClusterQTY,BUFoffset;
  u32 FCBbufSize,sector,NEXTCluster,qty;
  if(!(FileBUFSize * TotalFileBUFsQTYeachFCB))
     return(SUCC);
  if(((FCB[FCBsn].cur_position) >= FCB[FCBsn].CurBlockInBUF * FileBUFSize && 
     ((FCB[FCBsn].cur_position) <  (FCB[FCBsn].CurBlockInBUF + TotalFileBUFsQTYeachFCB) * FileBUFSize) ))
  {
  return(SUCC); 
  }

  //费时的地方 001
  if(FCB[FCBsn].Modified_Flag)
    if(Writeback_FCB_file_buffer(FCBsn) == FAIL)
	  return(FAIL);
  //Initialize FCB file buffer
  FCB[FCBsn].CurBlockInBUF = FCB[FCBsn].cur_position / FileBUFSize;
  ClusterQTY = FCB[FCBsn].CurBlockInBUF / BPB[disk_id].sector_per_cluster;
  qty = ClusterQTY;

  
  if(FCB[FCBsn].ClusterSNInBUF_for_read <= ClusterQTY)
  { 
    ClusterQTY -= FCB[FCBsn].ClusterSNInBUF_for_read;
    NEXTCluster = FCB[FCBsn].CurClusterInBUF_for_read;
  }
 else
 {
    NEXTCluster = FCB[FCBsn].FirstClusterOfFile; 
  }  
  //费时的地方 002
  while(ClusterQTY)
  {
   if(CORE[disk_id].fs_type == FAT16)
   {
	   if(NEXTCluster >= 0xfff4 && NEXTCluster <= 0xffff)
		 {  
		   return(SUCC);
		 }
   }
   else
   {
   
	   if(NEXTCluster >= 0xfffffff && NEXTCluster <= 0xffffffff)
	   {
		   return(SUCC);}
       }
   NEXTCluster =  Get_Next_Cluster_From_Current_Cluster(NEXTCluster);
   ClusterQTY--;
  } 
  FCB[FCBsn].CurClusterInBUF_for_read = NEXTCluster;
  FCB[FCBsn].ClusterSNInBUF_for_read = qty; 

  FCBbufSize = FileBUFSize * TotalFileBUFsQTYeachFCB; 
  BUFoffset = 0;
  i = FCB[FCBsn].CurBlockInBUF % BPB[disk_id].sector_per_cluster;
  sector = FirstSectorofCluster(NEXTCluster);
  //费时的地方 003
  do{  
   read_flash_sector(FCB[FCBsn].FileBUF + BUFoffset,sector + i,disk_id); 
   BUFoffset += FileBUFSize;
   FCBbufSize -= FileBUFSize;
   if(FCBbufSize == 0)
    {
     return(SUCC); 
    }
   i++;
   if(i >= BPB[disk_id].sector_per_cluster)
     {
       NEXTCluster= Get_Next_Cluster_From_Current_Cluster(NEXTCluster);
       if(CORE[disk_id].fs_type == FAT16)
	   {
	     if(NEXTCluster >= 0xfff4 && NEXTCluster <= 0xffff)
	       return(SUCC);
	   }
	   else
	   {
	     if(NEXTCluster >= 0xfffffff && NEXTCluster <= 0xffffffff)
	       return(SUCC);	   
	   
	   }
	   sector = FirstSectorofCluster(NEXTCluster);
       i = 0;
      }    
  }while(1);
}
#endif  

/*
===============================================================================
函数
分配一个FCB
入口：无
出口：EAllocate_FCB-- Fail,other--SUCC with FCB sequential number
===============================================================================
*/ 
u8 Allocate_FCB(void)
{  
 u8 i;
 for (i = 0; i < MaximumFCB;i++)
  if (FCB[i].file_openned_flag == UnusedFlag)
   {          
    FCB[i].file_openned_flag = UsedFlag;
	FCB[i].Modified_Flag = 0;
	FCB[i].disk = disk_id;
    return(i);
   }
 return(EAllocate_FCB);
}
/*
===============================================================================
函数
free a FCB
入口：FCB_sequential_number
出口：EFree_FCB,SUCC
===============================================================================
*/     
u8 Free_FCB(u8 FCB_sequential_number)
{
 if(FCB[FCB_sequential_number].file_openned_flag == UsedFlag)
  {
    FCB[FCB_sequential_number].file_openned_flag = UnusedFlag;
	FCB[FCB_sequential_number].Modified_Flag = 0;
    return(SUCC);
  }
  else
   return(EFree_FCB);
}
/*
===============================================================================
函数
FAT file system initialiation
入口：无
出口：无
===============================================================================
*/ 
#if complie_FAT_filesystem_initialiation
u8 FAT_filesystem_initialiation()
{ 
  u8 root[] = "C:\\",i;

  disk_id = 0;
//  Directory_Entry.filename[0]  = 0;
  
  CORE[disk_id].PartitionID = 0xff;
  CORE[0].CurrentDirectoryType =  RootDirectory; 
  stringcpy(root,CORE[0].current_folder);
  for (i = 0; i < MaximumFCB;i++)
  {
   FCB[i].file_openned_flag = UnusedFlag; //UsedFlag
   FCB[i].Modified_Flag = 0;
  }
  //read defalut partition BPB[disk_id] and related information to RAM buffer
  
  for(i = 0;i < maximum_disks;i++)
  { 
	  CORE[i].min_cluster_for_allocat = 2;
	  Read_partition_PBP(i);
  }
  disk_id = 0;

  //create the mutex
  fat_filesystem_Mutex = OSSemCreate (1);
  
  return(SUCC);
} 
#endif
/*
===============================================================================
函数
auto format FAT16 file system
入口：无
出口：无
===============================================================================
*/ 
#if complie_FAT16_filesystem_autoformat
u8 FAT16_filesystem_autoformat(u8 disk_)
{
 u8 buf[512]; 
 if( read_flash_sector(buf,0,disk_) == SUCC ) //read MBR
    {
      if ( ! ( buf[510] == 0x55 && buf[511] == 0xaa))
        {
          //put the autoformat code at here 
          write_flash_sector((u8 *)MBR,0,disk_);
          //write FAT1/FAT2,root directory
          //put code here
        }
    }
 return(SUCC); 
} 
#endif
   

/*
===============================================================================
函数 
打开文件
入口：u8 * filename:路径+文件名
出口：返回PCB_SN,FAIL
===============================================================================
*/   
#if complie_open_file 
u8 open_file(u8 * filename)
{ 
 u8 FCBsn;
 enter_Mutex;
 CORE[0].FullPathType = FilePath;
 if(FullPathToSectorCluster(filename) == SUCC)
 { 
     FCBsn = Allocate_FCB();
     if(FCBsn == EAllocate_FCB)
       {
	   	 release_Mutex;
	     return(FAIL);
	 	}
     FCB[FCBsn].cur_position = 0;
	 FCB[FCBsn].CurBlockInBUF = 0xffff;
  
     FCB[FCBsn].FirstClusterOfFile = CORE[disk_id].ClusterOfDirectoryEntry;



     FCB[FCBsn].CurClusterInBUF = FCB[FCBsn].FirstClusterOfFile;
     FCB[FCBsn].ClusterSNInBUF = 0;

	 FCB[FCBsn].CurClusterInBUF_for_read= FCB[FCBsn].FirstClusterOfFile;
     FCB[FCBsn].ClusterSNInBUF_for_read = 0;



     FCB[FCBsn].FileSize = CORE[disk_id].FileSize;
     FCB[FCBsn].Entry_Storedin_Sector = CORE[disk_id].PreEntrySectorNum ;  //Save sectorNUM of File Directory entry for current file
     FCB[FCBsn].Entry_Storedin_Sector_Offset = CORE[disk_id].PreEntryoffset; //Save offset in sector of File Directory entry for current file
     FCB[FCBsn].Modified_Flag = 0;
	 //第一个参数是文件的首簇号，第2个参数是文件对应的FCBsn,未分配FCB的文件使用0xff替代
	 if(Check_FileOpened_Status(CORE[disk_id].ClusterOfDirectoryEntry,FCBsn,disk_id) == FileAlreadyopenedByOtherHandle)
         FCB[FCBsn].Permission = ReadOnlyPermission;
	 else
         FCB[FCBsn].Permission = FullPermission;
   #if EnableFileBuf
     Update_FCB_file_buffer(FCBsn);
   #endif 
   #if enable_time_stamp_transaction
      write_time_stamp(FCBsn,ACCESS_TIME);
   #endif
	 release_Mutex;
     return(FCBsn); 
   }
  else
  {release_Mutex;
   return(FAIL);
   }
}
#endif 
/*
===============================================================================
函数 
关闭文件
入口：无
出口：SUCC,FAIL
===============================================================================
*/  
#if complie_close_file
u8 close_file(u8 FCBsn)
{  enter_Mutex;
   #if enable_time_stamp_transaction
      write_time_stamp(FCBsn,ACCESS_TIME);
   #endif
  if(Free_FCB(FCBsn) == SUCC)
    {
	release_Mutex;
	return(SUCC);}
  else
    {
	 release_Mutex;
	 return(FAIL);}
} 
#endif  
/*
===============================================================================
函数
Directory Entry offset+32 由建立文件和目录使用，遇末簇可自动分配空簇加入簇链
入口：buf--Current Sector Buffer
出口：SUCC,FAIL
===============================================================================
*/             
static u8 CORE_offset_add_32_With_EMPTY_CLUSTER_ALLOCATION(u8 *buf)
{
  u32 temp;
  CORE[disk_id].offset += 32;
  if (CORE[disk_id].offset >= 512)
  {
   CORE[disk_id].SectorNum++;
  if (CORE[disk_id].DirectoryType == RootDirectory && CORE[disk_id].fs_type == FAT16)
  {     
        if (CORE[disk_id].SectorNum < ( CORE[disk_id].RootDirSectors +  CORE[disk_id].FirstRootDirSecNum))
         {
           
           CORE[disk_id].offset = 0; 
           read_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
           return(SUCC);
         }
        else
           return(FAIL);
     }
    else  
     {		
        if( (CORE[disk_id].SectorNum - FirstSectorofCluster(CORE[disk_id].ClusterNum) + 1) > BPB[disk_id].sector_per_cluster)
         {
		   temp  = CORE[disk_id].ClusterNum;
           CORE[disk_id].ClusterNum = Get_Next_Cluster_From_Current_Cluster(CORE[disk_id].ClusterNum);
           if(( CORE[disk_id].ClusterNum >= 2 && CORE[disk_id].ClusterNum < 0xfff7 && CORE[disk_id].fs_type == FAT16)
			 ||(CORE[disk_id].ClusterNum >= 2 && CORE[disk_id].ClusterNum < 0xFFFFfff && CORE[disk_id].fs_type == FAT32))   
            {
               CORE[disk_id].SectorNum = FirstSectorofCluster(CORE[disk_id].ClusterNum); 
               CORE[disk_id].offset = 0;
               read_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
               return(SUCC);
            }
           else
		   {   //遇末簇，自动分配空簇加入簇链 
			   if(Allocate_EMPTY_CLUSTER_TO_CUR_CLUSTER_CHAIN(temp,&temp,buf,NeedCLS) == FAIL)
				   return(FAIL);

               CORE[disk_id].ClusterNum = temp;
               CORE[disk_id].SectorNum = FirstSectorofCluster(CORE[disk_id].ClusterNum); 
               CORE[disk_id].offset = 0;

               read_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
			   return(SUCC);
		   }
         }
        else
         {
            CORE[disk_id].offset = 0;
            read_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
            return(SUCC);
         }
     }
  }
  return(SUCC);
}
/*
===============================================================================
函数 
计算longfilenameentry校验和
入口：*entry:longfilenameentry首地址
出口：checksum
===============================================================================
*/
static u8 calculate_checksum_longfilenameentry(u8 *entry)
{
 u8 sum,i;
 sum=0;
 i = 0;
 while (i < 11)
  {
   sum = (sum >> 1) | (sum << 7);
   sum += entry[i];
   i++;
  }
  return(sum);
}
/*
===============================================================================
函数 
添加一个Directory_Entry到目录，目录首簇值为CORE[disk_id].ClusterOfDirectoryEntry或
CORE[disk_id].FirstRootDirSecNum
入口：Directory_Entry:Directory_Entry name,attr:attr of Directory_Entry
出口：SUCC,FAIL
===============================================================================
*/
static u8 Seek_Space_to_Write_Directory_Entry(u16 Entry_Resuested_QTY,u8 *buf)
{
  u32 SectorNum_LOCAL,ClusterNum_LOCAL;
  u16 Offset_LOCAL;
  u8 Empty_Entry_Space_Count,found_flag;
  if(CORE[disk_id].DirectoryType == RootDirectory)
  {
	if(CORE[disk_id].fs_type == FAT16)
      CORE[disk_id].SectorNum = CORE[disk_id].FirstRootDirSecNum;
	else
	{
	  CORE[disk_id].SectorNum = FirstSectorofCluster(CORE[disk_id].RootClus); 
	  CORE[disk_id].ClusterNum = CORE[disk_id].RootClus;
	}
  }
  else
  { 
    CORE[disk_id].ClusterNum = CORE[disk_id].ClusterOfDirectoryEntry;   //存放当前Enumerated Directory Entry所在Directory的ClusterNum,SectorNum,offset
    CORE[disk_id].SectorNum = FirstSectorofCluster(CORE[disk_id].ClusterNum);
  } 
  CORE[disk_id].offset = 0;

  read_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
  while(1)
  { 
    SectorNum_LOCAL = CORE[disk_id].SectorNum; 
    ClusterNum_LOCAL = CORE[disk_id].ClusterNum;
    Offset_LOCAL = CORE[disk_id].offset;
	found_flag = 0;
	Empty_Entry_Space_Count = 0;
    do{
       if(buf[CORE[disk_id].offset] == 0xe5 || buf[CORE[disk_id].offset] == 0)
	   {
	    Empty_Entry_Space_Count++;
	    if(Empty_Entry_Space_Count >= Entry_Resuested_QTY)
		{
	     found_flag = 1;
	     break;
		}
	   }
	   else
	     break;
	   if(CORE_offset_add_32_With_EMPTY_CLUSTER_ALLOCATION(buf) == FAIL)
			return(FAIL);
	}while(1);
    if(found_flag)
	{ 
	 CORE[disk_id].SectorNum = SectorNum_LOCAL; 
     CORE[disk_id].ClusterNum = ClusterNum_LOCAL;
     CORE[disk_id].offset = Offset_LOCAL;
     break;
	}
	else 
	{
	   if(CORE_offset_add_32_With_EMPTY_CLUSTER_ALLOCATION(buf) == FAIL)
	      return(FAIL);
    }
  } 

 return(SUCC); 

}
/*
===============================================================================
函数 
计算汉字字符串长度
入口：
出口：SUCC,FAIL
===============================================================================
*/
u8 *Lengthof_CHN_String(u8 *Entry_Name,u16 *len)
{
  u8 pos;
  *len = 0;
  pos = 0;
  do{
	  if(Entry_Name[pos]== 0)
	  {
	    return(Entry_Name+pos);
	  }
     if(Entry_Name[pos] & 0x80)
	 {
        pos+=2;
	 }
     else
	 { 
		 pos+=1;
	 }
	 (*len)++;
  }while(1);


}
/*
===============================================================================
函数 
根据GB2312编码求UNICODE
入口：
出口：SUCC,FAIL
===============================================================================
*/
u16 get_pos_in_table_about_unicode(u16 gb2312_code)
{
    

	/*
	u16 front,end;
	front = 0;
	end = 21790;
	
	do
	{
	  if(gb2312_code == gb2312_to_unicode_table[2*((front + end)/2)] )
	  {
		  return(2*((front + end)/2)+1);
	  }
	  else
	  {
	    if(gb2312_code > gb2312_to_unicode_table[2*((front + end)/2)])
			front = (front + end)/2;
		else
			end =  (front + end)/2;
	  }
	}while(front < end);*/ 
   u16 i;
	for(i = 0;i<sizeof(gb2312_to_unicode_table);i+=2)
	{
	  if(gb2312_to_unicode_table[i] == gb2312_code)
	  {
	    return(i+1);
	  }
	
	}
    return(0xffff);
}
/*
===============================================================================
函数 
完成填写一个LFN directory entry
入口：u8 * Directory_Entry_buf,u8 * Entry_Name,u8 LFN_record_FLAG
      u8 Entry_Name,u8 checksum
出口：SUCC,FAIL
===============================================================================
*/
static u8 Fill_LFN_Directory_Entry(u8 * Directory_Entry_buf,u8 * Entry_Name,u8 LFN_record_FLAG,u8 checksum)
{
  u16 i,pos;
  u16 table_position;
  u8 *point;
  point = Lengthof_CHN_String(Entry_Name,&i);

  pos = 1;
  if(i < 12) 
  {
	  do{          
		
		if(i >= 13)
			break;
		i++;
        point[pos++] = 0xff;
	  }while(1);
 }
  point[pos++] =0;

  pos = 0;
  Directory_Entry_buf[0] = LFN_record_FLAG;;
  for(i = 0;i < 5;i++)
  {
	if((Entry_Name[pos] & 0x80) && (Entry_Name[pos] != 0xff))
	{   
		table_position = get_pos_in_table_about_unicode(Entry_Name[pos]*256 + Entry_Name[pos+1]);
	    Directory_Entry_buf[1 + i * 2] = gb2312_to_unicode_table[table_position];
		Directory_Entry_buf[2 + i * 2] = gb2312_to_unicode_table[table_position]>>8;

		pos+=2;
	}
	else
	{
		Directory_Entry_buf[1 + i * 2] = Entry_Name[pos++];
	    if(Entry_Name[pos-1] != 0xff)
	      Directory_Entry_buf[2 + i * 2] = 0;
	    else 
          Directory_Entry_buf[2 + i * 2] = 0xff;
	}

  }
  Directory_Entry_buf[11] = ATTR_LONG_NAME; //0x0F, impossible file attribute used as signature
  Directory_Entry_buf[12] = 0;    //Reserved (?). Set to 0x00
  Directory_Entry_buf[13] = checksum;
  for(i = 0;i < 6;i++)
  {

    if((Entry_Name[pos] & 0x80) && (Entry_Name[pos] != 0xff))
	{    //get the unicode position in gb2312_to_unicode_table
		table_position = get_pos_in_table_about_unicode(Entry_Name[pos]*256 + Entry_Name[pos+1]);
	    Directory_Entry_buf[14 + i * 2] = gb2312_to_unicode_table[table_position];
		Directory_Entry_buf[15 + i * 2] = gb2312_to_unicode_table[table_position]>>8;
		pos+=2;
	}
	else
	{
	   Directory_Entry_buf[14 + i * 2] = Entry_Name[pos++];
	   if(Entry_Name[pos-1] != 0xff)
	     Directory_Entry_buf[15 + i * 2] = 0;
	   else
         Directory_Entry_buf[15 + i * 2] = 0xff;
	}

  }
  Directory_Entry_buf[26] = 0;  //First cluster number (always 0x0000 for long filename records)
  Directory_Entry_buf[27] = 0;
  for(i = 0;i < 2;i++)
  {
	if((Entry_Name[pos] & 0x80) && (Entry_Name[pos] != 0xff))
	{    //get the unicode position in gb2312_to_unicode_table
		table_position = get_pos_in_table_about_unicode(Entry_Name[pos]*256 + Entry_Name[pos+1]);
	    Directory_Entry_buf[28 + i * 2] = gb2312_to_unicode_table[table_position];
		Directory_Entry_buf[29 + i * 2] = gb2312_to_unicode_table[table_position]>>8;
		pos+=2;
	}
	else
	{
	Directory_Entry_buf[28 + i * 2] = Entry_Name[pos++];
	if(Entry_Name[pos-1] != 0xff)
	  Directory_Entry_buf[29 + i * 2] = 0;
	else
      Directory_Entry_buf[29 + i * 2] = 0xff;
	}



  }
  return(SUCC);
}
/*
===============================================================================
函数 
完成填写一个SFN directory entry
入口：u8 * Directory_Entry_buf,u8 * Entry_Name,u32 first_cluster,
      u32 FileSize,u8 attr
出口：SUCC,FAIL
===============================================================================
*/
static u8 Fill_SFN_Directory_Entry(u8 * Directory_Entry_buf,u8 * Entry_Name,u32 first_cluster,u32 FileSize,u8 attr)
{
 u8 i,flag,j;
 for(i = 0;i < 11;i++)
   Directory_Entry_buf[i] = 0x20; 
  i = 0; 
  j = 0;
  flag = FILE_NAME;
  do{ //Directory_Entry分离取得Shortfile Entry name and file extension           
     if( ( * Entry_Name) != 0) 
       { 
        if( flag == FILE_NAME)
         {            
          if(*Entry_Name == '.')
             {
              flag  = FILE_EXTENSION;
			  Entry_Name++;
             }  
          else
            {
             Directory_Entry_buf[i] =  *Entry_Name;
             i++; 
             Entry_Name++;
			 if( i > 8 )
               break;
		  }
          }
        else if( flag  == FILE_EXTENSION)
         {
          if( j >= 3 )
            break;
           Directory_Entry_buf[8+j] =  *Entry_Name;
          j++;
          Entry_Name++;
         } 
      }
	  else
		 break;
 }while(1);

 Directory_Entry_buf[11] = attr;
 Directory_Entry_buf[20] = (u8)((first_cluster >> 16) & 0xff);
 Directory_Entry_buf[21] = (u8)((first_cluster >> 24) & 0xff);
 Directory_Entry_buf[12] = 0;
 Directory_Entry_buf[26] = (u8)(first_cluster & 0xff);  //填写首簇号
 Directory_Entry_buf[27] = (u8)((first_cluster >> 8) & 0xff);
 Directory_Entry_buf[28] = (u8)(FileSize & 0xff);////填写文件长度
 Directory_Entry_buf[29] = (u8)((FileSize >> 8) & 0xff);
 Directory_Entry_buf[30] = (u8)((FileSize >> 16) & 0xff);
 Directory_Entry_buf[31] = (u8)((FileSize >> 24) & 0xff);
 return(SUCC);
}
/*
===============================================================================
函数 
把Longfilename Directory Entry转换为short filename
入口：Directory_Entry:Directory_Entry name,attr:attr of Directory_Entry
出口：SUCC,FAIL
===============================================================================
*/
static u8 LFN_convert_to_SFN(u8 * Directory_Entry,u8 * SFN_Directory_Entry_buf)
{
 u8 i,flag,j;
 for(i = 0;i < 11;i++)
   SFN_Directory_Entry_buf[i] = 0x20; 
  i = 0; 
  j = 0;
  flag = FILE_NAME;
  do{ //Directory_Entry分离取得Shortfile Entry name and file extension           
     if( ( * Directory_Entry) != 0) 
       { 
        if( flag == FILE_NAME)
         {            
          if(*Directory_Entry == '.')
             {
              flag  = FILE_EXTENSION;
			  Directory_Entry++;
             }  
          else
            {
             SFN_Directory_Entry_buf[i] =  *Directory_Entry;
             i++; 
			 if(i >= 6)
			 {
			  SFN_Directory_Entry_buf[i] = '~';
              SFN_Directory_Entry_buf[i+1] = '1';
              do{ 
			      if(*Directory_Entry == '.' || ( * Directory_Entry) == 0 )	  
				  {
				    flag  = FILE_EXTENSION;
				    break;
				  }
				  else
                    Directory_Entry++;
			  }while(1);
              if(( * Directory_Entry) == 0)
				  return(SUCC);
			 }
             Directory_Entry++;
            }
          }
        else if( flag  == FILE_EXTENSION)
         {
          if( j >= 3 )
            return(FAIL);
          SFN_Directory_Entry_buf[8+j] =  *Directory_Entry;
          j++;
          Directory_Entry++;
         } 
      }
	  else
		  return(SUCC);
 }while(1);
}
/*
===============================================================================
函数 
为LONGFILENAME计算path中的起始位置
入口：
出口：SUCC,FAIL
===============================================================================
*/
u8 *determine_pos(u8 *Directory_Entry,u16 len)
{
  u8 pos,cur_pos,counter;
  pos = 0;
  cur_pos = 0;
  counter = 0;
  if(len == 0)
     return(Directory_Entry+pos);

  do{
	  if(Directory_Entry[pos]== 0)
	  {//printf("len = %ld path = %s pos=%d",len,Directory_Entry,pos);
	    return(Directory_Entry+pos);
	  }
     if(Directory_Entry[pos] & 0x80)
	 {
        pos+=2;
	 }
     else
	 { 
		 pos+=1;
	 }
	 cur_pos++;
	 if((cur_pos)==13)
	 {cur_pos = 0;
	  counter++;
	  if((counter)== len)
	   {//printf("xxxxlen = %ld path = %s pos=%d",len,Directory_Entry,pos);
	      return(Directory_Entry + pos);
	  }
	 }
  }while(1);

}
/*
===============================================================================
函数 
将Longfilename Directory Entry写入磁盘
入口：Directory_Entry:Directory_Entry name,attr:attr of Directory_Entry
出口：SUCC,FAIL
===============================================================================
*/
static u8 Write_LongFileName_Entry(u8 * Directory_Entry,u8 attr,u32 first_cluster,u8 * buf,u32 FileSize)
{
  u16 len,length;
  u8 i,checksum;
  u8 Directory_Entry_buf[32];
  u8 SN,LFN_record_FLAG;
  //计算需要多少个Directory_Entry空间，Directory_Entry每个空间为32个字节
  //计算结果放在len中
  Lengthof_CHN_String(Directory_Entry,&length);
  len = length / 13;
  if(length % 13)
	len++;
  
  //printf("s =%s",determine_pos(Directory_Entry,1));
  //scanf("%c",&i);
  SN = len;  //sequence number reset to 0
  //从long-filename directory entry处理得其相应的short-filename directory entry
  LFN_convert_to_SFN(Directory_Entry,Directory_Entry_buf);

  //计算short-filename directory entry的校验和
  checksum = calculate_checksum_longfilenameentry(Directory_Entry_buf);
  //处理last long-filename directory entry for file
  read_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);    
  LFN_record_FLAG = SN | (u8)Last_LFN_Record;
  Fill_LFN_Directory_Entry(buf + CORE[disk_id].offset,determine_pos(Directory_Entry,len-1),LFN_record_FLAG,checksum);
  //////////////printf("%s",Directory_Entry + (len - 1) * 13);
  len--;
  write_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
  CORE_offset_add_32(buf);
  LFN_record_FLAG = SN;
  //处理其它LFN directory entry
  //do{
  while(len){
   SN--;
   //Directory_Entry[len * 13] = 0;
   Fill_LFN_Directory_Entry(buf + CORE[disk_id].offset,determine_pos(Directory_Entry,len-1),SN,checksum);
   len--;
   write_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
   CORE_offset_add_32(buf);

  }

  
  //处理LFN 相应的short-filename directory entry
  Directory_Entry_buf[11] = attr;
  Directory_Entry_buf[12] = 0;


  Directory_Entry_buf[20] = (u8)((first_cluster >> 16) & 0xff);
  Directory_Entry_buf[21] = (u8)((first_cluster >> 24) & 0xff);
  Directory_Entry_buf[26] = (u8)(first_cluster & 0xff);  //填写首簇号
  Directory_Entry_buf[27] = (u8)((first_cluster >> 8) & 0xff);
  Directory_Entry_buf[28] = (u8)(FileSize & 0xff);////填写文件长度
  Directory_Entry_buf[29] = (u8)((FileSize >> 8) & 0xff);
  Directory_Entry_buf[30] = (u8)((FileSize >> 16) & 0xff);
  Directory_Entry_buf[31] = (u8)((FileSize >> 24) & 0xff); 
  for(i = 0;i < 32;i++)
	{ 
	  buf[CORE[disk_id].offset + i] = Directory_Entry_buf[i];
	}
  write_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
  #if enable_time_stamp_transaction
    write_time_stamp_direct(CORE[disk_id].SectorNum,CORE[disk_id].offset,MODIFY_TIME | CREATE_TIME | ACCESS_TIME);
  #endif
  return(SUCC);
}


/*
===============================================================================
函数 
添加一个Directory_Entry到目录
入口：Directory_Entry:Directory_Entry name,attr:attr of Directory_Entry
出口：SUCC,FAIL
===============================================================================
*/
static u8 Add_A_Directory_Entry_(u8 * Directory_Entry,u8 attr,u32 first_cluster,u32 FileSize)
{
	u16 len,i,length;
	u8 buf[512],flag;
	u8 Directory_Entry_buf[32];
	u8 temp;
    //给新建directory分配首簇，和写默认的两个目录，“.”和“..”
	if(attr & ATTR_DIRECTORY)
	{
	  u32 sector_local;
	  if( Allocate_An_Empty_cluster(&first_cluster,buf,NeedCLS)== FAIL)
	  	  return(FAIL);
	  //初始化两个默认目录，目录名分别命名为“.”和“..”
	  sector_local =  FirstSectorofCluster(first_cluster);
      for(i = 0;i < 11;i++)
		 buf[i] = 0x20; 
	  buf[0] = '.';//填写目录名“.”
	  buf[11] = attr;
	  buf[12] = 0;
	  buf[20] = (u8)((first_cluster >> 16) & 0xff);
	  buf[21] = (u8)((first_cluster >> 24) & 0xff);
	  buf[26] = (u8)(first_cluster & 0xff);  //填写当前目录簇号
	  buf[27] = (u8)((first_cluster >> 8) & 0xff);
	  buf[28] = 0;////填写文件长度
	  buf[29] = 0;
	  buf[30] = 0;
	  buf[31] = 0;
      for(i = 0;i < 11;i++)
		 buf[i + 32] = 0x20; 
	  buf[32] = '.';   //填写目录名“..”
	  buf[33] = '.';
	  buf[32+11] = attr;
	  buf[32+12] = 0;
	  buf[32+20] = (u8)((CORE[disk_id].ClusterNum  >> 16) & 0xff);
	  buf[32+21] = (u8)((CORE[disk_id].ClusterNum  >> 24) & 0xff);
	  buf[32+26] = (u8)(CORE[disk_id].ClusterNum & 0xff);  //填写当前目录父目录首簇号
	  buf[32+27] = (u8)((CORE[disk_id].ClusterNum >> 8) & 0xff);
	  buf[32+28] = 0;////填写文件长度
	  buf[32+29] = 0;
	  buf[32+30] = 0;
	  buf[32+31] = 0;
	  for(i = 64;i < 512;i++)
		  buf[i] = 0;
	  write_flash_sector(buf,sector_local,disk_id);
	}
	//计算需要多少个Directory_Entry空间，Directory_Entry每个空间为32个字节
	//计算结果放在len中	
	//len = LengthofString(Directory_Entry) / 13;
	//if(LengthofString(Directory_Entry) % 13)
	//	len++;
	///if(len > 1)
	//	len++;
  Lengthof_CHN_String(Directory_Entry,&length);
  len = length / 13;
  if(length % 13)
	len++;
	if(len == 1)
	{
	   u8 count;
	   count = 0;
	   do{
		   if(Directory_Entry[count] == 0 || Directory_Entry[count] == '.')
			   break;
		   count++;
	   }while(1);
	   if(count > 8)
		   len = 2;
	
	}
	flag = 0;
	//check for chn 
    for(i = 0;i<LengthofString(Directory_Entry);i++)
	{
	   if(Directory_Entry[i] & 0x80)
	   {
	      flag = 1;
		  break;
	   }
	}
	if(Seek_Space_to_Write_Directory_Entry(len,buf) == SUCC)
	{ 
	  if(len == 1 && !flag)  //短文件名写入
	  {
	    UPCASE(Directory_Entry);
	    Fill_SFN_Directory_Entry(Directory_Entry_buf,Directory_Entry, first_cluster,FileSize,attr);
		read_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
		for(temp = 0;temp < 32;temp++)
		{ 
		  buf[CORE[disk_id].offset + temp] = Directory_Entry_buf[temp];
		}
		write_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
	    #if enable_time_stamp_transaction
         write_time_stamp_direct(CORE[disk_id].SectorNum,CORE[disk_id].offset,MODIFY_TIME | CREATE_TIME | ACCESS_TIME);
        #endif

		return(SUCC);
	  } 
	  else//写长文件名
	  {   
	 	  return(Write_LongFileName_Entry(Directory_Entry,attr,first_cluster,buf,FileSize));
	  }
	}
	else
	 return(FAIL);
}

 /*
===============================================================================
函数 
建立文件
入口：无
出口：无
===============================================================================
*/  
#if complie_create_file
u8 create_file(u8 * filename)
{ 
 u16 len;
 u16 temp;
 u8 buf[260],status;
 	 
 
 enter_Mutex;
 stringcpy(filename,buf);

 CORE[0].FullPathType = FilePath;
 if(FullPathToSectorCluster(buf) != SUCC)  //check File is already exist?
 {  
     len = LengthofString(buf);
     temp = len - 1;
     do{ 
         if(buf[temp] =='\\')
		  {
	        buf[temp] = 0;
			temp++;
	        break;
		  }	
	      if(!temp)
  	         break;
	      temp--;
	 }while(1);
     CORE[0].FullPathType = DirectoryPath; 
	 if(!temp)
	 {
	   if(FullPathToSectorCluster(CORE[0].current_folder) == FAIL)
	  {release_Mutex;
	   return(FAIL);
	  }	
	 } 
	 else if(temp <= 3 && buf[1] == ':')
	 {
	  u8 TEMP[4];
	  TEMP[0] = buf[0];
	  TEMP[1] = buf[1];
	  TEMP[2] = '\\';
	  TEMP[3] = 0;

	  if(FullPathToSectorCluster(TEMP) == FAIL)
	  {
	   release_Mutex;
	   return(FAIL);
	  }		  
	 }
	 else 
	 {
	  if(FullPathToSectorCluster(buf) == FAIL)
	  {
	    release_Mutex;
		return(FAIL);
	  }
	 }
	 status = Add_A_Directory_Entry_(buf+temp,ATTR_ARCHIVE,0,0);
	 release_Mutex;
	 return(status);
  }
  else
  { 
    release_Mutex;
  return(FAIL);}
}
#endif
/*
===============================================================================
函数
建立目录
入口：foldername--目录的路径
出口：无
===============================================================================
*/
#if complie_create_floder
u8 create_floder(u8 * foldername)
{  
 u16 len;
 u16 temp;
 u8 TEMP[4];
 u8 buf[260],status;
 enter_Mutex;

 stringcpy(foldername,buf);

 CORE[0].FullPathType = DirectoryPath; 
 if(FullPathToSectorCluster(buf) != SUCC)  //folder is already existed
  {
     len = LengthofString(buf);
     temp = len - 1;
     do{
         if(buf[temp] =='\\')
		  {
	        buf[temp] = 0;
			temp ++;
	        break;
		  }	
	      if(!temp)
  	         break;
	      temp--;
	 }while(1);

     CORE[0].FullPathType = DirectoryPath; 
	 if( ! temp)
	 { CORE[0].FullPathType = DirectoryPath; 
	   if(FullPathToSectorCluster(CORE[0].current_folder) == FAIL)
	  {release_Mutex;
	   return(FAIL);
	  }
	 } 
	 else if(buf[1] == ':' && temp <= 3)
	 {
	  TEMP[0] = buf[0];
	  TEMP[1] = buf[1];
	  TEMP[2] = '\\';
	  TEMP[3] = 0;
	  CORE[0].FullPathType = DirectoryPath; 
	  if(FullPathToSectorCluster(TEMP) == FAIL)
	  {release_Mutex;
	   return(FAIL);
	  }
	 }
	 else if(FullPathToSectorCluster(buf) == FAIL)
	 { release_Mutex;
	   return(FAIL);
	  }
	 status = Add_A_Directory_Entry_(buf + temp,ATTR_DIRECTORY,0,0);
     release_Mutex;
	 return(status);
  }
  else
    {
	  release_Mutex;
	  return(FAIL);
	}

}
#endif 

/*
===============================================================================
函数
函数---设定当前操作位置
入口：无
出口：无
===============================================================================
*/ 
#if compile_fseek
u8 f_seek(u8 FCBsn, s32 offset, u8 origin)
{  

 enter_Mutex;   
 if(FCB[FCBsn].file_openned_flag == UnusedFlag)
    {
	 release_Mutex;
	 return(FAIL);
	}
    #if enable_time_stamp_transaction
      write_time_stamp(FCBsn,ACCESS_TIME);
   #endif
 switch(origin)
  { 
   case SEEK_SET:  //文件开始位置 
    { 
      if(offset < 0 || (u32)offset >= FCB[FCBsn].FileSize)
        {release_Mutex;
		 return(FAIL);}
      FCB[FCBsn].cur_position = offset; 
    #if EnableFileBuf
      Update_FCB_file_buffer(FCBsn); 
    #endif
      break;
    }
   case SEEK_CUR: //文件当前位置
    { 
      if(((FCB[FCBsn].cur_position + offset) < 0 )|| 
        ((FCB[FCBsn].cur_position + offset)  >= FCB[FCBsn].FileSize)) 
        {release_Mutex;
		 return(FAIL);}
      FCB[FCBsn].cur_position += offset;
     #if EnableFileBuf
      Update_FCB_file_buffer(FCBsn);
     #endif
      break;
    }
   case SEEK_END: //文件结束位置
    {
      if(offset > 0)
       {  release_Mutex;
	     return(FAIL);}
       FCB[FCBsn].cur_position = FCB[FCBsn].FileSize + offset;
      #if EnableFileBuf
       Update_FCB_file_buffer(FCBsn);
      #endif
      break;
    }
   default:release_Mutex;return(FAIL);
  }
 release_Mutex;
 return(SUCC);
}
#endif 
/*
===============================================================================
函数
读文件
入口：无
出口：无
===============================================================================
*/ 
#if complie_read_file
u16 read_file(u8 FCBsn,u8* buffer,u16 length)
{  
 #if EnableFileBuf
  #if Read_File_Optimization_Selector
   u16 OffsetInbuf,temp;
   u32 readed_bytes;
   u8 * point;
   u32 i;
   enter_Mutex;
   if(FCB[FCBsn].file_openned_flag == UnusedFlag)
     {release_Mutex;
	  return(0);
	 } 
   disk_id = FCB[FCBsn].disk;
   #if enable_time_stamp_transaction
      write_time_stamp(FCBsn,ACCESS_TIME);
   #endif
   readed_bytes = 0;
   if((FCB[FCBsn].cur_position + length) > FCB[FCBsn].FileSize)
   { 
     length = FCB[FCBsn].FileSize - FCB[FCBsn].cur_position ;
	 if(!length)
		 {
		  release_Mutex;
		  return(readed_bytes);
		 }
   }  
   OffsetInbuf = (FCB[FCBsn].cur_position % (FileBUFSize * TotalFileBUFsQTYeachFCB));
   if(!OffsetInbuf)
   {
     Update_FCB_file_buffer(FCBsn);//更新file buffer 
   }
   temp = FileBUFSize * TotalFileBUFsQTYeachFCB - OffsetInbuf;
   point =  FCB[FCBsn].FileBUF + OffsetInbuf;
   if(temp >= length)
   { 
     for(i = 0;i < length ;i++)
       buffer[i] = point[i];
     FCB[FCBsn].cur_position += length;
     release_Mutex;
	 return(i); 
   }
   else 
   { 
     for(i = 0;i < temp ;i++)
       buffer[i] = point[i];
     FCB[FCBsn].cur_position += temp;
	 readed_bytes += i;
	 buffer += temp;
     length -= temp;
   }   
   do{
      Update_FCB_file_buffer(FCBsn);
      if(length <= FileBUFSize * TotalFileBUFsQTYeachFCB)
	  {  
        for(i = 0;i < length ;i++)
          buffer[i] = FCB[FCBsn].FileBUF[i];
        FCB[FCBsn].cur_position += length;
	    readed_bytes += i; 
        release_Mutex;
		return(readed_bytes); 	 
	  }  
	  else
	  { 
        for(i = 0;i < FileBUFSize * TotalFileBUFsQTYeachFCB ;i++)
          buffer[i] = FCB[FCBsn].FileBUF[i];
        FCB[FCBsn].cur_position += FileBUFSize * TotalFileBUFsQTYeachFCB;
        length -= FileBUFSize * TotalFileBUFsQTYeachFCB;	
	    readed_bytes += FileBUFSize * TotalFileBUFsQTYeachFCB;
	    buffer += FileBUFSize * TotalFileBUFsQTYeachFCB;
	  }
   }while(1);
  #else
   u16 i,OffsetInbuf;
   if(FCB[FCBsn].file_openned_flag == UnusedFlag)
    {
	 release_Mutex;
	 return(0);
	}
   disk_id = FCB[FCBsn].disk;  
   #if enable_time_stamp_transaction
      write_time_stamp(FCBsn,ACCESS_TIME);
   #endif
   for(i = 0;i < length ;i++)
   {                                   
	  if(FCB[FCBsn].cur_position >= FCB[FCBsn].FileSize)
        {
		 release_Mutex;
		 return(i);
		}
	  OffsetInbuf = (u16)(FCB[FCBsn].cur_position % (FileBUFSize * TotalFileBUFsQTYeachFCB));
      if ( ! OffsetInbuf)
        Update_FCB_file_buffer(FCBsn);  
      buffer[i] = FCB[FCBsn].FileBUF [OffsetInbuf];
      FCB[FCBsn].cur_position ++;
   }
  release_Mutex;
  return(i); 
 #endif
#else
   release_Mutex;
   return(i);
#endif
}  
#endif
/*
===============================================================================
函数
写文件
入口：无
出口：无
===============================================================================
*/    
#if complie_write_file
u16 write_file(u8 FCBsn,u8* buffer, u16 length)
{  
  #if EnableFileBuf
   #if Write_File_Optimization_Selector
   u16 temp;
   u32 writed_bytes;
   u16 i,OffsetInbuf;
   u8 *point;
   enter_Mutex;
   disk_id = FCB[FCBsn].disk;
   #if enable_time_stamp_transaction
      write_time_stamp(FCBsn,ACCESS_TIME | MODIFY_TIME);
   #endif   
   if(FCB[FCBsn].file_openned_flag == UnusedFlag)
     {
	  release_Mutex;
	  return(0);
	 }
   if(FCB[FCBsn].Permission == ReadOnlyPermission || length == 0)
     {
	  release_Mutex;
	  return(0);
	 }  //只有文件的只读权限,不可写,直接退出
   writed_bytes = 0;
   OffsetInbuf = (FCB[FCBsn].cur_position % (FileBUFSize * TotalFileBUFsQTYeachFCB));
   if(!OffsetInbuf)
	  {  
         if(Update_FCB_file_buffer(FCBsn) == FAIL)  ////回写缓冲区至磁盘和更新file buffer
			 {
			   release_Mutex;
			   return(0);
			 }
	  } 
   temp = FileBUFSize * TotalFileBUFsQTYeachFCB - OffsetInbuf;
   if(temp >= length)  /*file buffer 有效空间 大于length,直接写file buffer,并回写磁盘	*/
   { point =  FCB[FCBsn].FileBUF + OffsetInbuf;
     for(i = 0;i < length ;i++)
       point[i] = buffer[i];
     FCB[FCBsn].cur_position += length;
	 FCB[FCBsn].Modified_Flag = 1;
	 if(FCB[FCBsn].cur_position >= FCB[FCBsn].FileSize)
		  FCB[FCBsn].FileSize = FCB[FCBsn].cur_position ;
     if(Writeback_FCB_file_buffer(FCBsn) == FAIL)  //回写缓冲区至磁盘
	    i = 0;
     release_Mutex;
     return(i);
   }
   else	 //len超出file buffer有效空间, 写FILE BUFFER,回写缓冲区至磁盘,下面继续写
   { point =  FCB[FCBsn].FileBUF + OffsetInbuf;
     for(i = 0;i < temp ;i++)
       point[i] = buffer[i];
	 FCB[FCBsn].Modified_Flag = 1;
     FCB[FCBsn].cur_position += temp;
	 writed_bytes += temp;
	 buffer+= temp;
     length -= temp;
	 if(FCB[FCBsn].cur_position >= FCB[FCBsn].FileSize)
		  FCB[FCBsn].FileSize = FCB[FCBsn].cur_position; 
   }
   do{

     if(Update_FCB_file_buffer(FCBsn) == FAIL)  ////回写缓冲区至磁盘和更新file buffer
	    {
		 release_Mutex;
		 return(writed_bytes);
		}
     if(length <= FileBUFSize * TotalFileBUFsQTYeachFCB)
	 {//写最后的BUF数据到file buffer
      for(i = 0;i < length ;i++)
        FCB[FCBsn].FileBUF [i] = buffer[i];
      FCB[FCBsn].cur_position += length;
	  FCB[FCBsn].Modified_Flag = 1; 
	  if(FCB[FCBsn].cur_position >= FCB[FCBsn].FileSize)
		  FCB[FCBsn].FileSize = FCB[FCBsn].cur_position ;
      if(Writeback_FCB_file_buffer(FCBsn) == FAIL)  //回写缓冲区至磁盘
	    i = 0;
	  writed_bytes += i;
	  release_Mutex;
 
      return(writed_bytes);	//write_file 结束 
	 }
	 else
	 {//len>filebuffer 有效空间, 写FILE BUFFER,回写缓冲区至磁盘,下面继续写
      for(i = 0;i < FileBUFSize * TotalFileBUFsQTYeachFCB ;i++)
        FCB[FCBsn].FileBUF [i] = buffer[i];
	  FCB[FCBsn].Modified_Flag = 1;
      FCB[FCBsn].cur_position += FileBUFSize * TotalFileBUFsQTYeachFCB;
      length -= FileBUFSize * TotalFileBUFsQTYeachFCB;	
	  writed_bytes += FileBUFSize * TotalFileBUFsQTYeachFCB;
	  buffer += FileBUFSize * TotalFileBUFsQTYeachFCB;
	  if(FCB[FCBsn].cur_position >= FCB[FCBsn].FileSize)
		  FCB[FCBsn].FileSize = FCB[FCBsn].cur_position ;
	 }
   }while(1); 
  #else 
   u16 i,OffsetInbuf;
   if(FCB[FCBsn].file_openned_flag == UnusedFlag)
     {release_Mutex;
	  return(0);
	 }
   if(FCB[FCBsn].Permission == ReadOnlyPermission)
	 {release_Mutex;
	  return(0);
	}
   disk_id = FCB[FCBsn].disk;
   #if enable_time_stamp_transaction
      write_time_stamp(FCBsn,ACCESS_TIME | MODIFY_TIME);
   #endif
   for(i = 0;i < length ;i++)
   {                                    
    OffsetInbuf = (u16)(FCB[FCBsn].cur_position % (FileBUFSize * TotalFileBUFsQTYeachFCB));
	if(!OffsetInbuf)
	  { 
         if(Update_FCB_file_buffer(FCBsn) == FAIL)  ////回写缓冲区至磁盘和更新file buffer
			 {
			  release_Mutex;
			  return(0);
			 }
	  } 
      FCB[FCBsn].FileBUF [OffsetInbuf] = buffer[i];
	  if(!FCB[FCBsn].Modified_Flag)
	     FCB[FCBsn].Modified_Flag = 1 ;    
      if(FCB[FCBsn].cur_position >= FCB[FCBsn].FileSize)
		  FCB[FCBsn].FileSize = FCB[FCBsn].cur_position + 1;
	  FCB[FCBsn].cur_position ++;
   }
   if(Writeback_FCB_file_buffer(FCBsn) == FAIL)  //回写缓冲区至磁盘
	   i = 0;
   release_Mutex;
   return(i);
  #endif
 #else
  release_Mutex;
  return(i);
 #endif
}   
 #endif
/*
===============================================================================
函数
进入目录--更新当前目录
入口：foldername:目录名，
      mode: 0-- 进入子目录; >0--返回上一层目录
出口：SUCC,FAIL
===============================================================================
*/
#if compile_cd_folder
u8 cd_folder(u8 *foldername,u8 mode)
{ 
  u16 offset;
  enter_Mutex;
  if(mode)  //返回上一层目录
   {
    if (CORE[0].CurrentDirectoryType == RootDirectory)
     {
	   release_Mutex;
	   return(0x55);
	 }
    else
    {
	 CORE[0].FullPathType = DirectoryPath;
     if(FullPathToSectorCluster(CORE[0].current_folder) != SUCC)
	   {
	    release_Mutex;
	    return(FAIL);
	   }
     offset = LengthofString(CORE[0].current_folder);
     offset --;
     do{
      if(CORE[0].current_folder[offset] != '\\')
		  CORE[0].current_folder[offset] = 0;
      else
      {
	   if(CORE[0].current_folder[offset-1] == ':')
		 break;
       CORE[0].current_folder[offset] = 0;
       break;
      }
       offset--;
     }while(1);        

     if(LengthofString(CORE[0].current_folder) <= 3)
       CORE[0].CurrentDirectoryType = RootDirectory;
	 else
       CORE[0].CurrentDirectoryType = NoneRootDirectory;
     release_Mutex;
	 return(SUCC);
    }
   }
   else  //进入子目录
   {
    CORE[0].FullPathType = DirectoryPath; 
    if(FullPathToSectorCluster(foldername) == SUCC)
	{ 
	  if(((* foldername) >= 'C' && ( * foldername ) <= 'Z')
		  || ((* foldername) >= 'c' && ( * foldername ) <= 'z'))
	  {
       if(* (foldername + 1) == ':' &&  * (foldername + 2 ) == '\\')
		  {
           stringcpy(foldername,CORE[0].current_folder);  
		  }
		  else
		  { 
			if(LengthofString(CORE[0].current_folder) != 3)
			{
			 CORE[0].current_folder[LengthofString(CORE[0].current_folder)] = '\\';
             CORE[0].current_folder[LengthofString(CORE[0].current_folder) + 1] = 0;
			}
		    concanenateString(CORE[0].current_folder,foldername);
		  }
	  }
	  else if(*foldername == '\\')
	  {
	      stringcpy(foldername,CORE[0].current_folder + 1);
	  }
	 if(LengthofString(CORE[0].current_folder) <= 3)
	 {
      CORE[disk_id].SectorNum = CORE[disk_id].FirstRootDirSecNum; 
	  CORE[0].CurrentDirectoryType = RootDirectory; 
	 }
	 else
	 {
	   CORE[0].CurrentDirectoryType = NoneRootDirectory;
       CORE[disk_id].ClusterNum = CORE[disk_id].ClusterOfDirectoryEntry;
	   CORE[disk_id].ClusterNOofCurrentFolder = CORE[disk_id].ClusterOfDirectoryEntry;
       CORE[disk_id].SectorNum =  FirstSectorofCluster(CORE[disk_id].ClusterNum); 
	 }
	 CORE[disk_id].offset = 0;
	 release_Mutex;
	 return(SUCC);


	}   
   }
   release_Mutex;
   return(FAIL);
}
#endif 

/*
===============================================================================
函数
清除长文件名directory entry
入口：buf
出口：SUCC,FAIL
===============================================================================
*/ 
static u8 Remove_LONGFILENAME(u8 *buf,u8 *folder_filename)
{ 
  //struct Directory_Entry_  *Directory_Entry_Local;  
  u32 Cluster,Sector;
  u16 offset;//,length;
  u8 NextDirectoryEntryEMPTY; 
  u8 First_Entry_Deleted;
  Cluster = CORE[disk_id].ClusterNum;
  Sector = CORE[disk_id].SectorNum;
  offset = CORE[disk_id].offset;
  First_Entry_Deleted = 0;
  read_flash_sector(buf,Sector,disk_id);
  if(!buf[CORE[disk_id].offset])
   NextDirectoryEntryEMPTY = 1;
  else
   NextDirectoryEntryEMPTY = 0;  
  do{
    if(offset == 0)
    { 
      write_flash_sector(buf,Sector,disk_id);
      Sector--; 
      offset = 512 - 32;
      if(CORE[disk_id].DirectoryType == RootDirectory)
	  {  
		  if(Sector < CORE[disk_id].FirstRootDirSecNum)
	      return(SUCC); 
	  }        
      else if (Sector  < FirstSectorofCluster(Cluster))
	  { 

         Cluster = Get_Previous_Cluster_From_Current_Cluster(Cluster);
         Sector = FirstSectorofCluster(Cluster) ;
		 Sector += BPB[disk_id].sector_per_cluster - 1;
       } 
      read_flash_sector(buf,Sector,disk_id);     
    }
	else
	{
	  offset -= 32;
	}

    if(buf[offset + 11] == ATTR_LONG_NAME)
    {
	  if(NextDirectoryEntryEMPTY)
        buf[offset] = 0x0;
	  else
        buf[offset] = 0xe5;
    }
    else
	{
		if(First_Entry_Deleted == 0)
		{
	      if(NextDirectoryEntryEMPTY)
           buf[offset] = 0x0;
	      else
           buf[offset] = 0xe5;
		 First_Entry_Deleted = 1;
		 
		}
	    else
		{
		 write_flash_sector(buf,Sector,disk_id);
		 return(SUCC);
		}
	}
  }while(1);
//  return(SUCC);
}
/*
===============================================================================
函数
删除文件
入口：filename:文件路径名
出口：SUCC,FAIL
===============================================================================
*/ 
#if complie_delete_file
u8 delete_file( u8 *filename)
{ 
 u8 buf[512];
 enter_Mutex;
 CORE[0].FullPathType = FilePath; 
 if(FullPathToSectorCluster(filename) == SUCC)
    {
	 if(Check_FileOpened_Status(CORE[disk_id].ClusterOfDirectoryEntry,0xff,disk_id) == FileAlreadyopenedByOtherHandle)
         {
		  release_Mutex;
		  return(FAIL);
		 }
     if(Remove_LONGFILENAME(buf,filename) != SUCC)
         {
		  release_Mutex;
		  return(FAIL);
		 }
	 if(CORE[disk_id].ClusterOfDirectoryEntry)
       FreeClusterChain(CORE[disk_id].ClusterOfDirectoryEntry); 
     release_Mutex;
	 return(SUCC); 
   }
 else
   { 
     release_Mutex;
     return(FAIL);
   }
}
#endif
/*
===============================================================================
函数
删除目录
入口：char * foldername--路径+目录名
出口：无
===============================================================================
*/    
#if complie_delete_folder
u8 delete_folder(u8 * foldername)
{
 u8 buf[512];
 u32 sector;
 enter_Mutex;
 CORE[0].FullPathType = DirectoryPath; 
 if(FullPathToSectorCluster(foldername) == SUCC)
   {
     sector = FirstSectorofCluster(CORE[disk_id].ClusterOfDirectoryEntry);
	 read_flash_sector(buf,sector,disk_id);
     if(buf[64] != 0)  //不能删除非空目录
	 {
	  release_Mutex;
	  return(FAIL); 
	 }
	 Remove_LONGFILENAME(buf,foldername);
     FreeClusterChain(CORE[disk_id].ClusterOfDirectoryEntry);
     release_Mutex;
	 return(SUCC);
   }
  else
    { release_Mutex;
	  return(FAIL);
	}
     
} 
#endif
/*
===============================================================================
函数    
重命名文件
入口：oldfilename:指向旧文件名，newfilename:指向新文件名(新文件名)
出口：SUCC,FAIL
===============================================================================
*/  
#if complie_rename_file
u8 rename_file(u8 * oldfilename1,u8 * newfilename1)
{  
 u16 len,temp; 
 u8 buf[512];
 u32 first_cluster;
 u32 FileSize;  
 u8 buff1[260],buff[260], *newfilename,temp2,position,*oldfilename;
 enter_Mutex;
 stringcpy(newfilename1,buff);
 stringcpy(oldfilename1,buff1);
 newfilename = buff;
 oldfilename = buff1;
 UPCASE(oldfilename);
 UPCASE(newfilename);

 CORE[0].FullPathType = FilePath; 
 if(FullPathToSectorCluster(oldfilename) == SUCC)
 {  //第一个参数是文件的首簇号，第2个参数是文件对应的FCBsn,未分配FCB的文件使用0xff替代
	if(Check_FileOpened_Status(CORE[disk_id].ClusterOfDirectoryEntry,0xff,disk_id) == FileAlreadyopenedByOtherHandle)
           {
		   	 release_Mutex;
		     return(FAIL); }
		    //不能删除已打开文件
	//首簇号,FileSize保存
	FileSize = CORE[disk_id].FileSize;
	first_cluster = CORE[disk_id].ClusterOfDirectoryEntry;
    len = LengthofString(newfilename);
    temp = len - 1;
     do{
         if(newfilename[temp] =='\\')
		  {
			temp++;
	        break;
		  }	
	      if(!temp)
  	         break;
	      temp--;
	 }while(1);
	temp2 = 0;
	position = 0;
	do{
	    if(oldfilename[temp2] == 0)
		{
			temp2 = position;
		    break;
		}  
		if(oldfilename[temp2] == '\\')
			position = temp2;
	 temp2++;
	}while(1);

	/*  检查新文件可能有的重复情况,发现重复的名字则直接退出函数 */
	if(temp2 == 0)
	{
	  len = LengthofString(CORE[0].current_folder);
	  stringcpy(CORE[0].current_folder,oldfilename);
	  if(len > 3)
	  {
	     oldfilename[len] = '\\';  
		 oldfilename[len+1] = 0;
	  }
      concanenateString(oldfilename,newfilename + temp);
      if(FullPathToSectorCluster(oldfilename) == SUCC)
	     {
		  release_Mutex;
		  return(FAIL);}
	}
	else
	{
	  oldfilename[temp2 + 1] = 0;
      concanenateString(oldfilename,newfilename + temp);
      if(FullPathToSectorCluster(oldfilename) == SUCC)
	      {
		   release_Mutex;
		   return(FAIL);}

	}

    stringcpy(oldfilename1,oldfilename);
	CORE[0].FullPathType = FilePath; 
    FullPathToSectorCluster(oldfilename);
    Remove_LONGFILENAME(buf,oldfilename);//删除目录项，簇链不变化

    if(temp2 == 0)
	{ 
	  CORE[0].FullPathType = DirectoryPath; 
      FullPathToSectorCluster(CORE[0].current_folder); 
	}
	else
	{
		if(temp2 < 3 && oldfilename[1] == ':')
			oldfilename[temp2 + 1] = 0;
		else
            oldfilename[temp2] = 0;
		CORE[0].FullPathType = DirectoryPath;
        FullPathToSectorCluster(oldfilename);

	}
		    //写入新文件目录项，完成改名
    Add_A_Directory_Entry_(newfilename + temp,ATTR_ARCHIVE,first_cluster,FileSize);
    release_Mutex;
	return(SUCC);
  }
 else{
   release_Mutex;
   return(FAIL);
   }
}   
#endif

/*
===============================================================================
函数
cd_folder_for_disk_enum(for disk enumeration)
入口：无
出口：SUCC,FAIL
===============================================================================
*/
static u8 cd__folder_for_disk_enum()
{

     CORE[0].FullPathType = DirectoryPath;
     if(FullPathToSectorCluster(CORE[0].current_folder_for_disk_enum) != SUCC)
	      return(FAIL);

	 if(LengthofString(CORE[0].current_folder_for_disk_enum) <= 3)
	 {
      CORE[disk_id].SectorNum = CORE[disk_id].FirstRootDirSecNum; 
	  CORE[0].CurrentDirectoryType = RootDirectory; 
	 }
	 else
	 {
	   CORE[0].CurrentDirectoryType = NoneRootDirectory;
       CORE[disk_id].ClusterNum = CORE[disk_id].ClusterOfDirectoryEntry;
	   CORE[disk_id].ClusterNOofCurrentFolder = CORE[disk_id].ClusterOfDirectoryEntry;
       CORE[disk_id].SectorNum =  FirstSectorofCluster(CORE[disk_id].ClusterNum); 
	 }
	 CORE[disk_id].offset = 0;

	 return(SUCC);              
}

/*
===============================================================================
函数
cd..(for disk enumeration)
入口：无
出口：SUCC,FAIL
===============================================================================
*/ 
static u8 cd__() 
{ 
  u16 offset;
  u8 entry[256],extension[20],folderSplit[256],i;
  i = 254;
  folderSplit[i] = 0;
  i --;

     offset = LengthofString(CORE[0].current_folder_for_disk_enum);
     offset --;
     do{
      if(CORE[0].current_folder_for_disk_enum[offset] != '\\')
	  {
		  folderSplit[i] = CORE[0].current_folder_for_disk_enum[offset];
		  CORE[0].current_folder_for_disk_enum[offset] = 0;
	  }
      else
      {
	   if(CORE[0].current_folder_for_disk_enum[offset-1] == ':')
		 break;
       CORE[0].current_folder_for_disk_enum[offset] = 0;
       break;
      }
       offset--;
	   i--;
     }while(1);     
 
	 cd__folder_for_disk_enum();
	 
	 do{  
          if(GetEntryFromDirectory(entry,extension,Get_All_Entries) == FAIL)
		  {
		     if(CORE[0].CurrentDirectoryType == RootDirectory)
				 return(0x55);
		  }

	
          if(!(CORE[0].Entry_Attr & ATTR_DIRECTORY))
			  continue;

		  if(stringcmp(entry,folderSplit+i+1) == SUCC)
			  break;
	 }while(1);

     if(LengthofString(CORE[0].current_folder_for_disk_enum) <= 3)
       CORE[0].CurrentDirectoryType = RootDirectory;
	 else
       CORE[0].CurrentDirectoryType = NoneRootDirectory;
     return(SUCC);
}
/*
===============================================================================
函数
保存ENUM变量
入口：无
出口：无
===============================================================================
*/
save_enum_vars(u8 disk_or_folder)
{
   //保存Enumerated Directory Entry所在Directory的ClusterNum,SectorNum,offset
   CORE[disk_id].DIR_ENUM_ClusterNum[disk_or_folder] = CORE[disk_id].ClusterNum;  
   CORE[disk_id].DIR_ENUM_SectorNum[disk_or_folder] =CORE[disk_id].SectorNum ;
   CORE[disk_id].DIR_ENUM_offset[disk_or_folder] = CORE[disk_id].offset;
   //保存Enumerated Directory Entry所在Directory的ClusterNum,SectorNum,offset
   CORE[disk_id].DIR_ENUM_ClusterOfDirectoryEntry[disk_or_folder] = CORE[disk_id].ClusterOfDirectoryEntry;
   CORE[disk_id].DIR_ENUM_DirectoryType[disk_or_folder] = CORE[disk_id].DirectoryType; 
   CORE[disk_id].DIR_ENUM_FullPathType[disk_or_folder] = CORE[disk_id].FullPathType;
   CORE[disk_id].DIR_ENUM_CurPathType[disk_or_folder] = CORE[disk_id].CurPathType; 

}

/*
===============================================================================
函数
恢复ENUM变量
入口：无
出口：无
===============================================================================
*/
restore_enum_vars(u8 disk_or_folder)
{
   //恢复Enumerated Directory Entry所在Directory的ClusterNum,SectorNum,offset
   CORE[disk_id].ClusterNum = CORE[disk_id].DIR_ENUM_ClusterNum[disk_or_folder];   
   CORE[disk_id].SectorNum = CORE[disk_id].DIR_ENUM_SectorNum[disk_or_folder];
   CORE[disk_id].offset = CORE[disk_id].DIR_ENUM_offset[disk_or_folder];
   //恢复Directory Entry32字节中对应first Cluster Num
   CORE[disk_id].ClusterOfDirectoryEntry = CORE[disk_id].DIR_ENUM_ClusterOfDirectoryEntry[disk_or_folder];  
   CORE[disk_id].DirectoryType = CORE[disk_id].DIR_ENUM_DirectoryType[disk_or_folder]; 
   CORE[disk_id].FullPathType = CORE[disk_id].DIR_ENUM_FullPathType[disk_or_folder];
   CORE[disk_id].CurPathType = CORE[disk_id].DIR_ENUM_CurPathType[disk_or_folder]; 

}
/*
===============================================================================
函数
列举当前目录下的所有文件和目录
入口：无
出口：无
===============================================================================
*/  
#if complie_folder_dir
u8 folder_enumeration(u8 *return_string,u8 mode,u8 *ATTR)
{ 
  u8  Extension[20];
  u16 temp;
  enter_Mutex;
  if(mode == 0x0)
  {
   CORE[0].FullPathType = DirectoryPath; 
   if(cd_folder(CORE[0].current_folder,0) != SUCC)
	   {
	    release_Mutex;
		return(FAIL);}
   CORE[disk_id].offset = 0;
   if(CORE[0].CurrentDirectoryType ==  RootDirectory)
      CORE[disk_id].SectorNum = CORE[disk_id].FirstRootDirSecNum; 
   else
     {
       CORE[disk_id].ClusterNum = CORE[disk_id].ClusterNOofCurrentFolder;
       CORE[disk_id].SectorNum = FirstSectorofCluster(CORE[disk_id].ClusterNum); 
     }
     save_enum_vars(FOLDER_ENUM);
  } 

  restore_enum_vars(FOLDER_ENUM);
do{
  stringcpy(CORE[0].current_folder,return_string);
  temp = LengthofString(return_string);
  if(return_string[temp-1] != '\\')
  {
   return_string[temp] = '\\';
   return_string[temp+1] = 0;
  }
  
  if(GetEntryFromDirectory(return_string+LengthofString
	  (return_string), Extension,Get_All_Entries) == SUCC)
  {
   temp = LengthofString(return_string);
   *ATTR = CORE[0].Entry_Attr;
   if(temp > 0 && (!((*ATTR) & ATTR_DIRECTORY)))
   {
    if(Extension[0] != 0)
	{
     return_string[temp] = '.';
     return_string[temp+1] = 0;
	 concanenateString(return_string,Extension);
	}
   }

   //进滤"." ".."
  if(return_string[temp - 1] == '.' || (return_string[temp - 1] == '.'
	  && return_string[temp - 2] == '.'))
	  continue;
   save_enum_vars(FOLDER_ENUM);
   release_Mutex;
   return(SUCC);
  }
  else
   break;
 }while(1);
 release_Mutex;
 return(FAIL);
} 
#endif
/*
===============================================================================
函数
列举disk中所有文件和目录   
入口：无
出口：无
===============================================================================
*/ 
u8 disk_enumeration(u8 disk_,u8 *return_string,u8 mode,u8* ATTR)
{
 u16 temp; 
 u8 root_[] = "c:\\";
 u8 Extension[100];
 enter_Mutex;
 disk_id = disk_;
 
 if( ! mode)
 {  
   root_[0] += disk_id;
   stringcpy(root_,CORE[0].current_folder_for_disk_enum);
   cd__folder_for_disk_enum();
   save_enum_vars(DISK_ENUM);  
 }
 
   restore_enum_vars(DISK_ENUM);  
 do{
    stringcpy(CORE[0].current_folder_for_disk_enum,return_string);
    temp = LengthofString(return_string);

    if(return_string[temp-1] != '\\')
	  {
         return_string[temp] = '\\';
         return_string[temp+1] = 0;
	  }
     if(GetEntryFromDirectory(return_string + LengthofString(return_string), 
	    Extension,Get_All_Entries) == FAIL)
	 {    
	     //目录返回
		 if(cd__() == 0x55)
		 {
             release_Mutex;
		     return(FAIL);
		 }
         continue;
  
	  }	
	 else{  
	       temp = LengthofString(return_string);
           *ATTR = CORE[0].Entry_Attr; 
        #if filter_hidden_entry
		   if(*ATTR & ATTR_HIDDEN)
			   continue;
        #endif

           if(temp > 0 && (!((*ATTR) & ATTR_DIRECTORY)))
		   {
              if(Extension[0] != 0)
			  {
                  return_string[temp] = '.';
                  return_string[temp + 1] = 0;
	              concanenateString(return_string,Extension);
			  }

		   }
		   else{  temp = LengthofString(return_string);
		          //进滤"." ".."
		          if(return_string[temp - 1] == '.' || (return_string[temp - 1] == '.'
				  	  && return_string[temp - 2] == '.'))
					  continue;
				  stringcpy(return_string,CORE[0].current_folder_for_disk_enum);
                  cd__folder_for_disk_enum();

		   }
	  }
	 
   save_enum_vars(DISK_ENUM);
   release_Mutex;
   return(SUCC);
 }while(1);
  
}
/*
===============================================================================
函数
文件查找函数
入口：1）mode = 0：在当前目录下查找；2）mode=1：在整个磁盘中查找
      filename）被查找文件,Return_string
出口：无
===============================================================================
*/ 
#if complie_find_file
u8 find_file(u8 * filename,u8 mode,u8* Return_string)
{   
	u16 temp,POINTER;
	u8 ATTR,disk_enum_mode;
	enter_Mutex;
    if(! mode)
	{
      stringcpy(CORE[0].current_folder,Return_string);
      temp = LengthofString(Return_string);
	  if(*(Return_string + temp - 1) != '\\')
	  {
	    Return_string[temp] = '\\';
		Return_string[temp + 1] = 0;
	  }
      concanenateString(Return_string,filename);
     CORE[0].FullPathType = FilePath;
     if(FullPathToSectorCluster(Return_string) != SUCC)
	      {  
		     release_Mutex;
		     return(FAIL);
		   }
     release_Mutex;
	 return(SUCC);
	
	}
	else
	{
     disk_enum_mode = 0;
     while(disk_enumeration(0,Return_string,disk_enum_mode,&ATTR) == SUCC)
	 {  
	    disk_enum_mode = 1;
		temp = 0;
		POINTER = 0;
		if(ATTR & ATTR_DIRECTORY)
			continue;
        do{
		  if(Return_string[temp] == '\\')
			  POINTER = temp;
		  if(Return_string[temp] == 0)
			  break;
		  temp++;
		}while(1);

		if(stringcmp(Return_string + POINTER + 1,filename) == SUCC)
			{
			 release_Mutex;
			 return(SUCC);
		    }
	 }
	 release_Mutex; 
     return(FAIL);	
	}
}
#endif
/*
===============================================================================
函数
查询分区容量和剩余空间 --通过检查分区FAT表来实现
入口：partition_id(Supported ID:form 'C' to 'F'),u32 *volume_capacity, u32 *volume_free_space
出口：SUCC  (返回的分区容量和剩余空间以512 bytes扇区为单位)
===============================================================================
*/  
#if complie_volume_inquiry 
u8 volume_inquiry(u8 partition_id,u32 *volume_capacity, u32 *volume_free_space)
{   
  u16 i,j,x;
  u8 buf[512];
  enter_Mutex;
  if(partition_id >= 'c' && partition_id <= 'f')
       partition_id -= 32;   //convert to upper case
  if ( ! (partition_id >= 'C' && partition_id <= 'F'))
    {
	 release_Mutex;
	 return(FAIL);
	}
  disk_id = partition_id - 'C'; 
  
     *volume_free_space = 0;
	 i = 0;
	 x = 0;
	 while(i < (CORE[disk_id].CountofClusters)){
      if(read_flash_sector(buf,CORE[disk_id].FirstSectorofFAT1 + x,disk_id) == SUCC) 
       {
		if(CORE[disk_id].fs_type == FAT16)
        {
          for (j = 0;j < 512;j +=2)
		  { 
            if(buf[j] == 0 && buf[j + 1] == 0)
               (*volume_free_space) ++;
		    i++;
		    if(i >= (CORE[disk_id].CountofClusters))
			   break;
          }
		  x++;
		}
		else if (CORE[disk_id].fs_type == FAT32)
		{
		  for (j = 0;j < 512;j +=4)
		  { 
            if(buf[j] == 0 && buf[j + 1] == 0 && buf[j+2] == 0 && buf[j + 3] == 0)
               (*volume_free_space) ++;
		    i++;
		    if(i >= (CORE[disk_id].CountofClusters))
			   break;
          }
		  x++;
		
		}
		else
		 break;
      }
      else
      {
        *volume_capacity = 0;
        *volume_free_space = 0;
		release_Mutex;
        return(FAIL);
      } 
	 }
       *volume_free_space *=  BPB[disk_id].sector_per_cluster;
       *volume_capacity =  CORE[disk_id].total_sector; 
	   release_Mutex; 
       return(SUCC);

}
#endif

