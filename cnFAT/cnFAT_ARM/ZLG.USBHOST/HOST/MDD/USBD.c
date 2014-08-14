/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            usbd.c
** Latest modified Date: 2007-11-15
** Latest Version:       V1.0
** Description:          USBD层的实现
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗  Longsui Wu
** Created date:         2007-11-15
** Version:              V1.0
** Descriptions:         初始版本
**--------------------------------------------------------------------------------------------------------
** Modified by:          liuweiyun
** Modified date:        2009-02-02
** Version:              V1.0
** Description:          Improve some code
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
*********************************************************************************************************/
#include    "..\USBHostIncludes.h"

USB_DEVICE_DESCRIPTOR         GusbDeviceDescr;                          /*  设备描述符                  */
USB_CONFIGURATION_DESCRIPTOR  GusbConfigDescr;                          /*  配置描述符                  */
USB_OTG_DESCRIPTOR            GusbOtgDescr;

__USB_INTERFACE_EP_DESCR      GusbInterEpDescr[__USB_MAX_INTERFACE];    /*  配置描述符:接口描述符及该   */
                                                                        /*  ...接口所具有的端点的描述符 */
USB_INT16U                    __GusEpMaxPktSize[__USB_MAX_INTERFACE][32] = {0};

volatile __USB_HOST_FLAGS     GusbHostFlags;                            /*  标志位                      */
__USB_HOST_EVENT_CNT          __GusbHostEvtCnt;
INT8U                         __GucHostShedPrio = 0;
INT8U                         __GucHostEnumPrio = 0;

void                          (*__GpEnumSucessCallBack)(void) = NULL;   /*  枚举成功时回调函数          */
void                          (*__GpDevDisconCallBack)(void)  = NULL;   /*  设备拨出时回调函数          */

void __usbdInit (void (*pEnumSucessCallBack)(void), void (*pDevDisconCallBack)(void));

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************************
** Function name:       usbHostInitialize
** Descriptions:        USB 主机初始化
** input parameters:    ucShedPrio           调度任务的优先级
**                      ucEnumPrio           枚举任务的优先级
**                      pEnumSucessCallBack: 枚举成功回调函数,若没有则设置为NULL
**                      pDevDisconCallBack:  设备拨出回调函数,若没有则设置为NULL
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL usbHostInitialize (USB_INT8U ucShedPrio,
                            USB_INT8U ucEnumPrio,
                            void      (*pEnumSucessCallBack)(void),
                            void      (*pDevDisconCallBack)(void))
{
    __usbdInit(pEnumSucessCallBack, pDevDisconCallBack);

    if (!__usbShedInit()) {
        return FALSE;
    }

    __usbInitHardware();
    __hcdInit();
    __usbHostIntInit();
    __ohciInit();

    if (!__usbCreateShedTask(ucShedPrio, ucEnumPrio)) {                 /*  创建调度任务                */
        __GucHostShedPrio = 0;
        __GucHostEnumPrio = 0;
        return FALSE;
    }

    OS_ENTER_CRITICAL();
    __GucHostShedPrio = ucShedPrio;
    __GucHostEnumPrio = ucEnumPrio;
    OS_EXIT_CRITICAL();

    return TRUE;
}

/*********************************************************************************************************
** Function name:       usbHostDeInit
** Descriptions:        USB 主机卸载
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL usbHostDeInit (void)
{
    USB_INT8U  ucErr;
    USB_INT32U i;
    OS_TCB    *ptcb = NULL;

    USBDEBUG_SENDSTR("\r\nusbHostDeInit...\r\n");

    OS_ENTER_CRITICAL();
    GusbHostFlags.bExitDataOperaReq = TRUE;
    GusbHostFlags.bExitStdOperaReq  = TRUE;
    OS_EXIT_CRITICAL();

    /*
     *  等待没有任务任务操作USB总线
     */
    i = 0;
    while (__GusbHostEvtCnt.ucDataTranCnt || __GusbHostEvtCnt.ucStdTranCnt) {
        OSTimeDly(2);
        if (i++ > 1000) {                                               /*  等待10S                     */
            return FALSE;
        }
    }
    __ohciPortClose(USB_HOST_PORT);

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

#if 0

    usbPipeClose(USB_TRAN_TYPE_CONTROL);
    usbPipeClose(USB_TRAN_TYPE_INTR_OUT);
    usbPipeClose(USB_TRAN_TYPE_INTR_IN);
    usbPipeClose(USB_TRAN_TYPE_BULK_OUT);
    usbPipeClose(USB_TRAN_TYPE_BULK_IN);

#endif

    /*
     *  查看枚举任务是否存在,若存在,则删除该任务
     */
    if (__GucHostEnumPrio) {
        ptcb = OSTCBPrioTbl[__GucHostEnumPrio];                         /*  查询枚举任务是否已经存在    */
        if (ptcb != NULL) {                                             /*  删除枚举任务                */
            OSTaskDel(__GucHostEnumPrio);
            while (OSTCBPrioTbl[__GucHostEnumPrio] != NULL) {
                OSTimeDly(1);
            }
            OS_ENTER_CRITICAL();
            __GucHostEnumPrio = 0;
            OS_EXIT_CRITICAL();
        }
    }

    /*
     *  查看调度任务是否存在,若存在,则删除该任务
     */
    if (__GucHostShedPrio) {
        ptcb = OSTCBPrioTbl[__GucHostShedPrio];
        if (ptcb != NULL) {
            OSQPost(__GevtUsbMsgQeue, (void *)(__USB_SHED_DELSELF << 24));
                                                                        /*  调度任务已存在,发送消息     */
                                                                        /*  要求该任务删除自己          */
            while (OSTCBPrioTbl[__GucHostShedPrio] != NULL) {
                OSTimeDly(1);
            }
            OS_ENTER_CRITICAL();
            __GucHostShedPrio = 0;
            OS_EXIT_CRITICAL();
        }
    }

    OS_ENTER_CRITICAL();
    __GevtUsbMsgQeue = OSQDel(__GevtUsbMsgQeue, OS_DEL_ALWAYS, &ucErr); /*  删除调度消息队列            */
    __GevtUsbCtrlMbox     = OSMboxDel(__GevtUsbCtrlMbox, OS_DEL_ALWAYS, &ucErr);
    __GevtUsbDataTranMbox = OSMboxDel(__GevtUsbDataTranMbox, OS_DEL_ALWAYS, &ucErr);
    __GevtUsbCtrlSem      = OSSemDel(__GevtUsbCtrlSem, OS_DEL_ALWAYS, &ucErr);
    __GevtUsbDataTranSem  = OSSemDel(__GevtUsbDataTranSem, OS_DEL_ALWAYS, &ucErr);
    OS_EXIT_CRITICAL();

    return TRUE;
}

