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

 #define FILE_NAME 0xff
 #define FILE_EXTENSION 0xfe  
 #define OK 0
 #define unOK 0xff

 //File attributes
 #define ATTR_READ_ONLY (1 << 0)
 #define ATTR_HIDDEN    (1 << 1)
 #define ATTR_SYSTEM    (1 << 2)
 #define ATTR_VOLUME_ID (1 << 3)
 #define ATTR_DIRECTORY (1 << 4)
 #define ATTR_ARCHIVE   (1 << 5)
 #define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)
 //
 #define Last_LFN_Record (1 << 6) 
 //Partition Type ----0C-FAT32,06-FAT16 ect..
 #define FAT32 0xc
 #define FAT16 0x6
 //Directory Type
  //Maximum FullPath chars -In compliance with Microsoft Longfilename Specification
 #define Maximum_File_Path_Name 260
 //FAT Error Code Definitions As Following  
 //fseek origin
 #define SEEK_SET 0
 #define SEEK_CUR 1
 #define SEEK_END 2
 //
 #define EpathLengthsOVERFLOW 0xfa
 #define EAllocate_FCB 0xff
 #define EFree_FCB 0xfe
 #define Folder_Enumeration_Complete 1
 #define Create_File_Failed  2
 #define Create_Folder_Failed 3
 #define Rename_Of_File_Failed 4
 #define Open_File_Failed 5
 #define Open_Folder_Failed 6
 #define Read_File_Failed 7
 #define Write_File_Failed 8
 #define Delete_File_Failed 9
 #define Delete_Folder_Failed 10 
 #define Inquiry_Volume_Failed 11
 #define Autoformat_FAT_Filesystem_Failed 12
 #define  Invalid_PATH_Name 0xff
 #define  File_Extension_Over_limited_3_Char 0xfe
 #define  Invalid_chars_Found_In_File_Path  0xfd
 #define  File_Not_Found 0xfc 
 #define  LastSplitedNameofPath 0xfd
 //File opened flag
#define FileAlreadyopenedByOtherHandle 0
#define FileUnopenedByOtherHandle  0xff
//GetEntryFromDirectory mode
#define Get_Selected_ENTRIES 0 
#define Get_All_Entries 1
//Definition of FAT filesystem CORE Struct
struct core_{
  u8 current_folder[Maximum_File_Path_Name];//���浱ǰĿ¼--ֻ��cd_folder()�ܸı䵱ǰĿ¼
                                            //ϵͳ������Ĭ�ϵ�ǰĿ¼Ϊ"C:\"
  u8 current_folder_for_disk_enum[Maximum_File_Path_Name];
  u32 ClusterNOofCurrentFolder;//����Ŀ¼Ŀ¼������ClusterNO
  u8 CurrentDirectoryType; //RootDirectory,NomeRootDirectory
  u8 PartitionID; //��ǰBPB�ж�Ӧ��PartitionID
  u8 system_id; //Partition type:0C-FAT32,06-FAT16 ect..
  u32 relative_sector; //Begining sector address of Current Partition
  u32 total_sector;   //Total sectors of Current Partition
  u32 FirstDataSector; ////The start of the data region, the first sector of cluster 2
  u32 FirstSectorofFAT1;
  u32 FirstSectorofFAT2;
  u16 FirstRootDirSecNum;  
  u16 RootDirSectors; //the count of sectors occupied by the root directory
  u16 CountofClusters; //count of clusters
  u32 DataSec;  //we determine the count of sectors in the data region of the volume
  //Public variables for path resolution
  u32 ClusterNum;   //��ŵ�ǰEnumerated Directory Entry����Directory��ClusterNum,SectorNum,offset
  u32 SectorNum;
  u16 offset;
  u32 ClusterOfDirectoryEntry;  //���Directory Entry32�ֽ��ж�Ӧfirst Cluster Num
  u32 FileSize;
  u8 DirectoryType; 
  u8 FullPathType;
  u8 CurPathType; 
  //Variables for storage of pre-entry position
  u32 PreEntrySectorNum;
  u16 PreEntryoffset;
  //Variables for folder enumeration
  u32 DIR_ENUM_ClusterNum[2];   //��ŵ�ǰEnumerated Directory Entry����Directory��ClusterNum,SectorNum,offset
  u32 DIR_ENUM_SectorNum[2];
  u16 DIR_ENUM_offset[2];
  u32 DIR_ENUM_ClusterOfDirectoryEntry[2];  //���Directory Entry32�ֽ��ж�Ӧfirst Cluster Num
  u8 DIR_ENUM_DirectoryType[2]; 
  u8 DIR_ENUM_FullPathType[2];
  u8 DIR_ENUM_CurPathType[2]; 
  u8 Entry_Attr;
};
 #define FOLDER_ENUM 0
 #define DISK_ENUM 1

 #define RootDirectory 0x0   //��ǰĿ¼����ΪRootDirectory��
                             //RootDirectory�Ǵ����һƬ�����Ĺ̶�����
 #define NoneRootDirectory 0x1 //NoneRootDirectory�Ǵ���ڴ��������ӵ�scatterr region
 //Path Type
 #define DirectoryPath 0x10  //����CD����ʹ�õ�·��Type:DirectoryPath
 #define FilePath 0x3       //����open����ʹ�õ�·��Type:FilePath,��·����������ļ��� 
 //Definition of FileBufSize for FileRead/Write
 #define EnableFileBuf 1  
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#if EnableFileBuf
   #define TotalFileBUFsQTYeachFCB 1 //TotalFileBUFsQTY for each FCB 
