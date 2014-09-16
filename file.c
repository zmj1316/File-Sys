/*----------------------------------------------------------------------------/
/  FatFs - FAT file system module  R0.10b                (C)ChaN, 2014
/-----------------------------------------------------------------------------/
/ FatFs module is a generic FAT file system module for small embedded systems.
/ This is a free software that opened for education, research and commercial
/ developments under license policy of following terms.
/
/  Copyright (C) 2014, ChaN, all right reserved.
/
/ * The FatFs module is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-----------------------------------------------------------------------------/
/*由Key Zhang修改用作计算机硬件基础课程作业基于原作者开源协议仅供非商业使用，
转载需保留以上信息
Copyright (C) 2014, ChaN, all right reserved.*/
#include "file.h"			/* Declarations of FatFs API */
#include "diskio.h"		/* Declarations of disk I/O functions */



#define	ENTER_FF(fs)
#define LEAVE_FF(fs, res)	return res


#define	ABORT(fs, res)		{ fp->err = (BYTE)(res); LEAVE_FF(fs, res); }


/* 扇区大小默认512 */
#if (_MAX_SS < _MIN_SS) || (_MAX_SS != 512 && _MAX_SS != 1024 && _MAX_SS != 2048 && _MAX_SS != 4096) || (_MIN_SS != 512 && _MIN_SS != 1024 && _MIN_SS != 2048 && _MIN_SS != 4096)
#error Wrong sector size configuration.
#endif
#if _MAX_SS == _MIN_SS
#define	SS(fs)	((UINT)_MAX_SS)	/* 固定大小 */
#else
#define	SS(fs)	((fs)->ssize)	/* 可变大小 */
#endif






#define _DF1S	0



/* Character code support macros */
#define IsUpper(c)	(((c)>='A')&&((c)<='Z'))
#define IsLower(c)	(((c)>='a')&&((c)<='z'))
#define IsDigit(c)	(((c)>='0')&&((c)<='9'))



#define IsDBCS1(c)	0
#define IsDBCS2(c)	0


/* Name status flags文件名状态 */
#define NS			11		/* 文件名大小 */
#define NS_LOSS		0x01	/* Out of 8.3 format */
#define NS_LFN		0x02	/* Force to create LFN entry 使用长文件名*/
#define NS_LAST		0x04	/* Last segment */
#define NS_BODY		0x08	/* Lower case flag (body) 文件名主体*/
#define NS_EXT		0x10	/* Lower case flag (ext) 扩展名*/
#define NS_DOT		0x20	/* Dot entry */


/* FAT sub-type boundaries 默认FAT16 簇数量*/
#define MIN_FAT16	4086U	/* Minimum number of clusters for FAT16 */
#define	MIN_FAT32	65526U	/* Minimum number of clusters for FAT32 */