/*********************************************************************************************************
** Function name:       usbHostNotifyExitDataOpera
** Descriptions:        使USB 主机退出往从机写数据的操作
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL usbHostNotifyExitDataOpera (void)
{
    USB_INT32U i;

    OS_ENTER_CRITICAL();
    GusbHostFlags.bExitDataOperaReq = TRUE;
    OS_EXIT_CRITICAL();

    /*
     *  等待没有任务任务操作USB总线
     */
    i = 0;
    while (__GusbHostEvtCnt.ucDataTranCnt) {
        OSTimeDly(2);
        if (i++ > 1000) {                                               /*  等待10S                     */
            return FALSE;
        }
    }
    return TRUE;
}

/*********************************************************************************************************
** Function name:       usbHostNotifyExitStdOpera
** Descriptions:        使USB主机退出控制管道的操作
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL usbHostNotifyExitStdOpera (void)
{
    USB_INT32U i;

    OS_ENTER_CRITICAL();
    GusbHostFlags.bExitStdOperaReq = TRUE;
    OS_EXIT_CRITICAL();

    /*
     *  等待没有任务任务操作USB总线
     */
    i = 0;
    while (__GusbHostEvtCnt.ucStdTranCnt) {
        OSTimeDly(2);
        if (i++ > 1000) {                                               /*  等待10S                     */
            return FALSE;
        }
    }
    return TRUE;
}

