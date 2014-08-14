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
 
#include<stdio.h>
#include<include\types.h>
#include<Flash_Management\Flash_Management.h>   
#include<include\FAT_cfg.h>
//data struct CORE definition
static struct core_ CORE;
//BPB data struct definition
static struct partition_bpb BPB; 
//FCG data struct definition 
struct FileControlBlock FCB[MaximumFCB];
/*===============================================================================
函数
转换字符串成大写
入口：*string:字符串的地址
出口：SUCC
===============================================================================*/ 
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
/*===============================================================================
函数
字符串的长度的获得
入口：*string:字符串的地址
出口：字符串的总长度
===============================================================================*/ 
static u16 LengthofString(u8 * string)
{ 
 u16 i;
 i = 0;
 while(*string)//字符数的计数,直到遇到=0
  {
    i++;
    string++;
  }
 return(i);
} 
/*===============================================================================
函数
连接两个字符串,字符串2连接后不变
入口：*string:字符串的地址
出口：SUCC,FAIL
===============================================================================*/ 
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

/*===============================================================================
函数
拷贝字符串,字符串１到字符串２的拷贝
入口：*string1:字符串的地址；*string2:字符串的地址
出口：SUCC
===============================================================================*/ 
static u8 stringcpy(u8 *string1,u8 *string2)
{
 while(*string1) 
   { //逐字字符串拷贝
     *string2 = *string1;
     string1++;
     string2++;
   }  
 *string2 = 0;
 return(SUCC);
}
/*===============================================================================
函数
比较两个字符串(字符串的大小写不用区分)
入口：*string1:字符串1的地址；*string2:字符串2的地址
出口：SUCC,FAIL
===============================================================================*/ 
static u8 stringcmp(u8 *string1,u8 *string2)
{
 UPCASE(string1);
 UPCASE(string2);  
 while((*string1) && (*string2)) 
   {
     if((*string1) == (*string2))
	 {   //逐字字符串比较
         string1++;
         string2++;
      }
     else
       return(FAIL);
   }   
 //判断两字符串尾都是０吗？  
 if( ((*string1) == 0) && ((*string2) == 0))
 {
     return(SUCC);
 }
 else
     return(FAIL);
}

/*===============================================================================
函数
在簇链里找到当前簇的下一簇
入口：Cluster:当前簇号
出口: 返回下一簇号
===============================================================================*/ 
static u32 Get_Next_Cluster_From_Current_Cluster(u32 Cluster)
{
   u8 buf[512]; 
   u32 ThisFATSecNum,ThisFATEntOffset;
   //FAT16每个簇占用两个字节
   ThisFATEntOffset = Cluster * 2;
   //计算当前簇所在扇区
   ThisFATSecNum = CORE.relative_sector + BPB.reserved_sector 
                   + (ThisFATEntOffset / BPB.bytes_per_sector);
   //计算当前簇所在扇区的偏移量
   ThisFATEntOffset = ThisFATEntOffset % BPB.bytes_per_sector; 
   //读取当前簇所在扇区
   read_flash_sector(buf,ThisFATSecNum);
   //返回当前簇的下一簇
   return((u32)buf[ThisFATEntOffset] + ((u32)buf[ThisFATEntOffset + 1]) * 256);
}
/*===============================================================================
函数
在簇链里找到当前簇上一的簇号
入口：Cluster:当前簇号
出口: 返回当前簇之前的簇号
===============================================================================*/ 
static u32 Get_Previous_Cluster_From_Current_Cluster(u32 Cluster)
{
  u8 buf[512];
  u32 CUR_FATSecNum,CUR_FATEntOffset;
  u32 CUR_Checking_Cluster_No;
  //从簇号０开始找
  CUR_Checking_Cluster_No = 0;
  CUR_FATEntOffset = 0;
  CUR_FATSecNum = CORE.FirstSectorofFAT1; 
  read_flash_sector(buf,CUR_FATSecNum); 
  do{ 
	 //超出最大簇号，查找失败
	 if( CUR_Checking_Cluster_No == (CORE.CountofClusters + 2))
         {
          return(FAIL);
         } 
	 //检查下一簇是否是当前簇号?
     if( (u16)(buf[CUR_FATEntOffset] + buf[CUR_FATEntOffset + 1] * 256) == (u16)Cluster)
          return(CUR_Checking_Cluster_No);  //返回簇号
     CUR_Checking_Cluster_No++;
     CUR_FATEntOffset += 2;

     if(CUR_FATEntOffset >= 512)//检查了一个扇区，继续到下一扇区检查
      {
        CUR_FATSecNum++;

        read_flash_sector(buf,CUR_FATSecNum); 
        CUR_FATEntOffset = 0;     
      }
  }while(1);
}
/*===============================================================================
函数
删除一个簇链
入口：Cluster:首簇号
出口: SUCC,FAIL
===============================================================================*/ 
static u8 FreeClusterChain(u32 Cluster)
{
 u8 buf[512],i; 
 u32 ThisFAT1SecNum,ThisFATEntOffset,ThisFAT2SecNum;    
 do{
   
   //FAT16每个簇占用两个字节
   ThisFATEntOffset = Cluster * 2;
   //计算当前簇所在扇区
   ThisFAT1SecNum = CORE.relative_sector + BPB.reserved_sector 
                   + (ThisFATEntOffset / BPB.bytes_per_sector);
   ThisFAT2SecNum = ThisFAT1SecNum + BPB.sectors_per_FAT;

   //计算当前簇所在扇区的偏移量
   ThisFATEntOffset = (ThisFATEntOffset) % BPB.bytes_per_sector; 
   read_flash_sector(buf,ThisFAT1SecNum);
   //Cluster置为其下一簇
   Cluster = (u32)buf[ThisFATEntOffset]+ (u32)buf[ThisFATEntOffset + 1] * 256;  
   //将簇值清０，
   buf[ThisFATEntOffset] = 0x0;  
   buf[ThisFATEntOffset + 1] = 0x0;
   //回写ＦＡＴ１
   write_flash_sector(buf,ThisFAT1SecNum); 
   //回写ＦＡＴ２及所有ＦＡＴ表
   for(i = 1;i < BPB.numbers_of_FAT;i++) 
     {
      write_flash_sector(buf,ThisFAT2SecNum);
      ThisFAT2SecNum = ThisFAT2SecNum + BPB.sectors_per_FAT;
     } 
   //判断簇链是否结束？
   if(Cluster >= 0xfff6)
     return(SUCC);   
  }while(1);  
}
/*===============================================================================
函数
返回簇Ｎ的首扇区
入口：N：簇号
出口: 返回簇Ｎ的首扇区
===============================================================================*/  
static u32 FirstSectorofCluster(u32 N)
{
 return((N - 2) * BPB.sector_per_cluster + CORE.FirstDataSector);
}

/*===============================================================================
函数
从FAT中分配一个簇到簇链当前簇之后
入口：Cluster:当前簇号,added_cluster被分配的簇
出口: 加入的簇号
===============================================================================*/ 
u8 Allocate_EMPTY_CLUSTER_TO_CUR_CLUSTER_CHAIN(u32 Cluster,u32 *Added_Cluster,u8 *buf)
{
  u32 temp,EMPTY_CLUSTER,ThisFAT1SecNum,ThisFATEntOffset,ThisFAT2SecNum;
  u16 i; 
  EMPTY_CLUSTER = 0;
  //从起始ＦＡＴ表的起始扇区，找空簇以完成分配
  ThisFAT1SecNum = CORE.relative_sector + BPB.reserved_sector;
  ThisFAT2SecNum = ThisFAT1SecNum + BPB.sectors_per_FAT;
  do{    
  //从偏移量０开始找	  
  ThisFATEntOffset = 0; 
  read_flash_sector(buf,ThisFAT1SecNum);   
  for(i = 0;i < 256;i++)
  {  
     if(buf[ThisFATEntOffset] == 0 && buf[ThisFATEntOffset + 1] == 0)
      {//找到空簇,将当前簇的下一簇放到找到空簇的下一簇
       temp = Get_Next_Cluster_From_Current_Cluster(Cluster);
       buf[ThisFATEntOffset] = (u8)(temp & 0xff);  
       buf[ThisFATEntOffset + 1] = (u8)((temp >> 8) & 0xff);
       //回写ＦＡＴ１
	   write_flash_sector(buf,ThisFAT1SecNum);  
       temp = ThisFAT2SecNum;
       //回写其它ＦＡＴ２及所有的ＦＡＴ表
	   for(i = 1;i < BPB.numbers_of_FAT;i++)
         {
          write_flash_sector(buf,temp);
          temp +=  BPB.sectors_per_FAT;
         }  
       temp = Get_Next_Cluster_From_Current_Cluster(Cluster);
       //将当前簇的下一簇填写指定找到的空簇
	   ThisFATEntOffset = Cluster * 2;
       //计算当前簇所在扇区
	   ThisFAT1SecNum = CORE.relative_sector + BPB.reserved_sector 
                   + (ThisFATEntOffset / BPB.bytes_per_sector);
       
	    //计算当前簇所在扇区的偏移量
	   ThisFATEntOffset = ThisFATEntOffset % BPB.bytes_per_sector;
       ThisFAT2SecNum = ThisFAT1SecNum + BPB.sectors_per_FAT; 
       read_flash_sector(buf,ThisFAT1SecNum);
       buf[ThisFATEntOffset] = (u8)(EMPTY_CLUSTER & 0xff);//Add to cluster chain
       buf[ThisFATEntOffset + 1] = (u8)((EMPTY_CLUSTER >> 8) & 0xff); 
       //回写ＦＡＴ１
	   write_flash_sector(buf,ThisFAT1SecNum);  
       //回写其它ＦＡＴ２及所有的ＦＡＴ表
	   temp = ThisFAT2SecNum;
       for(i = 1;i < BPB.numbers_of_FAT;i++)  //update backup FAT2,etc..
         {          
		   read_flash_sector(buf,temp);
           buf[ThisFATEntOffset] = (u8)(EMPTY_CLUSTER & 0xff);//Add to cluster chain
           buf[ThisFATEntOffset + 1] = (u8)((EMPTY_CLUSTER >> 8) & 0xff); 
           write_flash_sector(buf,temp);
           temp +=  BPB.sectors_per_FAT;
         } 
	    //将空簇的数据清空
	   for(i = 0;i< 512;i++)
		   buf[i] = 0;
       ThisFAT1SecNum = FirstSectorofCluster(EMPTY_CLUSTER);
	   for(i = 0;i < BPB.sector_per_cluster;i++)
           write_flash_sector(buf,ThisFAT1SecNum + i);
       *Added_Cluster = EMPTY_CLUSTER;                  
       return(SUCC);
      }
     else
      {
        ThisFATEntOffset += 2;
        EMPTY_CLUSTER ++;
		//超出最大簇号，查找失败
        if( EMPTY_CLUSTER == (CORE.CountofClusters + 2))
         {
          return(FAIL);
         } 
      }
   }
  ThisFAT1SecNum ++;
  ThisFAT2SecNum ++;
  }while(1);     
}

