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
** Descriptions:        ����û�Ӧ�ó���
**
**--------------------------------------------------------------------------------------------------------
** Modified by:         LiuWeiyun
** Modified date:       2009-07-27
** Version:             V1.01
** Descriptions:        ��дUSB_HOST_NO_FS����
**m
** Rechecked by:        WangJianyu
*********************************************************************************************************/
#include "..\config.h"
#include "..\fat\types.h"
#include "..\fat\fat.h"

/*********************************************************************************************************
  ������궨��
*********************************************************************************************************/
#define BEEP        (1 << 11)                                           /* ������                       */
#define KEY1        (1 << 10)                                           /* P2.10����KEY1                */
#define BEEPOFF()   FIO0DIR |= BEEP;FIO0SET = BEEP                      /* ��������                     */
#define BEEPON()    FIO0DIR |= BEEP;FIO0CLR = BEEP                      /* ��������                     */

/*********************************************************************************************************
  TASK0 ����ID�����ȼ�����ջ���ü���������
*********************************************************************************************************/
#define TASK0_ID                5                                       /* �����ID                     */
#define TASK0_PRIO              TASK0_ID                                /* ��������ȼ�                 */
#define TASK0_STACK_SIZE        512                                     /* �����û���ջ����             */
OS_STK  TASK0_STACK[TASK0_STACK_SIZE];                                  /* ��������0 ��ջ               */
void    TASK0(void *pdata);                                             /* ��������0                    */

/*********************************************************************************************************
  TASK1 ����ID�����ȼ�����ջ���ü���������
*********************************************************************************************************/
#define TASK1_ID                6                                       /* �����ID                     */
#define TASK1_PRIO              TASK1_ID                                /* ��������ȼ�                 */
#define TASK1_STACK_SIZE        1024                                    /* �����û���ջ����             */
OS_STK  TASK1_STACK[TASK1_STACK_SIZE];                                  /* ��������1 ��ջ               */
void    TASK1(void *pdata);                                             /* ��������1                    */

/*********************************************************************************************************
  �궨��
*********************************************************************************************************/
#define MAIN_TASK_STK_SIZE      500                                     /* �������ջ��С               */

#define DEBUG_STR_SEND(str)     uart0StrSend((char *)str)               /* ����״̬�ַ������ͺ���       */

#define _sprintf(a, b, c)       sprintf(a, b, c)

#define BLOCK_SIZE              1024                                     /* ���С(�ֽ�)                 */
#define START_ADDR              1000                                    /* ��д�����ʼ��ַ             */

#define USB_DATA_SIZE           (sizeof(__GpTestData))                  /* ��д���ݳ���(�ֽ���)         */

/*********************************************************************************************************
  ����ȫ�ֱ���
*********************************************************************************************************/
const char    __GpTestData[] = {                                        /* �����������                 */
"        ZLG/MassStorage & ZLG/USB3200 HOSTʹ��ʾ��\r\n"
"  ��ʾ������ʾ���ʹ�ô����������������ZLG/MassStorage��\r\n"
"ZLG/USB3200 HOST������Դ��������豸���ж���д�������˴��Ĵ�����\r\n"
"���豸ָU�̡�USB�ӿڵ��ƶ�Ӳ�̺ʹ����濨�Ķ������ȡ�ע����ĵ�\r\n"
"\"������\"��ָ�Ѳ��봢�濨�Ķ�������\r\n"
"    ��ʾ�������ļ�ϵͳ���书��Ϊ������������豸д��һ����С�����ݣ�\r\n"
"Ȼ�������������У�飬��д�Ľ����ͨ�����ڷ��͵�PC���ϡ�ע���ʾ��ֱ��\r\n"
"����������豸��ĳЩ��д�����ݣ��⽫������Щ��ԭ�������ݶ�ʧ�����ڲ���ǰ\r\n"
"�ȱ��ݸô��������豸�е����ݡ�"
"        ZLG/MassStorage & ZLG/USB3200 HOSTʹ��ʾ��\r\n"
"  ��ʾ������ʾ���ʹ�ô����������������ZLG/MassStorage��\r\n"
"ZLG/USB3200 HOST������Դ��������豸���ж���д�������˴��Ĵ�����\r\n"
"���豸ָU�̡�USB�ӿڵ��ƶ�Ӳ�̺ʹ����濨�Ķ������ȡ�ע����ĵ�\r\n"
"\"������\"��ָ�Ѳ��봢�濨�Ķ�������\r\n"
"    ��ʾ�������ļ�ϵͳ���书��Ϊ������������豸д��һ����С�����ݣ�\r\n"
"Ȼ�������������У�飬��д�Ľ����ͨ�����ڷ��͵�PC���ϡ�ע���ʾ��ֱ��\r\n"
"����������豸��ĳЩ��д�����ݣ��⽫������Щ��ԭ�������ݶ�ʧ�����ڲ���ǰ\r\n"
"�ȱ��ݸô��������豸�е����ݡ�"
"        ZLG/MassStorage & ZLG/USB3200 HOSTʹ��ʾ��\r\n"
"  ��ʾ������ʾ���ʹ�ô����������������ZLG/MassStorage��\r\n"
"ZLG/USB3200 HOST������Դ��������豸���ж���д�������˴��Ĵ�����\r\n"
"���豸ָU�̡�USB�ӿڵ��ƶ�Ӳ�̺ʹ����濨�Ķ������ȡ�ע����ĵ�\r\n"
"\"������\"��ָ�Ѳ��봢�濨�Ķ�������\r\n"
"    ��ʾ�������ļ�ϵͳ���书��Ϊ������������豸д��һ����С�����ݣ�\r\n"
"Ȼ�������������У�飬��д�Ľ����ͨ�����ڷ��͵�PC���ϡ�ע���ʾ��ֱ��\r\n"
"����������豸��ĳЩ��д�����ݣ��⽫������Щ��ԭ�������ݶ�ʧ�����ڲ���ǰ\r\n"
"�ȱ��ݸô��������豸�е����ݡ�"
};