/* FatFs refers the members in the FAT structures as byte array instead of
/ structure member because the structure is not binary compatible between
/ different platforms */
/*以下为文件系统结构偏移*/
/*引导扇区不处理 不打算格式化了*/
#define BS_jmpBoot			0		/* Jump instruction (3) 启动跳转*/
#define BS_OEMName			3		/* OEM name (8) 制造商*/
#define BPB_BytsPerSec		11		/* Sector size [byte] (2) */
#define BPB_SecPerClus		13		/* Cluster size [sector] (1) */
#define BPB_RsvdSecCnt		14		/* Size of reserved area [sector] (2) */
#define BPB_NumFATs			16		/* Number of FAT copies (1) */
#define BPB_RootEntCnt		17		/* Number of root directory entries for FAT12/16 (2) */
#define BPB_TotSec16		19		/* Volume size [sector] (2) */
#define BPB_Media			21		/* Media descriptor (1) */
#define BPB_FATSz16			22		/* FAT size [sector] (2) */
#define BPB_SecPerTrk		24		/* Track size [sector] (2) */
#define BPB_NumHeads		26		/* Number of heads (2) */
#define BPB_HiddSec			28		/* Number of special hidden sectors (4) */
#define BPB_TotSec32		32		/* Volume size [sector] (4) */
#define BS_DrvNum			36		/* Physical drive number (2) */
#define BS_BootSig			38		/* Extended boot signature (1) */
#define BS_VolID			39		/* Volume serial number (4) */
#define BS_VolLab			43		/* Volume label (8) */
#define BS_FilSysType		54		/* File system type (1) */
#define BPB_FATSz32			36		/* FAT size [sector] (4) */
#define BPB_ExtFlags		40		/* Extended flags (2) */
#define BPB_FSVer			42		/* File system version (2) */
#define BPB_RootClus		44		/* Root directory first cluster (4) */
#define BPB_FSInfo			48		/* Offset of FSINFO sector (2) */
#define BPB_BkBootSec		50		/* Offset of backup boot sector (2) */
#define BS_DrvNum32			64		/* Physical drive number (2) */
#define BS_BootSig32		66		/* Extended boot signature (1) */
#define BS_VolID32			67		/* Volume serial number (4) */
#define BS_VolLab32			71		/* Volume label (8) */
#define BS_FilSysType32		82		/* File system type (1) */
/*文件系统信息*/
#define	FSI_LeadSig			0		/* FSI: Leading signature (4) */
#define	FSI_StrucSig		484		/* FSI: Structure signature (4) */
#define	FSI_Free_Count		488		/* FSI: Number of free clusters (4) 空闲簇数*/
#define	FSI_Nxt_Free		492		/* FSI: Last allocated cluster (4) 最后分配簇*/
#define MBR_Table			446		/* MBR: Partition table offset (2) */
#define	SZ_PTE				16		/* MBR: Size of a partition table entry */
#define BS_55AA				510		/* Signature word (2) */
/*目录数据偏移量*/
#define	DIR_Name			0		/* Short file name (11) 文件名*/
#define	DIR_Attr			11		/* Attribute (1) 文件属性*/
#define	DIR_NTres			12		/* NT flag (1) */
#define DIR_CrtTimeTenth	13		/* Created time sub-second (1) */
#define	DIR_CrtTime			14		/* Created time (2) */
#define	DIR_CrtDate			16		/* Created date (2) */
#define DIR_LstAccDate		18		/* Last accessed date (2) */
#define	DIR_FstClusHI		20		/* Higher 16-bit of first cluster (2) */
#define	DIR_WrtTime			22		/* Modified time (2) */
#define	DIR_WrtDate			24		/* Modified date (2) */
#define	DIR_FstClusLO		26		/* Lower 16-bit of first cluster (2) */
#define	DIR_FileSize		28		/* File size (4) */
/*长文件名不支持留着看看吧*/
#define	LDIR_Ord			0		/* LFN entry order and LLE flag (1) */
#define	LDIR_Attr			11		/* LFN attribute (1) */
#define	LDIR_Type			12		/* LFN type (1) */
#define	LDIR_Chksum			13		/* Sum of corresponding SFN entry */
#define	LDIR_FstClusLO		26		/* Filled by zero (0) */

#define	SZ_DIR				32		/* 目录记录大小 */
#define	LLE					0x40	/* Last long entry flag in LDIR_Ord 用于长文件名*/
#define	DDE					0xE5	/* Deleted directory entry mark in DIR_Name[0] 删除标记*/
#define	NDDE				0x05	/* Replacement of the character collides with DDE 取消删除标记*/


/*------------------------------------------------------------*/
/* Module private work area                                   */
/*------------------------------------------------------------*/
/* Note that uninitialized variables with static duration are
/  guaranteed zero/null as initial value. If not, either the
/  linker or start-up routine is out of ANSI-C standard.
*/
/*当同时使用多个卷时分配多个结构*/
#if _VOLUMES >= 1 || _VOLUMES <= 10
static
FATFS *FatFs[_VOLUMES];		/* Pointer to the file system objects (logical drives) */
#else
#error Number of volumes must be 1 to 10.
#endif

static
WORD Fsid;					/* File system mount ID 挂载id*/


#if _USE_LFN == 0			/* No LFN feature */
#define	DEF_NAMEBUF			BYTE sfn[12]
#define INIT_BUF(dobj)		(dobj).fn = sfn
#define	FREE_BUF()
#endif


#ifdef _EXCVT
static
const BYTE ExCvt[] = _EXCVT;	/* Upper conversion table for extended characters */
#endif





/*-----------------------------------------------------------------------*/
/* String functions             字符串操作                                        */
/*-----------------------------------------------------------------------*/

/* Copy memory to memory 内存复制*/
static
void mem_cpy (void* dst, const void* src, UINT cnt) {
	BYTE *d = (BYTE*)dst;
	const BYTE *s = (const BYTE*)src;

	while (cnt--)
		*d++ = *s++;
}

/* Fill memory */
static
void mem_set (void* dst, int val, UINT cnt) {
	BYTE *d = (BYTE*)dst;

	while (cnt--)
		*d++ = (BYTE)val;
}