/*===============================================================================
函数
从FAT中分配一个空簇
入口：cluster--被分配的簇
出口: SUCC,FAIL
===============================================================================*/ 
u8 Allocate_An_Empty_cluster(u32 * cluster,u8 * buf)
{
  u32 temp,EMPTY_CLUSTER,ThisFAT1SecNum,ThisFATEntOffset,ThisFAT2SecNum;
  u16 i; 

  //从起始ＦＡＴ表的起始扇区，找空簇以完成分配
  EMPTY_CLUSTER = 0;
  ThisFAT1SecNum = CORE.relative_sector + BPB.reserved_sector;
  ThisFAT2SecNum = ThisFAT1SecNum + BPB.sectors_per_FAT;
  do{      
  ThisFATEntOffset = 0; 
  read_flash_sector(buf,ThisFAT1SecNum);   
  for(i = 0;i < 256;i++)
   {
     if(buf[ThisFATEntOffset] == 0 && buf[ThisFATEntOffset + 1] == 0)
      {//找到空簇,设置空簇为０ＸＦＦＦＦ，即占用状态
       buf[ThisFATEntOffset] = 0xff;   
       buf[ThisFATEntOffset + 1] = 0xff;
       //回写ＦＡＴ１
	   write_flash_sector(buf,ThisFAT1SecNum);  
       //回写其它ＦＡＴ２及所有的ＦＡＴ表
	   temp = ThisFAT2SecNum;
       for(i = 1;i < BPB.numbers_of_FAT;i++)  //update backup FAT2,etc..
         {
          write_flash_sector(buf,temp);
          temp +=  BPB.sectors_per_FAT;
         }  
	   //将空簇的数据清空
	   for(i = 0;i< 512;i++)
		   buf[i] = 0;
       ThisFAT1SecNum = FirstSectorofCluster(EMPTY_CLUSTER);
	   for(i = 0; i < BPB.sector_per_cluster;i++)
           write_flash_sector(buf,ThisFAT1SecNum + i);
       *cluster = EMPTY_CLUSTER;                  
       return(SUCC);
      }
     else
      {
        ThisFATEntOffset += 2;
        EMPTY_CLUSTER ++; 
		//超出最大簇号，查找失败
        if( EMPTY_CLUSTER == (CORE.CountofClusters + 2))
         {
          return(FAIL);
         } 
      }
   }
  ThisFAT1SecNum ++;
  ThisFAT2SecNum ++;
  }while(1);     

}

  
/*===============================================================================
函数(CORE_INIT_2)
从root sector BPB计算FirstDataSector,FirstRootDirSecNum等等
入口：无
出口: 无
===============================================================================*/
static void BPB_INIT_2(void)
{ //计算根目录占用的扇区数 
  CORE.RootDirSectors = (BPB.boot_entries * 32 + (BPB.bytes_per_sector - 1)) / BPB.bytes_per_sector;                             
  //计算数据区的起始扇区                            
  CORE.FirstDataSector = CORE.relative_sector + BPB.reserved_sector + BPB.sectors_per_FAT
                               * BPB.numbers_of_FAT + CORE.RootDirSectors;
  //计算根目录的起始扇区
  CORE.FirstRootDirSecNum = CORE.relative_sector + BPB.reserved_sector+ BPB.sectors_per_FAT
                               * BPB.numbers_of_FAT;
  //计算数据区占用扇区数
  CORE.DataSec = CORE.total_sector - BPB.reserved_sector - CORE.RootDirSectors
                 - BPB.sectors_per_FAT * BPB.numbers_of_FAT;
  //计算数据区对应的簇数
  CORE.CountofClusters = CORE.DataSec / BPB.sector_per_cluster;
  //计算FAT1,FAT2的起始扇区
  CORE.FirstSectorofFAT1 = CORE.relative_sector + BPB.reserved_sector;
  CORE.FirstSectorofFAT2 = CORE.relative_sector + BPB.reserved_sector + BPB.sectors_per_FAT;
}
/*===============================================================================
函数
Read Partition PBP
入口：Partition ID
出口：无
===============================================================================*/ 
static u8 Read_partition_PBP(u8 partition_ID)
{  

  u8 buf[512];  
   if ((CORE.PartitionID - 'C') ==  partition_ID) 
    { 
	  //指定的BPB已经在缓冲中,直接返回
      return(SUCC);
    } 
   //读取MBR,即扇区0
   read_flash_sector(buf,0); //read MBR 
   if ( buf[0x1be] == 0x00 || buf[0x1be] == 0x80) // check boot indicator 00 or 0x80
   {
   
   }
   else
   {
	   CORE.relative_sector = 0; 
       CORE.total_sector =  *((u32*) (buf+32));//保存分区容量
       CORE.system_id = buf[0x1c2]; //保存分区类型 0C-FAT32,06-FAT16 ect..
       CORE.PartitionID= 'C' + partition_ID; //从C开始到Z结束 
	   //保存BPB数据
	   if ( buf[510] == 0x55 && buf[511] == 0xaa)
       {
        //每扇区字节数
        BPB.bytes_per_sector = buf[0xb] + buf[0xc] * 256;
        
        //每簇扇区数
        BPB.sector_per_cluster = buf[0xd];
        //保留扇区数
        BPB.reserved_sector = buf[14] + buf[15] * 256;

        //FAT副本数
        BPB.numbers_of_FAT = buf[16];
        //根目录项数，供FAT12/16使用
        BPB.boot_entries = buf[17] + buf[18] * 256;
        //This field is the old 16-bit total count of sectors on the volume.
        BPB.TotSec16 = buf[19] + buf[20] * 256;
        BPB.TotSec16 = 0xffff;
        //媒体描述符
        BPB.media_descriptor = buf[21];
        //每个FAT表占用的扇区数，供FAT12/16使用
        BPB.sectors_per_FAT =  buf[22] + buf[23] * 256;
        CORE.system_id = 0x6; //保存分区类型 0C-FAT32,06-FAT16 ect..
        BPB_INIT_2();
        return(SUCC);
       } 
	 }
    
  return(FAIL);
}  

