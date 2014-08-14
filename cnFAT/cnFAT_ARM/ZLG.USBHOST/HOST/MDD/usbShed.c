/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            usbShed.c
** Latest modified Date: 2007-11-24
** Latest Version:       V1.0
** Description:          调度任务的实现
**
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗   Longsui Wu
** Created date:         2007-11-06
** Version:              V1.0
** Descriptions:         初始版本
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

OS_EVENT *__GevtUsbMsgQeue;                                             /*  调度消息队列                */
void     *__GevtUsbMsgQeueTbl[__USB_MSGQEUE_LENGTH];                    /*  消息队列缓冲区              */

OS_EVENT *__GevtUsbCtrlMbox;                                            /*  控制传输邮箱                */
OS_EVENT *__GevtUsbDataTranMbox;                                        /*  批量,中断传输邮箱           */

OS_EVENT *__GevtUsbCtrlSem;                                             /*  控制传输信号量,用于实现互斥 */
OS_EVENT *__GevtUsbDataTranSem;                                         /*  批量,中断传输信号量         */

#define __TASK_USB_SCHED_STK_LEN    512                                 /*  调度任务堆栈长度            */
#define __TASK_USB_ENUM_STK_LEN     512                                 /*  枚举任务堆栈长度            */

void __taskUsbSheduler (void *pdata);
void __taskUsbEnum (void *pdata);
void __usbEnumDeal (USB_INT8U ucCode);
void __usbDoneHeadDeal (USB_INT32U uiTdHeadDoneList);
void __usbConectChange (USB_INT32U uiCode, USB_INT32U uiEnumPrio);

OS_STK	__taskUsbShedulerStk [__TASK_USB_SCHED_STK_LEN];                /*  调度任务堆栈                */
OS_STK	__taskUsbEnumStk [__TASK_USB_ENUM_STK_LEN];                     /*  枚举任务堆栈                */


/*********************************************************************************************************
** Function name:       __usbShedInit
** Descriptions:        USB调度的环境初始化化,主要是初始化消息队列,消息邮箱等
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 成功    FALSE : 失败
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
** Descriptions:        创建调度任务
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 创建成功    FALSE : 创建失败
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
** Descriptions:        uC/OS-II任务,用于对USB的各个事件进行调度
** input parameters:    pdata: uC/OS-II要求的参数
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

            switch ((__USBLSB(uiMsgCode, 3)) & 0xF0U) {                 /*  BIT31~28为事件类型          */

            case __USB_SHED_CONCHANGE:                                  /*  连接改变                    */
                __usbConectChange(uiMsgCode, (USB_INT32U)pdata);
                USBDEBUG_SENDSTR("\r\nconnect chng\r\n");
                break;

            case __USB_SHED_DONEHEAD:                                   /*  Write back done处理         */
                __usbDoneHeadDeal((uiMsgCode << 4) & 0xFFFFFFF0);
                break;

            case __USB_SHED_SHEDOVERRUN:                                /*  SchedulingOverrun 处理      */
                USBDEBUG_SENDSTR("\r\nSchedulingOverrun\r\n");
                break;

            case __USB_SHED_UNRECERROR:                                 /*  Unrecoverable Error         */
                USBDEBUG_SENDSTR("\r\nUnrecoverable Error\r\n");
                break;

            case __USB_SHED_DELSELF:                                    /*  删除自身任务                */
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
** Descriptions:        对write back done进行处理
** input parameters:    uiTdHeadDoneList 传输完成链表表头指针
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
    ucType = __hcdGetTdType(pTdHeadDoneList);                           /*  获取传输类型                */
    ucErrorCode = (USB_INT8U)((pTdHeadDoneList->uiControl >> 28) & 0x0F);
                                                                        /*  获取错误码,详见OHCI规范4.3.3*/
    __hcdGetEdInfo(ucType, &edInfo);
    if (ucErrorCode != USB_ERR_NOERR) {                                 /*  非正常结束                  */

        USBDEBUG_SENDSTR("\r\n!!Erroe code: ");
        USBDEBUG_SENDCHAR((USB_INT8U)(ucErrorCode + 0x30));
        USBDEBUG_SENDSTR("!!!\r\n\r\n");

        __hcdEdLetHeadEquTail(&edInfo);                                 /*  强制 HeadP = TailP          */
        __hcdFreeAllTd(&edInfo);                                        /*  释放所有 TD 资源            */
        if (ucType == USB_TRAN_TYPE_CONTROL) {                          /*  发送重试信息给传输函数      */
            OSMboxPost(__GevtUsbCtrlMbox, (void *)((ucErrorCode << 24) | __USB_TRANDEAL_ERR_TRY));
        } else {
            OSMboxPost(__GevtUsbDataTranMbox, (void *)((ucErrorCode << 24) | __USB_TRANDEAL_ERR_TRY));
        }
    } else {                                                            /*  传输正常                    */
        if (ucType == USB_TRAN_TYPE_CONTROL) {                          /*  控制传输                    */
            if ((USB_INT32U)pTdHeadDoneList == (USB_INT32U)__GsTranEndTd.pCtrlEndTd) {
                                                                        /*  判断传输是否完毕            */
                __GsTranEndTd.pCtrlEndTd = NULL;
                __hcdFreeAllTd(&edInfo);
                OSMboxPost(__GevtUsbCtrlMbox, (void *)__USB_TRANDEAL_OK);
            }
        } else {                                                        /*  数据传输(BULK或INTR传输)    */
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
** Descriptions:        连接改变处理
** input parameters:    uiCode      包括设备是插入还是拨出的信息代码
**                      uiEnumPrio  枚举任务的优先级
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

        if (!OSTCBPrioTbl[(INT8U)uiEnumPrio]) {                         /*  设备拨出后,如果枚举任务还在,    */
            OSTaskDelReq((INT8U)uiEnumPrio);                            /*  ...则申请删除                   */
        }
        if (__GpDevDisconCallBack) {
            (*__GpDevDisconCallBack)();                                 /*  执行设备拨出回调函数            */
        }

    } else {

        OSTimeDly(20);

        bRet = __ohciAffirmAttach(__USBLSB(uiCode, 1));                 /*  确认设备是否仍处插入状态(未拨出)*/
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
** Descriptions:        枚举后的处理
** input parameters:    ucCode 枚举是否成功标志
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