/*********************************************************************************************************
** Function name:       usbPipeOpen
** Descriptions:        打开传输管道
** input parameters:    ucTranType  传输类型
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL usbPipeOpen (USB_INT8U ucTranType)
{
    USB_INT8U ucEpNum;
    USB_INT16U usMaxPktSize;

    if (ucTranType != USB_TRAN_TYPE_CONTROL) {
        ucEpNum = usbGetEp(0, ucTranType);
        usMaxPktSize = usbGetEpMaxPktSize(0, ucEpNum);                  /*  获取该端点的最大包长度      */
        if (usMaxPktSize == 0) {                                        /*  获取失败, 可能该端点不存在  */
            return FALSE;
        }
    }

    switch (ucTranType) {

    case USB_TRAN_TYPE_CONTROL:                                         /*  控制传输                    */
        GusbHostFlags.ucCtrlPipe = 1;
        __hcEnableSchedule(ucTranType);
        break;

    case USB_TRAN_TYPE_BULK_OUT:                                        /*  批量 OUT 传输               */
        if (GusbHostFlags.ucBulkOutPipe == 0) {
            __GpohciEd->edsBulkOut.uiControl = (usMaxPktSize << 16) | (1 << 14) | (__GucUsbSpeed << 13) |
                                               ((ucEpNum & 0x0F) << 7) | __GucUsbDevAddr;

            GusbHostFlags.ucBulkOutPipe = 1;
            __hcEnableSchedule(ucTranType);
        }
        break;

    case USB_TRAN_TYPE_BULK_IN:                                         /*  批量 IN 传输                */
        if (GusbHostFlags.ucBulkInPipe == 0) {
            __GpohciEd->edsBulkIn.uiControl = (usMaxPktSize << 16) | (1 << 14) | (__GucUsbSpeed << 13) |
                                              ((ucEpNum & 0x0F) << 7) | __GucUsbDevAddr;

            GusbHostFlags.ucBulkInPipe = 1;
            __hcEnableSchedule(ucTranType);
        }
        break;

    case USB_TRAN_TYPE_INTR_OUT:                                        /*  中断 OUT 传输               */
        if (GusbHostFlags.ucIntrOutPipe == 0) {
            __GpohciEd->edsIntrOut.uiControl = (usMaxPktSize << 16) | (1 << 14) | (__GucUsbSpeed << 13) |
                                               ((ucEpNum & 0x0F) << 7) | __GucUsbDevAddr;
            GusbHostFlags.ucIntrOutPipe = 1;
            __hcEnableSchedule(ucTranType);
        }
        break;

    case USB_TRAN_TYPE_INTR_IN:                                         /*  中断 IN 传输                */
        if (GusbHostFlags.ucIntrInPipe == 0) {
            __GpohciEd->edsIntrIn.uiControl = (usMaxPktSize << 16) | (1 << 14) | (__GucUsbSpeed << 13) |
                                              ((ucEpNum & 0x0F) << 7) | __GucUsbDevAddr;
            GusbHostFlags.ucIntrInPipe = 1;
            __hcEnableSchedule(ucTranType);
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

/*********************************************************************************************************
** Function name:       usbPipeClose
** Descriptions:        关闭传输管道
** input parameters:    ucTranType  传输类型
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL usbPipeClose (USB_INT8U ucTranType)
{
    __hcDisEnSchedAndWaitNextFrame(ucTranType);

    OS_ENTER_CRITICAL();

    switch (ucTranType) {

    case USB_TRAN_TYPE_CONTROL:
        __GpohciEd->edsControl.uiControl |= 1 << 14;
        GusbHostFlags.ucCtrlPipe = 0;
        break;

    case USB_TRAN_TYPE_BULK_OUT:
        __GpohciEd->edsBulkOut.uiControl |= 1 << 14;
        GusbHostFlags.ucBulkOutPipe = 0;
        break;

    case USB_TRAN_TYPE_BULK_IN:
        __GpohciEd->edsBulkIn.uiControl |= 1 << 14;
        GusbHostFlags.ucBulkInPipe = 0;
        break;

    case USB_TRAN_TYPE_INTR_OUT:
        __GpohciEd->edsIntrOut.uiControl |= 1 << 14;
        GusbHostFlags.ucIntrOutPipe = 0;
        break;

    case USB_TRAN_TYPE_INTR_IN:
        __GpohciEd->edsIntrIn.uiControl |= 1 << 14;
        GusbHostFlags.ucIntrInPipe = 0;
        break;

    default:
        break;
    }

    OS_EXIT_CRITICAL();

    return TRUE;
}

void __usbGetDevString (void);
/*********************************************************************************************************
** Function name:       __taskUsbEnum
** Descriptions:        uC/OS-II任务,负责对插入的设备进行枚举
** input parameters:    pdata   uC/OS-II参数
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __taskUsbEnum (void *pdata)
{
#define   STATIC_MALLOC_EN              1                               /*  使能(1)或禁用(0)静态分配    */

#if STATIC_MALLOC_EN
    USB_INT8U  ucBuf[64] = {0};
#else
    USB_INT8U  ucBuf[22] = {0};
#endif

    USB_INT8U  retryCnt, ucRet;
    USB_INT16U usConfigDescLen;
    USB_INT8U *pucTmp;

    retryCnt = 0;
    do {
        do {
            /*
             *  USB规范规定在检测到设备插入后至少要等待100mS才能开始枚举,此处等待200mS左右
             */
           	OSTimeDly(OS_TICKS_PER_SEC / 4);

          	if (GusbHostFlags.ucAttached == 0) {
           	    break;
           	}

           	usbBusReset();                                              /*  复位USB总线			        */
            OSTimeDly(OS_TICKS_PER_SEC / 20);

            __ohciEnableInt(__USB_INT_RHSC);                            /*  复位总线将会禁止连接改变中断*/
           	                                                            /*  故在复位后就重新使能该中断  */
           	USBDEBUG_SENDSTR("\r\n usbEnumerate...\r\n\r\n");

           	__hcDisEnSchedAndWaitNextFrame(USB_TRAN_TYPE_CONTROL);
           	__GpohciEd->edsControl.uiControl = (__GpohciEd->edsControl.uiControl & ~(1 << 13)) |
           	                                   (__GucUsbSpeed << 13);   /*  配置端点速度(全速或低速)    */
           	__hcEnableSchedule(USB_TRAN_TYPE_CONTROL);

           	ucRet = usbGetDeviceDescription(0x08, ucBuf);               /*  获取设备描述符的前8字节     */
           	if (ucRet != USB_ERR_SUCESS) {
                if (ucRet == USB_ERR_STALL) {
                    usbClearFeature_EP(0);
                }
                break;
           	}

           	pucTmp = (USB_INT8U *)&GusbDeviceDescr;
           	usbMemCopy(pucTmp, ucBuf, 0x08);                            /*  将获取到的数据填充到设备    */
           	                                                            /*  ...描述符的结构体变量中     */

           	if ((GusbDeviceDescr.bLength         != 18)                            ||
                (GusbDeviceDescr.bDescriptorType != __USB_DESCRIPTOR_TYPE_DEVICE)) {
           	    break;                                                  /*  获取到的设备描述符错误      */
           	}

           	__hcDisEnSchedAndWaitNextFrame(USB_TRAN_TYPE_CONTROL);
           	__GpohciEd->edsControl.uiControl = (__GpohciEd->edsControl.uiControl & ~(0x7f << 16)) |
           	                                   (GusbDeviceDescr.bMaxPacketSize0 << 16);
           	__hcEnableSchedule(USB_TRAN_TYPE_CONTROL);

           	ucRet = usbSetAddress(0x01);                                /*  设置从机地址: 0x01          */
           	if (ucRet == USB_ERR_SUCESS) {
                __GucUsbDevAddr = 0x01;
                __hcDisEnSchedAndWaitNextFrame(USB_TRAN_TYPE_CONTROL);
                __GpohciEd->edsControl.uiControl = (__GpohciEd->edsControl.uiControl & ~0xff)
                                                 |  __GucUsbDevAddr;
                __hcEnableSchedule(USB_TRAN_TYPE_CONTROL);
           	} else {
                if (ucRet == USB_ERR_STALL) {
                    usbClearFeature_EP(0);
                }
           	    break;
          	}

           	ucRet = usbGetDeviceDescription(18, ucBuf);                 /*  重新获取18字节的设备描述符  */
           	if (ucRet != USB_ERR_SUCESS) {
                if (ucRet == USB_ERR_STALL) {
                    usbClearFeature_EP(0);
                }
                break;
           	}
           	pucTmp = (USB_INT8U *)&GusbDeviceDescr;
           	usbMemCopy(pucTmp, ucBuf, 18);                              /*  将获取到的数据填充到设备    */
           	                                                            /*  ...描述符的结构体变量中     */
           	__usbGetDevString();                                        /*  获取产商字符串描述符和产品字*/
           	                                                            /*  符串描述符,并发送到调试接口 */
           	ucRet = usbGetConfigDescription(0, 0, 9, ucBuf);            /*  获取9字节的配置描述符       */
           	if (ucRet != USB_ERR_SUCESS) {
                if (ucRet == USB_ERR_STALL) {
                    usbClearFeature_EP(0);
                }
                break;
           	}
           	pucTmp = (USB_INT8U *)&GusbConfigDescr;
           	usbMemCopy(pucTmp, ucBuf, 0x09);                            /*  将获取到的数据填充到配置    */
           	                                                            /*  ...描述符的结构体变量中     */
           	usConfigDescLen = (USB_INT16U)(GusbConfigDescr.wTotalLength1 << 8);
           	usConfigDescLen = (USB_INT16U)(usConfigDescLen + GusbConfigDescr.wTotalLength0);
           	                                                            /*  计算总配置描述符长度        */
#if STATIC_MALLOC_EN
            pucTmp = ucBuf;
#else
           	pucTmp = NULL;
           	pucTmp = (USB_INT8U *)malloc(usConfigDescLen);
           	if (pucTmp == NULL) {
           	    break;
           	}
#endif
           	ucRet = usbGetConfigDescription(0, 0, usConfigDescLen, pucTmp);
           	                                                            /*  获取所有的设备描述符        */
           	if (ucRet != USB_ERR_SUCESS) {
                if (ucRet == USB_ERR_STALL) {
                    usbClearFeature_EP(0);
                }
                break;
           	} else {
           	    usbInterfaceEpConfig(pucTmp, usConfigDescLen);          /*  获取到的配置描述符中的接口  */
            }                                                           /*  ..描述符和端点描述符进行整理*/

#if STATIC_MALLOC_EN
#else
           	free(pucTmp);                                               /*  释放缓冲区                  */
#endif

            ucRet = usbSetConfiguratiton();                             /*  设置配置                    */
           	if (ucRet != USB_ERR_SUCESS) {
                if (ucRet == USB_ERR_STALL) {
                    usbClearFeature_EP(0);
                }
                break;
           	}
            GusbHostFlags.ucEnumed = 1;                                 /*  标志枚举成功                */
            USBDEBUG_SENDSTR("\r\n usbEnumerate sucess!\r\n");

            usbPipeOpen(USB_TRAN_TYPE_INTR_OUT);
            usbPipeOpen(USB_TRAN_TYPE_INTR_IN);
            usbPipeOpen(USB_TRAN_TYPE_BULK_OUT);
            usbPipeOpen(USB_TRAN_TYPE_BULK_IN);

        } while (0);

        if (GusbHostFlags.ucEnumed) {
            if (__GpEnumSucessCallBack) {
                (*__GpEnumSucessCallBack)();                            /*  执行枚举成功回调函数        */
            }
            break;
        }

    } while (retryCnt++ < 3);

    if (GusbHostFlags.ucEnumed == 0) {
        USBDEBUG_SENDSTR("\r\n usbEnumerate failed!\r\n");
    }

    USBDEBUG_SENDSTR("\r\n Delete usbEnumerate task!\r\n\r\n");
    OSTaskDel(OS_PRIO_SELF);

    while (1) {
        ;
    }
}

/*********************************************************************************************************
** Function name:       __usbGetDevString
** Descriptions:        获取供应商描述符和产品描述符,并发送到调试接口上
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __usbGetDevString (void)
{
    USB_INT8U  ucBuf[44];
    USB_INT16U usLanID, usLen;
    INT8U      i, ucRet;

   	ucRet = usbGetStringDescription(0, 0, 4, ucBuf);                    /*  获取4字节语言ID             */
   	if (ucRet != USB_ERR_SUCESS) {
        if (ucRet == USB_ERR_STALL) {
            usbClearFeature_EP(0);
        }
        return;
   	} else {
   	    usLanID = (USB_INT16U)(ucBuf[3] << 8);
   	    usLanID = (USB_INT16U)(usLanID | ucBuf[2]);                     /*  语言ID                      */
   	}

   	if (GusbDeviceDescr.iManufacturer) {
   	    ucBuf[1] = 0;
   	    ucRet = usbGetStringDescription(GusbDeviceDescr.iManufacturer,
   	                                    usLanID, 2, ucBuf);             /*  供应商字符串描述符前两字节  */
       	if (ucBuf[1] != __USB_DESCRIPTOR_TYPE_STRING) {
       	    return;
       	}
       	usLen = ucBuf[0];                                               /*  获取供应商字符串描述符长度  */
       	if (usLen > sizeof(ucBuf)) {
       	    usLen = sizeof(ucBuf);
       	}
       	ucRet = usbGetStringDescription(GusbDeviceDescr.iManufacturer,
   	                                    usLanID, usLen, ucBuf);         /*  获取供应商字符串描述符      */
       	if (ucRet != USB_ERR_SUCESS) {
            if (ucRet == USB_ERR_STALL) {
                usbClearFeature_EP(0);
            } else {
                return;
            }
       	} else {
       	    USBDEBUG_SENDSTR("\r\nManufacturer : ");
       	    if (usLanID == 0x0409) {
       	        for (i = 2; i < usLen; i += 2) {
       	            USBDEBUG_SENDCHAR(ucBuf[i]);
                    USBDEBUG_SENDCHAR(ucBuf[i + 1]);
                }
            } else {
                for (i = 2; i < usLen; i++) {
       	            USBDEBUG_SENDCHAR(ucBuf[i]);
                }
            }
       	}
    }

   	if (GusbDeviceDescr.iProduct) {
   	    ucBuf[1] = 0;
   	    ucRet = usbGetStringDescription(GusbDeviceDescr.iProduct,
   	                                    usLanID, 2, ucBuf);             /*  获取产品字符串描述符前两字节*/
       	if (ucBuf[1] != __USB_DESCRIPTOR_TYPE_STRING) {
       	    return;
       	}
       	usLen = ucBuf[0];                                               /*  获取产品字符串描述符长度  */
       	if (usLen > sizeof(ucBuf)) {
       	    usLen = sizeof(ucBuf);
       	}
       	ucRet = usbGetStringDescription(GusbDeviceDescr.iProduct,
   	                                    usLanID, usLen, ucBuf);         /*  获取供应商字符串描述符      */
       	if (ucRet != USB_ERR_SUCESS) {
            if (ucRet == USB_ERR_STALL) {
                usbClearFeature_EP(0);
            } else {
                return;
            }
       	} else {
       	    USBDEBUG_SENDSTR("\r\nProduct : ");
       	    if (usLanID == 0x0409) {
       	        for (i = 2; i < usLen; i += 2) {
       	            USBDEBUG_SENDCHAR(ucBuf[i]);
                    USBDEBUG_SENDCHAR(ucBuf[i + 1]);
                }
            } else {
                for (i = 2; i < usLen; i++) {
       	            USBDEBUG_SENDCHAR(ucBuf[i]);
                }
            }
       	}
    }
}



