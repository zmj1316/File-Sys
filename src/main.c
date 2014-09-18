#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <locale.h>
#include "diskio.h"
#include "ff.h"

const TCHAR HelpStr[] =
"[´ÅÅÌ²Ù×÷]\n" 
" di <pd#> - ³õÊ¼»¯\n"
" dd [<pd#> <sect>] - Dump ÉÈÇø\n"
"[ÎÄ¼þÏµÍ³²Ù×÷]\n"
"fi[ld#] ³õÊ¼»¯\n"
"fl [<path>] - ²é¿´Ä¿Â¼ \n"
"fc[Filename] - ´´½¨ÎÄ¼þ \n"
"fr[Filename] - É¾³ýÎÄ¼þ\n"
"fm[dirname]  - ´´½¨Ä¿Â¼\n"
"ll[<path>] ²é¿´ÍêÕûÄ¿Â¼\n"
"cd[<path>] ¸Ä±äµ±Ç°Ä¿Â¼ \n"
"xi[FileName] µ±Ç°Ä¿Â¼Ð´ÈëÎÄ¼þ\n"
"xo[FileName] µ±Ç°Ä¿Â¼¶ÁÈ¡ÎÄ¼þ \n"

;
const TCHAR Draw[] =


" _   _    _____  __    __       ______  _   _       ___   __   _   _____ \n" 
"| | / /  | ____| \\ \\  / /      |___  / | | | |     /   | |  \\ | | /  ___| \n"
"| |/ /   | |__    \\ \\/ /          / /  | |_| |    / /| | |   \\| | | |     \n"
"| |\\ \\   |  __|    \\  /          / /   |  _  |   / / | | | |\\   | | |  _  \n"
"| | \\ \\  | |___    / /          / /__  | | | |  / /  | | | | \\  | | |_| | \n"
"|_|  \\_\\ |_____|  /_/          /_____| |_| |_| /_/   |_| |_|  \\_| \\_____/\n" 
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

BYTE Buff[262144];          /* Ã§Â¼â€œÃ¥Â­Ëœ */

/*----------------------------------------------*/
/* Get a value of the string       */
/*----------------------------------------------*/
/*  "123 -5   0x3ff 0b1111 0377  w "
        ^                           1st call returns 123 and next ptr
           ^                        2nd call returns -5 and next ptr
                   ^                3rd call returns 1023 and next ptr
                          ^         4th call returns 15 and next ptr
                               ^    5th call returns 255 and next ptr
                                  ^ 6th call fails and returns 0
*/





DWORD getsize(char * fn){
    DWORD count=0;
    FILE *fp;
    fp=fopen(fn,"rb+");
    while(!feof(fp)){
        fgetc(fp);
        count++;
    }
    fclose(fp);
    printf("%d",count);
    return count;
}                                  

