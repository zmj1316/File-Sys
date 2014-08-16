/*-----------------------------------------------------------------------*/
/* RAM disk control module for Win32              (C)ChaN, 2014          */
/*-----------------------------------------------------------------------*/

#include <windows.h>
#include "diskio.h"
#include "ff.h"


/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/


SYSTEMTIME SysTime;			/* Time at creation of RAM disk */

extern BYTE *RamDisk;		/* Poiter to the RAM disk (main.c) */
extern DWORD RamDiskSize;	/* Size of RAM disk in unit of sector (main.c) */


/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv		/* Physical drive nmuber */
)
{
	if (pdrv) return STA_NOINIT;

	if (!RamDisk) {
		RamDisk = VirtualAlloc(0, RamDiskSize * 512, MEM_COMMIT, PAGE_READWRITE);
		GetLocalTime(&SysTime);
	}

	return RamDisk ? 0 : STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0) */
)
{
	if (pdrv) return STA_NOINIT;

	return RamDisk ? 0 : STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,			/* Physical drive nmuber (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Number of sectors to read */
)
{
	if (pdrv || !RamDisk) return RES_NOTRDY;
	if (sector >= RamDiskSize) return RES_PARERR;

	CopyMemory(buff, RamDisk + sector * 512, count * 512);

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Number of sectors to write */
)
{
	if (pdrv || !RamDisk) return RES_NOTRDY;
	if (sector >= RamDiskSize) return RES_PARERR;

	CopyMemory(RamDisk + sector * 512, buff, count * 512);

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0) */
	BYTE ctrl,		/* Control code */
	void* buff		/* Buffer to send/receive data block */
)
{
	DRESULT dr;


	dr = RES_ERROR;
	if (!pdrv && RamDisk) {
		switch (ctrl) {
		case CTRL_SYNC:
			dr = RES_OK;
			break;

		case GET_SECTOR_COUNT:
			*(DWORD*)buff = RamDiskSize;
			dr = RES_OK;
			break;

		case GET_BLOCK_SIZE:
			*(DWORD*)buff = 1;
			dr = RES_OK;
			break;
		}
	}
	return dr;
}



/*-----------------------------------------------------------------------*/
/* Get current time                                                      */
/*-----------------------------------------------------------------------*/

DWORD get_fattime(void)
{
	return 	  (DWORD)(SysTime.wYear - 1980) << 25
			| (DWORD)SysTime.wMonth << 21
			| (DWORD)SysTime.wDay << 16
			| (DWORD)SysTime.wHour << 11
			| (DWORD)SysTime.wMinute << 5
			| (DWORD)SysTime.wSecond >> 1;
}