/*********************************************************************************************************
  以下为 USB 协议中的标准请求
*********************************************************************************************************/

/*********************************************************************************************************
** Function name:       usbGetStatus
** Descriptions:        读取状态请求
** input parameters:    ucType  要读取的状态的对象类型,指设备(0),接口(1),或端点(2)
**                      wIndex  索引,设备号(固定为0),或接口号,或端点号
** output parameters:   pucData 接收数据缓冲区,用于存放读取到的状态值
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbGetStatus (USB_INT8U ucType , USB_INT16U wIndex, USB_INT8U *pucData)
{
    USB_DEV_REQ usbDeviceRequest;

    USBDEBUG_SENDSTR("...usbGetStatus\r\n");

    usbMemSet(&usbDeviceRequest, 0, sizeof (USB_DEV_REQ));              /*  清零usbDeviceRequest        */

    usbDeviceRequest.bmRequestType = (USB_INT8U)(__USB_DEVICE_TO_HOST | __USB_STANDARD_REQUEST | ucType);
                                                                        /*  请求类型                    */
    usbDeviceRequest.bRequest      = __USB_REQUEST_GET_STATUS;          /*  请求号                      */
    usbDeviceRequest.wValue        = 0;
    usbDeviceRequest.wIndex        = wIndex;
    usbDeviceRequest.wLength       = 2;                                 /*  返回的数据长度固定为2字节   */

    return usbStandardReqTransfer(&usbDeviceRequest, pucData);          /*  发送请求,并取得设备返回值   */

}

