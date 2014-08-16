/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include <stdio.h>
#include <time.h>
/* Definitions of physical drive number for each media */
#define ATA		0
#define MMC		1
#define USB		2

FILE * fp;
/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat;
	int i;
	char a[256],buf[512];
	sprintf(a,"%d.img",pdrv);
	fp=fopen(a,"rb+");
    for(i = 0;i<20000;i++)
	  {
	   if((i%1000) == 0)
		   printf("#");
	   fwrite(buf, 1,512,fp);
	  
	  }
	 fclose(fp);
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat;
	int result;

	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	UINT count		/* Number of sectors to read (1..128) */
)
{
	DRESULT res;
	char a[255];
	sprintf(a,"%d.img",pdrv);
	fp=fopen(a,"rb+");
	fseek(fp,sector*512,0);
	fread(buff, 1,512,fp);
	fclose(fp);
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
	DRESULT res;
	char a[255];
	sprintf(a,"%d.img",pdrv);
	fp=fopen(a,"rb+");
	fseek(fp,sector*512,0);
	fwrite(buff, 1,512,fp);
	fflush(fp);
	fclose(fp);
	return 0;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	
	return 0;
}
#endif
DWORD get_fattime(){
	int year,month,date,hour,minute,second;
  struct tm *local;
  time_t t;
  t=time(NULL);
  local=localtime(&t);

  year = local->tm_year + 1900;
  month = local->tm_mon+1;
  date = local->tm_mday;
  hour = local->tm_hour;
  minute = local->tm_min;
  second = local->tm_sec;

	return 	  ((DWORD)(year - 1980) << 25)
			| ((DWORD)month << 21)
			| ((DWORD)date << 16)
			| (WORD)(hour << 11)
			| (WORD)(minute << 5)
			| (WORD)(second >> 1);
}
	
                   
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
)
{
	DRESULT res = RES_PARERR;


	if (pdrv >= Drives || (Stat[pdrv].status & STA_NOINIT))
		return RES_NOTRDY;

	switch (ctrl) {
	case CTRL_SYNC:
		res = RES_OK;
		break;

	case GET_SECTOR_COUNT:
		*(DWORD*)buff = 20000;
		res = RES_OK;
		break;

	case GET_SECTOR_SIZE:
		*(WORD*)buff = 512;
		res = RES_OK;
		break;

	case GET_BLOCK_SIZE:
		*(DWORD*)buff = 128;
		res = RES_OK;
		break;

	}

	return res;
}
