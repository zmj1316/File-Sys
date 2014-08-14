/*
+FHDR------------------------------------------------------------------
Copyright (c),
Tony Yang –specialized in fat usb firmware development  
Contact:qq 292942278  e-mail:tony_yang123@sina.com.cn
;;;;;;;;;;
Abstract:
$Id: fat.c,v 1.14 2007/05/11 03:00:55 design Exp $
-FHDR-------------------------------------------------------------------
*/      
#include<stdio.h>
#include<include\types.h>
#include<Flash_Management\Flash_Management.h>   
#include<include\FAT_cfg.h>  
#include<include\gb2312_to_unicode_table.h>
#include"time.h"                    
//Current Directory Entry 
static struct Directory_Entry_ Directory_Entry;
//CORE of FAT filesystem
static struct core_ CORE[maximum_disks];
//BPB
static struct partition_bpb BPB[maximum_disks]; 
//Define FCBs for FileRead/Write  
struct FileControlBlock FCB[MaximumFCB];


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
  read_flash_sector(buf,Entry_Storedin_Sector,disk_id);
  get_time(date_time);

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
  read_flash_sector(buf,Entry_Storedin_Sector,disk_id);

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
*/ /*
static u32 Get_Next_Cluster_From_Current_Cluster(u32 Cluster)
{
   u32 ThisFATSecNum,ThisFATEntOffset; 
   u8 *buf;
   //FAT16
   if(CORE[disk_id].fs_type == FAT16)
     ThisFATEntOffset = Cluster * 2;
   else
	 ThisFATEntOffset = Cluster * 4;
    buf = CORE[disk_id].cluster_buffer;
   ThisFATSecNum = CORE[disk_id].relative_sector + BPB[disk_id].reserved_sector 
                   + (ThisFATEntOffset / BPB[disk_id].bytes_per_sector);
   ThisFATEntOffset = ThisFATEntOffset % BPB[disk_id].bytes_per_sector; 
   if(CORE[disk_id].cluster_buffer_sector_no != ThisFATSecNum)
   {
     CORE[disk_id].cluster_buffer_sector_no = ThisFATSecNum;
     read_flash_sector(buf,CORE[disk_id].cluster_buffer_sector_no,disk_id);
   }
  
   if(CORE[disk_id].fs_type == FAT16)
     return((u32)buf[ThisFATEntOffset] +
	       ((u32)buf[ThisFATEntOffset + 1]) * 256);
   else
     return((u32)buf[ThisFATEntOffset] + 
	 ((u32)buf[ThisFATEntOffset + 1]) * 256
	 +((u32)buf[ThisFATEntOffset+2])*256*256
	 +((u32)buf[ThisFATEntOffset + 3]) * 256*256*256);
}
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
     {
		 if( (u16)(buf[CUR_FATEntOffset] + buf[CUR_FATEntOffset + 1] * 256) == (u16)Cluster)
          return(CUR_Checking_Cluster_No);
     }
	 else
	 {   if((u32) (buf[CUR_FATEntOffset] + buf[CUR_FATEntOffset + 1] * 256
		   + buf[CUR_FATEntOffset + 2] * 256*256 + + buf[CUR_FATEntOffset + 3] * 256 * 256 *256
		   ) == Cluster)
          return(CUR_Checking_Cluster_No);	   
	 }
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
static void BPB_INIT_1(u8 partition_ID,u8 *buf)
{
  //read_flash_sector(buf,0); //read MBR 
  CORE[disk_id].relative_sector = 0;
  CORE[disk_id].total_sector =  buf[32]+buf[33]*256+buf[34]*256*256+buf[35]*256*256*256;
  CORE[disk_id].system_id = buf[0x1c2]; //Partition Type 0C-FAT32,06-FAT16 ect..
  CORE[disk_id].PartitionID= 'C' + partition_ID; //从C开始到Z结束      
}  
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
	  //printf("disk is = %d fatype=fat32,counter of cluster = %d",disk_id,CORE[disk_id].CountofClusters);
  }
  else
  {  	  //printf("disk is = %d fatype=fat16,counter of cluster = %d",disk_id,CORE[disk_id].CountofClusters);
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
  u8 buf[512]; 
   disk_id = partition_ID;
   read_flash_sector(buf,0,disk_id); //read MBR 
   if ( buf[0x1be] == 0x00 || buf[0x1be] == 0x80) // check boot indicator 00 or 0x80
   {  
       CORE[partition_ID].relative_sector = *((u32*) (buf+454));
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
//////////////printf("\ntotal_sector =%d ",CORE[partition_ID].total_sector );
  //u16 bytes_per_sector;//每扇区字节数
BPB[partition_ID].bytes_per_sector = buf[0xb] + buf[0xc] * 256;
//////////////printf("\nbytes_per_sector =%d ",BPB[partition_ID].bytes_per_sector);
//  u8 sector_per_cluster; //每簇扇区数
BPB[partition_ID].sector_per_cluster = buf[0xd];

//////////printf("BPB[partition_ID].sector_per_cluster = %d",BPB[partition_ID].sector_per_cluster);
//////////////printf("\nsector_per_cluster=%d ",BPB[partition_ID].sector_per_cluster);
//  u16 reserved_sector;  //保留扇区数
BPB[partition_ID].reserved_sector = buf[14] + buf[15] * 256;
//////////////printf("\nreserved_sector=%d ",BPB[partition_ID].reserved_sector);
//  u8 numbers_of_FAT;//FAT副本数
BPB[partition_ID].numbers_of_FAT = buf[16];
//////////////printf("\nnumbers_of_FAT=%d ",BPB[partition_ID].numbers_of_FAT);
//  u16 boot_entries;//根目录项数，供FAT12/16使用
BPB[partition_ID].boot_entries = buf[17] + buf[18] * 256;
//////////////printf("\nboot_entries=%d ",BPB[partition_ID].boot_entries);

//  u16 TotSec16; //This field is the old 16-bit total count of sectors on the volume.
BPB[partition_ID].TotSec16 = buf[19] + buf[20] * 256;
if(CORE[partition_ID].total_sector  == 0)
  CORE[partition_ID].total_sector = BPB[partition_ID].TotSec16;
//BPB[partition_ID].TotSec16 = 0xffff;
//////////////printf("\nTotSec16=%d ",BPB[partition_ID].TotSec16);

//  u8 media_descriptor; //媒体描述符
BPB[partition_ID].media_descriptor = buf[21];
//  u16 sectors_per_FAT; //每个FAT表占用的扇区数，供FAT12/16使用
BPB[partition_ID].sectors_per_FAT =  buf[22] + buf[23] * 256;

if(BPB[partition_ID].sectors_per_FAT == 0)
  BPB[partition_ID].sectors_per_FAT =  buf[36] + buf[37] * 256 + buf[38] * 256*256 + buf[39] * 256*256*256;
//////////////printf("\nsectors_per_FAT=%ld",BPB[partition_ID].sectors_per_FAT);
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
             Return_Entry_Name[i] =  *Path;
             i++; 
             Path++;

      }
    else
      {
       if(!( * Path))
        {
          if(CORE[0].FullPathType == FilePath)
		  {
		  CORE[disk_id].CurPathType = FilePath;
		  }
		  else
		  {
		  CORE[disk_id].CurPathType = DirectoryPath;
		  }
          FileExtension[0] = 0;
          Return_Entry_Name[i] = 0;
          return(LastSplitedNameofPath);  
        } 
	   CORE[disk_id].CurPathType = DirectoryPath;
       (*RemovedCharsFromPath)++; 
       FileExtension[0] = 0;
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
           || ( CORE[disk_id].ClusterNum >= 2 && CORE[disk_id].ClusterNum < 0xfFFFFff && CORE[disk_id].fs_type == FAT32) )
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
  u16 len;
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

  
  if(LengthofString(Extension) > 0)
  {
      len = LengthofString(EntryName);
	  EntryName[len] = '.';
	  EntryName[len + 1] = 0;
	  concanenateString(EntryName,Extension);
	  Extension[0] = 0;
  
  }

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
根据UNICODE编码求GB2312编码
入口：UNICODE编码
出口：GB2312编码
===============================================================================
*/
u16 get_gb2312_from_unicode(u16 unicode)
{
//    u16 i;
	u16 front,end;
	front = 0;
	end = 21790;
	//二分法查找
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
	}while(front < end); 
	//for(i = 0;i<sizeof(gb2312_to_unicode_table);i+=2)
	//{
	// if(gb2312_to_unicode_table[i+1] == unicode)
	//  {
	//   return(gb2312_to_unicode_table[i]);
	//  }
	
	//}
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
 u8 flag;
 u16 len,gb2312_,i,k,pos;
 struct LongNameDirectoryEntry *LongNameDirectoryEntry_Local;
 *Extension = 0; 
 FileNameOffset = 0;
 pos = 258;
 directory_entry_name[pos] = 0;
 LongNameDirectoryEntry_Local = (struct LongNameDirectoryEntry *) (buf + CORE[disk_id].offset);
 do{
	 flag = FILE_NAME;
	 k = 0;//FileNameOffset;
     for(j = 1;j < 10;j+=2)
      {        
       if (LongNameDirectoryEntry_Local->dir_lname1[j] == 0 && LongNameDirectoryEntry_Local->dir_lname1[j+1] == 0)
	   {//////printf("break");
		   break;}   
	   if(LongNameDirectoryEntry_Local->dir_lname1[j+1] != 0 || LongNameDirectoryEntry_Local->dir_lname1[j] & 0x80)
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
              if (LongNameDirectoryEntry_Local->dir_lname2[j] == 0 && LongNameDirectoryEntry_Local->dir_lname2[j+1] == 0)
                 break;
	          if(LongNameDirectoryEntry_Local->dir_lname2[j+1] != 0 || LongNameDirectoryEntry_Local->dir_lname2[j] & 0x80)
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
                  if (LongNameDirectoryEntry_Local->dir_lname3[j] == 0 && LongNameDirectoryEntry_Local->dir_lname3[j+1] == 0)
                     break; 
				  if(LongNameDirectoryEntry_Local->dir_lname3[j+1] != 0 || LongNameDirectoryEntry_Local->dir_lname3[j] & 0x80)
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
			Extension[0] = 0;
            break;
           }
       flag = FILE_NAME;

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
u8 flag; 
u8 buf[512];

read_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
do{
   if(CORE[disk_id].offset == 512)
   {
    return(FAIL);
   }
  flag = FILE_NAME;  //or = FILE_EXTENSION 0xfe 
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
          {	////////printf("EntryName=%s",EntryName);
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
             else if( (CORE[disk_id].CurPathType == FilePath) &&( ! (LongNameDirectoryEntry_Local->dir_attr & ATTR_VOLUME_ID)) 
				  &&( ! (LongNameDirectoryEntry_Local->dir_attr & ATTR_DIRECTORY)) )
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

	   case ATTR_DIRECTORY | ATTR_HIDDEN :{ 
		  CORE[0].Entry_Attr = Directory_Entry_Local->file_attribute;
		  if(mode == Get_Selected_ENTRIES)
           if(CORE[0].FullPathType == FilePath && CORE[disk_id].CurPathType == FilePath)
             break;
          if(GetEntryWith8_3Name(buf,EntryName,Extension)  == SUCC)
             return(SUCC);          
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
return(SUCC);
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
   u8 Name_Compare_OK,Extension_Compare_OK;   
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
					
                  Extension_Compare_OK = OK;     
                  if(stringcmp(Extension,file_extension) != SUCC)
                     Extension_Compare_OK = unOK;
                  else
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
    return(SUCC);
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
   if( ((* filename) >= 'A' && ( * filename ) <= 'Z') || (*filename) & 0x80 )  //从指定盘符根目录开始寻址
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
		 {
			 UPCASE(CORE[0].current_folder);
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
  //////////printf("ClusterQTY = %d",ClusterQTY);
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

  while(ClusterQTY)
  {
   if(CORE[disk_id].fs_type == FAT16)
   {
	   if(NEXTCluster >= 0xfff4 && NEXTCluster <= 0xffff)
		 {  
		   return(SUCC);}
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
u8 FAT_filesystem_initialization(void)
{ 
  u8 root[] = "C:\\",i;

  disk_id = 0;
  Directory_Entry.filename[0]  = 0;
  
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

  
  return(SUCC);
} 
#endif


struct mbr MBR;

//write fat1/2
//    Parameter: none      
//    Return:none
//
static void write_fat(u8 disk_)
{
 U8 buf[512];
 U32 i;//,begin_sector,counter_of_sector;
 for(i = 0;i<512;i++)
	 buf[i] = 0x0;

 for(i = 1;i<10000;i++)
  write_flash_sector(buf,i,disk_);
 //run block erase, clr root/fat1/2 region
 //while(block_erase(1,1+SDs_BlockNum-2)== FAIL);

 buf[0] = 0xf8;buf[1] = 0xff;buf[2] = 0xff;buf[3] = 0xff;
 write_flash_sector(buf,1,disk_); 
 write_flash_sector(buf,257,disk_); 

}
// fill mbr and write fat1/2, root
//    Parameter: Tsectors-- total sector on the partition     
//    Return:none
//
static void fill_mbr_and_write_fat_root(U32 Tsectors,U8 disk_)
{U32 i;
 MBR.BS_jmpBoot[0] = 0XEB;
 MBR.BS_jmpBoot[1] = 0X3C;
 MBR.BS_jmpBoot[2] = 0X90;
 MBR.flag[0] = 0x55;
 MBR.flag[1] = 0xaa;
 MBR.BS_OEMName[0] = 'c';
 MBR.BS_OEMName[1] = 'e';
 MBR.BS_OEMName[2] = 'o';
 MBR.BS_OEMName[3] = ' ';
 MBR.BS_OEMName[4] = 's';
 MBR.BS_OEMName[5] = 'o';
 MBR.BS_OEMName[6] = 'f';
 MBR.BS_OEMName[7] = 't';
 /*write bpb*/

 /*BPB_BytsPerSec = 512*/
 MBR.BPB_BytsPerSec[0] = 0;
 MBR.BPB_BytsPerSec[1] = 2;
 /* BPB_SecPerClus*/
 MBR.BPB_SecPerClus = 64;
 /* RESERVED SECTOR COUNTER = 1*/
 MBR.BPB_RsvdSecCnt[0] = 1;
 MBR.BPB_RsvdSecCnt[1] = 0;
 /*NUM of FATS*/
 MBR.BPB_NumFATs = 2;
 /* RootEntCnt = 1024*/
 MBR.BPB_RootEntCnt[0] = 0x00;
 MBR.BPB_RootEntCnt[1] = 0x4; 
 /* TotSec16[2] = 0*/
 MBR.BPB_TotSec16[0] = 0x0;
 MBR.BPB_TotSec16[1] = 0x0;
 /*0xF8 is the standard value for “fixed”*/
 MBR.BPB_Media = 0XF8;
 /* This field is the FAT12/FAT16 16-bit count of sectors occupied by
ONE FAT*/
 MBR.BPB_FATSz16[0] = 0;
 MBR.BPB_FATSz16[1] = 1;

/*BPB_TotSec32*/
 MBR.BPB_TotSec32[0] = (U8)(Tsectors & 0xff); 
 MBR.BPB_TotSec32[1] = (U8)((Tsectors >> 8) & 0xff);
 MBR.BPB_TotSec32[2] = (U8)((Tsectors >> 16) & 0xff);
 MBR.BPB_TotSec32[3] = (U8)((Tsectors >> 24) & 0xff);
 for(i = 0;i<448;i++)
	 MBR.pad[i] = ' ';
 MBR.BS_FilSysType[0] = 'F';
 MBR.BS_FilSysType[1] = 'A';
 MBR.BS_FilSysType[2] = 'T';
 MBR.BS_FilSysType[3] = '1';
 MBR.BS_FilSysType[4] = '6';
 MBR.BS_FilSysType[5] = ' ';
 MBR.BS_FilSysType[6] = ' ';
 MBR.BS_FilSysType[7] = ' ';
 for(i = 0;i<11;i++)
  MBR.BS_VolLab[i] = ' ';
 MBR.BS_DrvNum = 0X0;
 MBR.BS_Reserved1 = 0;
 MBR.BS_BootSig = 0X29;
 MBR.BS_VolID[0] = 0XB9;
 MBR.BS_VolID[1] = 0XD2;
 MBR.BS_VolID[2] = 0X8F;
 MBR.BS_VolID[3] = 0XA8;
 write_flash_sector((U8 *)(&MBR),0,disk_);  
 //write_root();
 write_fat(disk_);
}
//boot sector data for FAT32
U8 const _boot_sector[512] = {
	0xEB, 0x58, 0x90, 0x4D, 0x53, 0x44, 0x4F, 0x53, 0x35, 0x2E, 0x30, 0x00, 0x02, 0x40, 0x24, 0x00, 
	0x02, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x3F, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x20, 0x76, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 
	0x01, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x29, 0x06, 0x8F, 0x5F, 0xFC, 0x4E, 0x4F, 0x20, 0x4E, 0x41, 0x4D, 0x45, 0x20, 0x20, 
	0x20, 0x20, 0x46, 0x41, 0x54, 0x33, 0x32, 0x20, 0x20, 0x20, 0x33, 0xC9, 0x8E, 0xD1, 0xBC, 0xF4, 
	0x7B, 0x8E, 0xC1, 0x8E, 0xD9, 0xBD, 0x00, 0x7C, 0x88, 0x4E, 0x02, 0x8A, 0x56, 0x40, 0xB4, 0x08, 
	0xCD, 0x13, 0x73, 0x05, 0xB9, 0xFF, 0xFF, 0x8A, 0xF1, 0x66, 0x0F, 0xB6, 0xC6, 0x40, 0x66, 0x0F, 
	0xB6, 0xD1, 0x80, 0xE2, 0x3F, 0xF7, 0xE2, 0x86, 0xCD, 0xC0, 0xED, 0x06, 0x41, 0x66, 0x0F, 0xB7, 
	0xC9, 0x66, 0xF7, 0xE1, 0x66, 0x89, 0x46, 0xF8, 0x83, 0x7E, 0x16, 0x00, 0x75, 0x38, 0x83, 0x7E, 
	0x2A, 0x00, 0x77, 0x32, 0x66, 0x8B, 0x46, 0x1C, 0x66, 0x83, 0xC0, 0x0C, 0xBB, 0x00, 0x80, 0xB9, 
	0x01, 0x00, 0xE8, 0x2B, 0x00, 0xE9, 0x48, 0x03, 0xA0, 0xFA, 0x7D, 0xB4, 0x7D, 0x8B, 0xF0, 0xAC, 
	0x84, 0xC0, 0x74, 0x17, 0x3C, 0xFF, 0x74, 0x09, 0xB4, 0x0E, 0xBB, 0x07, 0x00, 0xCD, 0x10, 0xEB, 
	0xEE, 0xA0, 0xFB, 0x7D, 0xEB, 0xE5, 0xA0, 0xF9, 0x7D, 0xEB, 0xE0, 0x98, 0xCD, 0x16, 0xCD, 0x19, 
	0x66, 0x60, 0x66, 0x3B, 0x46, 0xF8, 0x0F, 0x82, 0x4A, 0x00, 0x66, 0x6A, 0x00, 0x66, 0x50, 0x06, 
	0x53, 0x66, 0x68, 0x10, 0x00, 0x01, 0x00, 0x80, 0x7E, 0x02, 0x00, 0x0F, 0x85, 0x20, 0x00, 0xB4, 
	0x41, 0xBB, 0xAA, 0x55, 0x8A, 0x56, 0x40, 0xCD, 0x13, 0x0F, 0x82, 0x1C, 0x00, 0x81, 0xFB, 0x55, 
	0xAA, 0x0F, 0x85, 0x14, 0x00, 0xF6, 0xC1, 0x01, 0x0F, 0x84, 0x0D, 0x00, 0xFE, 0x46, 0x02, 0xB4, 
	0x42, 0x8A, 0x56, 0x40, 0x8B, 0xF4, 0xCD, 0x13, 0xB0, 0xF9, 0x66, 0x58, 0x66, 0x58, 0x66, 0x58, 
	0x66, 0x58, 0xEB, 0x2A, 0x66, 0x33, 0xD2, 0x66, 0x0F, 0xB7, 0x4E, 0x18, 0x66, 0xF7, 0xF1, 0xFE, 
	0xC2, 0x8A, 0xCA, 0x66, 0x8B, 0xD0, 0x66, 0xC1, 0xEA, 0x10, 0xF7, 0x76, 0x1A, 0x86, 0xD6, 0x8A, 
	0x56, 0x40, 0x8A, 0xE8, 0xC0, 0xE4, 0x06, 0x0A, 0xCC, 0xB8, 0x01, 0x02, 0xCD, 0x13, 0x66, 0x61, 
	0x0F, 0x82, 0x54, 0xFF, 0x81, 0xC3, 0x00, 0x02, 0x66, 0x40, 0x49, 0x0F, 0x85, 0x71, 0xFF, 0xC3, 
	0x4E, 0x54, 0x4C, 0x44, 0x52, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x0A, 0x52, 0x65, 
	0x6D, 0x6F, 0x76, 0x65, 0x20, 0x64, 0x69, 0x73, 0x6B, 0x73, 0x20, 0x6F, 0x72, 0x20, 0x6F, 0x74, 
	0x68, 0x65, 0x72, 0x20, 0x6D, 0x65, 0x64, 0x69, 0x61, 0x2E, 0xFF, 0x0D, 0x0A, 0x44, 0x69, 0x73, 
	0x6B, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72, 0xFF, 0x0D, 0x0A, 0x50, 0x72, 0x65, 0x73, 0x73, 0x20, 
	0x61, 0x6E, 0x79, 0x20, 0x6B, 0x65, 0x79, 0x20, 0x74, 0x6F, 0x20, 0x72, 0x65, 0x73, 0x74, 0x61, 
	0x72, 0x74, 0x0D, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAC, 0xCB, 0xD8, 0x00, 0x00, 0x55, 0xAA
};


// fill mbr and write fat1/2, root
//    Parameter: Tsectors-- total sector on partition     
//    Return:none
//
static void fill_mbr_and_write_fat32_root(U32 Tsectors,u8 disk_)
{
 
 U8 buf[512];
 U32 FAT_SECTORS,i,sectors_per_clusters;
 write_flash_sector((U8 *)_boot_sector,0,disk_);
 read_flash_sector(buf,0,disk_);
 //计算FAT32合适的每簇扇区数
 for(sectors_per_clusters = 1;sectors_per_clusters<=128;sectors_per_clusters=sectors_per_clusters*2)
 {  
    if((Tsectors / sectors_per_clusters) >= (66525) && (Tsectors / sectors_per_clusters) <= (66525*2))
		break;
 }
 //计算FAT32的FAT表占用扇区数
 FAT_SECTORS = Tsectors /(sectors_per_clusters) * 4 /512 + 2;
 //将FAT32的FAT表占用扇区数和分区总扇区数填入MBR扇区
 buf[36] = FAT_SECTORS;
 buf[37] = FAT_SECTORS / 256;
 buf[38] = FAT_SECTORS / 65536;
 buf[39] = FAT_SECTORS / (256 * 65536);

 buf[32] = Tsectors;
 buf[33] = Tsectors / 256;
 buf[34] = Tsectors / 65536;
 buf[35] = Tsectors / (256 * 65536);
 //将FAT32的每簇扇区数填入MBR扇区
 buf[13] = sectors_per_clusters;
 write_flash_sector(buf,0,disk_);

  for(i = 0;i<512;i++)
	 buf[i] = 0x0;

  //将保留扇区,FAT1/FAT2表,和簇号2清除
 for(i = 1;i< 36 + 2*FAT_SECTORS+sectors_per_clusters;i++)
 {
   write_flash_sector(buf,i,disk_);
   
 }
 //初始化FAT表1和表2
 buf[0] = 0xFF;buf[1] = 0xff;buf[2] = 0xff;buf[3] = 0x0f;
 buf[4] = 0xFF;buf[5] = 0xff;buf[6] = 0xff;buf[7] = 0x0f;
 buf[8] = 0xFF;buf[9] = 0xff;buf[10] = 0xff;buf[11] = 0x0f;
 write_flash_sector(buf,36,disk_); 
 write_flash_sector(buf,36+FAT_SECTORS,disk_); 

}   
/*
===============================================================================
函数
自动格式化 FAT 文件系统
入口：disk_:盘符   filesystem_type:以FAT16,FAT32为格式化目标  disk_capacity:磁盘容量
出口：SUCC,FAIL
===============================================================================
*/ 
#if complie_FAT16_filesystem_autoformat
u8 FAT_filesystem_autoformat(u8 disk_,u8 filesystem_type,u32 disk_capacity)
{
 u8 buf[512];
  if(disk_ >= 'c' && disk_ <= 'z')
       disk_ -= 32;   //convert to upper case
  disk_ = disk_ - 'C'; 
 if( read_flash_sector(buf,0,disk_) == SUCC ) //read MBR
 {   
     if ( ! ( buf[510] == 0x55 && buf[511] == 0xaa))
	  {  
		  
		 if(filesystem_type== FAT16)
	    	  fill_mbr_and_write_fat_root(disk_capacity,disk_);
         else if(filesystem_type== FAT32)
		 {
		      fill_mbr_and_write_fat32_root(disk_capacity,disk_);
		 }
	  
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
 CORE[0].FullPathType = FilePath;
 if(FullPathToSectorCluster(filename) == SUCC)
 { 
     FCBsn = Allocate_FCB();
     if(FCBsn == EAllocate_FCB)
       return(FAIL);
	 
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

     return(FCBsn); 
   }
  else
   return(FAIL);
}
#endif 
/*
===============================================================================
函数 
关闭文件
入口：FCBsn
出口：SUCC,FAIL
===============================================================================
*/  
#if complie_close_file
u8 close_file(u8 FCBsn)
{  
   #if enable_time_stamp_transaction
      write_time_stamp(FCBsn,ACCESS_TIME);
   #endif
  if(Free_FCB(FCBsn) == SUCC)
    return(SUCC);
  else
    return(FAIL);
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
	}while(front < end); 
   /*u16 i;
	for(i = 0;i<sizeof(gb2312_to_unicode_table);i+=2)
	{
	  if(gb2312_to_unicode_table[i] == gb2312_code)
	  {
	    return(i+1);
	  }
	
	}*/
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

u8 LFN_DirectoryType;
u32 LFN_offset;
u32 LFN_SectorNum;
u32 LFN_ClusterNum;
u8 LFN_Disk_ID;


u8 primary_LFN_DirectoryType;
u32 primary_LFN_offset;
u32 primary_LFN_SectorNum;
u32 primary_LFN_ClusterNum;
u8 primary_LFN_Disk_ID;

void save_cur_paramenters(void)
{
primary_LFN_DirectoryType = CORE[disk_id].DirectoryType;
primary_LFN_offset = CORE[disk_id].offset;
primary_LFN_SectorNum = CORE[disk_id].SectorNum;
primary_LFN_ClusterNum = CORE[disk_id].ClusterNum;
primary_LFN_Disk_ID = disk_id;

}

void load_lfn_dir_paramenter(void)
{
disk_id = LFN_Disk_ID;
CORE[disk_id].DirectoryType = LFN_DirectoryType;
CORE[disk_id].offset=LFN_offset ;
CORE[disk_id].SectorNum = LFN_SectorNum ;
CORE[disk_id].ClusterNum=LFN_ClusterNum ;
}

void restore_cur_paramenters(void)
{
disk_id = primary_LFN_Disk_ID;
CORE[disk_id].DirectoryType = primary_LFN_DirectoryType;
CORE[disk_id].offset=primary_LFN_offset ;
CORE[disk_id].SectorNum = primary_LFN_SectorNum ;
CORE[disk_id].ClusterNum=primary_LFN_ClusterNum ;

}



static u8 LFN_convert_to_SFN(u8 * Directory_Entry,u8 * SFN_Directory_Entry_buf)
{
 u8 i,flag,j;
 u8 buf[512];
 for(i = 0;i < 11;i++)
   SFN_Directory_Entry_buf[i] = 0x20; //SAVE FLODER PARAMENTE for LFN Generator

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
				  Directory_Entry--; 
			 }
             Directory_Entry++;  
            }
          }
        else if( flag  == FILE_EXTENSION)
		{  
          if( j >= 3 )
		  { //扩展名只取3个字符
		    Directory_Entry++;
			continue;
		  }
          SFN_Directory_Entry_buf[8+j] =  *Directory_Entry;
          j++;
          Directory_Entry++;
         } 
      }
	  else
	  {  //检查短文件名是否存在?存在则文件名号加一,直到没有重复
	    save_cur_paramenters();
        load_lfn_dir_paramenter();
		read_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
		do{
		    flag = SUCC;
			//检查短文件名
            for(i = 0;i<11;i++)
			{
			  if(SFN_Directory_Entry_buf[i] != buf[CORE[disk_id].offset+i])
			  {
			   flag = FAIL;
			   break;
			  }
			}
			
		    if(flag == SUCC)
			{  //短文件名存在, 文件名~SN++,
	           if(SFN_Directory_Entry_buf[7] == '9')
			   {
		         SFN_Directory_Entry_buf[7]= 'A';
			   }
		       else if(SFN_Directory_Entry_buf[7] == 255)
			   {
			      SFN_Directory_Entry_buf[7] = '1';
				  SFN_Directory_Entry_buf[6] ++;
			   }
			   else
			   {
		         SFN_Directory_Entry_buf[7]++;
			   }
		       load_lfn_dir_paramenter();
			   read_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
			}
		    else
			{
		       if(CORE_offset_add_32(buf)== FAIL)
			     break;
			}
		}while(1);	
		restore_cur_paramenters();
	    return(SUCC);
	  
	  }
 }while(1);
}
/*
===============================================================================
函数 
为文件名计算长文件名各段的的起始位置
入口：len-段号
出口：段的的起始位置
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
	  {
	    return(Directory_Entry+pos);
	  }
     if(Directory_Entry[pos] & 0x80)
	 {
        pos+=2;//中文字符占用2个字节
	 }
     else
	 { 
		 pos+=1;
	 }
	 cur_pos++;
	 if((cur_pos)==13)//一个DIRECTORY ENTRY是13个字符
	 {cur_pos = 0;
	  counter++;
	  if((counter)== len)
	   {
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

  SN = len;  //sequence number reset to 0
  //从long-filename directory entry处理得其相应的short-filename directory entry
  LFN_convert_to_SFN(Directory_Entry,Directory_Entry_buf);

  //计算short-filename directory entry的校验和
  checksum = calculate_checksum_longfilenameentry(Directory_Entry_buf);
  //处理last long-filename directory entry for file
  read_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);    
  LFN_record_FLAG = SN | (u8)Last_LFN_Record;
  Fill_LFN_Directory_Entry(buf + CORE[disk_id].offset,determine_pos(Directory_Entry,len-1),LFN_record_FLAG,checksum);
  //////////////////////printf("%s",Directory_Entry + (len - 1) * 13);
  len--;
  write_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
  CORE_offset_add_32(buf);
  LFN_record_FLAG = SN;
  //处理其它LFN directory entry
  //do{
  while(len){
   SN--;
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

//保存目录的位置参数,用于无重复短文件名的生成
LFN_DirectoryType = CORE[disk_id].DirectoryType;
LFN_Disk_ID = disk_id;
if(LFN_DirectoryType == RootDirectory)
  {
	if(CORE[disk_id].fs_type == FAT16)
      LFN_SectorNum = CORE[disk_id].FirstRootDirSecNum;
	else
	{
	  LFN_SectorNum = FirstSectorofCluster(CORE[disk_id].RootClus); 
	  LFN_ClusterNum = CORE[disk_id].RootClus;
	}
  }
  else
  { 
    LFN_ClusterNum = CORE[disk_id].ClusterOfDirectoryEntry;   //存放当前Enumerated Directory Entry所在Directory的ClusterNum,SectorNum,offset
    LFN_SectorNum = FirstSectorofCluster(CORE[disk_id].ClusterOfDirectoryEntry);
  } 
  LFN_offset = 0;



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
	//如果是中文文件名,则置标记,建立长文件名
    for(i = 0;i<LengthofString(Directory_Entry);i++)
	{
	   if(Directory_Entry[i] & 0x80)
	   {
	      flag = 1;
		  break;
	   }
	}

	//扩展名的检查,如果扩展名>3,建立文件名使用长文件名
	if(len == 1 || !flag)
	{
	    for(i = 0;i<LengthofString(Directory_Entry);i++)
		{
	      if(Directory_Entry[i] == '.')
		  {
		    break;
		  }
		}
        if((LengthofString(Directory_Entry)-(i+1)) >3)
			flag = 1;
	
	}
	if((len > 1) || flag)
		len++;
	if(Seek_Space_to_Write_Directory_Entry(len,buf) == SUCC)
	{ 
	  if(len == 1 && !flag)  //写短文件名
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
 stringcpy(filename,buf);

 CORE[0].FullPathType = FilePath;
 if(FullPathToSectorCluster(buf) != SUCC)  //检查文件是否已经存在?
 {   //文件不存在,则可以建立文件
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
	 { //是相对路径,在当前目录上建立文件
	   if(FullPathToSectorCluster(CORE[0].current_folder) == FAIL)
	  {
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
	  //是绝对路径,则在根目录下建立文件
	  if(FullPathToSectorCluster(TEMP) == FAIL)
	  {
	   return(FAIL);
	  }		  
	 }
	 else 
	 {//在指定目录下建立文件
	  if(FullPathToSectorCluster(buf) == FAIL)
	  { 
		  return(FAIL);}
	 }
	 //添加文件目录项
	 status = Add_A_Directory_Entry_(buf+temp,ATTR_ARCHIVE,0,0);
	 return(status);
  }
  else
  {
	  //文件存在,则不能建立文件,建立文件失败
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
u8 create_folder(u8 * foldername)
{  
 u16 len;
 u16 temp;
 u8 TEMP[4];
 u8 buf[260],status;
 stringcpy(foldername,buf);

 CORE[0].FullPathType = DirectoryPath; 
 if(FullPathToSectorCluster(buf) != SUCC)  //检查目录是否已经存在?
 {   //目录不存在,是建立目录
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
	 { //是相对路径,在当前目录上建立目录
		 CORE[0].FullPathType = DirectoryPath; 
	   if(FullPathToSectorCluster(CORE[0].current_folder) == FAIL)
	  {
	   return(FAIL);
	  }
	 } 
	 else if(buf[1] == ':' && temp <= 3)
	 {
	  TEMP[0] = buf[0];
	  TEMP[1] = buf[1];
	  TEMP[2] = '\\';
	  TEMP[3] = 0;
	  CORE[0].FullPathType = DirectoryPath; //是绝对路径,则在根目录下建立目录
	  if(FullPathToSectorCluster(TEMP) == FAIL)
	  {
	   return(FAIL);
	  }
	 }//在指定目录下建立文件  
	 else if(FullPathToSectorCluster(buf) == FAIL)
	 { 
	   return(FAIL);
	  }
	 //调用函数建立目录项
	 status = Add_A_Directory_Entry_(buf + temp,ATTR_DIRECTORY,0,0);
     return(status);
  }
  else
    return(FAIL);//目录存在,则不能建立目录,目录建立失败


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
 if(FCB[FCBsn].file_openned_flag == UnusedFlag)
    return(FAIL);
    #if enable_time_stamp_transaction
      write_time_stamp(FCBsn,ACCESS_TIME);
   #endif
 switch(origin)
  { 
   case SEEK_SET:  //从文件开始位置开始移动文件指针 
    { 
      if(offset < 0 || (u32)offset >= FCB[FCBsn].FileSize)
        return(FAIL);
      FCB[FCBsn].cur_position = offset; 
    #if EnableFileBuf
      Update_FCB_file_buffer(FCBsn); 
    #endif
      break;
    }
   case SEEK_CUR: //从当前位置开始移动文件指针
    { 
      if(((FCB[FCBsn].cur_position + offset) < 0 )|| 
        ((FCB[FCBsn].cur_position + offset)  >= FCB[FCBsn].FileSize)) 
         return(FAIL); 
      FCB[FCBsn].cur_position += offset;
     #if EnableFileBuf
      Update_FCB_file_buffer(FCBsn);
     #endif
      break;
    }
   case SEEK_END: //从结束位置开始移动文件指针
    {
      if(offset > 0)
       return(FAIL);
       FCB[FCBsn].cur_position = FCB[FCBsn].FileSize + offset;
      #if EnableFileBuf
       Update_FCB_file_buffer(FCBsn);
      #endif
      break;
    }
   default:return(FAIL);
  }
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
   if(FCB[FCBsn].file_openned_flag == UnusedFlag)
     return(0); 
   disk_id = FCB[FCBsn].disk;
   #if enable_time_stamp_transaction
      write_time_stamp(FCBsn,ACCESS_TIME);
   #endif
   readed_bytes = 0;
   if((FCB[FCBsn].cur_position + length) > FCB[FCBsn].FileSize)
   { 
     length = FCB[FCBsn].FileSize - FCB[FCBsn].cur_position ;
	 if(!length)
		 return(readed_bytes);
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
    return(0);
   disk_id = FCB[FCBsn].disk;  
   #if enable_time_stamp_transaction
      write_time_stamp(FCBsn,ACCESS_TIME);
   #endif
   for(i = 0;i < length ;i++)
   {                                   
	  if(FCB[FCBsn].cur_position >= FCB[FCBsn].FileSize)
        return(i);
	  OffsetInbuf = (u16)(FCB[FCBsn].cur_position % (FileBUFSize * TotalFileBUFsQTYeachFCB));
      if ( ! OffsetInbuf)
        Update_FCB_file_buffer(FCBsn);  
      buffer[i] = FCB[FCBsn].FileBUF [OffsetInbuf];
      FCB[FCBsn].cur_position ++;
   }
  return(i); 
 #endif
#else
   //return(i);
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
   disk_id = FCB[FCBsn].disk;
   #if enable_time_stamp_transaction
      write_time_stamp(FCBsn,ACCESS_TIME | MODIFY_TIME);
   #endif   
   if(FCB[FCBsn].file_openned_flag == UnusedFlag)
     return(0);
   if(FCB[FCBsn].Permission == ReadOnlyPermission || length == 0)
	 return(0); 
   writed_bytes = 0;
   OffsetInbuf = (FCB[FCBsn].cur_position % (FileBUFSize * TotalFileBUFsQTYeachFCB));
   if(!OffsetInbuf)
	  {  
         if(Update_FCB_file_buffer(FCBsn) == FAIL)  ////回写缓冲区至磁盘和更新file buffer
			 return(0);
	  } 
   temp = FileBUFSize * TotalFileBUFsQTYeachFCB - OffsetInbuf;
   if(temp >= length)
   { point =  FCB[FCBsn].FileBUF + OffsetInbuf;
     for(i = 0;i < length ;i++)
       point[i] = buffer[i];
     FCB[FCBsn].cur_position += length;
	 FCB[FCBsn].Modified_Flag = 1;
	 if(FCB[FCBsn].cur_position >= FCB[FCBsn].FileSize)
		  FCB[FCBsn].FileSize = FCB[FCBsn].cur_position ;
     if(Writeback_FCB_file_buffer(FCBsn) == FAIL)  //回写缓冲区至磁盘
	    i = 0;
     return(i);
   }
   else
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
	    return(writed_bytes);
     if(length <= FileBUFSize * TotalFileBUFsQTYeachFCB)
	 {
      for(i = 0;i < length ;i++)
        FCB[FCBsn].FileBUF [i] = buffer[i];
      FCB[FCBsn].cur_position += length;
	  FCB[FCBsn].Modified_Flag = 1; 
	  if(FCB[FCBsn].cur_position >= FCB[FCBsn].FileSize)
		  FCB[FCBsn].FileSize = FCB[FCBsn].cur_position ;
      if(Writeback_FCB_file_buffer(FCBsn) == FAIL)  //回写缓冲区至磁盘
	    i = 0;
	  writed_bytes += i; 
      return(writed_bytes);	 
	 }
	 else
	 {
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
   return(writed_bytes);
  #else 
   u16 i,OffsetInbuf;
   if(FCB[FCBsn].file_openned_flag == UnusedFlag)
     return(0);

   if(FCB[FCBsn].Permission == ReadOnlyPermission)
	 return(0);
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
			 return(0);
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
   return(i);
  #endif
 #else
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

  if(mode)  //返回上一层目录
   {
    if (CORE[0].CurrentDirectoryType == RootDirectory)
     return(0x55);
    else
    {
	 CORE[0].FullPathType = DirectoryPath;
     if(FullPathToSectorCluster(CORE[0].current_folder) != SUCC)
	   return(FAIL);
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
	 return(SUCC);


	}   
   }
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
  return(SUCC);
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
 CORE[0].FullPathType = FilePath; 
 if(FullPathToSectorCluster(filename) == SUCC)
    {
	 if(Check_FileOpened_Status(CORE[disk_id].ClusterOfDirectoryEntry,0xff,disk_id) == FileAlreadyopenedByOtherHandle)
         return(FAIL);
     if(Remove_LONGFILENAME(buf,filename) != SUCC)
         return(FAIL);
	 if(CORE[disk_id].ClusterOfDirectoryEntry)
       FreeClusterChain(CORE[disk_id].ClusterOfDirectoryEntry); 
     return(SUCC); 
   }
 else
     return(FAIL);
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
 u8 buf[512],type;
 u32 sector;
 u32 Cluster,Sector,offset;
 CORE[0].FullPathType = DirectoryPath; 
 if(FullPathToSectorCluster(foldername) == SUCC)
   {

     Cluster = CORE[disk_id].ClusterNum;
     Sector = CORE[disk_id].SectorNum;
     offset = CORE[disk_id].offset;
     type = CORE[disk_id].DirectoryType;
     CORE[disk_id].DirectoryType = NoneRootDirectory ;
     CORE[disk_id].offset=64;
     CORE[disk_id].SectorNum = FirstSectorofCluster(CORE[disk_id].ClusterOfDirectoryEntry);
     CORE[disk_id].ClusterNum=CORE[disk_id].ClusterOfDirectoryEntry ;
	 read_flash_sector(buf,CORE[disk_id].SectorNum,disk_id);
     do{
	    if(buf[CORE[disk_id].offset] == 0)  
		{
	      break;
		}
		else if(buf[CORE[disk_id].offset] == 0xe5)
		{
		   if(CORE_offset_add_32(buf)== FAIL)
			     break;
		} 
		else//不能删除非空目录
		{
		   return(FAIL);
		}

	 }while(1);

	 CORE[disk_id].ClusterNum = Cluster;
     CORE[disk_id].SectorNum = Sector;
     CORE[disk_id].offset = offset;
	 CORE[disk_id].DirectoryType = type;
	 Remove_LONGFILENAME(buf,foldername);
     FreeClusterChain(CORE[disk_id].ClusterOfDirectoryEntry);
     return(SUCC);
   }
  else
    return(FAIL);

     
} 
#endif

/*
===============================================================================
函数
获取文件,目录的attribute,如建立时间,存取时间,修改时间
入口：file_directory_name-文件,目录名, type-类型--The_DIRECTORY 或The_FILE,结果存放在attr中
出口：SUCC,FAIL
===============================================================================
*/   
u8 get_file_attribute(u8 * file_directory_name, u8 type, struct attribute * attr)
{
  u8 buf[512];
  U16 offset;  
  u16 time[2];
 if(type == The_DIRECTORY)
   CORE[0].FullPathType = DirectoryPath;
 else
   CORE[0].FullPathType = FilePath;
 if(FullPathToSectorCluster(file_directory_name) == SUCC)
 { 
   //读取ENTRY 所在的SECTOR
   read_flash_sector( buf,CORE[disk_id].PreEntrySectorNum,disk_id);
   offset = CORE[disk_id].PreEntryoffset;

   //读取存取时间
   time[0] = buf[offset + 19] * 256 + buf[offset + 18];
   attr->access_time_year = (time[0]  >> 9) + 1980;
   attr->access_time_month = (time[0] & 0x1e0) >> 5;
   attr->access_time_day = time[0] & 0x1f;
   //读取建立时间
   time[0] = buf[offset + 17] * 256 + buf[offset + 16];
   time[1] = buf[offset + 15] * 256 + buf[offset + 14];
   attr->create_time_year = (time[0]  >> 9) + 1980;
   attr->create_time_month = (time[0] & 0x1e0) >> 5;
   attr->create_time_day = time[0] & 0x1f;
   attr->create_time_hour = (time[1]  >> 11);
   attr->create_time_minute = (time[1] & 0x7e0) >> 5;
   attr->create_time_second = (time[1] & 0x1f)*2;
   //读取修改时间
   time[0] = buf[offset + 25] * 256 + buf[offset + 24];
   time[1] = buf[offset + 23] * 256 + buf[offset + 22];
   attr->modify_time_year = (time[0]  >> 9) + 1980;
   attr->modify_time_month = (time[0] & 0x1e0) >> 5;
   attr->modify_time_day = time[0] & 0x1f;
   attr->modify_time_hour = (time[1]  >> 11);
   attr->modify_time_minute = (time[1] & 0x7e0) >> 5;
   attr->modify_time_second = (time[1] & 0x1f)*2;

   attr->attr = buf[offset + 11];
   return(SUCC);
 }
 else
 {
   return(FAIL);
 }


}
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
           return(FAIL);  //不能删除已打开文件
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
	      return(FAIL);
	}
	else
	{
	  oldfilename[temp2 + 1] = 0;
      concanenateString(oldfilename,newfilename + temp);
      if(FullPathToSectorCluster(oldfilename) == SUCC)
	      return(FAIL);

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
    return(SUCC);
  }
 else
   return(FAIL);
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
     ////////printf("\nenter folder:%s\n",CORE[0].current_folder_for_disk_enum);
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
  if(mode == 0x0)
  {
   CORE[0].FullPathType = DirectoryPath; 
   if(cd_folder(CORE[0].current_folder,0) != SUCC)
	   return(FAIL);
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
   return(SUCC);
  }
  else
   break;
 }while(1);
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
	      return(FAIL);
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
			return(SUCC);
	 } 
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
  u32 i,j,x;
  u8 buf[512];
  if(partition_id >= 'c' && partition_id <= 'f')
       partition_id -= 32;   //convert to upper case
  if ( ! (partition_id >= 'C' && partition_id <= 'F'))
    return(FAIL);
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
        return(FAIL);
      } 
	 }
       *volume_free_space *=  BPB[disk_id].sector_per_cluster;
       *volume_capacity =  CORE[disk_id].total_sector;  
       return(SUCC);

}
#endif

