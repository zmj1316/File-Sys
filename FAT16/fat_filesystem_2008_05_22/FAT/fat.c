/*+FHDR------------------------------------------------------------------
��Ȩ����:

���ı�-רעUSB��FAT�ļ�ϵͳ�Ĺ̼��о�
��ϵ��ʽ:qq 292942278  ����:tony_yang123@sina.com.cn

����FAT16����Ѵ���,����Բ���,������о���
������FAT32����,�շѵİ汾,�����������ϵ������

Copyright (c),
Tony Yang �CSpecialized in the USB and FAT's firmware research and design
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
����
ת���ַ����ɴ�д
��ڣ�*string:�ַ����ĵ�ַ
���ڣ�SUCC
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
����
�ַ����ĳ��ȵĻ��
��ڣ�*string:�ַ����ĵ�ַ
���ڣ��ַ������ܳ���
===============================================================================*/ 
static u16 LengthofString(u8 * string)
{ 
 u16 i;
 i = 0;
 while(*string)//�ַ����ļ���,ֱ������=0
  {
    i++;
    string++;
  }
 return(i);
} 
/*===============================================================================
����
���������ַ���,�ַ���2���Ӻ󲻱�
��ڣ�*string:�ַ����ĵ�ַ
���ڣ�SUCC,FAIL
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
����
�����ַ���,�ַ��������ַ������Ŀ���
��ڣ�*string1:�ַ����ĵ�ַ��*string2:�ַ����ĵ�ַ
���ڣ�SUCC
===============================================================================*/ 
static u8 stringcpy(u8 *string1,u8 *string2)
{
 while(*string1) 
   { //�����ַ�������
     *string2 = *string1;
     string1++;
     string2++;
   }  
 *string2 = 0;
 return(SUCC);
}
/*===============================================================================
����
�Ƚ������ַ���(�ַ����Ĵ�Сд��������)
��ڣ�*string1:�ַ���1�ĵ�ַ��*string2:�ַ���2�ĵ�ַ
���ڣ�SUCC,FAIL
===============================================================================*/ 
static u8 stringcmp(u8 *string1,u8 *string2)
{
 UPCASE(string1);
 UPCASE(string2);  
 while((*string1) && (*string2)) 
   {
     if((*string1) == (*string2))
	 {   //�����ַ����Ƚ�
         string1++;
         string2++;
      }
     else
       return(FAIL);
   }   
 //�ж����ַ���β���ǣ���  
 if( ((*string1) == 0) && ((*string2) == 0))
 {
     return(SUCC);
 }
 else
     return(FAIL);
}

/*===============================================================================
����
�ڴ������ҵ���ǰ�ص���һ��
��ڣ�Cluster:��ǰ�غ�
����: ������һ�غ�
===============================================================================*/ 
static u32 Get_Next_Cluster_From_Current_Cluster(u32 Cluster)
{
   u8 buf[512]; 
   u32 ThisFATSecNum,ThisFATEntOffset;
   //FAT16ÿ����ռ�������ֽ�
   ThisFATEntOffset = Cluster * 2;
   //���㵱ǰ����������
   ThisFATSecNum = CORE.relative_sector + BPB.reserved_sector 
                   + (ThisFATEntOffset / BPB.bytes_per_sector);
   //���㵱ǰ������������ƫ����
   ThisFATEntOffset = ThisFATEntOffset % BPB.bytes_per_sector; 
   //��ȡ��ǰ����������
   read_flash_sector(buf,ThisFATSecNum);
   //���ص�ǰ�ص���һ��
   return((u32)buf[ThisFATEntOffset] + ((u32)buf[ThisFATEntOffset + 1]) * 256);
}
/*===============================================================================
����
�ڴ������ҵ���ǰ����һ�Ĵغ�
��ڣ�Cluster:��ǰ�غ�
����: ���ص�ǰ��֮ǰ�Ĵغ�
===============================================================================*/ 
static u32 Get_Previous_Cluster_From_Current_Cluster(u32 Cluster)
{
  u8 buf[512];
  u32 CUR_FATSecNum,CUR_FATEntOffset;
  u32 CUR_Checking_Cluster_No;
  //�Ӵغţ���ʼ��
  CUR_Checking_Cluster_No = 0;
  CUR_FATEntOffset = 0;
  CUR_FATSecNum = CORE.FirstSectorofFAT1; 
  read_flash_sector(buf,CUR_FATSecNum); 
  do{ 
	 //�������غţ�����ʧ��
	 if( CUR_Checking_Cluster_No == (CORE.CountofClusters + 2))
         {
          return(FAIL);
         } 
	 //�����һ���Ƿ��ǵ�ǰ�غ�?
     if( (u16)(buf[CUR_FATEntOffset] + buf[CUR_FATEntOffset + 1] * 256) == (u16)Cluster)
          return(CUR_Checking_Cluster_No);  //���شغ�
     CUR_Checking_Cluster_No++;
     CUR_FATEntOffset += 2;

     if(CUR_FATEntOffset >= 512)//�����һ����������������һ�������
      {
        CUR_FATSecNum++;

        read_flash_sector(buf,CUR_FATSecNum); 
        CUR_FATEntOffset = 0;     
      }
  }while(1);
}
/*===============================================================================
����
ɾ��һ������
��ڣ�Cluster:�״غ�
����: SUCC,FAIL
===============================================================================*/ 
static u8 FreeClusterChain(u32 Cluster)
{
 u8 buf[512],i; 
 u32 ThisFAT1SecNum,ThisFATEntOffset,ThisFAT2SecNum;    
 do{
   
   //FAT16ÿ����ռ�������ֽ�
   ThisFATEntOffset = Cluster * 2;
   //���㵱ǰ����������
   ThisFAT1SecNum = CORE.relative_sector + BPB.reserved_sector 
                   + (ThisFATEntOffset / BPB.bytes_per_sector);
   ThisFAT2SecNum = ThisFAT1SecNum + BPB.sectors_per_FAT;

   //���㵱ǰ������������ƫ����
   ThisFATEntOffset = (ThisFATEntOffset) % BPB.bytes_per_sector; 
   read_flash_sector(buf,ThisFAT1SecNum);
   //Cluster��Ϊ����һ��
   Cluster = (u32)buf[ThisFATEntOffset]+ (u32)buf[ThisFATEntOffset + 1] * 256;  
   //����ֵ�声��
   buf[ThisFATEntOffset] = 0x0;  
   buf[ThisFATEntOffset + 1] = 0x0;
   //��д�ƣ��ԣ�
   write_flash_sector(buf,ThisFAT1SecNum); 
   //��д�ƣ��ԣ������Уƣ��Ա�
   for(i = 1;i < BPB.numbers_of_FAT;i++) 
     {
      write_flash_sector(buf,ThisFAT2SecNum);
      ThisFAT2SecNum = ThisFAT2SecNum + BPB.sectors_per_FAT;
     } 
   //�жϴ����Ƿ������
   if(Cluster >= 0xfff6)
     return(SUCC);   
  }while(1);  
}
/*===============================================================================
����
���شأε�������
��ڣ�N���غ�
����: ���شأε�������
===============================================================================*/  
static u32 FirstSectorofCluster(u32 N)
{
 return((N - 2) * BPB.sector_per_cluster + CORE.FirstDataSector);
}