/*********************************************************************************************************
** Function name:       usbClearFeature
** Descriptions:        清除特性
** input parameters:    ucType  要读取的状态的对象类型,指设备(0),接口(1),或端点(2)
**                      wValue  特性选择符
**                      wIndex  索引,设备号(固定为0),或接口号,或端点号
** output parameters:   None
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbClearFeature (USB_INT8U ucType, USB_INT16U wValue, USB_INT16U wIndex)
{
    USB_DEV_REQ usbDeviceRequest;

    USBDEBUG_SENDSTR("...usbClearFeature\r\n");

    usbMemSet(&usbDeviceRequest, 0, sizeof (USB_DEV_REQ));

    usbDeviceRequest.bmRequestType = (USB_INT8U)(__USB_HOST_TO_DEVICE | __USB_STANDARD_REQUEST | ucType);
    usbDeviceRequest.bRequest      = __USB_REQUEST_CLEAR_FEATURE;
    usbDeviceRequest.wValue        = wValue;
    usbDeviceRequest.wIndex        = wIndex;
    usbDeviceRequest.wLength       = 0;

    return usbStandardReqTransfer(&usbDeviceRequest, NULL);
}

/*********************************************************************************************************
** Function name:       usbSetFeature
** Descriptions:        设置特性
** input parameters:    ucType  要读取的状态的对象类型,指设备(0),接口(1),或端点(2)
**                      wValue  特性选择符
**                      wIndex  索引,设备号(固定为0),或接口号,或端点号
** output parameters:   None
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbSetFeature (USB_INT8U ucType, USB_INT16U wValue, USB_INT16U wIndex)
{
    USB_DEV_REQ usbDeviceRequest;

    USBDEBUG_SENDSTR("...usbSetFeature\r\n");

    usbMemSet(&usbDeviceRequest, 0, sizeof (USB_DEV_REQ));

    usbDeviceRequest.bmRequestType = (USB_INT8U)(__USB_HOST_TO_DEVICE | __USB_STANDARD_REQUEST | ucType);
    usbDeviceRequest.bRequest      = __USB_REQUEST_SET_FEATURE;
    usbDeviceRequest.wValue        = wValue;
    usbDeviceRequest.wIndex        = wIndex;
    usbDeviceRequest.wLength       = 0;

    return usbStandardReqTransfer(&usbDeviceRequest, NULL);
}

/*********************************************************************************************************
** Function name:       usbSetAddress
** Descriptions:        设置地址
** input parameters:    wValue 地址值
** output parameters:   None
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbSetAddress (USB_INT16U wValue)
{
    USB_DEV_REQ usbDeviceRequest;

    USBDEBUG_SENDSTR("...usbSetAddress: Address: ");
    USBDEBUG_SENDCHAR((USB_INT8U)(wValue + '0'));
    USBDEBUG_SENDSTR("\r\n");

    usbMemSet(&usbDeviceRequest, 0, sizeof (USB_DEV_REQ));

    usbDeviceRequest.bmRequestType = (USB_INT8U)(__USB_HOST_TO_DEVICE | __USB_STANDARD_REQUEST |
                                      __USB_RECIPIENT_DEVICE);
    usbDeviceRequest.bRequest      = __USB_REQUEST_SET_ADDRESS;
    usbDeviceRequest.wValue        = wValue;
    usbDeviceRequest.wIndex        = 0;
    usbDeviceRequest.wLength       = 0;

    return usbStandardReqTransfer(&usbDeviceRequest, NULL);
}


/*********************************************************************************************************
** Function name:       usbGetDescription
** Descriptions:        获取描述符
** input parameters:    wValue  类型和索引
**                      wIndex  0 或语言 ID
**                      wLength 描述符长度
** output parameters:   pucData 接收描述符的缓冲区
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbGetDescription (USB_INT16U wValue,
                             USB_INT16U wIndex,
                             USB_INT16U wLength,
                             USB_INT8U *pucData)
{

    USB_DEV_REQ usbDeviceRequest;




    USBDEBUG_SENDSTR("\r\n...usbGetDescription.  wValue: ");
    USBDEBUG_SENDCHAR((USB_INT8U)((wValue >> 8) + '0'));
    USBDEBUG_SENDSTR("\r\n");

    usbMemSet(&usbDeviceRequest, 0, sizeof (USB_DEV_REQ));

    usbDeviceRequest.bmRequestType = (USB_INT8U)(__USB_DEVICE_TO_HOST | __USB_STANDARD_REQUEST |
                                      __USB_RECIPIENT_DEVICE);
    usbDeviceRequest.bRequest      = __USB_REQUEST_GET_DESCRIPTOR;
    usbDeviceRequest.wValue        = wValue;
    usbDeviceRequest.wIndex        = wIndex;
    usbDeviceRequest.wLength       = wLength;

    return usbStandardReqTransfer(&usbDeviceRequest, pucData);
}

/*********************************************************************************************************
** Function name:       usbSetDescription
** Descriptions:        设置描述符
** input parameters:    wValue  类型和索引
**                      wIndex  0 或语言ID
**                      wLength 描述符长度
** output parameters:   pucData 接收描述符的缓冲区
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbSetDescription (USB_INT16U wValue,
                             USB_INT16U wIndex,
                             USB_INT16U wLength,
                             USB_INT8U *pucData)
{

    USBDEBUG_SENDSTR("...usbSetDescription\r\n");

    return USB_ERR_SOFT_NOT_SUPPORT;
}

/*********************************************************************************************************
** Function name:       usbGetConfiguratiton
** Descriptions:        读取配置值请求
** input parameters:    None
** output parameters:   pucData 配置值
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbGetConfiguratiton (USB_INT8U *pucData)
{
    USB_DEV_REQ usbDeviceRequest;

    USBDEBUG_SENDSTR("...usbGetConfiguratiton\r\n");

    usbMemSet(&usbDeviceRequest, 0, sizeof (USB_DEV_REQ));

    usbDeviceRequest.bmRequestType = (USB_INT8U)(__USB_DEVICE_TO_HOST | __USB_STANDARD_REQUEST |
                                      __USB_RECIPIENT_DEVICE);
    usbDeviceRequest.bRequest      = __USB_REQUEST_GET_CONFIGURATION;
    usbDeviceRequest.wValue        = 0;
    usbDeviceRequest.wIndex        = 0;
    usbDeviceRequest.wLength       = 1;

    return usbStandardReqTransfer(&usbDeviceRequest, pucData);
}

/*********************************************************************************************************
** Function name:       usbSetConfiguratiton
** Descriptions:        设置配置值请求
** input parameters:    None
** output parameters:   None
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbSetConfiguratiton (void)
{
    USB_INT8U          ucErrCode;
    USB_DEV_REQ usbDeviceRequest;

    USBDEBUG_SENDSTR("...usbSetConfiguratiton :  ");

    usbMemSet(&usbDeviceRequest, 0, sizeof (USB_DEV_REQ));

    usbDeviceRequest.bmRequestType = (USB_INT8U)(__USB_HOST_TO_DEVICE
                                   |             __USB_STANDARD_REQUEST
                                   |             __USB_RECIPIENT_DEVICE);

    usbDeviceRequest.bRequest      = __USB_REQUEST_SET_CONFIGURATION;
    usbDeviceRequest.wValue        = GusbConfigDescr.bConfigurationValue;
    usbDeviceRequest.wIndex        = 0;
    usbDeviceRequest.wLength       = 0;

    ucErrCode = usbStandardReqTransfer(&usbDeviceRequest, NULL);
    if (ucErrCode) {
        USBDEBUG_SENDSTR("...usbSetConfiguratiton failed!");
        return ucErrCode;
    }

    USBDEBUG_SENDCHAR((USB_INT8U)(usbDeviceRequest.wValue + '0'));
    USBDEBUG_SENDSTR("\r\n");

    GusbHostFlags.ucConfiged = 1;                                       /*  标记设备已配置              */

    return USB_ERR_SUCESS;
}


