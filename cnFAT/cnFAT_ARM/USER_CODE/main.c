/****************************************Copyright (c)****************************************************
**                            Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                                 http://www.embedtools.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:           main.c
** Last modified Date:  2009-05-12
** Last Version:        V1.01
** Descriptions:        The main() function example template
**
**--------------------------------------------------------------------------------------------------------
** Created by:          Chengmingji
** Created date:        2009-07-24
** Version:             V1.00
** Descriptions:        添加用户应用程序
**
**--------------------------------------------------------------------------------------------------------
** Modified by:         LiuWeiyun
** Modified date:       2009-07-27
** Version:             V1.01
** Descriptions:        编写USB_HOST_NO_FS例程
**m
** Rechecked by:        WangJianyu
*********************************************************************************************************/
#include "..\config.h"
#include "..\fat\types.h"
#include "..\fat\fat.h"

/*********************************************************************************************************
  变量与宏定义
*********************************************************************************************************/
#define BEEP        (1 << 11)                                           /* 蜂鸣器                       */
#define KEY1        (1 << 10)                                           /* P2.10连接KEY1                */
#define BEEPOFF()   FIO0DIR |= BEEP;FIO0SET = BEEP                      /* 蜂鸣器关                     */
#define BEEPON()    FIO0DIR |= BEEP;FIO0CLR = BEEP                      /* 蜂鸣器开                     */

/*********************************************************************************************************
  TASK0 任务ID、优先级、堆栈设置及函数声明
*********************************************************************************************************/
#define TASK0_ID                5                                       /* 任务的ID                     */
#define TASK0_PRIO              TASK0_ID                                /* 任务的优先级                 */
#define TASK0_STACK_SIZE        512                                     /* 定义用户堆栈长度             */
OS_STK  TASK0_STACK[TASK0_STACK_SIZE];                                  /* 定义任务0 堆栈               */
void    TASK0(void *pdata);                                             /* 声明任务0                    */

/*********************************************************************************************************
  TASK1 任务ID、优先级、堆栈设置及函数声明
*********************************************************************************************************/
#define TASK1_ID                6                                       /* 任务的ID                     */
#define TASK1_PRIO              TASK1_ID                                /* 任务的优先级                 */
#define TASK1_STACK_SIZE        1024                                    /* 定义用户堆栈长度             */
OS_STK  TASK1_STACK[TASK1_STACK_SIZE];                                  /* 定义任务1 堆栈               */
void    TASK1(void *pdata);                                             /* 声明任务1                    */

/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define MAIN_TASK_STK_SIZE      500                                     /* 主任务堆栈大小               */

#define DEBUG_STR_SEND(str)     uart0StrSend((char *)str)               /* 调试状态字符串发送函数       */

#define _sprintf(a, b, c)       sprintf(a, b, c)

#define BLOCK_SIZE              1024                                     /* 块大小(字节)                 */
#define START_ADDR              1000                                    /* 读写块的起始地址             */

#define USB_DATA_SIZE           (sizeof(__GpTestData))                  /* 读写数据长度(字节数)         */

/*********************************************************************************************************
  定义全局变量
*********************************************************************************************************/
const char    __GpTestData[] = {                                        /* 测试填充数据                 */
"        ZLG/MassStorage & ZLG/USB3200 HOST使用示例\r\n"
"  本示例将演示如何使用大容量类主机软件包ZLG/MassStorage和\r\n"
"ZLG/USB3200 HOST软件包对大容量类设备进行读、写操作。此处的大容量\r\n"
"类设备指U盘、USB接口的移动硬盘和带储存卡的读卡器等。注意后文的\r\n"
"\"读卡器\"均指已插入储存卡的读卡器。\r\n"
"    本示例不带文件系统，其功能为先向大容量类设备写入一定大小的数据，\r\n"
"然后读出来并进行校验，读写的结果将通过串口发送到PC机上。注意此示例直接\r\n"
"向大容量类设备的某些块写入数据，这将导致这些块原来的数据丢失，请在操作前\r\n"
"先备份该大容量类设备中的数据。"
"        ZLG/MassStorage & ZLG/USB3200 HOST使用示例\r\n"
"  本示例将演示如何使用大容量类主机软件包ZLG/MassStorage和\r\n"
"ZLG/USB3200 HOST软件包对大容量类设备进行读、写操作。此处的大容量\r\n"
"类设备指U盘、USB接口的移动硬盘和带储存卡的读卡器等。注意后文的\r\n"
"\"读卡器\"均指已插入储存卡的读卡器。\r\n"
"    本示例不带文件系统，其功能为先向大容量类设备写入一定大小的数据，\r\n"
"然后读出来并进行校验，读写的结果将通过串口发送到PC机上。注意此示例直接\r\n"
"向大容量类设备的某些块写入数据，这将导致这些块原来的数据丢失，请在操作前\r\n"
"先备份该大容量类设备中的数据。"
"        ZLG/MassStorage & ZLG/USB3200 HOST使用示例\r\n"
"  本示例将演示如何使用大容量类主机软件包ZLG/MassStorage和\r\n"
"ZLG/USB3200 HOST软件包对大容量类设备进行读、写操作。此处的大容量\r\n"
"类设备指U盘、USB接口的移动硬盘和带储存卡的读卡器等。注意后文的\r\n"
"\"读卡器\"均指已插入储存卡的读卡器。\r\n"
"    本示例不带文件系统，其功能为先向大容量类设备写入一定大小的数据，\r\n"
"然后读出来并进行校验，读写的结果将通过串口发送到PC机上。注意此示例直接\r\n"
"向大容量类设备的某些块写入数据，这将导致这些块原来的数据丢失，请在操作前\r\n"
"先备份该大容量类设备中的数据。"
};