void xcopy(DIR * dp,char * ptr){/**/
    BYTE buff[512];
    BYTE name[20];
    DWORD ofs=dp->fs->dirbase;
    WORD fsect,dsect;
    DWORD size,remain;
    FILE *tar;
    BYTE idx=0,ofsect=0;
    char * fn;
    int i=0,count =0;
    char a[50]="data\\";
    if(dp->sclust)
        disk_read(0,buff,clust2sect(dp->fs,dp->sclust),1);
    else
        disk_read(0,buff,dp->fs->dirbase,1);
    for(count=0;count<8&&*ptr;count++){
        if (*ptr>47&&*ptr<58||*ptr>=65&&*ptr<=90||*ptr>=97&&*ptr<=122)
            if(*ptr>=97&&*ptr<=122)
                name[count]=*ptr-32;
            else
                name[count]=*ptr;
        else
            if (*(ptr++)=='.')
                break;
        ptr++;
    }
    for(;count<8;count++){
        name[count]=' ';
    }
    for(count=0;count<3&&*ptr;count++){
        if (*ptr>47&&*ptr<58||*ptr>=65&&*ptr<=90||*ptr>=97&&*ptr<=122)
            if(*ptr>=97&&*ptr<=122)
                name[count+8]=*ptr-32;
            else
                name[count+8]=*ptr;
        ptr++;
    }
    for(;count<3;count++){
        name[count+8]=' ';
    }
    while(1){
             
        if (!memcmp(buff+32*idx,name,11)) {
            printf("File  Found!\n");
            break;
        }
 
        else idx++;
    }
    fsect=LD_WORD(buff+26+32*idx);
    remain=size=LD_DWORD(buff+28+32*idx);
    fn=buff+32*idx;
    count=0;
    for(i=0;i<8;i++){
        if (*(fn+i)!=' ')
            name[count++]=*(fn+i);
    }
    name[count++]='.';
    for(;i<11;i++){
        if (*(fn+i)!=' ')
            name[count++]=*(fn+i);
    }
    name[count]=0;
    printf("start sector: %lx\n", fsect);
    getch();
    strcat(a,name);
    tar=fopen(a,"wb+");
    dsect=fsect;
    while(remain>512){
        disk_read(0,buff,FatFs[0].database+dsect-2,1);
        /*printf("%lx\n", dsect);*/
        fseek(tar,size-remain,0);
        /*printf("%lx\n",size-remain );*/
        fwrite(buff,512,1,tar);
        dsect=get_fat(&FatFs[0],dsect);
        remain-=512;
        /*printf("%s", buff);*/
    }
    disk_read(0,buff,FatFs[0].database+dsect-2,1);
    fseek(tar,size-remain,0);
    /*printf("%lx\n",size-remain );*/
    fwrite(buff,remain,1,tar);
    /*printf("%s", buff);*/
    fclose(tar);

}
void xprint(DIR * dp,char * ptr){/**/
    BYTE buff[512];
    BYTE name[20];
    DWORD ofs=dp->fs->dirbase;
    WORD fsect,dsect;
    DWORD size,remain;
    FILE *tar;
    BYTE idx=0,ofsect=0;
    char * fn;
    int i=0,count =0;
    if(dp->sclust)
        disk_read(0,buff,clust2sect(dp->fs,dp->sclust),1);
    else
        disk_read(0,buff,dp->fs->dirbase,1);
    for(count=0;count<8&&*ptr;count++){
        if (*ptr>47&&*ptr<58||*ptr>=65&&*ptr<=90||*ptr>=97&&*ptr<=122)
            if(*ptr>=97&&*ptr<=122)
                name[count]=*ptr-32;
            else
                name[count]=*ptr;
        else
            if (*(ptr++)=='.')
                break;
        ptr++;
    }
    for(;count<8;count++){
        name[count]=' ';
    }
    for(count=0;count<3&&*ptr;count++){
        if (*ptr>47&&*ptr<58||*ptr>=65&&*ptr<=90||*ptr>=97&&*ptr<=122)
            if(*ptr>=97&&*ptr<=122)
                name[count+8]=*ptr-32;
            else
                name[count+8]=*ptr;
        ptr++;
    }
    for(;count<3;count++){
        name[count+8]=' ';
    }
    while(1){
             
        if (!memcmp(buff+32*idx,name,11)) {
            printf("File  Found!\n");
            break;
        }
 
        else idx++;
    }
    fsect=LD_WORD(buff+26+32*idx);
    remain=size=LD_DWORD(buff+28+32*idx);
    fn=buff+32*idx;
    count=0;
    for(i=0;i<8;i++){
        if (*(fn+i)!=' ')
            name[count++]=*(fn+i);
    }
    name[count++]='.';
    for(;i<11;i++){
        if (*(fn+i)!=' ')
            name[count++]=*(fn+i);
    }
    name[count]=0;
    printf("start sector: %lx\n", fsect);
    getch();
    dsect=fsect;
    while(remain>512){
        disk_read(0,buff,FatFs[0].database+dsect-2,1);
        /*printf("%lx\n", dsect);*/
        dsect=get_fat(&FatFs[0],dsect);
        remain-=512;
        printf("%s", buff);
    }
    disk_read(0,buff,FatFs[0].database+dsect-2,1);
    /*printf("%lx\n",size-remain );*/
    printf("%s", buff);

}
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
    FIL f1, f2;      /*  */
    FRESULT res;         /* */
    char  cmd[100];
    int len;
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
    system("color f1");  
    system("diskmgmt.msc");
    printf("%s\n", Draw);
    printf("»¶Ó­Ê¹ÓÃ !\n ÍË³ö:'q' \n °ïÖú:'?'\nÇëÊ×ÏÈÓÃ'fi'¼ÓÔØ0ºÅÅÌ\n");
    f_opendir(&dir,"/");
    char c[126]="mspaint .\\data\\";
    char d[126]="mspaint .\\data\\";
    char t[126]="notepad .\\data\\";
    char tt[126]="notepad .\\data\\";
    while(1){
        ptr=cmd;
        printf(">");
        len=gets(ptr);
        fflush(stdin); 
        switch (*ptr++){
            case 'q': return 0; break;
            case '?': printf("%s\n",HelpStr ); break;
            case 'd':  /* Disk I/O command */
                switch (*ptr++) {   /* */
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

                    
                case 'i' :  /* di <pd#> -*/
                    if (!xatoi(&ptr, &p1)) break;
                    res = disk_initialize((BYTE)p1);
                    printf("rc=%d\n", res);
                    if (disk_ioctl((BYTE)p1, GET_SECTOR_SIZE, &w) == RES_OK)
                        printf("Sector size = %u\n", w);
                    if (disk_ioctl((BYTE)p1, GET_SECTOR_COUNT, &dw) == RES_OK)
                        printf("Number of sectors = %u\n", dw);
                break;
                case 's':
                    
                    printf("$DATABASE=0x%x\n",FatFs[0].database);
                    printf("$DIRBASE=0x%x\n",FatFs[0].dirbase);
                break;
                }
            break;

            case 'f' :  /* FatFs test command */
                switch (*ptr++) {   /* Branch by secondary command character */
                case 'i':/*fi <ld#> */
                    if (!xatoi(&ptr, &p1) || (UINT)p1 > 9) break;
                    if (!xatoi(&ptr, &p2)) p2 = 0;
                    sprintf(ptr, "%d:", p1);
                    res=f_mount(&FatFs[p1], ptr, (BYTE)p2);
                    f_opendir(&dir,"/");
                break;
                case 'l':
                    f_readdir(&dir,0);
                    while(1){
                        
                        f_readdir(&dir,&fno);
                        if (dir.sect){
                            if(!(fno.fattrib&AM_HID))
                            if (fno.fattrib==16)
                                printf("%s\\\n", fno.fname);
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
                case 'm':
                    f_mkdir(ptr);
                break;
            
                }
            break;
            case 'l':
                switch (*ptr++){
                case 'l':
                    f_readdir(&dir,0);
                    printf("ÎÄ¼þÃû      Ä¿Â¼ÉÈÇø    ´óÐ¡     ÈÕÆÚ\n"); 
                    while(1){
                        
                        f_readdir(&dir,&fno);
                        if (dir.sect){
                            if (fno.fattrib==16)
                                printf("%s\\  ", fno.fname);
                            else
                                printf("%s ", fno.fname);
                            printf("        %d       %d       %d %d %d\n",dir.sclust?dir.sclust-2+FatFs[0].database:FatFs[0].dirbase ,fno.fsize,(fno.fdate>>9)+1980,fno.fdate>>5&15,fno.fdate&31);
                        }
                        else
                            break;
                    }
                break;
                
                    
                }
            break;
            case 't': 
                /*xprint(0);*/
                /*xput(&FatFs[0],"ff.c");*/
                mkdir(&FatFs[0],"ff");
            break;
            case 'c':
                switch (*ptr++){
                    case 'd': 
                        if(res=f_opendir(&dir,ptr)) {put_rc(res);break;}
                        printf("ËùÔÚ´Ø:0x%x\n",dir.sclust);
                        printf("ÎÄ¼þÃû      Ä¿Â¼ÉÈÇø    ´óÐ¡ \n"); 
                        while(1){
                            f_readdir(&dir,&fno);
                            if (dir.sect){
                                if (fno.fattrib==16)
                                    printf("%s\\ ", fno.fname);
                                else
                                    printf("%s ", fno.fname);
                                printf("       %d       %d \n",dir.sclust?dir.sclust-2+FatFs[0].database:FatFs[0].dirbase ,fno.fsize);
                                }
                            else
                                break;
                        }
                    break;
                }
            break;
            case 'x':
                switch(*ptr++){
                    case 'i': xput(&dir,ptr); break;
                    case 'o': xcopy(&dir,ptr);break;
                    case 'p': xcopy(&dir,ptr); strcpy(c,d);strcat(c,ptr); system(c); break;
                    case 't': xcopy(&dir,ptr); strcpy(t,tt);strcat(t,ptr); system(t); break;
                }
            break;
            case 'p':   xprint(&dir,ptr); break;

        }
    }
    /*  */
    
    printf("%d\n",f_mount(&fs[0],"",1));
    printf("%d\n",f_mkfs("",0,0) );
    getch();
    printf("%d\n",f_mkdir("test"));
    printf("%d\n",f_mkdir("test\\test1"));
    f_open(&f1, "message.txt", FA_CREATE_NEW);
    printf("%d\n",f_mkdir("test2"));
    return;

}
