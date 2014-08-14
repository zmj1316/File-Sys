#include "file.h"	


#define	ABORT(fs, res)		{ fp->err = (BYTE)(res); LEAVE_FF(fs, res); } //错误码
/* Definitions of sector size 定义扇区大小默认512*/
#if (_MAX_SS < _MIN_SS) || (_MAX_SS != 512 && _MAX_SS != 1024 && _MAX_SS != 2048 && _MAX_SS != 4096) || (_MIN_SS != 512 && _MIN_SS != 1024 && _MIN_SS != 2048 && _MIN_SS != 4096)
#error Wrong sector size configuration.
#endif
#if _MAX_SS == _MIN_SS
#define	SS(fs)	((UINT)_MAX_SS)	/* Fixed sector size */
#else
#define	SS(fs)	((fs)->ssize)	/* Variable sector size */
#endif

typedef struct {
	FATFS *fs;		/* Object ID 1, volume (NULL:blank entry) */
	DWORD clu;		/* Object ID 2, directory (0:root) */
	WORD idx;		/* Object ID 3, directory index */
	WORD ctr;		/* Object open counter, 0:none, 0x01..0xFF:read mode open count, 0x100:write mode */
} FILESEM;//文件访问控制

/*------------------------------------------*/
#if _CODE_PAGE == 936	/* Simplified Chinese GBK 应设定为简体中文以下为文件名支持*/
#define _DF1S	0x81
#define _DF1E	0xFE
#define _DS1S	0x40
#define _DS1E	0x7E
#define _DS2S	0x80
#define _DS2E	0xFE

#elif _CODE_PAGE == 1	/* ASCII (for only non-LFN cfg) */
#if _USE_LFN
#error Cannot use LFN feature without valid code page.
#endif
#define _DF1S	0

#else
#error Unknown code page

#endif


/* Character code support macros */
#define IsUpper(c)	(((c)>='A')&&((c)<='Z'))
#define IsLower(c)	(((c)>='a')&&((c)<='z'))
#define IsDigit(c)	(((c)>='0')&&((c)<='9'))

#if _DF1S		/* Code page is DBCS */

#ifdef _DF2S	/* Two 1st byte areas */
#define IsDBCS1(c)	(((BYTE)(c) >= _DF1S && (BYTE)(c) <= _DF1E) || ((BYTE)(c) >= _DF2S && (BYTE)(c) <= _DF2E))
#else			/* One 1st byte area */
#define IsDBCS1(c)	((BYTE)(c) >= _DF1S && (BYTE)(c) <= _DF1E)
#endif

#ifdef _DS3S	/* Three 2nd byte areas */
#define IsDBCS2(c)	(((BYTE)(c) >= _DS1S && (BYTE)(c) <= _DS1E) || ((BYTE)(c) >= _DS2S && (BYTE)(c) <= _DS2E) || ((BYTE)(c) >= _DS3S && (BYTE)(c) <= _DS3E))
#else			/* Two 2nd byte areas */
#define IsDBCS2(c)	(((BYTE)(c) >= _DS1S && (BYTE)(c) <= _DS1E) || ((BYTE)(c) >= _DS2S && (BYTE)(c) <= _DS2E))
#endif

#else			/* Code page is SBCS */

#define IsDBCS1(c)	0
#define IsDBCS2(c)	0

#endif /* _DF1S */

/*----------------------------------------------------------------*/

/* Name status flags */
#define NS			11		/* Index of name status byte in fn[] */
#define NS_LOSS		0x01	/* Out of 8.3 format */
#define NS_LFN		0x02	/* Force to create LFN entry */
#define NS_LAST		0x04	/* Last segment */
#define NS_BODY		0x08	/* Lower case flag (body) */
#define NS_EXT		0x10	/* Lower case flag (ext) */
#define NS_DOT		0x20	/* Dot entry */


/* FAT sub-type boundaries */
#define MIN_FAT16	4086U	/* Minimum number of clusters for FAT16 */
#define	MIN_FAT32	65526U	/* Minimum number of clusters for FAT32 实际不计划支持FAT32*/