/* Compare memory to memory */
static
int mem_cmp (const void* dst, const void* src, UINT cnt) {
	const BYTE *d = (const BYTE *)dst, *s = (const BYTE *)src;
	int r = 0;

	while (cnt-- && (r = *d++ - *s++) == 0) ;
	return r;
}

/* Check if chr is contained in the string */
static
int chk_chr (const char* str, int chr) {
	while (*str && *str != chr) str++;
	return *str;
}




/*-----------------------------------------------------------------------*/
/* Move/Flush disk access window in the file system object               */
/*-----------------------------------------------------------------------*/
/*确认缓存写入完成（同步读写窗口）*/
#if !_FS_READONLY
static
FRESULT sync_window (
	FATFS* fs		/* File system object */
)
{
	DWORD wsect;
	UINT nf;


	if (fs->wflag) {	/* Write back the sector if it is dirty 检查是否缓存*/
		wsect = fs->winsect;	/* Current sector number 当前缓存的扇区地址*/
		if (disk_write(fs->drv, fs->win, wsect, 1))
			return FR_DISK_ERR;
		fs->wflag = 0;
		if (wsect - fs->fatbase < fs->fsize) {		/* Is it in the FAT area? */
			for (nf = fs->n_fats; nf >= 2; nf--) {	/* Reflect the change to all FAT copies 写入到其他fat*/
				wsect += fs->fsize;
				disk_write(fs->drv, fs->win, wsect, 1);
			}
		}
	}
	return FR_OK;
}
#endif

/*缓存扇区 至内存*/
static
FRESULT move_window (
	FATFS* fs,		/* File system object */
	DWORD sector	/* Sector number to make appearance in the fs->win[] 缓存扇区编号*/
)
{
	if (sector != fs->winsect) {	/* Changed current window 改变当前读写窗口*/
#if !_FS_READONLY
		if (sync_window(fs) != FR_OK)
			return FR_DISK_ERR;
#endif
		if (disk_read(fs->drv, fs->win, sector, 1))
			return FR_DISK_ERR;
		fs->winsect = sector;
	}

	return FR_OK;
}

/*-----------------------------------------------------------------------*/
/* Synchronize file system and strage device  同步文件信息                           */
/*-----------------------------------------------------------------------*/
#if !_FS_READONLY
static
FRESULT sync_fs (	/* FR_OK: successful, FR_DISK_ERR: failed */
	FATFS* fs		/* File system object */
)
{
	FRESULT res;


	res = sync_window(fs);
	if (res == FR_OK) {
		/* Update FSINFO sector if needed */
		if (fs->fs_type == FS_FAT32 && fs->fsi_flag == 1) {
			/* Create FSINFO structure */
			mem_set(fs->win, 0, SS(fs));
			ST_WORD(fs->win+BS_55AA, 0xAA55);
			ST_DWORD(fs->win+FSI_LeadSig, 0x41615252);
			ST_DWORD(fs->win+FSI_StrucSig, 0x61417272);
			ST_DWORD(fs->win+FSI_Free_Count, fs->free_clust);
			ST_DWORD(fs->win+FSI_Nxt_Free, fs->last_clust);
			/* Write it into the FSINFO sector */
			fs->winsect = fs->volbase + 1;
			disk_write(fs->drv, fs->win, fs->winsect, 1);
			fs->fsi_flag = 0;
		}
	}

	return res;
}
#endif


/*从簇得扇区*/
DWORD clust2sect (	/* !=0: Sector number, 0: Failed - invalid cluster# */
	FATFS* fs,		/* File system object */
	DWORD clst		/* Cluster# to be converted */
)
{
	clst -= 2;
	if (clst >= (fs->n_fatent - 2)) return 0;		/* Invalid cluster# */
	return clst * fs->csize + fs->database;
}

