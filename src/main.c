#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <locale.h>
#include "diskio.h"
#include "ff.h"
const TCHAR HelpStr[] =
"[Disk contorls]\n" 
" di <pd#> - Initialize disk\n"
" dd [<pd#> <sect>] - Dump a secrtor\n"
" ds <pd#> - Show disk status\n"
"[Buffer contorls]\n"
"[File system contorls]\n"
" fl [<path>] - Show a directory\n"
;



LONGLONG AccSize;           /* Work register for scan_files() */
WORD AccFiles, AccDirs;
FILINFO Finfo;
#if _USE_LFN
TCHAR LFName[256];
#endif

TCHAR Line[300];            /* Console input/output buffer */
HANDLE hCon, hKey;

FATFS FatFs[_VOLUMES];      /* File system object for logical drive */

#if _USE_FASTSEEK
DWORD SeekTbl[16];          /* Link map table for fast seek feature */
#endif

BYTE Buff[262144];          /* 缓存 */

/*----------------------------------------------*/
/* Get a value of the string   从字符串中取得参数值      */
/*----------------------------------------------*/
/*  "123 -5   0x3ff 0b1111 0377  w "
        ^                           1st call returns 123 and next ptr
           ^                        2nd call returns -5 and next ptr
                   ^                3rd call returns 1023 and next ptr
                          ^         4th call returns 15 and next ptr
                               ^    5th call returns 255 and next ptr
                                  ^ 6th call fails and returns 0
*/

int xatoi (         /* 0:Failed, 1:Successful */
    TCHAR **str,    /* Pointer to pointer to the string */
    long *res       /* Pointer to a valiable to store the value */
)
{
    unsigned long val;
    unsigned char r, s = 0;
    TCHAR c;


    *res = 0;
    while ((c = **str) == ' ') (*str)++;    /* Skip leading spaces */

    if (c == '-') {     /* negative? */
        s = 1;
        c = *(++(*str));
    }

    if (c == '0') {
        c = *(++(*str));
        switch (c) {
        case 'x':       /* hexdecimal */
            r = 16; c = *(++(*str));
            break;
        case 'b':       /* binary */
            r = 2; c = *(++(*str));
            break;
        default:
            if (c <= ' ') return 1; /* single zero */
            if (c < '0' || c > '9') return 0;   /* invalid char */
            r = 8;      /* octal */
        }
    } else {
        if (c < '0' || c > '9') return 0;   /* EOL or invalid char */
        r = 10;         /* decimal */
    }

    val = 0;
    while (c > ' ') {
        if (c >= 'a') c -= 0x20;
        c -= '0';
        if (c >= 17) {
            c -= 7;
            if (c <= 9) return 0;   /* invalid char */
        }
        if (c >= r) return 0;       /* invalid char for current radix */
        val = val * r + c;
        c = *(++(*str));
    }
    if (s) val = 0 - val;           /* apply sign if needed */

    *res = val;
    return 1;
}


/*----------------------------------------------*/
/* Dump a block of byte array                   */

void put_dump (
    const unsigned char* buff,  /* Pointer to the byte array to be dumped */
    unsigned long addr,         /* Heading address value */
    int cnt                     /* Number of bytes to be dumped */
)
{
    int i;


    printf("%08lX:", addr);

    for (i = 0; i < cnt; i++)
        printf(" %02X", buff[i]);

    putchar(' ');
    for (i = 0; i < cnt; i++)
        putchar((TCHAR)((buff[i] >= ' ' && buff[i] <= '~') ? buff[i] : '.'));
    putchar('\n');
}

void put_rc (FRESULT rc)
{
    const TCHAR *p =
        "OK\0DISK_ERR\0INT_ERR\0NOT_READY\0NO_FILE\0NO_PATH\0INVALID_NAME\0"
        "DENIED\0EXIST\0INVALID_OBJECT\0WRITE_PROTECTED\0INVALID_DRIVE\0"
        "NOT_ENABLED\0NO_FILE_SYSTEM\0MKFS_ABORTED\0TIMEOUT\0LOCKED\0"
        "NOT_ENOUGH_CORE\0TOO_MANY_OPEN_FILES\0";
    FRESULT i;

    for (i = 0; i != rc && *p; i++) {
        while(*p++) ;
    }
    printf("rc=%u FR_%s\n", (UINT)rc, p);
}