#define BS_jmpBoot			0		/* Jump instruction (3) */
#define BS_OEMName			3		/* OEM name (8) */
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
#define	FSI_LeadSig			0		/* FSI: Leading signature (4) */
#define	FSI_StrucSig		484		/* FSI: Structure signature (4) */
#define	FSI_Free_Count		488		/* FSI: Number of free clusters (4) */
#define	FSI_Nxt_Free		492		/* FSI: Last allocated cluster (4) */
#define MBR_Table			446		/* MBR: Partition table offset (2) */
#define	SZ_PTE				16		/* MBR: Size of a partition table entry */
#define BS_55AA				510		/* Signature word (2) */

#define	DIR_Name			0		/* Short file name (11) */
#define	DIR_Attr			11		/* Attribute (1) */
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
#define	LDIR_Ord			0		/* LFN entry order and LLE flag (1) */
#define	LDIR_Attr			11		/* LFN attribute (1) */
#define	LDIR_Type			12		/* LFN type (1) */
#define	LDIR_Chksum			13		/* Sum of corresponding SFN entry */
#define	LDIR_FstClusLO		26		/* Filled by zero (0) */
#define	SZ_DIR				32		/* Size of a directory entry */
#define	LLE					0x40	/* Last long entry flag in LDIR_Ord */
#define	DDE					0xE5	/* Deleted directory entry mark in DIR_Name[0] */
#define	NDDE				0x05	/* Replacement of the character collides with DDE */



#if _VOLUMES >= 1 || _VOLUMES <= 10
static
FATFS *FatFs[_VOLUMES];		/* Pointer to the file system objects (logical drives) */
#else
#error Number of volumes must be 1 to 10.
#endif

static
WORD Fsid;					/* File system mount ID */

#if _FS_RPATH && _VOLUMES >= 2
static
BYTE CurrVol;				/* Current drive */
#endif

#if _FS_LOCK
static
FILESEM	Files[_FS_LOCK];	/* Open object lock semaphores */
#endif

#if _USE_LFN == 0			/* No LFN feature */
#define	DEF_NAMEBUF			BYTE sfn[12]
#define INIT_BUF(dobj)		(dobj).fn = sfn
#define	FREE_BUF()

#elif _USE_LFN == 1			/* LFN feature with static working buffer */
static
WCHAR LfnBuf[_MAX_LFN+1];
#define	DEF_NAMEBUF			BYTE sfn[12]
#define INIT_BUF(dobj)		{ (dobj).fn = sfn; (dobj).lfn = LfnBuf; }
#define	FREE_BUF()

#elif _USE_LFN == 2 		/* LFN feature with dynamic working buffer on the stack */
#define	DEF_NAMEBUF			BYTE sfn[12]; WCHAR lbuf[_MAX_LFN+1]
#define INIT_BUF(dobj)		{ (dobj).fn = sfn; (dobj).lfn = lbuf; }
#define	FREE_BUF()

#elif _USE_LFN == 3 		/* LFN feature with dynamic working buffer on the heap */
#define	DEF_NAMEBUF			BYTE sfn[12]; WCHAR *lfn
#define INIT_BUF(dobj)		{ lfn = ff_memalloc((_MAX_LFN + 1) * 2); \
							  if (!lfn) LEAVE_FF((dobj).fs, FR_NOT_ENOUGH_CORE); \
							  (dobj).lfn = lfn;	(dobj).fn = sfn; }
#define	FREE_BUF()			ff_memfree(lfn)

#else
#error Wrong LFN configuration.
#endif


#ifdef _EXCVT
static
const BYTE ExCvt[] = _EXCVT;	/* Upper conversion table for extended characters */
#endif


/*---------------以下为函数区域*/

/*-----------------------------------------------------------------------*/
/* Get sector# from cluster# 返回扇区位置                                */
/*-----------------------------------------------------------------------*/


DWORD clust2sect (	/* !=0: Sector number, 0: Failed - invalid cluster# */
	FATFS* fs,		/* File system object */
	DWORD clst		/* Cluster# to be converted */
)
{
	clst -= 2;
	if (clst >= (fs->n_fatent - 2)) return 0;		/* Invalid cluster# */
	return clst * fs->csize + fs->database;
}