/*簇读取fat中入口并返回默认为fat16*/
DWORD get_fat (	/* 0xFFFFFFFF:Disk error, 1:Internal error, Else:Cluster status */
	FATFS* fs,	/* File system object */
	DWORD clst	/* Cluster# to get the link information */
)
{
	UINT wc, bc;
	BYTE *p;


	if (clst < 2 || clst >= fs->n_fatent)	/* Check range */
		return 1;
		if (move_window(fs, fs->fatbase + (clst / (SS(fs) / 2)))) break;/*16位的fat所以除2*/
		p = &fs->win[clst * 2 % SS(fs)];
		return LD_WORD(p);
}
FRESULT put_fat (
	FATFS* fs,	/* File system object */
	DWORD clst,	/* Cluster# to be changed in range of 2 to fs->n_fatent - 1 */
	DWORD val	/* New value to mark the cluster */
)
{
	UINT bc;
	BYTE *p;
	FRESULT res;


	if (clst < 2 || clst >= fs->n_fatent) {	/* Check range */
		res = FR_INT_ERR;

	} else {
			res = move_window(fs, fs->fatbase + (clst / (SS(fs) / 2)));
			if (res != FR_OK) break;
			p = &fs->win[clst * 2 % SS(fs)];
			ST_WORD(p, (WORD)val);/*16位小头存储*/
			break;

		fs->wflag = 1;/*标记缓存未写入状态*/
	}

	return res;
}

static
FRESULT remove_chain (
	FATFS* fs,			/* File system object */
	DWORD clst			/* Cluster# to remove a chain from */
)
{
	FRESULT res;
	DWORD nxt;


	if (clst < 2 || clst >= fs->n_fatent) {	/* Check range */
		res = FR_INT_ERR;

	} else {
		res = FR_OK;
		while (clst < fs->n_fatent) {			/* 判断是否超出范围*/
			nxt = get_fat(fs, clst);			/* Get cluster status */
			if (nxt == 0) break;				/* Empty cluster? 簇为空*/
			if (nxt == 1) { res = FR_INT_ERR; break; }	/* Internal error? 坏*/
			if (nxt == 0xFFFFFFFF) { res = FR_DISK_ERR; break; }	/* Disk error? */
			res = put_fat(fs, clst, 0);			/* 标记当前簇为空 */
			if (res != FR_OK) break;
			if (fs->free_clust != 0xFFFFFFFF) {	/* 更新文件系统 增加空扇区计数*/
				fs->free_clust++;
				fs->fsi_flag |= 1;/*标记等待写入*/
			}

			clst = nxt;	/* Next cluster */
		}
	}

	return res;
}

static
DWORD create_chain (	/* 0:No free cluster, 1:Internal error, 0xFFFFFFFF:Disk error, >=2:New cluster# */
	FATFS* fs,			/* File system object */
	DWORD clst			/* Cluster# to stretch. 0 means create a new chain. */
)
{
	DWORD cs, ncl, scl;
	FRESULT res;


	if (clst == 0) {		/* 创建新链 */
		scl = fs->last_clust;			/* 从最前的从未分配空簇开始 */
		if (!scl || scl >= fs->n_fatent) scl = 1;/*所有簇都已经分配过*/
	}
	else {					/* 连接 */
		cs = get_fat(fs, clst);			/* Check the cluster status *检查开始簇是否空/
		if (cs < 2) return 1;			/* Invalid value */
		if (cs == 0xFFFFFFFF) return cs;	/* A disk error occurred */
		if (cs < fs->n_fatent) return cs;	/* It is already followed by next cluster 已经占用返回下一个*/
		scl = clst;
	}

	ncl = scl;				/* Start cluster */
	/*循环直到找到下一个空簇*/
	for (;;) {
		ncl++;							/* Next cluster */
		if (ncl >= fs->n_fatent) {		/* 回到开头 */
			ncl = 2;
			if (ncl > scl) return 0;	/* No free cluster */
		}
		cs = get_fat(fs, ncl);			/* Get the cluster status */
		if (cs == 0) break;				/* Found a free cluster 找到空簇*/
		if (cs == 0xFFFFFFFF || cs == 1)/* An error occurred */
			return cs;
		if (ncl == scl) return 0;		/* No free cluster */
	}

	res = put_fat(fs, ncl, 0x0FFFFFFF);	/* 标记为文件结束 */
	if (res == FR_OK && clst != 0) {
		res = put_fat(fs, clst, ncl);	/* 与上一个连接 */
	}
	if (res == FR_OK) {
		fs->last_clust = ncl;			/* 更新文件系统 */
		if (fs->free_clust != 0xFFFFFFFF) {
			fs->free_clust--;
			fs->fsi_flag |= 1;
		}
	} else {
		ncl = (res == FR_DISK_ERR) ? 0xFFFFFFFF : 1;
	}

	return ncl;		/* 返回新簇*/
}


void st_clust (/*写入目录记录*/
	BYTE* dir,	/* Pointer to the directory entry */
	DWORD cl	/* Value to be set */
)
{
	ST_WORD(dir+DIR_FstClusLO, cl);
	ST_WORD(dir+DIR_FstClusHI, cl >> 16);/*用于fat32*/
}


