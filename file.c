
#include "file.h"			/* Declarations of FatFs API */
#include "diskio.h"		/* Declarations of disk I/O functions */

FATFS *FatFs
#define	SS(fs)	((UINT)_MAX_SS)

FRESULT f_mkfs (
	const TCHAR* path,	/* Logical drive number */
	BYTE sfd,			/* Partitioning rule 0:FDISK, 1:SFD */
	UINT au				/* Allocation unit [bytes] */
)
{
	static const WORD vst[] = { 1024,   512,  256,  128,   64,    32,   16,    8,    4,    2,   0};
	static const WORD cst[] = {32768, 16384, 8192, 4096, 2048, 16384, 8192, 4096, 2048, 1024, 512};
	int vol;
	BYTE fmt, md, sys, *tbl, pdrv, part;
	DWORD n_clst, vs, n, wsect;
	UINT i;
	DWORD b_vol, b_fat, b_dir, b_data;	/* LBA */
	DWORD n_vol, n_rsv, n_fat, n_dir;	/* Size */
	FATFS *fs;
	DSTATUS stat;


	fs = FatFs;
	fs->fs_type = 0;
	pdrv = 0;	/* Physical drive */
	part = 0;	/* Partition (0:auto detect, 1-4:get from partition table)*/

	/* Get disk statics */
	stat = disk_initialize(pdrv);
		/* Create a partition in this function */
	if (disk_ioctl(pdrv, GET_SECTOR_COUNT, &n_vol) != RES_OK || n_vol < 128)
		return FR_DISK_ERR;
	b_vol = (sfd) ? 0 : 63;		/* Volume start sector */
	n_vol -= b_vol;				/* Volume size */
	

	if (!au) {				/* AU auto selection */
		vs = n_vol / (2000 / 512 / 512));
		for (i = 0; vs < vst[i]; i++) ;
		au = cst[i];
	}
	au /= 512;		/* Number of sectors per cluster */
	if (au == 0) au = 1;
	if (au > 128) au = 128;

	/* Pre-compute number of clusters and FAT sub-type */
	n_clst = n_vol / au;
	fmt = FS_FAT16;

	/* Determine offset and size of FAT structure */
	if (fmt == FS_FAT32) {
		n_fat = ((n_clst * 4) + 8 + SS(fs) - 1) / SS(fs);
		n_rsv = 32;
		n_dir = 0;
	} else {
		n_fat = (fmt == FS_FAT12) ? (n_clst * 3 + 1) / 2 + 3 : (n_clst * 2) + 4;
		n_fat = (n_fat + SS(fs) - 1) / SS(fs);
		n_rsv = 1;
		n_dir = (DWORD)N_ROOTDIR * SZ_DIR / SS(fs);
	}
	b_fat = b_vol + n_rsv;				/* FAT area start sector */
	b_dir = b_fat + n_fat * N_FATS;		/* Directory area start sector */
	b_data = b_dir + n_dir;				/* Data area start sector */

	/* Align data start sector to erase block boundary (for flash memory media) */
	if (disk_ioctl(pdrv, GET_BLOCK_SIZE, &n) != RES_OK || !n || n > 32768) n = 1;
	n = (b_data + n - 1) & ~(n - 1);	/* Next nearest erase block from current data start */
	n = (n - b_data) / N_FATS;
	if (fmt == FS_FAT32) {		/* FAT32: Move FAT offset */
		n_rsv += n;
		b_fat += n;
	} else {					/* FAT12/16: Expand FAT size */
		n_fat += n;
	}

	/* Determine number of clusters and final check of validity of the FAT sub-type */
	n_clst = (n_vol - n_rsv - n_fat * N_FATS - n_dir) / au;
	if (   (fmt == FS_FAT16 && n_clst < MIN_FAT16)
		|| (fmt == FS_FAT32 && n_clst < MIN_FAT32))
		return FR_MKFS_ABORTED;

	/* Determine system ID in the partition table */
	if (fmt == FS_FAT32) {
		sys = 0x0C;		/* FAT32X */
	} else {
		if (fmt == FS_FAT12 && n_vol < 0x10000) {
			sys = 0x01;	/* FAT12(<65536) */
		} else {
			sys = (n_vol < 0x10000) ? 0x04 : 0x06;	/* FAT16(<65536) : FAT12/16(>=65536) */
		}
	}

	if (_MULTI_PARTITION && part) {
		/* Update system ID in the partition table */
		tbl = &fs->win[MBR_Table + (part - 1) * SZ_PTE];
		tbl[4] = sys;
		if (disk_write(pdrv, fs->win, 0, 1))	/* Write it to teh MBR */
			return FR_DISK_ERR;
		md = 0xF8;
	} else {
		if (sfd) {	/* No partition table (SFD) */
			md = 0xF0;
		} else {	/* Create partition table (FDISK) */
			mem_set(fs->win, 0, SS(fs));
			tbl = fs->win+MBR_Table;	/* Create partition table for single partition in the drive */
			tbl[1] = 1;						/* Partition start head */
			tbl[2] = 1;						/* Partition start sector */
			tbl[3] = 0;						/* Partition start cylinder */
			tbl[4] = sys;					/* System type */
			tbl[5] = 254;					/* Partition end head */
			n = (b_vol + n_vol) / 63 / 255;
			tbl[6] = (BYTE)(n >> 2 | 63);	/* Partition end sector */
			tbl[7] = (BYTE)n;				/* End cylinder */
			ST_DWORD(tbl+8, 63);			/* Partition start in LBA */
			ST_DWORD(tbl+12, n_vol);		/* Partition size in LBA */
			ST_WORD(fs->win+BS_55AA, 0xAA55);	/* MBR signature */
			if (disk_write(pdrv, fs->win, 0, 1))	/* Write it to the MBR */
				return FR_DISK_ERR;
			md = 0xF8;
		}
	}

	/* Create BPB in the VBR */
	tbl = fs->win;							/* Clear sector */
	mem_set(tbl, 0, SS(fs));
	mem_cpy(tbl, "\xEB\xFE\x90" "MSDOS5.0", 11);/* Boot jump code, OEM name */
	i = SS(fs);								/* Sector size */
	ST_WORD(tbl+BPB_BytsPerSec, i);
	tbl[BPB_SecPerClus] = (BYTE)au;			/* Sectors per cluster */
	ST_WORD(tbl+BPB_RsvdSecCnt, n_rsv);		/* Reserved sectors */
	tbl[BPB_NumFATs] = N_FATS;				/* Number of FATs */
	i = (fmt == FS_FAT32) ? 0 : N_ROOTDIR;	/* Number of root directory entries */
	ST_WORD(tbl+BPB_RootEntCnt, i);
	if (n_vol < 0x10000) {					/* Number of total sectors */
		ST_WORD(tbl+BPB_TotSec16, n_vol);
	} else {
		ST_DWORD(tbl+BPB_TotSec32, n_vol);
	}
	tbl[BPB_Media] = md;					/* Media descriptor */
	ST_WORD(tbl+BPB_SecPerTrk, 63);			/* Number of sectors per track */
	ST_WORD(tbl+BPB_NumHeads, 255);			/* Number of heads */
	ST_DWORD(tbl+BPB_HiddSec, b_vol);		/* Hidden sectors */
	n = get_fattime();						/* Use current time as VSN */
	if (fmt == FS_FAT32) {
		ST_DWORD(tbl+BS_VolID32, n);		/* VSN */
		ST_DWORD(tbl+BPB_FATSz32, n_fat);	/* Number of sectors per FAT */
		ST_DWORD(tbl+BPB_RootClus, 2);		/* Root directory start cluster (2) */
		ST_WORD(tbl+BPB_FSInfo, 1);			/* FSINFO record offset (VBR+1) */
		ST_WORD(tbl+BPB_BkBootSec, 6);		/* Backup boot record offset (VBR+6) */
		tbl[BS_DrvNum32] = 0x80;			/* Drive number */
		tbl[BS_BootSig32] = 0x29;			/* Extended boot signature */
		mem_cpy(tbl+BS_VolLab32, "NO NAME    " "FAT32   ", 19);	/* Volume label, FAT signature */
	} else {
		ST_DWORD(tbl+BS_VolID, n);			/* VSN */
		ST_WORD(tbl+BPB_FATSz16, n_fat);	/* Number of sectors per FAT */
		tbl[BS_DrvNum] = 0x80;				/* Drive number */
		tbl[BS_BootSig] = 0x29;				/* Extended boot signature */
		mem_cpy(tbl+BS_VolLab, "NO NAME    " "FAT     ", 19);	/* Volume label, FAT signature */
	}
	ST_WORD(tbl+BS_55AA, 0xAA55);			/* Signature (Offset is fixed here regardless of sector size) */
	if (disk_write(pdrv, tbl, b_vol, 1))	/* Write it to the VBR sector */
		return FR_DISK_ERR;
	if (fmt == FS_FAT32)					/* Write backup VBR if needed (VBR+6) */
		disk_write(pdrv, tbl, b_vol + 6, 1);

	/* Initialize FAT area */
	wsect = b_fat;
	for (i = 0; i < N_FATS; i++) {		/* Initialize each FAT copy */
		mem_set(tbl, 0, SS(fs));			/* 1st sector of the FAT  */
		n = md;								/* Media descriptor byte */
		if (fmt != FS_FAT32) {
			n |= (fmt == FS_FAT12) ? 0x00FFFF00 : 0xFFFFFF00;
			ST_DWORD(tbl+0, n);				/* Reserve cluster #0-1 (FAT12/16) */
		} else {
			n |= 0xFFFFFF00;
			ST_DWORD(tbl+0, n);				/* Reserve cluster #0-1 (FAT32) */
			ST_DWORD(tbl+4, 0xFFFFFFFF);
			ST_DWORD(tbl+8, 0x0FFFFFFF);	/* Reserve cluster #2 for root directory */
		}
		if (disk_write(pdrv, tbl, wsect++, 1))
			return FR_DISK_ERR;
		mem_set(tbl, 0, SS(fs));			/* Fill following FAT entries with zero */
		for (n = 1; n < n_fat; n++) {		/* This loop may take a time on FAT32 volume due to many single sector writes */
			if (disk_write(pdrv, tbl, wsect++, 1))
				return FR_DISK_ERR;
		}
	}
	/* Initialize root directory */
	i = (fmt == FS_FAT32) ? au : (UINT)n_dir;
	do {
		if (disk_write(pdrv, tbl, wsect++, 1))
			return FR_DISK_ERR;
	} while (--i);

#if _USE_ERASE	/* Erase data area if needed */
	{
		DWORD eb[2];

		eb[0] = wsect; eb[1] = wsect + (n_clst - ((fmt == FS_FAT32) ? 1 : 0)) * au - 1;
		disk_ioctl(pdrv, CTRL_ERASE_SECTOR, eb);
	}
#endif

	/* Create FSINFO if needed */
	if (fmt == FS_FAT32) {
		ST_DWORD(tbl+FSI_LeadSig, 0x41615252);
		ST_DWORD(tbl+FSI_StrucSig, 0x61417272);
		ST_DWORD(tbl+FSI_Free_Count, n_clst - 1);	/* Number of free clusters */
		ST_DWORD(tbl+FSI_Nxt_Free, 2);				/* Last allocated cluster# */
		ST_WORD(tbl+BS_55AA, 0xAA55);
		disk_write(pdrv, tbl, b_vol + 1, 1);	/* Write original (VBR+1) */
		disk_write(pdrv, tbl, b_vol + 7, 1);	/* Write backup (VBR+7) */
	}
	return 0;
}