/*-----------------------------------------------------------------------*/
/* Move/Flush disk access window in the file system object 读盘              */
/*-----------------------------------------------------------------------*/
#if !_FS_READONLY
static
FRESULT sync_window (
	FATFS* fs		/* File system object */
)
{
	DWORD wsect;
	UINT nf;


	if (fs->wflag) {	/* Write back the sector if it is dirty */
		wsect = fs->winsect;	/* Current sector number */
		if (disk_write(fs->drv, fs->win, wsect, 1))
			return FR_DISK_ERR;
		fs->wflag = 0;
		if (wsect - fs->fatbase < fs->fsize) {		/* Is it in the FAT area? */
			for (nf = fs->n_fats; nf >= 2; nf--) {	/* Reflect the change to all FAT copies */
				wsect += fs->fsize;
				disk_write(fs->drv, fs->win, wsect, 1);
			}
		}
	}
	return FR_OK;
}
#endif


static
FRESULT move_window (
	FATFS* fs,		/* File system object */
	DWORD sector	/* Sector number to make appearance in the fs->win[] */
)
{
	if (sector != fs->winsect) {	/* Changed current window */
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
/* FAT access - Read value of a FAT entry                                */
/*-----------------------------------------------------------------------*/


DWORD get_fat (	/* 0xFFFFFFFF:Disk error, 1:Internal error, Else:Cluster status */
	FATFS* fs,	/* File system object */
	DWORD clst	/* Cluster# to get the link information */
)
{
	UINT wc, bc;
	BYTE *p;


	if (clst < 2 || clst >= fs->n_fatent)	/* Check range */
		return 1;

	switch (fs->fs_type) {
	case FS_FAT12 :
		bc = (UINT)clst; bc += bc / 2;
		if (move_window(fs, fs->fatbase + (bc / SS(fs)))) break;
		wc = fs->win[bc % SS(fs)]; bc++;
		if (move_window(fs, fs->fatbase + (bc / SS(fs)))) break;
		wc |= fs->win[bc % SS(fs)] << 8;
		return clst & 1 ? wc >> 4 : (wc & 0xFFF);

	case FS_FAT16 :
		if (move_window(fs, fs->fatbase + (clst / (SS(fs) / 2)))) break;
		p = &fs->win[clst * 2 % SS(fs)];
		return LD_WORD(p);

	case FS_FAT32 :
		if (move_window(fs, fs->fatbase + (clst / (SS(fs) / 4)))) break;
		p = &fs->win[clst * 4 % SS(fs)];
		return LD_DWORD(p) & 0x0FFFFFFF;

	default:
		return 1;
	}

	return 0xFFFFFFFF;	/* An error occurred at the disk I/O layer */
}



/*-----------------------------------------------------------------------*/
/* FAT access - Change value of a FAT entry                              */
/*-----------------------------------------------------------------------*/
#if !_FS_READONLY

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
		switch (fs->fs_type) {
		case FS_FAT12 :
			bc = (UINT)clst; bc += bc / 2;
			res = move_window(fs, fs->fatbase + (bc / SS(fs)));
			if (res != FR_OK) break;
			p = &fs->win[bc % SS(fs)];
			*p = (clst & 1) ? ((*p & 0x0F) | ((BYTE)val << 4)) : (BYTE)val;
			bc++;
			fs->wflag = 1;
			res = move_window(fs, fs->fatbase + (bc / SS(fs)));
			if (res != FR_OK) break;
			p = &fs->win[bc % SS(fs)];
			*p = (clst & 1) ? (BYTE)(val >> 4) : ((*p & 0xF0) | ((BYTE)(val >> 8) & 0x0F));
			break;

		case FS_FAT16 :
			res = move_window(fs, fs->fatbase + (clst / (SS(fs) / 2)));
			if (res != FR_OK) break;
			p = &fs->win[clst * 2 % SS(fs)];
			ST_WORD(p, (WORD)val);
			break;

		case FS_FAT32 :
			res = move_window(fs, fs->fatbase + (clst / (SS(fs) / 4)));
			if (res != FR_OK) break;
			p = &fs->win[clst * 4 % SS(fs)];
			val |= LD_DWORD(p) & 0xF0000000;
			ST_DWORD(p, val);
			break;

		default :
			res = FR_INT_ERR;
		}
		fs->wflag = 1;
	}

	return res;
}
#endif /* !_FS_READONLY */