static
void get_fileinfo (		/* No return code */
	DIR* dp,			/* Pointer to the directory object */
	FILINFO* fno	 	/* Pointer to the file information to be filled */
)
{
	UINT i;
	TCHAR *p, c;


	p = fno->fname;
	if (dp->sect) {		/* Get SFN 获取名称*/
		BYTE *dir = dp->dir;/*目录索引目标名称*/
		i = 0;
		while (i < 11) {		/* Copy name body and extension 文件名和扩展名*/
			c = (TCHAR)dir[i++];
			if (c == ' ') continue;			/* Skip padding spaces */
			if (c == NDDE) c = (TCHAR)DDE;	/* Restore replaced DDE character */
			if (i == 9) *p++ = '.';			/* Insert a . if extension is exist 扩展名*/
			*p++ = c;
		}
		fno->fattrib = dir[DIR_Attr];				/* Attribute 读取属性*/
		fno->fsize = LD_DWORD(dir+DIR_FileSize);	/* Size 大小*/
		fno->fdate = LD_WORD(dir+DIR_WrtDate);		/* Date */
		fno->ftime = LD_WORD(dir+DIR_WrtTime);		/* Time */
	}
	*p = 0;		/* Terminate SFN string by a \0 文件名结束*/
}


/*-----------------------------------------------------------------------*/
/* Find logical drive and check if the volume is mounted   重要挂载函数
读取分区信息写入内存结构       */
/*-----------------------------------------------------------------------*/

