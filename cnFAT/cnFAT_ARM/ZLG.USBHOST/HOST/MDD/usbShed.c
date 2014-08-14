/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            usbShed.c
** Latest modified Date: 2007-11-24
** Latest Version:       V1.0
** Description:          ���������ʵ��
**
**--------------------------------------------------------------------------------------------------------
** Created By:           ��¡��   Longsui Wu
** Created date:         2007-11-06
** Version:              V1.0
** Descriptions:         ��ʼ�汾
**
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
**
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
**
*********************************************************************************************************/
#include "..\USBHostIncludes.h"

OS_EVENT *__GevtUsbMsgQeue;                                             /*  ������Ϣ����                */
void     *__GevtUsbMsgQeueTbl[__USB_MSGQEUE_LENGTH];                    /*  ��Ϣ���л�����              */

OS_EVENT *__GevtUsbCtrlMbox;                                            /*  ���ƴ�������                */
OS_EVENT *__GevtUsbDataTranMbox;                                        /*  ����,�жϴ�������           */

OS_EVENT *__GevtUsbCtrlSem;                                             /*  ���ƴ����ź���,����ʵ�ֻ��� */
OS_EVENT *__GevtUsbDataTranSem;                                         /*  ����,�жϴ����ź���         */

#define __TASK_USB_SCHED_STK_LEN    512                                 /*  ���������ջ����            */
#define __TASK_USB_ENUM_STK_LEN     512                                 /*  ö�������ջ����            */

void __taskUsbSheduler (void *pdata);
void __taskUsbEnum (void *pdata);
void __usbEnumDeal (USB_INT8U ucCode);
void __usbDoneHeadDeal (USB_INT32U uiTdHeadDoneList);
void __usbConectChange (USB_INT32U uiCode, USB_INT32U uiEnumPrio);

OS_STK	__taskUsbShedulerStk [__TASK_USB_SCHED_STK_LEN];                /*  ���������ջ                */
OS_STK	__taskUsbEnumStk [__TASK_USB_ENUM_STK_LEN];                     /*  ö�������ջ                */


/*********************************************************************************************************
** Function name:       __usbShedInit
** Descriptions:        USB���ȵĻ�����ʼ����,��Ҫ�ǳ�ʼ����Ϣ����,��Ϣ�����
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : �ɹ�    FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL __usbShedInit (void)
{
    __GevtUsbMsgQeue = OSQCreate(&__GevtUsbMsgQeueTbl[0], __USB_MSGQEUE_LENGTH);
    if (__GevtUsbMsgQeue == NULL) {
        return FALSE;
    }

    __GevtUsbCtrlMbox = OSMboxCreate((void *)0);
    if (__GevtUsbCtrlMbox == NULL) {
        return FALSE;
    }

    __GevtUsbDataTranMbox = OSMboxCreate((void *)0);
    if (__GevtUsbDataTranMbox == NULL) {
        return FALSE;
    }

    __GevtUsbCtrlSem = OSSemCreate(1);
    if (__GevtUsbCtrlSem == NULL) {
        return FALSE;
    }

    __GevtUsbDataTranSem = OSSemCreate(1);
    if (__GevtUsbDataTranSem == NULL) {
        return FALSE;
    }

    return TRUE;

}

/*********************************************************************************************************
** Function name:       __usbCreateShedTask
** Descriptions:        ������������
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : �����ɹ�    FALSE : ����ʧ��
*********************************************************************************************************/
USB_BOOL __usbCreateShedTask (USB_INT8U ucShedPrio, USB_INT8U ucEnumPrio)
{
    if (OSTaskCreate (__taskUsbSheduler, (void *)(USB_INT32U)ucEnumPrio, \
        &__taskUsbShedulerStk[__TASK_USB_SCHED_STK_LEN - 1], ucShedPrio) == OS_NO_ERR) {
        return TRUE;
    } else {
        USBDEBUG_SENDSTR("\r\n!!Create scheduler task failed!\r\n ");
        return FALSE;
    }
}