/*********************************************************************************************************
** Function name:       usbGetInterface
** Descriptions:        读取指定接口的设置值,即接口描述符中的bAlternateSetting字段值
** input parameters:    wIndex  接口号
** output parameters:   pucData 返回的bAlternateSetting字段值
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbGetInterface (USB_INT16U wIndex, USB_INT8U *pucData)
{
    /*
     *  此函数尚未测试
     */
    USB_DEV_REQ usbDeviceRequest;

    USBDEBUG_SENDSTR("...usbGetInterface\r\n");

    usbMemSet(&usbDeviceRequest, 0, sizeof (USB_DEV_REQ));

    usbDeviceRequest.bmRequestType = (USB_INT8U)(__USB_DEVICE_TO_HOST | __USB_STANDARD_REQUEST |
                                      __USB_RECIPIENT_DEVICE);
    usbDeviceRequest.bRequest      = __USB_REQUEST_GET_INTERFACE;
    usbDeviceRequest.wValue        = 0;
    usbDeviceRequest.wIndex        = wIndex;
    usbDeviceRequest.wLength       = 1;

    return usbStandardReqTransfer(&usbDeviceRequest, pucData);
}

/*********************************************************************************************************
** Function name:       usbSetInterface
** Descriptions:        设置接口请求,与usbGetInterface相对应
** input parameters:    wValue  可替换的设置值
**                      wIndex  接口号
** output parameters:   None
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbSetInterface (USB_INT16U wValue, USB_INT16U wIndex)
{
    /*
     *  此函数尚未测试
     */
    USB_DEV_REQ usbDeviceRequest;

    if (GusbHostFlags.ucConfiged == 0) {                                /*  仅在USB设备处在配置状态时   */
        return FALSE;                                                   /*  ...才能设置接口             */
    }

    USBDEBUG_SENDSTR("...usbSetInterface\r\n");

    usbMemSet(&usbDeviceRequest, 0, sizeof (USB_DEV_REQ));

    usbDeviceRequest.bmRequestType = (USB_INT8U)(__USB_HOST_TO_DEVICE | __USB_STANDARD_REQUEST |
                                      __USB_RECIPIENT_INTERFACE);
    usbDeviceRequest.bRequest      = __USB_REQUEST_SET_INTERFACE;
    usbDeviceRequest.wValue        = wValue;
    usbDeviceRequest.wIndex        = wIndex;
    usbDeviceRequest.wLength       = 0;

    return usbStandardReqTransfer(&usbDeviceRequest, NULL);
}

/*********************************************************************************************************
** Function name:       usbSunchFrame
** Descriptions:        同步帧请求,用于设置并报告端点的同步帧号
** input parameters:    wIndex  端点号
** output parameters:   pucData 返回的帧号
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_INT8U usbSunchFrame (USB_INT16U wIndex, USB_INT8U *pucData)
{
    return USB_ERR_SOFT_NOT_SUPPORT;                                    /*  当前版本不支持同步传输      */
}