static
FRESULT find_volume (	/* FR_OK(0): successful, !=0: any error occurred */
	FATFS** rfs,		/* Pointer to pointer to the found file system object */
	const TCHAR** path,	/* Pointer to pointer to the path name (drive number) */
	BYTE wmode			/* !=0: Check write protection for write access */
)
{
	BYTE fmt;
	int vol;
	DSTATUS stat;
	DWORD bsect, fasize, tsect, sysect, nclst, szbfat;
	WORD nrsv;
	FATFS *fs;


	/* Get logical drive number from the path name */
	*rfs = 0;
	vol = 0;

	/* Check if the file system object is valid or not */
	fs = FatFs[vol];					/* Get pointer to the file system object */
	if (!fs) return FR_NOT_ENABLED;		/* Is the file system object available? */

	*rfs = fs;							/* Return pointer to the file system object */


	/* The file system object is not valid. */
	/* Following code attempts to mount the volume. (analyze BPB and initialize the fs object) */

	fs->fs_type = 0;					/* Clear the file system object */
	fs->drv = LD2PD(vol);				/* Bind the logical drive and a physical drive */
	stat = disk_initialize(fs->drv);	/* Initialize the physical drive */
	if (stat & STA_NOINIT)				/* Check if the initialization succeeded */
		return FR_NOT_READY;			/* Failed to initialize due to no medium or hard error */
	if (!_FS_READONLY && wmode && (stat & STA_PROTECT))	/* Check disk write protection if needed */
		return FR_WRITE_PROTECTED;

	/* Find an FAT partition on the drive. Supports only generic partitioning, FDISK and SFD. */
	bsect = 0;
	fmt = check_fs(fs, bsect);					/* Load sector 0 and check if it is an FAT boot sector as SFD */
	if (fmt == 1 || (!fmt && (LD2PT(vol)))) {	/* Not an FAT boot sector or forced partition number */
		UINT i;
		DWORD br[4];

		for (i = 0; i < 4; i++) {			/* Get partition offset */
			BYTE *pt = fs->win+MBR_Table + i * SZ_PTE;
			br[i] = pt[4] ? LD_DWORD(&pt[8]) : 0;
		}
		i = LD2PT(vol);						/* Partition number: 0:auto, 1-4:forced */
		if (i) i--;
		do {								/* Find an FAT volume */
			bsect = br[i];
			fmt = bsect ? check_fs(fs, bsect) : 2;	/* Check the partition */
		} while (!LD2PT(vol) && fmt && ++i < 4);
	}
	if (fmt == 3) return FR_DISK_ERR;		/* An error occured in the disk I/O layer */
	if (fmt) return FR_NO_FILESYSTEM;		/* No FAT volume is found */

	/* An FAT volume is found. Following code initializes the file system object */

	if (LD_WORD(fs->win+BPB_BytsPerSec) != SS(fs))		/* (BPB_BytsPerSec must be equal to the physical sector size) */
		return FR_NO_FILESYSTEM;

	fasize = LD_WORD(fs->win+BPB_FATSz16);				/* Number of sectors per FAT */
	if (!fasize) fasize = LD_DWORD(fs->win+BPB_FATSz32);
	fs->fsize = fasize;

	fs->n_fats = fs->win[BPB_NumFATs];					/* Number of FAT copies */
	if (fs->n_fats != 1 && fs->n_fats != 2)				/* (Must be 1 or 2) */
		return FR_NO_FILESYSTEM;
	fasize *= fs->n_fats;								/* Number of sectors for FAT area */

	fs->csize = fs->win[BPB_SecPerClus];				/* Number of sectors per cluster */
	if (!fs->csize || (fs->csize & (fs->csize - 1)))	/* (Must be power of 2) */
		return FR_NO_FILESYSTEM;

	fs->n_rootdir = LD_WORD(fs->win+BPB_RootEntCnt);	/* Number of root directory entries */
	if (fs->n_rootdir % (SS(fs) / SZ_DIR))				/* (Must be sector aligned) */
		return FR_NO_FILESYSTEM;

	tsect = LD_WORD(fs->win+BPB_TotSec16);				/* Number of sectors on the volume */
	if (!tsect) tsect = LD_DWORD(fs->win+BPB_TotSec32);

	nrsv = LD_WORD(fs->win+BPB_RsvdSecCnt);				/* Number of reserved sectors */
	if (!nrsv) return FR_NO_FILESYSTEM;					/* (Must not be 0) */

	/* Determine the FAT sub type */
	sysect = nrsv + fasize + fs->n_rootdir / (SS(fs) / SZ_DIR);	/* RSV+FAT+DIR */
	if (tsect < sysect) return FR_NO_FILESYSTEM;		/* (Invalid volume size) */
	nclst = (tsect - sysect) / fs->csize;				/* Number of clusters */
	if (!nclst) return FR_NO_FILESYSTEM;				/* (Invalid volume size) */
	fmt = FS_FAT12;
	if (nclst >= MIN_FAT16) fmt = FS_FAT16;
	if (nclst >= MIN_FAT32) fmt = FS_FAT32;

	/* Boundaries and Limits */
	fs->n_fatent = nclst + 2;							/* Number of FAT entries */
	fs->volbase = bsect;								/* Volume start sector */
	fs->fatbase = bsect + nrsv; 						/* FAT start sector */
	fs->database = bsect + sysect;						/* Data start sector */
	if (fmt == FS_FAT32) {
		if (fs->n_rootdir) return FR_NO_FILESYSTEM;		/* (BPB_RootEntCnt must be 0) */
		fs->dirbase = LD_DWORD(fs->win+BPB_RootClus);	/* Root directory start cluster */
		szbfat = fs->n_fatent * 4;						/* (Needed FAT size) */
	} else {
		if (!fs->n_rootdir)	return FR_NO_FILESYSTEM;	/* (BPB_RootEntCnt must not be 0) */
		fs->dirbase = fs->fatbase + fasize;				/* Root directory start sector */
		szbfat = (fmt == FS_FAT16) ?					/* (Needed FAT size) */
			fs->n_fatent * 2 : fs->n_fatent * 3 / 2 + (fs->n_fatent & 1);
	}
	if (fs->fsize < (szbfat + (SS(fs) - 1)) / SS(fs))	/* (BPB_FATSz must not be less than needed) */
		return FR_NO_FILESYSTEM;

	/* Initialize cluster allocation information */
	fs->last_clust = fs->free_clust = 0xFFFFFFFF;

	/* Get fsinfo if available */
	fs->fsi_flag = 0x80;
	if (fmt == FS_FAT32				/* Enable FSINFO only if FAT32 and BPB_FSInfo is 1 */
		&& LD_WORD(fs->win+BPB_FSInfo) == 1
		&& move_window(fs, bsect + 1) == FR_OK)
	{
		fs->fsi_flag = 0;
		if (LD_WORD(fs->win+BS_55AA) == 0xAA55	/* Load FSINFO data if available */
			&& LD_DWORD(fs->win+FSI_LeadSig) == 0x41615252
			&& LD_DWORD(fs->win+FSI_StrucSig) == 0x61417272)
		{
			fs->free_clust = LD_DWORD(fs->win+FSI_Free_Count);
			fs->last_clust = LD_DWORD(fs->win+FSI_Nxt_Free);
		}
	}
	fs->fs_type = fmt;	/* FAT sub-type */
	fs->id = ++Fsid;	/* File system mount ID */



	return FR_OK;
}