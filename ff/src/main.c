#include "ff.h"




void main (void)
{
    FATFS fs[2];         /* 逻辑驱动器的工作区(文件系统对象) */
    FIL fsrc, fdst;      /* 文件对象 */
    BYTE buffer[4096];   /* 文件拷贝缓冲区 */
    FRESULT res;         /* FatFs 函数公共结果代码 */
    UINT br, bw;         /* 文件读/写字节计数 */
    DWORD plist[] = {50, 50, 0, 0};
    BYTE work[_MAX_SS];
    /*printf("%d\n",f_fdisk(0, plist, work));
    /* 为逻辑驱动器注册工作区 */
    printf("%d\n",f_mount(&fs[0],"0",1));

    printf("%d\n",f_mkfs("0",0,0) );
    getch();
    printf("%d\n",f_mkdir("test"));
    

}