/*===============================================================================
函数
从路径中读一个entry
入口：
出口：SUCC,FAIL
===============================================================================*/
static u8 SplitNameFromPath(u8 *Path,u8 *Return_Entry_Name,u8 *FileExtension,u8 *RemovedCharsFromPath)
{  
  u8 i,flag,j;
  flag = FILE_NAME;
  *RemovedCharsFromPath = 0; 
  CORE.CurPathType = DirectoryPath; 
  i = 0; 
  j = 0;
  do{           
     if( ( * Path) != 0 && ( * Path ) != '\\') //Path分离得到Entry name and file extension 
       { 
        (*RemovedCharsFromPath)++; 
        //分离ENTRY NAME
		if( flag == FILE_NAME)
         {            
          if(*Path == '.')
             {//遇到".",则开始分离file extension 
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
         {//分离file extension
          if( j >= 3 )
            return(FAIL);
          FileExtension[j] =  *Path;
          j++;
          Path++;
         } 
      }
    else
      {//分离结束,检查是否是最后的一个ENTRY?
       if(!( * Path))
        {
          if(CORE.FullPathType == FilePath)
		  {
		  CORE.CurPathType = FilePath;
		  }
          FileExtension[j] = 0;
          Return_Entry_Name[i] = 0;
		  //最后一个ENTRY,返回LastSplitedNameofPath
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
/*===============================================================================
函数
Directory Entry的位置+32,转向下一个ENTRY 
入口：无
出口：SUCC,FAIL
===============================================================================*/             
static u8 CORE_offset_add_32(u8 *buf)
{ //位置+32
  CORE.offset += 32;
  if (CORE.offset >= 512)
  {
  //位置越过一个扇区
  if (CORE.DirectoryType == RootDirectory)
  {     //针对根目录的处理
        if (CORE.SectorNum < ( CORE.RootDirSectors +  CORE.FirstRootDirSecNum))
         {
           CORE.SectorNum++;
           CORE.offset = 0; 
           read_flash_sector(buf,CORE.SectorNum);
		   if(buf[CORE.offset] == 0 && buf[CORE.offset+1] == 0)  //End of the directory
		     return(FAIL);//已没有ENTRY,返回FAIL
           return(SUCC);
         }
        else
           return(FAIL);
     }
    else  
     {
        if( (CORE.SectorNum - FirstSectorofCluster(CORE.ClusterNum) + 1) >= BPB.sector_per_cluster)
		{ //位置越过一个簇, 取下一簇,继续处理
           CORE.ClusterNum = Get_Next_Cluster_From_Current_Cluster(CORE.ClusterNum);
           if( CORE.ClusterNum >= 2 && CORE.ClusterNum <= 0xfff7)
            {
               CORE.SectorNum = FirstSectorofCluster(CORE.ClusterNum); 
               CORE.offset = 0;
               read_flash_sector(buf,CORE.SectorNum);
			   if(buf[CORE.offset] == 0 && buf[CORE.offset+1] == 0)  //End of the directory
                   return(FAIL);//已没有ENTRY,返回FAIL
               return(SUCC);
            }
           else
			 return(FAIL);
         }
        else
         {
            CORE.SectorNum++; 
            CORE.offset = 0;
            read_flash_sector(buf,CORE.SectorNum);
			if(buf[CORE.offset] == 0 && buf[CORE.offset+1] == 0)  //End of the directory
                return(FAIL);//已没有ENTRY,返回FAIL
            return(SUCC);
         }
     }
  }
  if(buf[CORE.offset] == 0 && buf[CORE.offset+1] == 0)  //End of the directory
       return(FAIL);//已没有ENTRY,返回FAIL
  return(SUCC);
}
/*===============================================================================
函数
从目录读取短文件名
入口：EntryName-返回文件名,Extension=返回扩展名
出口：SUCC,FAIL
===============================================================================*/ 
static u8 GetEntryWith8_3Name(u8 *buf,u8* EntryName,u8 *Extension)
{
  u8 j;
  struct Directory_Entry_  *Directory_Entry_Local;  
  u8 *pointer;
  pointer = buf;
  Directory_Entry_Local = (struct Directory_Entry_ *) (pointer + CORE.offset);
  //从DIRECTORY 中读取EntryName  
  for(j = 0;j < 8;j++)
   {
    if(Directory_Entry_Local->filename[j] == 0x20)
      break;
    EntryName[j] = Directory_Entry_Local->filename[j];
   }   
  EntryName[j] = 0; 
  //从DIRECTORY 中读取Extension
  for(j = 0;j < 3;j++)
   {  
    if(Directory_Entry_Local->file_extention[j] == 0x20)
        break;
    Extension[j] = Directory_Entry_Local->file_extention[j]; 
   }
  Extension[j] = 0;

  if(CORE.FullPathType == FilePath && CORE.CurPathType == FilePath)
     CORE.FileSize = *(u32*)Directory_Entry_Local->file_length;//保存文件长度
  Directory_Entry_Local->file_length[0] = 0;
  Directory_Entry_Local->file_length[0] = 0;
  //保存文件首簇号
  CORE.ClusterOfDirectoryEntry = (Directory_Entry_Local->first_cluster_number_low2bytes[0]) + 
	  (Directory_Entry_Local->first_cluster_number_low2bytes[1]) * 256;
  CORE.PreEntrySectorNum = CORE.SectorNum;
  CORE.PreEntryoffset = CORE.offset;
  CORE_offset_add_32(buf);//Directory Entry的位置+32 
  return(SUCC);
}
/*===============================================================================
函数
从目录读取长文件名
入口：longFileName-返回文件名,Extension=返回扩展名
出口：SUCC,FAIL
===============================================================================*/ 
static u8 GetEntryWithLongFileName(u8 *buf,u8* longFileName,u8 *Extension)
{
 u8 j,FileNameOffset;
                   
 u8 flag,k,i;
 u16 len;
 struct LongNameDirectoryEntry *LongNameDirectoryEntry_Local;
 *Extension = 0;

 FileNameOffset = 242;
 LongNameDirectoryEntry_Local = (struct LongNameDirectoryEntry *) (buf + CORE.offset);
 do{
	 k = FileNameOffset;
	 //读取longNameDirectoryEntry中的前5个字节
     for(j = 1;j < 10;j+=2)
      {        
       if (LongNameDirectoryEntry_Local->dir_lname1[j] == 0)
          break; //遇0结束退出  
       longFileName[k] = LongNameDirectoryEntry_Local->dir_lname1[j];
       k ++;                 
      } 
	 longFileName[k] = 0;
       if(j >= 10)
         { //读取longNameDirectoryEntry中的第二6个字节
           for(j = 0;j < 12;j += 2)
            {  
              if (LongNameDirectoryEntry_Local->dir_lname2[j] == 0)
                 break;

                 longFileName[k] = LongNameDirectoryEntry_Local->dir_lname2[j];
                 k++;           
            }
           if(j >= 12)   
                for(j = 0;j < 4;j += 2)//读取longNameDirectoryEntry中的第三大作2个字节
                 { 
                  if (LongNameDirectoryEntry_Local->dir_lname3[j] == 0)
                     break;       
                    longFileName[k] = LongNameDirectoryEntry_Local->dir_lname3[j];
                    k ++;                   
                 }
          }
	 if(k > 242)
	  longFileName[k] = 0;
	CORE.PreEntrySectorNum = CORE.SectorNum;
    CORE.PreEntryoffset = CORE.offset;
	if(CORE_offset_add_32(buf) == FAIL) 
       return(FAIL);//Directory Entry的位置+32 
    FileNameOffset -= 13;
    k = FileNameOffset;
    LongNameDirectoryEntry_Local = (struct LongNameDirectoryEntry *) (buf + CORE.offset);
    if(LongNameDirectoryEntry_Local->dir_attr != ATTR_LONG_NAME)
	{      //遇到非ATTR_LONG_NAME表示一个LONG FILE NAME读怒即将结束,处理最后的short entry
           if ( ! (LongNameDirectoryEntry_Local->dir_attr & ATTR_VOLUME_ID)) 
           {//保存首簇号
			CORE.ClusterOfDirectoryEntry = LongNameDirectoryEntry_Local->dir_first[0]+
				LongNameDirectoryEntry_Local->dir_first[1] * 256;
			//保存文件长度
            CORE.FileSize = *((u32*)LongNameDirectoryEntry_Local->dir_lname3);
			//拷贝longFileName到变量longFileName
			stringcpy(longFileName+FileNameOffset+13,longFileName);
			//分离文件扩展名
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
			   if(i >= 15 || len == 0 )
				 break;
			}while(1);
            break;
           }
       flag = FILE_NAME;
       FileNameOffset = 256 - 13;
       k = FileNameOffset; 
	   do{
		  CORE.PreEntrySectorNum = CORE.SectorNum;
          CORE.PreEntryoffset = CORE.offset;
          if(CORE_offset_add_32(buf) == FAIL) 
            return(FAIL); //Directory Entry的位置+32 
          LongNameDirectoryEntry_Local = (struct LongNameDirectoryEntry *) (buf + CORE.offset);
	      if(LongNameDirectoryEntry_Local->dir_lname1[0] == 0xe5)
             continue;//过滤已删除ENTRY
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

              
/*===============================================================================
函数
从目录读取一个Entry
入口：mode = 0：返回所有directory entry
出口：SUCC,FAIL
===============================================================================*/     
static u8 GetEntryFromDirectory(u8 *EntryName, u8 *Extension,u8 mode)
{ 
struct Directory_Entry_  *Directory_Entry_Local; 
struct LongNameDirectoryEntry *LongNameDirectoryEntry_Local; 
u8 flag; 
u8 buf[512];
read_flash_sector(buf,CORE.SectorNum);
do{ 
  if(CORE.offset == 512)
  {
    return(FAIL);
  }
  flag = FILE_NAME; 
  Directory_Entry_Local = (struct Directory_Entry_ *) (buf + CORE.offset);
  if(Directory_Entry_Local->filename[0] == 0x0)//遇目录结尾,读取已至结尾,退出返回FAIL
	  return(FAIL);
  if(Directory_Entry_Local->filename[0] == 0xe5)//过滤已删除ENTRY
  {
   CORE.PreEntrySectorNum = CORE.SectorNum;
   CORE.PreEntryoffset = CORE.offset;
   if(CORE_offset_add_32(buf) == FAIL) //Directory Entry的位置+32 
     return(FAIL);
   continue;
  }  
  switch(Directory_Entry_Local->file_attribute) //检查file_attribute, 
    {
       case ATTR_LONG_NAME:{ //从目录读取长文件名
          if(GetEntryWithLongFileName(buf,EntryName,Extension) == SUCC)
          {	
		   read_flash_sector(buf,CORE.SectorNum);
           LongNameDirectoryEntry_Local = (struct LongNameDirectoryEntry *)(buf + CORE.offset);
		   CORE.Entry_Attr = LongNameDirectoryEntry_Local->dir_attr;
		   if(mode == Get_Selected_ENTRIES)
		   {
             if(CORE.CurPathType == DirectoryPath && 
               (LongNameDirectoryEntry_Local->dir_attr & ATTR_DIRECTORY))
			 { 
               CORE.ClusterOfDirectoryEntry = *(u16*)LongNameDirectoryEntry_Local->dir_first;
               CORE.PreEntrySectorNum = CORE.SectorNum;
               CORE.PreEntryoffset = CORE.offset;
			   CORE_offset_add_32(buf);///Directory Entry的位置+32 
               return(SUCC);
			 }
             else if ( ! (LongNameDirectoryEntry_Local->dir_attr & ATTR_VOLUME_ID)) 
			 {
              CORE.PreEntrySectorNum = CORE.SectorNum;
             CORE.PreEntryoffset = CORE.offset;
			 CORE_offset_add_32(buf);//Directory Entry的位置+32 
             return(SUCC);
             }	   
		   }
		   else
		   {
			CORE.PreEntrySectorNum = CORE.SectorNum;
            CORE.PreEntryoffset = CORE.offset;
		    CORE.ClusterOfDirectoryEntry = *(u16*)LongNameDirectoryEntry_Local->dir_first;
            CORE_offset_add_32(buf);//Directory Entry的位置+32 
            
			return(SUCC);
		   }
          }
          break;
        }
       case ATTR_DIRECTORY:{//从目录读取目录(短文件名)
		  CORE.Entry_Attr = Directory_Entry_Local->file_attribute;
		  if(mode == Get_Selected_ENTRIES)
           if(CORE.FullPathType == FilePath && CORE.CurPathType == FilePath)
             break;
          if(GetEntryWith8_3Name(buf,EntryName,Extension)  == SUCC)
             return(SUCC);       
          break;
       }
       case ATTR_VOLUME_ID:CORE.Entry_Attr = Directory_Entry_Local->file_attribute;
       //case 0:break;
       default:
        {//从目录读取文件(短文件名)
		 CORE.Entry_Attr = Directory_Entry_Local->file_attribute;
	     if(mode == Get_Selected_ENTRIES)
          if(CORE.FullPathType == DirectoryPath)
             break;
          return(GetEntryWith8_3Name(buf,EntryName,Extension)); 
        }
     } 
  CORE.PreEntrySectorNum = CORE.SectorNum;
  CORE.PreEntryoffset = CORE.offset;
  if(CORE_offset_add_32(buf) == FAIL) 
    return(FAIL);//Directory Entry的位置+32 
 }while(1); 
return(SUCC);
}  
/*===============================================================================
函数
从目录中查找一个Entry
入口：floder_name-文件名,file_extension-扩展名
出口：SUCC,FAIL
===============================================================================*/
static u8 FindEntryStruct(u8 *floder_name,u8 *file_extension)
{  
   u8 EntryName[256],Extension[20]; 
   u8 Name_Compare_OK,Extension_Compare_OK;   
   do{	  //从目录读取一个Entry	   
          if(GetEntryFromDirectory(EntryName,Extension,Get_Selected_ENTRIES) != SUCC)
		  {
            return(FAIL);
		  }
       Name_Compare_OK = OK;
	   //检查文件名
       if(stringcmp(EntryName,floder_name) != SUCC)
             Name_Compare_OK = unOK;        
       if(Name_Compare_OK == OK)  //检查文件扩展名
         {      
           if(CORE.FullPathType == FilePath && CORE.CurPathType == FilePath)
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
/*===============================================================================
函数
相对的路径转换成Sector,SectorOffset,Cluster
入口：u8 *filename
出口：SUCC,FAIL
===============================================================================*/
static u8 RelativePathToSectorCluster(u8 *RelativePath)
{ 
  u8 floder_name[256],file_extension[20]; 
  u8 Splited_Count;
  u8 Splited_Status;
   //调用SplitNameFromPath()从RelativePath分离出一个目录或文件元素
  Splited_Status = SplitNameFromPath(RelativePath,floder_name,file_extension,&Splited_Count);
  if(Splited_Status == FAIL)
      return(FAIL);
  RelativePath += Splited_Count;  
   //从目录中查找一个Entry
  if(FindEntryStruct(floder_name,file_extension) != SUCC)
  {
   return(FAIL); //查找失败退出
  }//DirectoryType置为非RootDirectory
   if(CORE.CurPathType == DirectoryPath)
     if(CORE.DirectoryType == RootDirectory)
	  {
	  CORE.DirectoryType = NoneRootDirectory; 
	  }
      
  if(Splited_Status == LastSplitedNameofPath)
  {
   return(SUCC); 
  }    
  do{ //调用SplitNameFromPath()从RelativePath分离出一个目录或文件元素
     Splited_Status = SplitNameFromPath(RelativePath,floder_name,file_extension,&Splited_Count);
     if(Splited_Status == FAIL)
	  return(FAIL);    
     else 
       { //设置变量,进入目录去查找
         CORE.ClusterNum = CORE.ClusterOfDirectoryEntry;
         CORE.SectorNum = FirstSectorofCluster(CORE.ClusterNum);
         CORE.offset = 0;  
       }
     RelativePath += Splited_Count;
	 //DirectoryType置为非RootDirectory
     if(CORE.CurPathType == DirectoryPath)
      if(CORE.DirectoryType == RootDirectory)
	  {
	  CORE.DirectoryType = NoneRootDirectory; 
	  }
	  //从目录中查找一个Entry
     if(FindEntryStruct(floder_name,file_extension) != SUCC)
	 {
       return(FAIL); //查找失败退出
	 }
     else if(Splited_Status == LastSplitedNameofPath)
	 {
      return(SUCC);
	 }  

    }while(1);
    return(SUCC);
}
/*===============================================================================
函数
完整的路径转换成Sector,SectorOffset,Cluster,可用于检查文件是否存在
入口：u8 *filename
出口：SUCC,FAIL
===============================================================================*/
static u8 FullPathToSectorCluster(u8 * filename1)
{
   u8 buf[260],* filename;
   stringcpy(filename1,buf);
   filename = buf;
   UPCASE(filename);

     if( ((* filename) >= 'A' && ( * filename ) <= 'Z')||
		((* filename) >= 'a' && ( * filename ) <= 'z') )  
       {
         if(( * (filename + 1)) == ':')
          {                          
           if( *( filename + 2 ) == '\\')
            { //绝对路径,从根目录进行转换
              if(LengthofString(filename) > Maximum_File_Path_Name)
			      return(EpathLengthsOVERFLOW);
              if(Read_partition_PBP((u8)((*filename) - 'C')) != SUCC)
                 return(FAIL);
              filename += 3; 
			  //设置参数,根目录寻址
              CORE.SectorNum = CORE.FirstRootDirSecNum; 
              CORE.DirectoryType =  RootDirectory;
              CORE.offset = 0;   
             }
          }
         else 
		 {   //相对路径,从当前目录进行转换  
             if((LengthofString(filename) + LengthofString(CORE.current_folder)) > Maximum_File_Path_Name)
                   return(EpathLengthsOVERFLOW);
             //设置参数
			 if(CORE.CurrentDirectoryType ==  RootDirectory)
			 {
			    CORE.SectorNum = CORE.FirstRootDirSecNum;
			 }  
             else
              {
                 CORE.ClusterNum = CORE.ClusterNOofCurrentFolder;
                 CORE.SectorNum = FirstSectorofCluster(CORE.ClusterNum); 
              }
             CORE.DirectoryType = CORE.CurrentDirectoryType;
             CORE.offset = 0;	 

          }  
       }
       else if((* filename) == '\\')//从当前盘符进行转换
            {
             if((LengthofString(filename) + 1) > Maximum_File_Path_Name)
                   return(EpathLengthsOVERFLOW); 
              //设置参数,根目录寻址
             filename ++;    
             CORE.SectorNum = CORE.FirstRootDirSecNum; 
             CORE.DirectoryType = RootDirectory;
             CORE.offset = 0;

            } 
  if(*filename)
     return(RelativePathToSectorCluster(filename));
  else
	return(SUCC);
   
}
/*===============================================================================
函数
检查文件是否已打开？
入口：FirstClusterOfFile-文件首簇号
出口：FileAlreadyopenedByOtherHandle,FileUnopenedByOtherHandle
===============================================================================*/ 
u8 Check_FileOpened_Status(u32 FirstClusterOfFile,u16 j)
{
  u8 i;
  for(i = 0;i < MaximumFCB;i++)
  {
    if(i == j)
		continue;
   if(FCB[i].file_openned_flag == UsedFlag)
   { //FCB的首簇号与文件首簇号相同,则文件已被打开
     if(FCB[i].FirstClusterOfFile == FirstClusterOfFile)
        return(FileAlreadyopenedByOtherHandle);
   }
  }
  //未发现相同首簇号,文件未打开,返回
  return(FileUnopenedByOtherHandle);
} 
/*===============================================================================
函数
将file buffer回写磁盘
入口：u8 FCBsn
出口：SUCC,FAIL
===============================================================================*/ 
#if EnableFileBuf
u8 Writeback_FCB_file_buffer(u8 FCBsn)
{ 
  u16 i;
  u16 ClusterQTY,BUFoffset,TEMP;
  u32 FCBbufSize,sector,NEXTCluster,qty; 
  u8  buf[512];
  u32 FILESIZE;
  u16 wrote_sectors_count;
  
  if(FCB[FCBsn].FirstClusterOfFile == 0)
  { //未分配文件首簇号，则执行分配
	if( Allocate_An_Empty_cluster(&FCB[FCBsn].FirstClusterOfFile,buf)== FAIL)
	  return(FAIL);
	//将首簇号写入文件的ENTRY内
	read_flash_sector(buf,FCB[FCBsn].Entry_Storedin_Sector);
    i = FCB[FCBsn].Entry_Storedin_Sector_Offset + 26;
	buf[i] = (u8)(FCB[FCBsn].FirstClusterOfFile & 0xff);
	buf[i+1] = (u8)((FCB[FCBsn].FirstClusterOfFile >> 8) & 0xff);
	write_flash_sector(buf,FCB[FCBsn].Entry_Storedin_Sector);
    FCB[FCBsn].CurClusterInBUF = FCB[FCBsn].FirstClusterOfFile;
  }
  //根据当前CurBlockInBUF计算需要移动CLUSTER数量
  ClusterQTY = FCB[FCBsn].CurBlockInBUF / BPB.sector_per_cluster;
  NEXTCluster = FCB[FCBsn].FirstClusterOfFile; 
  //计算需要移动CLUSTER数量
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
  //移动CLUSTER
  while(ClusterQTY)
  {TEMP = NEXTCluster;
   NEXTCluster =  Get_Next_Cluster_From_Current_Cluster(NEXTCluster);
   ClusterQTY--;
   if(NEXTCluster >= 0xfff4 && NEXTCluster <= 0xffff)
	   {
	    if (Allocate_EMPTY_CLUSTER_TO_CUR_CLUSTER_CHAIN(TEMP,&NEXTCluster,buf) == FAIL)
			return(FAIL);
	   }
  } 

  //保存当前CurClusterInBUF及ClusterSNInBUF
  FCB[FCBsn].CurClusterInBUF = NEXTCluster;
  FCB[FCBsn].ClusterSNInBUF = qty; 
  //将file buffer回写磁盘
  FCBbufSize = FileBUFSize * TotalFileBUFsQTYeachFCB; 
  BUFoffset = 0;
  i = FCB[FCBsn].CurBlockInBUF % BPB.sector_per_cluster;
  //获取首扇区
  sector = FirstSectorofCluster(NEXTCluster);
  wrote_sectors_count = 0; 
  do{ 
     write_flash_sector(FCB[FCBsn].FileBUF + BUFoffset,sector + i); 
     wrote_sectors_count++;
	 i++;
	 //超过FILE SIZE直接退出
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
     if(i >= BPB.sector_per_cluster)
	 { //超过簇边界, 转到下一簇继续写 
	   TEMP = NEXTCluster; 
       NEXTCluster = Get_Next_Cluster_From_Current_Cluster(NEXTCluster);
       if(NEXTCluster >= 0xfff4 && NEXTCluster <= 0xffff)
	   {
	    if (Allocate_EMPTY_CLUSTER_TO_CUR_CLUSTER_CHAIN(TEMP,&NEXTCluster,buf) == FAIL)
			return(FAIL);
	   }
	   sector = FirstSectorofCluster(NEXTCluster);
       i = 0;
      }    
  }while(1);
  //清除Modified_Flag
  FCB[FCBsn].Modified_Flag = 0;
  //读取文件目录项, 从而更新文件大小
  read_flash_sector(buf,FCB[FCBsn].Entry_Storedin_Sector);
  i = FCB[FCBsn].Entry_Storedin_Sector_Offset + 28;
  FILESIZE = buf[i] + buf[i+1] * 256 + buf[i+2] * 256 * 256+ buf[i+3]  * 256 * 256 *256;
  //检查文件大小是否已更新?
  if(FILESIZE != FCB[FCBsn].FileSize) 
  { //检查文件大小已更新,则更新File Directory entry的文件大小
	FILESIZE = FCB[FCBsn].FileSize;
	buf[i] = (u8)(FILESIZE & 0xff);
	buf[i+1] = (u8)((FILESIZE >> 8) & 0xff);
	buf[i+2] = (u8)((FILESIZE >> 16) & 0xff);
	buf[i+3] = (u8)((FILESIZE >> 24) & 0xff);
    write_flash_sector(buf,FCB[FCBsn].Entry_Storedin_Sector);
  }
  return(SUCC);
}
#endif 

/*===============================================================================
函数
基于文件当前的位置，完成更新文件缓冲区
入口：u8 FCBsn
出口：SUCC,FAIL
===============================================================================*/ 
#if EnableFileBuf
u8 Update_FCB_file_buffer(u8 FCBsn)
{ 
  u16 i;
  u16 ClusterQTY,BUFoffset;
  u32 FCBbufSize,sector,NEXTCluster,qty,previous_cluster; 
  if(!(FileBUFSize * TotalFileBUFsQTYeachFCB))
     return(SUCC);
  if(((FCB[FCBsn].cur_position) >= FCB[FCBsn].CurBlockInBUF * FileBUFSize && 
     ((FCB[FCBsn].cur_position) <  (FCB[FCBsn].CurBlockInBUF + TotalFileBUFsQTYeachFCB) * FileBUFSize) ))
  {
  return(SUCC); 
  }
  if(FCB[FCBsn].Modified_Flag)//检查Modified_Flag,如果不为0,则需要回写FILE BUFFER
    if(Writeback_FCB_file_buffer(FCBsn) == FAIL)
	  return(FAIL);
  //根据当前CurBlockInBUF计算需要移动CLUSTER数量
  FCB[FCBsn].CurBlockInBUF = FCB[FCBsn].cur_position / FileBUFSize;
  ClusterQTY = FCB[FCBsn].CurBlockInBUF / BPB.sector_per_cluster;
  NEXTCluster = FCB[FCBsn].FirstClusterOfFile; 
  qty = ClusterQTY;
  
  //计算需要移动CLUSTER数量  
  if(FCB[FCBsn].ClusterSNInBUF <= ClusterQTY)
  { 
    ClusterQTY -= FCB[FCBsn].ClusterSNInBUF;
    NEXTCluster = FCB[FCBsn].CurClusterInBUF;
  }
 else
 {
    NEXTCluster = FCB[FCBsn].FirstClusterOfFile; 
  } 

  //移动CLUSTER
  while(ClusterQTY)
  {previous_cluster = NEXTCluster;
   if(NEXTCluster >= 0xfff4 && NEXTCluster <= 0xffff)
	   return(SUCC);
   NEXTCluster =  Get_Next_Cluster_From_Current_Cluster(NEXTCluster);
   ClusterQTY--;
   
  } 
   if(!(NEXTCluster >= 0xfff4 && NEXTCluster <= 0xffff))
   {
           FCB[FCBsn].CurClusterInBUF = NEXTCluster;
           FCB[FCBsn].ClusterSNInBUF = qty;  
   }
   else
   {
           FCB[FCBsn].CurClusterInBUF = previous_cluster;
           FCB[FCBsn].ClusterSNInBUF = qty-1; 
		   return(SUCC);
   
   }


  FCBbufSize = FileBUFSize * TotalFileBUFsQTYeachFCB; 
  BUFoffset = 0;
  i = FCB[FCBsn].CurBlockInBUF % BPB.sector_per_cluster;
    //获取首扇区
  sector = FirstSectorofCluster(NEXTCluster);
  do{  
   read_flash_sector(FCB[FCBsn].FileBUF + BUFoffset,sector + i); 
   BUFoffset += FileBUFSize;
   FCBbufSize -= FileBUFSize;
   if(FCBbufSize == 0)
    {
     return(SUCC); 
    }
   i++;
   if(i >= BPB.sector_per_cluster)
     {//超过簇边界, 转到下一簇继续读 
       NEXTCluster= Get_Next_Cluster_From_Current_Cluster(NEXTCluster);
       if(NEXTCluster >= 0xfff4 && NEXTCluster <= 0xffff)
	   return(SUCC);
	   sector = FirstSectorofCluster(NEXTCluster);
       i = 0;
      }    
  }while(1);
}
#endif 

/*===============================================================================
函数
分配FCB
入口：无
出口：EAllocate_FCB-- Fail,other--SUCC with FCB sequential number
===============================================================================*/ 
u8 Allocate_FCB(void)
{  
 u8 i;
 for (i = 0; i < MaximumFCB;i++)//找一个UnusedFlag的FCB,返回给调用者使用
  if (FCB[i].file_openned_flag == UnusedFlag)  
   {          
    FCB[i].file_openned_flag = UsedFlag;//设置为占用
	FCB[i].Modified_Flag = 0;
    return(i);//返回FCB的编号
   }
 return(EAllocate_FCB);
}
/*===============================================================================
函数
释放一个FCB
入口：FCB_sn
出口：EFree_FCB,SUCC
===============================================================================*/     
u8 Free_FCB(u8 FCBsn)
{//检查FCB是否占用
 if(FCB[FCBsn].file_openned_flag == UsedFlag)
  {
    FCB[FCBsn].file_openned_flag = UnusedFlag;//设置为非占用
	FCB[FCBsn].Modified_Flag = 0;
    return(SUCC);
  }
  else
   return(EFree_FCB);
}
/*===============================================================================
函数
FAT初始化函数,一般由main()在使用文件系统之前调用
入口：无
出口：无
===============================================================================*/ 
#if complie_FAT_filesystem_initialiation
u8 FAT_filesystem_initialiation(void)
{ 
  u8 root[] = "C:\\",i;
  CORE.PartitionID = 0xff;
  CORE.CurrentDirectoryType =  RootDirectory; 
  //初始化当前目录为'C:\'
  stringcpy(root,CORE.current_folder);
  //所有FCB设置为非占用
  for (i = 0; i < MaximumFCB;i++)
  {
   FCB[i].file_openned_flag = UnusedFlag; 
   FCB[i].Modified_Flag = 0;
  }
  //读取缺省的PBP和相关的信息
  return(Read_partition_PBP(0)); 
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
  write_flash_sector(buf,i);
 //run block erase, clr root/fat1/2 region
 //while(block_erase(1,1+SDs_BlockNum-2)== FAIL);

 buf[0] = 0xf8;buf[1] = 0xff;buf[2] = 0xff;buf[3] = 0xff;
 write_flash_sector(buf,1); 
 write_flash_sector(buf,257); 

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
 write_flash_sector((U8 *)(&MBR),0);  
 //write_root();
 write_fat(disk_);
}
  
/*===============================================================================
函数
自动格式化 FAT 文件系统
入口：disk_:盘符   filesystem_type:以FAT16,FAT32为格式化目标  disk_capacity:磁盘容量
出口：SUCC,FAIL
===============================================================================*/ 
#if complie_FAT16_filesystem_autoformat
u8 FAT_filesystem_autoformat(u8 disk_,u8 filesystem_type,u32 disk_capacity)
{
 u8 buf[512];
 if( read_flash_sector(buf,0) == SUCC ) //read MBR
 {   
     if ( ! ( buf[510] == 0x55 && buf[511] == 0xaa))
	  {  
		  
		 if(filesystem_type== FAT16)
	    	  fill_mbr_and_write_fat_root(disk_capacity,disk_);
         else
		 {

		 }
	  
	  }
    }
 return(SUCC); 
} 
#endif
   

/*===============================================================================
函数 
打开文件
入口：u8 * filename:路径+文件名
出口：返回FCB_SN,FAIL
===============================================================================*/   
#if complie_open_file 
u8 open_file(u8 * filename)
{ 
 u8 FCBsn;
 CORE.FullPathType = FilePath; 
 //检查文件是否存在?
 if(FullPathToSectorCluster(filename) == SUCC)
 {   //有这个文件,则执行FCB分配
     FCBsn = Allocate_FCB();
     if(FCBsn == EAllocate_FCB)
       return(FAIL);
     //初始化FCB的各个参数
	 FCB[FCBsn].cur_position = 0;
	 FCB[FCBsn].CurBlockInBUF = 0xffff;
	 FCB[FCBsn].FirstClusterOfFile = CORE.ClusterOfDirectoryEntry;

     FCB[FCBsn].CurClusterInBUF = FCB[FCBsn].FirstClusterOfFile;
     FCB[FCBsn].ClusterSNInBUF = 0;


     FCB[FCBsn].FileSize = CORE.FileSize;
     FCB[FCBsn].Entry_Storedin_Sector = CORE.PreEntrySectorNum ;  //Save sectorNUM of File Directory entry for current file
     FCB[FCBsn].Entry_Storedin_Sector_Offset = CORE.PreEntryoffset; //Save offset in sector of File Directory entry for current file
     FCB[FCBsn].Modified_Flag = 0;
	 //检查文件是否已打开?
	 if(Check_FileOpened_Status(CORE.ClusterOfDirectoryEntry,FCBsn) == FileAlreadyopenedByOtherHandle)
         FCB[FCBsn].Permission = ReadOnlyPermission;
	 else
         FCB[FCBsn].Permission = FullPermission;
   //调用Update_FCB_file_buffer(),将文件读取到文件缓冲区
   #if EnableFileBuf
     Update_FCB_file_buffer(FCBsn);
   #endif 
     return(FCBsn); 
   }
  else
   return(FAIL);//文件不存在,文件打开失败
}
#endif 
/*===============================================================================
函数 
关闭文件
入口：FCBsn
出口：SUCC,FAIL
===============================================================================*/  
#if complie_close_file
u8 close_file(u8 FCBsn)
{  
  if(Free_FCB(FCBsn) == SUCC)//释放FCB
    return(SUCC);
  else
    return(FAIL);
} 
#endif 
 
/*===============================================================================
函数
Directory Entry offset+32 由建立文件/目录使用，遇末簇可自动分配空簇加入簇链
入口：buf--
出口：SUCC,FAIL
===============================================================================*/             
static u8 CORE_offset_add_32_With_EMPTY_CLUSTER_ALLOCATION(u8 *buf)
{
  u32 temp;
  //位置+32
  CORE.offset += 32;
  if (CORE.offset >= 512)
  {//位置越过一个扇区
   CORE.SectorNum++;
  if (CORE.DirectoryType == RootDirectory)
  {     //针对根目录的处理
        if (CORE.SectorNum < ( CORE.RootDirSectors +  CORE.FirstRootDirSecNum))
         {
           
           CORE.offset = 0; 
           read_flash_sector(buf,CORE.SectorNum);
           return(SUCC);
         }
        else
           return(FAIL);
     }
    else  
     {//位置越过一个簇, 取下一簇,继续处理
        if( (CORE.SectorNum - FirstSectorofCluster(CORE.ClusterNum) + 1) > BPB.sector_per_cluster)
         {
		   temp  = CORE.ClusterNum;
           CORE.ClusterNum = Get_Next_Cluster_From_Current_Cluster(CORE.ClusterNum);
           if( CORE.ClusterNum >= 2 && CORE.ClusterNum < 0xfff7)
            {
               CORE.SectorNum = FirstSectorofCluster(CORE.ClusterNum); 
               CORE.offset = 0;
               read_flash_sector(buf,CORE.SectorNum);
               return(SUCC);
            }
           else
		   {   //遇末簇，分配空簇加入簇链 
			   if(Allocate_EMPTY_CLUSTER_TO_CUR_CLUSTER_CHAIN(temp,&temp,buf) == FAIL)
				   return(FAIL);
               CORE.ClusterNum = temp;
               CORE.SectorNum = FirstSectorofCluster(CORE.ClusterNum); 
               CORE.offset = 0;

               read_flash_sector(buf,CORE.SectorNum);
			   return(SUCC);
		   }
         }
        else
         {
            CORE.offset = 0;
            read_flash_sector(buf,CORE.SectorNum);
            return(SUCC);
         }
     }
  }
  return(SUCC);
}
/*===============================================================================
函数 
计算长文件目录项的校验和
入口：*entry:longfilenameentry首地址
出口：checksum
===============================================================================*/
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
/*===============================================================================
函数 
在目录中寻找给定数量DIRECTORY ENTRY的空间
入口：Entry_Resuested_QTY-ENTRY的数量
出口：SUCC,FAIL
===============================================================================*/
static u8 Seek_Space_to_Write_Directory_Entry(u16 Entry_Resuested_QTY,u8 *buf)
{
  u32 SectorNum_LOCAL,ClusterNum_LOCAL;
  u16 Offset_LOCAL;
  u8 Empty_Entry_Space_Count,found_flag;

  //根目录和非根目录的参数装载
  if(CORE.DirectoryType == RootDirectory)
  {
    CORE.SectorNum = CORE.FirstRootDirSecNum;
  }
  else
  { 
    CORE.ClusterNum = CORE.ClusterOfDirectoryEntry;
    CORE.SectorNum = FirstSectorofCluster(CORE.ClusterNum);
  } 
  CORE.offset = 0;
  read_flash_sector(buf,CORE.SectorNum);
  while(1)
  { 
    SectorNum_LOCAL = CORE.SectorNum; // 保存当前ENTRY的首位置
    ClusterNum_LOCAL = CORE.ClusterNum;
    Offset_LOCAL = CORE.offset;
	found_flag = 0;
	Empty_Entry_Space_Count = 0;
    do{
       if(buf[CORE.offset] == 0xe5 || buf[CORE.offset] == 0)//空ENTRY
	   {
	    Empty_Entry_Space_Count++;
		//直到找到指定数量的空ENTRY
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
    if(found_flag)//找到指定数量的空ENTRY
	{ 
	 CORE.SectorNum = SectorNum_LOCAL; //置空ENTRY的首位置
     CORE.ClusterNum = ClusterNum_LOCAL;
     CORE.offset = Offset_LOCAL;
     break;
	}
	else if(CORE_offset_add_32_With_EMPTY_CLUSTER_ALLOCATION(buf) == FAIL)
	   return(FAIL);
  }   
 return(SUCC); 

}
/*===============================================================================
函数 
填写一个LFN directory entry
入口：u8 * Directory_Entry_buf,u8 * Entry_Name,u8 LFN_record_FLAG
      u8 Entry_Name,u8 checksum
出口：SUCC,FAIL
===============================================================================*/
static u8 Fill_LFN_Directory_Entry(u8 * Directory_Entry_buf,u8 * Entry_Name,u8 LFN_record_FLAG,u8 checksum)
{
  u8 i; 
  i = LengthofString(Entry_Name); 
  if(i < 12) 
  {
	  do{          //填写空白空间
		i++;
		if(i >= 13)
			break;
        Entry_Name[i] = 0xff;
	  }while(1);
  }
  Directory_Entry_buf[0] = LFN_record_FLAG;;
  //填写前5个字节
  for(i = 0;i < 5;i++)
  {
	Directory_Entry_buf[1 + i * 2] = Entry_Name[i];
	if(Entry_Name[i] != 0xff)
	  Directory_Entry_buf[2 + i * 2] = 0;
	else 
      Directory_Entry_buf[2 + i * 2] = 0xff;
  }
  Directory_Entry_buf[11] = ATTR_LONG_NAME; //0x0F, impossible file attribute used as signature
  Directory_Entry_buf[12] = 0;    //Reserved (?). Set to 0x00
  Directory_Entry_buf[13] = checksum;//填写校验和
  //填写中间6个字节
  for(i = 0;i < 6;i++)
  {
	Directory_Entry_buf[14 + i * 2] = Entry_Name[5 + i];
	if(Entry_Name[5 + i] != 0xff)
	  Directory_Entry_buf[15 + i * 2] = 0;
	else
      Directory_Entry_buf[15 + i * 2] = 0xff;
  }
  Directory_Entry_buf[26] = 0;  //First cluster number (always 0x0000 for long filename records)
  Directory_Entry_buf[27] = 0;
  //填写2个字节
  for(i = 0;i < 2;i++)
  {
	Directory_Entry_buf[28 + i * 2] = Entry_Name[11 + i];
	if(Entry_Name[11 + i] != 0xff)
	  Directory_Entry_buf[29 + i * 2] = 0;
	else
      Directory_Entry_buf[29 + i * 2] = 0xff;
  }
  return(SUCC);
}
/*===============================================================================
函数 
填写短文件directory entry
入口：u8 * Directory_Entry_buf,u8 * Entry_Name,u32 first_cluster,
      u32 FileSize,u8 attr
出口：SUCC,FAIL
===============================================================================*/
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
          if(*Entry_Name == '.')//遇到'.',转入填写File extension
             {
              flag  = FILE_EXTENSION;
			  Entry_Name++;
             }  
          else
            {
             Directory_Entry_buf[i] =  *Entry_Name;//填写Entry Name
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
           Directory_Entry_buf[8+j] =  *Entry_Name;//填写File extension
          j++;
          Entry_Name++;
         } 
      }
	  else
		 break;
 }while(1);
 Directory_Entry_buf[11] = attr;
 Directory_Entry_buf[12] = 0;
 Directory_Entry_buf[26] = (u8)(first_cluster & 0xff);  //填写首簇号
 Directory_Entry_buf[27] = (u8)((first_cluster >> 8) & 0xff);
 Directory_Entry_buf[28] = (u8)(FileSize & 0xff);////填写文件长度
 Directory_Entry_buf[29] = (u8)((FileSize >> 8) & 0xff);
 Directory_Entry_buf[30] = (u8)((FileSize >> 16) & 0xff);
 Directory_Entry_buf[31] = (u8)((FileSize >> 24) & 0xff);
 return(SUCC);
}
/*===============================================================================
函数 
把Longfilename Directory Entry转换为short filename
入口：Directory_Entry:Directory_Entry name,attr:attr of Directory_Entry
出口：SUCC,FAIL
===============================================================================*/
static u8 LFN_convert_to_SFN(u8 * Directory_Entry,u8 * SFN_Directory_Entry_buf)
{
 u8 i,flag,j;
 for(i = 0;i < 11;i++)
   SFN_Directory_Entry_buf[i] = 0x20; //初始化DIRECTORY ENTRY
  i = 0; 
  j = 0;
  flag = FILE_NAME;
  do{           
     if( ( * Directory_Entry) != 0) 
       { 
        if( flag == FILE_NAME)
         {            
          if(*Directory_Entry == '.')//遇到'.',转入填写File extension
             {
              flag  = FILE_EXTENSION;
			  Directory_Entry++;
             }  
          else
            {
             SFN_Directory_Entry_buf[i] =  *Directory_Entry;//填写Entry Name
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
			 }
             Directory_Entry++;
            }
          }
        else if( flag  == FILE_EXTENSION)
         {
          if( j >= 3 )
            return(FAIL);//填写File extension
          SFN_Directory_Entry_buf[8+j] =  *Directory_Entry;
          j++;
          Directory_Entry++;
         } 
      }
	  else
		  return(SUCC);
 }while(1);
}

/*===============================================================================
函数 
将写Longfilename Directory Entry入磁盘
入口：Directory_Entry:Directory_Entry name,attr:attr of Directory_Entry
出口：SUCC,FAIL
===============================================================================*/
static u8 Write_LongFileName_Entry(u8 * Directory_Entry,u8 attr,u32 first_cluster,u8 * buf,u32 FileSize)
{
  u16 len;
  u8 i,checksum;
  u8 Directory_Entry_buf[32];
  u8 SN,LFN_record_FLAG;
  //计算需要的Directory_Entry空间，每个Directory_Entry空间为32个字节,计算结果放在len中
  len = LengthofString(Directory_Entry) / 13;
  if(LengthofString(Directory_Entry) % 13)
	len++;
  SN = len; 
  //从long-filename directory entry处理得其对应的short-filename directory entry
  LFN_convert_to_SFN(Directory_Entry,Directory_Entry_buf);
  //计算short-filename directory entry校验和
  checksum = calculate_checksum_longfilenameentry(Directory_Entry_buf);
  //处理last long-filename directory entry for file
  read_flash_sector(buf,CORE.SectorNum);    
  LFN_record_FLAG = SN | (u8)Last_LFN_Record;
  Fill_LFN_Directory_Entry(buf + CORE.offset,Directory_Entry + (len - 1) * 13,LFN_record_FLAG,checksum);
  len--;
  write_flash_sector(buf,CORE.SectorNum);
  CORE_offset_add_32(buf);
  LFN_record_FLAG = SN;
  //处理其它LFN directory entry
  //do{
  while(len){
   SN--;
   Directory_Entry[len * 13] = 0;
   Fill_LFN_Directory_Entry(buf + CORE.offset,Directory_Entry + (len - 1) * 13,SN,checksum);
   len--;
   write_flash_sector(buf,CORE.SectorNum);
   CORE_offset_add_32(buf);
  }

  
  //处理LFN 相应的short-filename directory entry
  Directory_Entry_buf[11] = attr;
  Directory_Entry_buf[12] = 0;
  Directory_Entry_buf[26] = (u8)(first_cluster & 0xff);  //填写首簇号
  Directory_Entry_buf[27] = (u8)((first_cluster >> 8) & 0xff);
  Directory_Entry_buf[28] = (u8)(FileSize & 0xff);////填写文件长度
  Directory_Entry_buf[29] = (u8)((FileSize >> 8) & 0xff);
  Directory_Entry_buf[30] = (u8)((FileSize >> 16) & 0xff);
  Directory_Entry_buf[31] = (u8)((FileSize >> 24) & 0xff); 
  for(i = 0;i < 32;i++)
	{
	  buf[CORE.offset + i] = Directory_Entry_buf[i];
	}
  write_flash_sector(buf,CORE.SectorNum);
  return(SUCC);
}
/*===============================================================================
函数 
添加Directory_Entry至目录
入口：Directory_Entry-
出口：SUCC,FAIL
===============================================================================*/
static u8 Add_A_Directory_Entry_(u8 * Directory_Entry,u8 attr,u32 first_cluster,u32 FileSize)
{
	u16 len;
	u8 buf[512];
	u8 Directory_Entry_buf[32];
	u8 temp,i;//,j;
    
	if(attr & ATTR_DIRECTORY)
	{//给新建的directory分配簇号再写入默认两个目录，“.”和“..”
	  u32 sector_local;
	  if( Allocate_An_Empty_cluster(&first_cluster,buf)== FAIL)
	  	  return(FAIL);
	  //初始化默认两个目录，目录名分别取名为“.”和“..”
	  sector_local =  FirstSectorofCluster(first_cluster);
      for(i = 0;i < 11;i++)
		 buf[i] = 0x20; 
	  buf[0] = '.';//填写目录名“.”
	  buf[11] = attr;
	  buf[12] = 0;
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
	  buf[32+26] = (u8)(CORE.ClusterNum & 0xff);  //填写当前目录父目录的首簇号
	  buf[32+27] = (u8)((CORE.ClusterNum >> 8) & 0xff);
	  buf[32+28] = 0;////填写文件长度
	  buf[32+29] = 0;
	  buf[32+30] = 0;
	  buf[32+31] = 0;
	  buf[64] = 0;
	  write_flash_sector(buf,sector_local);
	}
	//计算Directory_Entry需要多少个空间，Directory_Entry每个空间为32个字节,计算结果放在len中	
	len = LengthofString(Directory_Entry) / 13;
	if(LengthofString(Directory_Entry) % 13)
		len++;
	if(len > 1)
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
	//寻找指定数量的空间
	if(Seek_Space_to_Write_Directory_Entry(len,buf) == SUCC)
	{ 
	  if(len == 1)  //写短文件名
	  {
		UPCASE(Directory_Entry);
		Fill_SFN_Directory_Entry(Directory_Entry_buf,Directory_Entry, first_cluster,FileSize,attr);
		read_flash_sector(buf,CORE.SectorNum);
		for(temp = 0;temp < 32;temp++)
		{ 
		  buf[CORE.offset + temp] = Directory_Entry_buf[temp];
		}
		write_flash_sector(buf,CORE.SectorNum);
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

/*===============================================================================
函数 
建立文件
入口：无
出口：无
===============================================================================*/  
#if complie_create_file
u8 create_file(u8 * filename)
{ 
 u16 len;
 u16 temp;
 u8 buf[260],status;
 stringcpy(filename,buf);
 CORE.FullPathType = FilePath;
 if(FullPathToSectorCluster(filename) != SUCC)  //检查文件是否已经存在
 {   //文件不存在,才可以建文件
     len = LengthofString(buf);
	 //从文件名中分离路径
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
     CORE.FullPathType = DirectoryPath; 
	 if(!temp)
	 {  //相对路径,当前目录下建文件
	   if(FullPathToSectorCluster(CORE.current_folder) == FAIL)
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
	  //根目录下建文件
	  if(FullPathToSectorCluster(TEMP) == FAIL)
	  {
	   return(FAIL);
	  }		  
	 }
	 else 
	 {
	   if(FullPathToSectorCluster(buf) == FAIL)
		 return(FAIL);
	 }
	 //文件写入磁盘
	 status = Add_A_Directory_Entry_(buf+temp,ATTR_ARCHIVE,0,0);
	 return(status);
  }
  else
	 return(FAIL);
}
#endif
/*===============================================================================
函数
建立目录
入口：foldername--目录的路径
出口：无
===============================================================================*/
#if complie_create_floder
u8 create_floder(u8 * foldername)
{  
 u16 len;
 u16 temp;
 u8 TEMP[4];
 u8 buf[260],status;
 stringcpy(foldername,buf);

 CORE.FullPathType = DirectoryPath; 
 if(FullPathToSectorCluster(buf) != SUCC)  //检查目录是否已经存在
  {//目录不存在,才可以建目录
     len = LengthofString(buf);
	 //从目录名中分离路径
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
     CORE.FullPathType = DirectoryPath; 
	 if( ! temp)
	 { CORE.FullPathType = DirectoryPath; //相对路径,当前目录下建目录
	   if(FullPathToSectorCluster(CORE.current_folder) == FAIL)
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
	  CORE.FullPathType = DirectoryPath; 
	  //根目录下建目录
	  if(FullPathToSectorCluster(TEMP) == FAIL)
		 	  {
	   return(FAIL);
	  }
	 }
	 else if(FullPathToSectorCluster(buf) == FAIL)
	 { 
	   return(FAIL);  
	  }
	 //目录写入磁盘
	 status = Add_A_Directory_Entry_(buf + temp,ATTR_DIRECTORY,0,0);
     return(status);
  }
  else
    return(FAIL);


}
#endif 

/*===============================================================================
函数
当前操作位置设定函数
入口：无
出口：无
===============================================================================*/ 
#if compile_fseek
u8 f_seek(u8 FCBsn, s32 offset, u8 origin)
{     
 if(FCB[FCBsn].file_openned_flag == UnusedFlag)
    return(FAIL);//FCB未占用,返回FAIL
 switch(origin)
  { 
   case SEEK_SET:  //相对于文件开始位置寻址 
    { 
      if(offset < 0 || (u32)offset >= FCB[FCBsn].FileSize)
        return(FAIL);
      FCB[FCBsn].cur_position = offset; 
    //根据文件的当前位置更新文件缓冲
    #if EnableFileBuf
      Update_FCB_file_buffer(FCBsn); 
    #endif
      break;
    }
   case SEEK_CUR: //相对于文件当前位置寻址 
    { 
      if(((FCB[FCBsn].cur_position + offset) < 0 )|| 
        ((FCB[FCBsn].cur_position + offset)  >= FCB[FCBsn].FileSize)) 
         return(FAIL); 
      FCB[FCBsn].cur_position += offset;
     //根据文件的当前位置更新文件缓冲
     #if EnableFileBuf
      Update_FCB_file_buffer(FCBsn);
     #endif
      break;
    }
   case SEEK_END: //相对于文件结束位置寻址 
    {
      if(offset > 0)
       return(FAIL);
       FCB[FCBsn].cur_position = FCB[FCBsn].FileSize + offset;
	   //根据文件的当前位置更新文件缓冲
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
/*===============================================================================
函数
读文件
入口：u8 FCBsn,u8* buffer,u16 length
出口：读取的字节数
===============================================================================*/ 
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
     return(0); //FCB未占用,返回FAIL
   readed_bytes = 0;//已读取的字节数置0
   if((FCB[FCBsn].cur_position + length) > FCB[FCBsn].FileSize)
   { //如果读取的位置已至尾,读取length个字符超过了可读取的字符数.
	 //length置为当前可以读取的字符数
     length = FCB[FCBsn].FileSize - FCB[FCBsn].cur_position ;
	 if(!length)
		 return(readed_bytes);
   }  
   OffsetInbuf = (FCB[FCBsn].cur_position % (FileBUFSize * TotalFileBUFsQTYeachFCB));
   if(!OffsetInbuf)
   {//根据文件的当前位置更新文件缓冲
     Update_FCB_file_buffer(FCBsn);
   }
   temp = FileBUFSize * TotalFileBUFsQTYeachFCB - OffsetInbuf;
   point =  FCB[FCBsn].FileBUF + OffsetInbuf;
   if(temp >= length)
   { //length字节的数据可以直接从缓冲区中读数据
     for(i = 0;i < length ;i++)
       buffer[i] = point[i];
     FCB[FCBsn].cur_position += length;
     return(i); //读取结束
   }
   else 
   { //直接从缓冲区中读数据
     for(i = 0;i < temp ;i++)
       buffer[i] = point[i];
     FCB[FCBsn].cur_position += temp;
	 readed_bytes += i;
	 buffer += temp;
     length -= temp;
   }   
   do{
      Update_FCB_file_buffer(FCBsn);//根据文件的当前位置更新文件缓冲
      if(length <= FileBUFSize * TotalFileBUFsQTYeachFCB)
	  {  //length字节的数据可以直接从缓冲区中读数据,读取结束
        for(i = 0;i < length ;i++)
          buffer[i] = FCB[FCBsn].FileBUF[i];
        FCB[FCBsn].cur_position += length;
	    readed_bytes += i; 
        return(readed_bytes); //读取结束	 
	  }  
	  else
	  { //直接从缓冲区中读数据
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
    return(0); //FCB未占用,返回FAIL
   for(i = 0;i < length ;i++)
   {                                   
	  if(FCB[FCBsn].cur_position >= FCB[FCBsn].FileSize)
        return(i);
	  OffsetInbuf = (u16)(FCB[FCBsn].cur_position % (FileBUFSize * TotalFileBUFsQTYeachFCB));
      if ( ! OffsetInbuf)
        Update_FCB_file_buffer(FCBsn);  //根据文件的当前位置更新文件缓冲
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
/*===============================================================================
函数
写文件
入口：无
出口：无
===============================================================================*/    
#if complie_write_file
u16 write_file(u8 FCBsn,u8* buffer, u16 length)
{  
  #if EnableFileBuf
   #if Write_File_Optimization_Selector
   u16 temp;
   u32 writed_bytes;
   u16 i,OffsetInbuf;
   u8 *point;     
   if(FCB[FCBsn].file_openned_flag == UnusedFlag)
     return(0);//FCB未占用,返回FAIL
   if(FCB[FCBsn].Permission == ReadOnlyPermission || length == 0)
	 return(0); //如果文件为只读, 则返回
   writed_bytes = 0;
   OffsetInbuf = (FCB[FCBsn].cur_position % (FileBUFSize * TotalFileBUFsQTYeachFCB));
   if(!OffsetInbuf)
	  {  
         if(Update_FCB_file_buffer(FCBsn) == FAIL) 
			 return(0);//将file buffer回写磁盘,再更新FILE BUFFER
	  } 
   temp = FileBUFSize * TotalFileBUFsQTYeachFCB - OffsetInbuf;
   if(temp >= length)
   { //直接向缓冲区中写数据,并回写磁盘
	 point =  FCB[FCBsn].FileBUF + OffsetInbuf;
     for(i = 0;i < length ;i++)
       point[i] = buffer[i];
     FCB[FCBsn].cur_position += length;
	 FCB[FCBsn].Modified_Flag = 1;
	 if(FCB[FCBsn].cur_position >= FCB[FCBsn].FileSize)
		  FCB[FCBsn].FileSize = FCB[FCBsn].cur_position ;
     if(Writeback_FCB_file_buffer(FCBsn) == FAIL)  
	    i = 0;//将file buffer回写磁盘,再更新FILE BUFFER
     return(i);
   }
   else
   { //直接向缓冲区中写数据,
	 point =  FCB[FCBsn].FileBUF + OffsetInbuf;
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
     if(Update_FCB_file_buffer(FCBsn) == FAIL) 
	    return(writed_bytes);// 将file buffer回写磁盘,再更新FILE BUFFER
     if(length <= FileBUFSize * TotalFileBUFsQTYeachFCB)
	 {//将剩余的数据直接向缓冲区中写,并回写磁盘
      for(i = 0;i < length ;i++)
        FCB[FCBsn].FileBUF [i] = buffer[i];
      FCB[FCBsn].cur_position += length;
	  FCB[FCBsn].Modified_Flag = 1; 
	  if(FCB[FCBsn].cur_position >= FCB[FCBsn].FileSize)
		  FCB[FCBsn].FileSize = FCB[FCBsn].cur_position ;
      if(Writeback_FCB_file_buffer(FCBsn) == FAIL) 
	    i = 0;// 将file buffer回写磁盘,再更新FILE BUFFER
	  writed_bytes += i; 
      return(writed_bytes);	 
	 }
	 else
	 {//直接向缓冲区中写数据,写满一个缓冲区
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
     return(0);//FCB未占用,返回FAIL
   if(FCB[FCBsn].Permission == ReadOnlyPermission)
	 return(0);//如果文件为只读, 则返回
   for(i = 0;i < length ;i++)
   {                                    
    OffsetInbuf = (u16)(FCB[FCBsn].cur_position % (FileBUFSize * TotalFileBUFsQTYeachFCB));
	if(!OffsetInbuf)
	  { 
         if(Update_FCB_file_buffer(FCBsn) == FAIL)
			 return(0);// 将file buffer回写磁盘,再更新FILE BUFFER
	  } 
      FCB[FCBsn].FileBUF [OffsetInbuf] = buffer[i];
	  if(!FCB[FCBsn].Modified_Flag)
	     FCB[FCBsn].Modified_Flag = 1 ;    
      if(FCB[FCBsn].cur_position >= FCB[FCBsn].FileSize)
		  FCB[FCBsn].FileSize = FCB[FCBsn].cur_position + 1;
	  FCB[FCBsn].cur_position ++;
   }
   if(Writeback_FCB_file_buffer(FCBsn) == FAIL)  
	   i = 0;// 将file buffer回写磁盘,再更新FILE BUFFER
   return(i);
  #endif
 #else
  return(i);
 #endif
}   
 #endif
/*===============================================================================
函数
改变当前目录
入口：foldername:目录名，
      mode: 0-- 进入目录; >0--返回上层目录
出口：SUCC,FAIL
===============================================================================*/
#if compile_cd_folder
u8 cd_folder(u8 *foldername,u8 mode)
{ 
  u16 offset;
  if(mode)  //mode=1，返回上层目录
   {
    if (CORE.CurrentDirectoryType == RootDirectory)//如果当前目录为根目录，则返回上层目录失败
     return(0x55);
    else
    {
	 CORE.FullPathType = DirectoryPath;
     if(FullPathToSectorCluster(CORE.current_folder) != SUCC)
	   return(FAIL);
     //寻找目录分隔符＇＼＇,将当前目录减去一个目录
	 offset = LengthofString(CORE.current_folder);
     offset --;
     do{
      if(CORE.current_folder[offset] != '\\')
		  CORE.current_folder[offset] = 0;
      else
      {
	   if(CORE.current_folder[offset-1] == ':')
		 break;
       CORE.current_folder[offset] = 0;
       break;
      }
       offset--;
     }while(1);        

     if(LengthofString(CORE.current_folder) <= 3)
       CORE.CurrentDirectoryType = RootDirectory;
	 else
       CORE.CurrentDirectoryType = NoneRootDirectory;
     return(SUCC);
    }
   }
   else  //进入子目录
   {
    CORE.FullPathType = DirectoryPath;      
    if(FullPathToSectorCluster(foldername) == SUCC)//检查目录是否存在？
	{ 
	  if((* foldername) >= 'C' && ( * foldername ) <= 'Z')
	  {
       if(* (foldername + 1) == ':' &&  * (foldername + 2 ) == '\\')
		  {//绝对路径，直接拷入CORE.current_folder
           stringcpy(foldername,CORE.current_folder);  
		  }
		  else
		  { //相对路径，与CORE.current_folder连接后，拷贝进入CORE.current_folder
			if(LengthofString(CORE.current_folder) != 3)
			{
			 CORE.current_folder[LengthofString(CORE.current_folder)] = '\\';
             CORE.current_folder[LengthofString(CORE.current_folder) + 1] = 0;
			}
		    concanenateString(CORE.current_folder,foldername);
		  }
	  }
	  else if(*foldername == '\\')
	  {
	      stringcpy(foldername,CORE.current_folder + 1);
	  }
	//保存目录的相关参数
	 if(LengthofString(CORE.current_folder) <= 3)
	 {
      CORE.SectorNum = CORE.FirstRootDirSecNum; 
	  CORE.CurrentDirectoryType = RootDirectory; 
	 }
	 else
	 {
	   CORE.CurrentDirectoryType = NoneRootDirectory;
       CORE.ClusterNum = CORE.ClusterOfDirectoryEntry;
	   CORE.ClusterNOofCurrentFolder = CORE.ClusterOfDirectoryEntry;
       CORE.SectorNum =  FirstSectorofCluster(CORE.ClusterNum); 
	 }
	 CORE.offset = 0;
	 return(SUCC);
	}   
   }
   return(FAIL);
}
#endif 

/*===============================================================================
函数
清除长文件名目录项
入口：buf
出口：SUCC,FAIL
===============================================================================*/ 
static u8 Remove_LONGFILENAME(u8 *buf,u8 *folder_filename)
{
  u32 Cluster,Sector;
  u16 offset;//,length;
  u8 NextDirectoryEntryEMPTY; 
  u8 First_Entry_Deleted;
  Cluster = CORE.ClusterNum;
  Sector = CORE.SectorNum;
  offset = CORE.offset;
  First_Entry_Deleted = 0;
  read_flash_sector(buf,Sector);
  if(!buf[CORE.offset])//目录项之后的是空的吗？
   NextDirectoryEntryEMPTY = 1;
  else
   NextDirectoryEntryEMPTY = 0;  

  do{
	  //位置转向前一个目录项
    if(offset == 0)
    { //遇到扇区边界－－执行边界处理
      write_flash_sector(buf,Sector);
      Sector--; 
      offset = 512 - 32;
      if(CORE.DirectoryType == RootDirectory)
	  { 
		  if(Sector < CORE.FirstRootDirSecNum)
	      return(SUCC); 
	  }        
      else if (Sector  < FirstSectorofCluster(Cluster))
	  { //遇到簇边界，－－执行边界处理
         Cluster = Get_Previous_Cluster_From_Current_Cluster(Cluster);
         Sector = FirstSectorofCluster(Cluster) ;
		 Sector += BPB.sector_per_cluster - 1;

       } 
      read_flash_sector(buf,Sector);     
    }
	else
	{
	  offset -= 32;
	}

    if(buf[offset + 11] == ATTR_LONG_NAME)
    {
	  if(NextDirectoryEntryEMPTY)//将目录项置为删除
        buf[offset] = 0x0;
	  else
        buf[offset] = 0xe5;
    }
    else
	{
		if(First_Entry_Deleted == 0)
		{
	      if(NextDirectoryEntryEMPTY)
           buf[offset] = 0x0;//将目录项置为删除
	      else
           buf[offset] = 0xe5;
		 First_Entry_Deleted = 1;
		}
	    else
		{
		 write_flash_sector(buf,Sector);
		 return(SUCC);
		}
	}
  }while(1);
  return(SUCC);
}
/*===============================================================================
函数
文件删除
入口：filename:文件路径
出口：SUCC,FAIL
===============================================================================*/ 
#if complie_delete_file
u8 delete_file( u8 *filename)
{ 
 u8 buf[512];
 CORE.FullPathType = FilePath; 
 if(FullPathToSectorCluster(filename) == SUCC)//检查文件是否存在？
    {//检查文件是否被打开？
	 if(Check_FileOpened_Status(CORE.ClusterOfDirectoryEntry,0xff) == FileAlreadyopenedByOtherHandle)
         return(FAIL);//文件打开中，删除失败
	 //调用子函数，删除文件目录项
     if(Remove_LONGFILENAME(buf,filename) != SUCC)
         return(FAIL);
	 if(CORE.ClusterOfDirectoryEntry)//删除文件占用的簇链
       FreeClusterChain(CORE.ClusterOfDirectoryEntry); 
     return(SUCC); 
   }
 else
     return(FAIL);
}
#endif
/*===============================================================================
函数
目录删除
入口：char * foldername--路径+目录名
出口：SUCC,FAIL
===============================================================================*/    
#if complie_delete_folder
u8 delete_folder(u8 * foldername)
{
 u8 buf[512];
 u32 sector;
 CORE.FullPathType = DirectoryPath; 
 if(FullPathToSectorCluster(foldername) == SUCC)//检查文件是否存在？
   {
     sector = FirstSectorofCluster(CORE.ClusterOfDirectoryEntry);
	 read_flash_sector(buf,sector);
     if(buf[64] != 0)  //不允许删除非空目录
	 {
	  return(FAIL); 
	 }
     //调用子函数，删除目录项
	 Remove_LONGFILENAME(buf,foldername);
     //删除目录占用的簇链
     FreeClusterChain(CORE.ClusterOfDirectoryEntry);
     return(SUCC);
   }
  else
    return(FAIL);

     
} 
#endif
/*===============================================================================
函数    
重命名文件
入口：oldfilename:指向旧文件，newfilename:指向新文件(新文件名)
出口：SUCC,FAIL
===============================================================================*/  
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

 CORE.FullPathType = FilePath; 
 if(FullPathToSectorCluster(oldfilename) == SUCC)//检查旧文件是否存在?
 {  //检查旧文件是否被打开?
	if(Check_FileOpened_Status(CORE.ClusterOfDirectoryEntry,0xff) == FileAlreadyopenedByOtherHandle)
           return(FAIL);  //旧文件被打开中,不能删除
	//首簇号,FileSize保存
	FileSize = CORE.FileSize;
	first_cluster = CORE.ClusterOfDirectoryEntry;
    len = LengthofString(newfilename);
    temp = len - 1;
	//从新文件路径中分离出文件名
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
	 //从旧文件路径中分离出路径
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

	
	if(temp2 == 0)
	{
	  len = LengthofString(CORE.current_folder);
	  stringcpy(CORE.current_folder,oldfilename);
	  if(len > 3)
	  {
	     oldfilename[len] = '\\';  
		 oldfilename[len+1] = 0;
	  }
	  /*  检查新文件可能有的重复情况,发现重复的情况则直接退出函数 */
      concanenateString(oldfilename,newfilename + temp);
      if(FullPathToSectorCluster(oldfilename) == SUCC)
	      return(FAIL);
	}
	else
	{
	  oldfilename[temp2 + 1] = 0;
      concanenateString(oldfilename,newfilename + temp);
      /*  检查新文件可能有的重复情况,发现重复的情况则直接退出函数 */
	  if(FullPathToSectorCluster(oldfilename) == SUCC)
	      return(FAIL);

	}

    stringcpy(oldfilename1,oldfilename);
	CORE.FullPathType = FilePath; 
    FullPathToSectorCluster(oldfilename);
    Remove_LONGFILENAME(buf,oldfilename);//删除目录项，簇链不变化

    if(temp2 == 0)
	{ //旧文件名为相对路径寻址
	  CORE.FullPathType = DirectoryPath; 
      FullPathToSectorCluster(CORE.current_folder); 
	}
	else
	{
		if(temp2 < 3 && oldfilename[1] == ':')
			oldfilename[temp2 + 1] = 0;
		else
            oldfilename[temp2] = 0;
		//旧文件句为绝对路径寻址
		CORE.FullPathType = DirectoryPath;
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

/********************************************************************************************
函数
进入目录(由disk enumeration使用)
入口：无
出口：SUCC,FAIL
********************************************************************************************/
static u8 cd__folder_for_disk_enum()
{

     CORE.FullPathType = DirectoryPath;
	 //检查目录是否存在?
     if(FullPathToSectorCluster(CORE.current_folder_for_disk_enum) != SUCC)
	      return(FAIL);
	 //保存目录参数
	 if(LengthofString(CORE.current_folder_for_disk_enum) <= 3)
	 {//保存根目录参数
      CORE.SectorNum = CORE.FirstRootDirSecNum; 
	  CORE.CurrentDirectoryType = RootDirectory; 
	 }
	 else
	 { //保存非根录参数
	   CORE.CurrentDirectoryType = NoneRootDirectory;
       CORE.ClusterNum = CORE.ClusterOfDirectoryEntry;
	   CORE.ClusterNOofCurrentFolder = CORE.ClusterOfDirectoryEntry;
       CORE.SectorNum =  FirstSectorofCluster(CORE.ClusterNum); 
	 }
	 CORE.offset = 0;
	 return(SUCC);              
}

/********************************************************************************************
函数
退出目录(由disk enumeration使用)
入口：无
出口：SUCC,FAIL
********************************************************************************************/ 
static u8 cd__() 
{ 
  u16 offset;
  u8 entry[256],extension[20],folderSplit[256],i;
  i = 254;
  folderSplit[i] = 0;
  i --;

     offset = LengthofString(CORE.current_folder_for_disk_enum);
     offset --;
	 //从current_folder_for_disk_enum中分离出上一级目录
	 //并保存分离丢弃的目录进入folderSplit[],以用于CD__后面的位置定位
     do{
      if(CORE.current_folder_for_disk_enum[offset] != '\\')
	  {
		  folderSplit[i] = CORE.current_folder_for_disk_enum[offset];
		  CORE.current_folder_for_disk_enum[offset] = 0;
	  }
      else
      {
	   if(CORE.current_folder_for_disk_enum[offset-1] == ':')
		 break;
       CORE.current_folder_for_disk_enum[offset] = 0;
       break;
      }
       offset--;
	   i--;
     }while(1);     
     //进入上一级目录
	 cd__folder_for_disk_enum();
	 
	 do{ //CD__之后的位置定位到被丢弃目录之后
         //读取一个ENTRY 
		 if(GetEntryFromDirectory(entry,extension,Get_All_Entries) == FAIL)
		  {
		     if(CORE.CurrentDirectoryType == RootDirectory)
				 return(0x55);
		  }

	      //读取了非目录,继续读下一个ENTRY
          if(!(CORE.Entry_Attr & ATTR_DIRECTORY))
			  continue;
          //读取的目录是否与被丢弃目录相同,相同退出
		  if(stringcmp(entry,folderSplit+i+1) == SUCC)
			  break;
	 }while(1);

     if(LengthofString(CORE.current_folder_for_disk_enum) <= 3)
       CORE.CurrentDirectoryType = RootDirectory;
	 else
       CORE.CurrentDirectoryType = NoneRootDirectory;
     return(SUCC);
}
/********************************************************************************************
函数
保存枚举的当前位置
入口：无
出口：无
********************************************************************************************/
save_enum_vars(u8 disk_or_folder)
{
   //保存ClusterNum,SectorNum,offset
   CORE.DIR_ENUM_ClusterNum[disk_or_folder] = CORE.ClusterNum;  
   CORE.DIR_ENUM_SectorNum[disk_or_folder] =CORE.SectorNum ;
   CORE.DIR_ENUM_offset[disk_or_folder] = CORE.offset;
   //保存ClusterNum,SectorNum,offset
   CORE.DIR_ENUM_ClusterOfDirectoryEntry[disk_or_folder] = CORE.ClusterOfDirectoryEntry;
   CORE.DIR_ENUM_DirectoryType[disk_or_folder] = CORE.DirectoryType; 
   CORE.DIR_ENUM_FullPathType[disk_or_folder] = CORE.FullPathType;
   CORE.DIR_ENUM_CurPathType[disk_or_folder] = CORE.CurPathType; 

}

/********************************************************************************************
函数
恢复枚举的当前位置
入口：无
出口：无
********************************************************************************************/
restore_enum_vars(u8 disk_or_folder)
{
   //恢复ClusterNum,SectorNum,offset
   CORE.ClusterNum = CORE.DIR_ENUM_ClusterNum[disk_or_folder];   
   CORE.SectorNum = CORE.DIR_ENUM_SectorNum[disk_or_folder];
   CORE.offset = CORE.DIR_ENUM_offset[disk_or_folder];
   //恢复ClusterNum,SectorNum,offset
   CORE.ClusterOfDirectoryEntry = CORE.DIR_ENUM_ClusterOfDirectoryEntry[disk_or_folder];  
   CORE.DirectoryType = CORE.DIR_ENUM_DirectoryType[disk_or_folder]; 
   CORE.FullPathType = CORE.DIR_ENUM_FullPathType[disk_or_folder];
   CORE.CurPathType = CORE.DIR_ENUM_CurPathType[disk_or_folder]; 

}
/********************************************************************************************
函数
列举当前目录下的所有文件和目录(子目录不列举)
入口：无
出口：无
********************************************************************************************/  
#if complie_folder_dir
u8 folder_enumeration(u8 *return_string,u8 mode,u8 *ATTR)
{ 
  u8  Extension[20];
  u16 temp;
  if(mode == 0x0)
  {//从新开始枚举
   CORE.FullPathType = DirectoryPath; 
   //进入当前目录
   if(cd_folder(CORE.current_folder,0) != SUCC)
	   return(FAIL);
   CORE.offset = 0;
   //保存当前目录的参数
   if(CORE.CurrentDirectoryType ==  RootDirectory)
      CORE.SectorNum = CORE.FirstRootDirSecNum; 
   else
     {
       CORE.ClusterNum = CORE.ClusterNOofCurrentFolder;
       CORE.SectorNum = FirstSectorofCluster(CORE.ClusterNum); 
     }
     save_enum_vars(FOLDER_ENUM);//保存枚举的当前位置
  } 

  restore_enum_vars(FOLDER_ENUM);//恢复枚举的当前位置
do{
  stringcpy(CORE.current_folder,return_string);
  temp = LengthofString(return_string);
  if(return_string[temp-1] != '\\')
  {
   return_string[temp] = '\\';
   return_string[temp+1] = 0;
  }
  //从目录中读取一个ENTRY
  if(GetEntryFromDirectory(return_string+LengthofString
	  (return_string), Extension,Get_All_Entries) == SUCC)
  {
   temp = LengthofString(return_string);
   *ATTR = CORE.Entry_Attr;
   //与FILE EXTENSION完成连接
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
   save_enum_vars(FOLDER_ENUM);//保存枚举的当前位置
   return(SUCC);
  }
  else
   break;
 }while(1);
 return(FAIL);
} 
#endif
/*******************************************************************************************
函数
列举disk中所有文件和目录   
入口：无
出口：无
********************************************************************************************/ 
u8 disk_enumeration(u8 *return_string,u8 mode,u8* ATTR)
{
 u16 temp; 
 u8 Extension[20];
 if( ! mode)
 {//从新开始枚举
   stringcpy("C:\\",CORE.current_folder_for_disk_enum);
   cd__folder_for_disk_enum();
   save_enum_vars(DISK_ENUM); //保存枚举的当前位置 
 }  
   restore_enum_vars(DISK_ENUM);//恢复枚举的当前位置
 do{
    stringcpy(CORE.current_folder_for_disk_enum,return_string);
    temp = LengthofString(return_string);

    if(return_string[temp-1] != '\\')
	  {
         return_string[temp] = '\\';
         return_string[temp+1] = 0;
	  }
	//从目录中读取一个ENTRY
     if(GetEntryFromDirectory(return_string+LengthofString(return_string), 
	      Extension,Get_All_Entries) == FAIL)
	 {   
	     //返回上一级目录
		 if(cd__() == 0x55)
		 {
		     return(FAIL);
		 }
         continue;

	  }	
	 else{
	       temp = LengthofString(return_string);
           *ATTR = CORE.Entry_Attr; 
        #if filter_hidden_entry
		   if(*ATTR & ATTR_HIDDEN)
			   continue;
        #endif
           //与FILE EXTENSION完成连接
           if(temp > 0 && (!((*ATTR) & ATTR_DIRECTORY)))
		   {
              if(Extension[0] != 0)
			  {
                  return_string[temp] = '.';
                  return_string[temp+1] = 0;
	              concanenateString(return_string,Extension);
			  }

		   }
		   else{  temp = LengthofString(return_string);
		          //进滤"." ".."
		          if(return_string[temp - 1] == '.' || (return_string[temp - 1] == '.'
					  && return_string[temp - 2] == '.'))
					  continue;
				  stringcpy(return_string,CORE.current_folder_for_disk_enum);
                  //进入目录去枚举
				  cd__folder_for_disk_enum();
		   }

	  }
   save_enum_vars(DISK_ENUM);//保存枚举的当前位置 
   return(SUCC);
 }while(1);
  
}
/********************************************************************************************
函数
文件查找函数
入口：1）mode = 0：在当前目录下查找；2）mode=1：在整个磁盘中查找
      filename-被查找文件,Return_string
出口：无
********************************************************************************************/
#if complie_find_file
u8 find_file(u8 * filename,u8 mode,u8* Return_string)
{   
	u16 temp,POINTER;
	u8 ATTR,disk_enum_mode;

    if(! mode)
	{  //在当前目录下查找文件
      stringcpy(CORE.current_folder,Return_string);
      temp = LengthofString(Return_string);
	  if(*(Return_string + temp - 1) != '\\')
	  {
	    Return_string[temp] = '\\';
		Return_string[temp + 1] = 0;
	  }
      concanenateString(Return_string,filename);
     CORE.FullPathType = FilePath;
	 //直接用FullPathToSectorCluster()检查文件是否存在?
     if(FullPathToSectorCluster(Return_string) != SUCC)
	      return(FAIL);//文件不存在,查找失败
	 return(SUCC);//文件存在,查找成功
	
	}
	else
	{//在磁盘中查找
     disk_enum_mode = 0;
	 //用disk_enumeration()读取磁盘所有的ENTRY
     while(disk_enumeration(Return_string,disk_enum_mode,&ATTR) == SUCC)
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
        //检查文件名是相同?
		if(stringcmp(Return_string + POINTER + 1,filename) == SUCC)
			return(SUCC);
	 } 
     return(FAIL);	
	}
}
#endif
/********************************************************************************************
函数
查询分区容量和剩余空间 --通过检查分区FAT表来实现
入口：partition_id(Supported ID:form 'C' to 'F'),u32 *volume_capacity, u32 *volume_free_space
出口：SUCC  (返回的分区容量和剩余空间以512 bytes扇区为单位)
**********************************************************************************************/  
#if complie_volume_inquiry 
u8 volume_inquiry(u8 partition_id,u32 *volume_capacity, u32 *volume_free_space)
{   
  u16 i,j,x;
  u8 buf[512];
  if(partition_id >= 'c' && partition_id <= 'f')
       partition_id -= 32;   
  if ( ! (partition_id >= 'C' && partition_id <= 'F'))
    return(FAIL); 
  //读取分区的BPB信息
  Read_partition_PBP((u8)(partition_id - 'C'));
   if (CORE.system_id == 0x6)
   {   
     *volume_free_space = 0;
	 i = 0;
	 x = 0;
	 while(i < (CORE.CountofClusters) + 2){
	 //从扇区CORE.FirstSectorofFAT1开始检查
      if(read_flash_sector(buf,CORE.FirstSectorofFAT1 + x) == SUCC) 
       {//检查一整个扇区
        for (j = 0;j < 512;j +=2)
         { 
          if(buf[j] == 0 && buf[j + 1] == 0)
             (*volume_free_space) ++;
		  i++;
		  if(i >= (CORE.CountofClusters + 2))
			  break;
         }
		 x++;//扇区号+1
        }
       else
        {
          *volume_capacity = 0;
          *volume_free_space = 0;
          return(FAIL);
         } 
	 }
       *volume_free_space *=  BPB.sector_per_cluster;//函数完成,返回结果
       *volume_capacity =  CORE.total_sector;  
       return(SUCC);
   }
   else
   {
      *volume_capacity = 0;
      *volume_free_space = 0;
      return(FAIL);   
   }
}
#endif