/*********************************************************************************************************
** Function name:       usbInterfaceEpConfig
** Descriptions:        根据获取到的配置描述符数据找出接口描述符及其端点描述符,
**                      并将其填充到相应的描述符结构体中
** input parameters:    pucBuf          获取到的配置描述符数据缓冲区
**                      usConfigDescLen 配置描述符总长度
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL usbInterfaceEpConfig (USB_INT8U *pucBuf, USB_INT16U usConfigDescLen)
{
    USB_INT8U  ucEpNum;
    USB_INT16U usTmp;
    USB_INT16U usCount;
    USB_INT16U usMaxPktSize;
    USB_INT8U *pucTmp;

    USB_INT8   cInterfaceIndex;
    USB_INT8U  ucBulkInIndex, ucBulkOutIndex;
    USB_INT8U  ucIntrInIndex, ucIntrOutIndex;
    USB_INT8U  ucIsoInIndex, ucIsoOutIndex;

    USBDEBUG_SENDSTR("\t...usbInterfaceEpConfig\r\n");

   	/*
   	 *  获取接口和端点的描述符信息
   	 */
   	cInterfaceIndex = -1;
   	usCount = 9;                                                        /*  跳过9字节基本的配置描述符   */
   	do {
   	    usTmp = *(pucBuf + usCount + 1);                                /*  描述符类型                  */
   	    if (usTmp == __USB_DESCRIPTOR_TYPE_INTERFACE) {                 /*  接口描述符                  */
   	        cInterfaceIndex++;
   	        GusbInterEpDescr[cInterfaceIndex].ucInterfaceIndex = cInterfaceIndex;
   	        if (cInterfaceIndex >= __USB_MAX_INTERFACE) {               /*  含有的接口描述符已超过了    */
   	            break;                                                  /*  ...预定的接口描述符数       */
   	        }

   	        /*
   	         *  找到第cInterfaceIndex个接口
   	         */
   	        pucTmp = (USB_INT8U *)&(GusbInterEpDescr[cInterfaceIndex].usbInterfaceDescr);
   	        usbMemCopy(pucTmp, pucBuf + usCount, sizeof(USB_INTERFACE_DESCRIPTOR));

      	    /*
      	     *  清零该接口的各端点计数器,以方便对下一接口的端点数进行计数
      	     */
      	    ucBulkInIndex  = 0;
   	        ucBulkOutIndex = 0;
   	        ucIntrInIndex  = 0;
   	        ucIntrOutIndex = 0;
   	        ucIsoInIndex   = 0;
   	        ucIsoOutIndex  = 0;

	    } else if (usTmp == __USB_DESCRIPTOR_TYPE_ENDPOINT) {           /*  端点描述符                  */
	        if (cInterfaceIndex < 0) {                                  /*  尚未找到接口描述符          */
	            return FALSE;
	        }

   	        if ( (*(pucBuf + usCount + 3) & 0x03) == 0x01) {            /*  同步传输                    */
   	            if ( (*(pucBuf + usCount + 2) & (USB_INT8U)0x80) != 0) {/*  IN 端点                     */
   	                pucTmp = (USB_INT8U *)&GusbInterEpDescr[cInterfaceIndex].\
   	                                       usbIsoInEpDescr[ucIsoInIndex++];
   	                GusbInterEpDescr[cInterfaceIndex].ucIsoInEpNum++;
   	            } else {                                                /*  OUT 端点                    */
   	                pucTmp = (USB_INT8U *)&GusbInterEpDescr[cInterfaceIndex].\
   	                                       usbIsoOutEpDescr[ucIsoOutIndex++];
   	                GusbInterEpDescr[cInterfaceIndex].ucIsoOutEpNum++;
   	            }
   	        } else if ( (*(pucBuf + usCount + 3) & 0x03) == 0x02) {     /*  块传输                      */
   	            if ( (*(pucBuf + usCount + 2) & (USB_INT8U)0x80) != 0) {/*  IN 端点                     */
   	                pucTmp = (USB_INT8U *)&GusbInterEpDescr[cInterfaceIndex].\
   	                                       usbBulkInEpDescr[ucBulkInIndex++];
   	                GusbInterEpDescr[cInterfaceIndex].ucBulkInEpNum++;
   	            } else {                                                /*  OUT 端点                    */
   	                pucTmp = (USB_INT8U *)&GusbInterEpDescr[cInterfaceIndex].\
   	                                       usbBulkOutEpDescr[ucBulkOutIndex++];
   	                GusbInterEpDescr[cInterfaceIndex].ucBulkOutEpNum++;
   	            }
   	        } else if ( (*(pucBuf + usCount + 3) & 0x03) == 0x03) {     /*  中断传输                    */
   	            if ( (*(pucBuf + usCount + 2) & (USB_INT8U)0x80) != 0) {/*  IN 端点                     */
   	                pucTmp = (USB_INT8U *)&GusbInterEpDescr[cInterfaceIndex].\
   	                                       usbIntrInEpDescr[ucIntrInIndex++];
   	                GusbInterEpDescr[cInterfaceIndex].ucIntrInEpNum++;
   	            } else {                                                /*  OUT 端点                    */
   	                pucTmp = (USB_INT8U *)&GusbInterEpDescr[cInterfaceIndex].\
   	                                       usbIntrOutEpDescr[ucIntrOutIndex++];
   	                GusbInterEpDescr[cInterfaceIndex].ucIntrOutEpNum++;
   	            }
   	        }
   	        usbMemCopy(pucTmp, pucBuf + usCount, sizeof(USB_ENDPOINT_DESCRIPTOR));
   	                                                                    /*  将数据填充到相应的端点描述符*/
   	                                                                    /*  ...结构体变量               */
            ucEpNum = *(pucBuf + usCount + 2);
            ucEpNum = (USB_INT8U)(((ucEpNum >> 3) | ucEpNum) & 0x1F);
            usMaxPktSize = (USB_INT16U)(*(pucBuf + usCount + 5) << 8);
            usMaxPktSize = (USB_INT16U)(usMaxPktSize + *(pucBuf + usCount + 4));
            __GusEpMaxPktSize[cInterfaceIndex][ucEpNum] = usMaxPktSize;

        } else if (usTmp == __USB_DESCRIPTOR_TYPE_OTG) {                /*  OTG描述符                   */
            pucTmp = (USB_INT8U *)&GusbOtgDescr;
            usbMemCopy(pucTmp, pucBuf + usCount, sizeof(USB_OTG_DESCRIPTOR));

        } else {                                                        /*  对其它类型描述符跳过,不关心 */
            ;
        }
        usCount = (USB_INT16U)(usCount + *(pucBuf + usCount));          /*  更新计数器                  */
    } while (usCount < usConfigDescLen);

    if (cInterfaceIndex < 0) {                                          /*  未找到接口描述符            */
        return FALSE;
    }

    /*
     *  将所具有的接口描述符总数写到每个接口描述符结构体变量的ucInterfaceNum字段中
     */
    for (usCount = 0; usCount <= cInterfaceIndex; usCount++) {
        GusbInterEpDescr[usCount].ucInterfaceNum = (USB_INT8U)(cInterfaceIndex + 1);
    }

    return TRUE;
}