/*********************************************************************************************************
** Function name:       EnumSucessCallBack
** Descriptions:        �豸�Ͽ����ӻص�����
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
void EnumSucessCallBack (void)
{
    DEBUG_STR_SEND("\r\n Hello, Enum is Success\r\n");
}

/*********************************************************************************************************
** Function name:       DevDisconCallBack
** Descriptions:        �豸�Ͽ����ӻص�����
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
void DevDisconCallBack (void)
{
    DEBUG_STR_SEND("\r\nHello, device is disconnect\r\n");
}

/*********************************************************************************************************
** Function name:       usbHostTestInit
** Descriptions:        ��ʼ��USB HOST
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
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
** Descriptions:        ���������д
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
*********************************************************************************************************/
void usbHostTest (void)
{

    INT8U  ucRet;
//    INT32U uiLen;
    char   strBuf[40];
//    INT8U  pUsbData[USB_DATA_SIZE + BLOCK_SIZE];                        /* ��д������                   */
  	u8 handle,handle1,mode;
	u8 buf[512],ATTR;
	u8 entry_name[100];
	u32 i;
    /**
     * �ȴ�ö�����豸
     */
    do {
        OSTimeDly(10);
    } while (!usbIsDeviceReady());

    ucRet = msHostInit(0);                                              /* ���������豸��ʼ��           */
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
    (void)memcpy(pUsbData, __GpTestData, USB_DATA_SIZE);                /* ��ʼ����д������ݻ�����     */

    /**
     * ���������
     */
    uiLen = (USB_DATA_SIZE / BLOCK_SIZE);
    if ((USB_DATA_SIZE % BLOCK_SIZE) != 0) {
        uiLen += 1;
    }

    ucRet = rbcWrite10(0, 0, START_ADDR, uiLen, pUsbData);              /* ������д����������豸��     */
    if (ucRet != MS_ERR_SUCESS) {
        (void)_sprintf(strBuf, "\r\n!!read failed!: ErrorCode : 0x%-X\r\n", ucRet);
        DEBUG_STR_SEND(strBuf);
    } else {
          DEBUG_STR_SEND("\r\nrbcWrite10 sucess!\r\n");
    }



    (void)memset(pUsbData, 0, USB_DATA_SIZE);                           /* ������ݻ�����               */

    ucRet = rbcRead10(0, 0, START_ADDR, uiLen, pUsbData);               /* �Ӵ��������豸�ж�������     */
    if (ucRet != MS_ERR_SUCESS) {
        (void)_sprintf(strBuf, "\r\n!!read failed!: ErrorCode : 0x%-X\r\n", ucRet);
        DEBUG_STR_SEND(strBuf);
    } else {
          DEBUG_STR_SEND("\r\nrbcRead10 sucess!Here comes the data:\r\n---\r\n");
          DEBUG_STR_SEND(pUsbData);
          DEBUG_STR_SEND("\r\n---\r\n");
    }

    if (0 == strcmp(__GpTestData, (const char *)pUsbData)) {
        DEBUG_STR_SEND("\r\nУ��ɹ�!\r\n");
    } else {
          DEBUG_STR_SEND("\r\nУ��ʧ��!\r\n");
    }
	#endif
    DEBUG_STR_SEND("\r\n��γ��豸!\r\n");
    while (usbIsDeviceReady()) {
        OSTimeDly(1);                                                   /* �ȴ��豸�γ�                 */
    }

    if (msHostDeInit() == TRUE) {
        DEBUG_STR_SEND("\r\nMass store class driver deleted\r\n");
    } else {
        DEBUG_STR_SEND("\r\nMass store class driver delete failed\r\n");
        while (1);
    }
    DEBUG_STR_SEND("\r\n �豸�Ѿ��γ�! \r\n");
	
}

/*********************************************************************************************************
** Function name:       main
** Descriptions:        ��ʵ��ΪUSB HOST�����ļ�ϵͳ�´�����������ʵ��
**                      ʵ��ǰ�轫��������JP13��USB HOST��Ӧ�����ж˿�������ñ�̽�
**                      (OVRCR��P1_27, H_VBUS��P1_22, H_PPWR��P1_19, HOST_D-��P0_30, HOST_D+��P0_29)
**                      ͬʱ�ô����߽�������UART0��PC����
**                      ��������ñ�̽�JP14��UART0��Ӧ�Ķ˿�(RXD0��P0_3, TXD0��P0_2)
**                      �򿪴��ڵ������,����������Ϊ115200��ģʽ��Ϊ8��N��1
** Input parameters:    ��
** output parameters:   ��
** Returned value:      ��
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
** Descriptions:        ����TASK1
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
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
** Descriptions:        ���������д����
** input parameters:    ��
** output parameters:   ��
** Returned value:      ��
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