/*********************************************************************************************************
** Function name:       __taskUsbSheduler
** Descriptions:        uC/OS-II����,���ڶ�USB�ĸ����¼����е���
** input parameters:    pdata: uC/OS-IIҪ��Ĳ���
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __taskUsbSheduler (void *pdata)
{
    INT8U  ucErr;
    INT32U uiMsgCode;

    while (1) {
        uiMsgCode = (INT32U)OSQPend(__GevtUsbMsgQeue, 0, &ucErr);

        if (ucErr == OS_NO_ERR) {

            //USBDEBUG_SENDSTR("\r\nstat change\r\n");

            switch ((__USBLSB(uiMsgCode, 3)) & 0xF0U) {                 /*  BIT31~28Ϊ�¼�����          */

            case __USB_SHED_CONCHANGE:                                  /*  ���Ӹı�                    */
                __usbConectChange(uiMsgCode, (USB_INT32U)pdata);
                USBDEBUG_SENDSTR("\r\nconnect chng\r\n");
                break;

            case __USB_SHED_DONEHEAD:                                   /*  Write back done����         */
                __usbDoneHeadDeal((uiMsgCode << 4) & 0xFFFFFFF0);
                break;

            case __USB_SHED_SHEDOVERRUN:                                /*  SchedulingOverrun ����      */
                USBDEBUG_SENDSTR("\r\nSchedulingOverrun\r\n");
                break;

            case __USB_SHED_UNRECERROR:                                 /*  Unrecoverable Error         */
                USBDEBUG_SENDSTR("\r\nUnrecoverable Error\r\n");
                break;

            case __USB_SHED_DELSELF:                                    /*  ɾ����������                */
                OSTaskDel(OS_PRIO_SELF);
                break;

            default:
                USBDEBUG_SENDSTR("\r\n!!Unknow MsgCode!\r\n\r\n");
                break;
            }
        }
    }
}

/*********************************************************************************************************
** Function name:       __usbDoneHeadDeal
** Descriptions:        ��write back done���д���
** input parameters:    uiTdHeadDoneList ������������ͷָ��
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __usbDoneHeadDeal (USB_INT32U uiTdHeadDoneList)
{
    USB_INT8U                     ucErrorCode;
    USB_INT8U                     ucType;

    __PHC_GEN_TRANSFER_DESCRIPTOR pTdHeadDoneList;
    __ED_INFO                     edInfo;


    pTdHeadDoneList = (__PHC_GEN_TRANSFER_DESCRIPTOR)uiTdHeadDoneList;
    ucType = __hcdGetTdType(pTdHeadDoneList);                           /*  ��ȡ��������                */
    ucErrorCode = (USB_INT8U)((pTdHeadDoneList->uiControl >> 28) & 0x0F);
                                                                        /*  ��ȡ������,���OHCI�淶4.3.3*/
    __hcdGetEdInfo(ucType, &edInfo);
    if (ucErrorCode != USB_ERR_NOERR) {                                 /*  ����������                  */

        USBDEBUG_SENDSTR("\r\n!!Erroe code: ");
        USBDEBUG_SENDCHAR((USB_INT8U)(ucErrorCode + 0x30));
        USBDEBUG_SENDSTR("!!!\r\n\r\n");

        __hcdEdLetHeadEquTail(&edInfo);                                 /*  ǿ�� HeadP = TailP          */
        __hcdFreeAllTd(&edInfo);                                        /*  �ͷ����� TD ��Դ            */
        if (ucType == USB_TRAN_TYPE_CONTROL) {                          /*  ����������Ϣ�����亯��      */
            OSMboxPost(__GevtUsbCtrlMbox, (void *)((ucErrorCode << 24) | __USB_TRANDEAL_ERR_TRY));
        } else {
            OSMboxPost(__GevtUsbDataTranMbox, (void *)((ucErrorCode << 24) | __USB_TRANDEAL_ERR_TRY));
        }
    } else {                                                            /*  ��������                    */
        if (ucType == USB_TRAN_TYPE_CONTROL) {                          /*  ���ƴ���                    */
            if ((USB_INT32U)pTdHeadDoneList == (USB_INT32U)__GsTranEndTd.pCtrlEndTd) {
                                                                        /*  �жϴ����Ƿ����            */
                __GsTranEndTd.pCtrlEndTd = NULL;
                __hcdFreeAllTd(&edInfo);
                OSMboxPost(__GevtUsbCtrlMbox, (void *)__USB_TRANDEAL_OK);
            }
        } else {                                                        /*  ���ݴ���(BULK��INTR����)    */
            if ((USB_INT32U)pTdHeadDoneList == (USB_INT32U)__GsTranEndTd.pDataEndTd) {
                __GsTranEndTd.pDataEndTd = NULL;
                __hcdFreeAllTd(&edInfo);
                OSMboxPost(__GevtUsbDataTranMbox, (void *)__USB_TRANDEAL_OK);
            }
        }
    }
}