/*********************************************************************************************************
** Function name:       EnumSucessCallBack
** Descriptions:        设备断开连接回调函数
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void EnumSucessCallBack (void)
{
    DEBUG_STR_SEND("\r\n Hello, Enum is Success\r\n");
}

/*********************************************************************************************************
** Function name:       DevDisconCallBack
** Descriptions:        设备断开连接回调函数
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void DevDisconCallBack (void)
{
    DEBUG_STR_SEND("\r\nHello, device is disconnect\r\n");
}

/*********************************************************************************************************
** Function name:       usbHostTestInit
** Descriptions:        初始化USB HOST
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void usbHostTestInit (void)
{
    unsigned char ucRet;

     if (ZY_OK != zyIsrSet(NVIC_USB, (unsigned long)usbHostException, 9 << 3)) {
        while (1);
    }

    DEBUG_STR_SEND("Hello!! Here comes the USB Host demo without FS\r\n\r\n");

    ucRet = usbHostInitialize(10, 11, EnumSucessCallBack, DevDisconCallBack);
    if (ucRet != TRUE) {
        DEBUG_STR_SEND("HOST init fail\r\n");
        while (1);
    }
}

/*********************************************************************************************************
** Function name:       usbHostTest
** Descriptions:        大容量类读写
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void usbHostTest (void)
{

    INT8U  ucRet;
//    INT32U uiLen;
    char   strBuf[40];
//    INT8U  pUsbData[USB_DATA_SIZE + BLOCK_SIZE];                        /* 读写缓冲区                   */
  	u8 handle,handle1,mode;
	u8 buf[512],ATTR;
	u8 entry_name[100];
	u32 i;
    /**
     * 等待枚举完设备
     */
    do {
        OSTimeDly(10);
    } while (!usbIsDeviceReady());

    ucRet = msHostInit(0);                                              /* 大容量类设备初始化           */
    if (ucRet != MS_ERR_SUCESS) {
        (void)_sprintf(strBuf, "\r\n!!msInit failed: 0x%-X\r\n", ucRet);
        DEBUG_STR_SEND(strBuf);
    } else {
          DEBUG_STR_SEND("\r\nmsHostInit sucess!\r\n");
    }
    
	
	FAT_filesystem_initialiation();
    //create driectory "ok" at root directory
	create_floder("c:\\ok");
	//create 3 files under folder "ok"
	create_file("c:\\ok\\test.txt");
	create_file("c:\\ok\\test1.txt");
	create_file("c:\\ok\\test2.txt");
	
	//write the file "c:\\ok\\test.txt" 
	
	handle  = open_file("c:\\ok\\test.txt");

	for(i = 0;i<512;i++)
	  buf[i] = i;
	write_file(handle,buf, 512);
	write_file(handle,buf, 512);
	write_file(handle,buf, 512);
    close_file(handle);

	//rough copy 
	//copy folder "ok" to "xx"
	create_floder("c:\\xx");
    cd_folder("c:\\ok",0);
    mode = 0;
    while(folder_enumeration(entry_name,mode,&ATTR) == SUCC)
    { 
	  handle = open_file(entry_name);

      entry_name[3] = 'x';
	  entry_name[4] = 'x';
	  //copy files
	  create_file(entry_name);
	  handle1 = open_file(entry_name);
	  do{
	      i = read_file(handle,buf,100);
		  write_file(handle1,buf,i);
	  }while(i == 100);
	  
	  close_file(handle);
	  close_file(handle1);
      if(mode == 0)
	   mode = 1;
    }




 	#ifdef   disable
    (void)memcpy(pUsbData, __GpTestData, USB_DATA_SIZE);                /* 初始化待写入的数据缓冲区     */

    /**
     * 计算块总数
     */
    uiLen = (USB_DATA_SIZE / BLOCK_SIZE);
    if ((USB_DATA_SIZE % BLOCK_SIZE) != 0) {
        uiLen += 1;
    }

    ucRet = rbcWrite10(0, 0, START_ADDR, uiLen, pUsbData);              /* 将数据写入大容量类设备中     */
    if (ucRet != MS_ERR_SUCESS) {
        (void)_sprintf(strBuf, "\r\n!!read failed!: ErrorCode : 0x%-X\r\n", ucRet);
        DEBUG_STR_SEND(strBuf);
    } else {
          DEBUG_STR_SEND("\r\nrbcWrite10 sucess!\r\n");
    }



    (void)memset(pUsbData, 0, USB_DATA_SIZE);                           /* 清空数据缓冲区               */

    ucRet = rbcRead10(0, 0, START_ADDR, uiLen, pUsbData);               /* 从大容量类设备中读出数据     */
    if (ucRet != MS_ERR_SUCESS) {
        (void)_sprintf(strBuf, "\r\n!!read failed!: ErrorCode : 0x%-X\r\n", ucRet);
        DEBUG_STR_SEND(strBuf);
    } else {
          DEBUG_STR_SEND("\r\nrbcRead10 sucess!Here comes the data:\r\n---\r\n");
          DEBUG_STR_SEND(pUsbData);
          DEBUG_STR_SEND("\r\n---\r\n");
    }

    if (0 == strcmp(__GpTestData, (const char *)pUsbData)) {
        DEBUG_STR_SEND("\r\n校验成功!\r\n");
    } else {
          DEBUG_STR_SEND("\r\n校验失败!\r\n");
    }
	#endif
    DEBUG_STR_SEND("\r\n请拔出设备!\r\n");
    while (usbIsDeviceReady()) {
        OSTimeDly(1);                                                   /* 等待设备拔出                 */
    }

    if (msHostDeInit() == TRUE) {
        DEBUG_STR_SEND("\r\nMass store class driver deleted\r\n");
    } else {
        DEBUG_STR_SEND("\r\nMass store class driver delete failed\r\n");
        while (1);
    }
    DEBUG_STR_SEND("\r\n 设备已经拔出! \r\n");
	
}

