#include "ff.h"
const TCHAR HelpStr[] = {
    "[Disk contorls]\n
      di <pd#> - Initialize disk\n
         dd [<pd#> <sect>] - Dump a secrtor\n
         ds <pd#> - Show disk status\n
         dl <file> - Load FAT image into RAM disk (pd#0)\n
    "
}



void main (void)
{
    FATFS fs[2];         /* 逻辑驱动器的工作区(文件系统对象) */
    FIL f1, f2;      /* 文件对象 */
    BYTE buffer[4096];   /* 文件拷贝缓冲区 */
    FRESULT res;         /* FatFs 函数公共结果代码 */
    UINT br, bw;         /* 文件读/写字节计数 */
    DWORD plist[] = {50, 50, 0, 0};
    BYTE work[_MAX_SS];
    char  cmd[100];
    char* ptr;
    ptr=cmd;
    

    printf(">");
    scanf("%s",ptr);
    switch (*ptr++){
        case 'q': return 0;
        case '?': printf("%s\n",HelpStr ); break;
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