/*********************************************************************************************************
** Function name:       __usbConectChange
** Descriptions:        ���Ӹı䴦��
** input parameters:    uiCode      �����豸�ǲ��뻹�ǲ�������Ϣ����
**                      uiEnumPrio  ö����������ȼ�
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __usbConectChange (USB_INT32U uiCode, USB_INT32U uiEnumPrio)
{
    USB_INT8U ucErr, bRet;

    if (__USBLSB(uiCode, 0) == 0x00) {
        if (GusbHostFlags.ucAttached == 0) {
            __ohciInit();
            return;
        }

        OS_ENTER_CRITICAL();
        GusbHostFlags.ucEnumed = 0;
        GusbHostFlags.ucConfiged = 0;
        GusbHostFlags.ucAttached = 0;

        GusbHostFlags.ucCtrlPipe = 0;
        GusbHostFlags.ucBulkOutPipe = 0;
        GusbHostFlags.ucBulkInPipe = 0;
        GusbHostFlags.ucIntrOutPipe = 0;
        GusbHostFlags.ucIntrInPipe = 0;
        OS_EXIT_CRITICAL();

        usbPipeClose(USB_TRAN_TYPE_CONTROL);
        usbPipeClose(USB_TRAN_TYPE_INTR_OUT);
        usbPipeClose(USB_TRAN_TYPE_INTR_IN);
        usbPipeClose(USB_TRAN_TYPE_BULK_OUT);
        usbPipeClose(USB_TRAN_TYPE_BULK_IN);

        __ohciInit();

        USBDEBUG_SENDSTR("Device remove! Close Pipe!\r\n");

        if (!OSTCBPrioTbl[(INT8U)uiEnumPrio]) {                         /*  �豸������,���ö��������,    */
            OSTaskDelReq((INT8U)uiEnumPrio);                            /*  ...������ɾ��                   */
        }
        if (__GpDevDisconCallBack) {
            (*__GpDevDisconCallBack)();                                 /*  ִ���豸�����ص�����            */
        }

    } else {

        OSTimeDly(20);

        bRet = __ohciAffirmAttach(__USBLSB(uiCode, 1));                 /*  ȷ���豸�Ƿ��Դ�����״̬(δ����)*/
        if (bRet == FALSE) {
            __ohciInit();
            return;
        }

        GusbHostFlags.ucAttached = 1;

        USBDEBUG_SENDSTR("\r\n\r\nDevice attach!\r\n");

        __hcdInit();
        __ohciInit2(__USBLSB(uiCode, 1));

        uiEnumPrio = uiEnumPrio & 0xFF;
        if (GusbHostFlags.ucCtrlPipe == 1) {
            return;
        }
        usbPipeOpen(USB_TRAN_TYPE_CONTROL);

        USBDEBUG_SENDSTR("Open Pipe!\r\n");

        ucErr = OSTaskCreate (__taskUsbEnum, (void *)0, &__taskUsbEnumStk[__TASK_USB_ENUM_STK_LEN - 1], \
                             (USB_INT8U)uiEnumPrio);
        if (ucErr == OS_NO_ERR) {
            USBDEBUG_SENDSTR("\r\n Create Enumerate task sucess! Start enumerate...\r\n");
        } else {
            USBDEBUG_SENDSTR("\r\n\r\n Create Enumerate task failed!\r\n");
        }
    }
}

/*********************************************************************************************************
** Function name:       __usbEnumDeal
** Descriptions:        ö�ٺ�Ĵ���
** input parameters:    ucCode ö���Ƿ�ɹ���־
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __usbEnumDeal (USB_INT8U ucCode)
{
    if (ucCode == 0x01) {
        usbPipeOpen(USB_TRAN_TYPE_INTR_OUT);
        usbPipeOpen(USB_TRAN_TYPE_INTR_IN);
        usbPipeOpen(USB_TRAN_TYPE_BULK_OUT);
        usbPipeOpen(USB_TRAN_TYPE_BULK_IN);
    }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
