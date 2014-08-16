#include "ff.h"




void main (void)
{
    FATFS fs[2];         /* 逻辑驱动器的工作区(文件系统对象) */
    FIL fsrc, fdst;      /* 文件对象 */
    BYTE buffer[4096];   /* 文件拷贝缓冲区 */
    FRESULT res;         /* FatFs 函数公共结果代码 */
    UINT br, bw;         /* 文件读/写字节计数 */
 
 
    /* 为逻辑驱动器注册工作区 */
    f_mount(0, &fs[0],1);
    f_mount(1, &fs[1],1);
 
    /* 打开驱动器 1 上的源文件 */
    res = f_open(&fsrc, "1:srcfile.dat", FA_OPEN_EXISTING | FA_READ);
 
    /* 在驱动器 0 上创建目标文件 */
    res = f_open(&fdst, "0:dstfile.dat", FA_CREATE_ALWAYS | FA_WRITE);
 
    /* 拷贝源文件到目标文件 */
    for (;;) {
        res = f_read(&fsrc, buffer, sizeof(buffer), &br);
        if (res || br == 0) break;   /* 文件结束错误 */
        res = f_write(&fdst, buffer, br, &bw);
        if (res || bw < br) break;   /* 磁盘满错误 */
    }
 
    /* 关闭打开的文件 */
    f_close(&fsrc);
    f_close(&fdst);
 
    /* 注销工作区(在废弃前) */
    f_mount(0, NULL,1);
    f_mount(1, NULL,1);
}