/*********************************************************************************************************
** Function name:       usbGetEpMaxPktSize
** Descriptions:        获取端点的最大包大小
** input parameters:    ucInterfaceIndex 接口索引
**                      ucEpNum          端点号
** output parameters:   None
** Returned value:      > 0 : 所要端点的最大包大小,  0 : 失败,表示端点ucEpNum不存在
*********************************************************************************************************/
USB_INT16U usbGetEpMaxPktSize (USB_INT8U ucInterfaceIndex, USB_INT8U ucEpNum)
{
    ucEpNum = (USB_INT8U)(((ucEpNum >> 3) | ucEpNum) & 0x1F);

    return __GusEpMaxPktSize[ucInterfaceIndex][ucEpNum];
}

/*********************************************************************************************************
** Function name:       usbIsDeviceReady
** Descriptions:        判断设备是否准备好
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 设备准备好   FALSE : 设备未准备好
*********************************************************************************************************/
USB_BOOL usbIsDeviceReady (void)
{
    return (USB_BOOL)(GusbHostFlags.ucEnumed == 1);
}

/*********************************************************************************************************
** Function name:       usbIsDeviceAttach
** Descriptions:        判断设备是否插入
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 设备已插入   FALSE : 设备未准备插入
*********************************************************************************************************/
USB_BOOL usbIsDeviceAttach (void)
{
    return (USB_BOOL)(GusbHostFlags.ucAttached == 1);
}

/*********************************************************************************************************
** Function name:       usbGetDeviceType
** Descriptions:        获取设备类型信息,包括设备类型,所支持协议等
** input parameters:    ucInterfaceIndex 接口号
** output parameters:   pusbDeviceType   设备类型
** Returned value:      TRUE : 成功   FALSE : 失败
*********************************************************************************************************/
USB_BOOL usbGetDeviceType (USB_INT8U ucInterfaceIndex, PUSB_DEVICE_TYPE pusbDeviceType)
{
    if (!usbIsDeviceReady()) {
        return FALSE;
    }

    pusbDeviceType->bDeviceClass    = GusbDeviceDescr.bDeviceClass;
    pusbDeviceType->bDeviceProtocol = GusbDeviceDescr.bDeviceProtocol;

    pusbDeviceType->bInterfaceClass = GusbInterEpDescr[ucInterfaceIndex].\
                                      usbInterfaceDescr.bInterfaceClass;
    pusbDeviceType->bInterfaceProtocol = GusbInterEpDescr[ucInterfaceIndex].\
                                         usbInterfaceDescr.bInterfaceProtocol;
    pusbDeviceType->bInterfaceSubClass = GusbInterEpDescr[ucInterfaceIndex].\
                                         usbInterfaceDescr.bInterfaceSubClass;

    return TRUE;
}

/*********************************************************************************************************
** Function name:       usbGetEp
** Descriptions:        获取非控制端点的端点号
** input parameters:    ucInterface 接口号
**                      ucTranType  传输类型
** output parameters:   None
** Returned value:      > 0 : 端点号,  = 0 : 获取失败,可能是设备不存在该类型端点
*********************************************************************************************************/
USB_INT8U usbGetEp (USB_INT8U ucInterface, USB_INT8U ucTranType)
{
    USB_INT8U ucEpNum = 0;

    switch (ucTranType) {

    case USB_TRAN_TYPE_BULK_OUT:                                        /*  Bulk OUT 传输               */
        ucEpNum = GusbInterEpDescr[ucInterface].usbBulkOutEpDescr[0].bEndpointAddress;
        break;

    case USB_TRAN_TYPE_BULK_IN:                                         /*  Bulk IN 传输                */
        ucEpNum = GusbInterEpDescr[ucInterface].usbBulkInEpDescr[0].bEndpointAddress;
        break;

    case USB_TRAN_TYPE_INTR_OUT:                                        /*  Intrrupt OUT  传输          */
        ucEpNum = GusbInterEpDescr[ucInterface].usbIntrOutEpDescr[0].bEndpointAddress;
        break;

    case USB_TRAN_TYPE_INTR_IN:                                         /*  Intrrupt IN 传输            */
        ucEpNum = GusbInterEpDescr[ucInterface].usbIntrInEpDescr[0].bEndpointAddress;
        break;

    default:                                                            /*  不支持的传输类型            */
        ucEpNum = 0;
    }

    return ucEpNum;
}

/*********************************************************************************************************
** Function name:       usbGetVer
** Descriptions:        取得USB协议栈的版本号
** input parameters:    None
** output parameters:   None
** Returned value:      32位版本号
*********************************************************************************************************/
INT32U usbGetVer (void)
{
#define __CPU_TYPE          3200

#define __USB_TYPE_DEVICE   0
#define __USB_TYPE_HOST     1U
#define __USB_TYPE_OTG      2U

#define __USB_TYPE          __USB_TYPE_HOST

#define __USB_VERSION       0x0110

	return (__USB_TYPE_HOST << 30) | (__CPU_TYPE << 16) | __USB_VERSION;/*  主机协议栈,for LPC3250,     */
														                /*  ...版本 0x0110              */
}

#ifdef __cplusplus
 }
#endif

/*********************************************************************************************************
** Function name:       __usbdInit
** Descriptions:        USBD层的环境初始化
** input parameters:    pEnumSucessCallBack: 枚举成功时回调函数
**                      pDevDisconCallBack:  设备拨出时回调函数
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __usbdInit (void (*pEnumSucessCallBack)(void), void (*pDevDisconCallBack)(void))
{
    __GucUsbDevAddr                = 0;

    __GusbHostEvtCnt.ucDataTranCnt = 0;
    __GusbHostEvtCnt.ucStdTranCnt  = 0;

    GusbHostFlags.bExitDataOperaReq = FALSE;
    GusbHostFlags.bExitStdOperaReq  = FALSE;

    __GpEnumSucessCallBack = pEnumSucessCallBack;
    __GpDevDisconCallBack  = pDevDisconCallBack;

    usbMemSet(&GusbDeviceDescr, 0, sizeof(GusbDeviceDescr));
    usbMemSet(&GusbConfigDescr, 0, sizeof(GusbConfigDescr));
    usbMemSet(&GusbOtgDescr, 0, sizeof(GusbOtgDescr));
    usbMemSet(&GusbInterEpDescr, 0, sizeof(GusbInterEpDescr));
    usbMemSet(&__GusEpMaxPktSize, 0, sizeof(__GusEpMaxPktSize));
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