void main (void)
{
    FIL f1, f2;      /* 文件对象 */
    FRESULT res;         /* FatFs 函数公共结果代码 */
    char  cmd[100];
    TCHAR *ptr, *ptr2, pool[50];
    long p1, p2, p3;
    BYTE *buf;
    UINT s1, s2, cnt;
    WORD w;
    DWORD dw, ofs = 0, sect = 0, drv = 0;
    static const BYTE ft[] = {0, 12, 16, 32};
    FATFS *fs;              /* Pointer to file system object */
    DIR dir;                /* Directory object */
    FIL file[2];            /* File objects */
    FILINFO fno;

    while(1){
        ptr=cmd;
        printf(">");
        scanf("%s",ptr);
        switch (*ptr++){
            case 'q': return 0;
            case '?': printf("%s\n",HelpStr ); break;
            case 'd':  /* Disk I/O command */
                switch (*ptr++) {   /* 第二个指令 */
                case 'd' :  /* dd [<pd#> <sect>] - Dump a secrtor */
                    if (!xatoi(&ptr, &p1)) {
                        p1 = drv; p2 = sect;
                    } else {
                        if (!xatoi(&ptr, &p2)) break;
                    }
                    res = disk_read((BYTE)p1, Buff, p2, 1);
                    if (res) { printf("rc=%d\n", (WORD)res); break; }
                    printf("Drive:%u Sector:%lu\n", p1, p2);
                    if (disk_ioctl((BYTE)p1, GET_SECTOR_SIZE, &w) != RES_OK) break;
                    sect = p2 + 1; drv = p1; 
                    for (buf = Buff, ofs = 0; ofs < w; buf += 16, ofs += 16)
                        put_dump(buf, ofs, 16);
                    break;

                    
                case 'i' :  /* di <pd#> - 磁盘初始化 */
                    if (!xatoi(&ptr, &p1)) break;
                    res = disk_initialize((BYTE)p1);
                    printf("rc=%d\n", res);
                    if (disk_ioctl((BYTE)p1, GET_SECTOR_SIZE, &w) == RES_OK)
                        printf("Sector size = %u\n", w);
                    if (disk_ioctl((BYTE)p1, GET_SECTOR_COUNT, &dw) == RES_OK)
                        printf("Number of sectors = %u\n", dw);
                    break;
                case 's':
                    if (!xatoi(&ptr, &p1)) break;
                    if (disk_ioctl((BYTE)p1, GET_SECTOR_SIZE, &w) == RES_OK)
                        printf("Sector size = %u\n", w);
                    if (disk_ioctl((BYTE)p1, GET_SECTOR_COUNT, &dw) == RES_OK)
                        printf("Number of sectors = %u\n", dw);
                }

            case 'f' :  /* FatFs test command */
                switch (*ptr++) {   /* Branch by secondary command character */
                case 'i':/*挂载fi <ld#> [<mount>]*/
                    if (!xatoi(&ptr, &p1) || (UINT)p1 > 9) break;
                    if (!xatoi(&ptr, &p2)) p2 = 0;
                    sprintf(ptr, "%d:", p1);
                    res=f_mount(&FatFs[p1], ptr, (BYTE)p2);
                    printf("%d\n",res );
                    break;
                case 'l':
                    f_opendir(&dir,"/");
                    while(1){
                        f_readdir(&dir,&fno);
                        if (dir.sect){
                            if (fno.fattrib==16)
                                printf("%s\\ \n", fno.fname);
                            else
                                printf("%s \n", fno.fname);
                        }
                        else
                            break;
                    }
                    break;
                case 'c':

                    f_open(&f1,ptr,FA_CREATE_NEW);
                    f_close(&f1);
                    break;
                case 'r':
                    f_unlink(ptr);
                    break;
                }
            case 'l':
                switch (*ptr++){
                case 'l':
                    f_opendir(&dir,"/");
                    while(1){
                        f_readdir(&dir,&fno);
                        if (dir.sect){
                            if (fno.fattrib==16)
                                printf("%s\\ \n", fno.fname);
                            else
                                printf("%s \n", fno.fname);
                        }
                        else
                            break;
                    }
                    break;
                case 'f':
                    
                }


        }
    }
    /* 为逻辑驱动器注册工作区 */
    printf("%d\n",f_mount(&fs[0],"",1));
    printf("%d\n",f_mkfs("",0,0) );
    getch();
    printf("%d\n",f_mkdir("test"));
    printf("%d\n",f_mkdir("test\\test1"));
    f_open(&f1, "message.txt", FA_CREATE_NEW);
    printf("%d\n",f_mkdir("test2"));
    return;

}