#else 
   #define TotalFileBUFsQTYeachFCB 0 //TotalFileBUFsQTY for each FCB 
#endif
 #define FileBUFSize 512 //This is a fixed size,!!!Pls don't change!!! 
 #define MaximumFCB 2     //which means 4 files can be opened at the same time
 #define UnusedFlag 0x0
 #define UsedFlag 0x1   
//Read File Optimization Selector--ѡ��Read Fileʵ�ֵ����ַ���֮һ
//0--��Ҫ������
//1--�Ի�������������
#define Read_File_Optimization_Selector 1
//Write File Optimization Selector--ѡ��Write Fileʵ�ֵ����ַ���֮һ
//0--��Ҫ������
//1--�Ի�������������
#define Write_File_Optimization_Selector 1

 //Permission
 #define FullPermission 0
 #define ReadOnlyPermission 1  
//FCB Struct   
struct FileControlBlock{
  //Public variables and FileBlockBUF for fileRead/Write
  u8 file_openned_flag;
  u32 CurBlockInBUF;
  
  u32 CurClusterInBUF;
  u32 ClusterSNInBUF;

  u32 CurClusterInBUF_for_read;
  u32 ClusterSNInBUF_for_read;
 #if EnableFileBuf
  u8 FileBUF[FileBUFSize * TotalFileBUFsQTYeachFCB]; 
 #endif                                              
  u32 FileSize;
  u32 cur_position;
  u32 FirstClusterOfFile; 
  u8  Permission;
  u8  Modified_Flag;
  u32 Entry_Storedin_Sector;  //Storage sectorNUM of File Directory entry 
  u16 Entry_Storedin_Sector_Offset; //Storage of offset in sector of File Directory entry
}; 
//����ʱ�䶯�������
//0-������������-����
#define enable_time_transaction 0

//Definition of BPB Struct
struct partition_bpb{
  u16 bytes_per_sector;//ÿ�����ֽ���
  u8 sector_per_cluster; //ÿ��������
  u16 reserved_sector;  //����������
  u8 numbers_of_FAT;//FAT������
  u16 boot_entries;//��Ŀ¼��������FAT12/16ʹ��
  u16 TotSec16; //This field is the old 16-bit total count of sectors on the volume.
  u8 media_descriptor; //ý��������
  u16 sectors_per_FAT; //ÿ��FAT��ռ�õ�����������FAT12/16ʹ��
  u16 sectors_per_track; //ÿ��������
  u16 number_of_head; //��ͷ��
  u32 BPB_HiddSec; //����������
  u32 BPB_TotSec32;//��������������FAT32��������
  u8 BS_DrvNum;
  u8 BS_Reserved1;
  u8 BS_BootSig;
  u32 BS_VolID;
  u8 BS_VolLab[11];
  u8 BS_FilSysType[8];
};

//Definition of Directory Entry Struct
struct Directory_Entry_{
  u8 filename[8];//�ļ���
  u8 file_extention[3]; //�ļ���չ��
  u8 file_attribute;//����
  u8 reserved;
  u8 create_time_10ms;//����ʱ���10����λ
  u8 file_created_time[2];//�ļ�����ʱ��
  u8 file_created_date[2];//�ļ���������
  u8 last_access_date[2];//�ļ�����������
  u8 first_cluster_number_high2bytes[2]; //�ļ��״غŸ�16λ
  u8 recent_modified_time[2];//�ļ�����޸�ʱ��
  u8 recent_modified_data[2];//�ļ�����޸�����
  u8 first_cluster_number_low2bytes[2]; //�ļ��״غŵ�16λ
  u8 file_length[4];//�ļ�����
};
struct mbr{
U8 BS_jmpBoot[3];
U8 BS_OEMName[8];
//bpb region
U8 BPB_BytsPerSec[2];
U8 BPB_SecPerClus;
U8 BPB_RsvdSecCnt[2];
U8 BPB_NumFATs;
U8 BPB_RootEntCnt[2];
U8 BPB_TotSec16[2];
U8 BPB_Media;
U8 BPB_FATSz16[2];
U8 BPB_SecPerTrk[2];
U8 BPB_NumHeads[2];
U8 BPB_HiddSec[4];
U8 BPB_TotSec32[4];
//Fat12 and Fat16 Structure Starting at Offset 36
U8 BS_DrvNum;
U8 BS_Reserved1;
U8 BS_BootSig;
U8 BS_VolID[4];
U8 BS_VolLab[11];
U8 BS_FilSysType[8];
//offset 62
U8 pad[448];
//0x55,0xaa
U8 flag[2];

};
//Long Name Directory Entry
struct LongNameDirectoryEntry{
u8 dir_lname1[10];//long name string
u8 dir_sig;   // signature byte
u8 dir_attr;   // file attributes
u8 dir_flags;   //flags byte (TBD)
u8 dir_chksum; //checksum of 8.3 name
u8 dir_lname2[12];// long name string
u8 dir_first[2];     //first cluster number, must be 0
u8 dir_lname3[4]; //long name string
};
// disk enumeration configuration
#define filter_hidden_entry 0
//compile functions of FAT filesystem ?
#define complie_create_file 1
#define complie_create_floder 1 
#define complie_open_file 1
#define complie_close_file 1
#define complie_rename_file 1
#define complie_folder_dir 1 
#define compile_fseek 1
#define complie_read_file 1
#define complie_write_file 1
#define compile_cd_folder 1
#define complie_delete_file 1
#define complie_delete_folder 1
#define complie_volume_inquiry 1
#define complie_find_file 1
#define complie_FAT_filesystem_initialiation 1
#define complie_FAT16_filesystem_autoformat 1 