/*===============================================================================
����
��FAT�з���һ���ص�������ǰ��֮��
��ڣ�Cluster:��ǰ�غ�,added_cluster������Ĵ�
����: ����Ĵغ�
===============================================================================*/ 
u8 Allocate_EMPTY_CLUSTER_TO_CUR_CLUSTER_CHAIN(u32 Cluster,u32 *Added_Cluster,u8 *buf)
{
  u32 temp,EMPTY_CLUSTER,ThisFAT1SecNum,ThisFATEntOffset,ThisFAT2SecNum;
  u16 i; 
  EMPTY_CLUSTER = 0;
  //����ʼ�ƣ��Ա����ʼ�������ҿմ�����ɷ���
  ThisFAT1SecNum = CORE.relative_sector + BPB.reserved_sector;
  ThisFAT2SecNum = ThisFAT1SecNum + BPB.sectors_per_FAT;
  do{    
  //��ƫ��������ʼ��	  
  ThisFATEntOffset = 0; 
  read_flash_sector(buf,ThisFAT1SecNum);   
  for(i = 0;i < 256;i++)
  {  
     if(buf[ThisFATEntOffset] == 0 && buf[ThisFATEntOffset + 1] == 0)
      {//�ҵ��մ�,����ǰ�ص���һ�طŵ��ҵ��մص���һ��
       temp = Get_Next_Cluster_From_Current_Cluster(Cluster);
       buf[ThisFATEntOffset] = (u8)(temp & 0xff);  
       buf[ThisFATEntOffset + 1] = (u8)((temp >> 8) & 0xff);
       //��д�ƣ��ԣ�
	   write_flash_sector(buf,ThisFAT1SecNum);  
       temp = ThisFAT2SecNum;
       //��д�����ƣ��ԣ������еģƣ��Ա�
	   for(i = 1;i < BPB.numbers_of_FAT;i++)
         {
          write_flash_sector(buf,temp);
          temp +=  BPB.sectors_per_FAT;
         }  
       temp = Get_Next_Cluster_From_Current_Cluster(Cluster);
       //����ǰ�ص���һ����дָ���ҵ��Ŀմ�
	   ThisFATEntOffset = Cluster * 2;
       //���㵱ǰ����������
	   ThisFAT1SecNum = CORE.relative_sector + BPB.reserved_sector 
                   + (ThisFATEntOffset / BPB.bytes_per_sector);
       
	    //���㵱ǰ������������ƫ����
	   ThisFATEntOffset = ThisFATEntOffset % BPB.bytes_per_sector;
       ThisFAT2SecNum = ThisFAT1SecNum + BPB.sectors_per_FAT; 
       read_flash_sector(buf,ThisFAT1SecNum);
       buf[ThisFATEntOffset] = (u8)(EMPTY_CLUSTER & 0xff);//Add to cluster chain
       buf[ThisFATEntOffset + 1] = (u8)((EMPTY_CLUSTER >> 8) & 0xff); 
       //��д�ƣ��ԣ�
	   write_flash_sector(buf,ThisFAT1SecNum);  
       //��д�����ƣ��ԣ������еģƣ��Ա�
	   temp = ThisFAT2SecNum;
       for(i = 1;i < BPB.numbers_of_FAT;i++)  //update backup FAT2,etc..
         {          
		   read_flash_sector(buf,temp);
           buf[ThisFATEntOffset] = (u8)(EMPTY_CLUSTER & 0xff);//Add to cluster chain
           buf[ThisFATEntOffset + 1] = (u8)((EMPTY_CLUSTER >> 8) & 0xff); 
           write_flash_sector(buf,temp);
           temp +=  BPB.sectors_per_FAT;
         } 
	    //���մص��������
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
		//�������غţ�����ʧ��
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
����
��FAT�з���һ���մ�
��ڣ�cluster--������Ĵ�
����: SUCC,FAIL
===============================================================================*/ 
u8 Allocate_An_Empty_cluster(u32 * cluster,u8 * buf)
{
  u32 temp,EMPTY_CLUSTER,ThisFAT1SecNum,ThisFATEntOffset,ThisFAT2SecNum;
  u16 i; 

  //����ʼ�ƣ��Ա����ʼ�������ҿմ�����ɷ���
  EMPTY_CLUSTER = 0;
  ThisFAT1SecNum = CORE.relative_sector + BPB.reserved_sector;
  ThisFAT2SecNum = ThisFAT1SecNum + BPB.sectors_per_FAT;
  do{      
  ThisFATEntOffset = 0; 
  read_flash_sector(buf,ThisFAT1SecNum);   
  for(i = 0;i < 256;i++)
   {
     if(buf[ThisFATEntOffset] == 0 && buf[ThisFATEntOffset + 1] == 0)
      {//�ҵ��մ�,���ÿմ�Ϊ���أƣƣƣƣ���ռ��״̬
       buf[ThisFATEntOffset] = 0xff;   
       buf[ThisFATEntOffset + 1] = 0xff;
       //��д�ƣ��ԣ�
	   write_flash_sector(buf,ThisFAT1SecNum);  
       //��д�����ƣ��ԣ������еģƣ��Ա�
	   temp = ThisFAT2SecNum;
       for(i = 1;i < BPB.numbers_of_FAT;i++)  //update backup FAT2,etc..
         {
          write_flash_sector(buf,temp);
          temp +=  BPB.sectors_per_FAT;
         }  
	   //���մص��������
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
		//�������غţ�����ʧ��
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
����(CORE_INIT_2)
��root sector BPB����FirstDataSector,FirstRootDirSecNum�ȵ�
��ڣ���
����: ��
===============================================================================*/
static void BPB_INIT_2(void)
{ //�����Ŀ¼ռ�õ������� 
  CORE.RootDirSectors = (BPB.boot_entries * 32 + (BPB.bytes_per_sector - 1)) / BPB.bytes_per_sector;                             
  //��������������ʼ����                            
  CORE.FirstDataSector = CORE.relative_sector + BPB.reserved_sector + BPB.sectors_per_FAT
                               * BPB.numbers_of_FAT + CORE.RootDirSectors;
  //�����Ŀ¼����ʼ����
  CORE.FirstRootDirSecNum = CORE.relative_sector + BPB.reserved_sector+ BPB.sectors_per_FAT
                               * BPB.numbers_of_FAT;
  //����������ռ��������
  CORE.DataSec = CORE.total_sector - BPB.reserved_sector - CORE.RootDirSectors
                 - BPB.sectors_per_FAT * BPB.numbers_of_FAT;
  //������������Ӧ�Ĵ���
  CORE.CountofClusters = CORE.DataSec / BPB.sector_per_cluster;
  //����FAT1,FAT2����ʼ����
  CORE.FirstSectorofFAT1 = CORE.relative_sector + BPB.reserved_sector;
  CORE.FirstSectorofFAT2 = CORE.relative_sector + BPB.reserved_sector + BPB.sectors_per_FAT;
}
/*===============================================================================
����
Read Partition PBP
��ڣ�Partition ID
���ڣ���
===============================================================================*/ 
static u8 Read_partition_PBP(u8 partition_ID)
{  

  u8 buf[512];  
   if ((CORE.PartitionID - 'C') ==  partition_ID) 
    { 
	  //ָ����BPB�Ѿ��ڻ�����,ֱ�ӷ���
      return(SUCC);
    } 
   //��ȡMBR,������0
   read_flash_sector(buf,0); //read MBR 
   if ( buf[0x1be] == 0x00 || buf[0x1be] == 0x80) // check boot indicator 00 or 0x80
   {
   
   }
   else
   {
	   CORE.relative_sector = 0; 
       CORE.total_sector =  *((u32*) (buf+32));//�����������
       CORE.system_id = buf[0x1c2]; //����������� 0C-FAT32,06-FAT16 ect..
       CORE.PartitionID= 'C' + partition_ID; //��C��ʼ��Z���� 
	   //����BPB����
	   if ( buf[510] == 0x55 && buf[511] == 0xaa)
       {
        //ÿ�����ֽ���
        BPB.bytes_per_sector = buf[0xb] + buf[0xc] * 256;
        
        //ÿ��������
        BPB.sector_per_cluster = buf[0xd];
        //����������
        BPB.reserved_sector = buf[14] + buf[15] * 256;

        //FAT������
        BPB.numbers_of_FAT = buf[16];
        //��Ŀ¼��������FAT12/16ʹ��
        BPB.boot_entries = buf[17] + buf[18] * 256;
        //This field is the old 16-bit total count of sectors on the volume.
        BPB.TotSec16 = buf[19] + buf[20] * 256;
        BPB.TotSec16 = 0xffff;
        //ý��������
        BPB.media_descriptor = buf[21];
        //ÿ��FAT��ռ�õ�����������FAT12/16ʹ��
        BPB.sectors_per_FAT =  buf[22] + buf[23] * 256;
        CORE.system_id = 0x6; //����������� 0C-FAT32,06-FAT16 ect..
        BPB_INIT_2();
        return(SUCC);
       } 
	 }
    
  return(FAIL);
}  

/*===============================================================================
����
��·���ж�һ��entry
��ڣ�
���ڣ�SUCC,FAIL
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
     if( ( * Path) != 0 && ( * Path ) != '\\') //Path����õ�Entry name and file extension 
       { 
        (*RemovedCharsFromPath)++; 
        //����ENTRY NAME
		if( flag == FILE_NAME)
         {            
          if(*Path == '.')
             {//����".",��ʼ����file extension 
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
         {//����file extension
          if( j >= 3 )
            return(FAIL);
          FileExtension[j] =  *Path;
          j++;
          Path++;
         } 
      }
    else
      {//�������,����Ƿ�������һ��ENTRY?
       if(!( * Path))
        {
          if(CORE.FullPathType == FilePath)
		  {
		  CORE.CurPathType = FilePath;
		  }
          FileExtension[j] = 0;
          Return_Entry_Name[i] = 0;
		  //���һ��ENTRY,����LastSplitedNameofPath
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
����
Directory Entry��λ��+32,ת����һ��ENTRY 
��ڣ���
���ڣ�SUCC,FAIL
===============================================================================*/             
static u8 CORE_offset_add_32(u8 *buf)
{ //λ��+32
  CORE.offset += 32;
  if (CORE.offset >= 512)
  {
  //λ��Խ��һ������
  if (CORE.DirectoryType == RootDirectory)
  {     //��Ը�Ŀ¼�Ĵ���
        if (CORE.SectorNum < ( CORE.RootDirSectors +  CORE.FirstRootDirSecNum))
         {
           CORE.SectorNum++;
           CORE.offset = 0; 
           read_flash_sector(buf,CORE.SectorNum);
		   if(buf[CORE.offset] == 0 && buf[CORE.offset+1] == 0)  //End of the directory
		     return(FAIL);//��û��ENTRY,����FAIL
           return(SUCC);
         }
        else
           return(FAIL);
     }
    else  
     {
        if( (CORE.SectorNum - FirstSectorofCluster(CORE.ClusterNum) + 1) >= BPB.sector_per_cluster)
		{ //λ��Խ��һ����, ȡ��һ��,��������
           CORE.ClusterNum = Get_Next_Cluster_From_Current_Cluster(CORE.ClusterNum);
           if( CORE.ClusterNum >= 2 && CORE.ClusterNum <= 0xfff7)
            {
               CORE.SectorNum = FirstSectorofCluster(CORE.ClusterNum); 
               CORE.offset = 0;
               read_flash_sector(buf,CORE.SectorNum);
			   if(buf[CORE.offset] == 0 && buf[CORE.offset+1] == 0)  //End of the directory
                   return(FAIL);//��û��ENTRY,����FAIL
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
                return(FAIL);//��û��ENTRY,����FAIL
            return(SUCC);
         }
     }
  }
  if(buf[CORE.offset] == 0 && buf[CORE.offset+1] == 0)  //End of the directory
       return(FAIL);//��û��ENTRY,����FAIL
  return(SUCC);
}
/*===============================================================================
����
��Ŀ¼��ȡ���ļ���
��ڣ�EntryName-�����ļ���,Extension=������չ��
���ڣ�SUCC,FAIL
===============================================================================*/ 
static u8 GetEntryWith8_3Name(u8 *buf,u8* EntryName,u8 *Extension)
{
  u8 j;
  struct Directory_Entry_  *Directory_Entry_Local;  
  u8 *pointer;
  pointer = buf;
  Directory_Entry_Local = (struct Directory_Entry_ *) (pointer + CORE.offset);
  //��DIRECTORY �ж�ȡEntryName  
  for(j = 0;j < 8;j++)
   {
    if(Directory_Entry_Local->filename[j] == 0x20)
      break;
    EntryName[j] = Directory_Entry_Local->filename[j];
   }   
  EntryName[j] = 0; 
  //��DIRECTORY �ж�ȡExtension
  for(j = 0;j < 3;j++)
   {  
    if(Directory_Entry_Local->file_extention[j] == 0x20)
        break;
    Extension[j] = Directory_Entry_Local->file_extention[j]; 
   }
  Extension[j] = 0;

  if(CORE.FullPathType == FilePath && CORE.CurPathType == FilePath)
     CORE.FileSize = *(u32*)Directory_Entry_Local->file_length;//�����ļ�����
  Directory_Entry_Local->file_length[0] = 0;
  Directory_Entry_Local->file_length[0] = 0;
  //�����ļ��״غ�
  CORE.ClusterOfDirectoryEntry = (Directory_Entry_Local->first_cluster_number_low2bytes[0]) + 
	  (Directory_Entry_Local->first_cluster_number_low2bytes[1]) * 256;
  CORE.PreEntrySectorNum = CORE.SectorNum;
  CORE.PreEntryoffset = CORE.offset;
  CORE_offset_add_32(buf);//Directory Entry��λ��+32 
  return(SUCC);
}
/*===============================================================================
����
��Ŀ¼��ȡ���ļ���
��ڣ�longFileName-�����ļ���,Extension=������չ��
���ڣ�SUCC,FAIL
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
	 //��ȡlongNameDirectoryEntry�е�ǰ5���ֽ�
     for(j = 1;j < 10;j+=2)
      {        
       if (LongNameDirectoryEntry_Local->dir_lname1[j] == 0)
          break; //��0�����˳�  
       longFileName[k] = LongNameDirectoryEntry_Local->dir_lname1[j];
       k ++;                 
      } 
	 longFileName[k] = 0;
       if(j >= 10)
         { //��ȡlongNameDirectoryEntry�еĵڶ�6���ֽ�
           for(j = 0;j < 12;j += 2)
            {  
              if (LongNameDirectoryEntry_Local->dir_lname2[j] == 0)
                 break;

                 longFileName[k] = LongNameDirectoryEntry_Local->dir_lname2[j];
                 k++;           
            }
           if(j >= 12)   
                for(j = 0;j < 4;j += 2)//��ȡlongNameDirectoryEntry�еĵ�������2���ֽ�
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
       return(FAIL);//Directory Entry��λ��+32 
    FileNameOffset -= 13;
    k = FileNameOffset;
    LongNameDirectoryEntry_Local = (struct LongNameDirectoryEntry *) (buf + CORE.offset);
    if(LongNameDirectoryEntry_Local->dir_attr != ATTR_LONG_NAME)
	{      //������ATTR_LONG_NAME��ʾһ��LONG FILE NAME��ŭ��������,��������short entry
           if ( ! (LongNameDirectoryEntry_Local->dir_attr & ATTR_VOLUME_ID)) 
           {//�����״غ�
			CORE.ClusterOfDirectoryEntry = LongNameDirectoryEntry_Local->dir_first[0]+
				LongNameDirectoryEntry_Local->dir_first[1] * 256;
			//�����ļ�����
            CORE.FileSize = *((u32*)LongNameDirectoryEntry_Local->dir_lname3);
			//����longFileName������longFileName
			stringcpy(longFileName+FileNameOffset+13,longFileName);
			//�����ļ���չ��
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
            return(FAIL); //Directory Entry��λ��+32 
          LongNameDirectoryEntry_Local = (struct LongNameDirectoryEntry *) (buf + CORE.offset);
	      if(LongNameDirectoryEntry_Local->dir_lname1[0] == 0xe5)
             continue;//������ɾ��ENTRY
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
����
��Ŀ¼��ȡһ��Entry
��ڣ�mode = 0����������directory entry
���ڣ�SUCC,FAIL
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
  if(Directory_Entry_Local->filename[0] == 0x0)//��Ŀ¼��β,��ȡ������β,�˳�����FAIL
	  return(FAIL);
  if(Directory_Entry_Local->filename[0] == 0xe5)//������ɾ��ENTRY
  {
   CORE.PreEntrySectorNum = CORE.SectorNum;
   CORE.PreEntryoffset = CORE.offset;
   if(CORE_offset_add_32(buf) == FAIL) //Directory Entry��λ��+32 
     return(FAIL);
   continue;
  }  
  switch(Directory_Entry_Local->file_attribute) //���file_attribute, 
    {
       case ATTR_LONG_NAME:{ //��Ŀ¼��ȡ���ļ���
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
			   CORE_offset_add_32(buf);///Directory Entry��λ��+32 
               return(SUCC);
			 }
             else if ( ! (LongNameDirectoryEntry_Local->dir_attr & ATTR_VOLUME_ID)) 
			 {
              CORE.PreEntrySectorNum = CORE.SectorNum;
             CORE.PreEntryoffset = CORE.offset;
			 CORE_offset_add_32(buf);//Directory Entry��λ��+32 
             return(SUCC);
             }	   
		   }
		   else
		   {
			CORE.PreEntrySectorNum = CORE.SectorNum;
            CORE.PreEntryoffset = CORE.offset;
		    CORE.ClusterOfDirectoryEntry = *(u16*)LongNameDirectoryEntry_Local->dir_first;
            CORE_offset_add_32(buf);//Directory Entry��λ��+32 
            
			return(SUCC);
		   }
          }
          break;
        }
       case ATTR_DIRECTORY:{//��Ŀ¼��ȡĿ¼(���ļ���)
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
        {//��Ŀ¼��ȡ�ļ�(���ļ���)
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
    return(FAIL);//Directory Entry��λ��+32 
 }while(1); 
return(SUCC);
}  
/*===============================================================================
����
��Ŀ¼�в���һ��Entry
��ڣ�floder_name-�ļ���,file_extension-��չ��
���ڣ�SUCC,FAIL
===============================================================================*/
static u8 FindEntryStruct(u8 *floder_name,u8 *file_extension)
{  
   u8 EntryName[256],Extension[20]; 
   u8 Name_Compare_OK,Extension_Compare_OK;   
   do{	  //��Ŀ¼��ȡһ��Entry	   
          if(GetEntryFromDirectory(EntryName,Extension,Get_Selected_ENTRIES) != SUCC)
		  {
            return(FAIL);
		  }
       Name_Compare_OK = OK;
	   //����ļ���
       if(stringcmp(EntryName,floder_name) != SUCC)
             Name_Compare_OK = unOK;        
       if(Name_Compare_OK == OK)  //����ļ���չ��
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
����
��Ե�·��ת����Sector,SectorOffset,Cluster
��ڣ�u8 *filename
���ڣ�SUCC,FAIL
===============================================================================*/
static u8 RelativePathToSectorCluster(u8 *RelativePath)
{ 
  u8 floder_name[256],file_extension[20]; 
  u8 Splited_Count;
  u8 Splited_Status;
   //����SplitNameFromPath()��RelativePath�����һ��Ŀ¼���ļ�Ԫ��
  Splited_Status = SplitNameFromPath(RelativePath,floder_name,file_extension,&Splited_Count);
  if(Splited_Status == FAIL)
      return(FAIL);
  RelativePath += Splited_Count;  
   //��Ŀ¼�в���һ��Entry
  if(FindEntryStruct(floder_name,file_extension) != SUCC)
  {
   return(FAIL); //����ʧ���˳�
  }//DirectoryType��Ϊ��RootDirectory
   if(CORE.CurPathType == DirectoryPath)
     if(CORE.DirectoryType == RootDirectory)
	  {
	  CORE.DirectoryType = NoneRootDirectory; 
	  }
      
  if(Splited_Status == LastSplitedNameofPath)
  {
   return(SUCC); 
  }    
  do{ //����SplitNameFromPath()��RelativePath�����һ��Ŀ¼���ļ�Ԫ��
     Splited_Status = SplitNameFromPath(RelativePath,floder_name,file_extension,&Splited_Count);
     if(Splited_Status == FAIL)
	  return(FAIL);    
     else 
       { //���ñ���,����Ŀ¼ȥ����
         CORE.ClusterNum = CORE.ClusterOfDirectoryEntry;
         CORE.SectorNum = FirstSectorofCluster(CORE.ClusterNum);
         CORE.offset = 0;  
       }
     RelativePath += Splited_Count;
	 //DirectoryType��Ϊ��RootDirectory
     if(CORE.CurPathType == DirectoryPath)
      if(CORE.DirectoryType == RootDirectory)
	  {
	  CORE.DirectoryType = NoneRootDirectory; 
	  }
	  //��Ŀ¼�в���һ��Entry
     if(FindEntryStruct(floder_name,file_extension) != SUCC)
	 {
       return(FAIL); //����ʧ���˳�
	 }
     else if(Splited_Status == LastSplitedNameofPath)
	 {
      return(SUCC);
	 }  

    }while(1);
    return(SUCC);
}
/*===============================================================================
����
������·��ת����Sector,SectorOffset,Cluster,�����ڼ���ļ��Ƿ����
��ڣ�u8 *filename
���ڣ�SUCC,FAIL
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
            { //����·��,�Ӹ�Ŀ¼����ת��
              if(LengthofString(filename) > Maximum_File_Path_Name)
			      return(EpathLengthsOVERFLOW);
              if(Read_partition_PBP((u8)((*filename) - 'C')) != SUCC)
                 return(FAIL);
              filename += 3; 
			  //���ò���,��Ŀ¼Ѱַ
              CORE.SectorNum = CORE.FirstRootDirSecNum; 
              CORE.DirectoryType =  RootDirectory;
              CORE.offset = 0;   
             }
          }
         else 
		 {   //���·��,�ӵ�ǰĿ¼����ת��  
             if((LengthofString(filename) + LengthofString(CORE.current_folder)) > Maximum_File_Path_Name)
                   return(EpathLengthsOVERFLOW);
             //���ò���
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
       else if((* filename) == '\\')//�ӵ�ǰ�̷�����ת��
            {
             if((LengthofString(filename) + 1) > Maximum_File_Path_Name)
                   return(EpathLengthsOVERFLOW); 
              //���ò���,��Ŀ¼Ѱַ
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
����
����ļ��Ƿ��Ѵ򿪣�
��ڣ�FirstClusterOfFile-�ļ��״غ�
���ڣ�FileAlreadyopenedByOtherHandle,FileUnopenedByOtherHandle
===============================================================================*/ 
u8 Check_FileOpened_Status(u32 FirstClusterOfFile,u16 j)
{
  u8 i;
  for(i = 0;i < MaximumFCB;i++)
  {
    if(i == j)
		continue;
   if(FCB[i].file_openned_flag == UsedFlag)
   { //FCB���״غ����ļ��״غ���ͬ,���ļ��ѱ���
     if(FCB[i].FirstClusterOfFile == FirstClusterOfFile)
        return(FileAlreadyopenedByOtherHandle);
   }
  }
  //δ������ͬ�״غ�,�ļ�δ��,����
  return(FileUnopenedByOtherHandle);
} 
/*===============================================================================
����
��file buffer��д����
��ڣ�u8 FCBsn
���ڣ�SUCC,FAIL
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
  { //δ�����ļ��״غţ���ִ�з���
	if( Allocate_An_Empty_cluster(&FCB[FCBsn].FirstClusterOfFile,buf)== FAIL)
	  return(FAIL);
	//���״غ�д���ļ���ENTRY��
	read_flash_sector(buf,FCB[FCBsn].Entry_Storedin_Sector);
    i = FCB[FCBsn].Entry_Storedin_Sector_Offset + 26;
	buf[i] = (u8)(FCB[FCBsn].FirstClusterOfFile & 0xff);
	buf[i+1] = (u8)((FCB[FCBsn].FirstClusterOfFile >> 8) & 0xff);
	write_flash_sector(buf,FCB[FCBsn].Entry_Storedin_Sector);
    FCB[FCBsn].CurClusterInBUF = FCB[FCBsn].FirstClusterOfFile;
  }
  //���ݵ�ǰCurBlockInBUF������Ҫ�ƶ�CLUSTER����
  ClusterQTY = FCB[FCBsn].CurBlockInBUF / BPB.sector_per_cluster;
  NEXTCluster = FCB[FCBsn].FirstClusterOfFile; 
  //������Ҫ�ƶ�CLUSTER����
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
  //�ƶ�CLUSTER
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

  //���浱ǰCurClusterInBUF��ClusterSNInBUF
  FCB[FCBsn].CurClusterInBUF = NEXTCluster;
  FCB[FCBsn].ClusterSNInBUF = qty; 
  //��file buffer��д����
  FCBbufSize = FileBUFSize * TotalFileBUFsQTYeachFCB; 
  BUFoffset = 0;
  i = FCB[FCBsn].CurBlockInBUF % BPB.sector_per_cluster;
  //��ȡ������
  sector = FirstSectorofCluster(NEXTCluster);
  wrote_sectors_count = 0; 
  do{ 
     write_flash_sector(FCB[FCBsn].FileBUF + BUFoffset,sector + i); 
     wrote_sectors_count++;
	 i++;
	 //����FILE SIZEֱ���˳�
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
	 { //�����ر߽�, ת����һ�ؼ���д 
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
  //���Modified_Flag
  FCB[FCBsn].Modified_Flag = 0;
  //��ȡ�ļ�Ŀ¼��, �Ӷ������ļ���С
  read_flash_sector(buf,FCB[FCBsn].Entry_Storedin_Sector);
  i = FCB[FCBsn].Entry_Storedin_Sector_Offset + 28;
  FILESIZE = buf[i] + buf[i+1] * 256 + buf[i+2] * 256 * 256+ buf[i+3]  * 256 * 256 *256;
  //����ļ���С�Ƿ��Ѹ���?
  if(FILESIZE != FCB[FCBsn].FileSize) 
  { //����ļ���С�Ѹ���,�����File Directory entry���ļ���С
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
����
�����ļ���ǰ��λ�ã���ɸ����ļ�������
��ڣ�u8 FCBsn
���ڣ�SUCC,FAIL
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
  if(FCB[FCBsn].Modified_Flag)//���Modified_Flag,�����Ϊ0,����Ҫ��дFILE BUFFER
    if(Writeback_FCB_file_buffer(FCBsn) == FAIL)
	  return(FAIL);
  //���ݵ�ǰCurBlockInBUF������Ҫ�ƶ�CLUSTER����
  FCB[FCBsn].CurBlockInBUF = FCB[FCBsn].cur_position / FileBUFSize;
  ClusterQTY = FCB[FCBsn].CurBlockInBUF / BPB.sector_per_cluster;
  NEXTCluster = FCB[FCBsn].FirstClusterOfFile; 
  qty = ClusterQTY;
  
  //������Ҫ�ƶ�CLUSTER����  
  if(FCB[FCBsn].ClusterSNInBUF <= ClusterQTY)
  { 
    ClusterQTY -= FCB[FCBsn].ClusterSNInBUF;
    NEXTCluster = FCB[FCBsn].CurClusterInBUF;
  }
 else
 {
    NEXTCluster = FCB[FCBsn].FirstClusterOfFile; 
  } 

  //�ƶ�CLUSTER
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
    //��ȡ������
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
     {//�����ر߽�, ת����һ�ؼ����� 
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
����
����FCB
��ڣ���
���ڣ�EAllocate_FCB-- Fail,other--SUCC with FCB sequential number
===============================================================================*/ 
u8 Allocate_FCB(void)
{  
 u8 i;
 for (i = 0; i < MaximumFCB;i++)//��һ��UnusedFlag��FCB,���ظ�������ʹ��
  if (FCB[i].file_openned_flag == UnusedFlag)  
   {          
    FCB[i].file_openned_flag = UsedFlag;//����Ϊռ��
	FCB[i].Modified_Flag = 0;
    return(i);//����FCB�ı��
   }
 return(EAllocate_FCB);
}
/*===============================================================================
����
�ͷ�һ��FCB
��ڣ�FCB_sn
���ڣ�EFree_FCB,SUCC
===============================================================================*/     
u8 Free_FCB(u8 FCBsn)
{//���FCB�Ƿ�ռ��
 if(FCB[FCBsn].file_openned_flag == UsedFlag)
  {
    FCB[FCBsn].file_openned_flag = UnusedFlag;//����Ϊ��ռ��
	FCB[FCBsn].Modified_Flag = 0;
    return(SUCC);
  }
  else
   return(EFree_FCB);
}
/*===============================================================================
����
FAT��ʼ������,һ����main()��ʹ���ļ�ϵͳ֮ǰ����
��ڣ���
���ڣ���
===============================================================================*/ 
#if complie_FAT_filesystem_initialiation
u8 FAT_filesystem_initialiation(void)
{ 
  u8 root[] = "C:\\",i;
  CORE.PartitionID = 0xff;
  CORE.CurrentDirectoryType =  RootDirectory; 
  //��ʼ����ǰĿ¼Ϊ'C:\'
  stringcpy(root,CORE.current_folder);
  //����FCB����Ϊ��ռ��
  for (i = 0; i < MaximumFCB;i++)
  {
   FCB[i].file_openned_flag = UnusedFlag; 
   FCB[i].Modified_Flag = 0;
  }
  //��ȡȱʡ��PBP����ص���Ϣ
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
 /*0xF8 is the standard value for ��fixed��*/
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
����
�Զ���ʽ�� FAT �ļ�ϵͳ
��ڣ�disk_:�̷�   filesystem_type:��FAT16,FAT32Ϊ��ʽ��Ŀ��  disk_capacity:��������
���ڣ�SUCC,FAIL
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
���� 
���ļ�
��ڣ�u8 * filename:·��+�ļ���
���ڣ�����FCB_SN,FAIL
===============================================================================*/   
#if complie_open_file 
u8 open_file(u8 * filename)
{ 
 u8 FCBsn;
 CORE.FullPathType = FilePath; 
 //����ļ��Ƿ����?
 if(FullPathToSectorCluster(filename) == SUCC)
 {   //������ļ�,��ִ��FCB����
     FCBsn = Allocate_FCB();
     if(FCBsn == EAllocate_FCB)
       return(FAIL);
     //��ʼ��FCB�ĸ�������
	 FCB[FCBsn].cur_position = 0;
	 FCB[FCBsn].CurBlockInBUF = 0xffff;
	 FCB[FCBsn].FirstClusterOfFile = CORE.ClusterOfDirectoryEntry;

     FCB[FCBsn].CurClusterInBUF = FCB[FCBsn].FirstClusterOfFile;
     FCB[FCBsn].ClusterSNInBUF = 0;


     FCB[FCBsn].FileSize = CORE.FileSize;
     FCB[FCBsn].Entry_Storedin_Sector = CORE.PreEntrySectorNum ;  //Save sectorNUM of File Directory entry for current file
     FCB[FCBsn].Entry_Storedin_Sector_Offset = CORE.PreEntryoffset; //Save offset in sector of File Directory entry for current file
     FCB[FCBsn].Modified_Flag = 0;
	 //����ļ��Ƿ��Ѵ�?
	 if(Check_FileOpened_Status(CORE.ClusterOfDirectoryEntry,FCBsn) == FileAlreadyopenedByOtherHandle)
         FCB[FCBsn].Permission = ReadOnlyPermission;
	 else
         FCB[FCBsn].Permission = FullPermission;
   //����Update_FCB_file_buffer(),���ļ���ȡ���ļ�������
   #if EnableFileBuf
     Update_FCB_file_buffer(FCBsn);
   #endif 
     return(FCBsn); 
   }
  else
   return(FAIL);//�ļ�������,�ļ���ʧ��
}
#endif 
/*===============================================================================
���� 
�ر��ļ�
��ڣ�FCBsn
���ڣ�SUCC,FAIL
===============================================================================*/  
#if complie_close_file
u8 close_file(u8 FCBsn)
{  
  if(Free_FCB(FCBsn) == SUCC)//�ͷ�FCB
    return(SUCC);
  else
    return(FAIL);
} 
#endif 
 
/*===============================================================================
����
Directory Entry offset+32 �ɽ����ļ�/Ŀ¼ʹ�ã���ĩ�ؿ��Զ�����մؼ������
��ڣ�buf--
���ڣ�SUCC,FAIL
===============================================================================*/             
static u8 CORE_offset_add_32_With_EMPTY_CLUSTER_ALLOCATION(u8 *buf)
{
  u32 temp;
  //λ��+32
  CORE.offset += 32;
  if (CORE.offset >= 512)
  {//λ��Խ��һ������
   CORE.SectorNum++;
  if (CORE.DirectoryType == RootDirectory)
  {     //��Ը�Ŀ¼�Ĵ���
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
     {//λ��Խ��һ����, ȡ��һ��,��������
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
		   {   //��ĩ�أ�����մؼ������ 
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
���� 
���㳤�ļ�Ŀ¼���У���
��ڣ�*entry:longfilenameentry�׵�ַ
���ڣ�checksum
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
���� 
��Ŀ¼��Ѱ�Ҹ�������DIRECTORY ENTRY�Ŀռ�
��ڣ�Entry_Resuested_QTY-ENTRY������
���ڣ�SUCC,FAIL
===============================================================================*/
static u8 Seek_Space_to_Write_Directory_Entry(u16 Entry_Resuested_QTY,u8 *buf)
{
  u32 SectorNum_LOCAL,ClusterNum_LOCAL;
  u16 Offset_LOCAL;
  u8 Empty_Entry_Space_Count,found_flag;

  //��Ŀ¼�ͷǸ�Ŀ¼�Ĳ���װ��
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
    SectorNum_LOCAL = CORE.SectorNum; // ���浱ǰENTRY����λ��
    ClusterNum_LOCAL = CORE.ClusterNum;
    Offset_LOCAL = CORE.offset;
	found_flag = 0;
	Empty_Entry_Space_Count = 0;
    do{
       if(buf[CORE.offset] == 0xe5 || buf[CORE.offset] == 0)//��ENTRY
	   {
	    Empty_Entry_Space_Count++;
		//ֱ���ҵ�ָ�������Ŀ�ENTRY
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
    if(found_flag)//�ҵ�ָ�������Ŀ�ENTRY
	{ 
	 CORE.SectorNum = SectorNum_LOCAL; //�ÿ�ENTRY����λ��
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
���� 
��дһ��LFN directory entry
��ڣ�u8 * Directory_Entry_buf,u8 * Entry_Name,u8 LFN_record_FLAG
      u8 Entry_Name,u8 checksum
���ڣ�SUCC,FAIL
===============================================================================*/
static u8 Fill_LFN_Directory_Entry(u8 * Directory_Entry_buf,u8 * Entry_Name,u8 LFN_record_FLAG,u8 checksum)
{
  u8 i; 
  i = LengthofString(Entry_Name); 
  if(i < 12) 
  {
	  do{          //��д�հ׿ռ�
		i++;
		if(i >= 13)
			break;
        Entry_Name[i] = 0xff;
	  }while(1);
  }
  Directory_Entry_buf[0] = LFN_record_FLAG;;
  //��дǰ5���ֽ�
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
  Directory_Entry_buf[13] = checksum;//��дУ���
  //��д�м�6���ֽ�
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
  //��д2���ֽ�
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
���� 
��д���ļ�directory entry
��ڣ�u8 * Directory_Entry_buf,u8 * Entry_Name,u32 first_cluster,
      u32 FileSize,u8 attr
���ڣ�SUCC,FAIL
===============================================================================*/
static u8 Fill_SFN_Directory_Entry(u8 * Directory_Entry_buf,u8 * Entry_Name,u32 first_cluster,u32 FileSize,u8 attr)
{
 u8 i,flag,j;
 for(i = 0;i < 11;i++)
   Directory_Entry_buf[i] = 0x20; 
  i = 0; 
  j = 0;
  flag = FILE_NAME;
  do{ //Directory_Entry����ȡ��Shortfile Entry name and file extension           
     if( ( * Entry_Name) != 0) 
       { 
        if( flag == FILE_NAME)
         {            
          if(*Entry_Name == '.')//����'.',ת����дFile extension
             {
              flag  = FILE_EXTENSION;
			  Entry_Name++;
             }  
          else
            {
             Directory_Entry_buf[i] =  *Entry_Name;//��дEntry Name
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
           Directory_Entry_buf[8+j] =  *Entry_Name;//��дFile extension
          j++;
          Entry_Name++;
         } 
      }
	  else
		 break;
 }while(1);
 Directory_Entry_buf[11] = attr;
 Directory_Entry_buf[12] = 0;
 Directory_Entry_buf[26] = (u8)(first_cluster & 0xff);  //��д�״غ�
 Directory_Entry_buf[27] = (u8)((first_cluster >> 8) & 0xff);
 Directory_Entry_buf[28] = (u8)(FileSize & 0xff);////��д�ļ�����
 Directory_Entry_buf[29] = (u8)((FileSize >> 8) & 0xff);
 Directory_Entry_buf[30] = (u8)((FileSize >> 16) & 0xff);
 Directory_Entry_buf[31] = (u8)((FileSize >> 24) & 0xff);
 return(SUCC);
}
/*===============================================================================
���� 
��Longfilename Directory Entryת��Ϊshort filename
��ڣ�Directory_Entry:Directory_Entry name,attr:attr of Directory_Entry
���ڣ�SUCC,FAIL
===============================================================================*/
static u8 LFN_convert_to_SFN(u8 * Directory_Entry,u8 * SFN_Directory_Entry_buf)
{
 u8 i,flag,j;
 for(i = 0;i < 11;i++)
   SFN_Directory_Entry_buf[i] = 0x20; //��ʼ��DIRECTORY ENTRY
  i = 0; 
  j = 0;
  flag = FILE_NAME;
  do{           
     if( ( * Directory_Entry) != 0) 
       { 
        if( flag == FILE_NAME)
         {            
          if(*Directory_Entry == '.')//����'.',ת����дFile extension
             {
              flag  = FILE_EXTENSION;
			  Directory_Entry++;
             }  
          else
            {
             SFN_Directory_Entry_buf[i] =  *Directory_Entry;//��дEntry Name
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
            return(FAIL);//��дFile extension
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
���� 
��дLongfilename Directory Entry�����
��ڣ�Directory_Entry:Directory_Entry name,attr:attr of Directory_Entry
���ڣ�SUCC,FAIL
===============================================================================*/
static u8 Write_LongFileName_Entry(u8 * Directory_Entry,u8 attr,u32 first_cluster,u8 * buf,u32 FileSize)
{
  u16 len;
  u8 i,checksum;
  u8 Directory_Entry_buf[32];
  u8 SN,LFN_record_FLAG;
  //������Ҫ��Directory_Entry�ռ䣬ÿ��Directory_Entry�ռ�Ϊ32���ֽ�,����������len��
  len = LengthofString(Directory_Entry) / 13;
  if(LengthofString(Directory_Entry) % 13)
	len++;
  SN = len; 
  //��long-filename directory entry��������Ӧ��short-filename directory entry
  LFN_convert_to_SFN(Directory_Entry,Directory_Entry_buf);
  //����short-filename directory entryУ���
  checksum = calculate_checksum_longfilenameentry(Directory_Entry_buf);
  //����last long-filename directory entry for file
  read_flash_sector(buf,CORE.SectorNum);    
  LFN_record_FLAG = SN | (u8)Last_LFN_Record;
  Fill_LFN_Directory_Entry(buf + CORE.offset,Directory_Entry + (len - 1) * 13,LFN_record_FLAG,checksum);
  len--;
  write_flash_sector(buf,CORE.SectorNum);
  CORE_offset_add_32(buf);
  LFN_record_FLAG = SN;
  //��������LFN directory entry
  //do{
  while(len){
   SN--;
   Directory_Entry[len * 13] = 0;
   Fill_LFN_Directory_Entry(buf + CORE.offset,Directory_Entry + (len - 1) * 13,SN,checksum);
   len--;
   write_flash_sector(buf,CORE.SectorNum);
   CORE_offset_add_32(buf);
  }

  
  //����LFN ��Ӧ��short-filename directory entry
  Directory_Entry_buf[11] = attr;
  Directory_Entry_buf[12] = 0;
  Directory_Entry_buf[26] = (u8)(first_cluster & 0xff);  //��д�״غ�
  Directory_Entry_buf[27] = (u8)((first_cluster >> 8) & 0xff);
  Directory_Entry_buf[28] = (u8)(FileSize & 0xff);////��д�ļ�����
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
���� 
���Directory_Entry��Ŀ¼
��ڣ�Directory_Entry-
���ڣ�SUCC,FAIL
===============================================================================*/
static u8 Add_A_Directory_Entry_(u8 * Directory_Entry,u8 attr,u32 first_cluster,u32 FileSize)
{
	u16 len;
	u8 buf[512];
	u8 Directory_Entry_buf[32];
	u8 temp,i;//,j;
    
	if(attr & ATTR_DIRECTORY)
	{//���½���directory����غ���д��Ĭ������Ŀ¼����.���͡�..��
	  u32 sector_local;
	  if( Allocate_An_Empty_cluster(&first_cluster,buf)== FAIL)
	  	  return(FAIL);
	  //��ʼ��Ĭ������Ŀ¼��Ŀ¼���ֱ�ȡ��Ϊ��.���͡�..��
	  sector_local =  FirstSectorofCluster(first_cluster);
      for(i = 0;i < 11;i++)
		 buf[i] = 0x20; 
	  buf[0] = '.';//��дĿ¼����.��
	  buf[11] = attr;
	  buf[12] = 0;
	  buf[26] = (u8)(first_cluster & 0xff);  //��д��ǰĿ¼�غ�
	  buf[27] = (u8)((first_cluster >> 8) & 0xff);
	  buf[28] = 0;////��д�ļ�����
	  buf[29] = 0;
	  buf[30] = 0;
	  buf[31] = 0;
      for(i = 0;i < 11;i++)
		 buf[i + 32] = 0x20; 
	  buf[32] = '.';   //��дĿ¼����..��
	  buf[33] = '.';
	  buf[32+11] = attr;
	  buf[32+12] = 0;
	  buf[32+26] = (u8)(CORE.ClusterNum & 0xff);  //��д��ǰĿ¼��Ŀ¼���״غ�
	  buf[32+27] = (u8)((CORE.ClusterNum >> 8) & 0xff);
	  buf[32+28] = 0;////��д�ļ�����
	  buf[32+29] = 0;
	  buf[32+30] = 0;
	  buf[32+31] = 0;
	  buf[64] = 0;
	  write_flash_sector(buf,sector_local);
	}
	//����Directory_Entry��Ҫ���ٸ��ռ䣬Directory_Entryÿ���ռ�Ϊ32���ֽ�,����������len��	
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
	//Ѱ��ָ�������Ŀռ�
	if(Seek_Space_to_Write_Directory_Entry(len,buf) == SUCC)
	{ 
	  if(len == 1)  //д���ļ���
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
	  else//д���ļ���
	  { 
	 	  return(Write_LongFileName_Entry(Directory_Entry,attr,first_cluster,buf,FileSize));
	  }
	}
	else
	 return(FAIL);
}

/*===============================================================================
���� 
�����ļ�
��ڣ���
���ڣ���
===============================================================================*/  
#if complie_create_file
u8 create_file(u8 * filename)
{ 
 u16 len;
 u16 temp;
 u8 buf[260],status;
 stringcpy(filename,buf);
 CORE.FullPathType = FilePath;
 if(FullPathToSectorCluster(filename) != SUCC)  //����ļ��Ƿ��Ѿ�����
 {   //�ļ�������,�ſ��Խ��ļ�
     len = LengthofString(buf);
	 //���ļ����з���·��
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
	 {  //���·��,��ǰĿ¼�½��ļ�
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
	  //��Ŀ¼�½��ļ�
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
	 //�ļ�д�����
	 status = Add_A_Directory_Entry_(buf+temp,ATTR_ARCHIVE,0,0);
	 return(status);
  }
  else
	 return(FAIL);
}
#endif
/*===============================================================================
����
����Ŀ¼
��ڣ�foldername--Ŀ¼��·��
���ڣ���
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
 if(FullPathToSectorCluster(buf) != SUCC)  //���Ŀ¼�Ƿ��Ѿ�����
  {//Ŀ¼������,�ſ��Խ�Ŀ¼
     len = LengthofString(buf);
	 //��Ŀ¼���з���·��
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
	 { CORE.FullPathType = DirectoryPath; //���·��,��ǰĿ¼�½�Ŀ¼
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
	  //��Ŀ¼�½�Ŀ¼
	  if(FullPathToSectorCluster(TEMP) == FAIL)
		 	  {
	   return(FAIL);
	  }
	 }
	 else if(FullPathToSectorCluster(buf) == FAIL)
	 { 
	   return(FAIL);  
	  }
	 //Ŀ¼д�����
	 status = Add_A_Directory_Entry_(buf + temp,ATTR_DIRECTORY,0,0);
     return(status);
  }
  else
    return(FAIL);


}
#endif 

/*===============================================================================
����
��ǰ����λ���趨����
��ڣ���
���ڣ���
===============================================================================*/ 
#if compile_fseek
u8 f_seek(u8 FCBsn, s32 offset, u8 origin)
{     
 if(FCB[FCBsn].file_openned_flag == UnusedFlag)
    return(FAIL);//FCBδռ��,����FAIL
 switch(origin)
  { 
   case SEEK_SET:  //������ļ���ʼλ��Ѱַ 
    { 
      if(offset < 0 || (u32)offset >= FCB[FCBsn].FileSize)
        return(FAIL);
      FCB[FCBsn].cur_position = offset; 
    //�����ļ��ĵ�ǰλ�ø����ļ�����
    #if EnableFileBuf
      Update_FCB_file_buffer(FCBsn); 
    #endif
      break;
    }
   case SEEK_CUR: //������ļ���ǰλ��Ѱַ 
    { 
      if(((FCB[FCBsn].cur_position + offset) < 0 )|| 
        ((FCB[FCBsn].cur_position + offset)  >= FCB[FCBsn].FileSize)) 
         return(FAIL); 
      FCB[FCBsn].cur_position += offset;
     //�����ļ��ĵ�ǰλ�ø����ļ�����
     #if EnableFileBuf
      Update_FCB_file_buffer(FCBsn);
     #endif
      break;
    }
   case SEEK_END: //������ļ�����λ��Ѱַ 
    {
      if(offset > 0)
       return(FAIL);
       FCB[FCBsn].cur_position = FCB[FCBsn].FileSize + offset;
	   //�����ļ��ĵ�ǰλ�ø����ļ�����
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
����
���ļ�
��ڣ�u8 FCBsn,u8* buffer,u16 length
���ڣ���ȡ���ֽ���
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
     return(0); //FCBδռ��,����FAIL
   readed_bytes = 0;//�Ѷ�ȡ���ֽ�����0
   if((FCB[FCBsn].cur_position + length) > FCB[FCBsn].FileSize)
   { //�����ȡ��λ������β,��ȡlength���ַ������˿ɶ�ȡ���ַ���.
	 //length��Ϊ��ǰ���Զ�ȡ���ַ���
     length = FCB[FCBsn].FileSize - FCB[FCBsn].cur_position ;
	 if(!length)
		 return(readed_bytes);
   }  
   OffsetInbuf = (FCB[FCBsn].cur_position % (FileBUFSize * TotalFileBUFsQTYeachFCB));
   if(!OffsetInbuf)
   {//�����ļ��ĵ�ǰλ�ø����ļ�����
     Update_FCB_file_buffer(FCBsn);
   }
   temp = FileBUFSize * TotalFileBUFsQTYeachFCB - OffsetInbuf;
   point =  FCB[FCBsn].FileBUF + OffsetInbuf;
   if(temp >= length)
   { //length�ֽڵ����ݿ���ֱ�Ӵӻ������ж�����
     for(i = 0;i < length ;i++)
       buffer[i] = point[i];
     FCB[FCBsn].cur_position += length;
     return(i); //��ȡ����
   }
   else 
   { //ֱ�Ӵӻ������ж�����
     for(i = 0;i < temp ;i++)
       buffer[i] = point[i];
     FCB[FCBsn].cur_position += temp;
	 readed_bytes += i;
	 buffer += temp;
     length -= temp;
   }   
   do{
      Update_FCB_file_buffer(FCBsn);//�����ļ��ĵ�ǰλ�ø����ļ�����
      if(length <= FileBUFSize * TotalFileBUFsQTYeachFCB)
	  {  //length�ֽڵ����ݿ���ֱ�Ӵӻ������ж�����,��ȡ����
        for(i = 0;i < length ;i++)
          buffer[i] = FCB[FCBsn].FileBUF[i];
        FCB[FCBsn].cur_position += length;
	    readed_bytes += i; 
        return(readed_bytes); //��ȡ����	 
	  }  
	  else
	  { //ֱ�Ӵӻ������ж�����
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
    return(0); //FCBδռ��,����FAIL
   for(i = 0;i < length ;i++)
   {                                   
	  if(FCB[FCBsn].cur_position >= FCB[FCBsn].FileSize)
        return(i);
	  OffsetInbuf = (u16)(FCB[FCBsn].cur_position % (FileBUFSize * TotalFileBUFsQTYeachFCB));
      if ( ! OffsetInbuf)
        Update_FCB_file_buffer(FCBsn);  //�����ļ��ĵ�ǰλ�ø����ļ�����
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
����
д�ļ�
��ڣ���
���ڣ���
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
     return(0);//FCBδռ��,����FAIL
   if(FCB[FCBsn].Permission == ReadOnlyPermission || length == 0)
	 return(0); //����ļ�Ϊֻ��, �򷵻�
   writed_bytes = 0;
   OffsetInbuf = (FCB[FCBsn].cur_position % (FileBUFSize * TotalFileBUFsQTYeachFCB));
   if(!OffsetInbuf)
	  {  
         if(Update_FCB_file_buffer(FCBsn) == FAIL) 
			 return(0);//��file buffer��д����,�ٸ���FILE BUFFER
	  } 
   temp = FileBUFSize * TotalFileBUFsQTYeachFCB - OffsetInbuf;
   if(temp >= length)
   { //ֱ���򻺳�����д����,����д����
	 point =  FCB[FCBsn].FileBUF + OffsetInbuf;
     for(i = 0;i < length ;i++)
       point[i] = buffer[i];
     FCB[FCBsn].cur_position += length;
	 FCB[FCBsn].Modified_Flag = 1;
	 if(FCB[FCBsn].cur_position >= FCB[FCBsn].FileSize)
		  FCB[FCBsn].FileSize = FCB[FCBsn].cur_position ;
     if(Writeback_FCB_file_buffer(FCBsn) == FAIL)  
	    i = 0;//��file buffer��д����,�ٸ���FILE BUFFER
     return(i);
   }
   else
   { //ֱ���򻺳�����д����,
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
	    return(writed_bytes);// ��file buffer��д����,�ٸ���FILE BUFFER
     if(length <= FileBUFSize * TotalFileBUFsQTYeachFCB)
	 {//��ʣ�������ֱ���򻺳�����д,����д����
      for(i = 0;i < length ;i++)
        FCB[FCBsn].FileBUF [i] = buffer[i];
      FCB[FCBsn].cur_position += length;
	  FCB[FCBsn].Modified_Flag = 1; 
	  if(FCB[FCBsn].cur_position >= FCB[FCBsn].FileSize)
		  FCB[FCBsn].FileSize = FCB[FCBsn].cur_position ;
      if(Writeback_FCB_file_buffer(FCBsn) == FAIL) 
	    i = 0;// ��file buffer��д����,�ٸ���FILE BUFFER
	  writed_bytes += i; 
      return(writed_bytes);	 
	 }
	 else
	 {//ֱ���򻺳�����д����,д��һ��������
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
     return(0);//FCBδռ��,����FAIL
   if(FCB[FCBsn].Permission == ReadOnlyPermission)
	 return(0);//����ļ�Ϊֻ��, �򷵻�
   for(i = 0;i < length ;i++)
   {                                    
    OffsetInbuf = (u16)(FCB[FCBsn].cur_position % (FileBUFSize * TotalFileBUFsQTYeachFCB));
	if(!OffsetInbuf)
	  { 
         if(Update_FCB_file_buffer(FCBsn) == FAIL)
			 return(0);// ��file buffer��д����,�ٸ���FILE BUFFER
	  } 
      FCB[FCBsn].FileBUF [OffsetInbuf] = buffer[i];
	  if(!FCB[FCBsn].Modified_Flag)
	     FCB[FCBsn].Modified_Flag = 1 ;    
      if(FCB[FCBsn].cur_position >= FCB[FCBsn].FileSize)
		  FCB[FCBsn].FileSize = FCB[FCBsn].cur_position + 1;
	  FCB[FCBsn].cur_position ++;
   }
   if(Writeback_FCB_file_buffer(FCBsn) == FAIL)  
	   i = 0;// ��file buffer��д����,�ٸ���FILE BUFFER
   return(i);
  #endif
 #else
  return(i);
 #endif
}   
 #endif
/*===============================================================================
����
�ı䵱ǰĿ¼
��ڣ�foldername:Ŀ¼����
      mode: 0-- ����Ŀ¼; >0--�����ϲ�Ŀ¼
���ڣ�SUCC,FAIL
===============================================================================*/
#if compile_cd_folder
u8 cd_folder(u8 *foldername,u8 mode)
{ 
  u16 offset;
  if(mode)  //mode=1�������ϲ�Ŀ¼
   {
    if (CORE.CurrentDirectoryType == RootDirectory)//�����ǰĿ¼Ϊ��Ŀ¼���򷵻��ϲ�Ŀ¼ʧ��
     return(0x55);
    else
    {
	 CORE.FullPathType = DirectoryPath;
     if(FullPathToSectorCluster(CORE.current_folder) != SUCC)
	   return(FAIL);
     //Ѱ��Ŀ¼�ָ������ܣ�,����ǰĿ¼��ȥһ��Ŀ¼
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
   else  //������Ŀ¼
   {
    CORE.FullPathType = DirectoryPath;      
    if(FullPathToSectorCluster(foldername) == SUCC)//���Ŀ¼�Ƿ���ڣ�
	{ 
	  if((* foldername) >= 'C' && ( * foldername ) <= 'Z')
	  {
       if(* (foldername + 1) == ':' &&  * (foldername + 2 ) == '\\')
		  {//����·����ֱ�ӿ���CORE.current_folder
           stringcpy(foldername,CORE.current_folder);  
		  }
		  else
		  { //���·������CORE.current_folder���Ӻ󣬿�������CORE.current_folder
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
	//����Ŀ¼����ز���
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
����
������ļ���Ŀ¼��
��ڣ�buf
���ڣ�SUCC,FAIL
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
  if(!buf[CORE.offset])//Ŀ¼��֮����ǿյ���
   NextDirectoryEntryEMPTY = 1;
  else
   NextDirectoryEntryEMPTY = 0;  

  do{
	  //λ��ת��ǰһ��Ŀ¼��
    if(offset == 0)
    { //���������߽磭��ִ�б߽紦��
      write_flash_sector(buf,Sector);
      Sector--; 
      offset = 512 - 32;
      if(CORE.DirectoryType == RootDirectory)
	  { 
		  if(Sector < CORE.FirstRootDirSecNum)
	      return(SUCC); 
	  }        
      else if (Sector  < FirstSectorofCluster(Cluster))
	  { //�����ر߽磬����ִ�б߽紦��
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
	  if(NextDirectoryEntryEMPTY)//��Ŀ¼����Ϊɾ��
        buf[offset] = 0x0;
	  else
        buf[offset] = 0xe5;
    }
    else
	{
		if(First_Entry_Deleted == 0)
		{
	      if(NextDirectoryEntryEMPTY)
           buf[offset] = 0x0;//��Ŀ¼����Ϊɾ��
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
����
�ļ�ɾ��
��ڣ�filename:�ļ�·��
���ڣ�SUCC,FAIL
===============================================================================*/ 
#if complie_delete_file
u8 delete_file( u8 *filename)
{ 
 u8 buf[512];
 CORE.FullPathType = FilePath; 
 if(FullPathToSectorCluster(filename) == SUCC)//����ļ��Ƿ���ڣ�
    {//����ļ��Ƿ񱻴򿪣�
	 if(Check_FileOpened_Status(CORE.ClusterOfDirectoryEntry,0xff) == FileAlreadyopenedByOtherHandle)
         return(FAIL);//�ļ����У�ɾ��ʧ��
	 //�����Ӻ�����ɾ���ļ�Ŀ¼��
     if(Remove_LONGFILENAME(buf,filename) != SUCC)
         return(FAIL);
	 if(CORE.ClusterOfDirectoryEntry)//ɾ���ļ�ռ�õĴ���
       FreeClusterChain(CORE.ClusterOfDirectoryEntry); 
     return(SUCC); 
   }
 else
     return(FAIL);
}
#endif
/*===============================================================================
����
Ŀ¼ɾ��
��ڣ�char * foldername--·��+Ŀ¼��
���ڣ�SUCC,FAIL
===============================================================================*/    
#if complie_delete_folder
u8 delete_folder(u8 * foldername)
{
 u8 buf[512];
 u32 sector;
 CORE.FullPathType = DirectoryPath; 
 if(FullPathToSectorCluster(foldername) == SUCC)//����ļ��Ƿ���ڣ�
   {
     sector = FirstSectorofCluster(CORE.ClusterOfDirectoryEntry);
	 read_flash_sector(buf,sector);
     if(buf[64] != 0)  //������ɾ���ǿ�Ŀ¼
	 {
	  return(FAIL); 
	 }
     //�����Ӻ�����ɾ��Ŀ¼��
	 Remove_LONGFILENAME(buf,foldername);
     //ɾ��Ŀ¼ռ�õĴ���
     FreeClusterChain(CORE.ClusterOfDirectoryEntry);
     return(SUCC);
   }
  else
    return(FAIL);

     
} 
#endif
/*===============================================================================
����    
�������ļ�
��ڣ�oldfilename:ָ����ļ���newfilename:ָ�����ļ�(���ļ���)
���ڣ�SUCC,FAIL
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
 if(FullPathToSectorCluster(oldfilename) == SUCC)//�����ļ��Ƿ����?
 {  //�����ļ��Ƿ񱻴�?
	if(Check_FileOpened_Status(CORE.ClusterOfDirectoryEntry,0xff) == FileAlreadyopenedByOtherHandle)
           return(FAIL);  //���ļ�������,����ɾ��
	//�״غ�,FileSize����
	FileSize = CORE.FileSize;
	first_cluster = CORE.ClusterOfDirectoryEntry;
    len = LengthofString(newfilename);
    temp = len - 1;
	//�����ļ�·���з�����ļ���
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
	 //�Ӿ��ļ�·���з����·��
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
	  /*  ������ļ������е��ظ����,�����ظ��������ֱ���˳����� */
      concanenateString(oldfilename,newfilename + temp);
      if(FullPathToSectorCluster(oldfilename) == SUCC)
	      return(FAIL);
	}
	else
	{
	  oldfilename[temp2 + 1] = 0;
      concanenateString(oldfilename,newfilename + temp);
      /*  ������ļ������е��ظ����,�����ظ��������ֱ���˳����� */
	  if(FullPathToSectorCluster(oldfilename) == SUCC)
	      return(FAIL);

	}

    stringcpy(oldfilename1,oldfilename);
	CORE.FullPathType = FilePath; 
    FullPathToSectorCluster(oldfilename);
    Remove_LONGFILENAME(buf,oldfilename);//ɾ��Ŀ¼��������仯

    if(temp2 == 0)
	{ //���ļ���Ϊ���·��Ѱַ
	  CORE.FullPathType = DirectoryPath; 
      FullPathToSectorCluster(CORE.current_folder); 
	}
	else
	{
		if(temp2 < 3 && oldfilename[1] == ':')
			oldfilename[temp2 + 1] = 0;
		else
            oldfilename[temp2] = 0;
		//���ļ���Ϊ����·��Ѱַ
		CORE.FullPathType = DirectoryPath;
        FullPathToSectorCluster(oldfilename);

	}
	//д�����ļ�Ŀ¼���ɸ���
    Add_A_Directory_Entry_(newfilename + temp,ATTR_ARCHIVE,first_cluster,FileSize);
    return(SUCC);
  }
 else
   return(FAIL);
}   
#endif

/********************************************************************************************
����
����Ŀ¼(��disk enumerationʹ��)
��ڣ���
���ڣ�SUCC,FAIL
********************************************************************************************/
static u8 cd__folder_for_disk_enum()
{

     CORE.FullPathType = DirectoryPath;
	 //���Ŀ¼�Ƿ����?
     if(FullPathToSectorCluster(CORE.current_folder_for_disk_enum) != SUCC)
	      return(FAIL);
	 //����Ŀ¼����
	 if(LengthofString(CORE.current_folder_for_disk_enum) <= 3)
	 {//�����Ŀ¼����
      CORE.SectorNum = CORE.FirstRootDirSecNum; 
	  CORE.CurrentDirectoryType = RootDirectory; 
	 }
	 else
	 { //����Ǹ�¼����
	   CORE.CurrentDirectoryType = NoneRootDirectory;
       CORE.ClusterNum = CORE.ClusterOfDirectoryEntry;
	   CORE.ClusterNOofCurrentFolder = CORE.ClusterOfDirectoryEntry;
       CORE.SectorNum =  FirstSectorofCluster(CORE.ClusterNum); 
	 }
	 CORE.offset = 0;
	 return(SUCC);              
}

/********************************************************************************************
����
�˳�Ŀ¼(��disk enumerationʹ��)
��ڣ���
���ڣ�SUCC,FAIL
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
	 //��current_folder_for_disk_enum�з������һ��Ŀ¼
	 //��������붪����Ŀ¼����folderSplit[],������CD__�����λ�ö�λ
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
     //������һ��Ŀ¼
	 cd__folder_for_disk_enum();
	 
	 do{ //CD__֮���λ�ö�λ��������Ŀ¼֮��
         //��ȡһ��ENTRY 
		 if(GetEntryFromDirectory(entry,extension,Get_All_Entries) == FAIL)
		  {
		     if(CORE.CurrentDirectoryType == RootDirectory)
				 return(0x55);
		  }

	      //��ȡ�˷�Ŀ¼,��������һ��ENTRY
          if(!(CORE.Entry_Attr & ATTR_DIRECTORY))
			  continue;
          //��ȡ��Ŀ¼�Ƿ��뱻����Ŀ¼��ͬ,��ͬ�˳�
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
����
����ö�ٵĵ�ǰλ��
��ڣ���
���ڣ���
********************************************************************************************/
save_enum_vars(u8 disk_or_folder)
{
   //����ClusterNum,SectorNum,offset
   CORE.DIR_ENUM_ClusterNum[disk_or_folder] = CORE.ClusterNum;  
   CORE.DIR_ENUM_SectorNum[disk_or_folder] =CORE.SectorNum ;
   CORE.DIR_ENUM_offset[disk_or_folder] = CORE.offset;
   //����ClusterNum,SectorNum,offset
   CORE.DIR_ENUM_ClusterOfDirectoryEntry[disk_or_folder] = CORE.ClusterOfDirectoryEntry;
   CORE.DIR_ENUM_DirectoryType[disk_or_folder] = CORE.DirectoryType; 
   CORE.DIR_ENUM_FullPathType[disk_or_folder] = CORE.FullPathType;
   CORE.DIR_ENUM_CurPathType[disk_or_folder] = CORE.CurPathType; 

}

/********************************************************************************************
����
�ָ�ö�ٵĵ�ǰλ��
��ڣ���
���ڣ���
********************************************************************************************/
restore_enum_vars(u8 disk_or_folder)
{
   //�ָ�ClusterNum,SectorNum,offset
   CORE.ClusterNum = CORE.DIR_ENUM_ClusterNum[disk_or_folder];   
   CORE.SectorNum = CORE.DIR_ENUM_SectorNum[disk_or_folder];
   CORE.offset = CORE.DIR_ENUM_offset[disk_or_folder];
   //�ָ�ClusterNum,SectorNum,offset
   CORE.ClusterOfDirectoryEntry = CORE.DIR_ENUM_ClusterOfDirectoryEntry[disk_or_folder];  
   CORE.DirectoryType = CORE.DIR_ENUM_DirectoryType[disk_or_folder]; 
   CORE.FullPathType = CORE.DIR_ENUM_FullPathType[disk_or_folder];
   CORE.CurPathType = CORE.DIR_ENUM_CurPathType[disk_or_folder]; 

}
/********************************************************************************************
����
�оٵ�ǰĿ¼�µ������ļ���Ŀ¼(��Ŀ¼���о�)
��ڣ���
���ڣ���
********************************************************************************************/  
#if complie_folder_dir
u8 folder_enumeration(u8 *return_string,u8 mode,u8 *ATTR)
{ 
  u8  Extension[20];
  u16 temp;
  if(mode == 0x0)
  {//���¿�ʼö��
   CORE.FullPathType = DirectoryPath; 
   //���뵱ǰĿ¼
   if(cd_folder(CORE.current_folder,0) != SUCC)
	   return(FAIL);
   CORE.offset = 0;
   //���浱ǰĿ¼�Ĳ���
   if(CORE.CurrentDirectoryType ==  RootDirectory)
      CORE.SectorNum = CORE.FirstRootDirSecNum; 
   else
     {
       CORE.ClusterNum = CORE.ClusterNOofCurrentFolder;
       CORE.SectorNum = FirstSectorofCluster(CORE.ClusterNum); 
     }
     save_enum_vars(FOLDER_ENUM);//����ö�ٵĵ�ǰλ��
  } 

  restore_enum_vars(FOLDER_ENUM);//�ָ�ö�ٵĵ�ǰλ��
do{
  stringcpy(CORE.current_folder,return_string);
  temp = LengthofString(return_string);
  if(return_string[temp-1] != '\\')
  {
   return_string[temp] = '\\';
   return_string[temp+1] = 0;
  }
  //��Ŀ¼�ж�ȡһ��ENTRY
  if(GetEntryFromDirectory(return_string+LengthofString
	  (return_string), Extension,Get_All_Entries) == SUCC)
  {
   temp = LengthofString(return_string);
   *ATTR = CORE.Entry_Attr;
   //��FILE EXTENSION�������
   if(temp > 0 && (!((*ATTR) & ATTR_DIRECTORY)))
   {
    if(Extension[0] != 0)
	{
     return_string[temp] = '.';
     return_string[temp+1] = 0;
	 concanenateString(return_string,Extension);
	}
   }

   //����"." ".."
  if(return_string[temp - 1] == '.' || (return_string[temp - 1] == '.'
	  && return_string[temp - 2] == '.'))
	  continue;
   save_enum_vars(FOLDER_ENUM);//����ö�ٵĵ�ǰλ��
   return(SUCC);
  }
  else
   break;
 }while(1);
 return(FAIL);
} 
#endif
/*******************************************************************************************
����
�о�disk�������ļ���Ŀ¼   
��ڣ���
���ڣ���
********************************************************************************************/ 
u8 disk_enumeration(u8 *return_string,u8 mode,u8* ATTR)
{
 u16 temp; 
 u8 Extension[20];
 if( ! mode)
 {//���¿�ʼö��
   stringcpy("C:\\",CORE.current_folder_for_disk_enum);
   cd__folder_for_disk_enum();
   save_enum_vars(DISK_ENUM); //����ö�ٵĵ�ǰλ�� 
 }  
   restore_enum_vars(DISK_ENUM);//�ָ�ö�ٵĵ�ǰλ��
 do{
    stringcpy(CORE.current_folder_for_disk_enum,return_string);
    temp = LengthofString(return_string);

    if(return_string[temp-1] != '\\')
	  {
         return_string[temp] = '\\';
         return_string[temp+1] = 0;
	  }
	//��Ŀ¼�ж�ȡһ��ENTRY
     if(GetEntryFromDirectory(return_string+LengthofString(return_string), 
	      Extension,Get_All_Entries) == FAIL)
	 {   
	     //������һ��Ŀ¼
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
           //��FILE EXTENSION�������
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
		          //����"." ".."
		          if(return_string[temp - 1] == '.' || (return_string[temp - 1] == '.'
					  && return_string[temp - 2] == '.'))
					  continue;
				  stringcpy(return_string,CORE.current_folder_for_disk_enum);
                  //����Ŀ¼ȥö��
				  cd__folder_for_disk_enum();
		   }

	  }
   save_enum_vars(DISK_ENUM);//����ö�ٵĵ�ǰλ�� 
   return(SUCC);
 }while(1);
  
}
/********************************************************************************************
����
�ļ����Һ���
��ڣ�1��mode = 0���ڵ�ǰĿ¼�²��ң�2��mode=1�������������в���
      filename-�������ļ�,Return_string
���ڣ���
********************************************************************************************/
#if complie_find_file
u8 find_file(u8 * filename,u8 mode,u8* Return_string)
{   
	u16 temp,POINTER;
	u8 ATTR,disk_enum_mode;

    if(! mode)
	{  //�ڵ�ǰĿ¼�²����ļ�
      stringcpy(CORE.current_folder,Return_string);
      temp = LengthofString(Return_string);
	  if(*(Return_string + temp - 1) != '\\')
	  {
	    Return_string[temp] = '\\';
		Return_string[temp + 1] = 0;
	  }
      concanenateString(Return_string,filename);
     CORE.FullPathType = FilePath;
	 //ֱ����FullPathToSectorCluster()����ļ��Ƿ����?
     if(FullPathToSectorCluster(Return_string) != SUCC)
	      return(FAIL);//�ļ�������,����ʧ��
	 return(SUCC);//�ļ�����,���ҳɹ�
	
	}
	else
	{//�ڴ����в���
     disk_enum_mode = 0;
	 //��disk_enumeration()��ȡ�������е�ENTRY
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
        //����ļ�������ͬ?
		if(stringcmp(Return_string + POINTER + 1,filename) == SUCC)
			return(SUCC);
	 } 
     return(FAIL);	
	}
}
#endif
/********************************************************************************************
����
��ѯ����������ʣ��ռ� --ͨ��������FAT����ʵ��
��ڣ�partition_id(Supported ID:form 'C' to 'F'),u32 *volume_capacity, u32 *volume_free_space
���ڣ�SUCC  (���صķ���������ʣ��ռ���512 bytes����Ϊ��λ)
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
  //��ȡ������BPB��Ϣ
  Read_partition_PBP((u8)(partition_id - 'C'));
   if (CORE.system_id == 0x6)
   {   
     *volume_free_space = 0;
	 i = 0;
	 x = 0;
	 while(i < (CORE.CountofClusters) + 2){
	 //������CORE.FirstSectorofFAT1��ʼ���
      if(read_flash_sector(buf,CORE.FirstSectorofFAT1 + x) == SUCC) 
       {//���һ��������
        for (j = 0;j < 512;j +=2)
         { 
          if(buf[j] == 0 && buf[j + 1] == 0)
             (*volume_free_space) ++;
		  i++;
		  if(i >= (CORE.CountofClusters + 2))
			  break;
         }
		 x++;//������+1
        }
       else
        {
          *volume_capacity = 0;
          *volume_free_space = 0;
          return(FAIL);
         } 
	 }
       *volume_free_space *=  BPB.sector_per_cluster;//�������,���ؽ��
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