/*********************************************************************************************************
** Function name:       main
** Descriptions:        本实验为USB HOST在无文件系统下大容量类主机实验
**                      实验前需将开发板上JP13上USB HOST对应的所有端口用跳线帽短接
**                      (OVRCR和P1_27, H_VBUS和P1_22, H_PPWR和P1_19, HOST_D-和P0_30, HOST_D+和P0_29)
**                      同时用串口线将开发板UART0与PC相连
**                      并用跳线帽短接JP14上UART0对应的端口(RXD0和P0_3, TXD0和P0_2)
**                      打开串口调试软件,将波特率设为115200，模式设为8，N，1
** Input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
int main (void)
{
    targetInit();
    pinInit();

    OSInit();
    OSTaskCreateExt(TASK0,
                    (void *)0,
                    &TASK0_STACK[TASK0_STACK_SIZE-1],
                    TASK0_PRIO,
                    TASK0_ID,
                    &TASK0_STACK[0],
                    TASK0_STACK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    OSStart();
    return 0;
}

/*********************************************************************************************************
** Function name:       TASK0
** Descriptions:        创建TASK1
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void TASK0 (void *pdata)
{
#if 1
    BEEPON();
    OSTimeDly(OS_TICKS_PER_SEC / 10);
    BEEPOFF();
    OSTimeDly(OS_TICKS_PER_SEC / 10);
    BEEPON();
    OSTimeDly(OS_TICKS_PER_SEC / 10);
    BEEPOFF();
    OSTimeDly(OS_TICKS_PER_SEC / 10);
#endif

    pdata = pdata;

    OSTaskCreateExt(TASK1,
                    (void *)200,
                    &TASK1_STACK[TASK1_STACK_SIZE-1],
                    TASK1_PRIO,
                    TASK1_ID,
                    &TASK1_STACK[0],
                    TASK1_STACK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);



    while(1) {
        OSTimeDly(OS_TICKS_PER_SEC);
        OSTimeDly(OS_TICKS_PER_SEC);
    }
}

/*********************************************************************************************************
** Function name:       TASK1
** Descriptions:        大容量类读写任务
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void TASK1 (void *pdata)
{
    pdata = pdata;

    uart0Init();
    usbHostTestInit();
    while(1) {
        usbHostTest();
        OSTimeDly(1);
    }
}



/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
