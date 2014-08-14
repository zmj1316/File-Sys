/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            usbd.c
** Latest modified Date: 2007-11-15
** Latest Version:       V1.0
** Description:          USBD���ʵ��
**--------------------------------------------------------------------------------------------------------
** Created By:           ��¡��  Longsui Wu
** Created date:         2007-11-15
** Version:              V1.0
** Descriptions:         ��ʼ�汾
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

USB_DEVICE_DESCRIPTOR         GusbDeviceDescr;                          /*  �豸������                  */
USB_CONFIGURATION_DESCRIPTOR  GusbConfigDescr;                          /*  ����������                  */
USB_OTG_DESCRIPTOR            GusbOtgDescr;

__USB_INTERFACE_EP_DESCR      GusbInterEpDescr[__USB_MAX_INTERFACE];    /*  ����������:�ӿ�����������   */
                                                                        /*  ...�ӿ������еĶ˵�������� */
USB_INT16U                    __GusEpMaxPktSize[__USB_MAX_INTERFACE][32] = {0};

volatile __USB_HOST_FLAGS     GusbHostFlags;                            /*  ��־λ                      */
__USB_HOST_EVENT_CNT          __GusbHostEvtCnt;
INT8U                         __GucHostShedPrio = 0;
INT8U                         __GucHostEnumPrio = 0;

void                          (*__GpEnumSucessCallBack)(void) = NULL;   /*  ö�ٳɹ�ʱ�ص�����          */
void                          (*__GpDevDisconCallBack)(void)  = NULL;   /*  �豸����ʱ�ص�����          */

void __usbdInit (void (*pEnumSucessCallBack)(void), void (*pDevDisconCallBack)(void));

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************************
** Function name:       usbHostInitialize
** Descriptions:        USB ������ʼ��
** input parameters:    ucShedPrio           ������������ȼ�
**                      ucEnumPrio           ö����������ȼ�
**                      pEnumSucessCallBack: ö�ٳɹ��ص�����,��û��������ΪNULL
**                      pDevDisconCallBack:  �豸�����ص�����,��û��������ΪNULL
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
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

    if (!__usbCreateShedTask(ucShedPrio, ucEnumPrio)) {                 /*  ������������                */
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
** Descriptions:        USB ����ж��
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
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
     *  �ȴ�û�������������USB����
     */
    i = 0;
    while (__GusbHostEvtCnt.ucDataTranCnt || __GusbHostEvtCnt.ucStdTranCnt) {
        OSTimeDly(2);
        if (i++ > 1000) {                                               /*  �ȴ�10S                     */
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
     *  �鿴ö�������Ƿ����,������,��ɾ��������
     */
    if (__GucHostEnumPrio) {
        ptcb = OSTCBPrioTbl[__GucHostEnumPrio];                         /*  ��ѯö�������Ƿ��Ѿ�����    */
        if (ptcb != NULL) {                                             /*  ɾ��ö������                */
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
     *  �鿴���������Ƿ����,������,��ɾ��������
     */
    if (__GucHostShedPrio) {
        ptcb = OSTCBPrioTbl[__GucHostShedPrio];
        if (ptcb != NULL) {
            OSQPost(__GevtUsbMsgQeue, (void *)(__USB_SHED_DELSELF << 24));
                                                                        /*  ���������Ѵ���,������Ϣ     */
                                                                        /*  Ҫ�������ɾ���Լ�          */
            while (OSTCBPrioTbl[__GucHostShedPrio] != NULL) {
                OSTimeDly(1);
            }
            OS_ENTER_CRITICAL();
            __GucHostShedPrio = 0;
            OS_EXIT_CRITICAL();
        }
    }

    OS_ENTER_CRITICAL();
    __GevtUsbMsgQeue = OSQDel(__GevtUsbMsgQeue, OS_DEL_ALWAYS, &ucErr); /*  ɾ��������Ϣ����            */
    __GevtUsbCtrlMbox     = OSMboxDel(__GevtUsbCtrlMbox, OS_DEL_ALWAYS, &ucErr);
    __GevtUsbDataTranMbox = OSMboxDel(__GevtUsbDataTranMbox, OS_DEL_ALWAYS, &ucErr);
    __GevtUsbCtrlSem      = OSSemDel(__GevtUsbCtrlSem, OS_DEL_ALWAYS, &ucErr);
    __GevtUsbDataTranSem  = OSSemDel(__GevtUsbDataTranSem, OS_DEL_ALWAYS, &ucErr);
    OS_EXIT_CRITICAL();

    return TRUE;
}

/*********************************************************************************************************
** Function name:       usbHostNotifyExitDataOpera
** Descriptions:        ʹUSB �����˳����ӻ�д���ݵĲ���
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL usbHostNotifyExitDataOpera (void)
{
    USB_INT32U i;

    OS_ENTER_CRITICAL();
    GusbHostFlags.bExitDataOperaReq = TRUE;
    OS_EXIT_CRITICAL();

    /*
     *  �ȴ�û�������������USB����
     */
    i = 0;
    while (__GusbHostEvtCnt.ucDataTranCnt) {
        OSTimeDly(2);
        if (i++ > 1000) {                                               /*  �ȴ�10S                     */
            return FALSE;
        }
    }
    return TRUE;
}

/*********************************************************************************************************
** Function name:       usbHostNotifyExitStdOpera
** Descriptions:        ʹUSB�����˳����ƹܵ��Ĳ���
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL usbHostNotifyExitStdOpera (void)
{
    USB_INT32U i;

    OS_ENTER_CRITICAL();
    GusbHostFlags.bExitStdOperaReq = TRUE;
    OS_EXIT_CRITICAL();

    /*
     *  �ȴ�û�������������USB����
     */
    i = 0;
    while (__GusbHostEvtCnt.ucStdTranCnt) {
        OSTimeDly(2);
        if (i++ > 1000) {                                               /*  �ȴ�10S                     */
            return FALSE;
        }
    }
    return TRUE;
}

/*********************************************************************************************************
** Function name:       usbPipeOpen
** Descriptions:        �򿪴���ܵ�
** input parameters:    ucTranType  ��������
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL usbPipeOpen (USB_INT8U ucTranType)
{
    USB_INT8U ucEpNum;
    USB_INT16U usMaxPktSize;

    if (ucTranType != USB_TRAN_TYPE_CONTROL) {
        ucEpNum = usbGetEp(0, ucTranType);
        usMaxPktSize = usbGetEpMaxPktSize(0, ucEpNum);                  /*  ��ȡ�ö˵����������      */
        if (usMaxPktSize == 0) {                                        /*  ��ȡʧ��, ���ܸö˵㲻����  */
            return FALSE;
        }
    }

    switch (ucTranType) {

    case USB_TRAN_TYPE_CONTROL:                                         /*  ���ƴ���                    */
        GusbHostFlags.ucCtrlPipe = 1;
        __hcEnableSchedule(ucTranType);
        break;

    case USB_TRAN_TYPE_BULK_OUT:                                        /*  ���� OUT ����               */
        if (GusbHostFlags.ucBulkOutPipe == 0) {
            __GpohciEd->edsBulkOut.uiControl = (usMaxPktSize << 16) | (1 << 14) | (__GucUsbSpeed << 13) |
                                               ((ucEpNum & 0x0F) << 7) | __GucUsbDevAddr;

            GusbHostFlags.ucBulkOutPipe = 1;
            __hcEnableSchedule(ucTranType);
        }
        break;

    case USB_TRAN_TYPE_BULK_IN:                                         /*  ���� IN ����                */
        if (GusbHostFlags.ucBulkInPipe == 0) {
            __GpohciEd->edsBulkIn.uiControl = (usMaxPktSize << 16) | (1 << 14) | (__GucUsbSpeed << 13) |
                                              ((ucEpNum & 0x0F) << 7) | __GucUsbDevAddr;

            GusbHostFlags.ucBulkInPipe = 1;
            __hcEnableSchedule(ucTranType);
        }
        break;

    case USB_TRAN_TYPE_INTR_OUT:                                        /*  �ж� OUT ����               */
        if (GusbHostFlags.ucIntrOutPipe == 0) {
            __GpohciEd->edsIntrOut.uiControl = (usMaxPktSize << 16) | (1 << 14) | (__GucUsbSpeed << 13) |
                                               ((ucEpNum & 0x0F) << 7) | __GucUsbDevAddr;
            GusbHostFlags.ucIntrOutPipe = 1;
            __hcEnableSchedule(ucTranType);
        }
        break;

    case USB_TRAN_TYPE_INTR_IN:                                         /*  �ж� IN ����                */
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
** Descriptions:        �رմ���ܵ�
** input parameters:    ucTranType  ��������
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
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
** Descriptions:        uC/OS-II����,����Բ�����豸����ö��
** input parameters:    pdata   uC/OS-II����
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __taskUsbEnum (void *pdata)
{
#define   STATIC_MALLOC_EN              1                               /*  ʹ��(1)�����(0)��̬����    */

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
             *  USB�淶�涨�ڼ�⵽�豸���������Ҫ�ȴ�100mS���ܿ�ʼö��,�˴��ȴ�200mS����
             */
           	OSTimeDly(OS_TICKS_PER_SEC / 4);

          	if (GusbHostFlags.ucAttached == 0) {
           	    break;
           	}

           	usbBusReset();                                              /*  ��λUSB����			        */
            OSTimeDly(OS_TICKS_PER_SEC / 20);

            __ohciEnableInt(__USB_INT_RHSC);                            /*  ��λ���߽����ֹ���Ӹı��ж�*/
           	                                                            /*  ���ڸ�λ�������ʹ�ܸ��ж�  */
           	USBDEBUG_SENDSTR("\r\n usbEnumerate...\r\n\r\n");

           	__hcDisEnSchedAndWaitNextFrame(USB_TRAN_TYPE_CONTROL);
           	__GpohciEd->edsControl.uiControl = (__GpohciEd->edsControl.uiControl & ~(1 << 13)) |
           	                                   (__GucUsbSpeed << 13);   /*  ���ö˵��ٶ�(ȫ�ٻ����)    */
           	__hcEnableSchedule(USB_TRAN_TYPE_CONTROL);

           	ucRet = usbGetDeviceDescription(0x08, ucBuf);               /*  ��ȡ�豸��������ǰ8�ֽ�     */
           	if (ucRet != USB_ERR_SUCESS) {
                if (ucRet == USB_ERR_STALL) {
                    usbClearFeature_EP(0);
                }
                break;
           	}

           	pucTmp = (USB_INT8U *)&GusbDeviceDescr;
           	usbMemCopy(pucTmp, ucBuf, 0x08);                            /*  ����ȡ����������䵽�豸    */
           	                                                            /*  ...�������Ľṹ�������     */

           	if ((GusbDeviceDescr.bLength         != 18)                            ||
                (GusbDeviceDescr.bDescriptorType != __USB_DESCRIPTOR_TYPE_DEVICE)) {
           	    break;                                                  /*  ��ȡ�����豸����������      */
           	}

           	__hcDisEnSchedAndWaitNextFrame(USB_TRAN_TYPE_CONTROL);
           	__GpohciEd->edsControl.uiControl = (__GpohciEd->edsControl.uiControl & ~(0x7f << 16)) |
           	                                   (GusbDeviceDescr.bMaxPacketSize0 << 16);
           	__hcEnableSchedule(USB_TRAN_TYPE_CONTROL);

           	ucRet = usbSetAddress(0x01);                                /*  ���ôӻ���ַ: 0x01          */
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

           	ucRet = usbGetDeviceDescription(18, ucBuf);                 /*  ���»�ȡ18�ֽڵ��豸������  */
           	if (ucRet != USB_ERR_SUCESS) {
                if (ucRet == USB_ERR_STALL) {
                    usbClearFeature_EP(0);
                }
                break;
           	}
           	pucTmp = (USB_INT8U *)&GusbDeviceDescr;
           	usbMemCopy(pucTmp, ucBuf, 18);                              /*  ����ȡ����������䵽�豸    */
           	                                                            /*  ...�������Ľṹ�������     */
           	__usbGetDevString();                                        /*  ��ȡ�����ַ����������Ͳ�Ʒ��*/
           	                                                            /*  ����������,�����͵����Խӿ� */
           	ucRet = usbGetConfigDescription(0, 0, 9, ucBuf);            /*  ��ȡ9�ֽڵ�����������       */
           	if (ucRet != USB_ERR_SUCESS) {
                if (ucRet == USB_ERR_STALL) {
                    usbClearFeature_EP(0);
                }
                break;
           	}
           	pucTmp = (USB_INT8U *)&GusbConfigDescr;
           	usbMemCopy(pucTmp, ucBuf, 0x09);                            /*  ����ȡ����������䵽����    */
           	                                                            /*  ...�������Ľṹ�������     */
           	usConfigDescLen = (USB_INT16U)(GusbConfigDescr.wTotalLength1 << 8);
           	usConfigDescLen = (USB_INT16U)(usConfigDescLen + GusbConfigDescr.wTotalLength0);
           	                                                            /*  ��������������������        */
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
           	                                                            /*  ��ȡ���е��豸������        */
           	if (ucRet != USB_ERR_SUCESS) {
                if (ucRet == USB_ERR_STALL) {
                    usbClearFeature_EP(0);
                }
                break;
           	} else {
           	    usbInterfaceEpConfig(pucTmp, usConfigDescLen);          /*  ��ȡ���������������еĽӿ�  */
            }                                                           /*  ..�������Ͷ˵���������������*/

#if STATIC_MALLOC_EN
#else
           	free(pucTmp);                                               /*  �ͷŻ�����                  */
#endif

            ucRet = usbSetConfiguratiton();                             /*  ��������                    */
           	if (ucRet != USB_ERR_SUCESS) {
                if (ucRet == USB_ERR_STALL) {
                    usbClearFeature_EP(0);
                }
                break;
           	}
            GusbHostFlags.ucEnumed = 1;                                 /*  ��־ö�ٳɹ�                */
            USBDEBUG_SENDSTR("\r\n usbEnumerate sucess!\r\n");

            usbPipeOpen(USB_TRAN_TYPE_INTR_OUT);
            usbPipeOpen(USB_TRAN_TYPE_INTR_IN);
            usbPipeOpen(USB_TRAN_TYPE_BULK_OUT);
            usbPipeOpen(USB_TRAN_TYPE_BULK_IN);

        } while (0);

        if (GusbHostFlags.ucEnumed) {
            if (__GpEnumSucessCallBack) {
                (*__GpEnumSucessCallBack)();                            /*  ִ��ö�ٳɹ��ص�����        */
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
** Descriptions:        ��ȡ��Ӧ���������Ͳ�Ʒ������,�����͵����Խӿ���
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __usbGetDevString (void)
{
    USB_INT8U  ucBuf[44];
    USB_INT16U usLanID, usLen;
    INT8U      i, ucRet;

   	ucRet = usbGetStringDescription(0, 0, 4, ucBuf);                    /*  ��ȡ4�ֽ�����ID             */
   	if (ucRet != USB_ERR_SUCESS) {
        if (ucRet == USB_ERR_STALL) {
            usbClearFeature_EP(0);
        }
        return;
   	} else {
   	    usLanID = (USB_INT16U)(ucBuf[3] << 8);
   	    usLanID = (USB_INT16U)(usLanID | ucBuf[2]);                     /*  ����ID                      */
   	}

   	if (GusbDeviceDescr.iManufacturer) {
   	    ucBuf[1] = 0;
   	    ucRet = usbGetStringDescription(GusbDeviceDescr.iManufacturer,
   	                                    usLanID, 2, ucBuf);             /*  ��Ӧ���ַ���������ǰ���ֽ�  */
       	if (ucBuf[1] != __USB_DESCRIPTOR_TYPE_STRING) {
       	    return;
       	}
       	usLen = ucBuf[0];                                               /*  ��ȡ��Ӧ���ַ�������������  */
       	if (usLen > sizeof(ucBuf)) {
       	    usLen = sizeof(ucBuf);
       	}
       	ucRet = usbGetStringDescription(GusbDeviceDescr.iManufacturer,
   	                                    usLanID, usLen, ucBuf);         /*  ��ȡ��Ӧ���ַ���������      */
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
   	                                    usLanID, 2, ucBuf);             /*  ��ȡ��Ʒ�ַ���������ǰ���ֽ�*/
       	if (ucBuf[1] != __USB_DESCRIPTOR_TYPE_STRING) {
       	    return;
       	}
       	usLen = ucBuf[0];                                               /*  ��ȡ��Ʒ�ַ�������������  */
       	if (usLen > sizeof(ucBuf)) {
       	    usLen = sizeof(ucBuf);
       	}
       	ucRet = usbGetStringDescription(GusbDeviceDescr.iProduct,
   	                                    usLanID, usLen, ucBuf);         /*  ��ȡ��Ӧ���ַ���������      */
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
  ����Ϊ USB Э���еı�׼����
*********************************************************************************************************/

/*********************************************************************************************************
** Function name:       usbGetStatus
** Descriptions:        ��ȡ״̬����
** input parameters:    ucType  Ҫ��ȡ��״̬�Ķ�������,ָ�豸(0),�ӿ�(1),��˵�(2)
**                      wIndex  ����,�豸��(�̶�Ϊ0),��ӿں�,��˵��
** output parameters:   pucData �������ݻ�����,���ڴ�Ŷ�ȡ����״ֵ̬
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
*********************************************************************************************************/
USB_INT8U usbGetStatus (USB_INT8U ucType , USB_INT16U wIndex, USB_INT8U *pucData)
{
    USB_DEV_REQ usbDeviceRequest;

    USBDEBUG_SENDSTR("...usbGetStatus\r\n");

    usbMemSet(&usbDeviceRequest, 0, sizeof (USB_DEV_REQ));              /*  ����usbDeviceRequest        */

    usbDeviceRequest.bmRequestType = (USB_INT8U)(__USB_DEVICE_TO_HOST | __USB_STANDARD_REQUEST | ucType);
                                                                        /*  ��������                    */
    usbDeviceRequest.bRequest      = __USB_REQUEST_GET_STATUS;          /*  �����                      */
    usbDeviceRequest.wValue        = 0;
    usbDeviceRequest.wIndex        = wIndex;
    usbDeviceRequest.wLength       = 2;                                 /*  ���ص����ݳ��ȹ̶�Ϊ2�ֽ�   */

    return usbStandardReqTransfer(&usbDeviceRequest, pucData);          /*  ��������,��ȡ���豸����ֵ   */

}

/*********************************************************************************************************
** Function name:       usbClearFeature
** Descriptions:        �������
** input parameters:    ucType  Ҫ��ȡ��״̬�Ķ�������,ָ�豸(0),�ӿ�(1),��˵�(2)
**                      wValue  ����ѡ���
**                      wIndex  ����,�豸��(�̶�Ϊ0),��ӿں�,��˵��
** output parameters:   None
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
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
** Descriptions:        ��������
** input parameters:    ucType  Ҫ��ȡ��״̬�Ķ�������,ָ�豸(0),�ӿ�(1),��˵�(2)
**                      wValue  ����ѡ���
**                      wIndex  ����,�豸��(�̶�Ϊ0),��ӿں�,��˵��
** output parameters:   None
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
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
** Descriptions:        ���õ�ַ
** input parameters:    wValue ��ֵַ
** output parameters:   None
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
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
** Descriptions:        ��ȡ������
** input parameters:    wValue  ���ͺ�����
**                      wIndex  0 ������ ID
**                      wLength ����������
** output parameters:   pucData �����������Ļ�����
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
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
** Descriptions:        ����������
** input parameters:    wValue  ���ͺ�����
**                      wIndex  0 ������ID
**                      wLength ����������
** output parameters:   pucData �����������Ļ�����
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
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
** Descriptions:        ��ȡ����ֵ����
** input parameters:    None
** output parameters:   pucData ����ֵ
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
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
** Descriptions:        ��������ֵ����
** input parameters:    None
** output parameters:   None
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
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

    GusbHostFlags.ucConfiged = 1;                                       /*  ����豸������              */

    return USB_ERR_SUCESS;
}


/*********************************************************************************************************
** Function name:       usbGetInterface
** Descriptions:        ��ȡָ���ӿڵ�����ֵ,���ӿ��������е�bAlternateSetting�ֶ�ֵ
** input parameters:    wIndex  �ӿں�
** output parameters:   pucData ���ص�bAlternateSetting�ֶ�ֵ
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
*********************************************************************************************************/
USB_INT8U usbGetInterface (USB_INT16U wIndex, USB_INT8U *pucData)
{
    /*
     *  �˺�����δ����
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
** Descriptions:        ���ýӿ�����,��usbGetInterface���Ӧ
** input parameters:    wValue  ���滻������ֵ
**                      wIndex  �ӿں�
** output parameters:   None
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
*********************************************************************************************************/
USB_INT8U usbSetInterface (USB_INT16U wValue, USB_INT16U wIndex)
{
    /*
     *  �˺�����δ����
     */
    USB_DEV_REQ usbDeviceRequest;

    if (GusbHostFlags.ucConfiged == 0) {                                /*  ����USB�豸��������״̬ʱ   */
        return FALSE;                                                   /*  ...�������ýӿ�             */
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
** Descriptions:        ͬ��֡����,�������ò�����˵��ͬ��֡��
** input parameters:    wIndex  �˵��
** output parameters:   pucData ���ص�֡��
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
USB_INT8U usbSunchFrame (USB_INT16U wIndex, USB_INT8U *pucData)
{
    return USB_ERR_SOFT_NOT_SUPPORT;                                    /*  ��ǰ�汾��֧��ͬ������      */
}


/*********************************************************************************************************
** Function name:       usbInterfaceEpConfig
** Descriptions:        ���ݻ�ȡ�������������������ҳ��ӿ�����������˵�������,
**                      ��������䵽��Ӧ���������ṹ����
** input parameters:    pucBuf          ��ȡ�����������������ݻ�����
**                      usConfigDescLen �����������ܳ���
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
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
   	 *  ��ȡ�ӿںͶ˵����������Ϣ
   	 */
   	cInterfaceIndex = -1;
   	usCount = 9;                                                        /*  ����9�ֽڻ���������������   */
   	do {
   	    usTmp = *(pucBuf + usCount + 1);                                /*  ����������                  */
   	    if (usTmp == __USB_DESCRIPTOR_TYPE_INTERFACE) {                 /*  �ӿ�������                  */
   	        cInterfaceIndex++;
   	        GusbInterEpDescr[cInterfaceIndex].ucInterfaceIndex = cInterfaceIndex;
   	        if (cInterfaceIndex >= __USB_MAX_INTERFACE) {               /*  ���еĽӿ��������ѳ�����    */
   	            break;                                                  /*  ...Ԥ���Ľӿ���������       */
   	        }

   	        /*
   	         *  �ҵ���cInterfaceIndex���ӿ�
   	         */
   	        pucTmp = (USB_INT8U *)&(GusbInterEpDescr[cInterfaceIndex].usbInterfaceDescr);
   	        usbMemCopy(pucTmp, pucBuf + usCount, sizeof(USB_INTERFACE_DESCRIPTOR));

      	    /*
      	     *  ����ýӿڵĸ��˵������,�Է������һ�ӿڵĶ˵������м���
      	     */
      	    ucBulkInIndex  = 0;
   	        ucBulkOutIndex = 0;
   	        ucIntrInIndex  = 0;
   	        ucIntrOutIndex = 0;
   	        ucIsoInIndex   = 0;
   	        ucIsoOutIndex  = 0;

	    } else if (usTmp == __USB_DESCRIPTOR_TYPE_ENDPOINT) {           /*  �˵�������                  */
	        if (cInterfaceIndex < 0) {                                  /*  ��δ�ҵ��ӿ�������          */
	            return FALSE;
	        }

   	        if ( (*(pucBuf + usCount + 3) & 0x03) == 0x01) {            /*  ͬ������                    */
   	            if ( (*(pucBuf + usCount + 2) & (USB_INT8U)0x80) != 0) {/*  IN �˵�                     */
   	                pucTmp = (USB_INT8U *)&GusbInterEpDescr[cInterfaceIndex].\
   	                                       usbIsoInEpDescr[ucIsoInIndex++];
   	                GusbInterEpDescr[cInterfaceIndex].ucIsoInEpNum++;
   	            } else {                                                /*  OUT �˵�                    */
   	                pucTmp = (USB_INT8U *)&GusbInterEpDescr[cInterfaceIndex].\
   	                                       usbIsoOutEpDescr[ucIsoOutIndex++];
   	                GusbInterEpDescr[cInterfaceIndex].ucIsoOutEpNum++;
   	            }
   	        } else if ( (*(pucBuf + usCount + 3) & 0x03) == 0x02) {     /*  �鴫��                      */
   	            if ( (*(pucBuf + usCount + 2) & (USB_INT8U)0x80) != 0) {/*  IN �˵�                     */
   	                pucTmp = (USB_INT8U *)&GusbInterEpDescr[cInterfaceIndex].\
   	                                       usbBulkInEpDescr[ucBulkInIndex++];
   	                GusbInterEpDescr[cInterfaceIndex].ucBulkInEpNum++;
   	            } else {                                                /*  OUT �˵�                    */
   	                pucTmp = (USB_INT8U *)&GusbInterEpDescr[cInterfaceIndex].\
   	                                       usbBulkOutEpDescr[ucBulkOutIndex++];
   	                GusbInterEpDescr[cInterfaceIndex].ucBulkOutEpNum++;
   	            }
   	        } else if ( (*(pucBuf + usCount + 3) & 0x03) == 0x03) {     /*  �жϴ���                    */
   	            if ( (*(pucBuf + usCount + 2) & (USB_INT8U)0x80) != 0) {/*  IN �˵�                     */
   	                pucTmp = (USB_INT8U *)&GusbInterEpDescr[cInterfaceIndex].\
   	                                       usbIntrInEpDescr[ucIntrInIndex++];
   	                GusbInterEpDescr[cInterfaceIndex].ucIntrInEpNum++;
   	            } else {                                                /*  OUT �˵�                    */
   	                pucTmp = (USB_INT8U *)&GusbInterEpDescr[cInterfaceIndex].\
   	                                       usbIntrOutEpDescr[ucIntrOutIndex++];
   	                GusbInterEpDescr[cInterfaceIndex].ucIntrOutEpNum++;
   	            }
   	        }
   	        usbMemCopy(pucTmp, pucBuf + usCount, sizeof(USB_ENDPOINT_DESCRIPTOR));
   	                                                                    /*  ��������䵽��Ӧ�Ķ˵�������*/
   	                                                                    /*  ...�ṹ�����               */
            ucEpNum = *(pucBuf + usCount + 2);
            ucEpNum = (USB_INT8U)(((ucEpNum >> 3) | ucEpNum) & 0x1F);
            usMaxPktSize = (USB_INT16U)(*(pucBuf + usCount + 5) << 8);
            usMaxPktSize = (USB_INT16U)(usMaxPktSize + *(pucBuf + usCount + 4));
            __GusEpMaxPktSize[cInterfaceIndex][ucEpNum] = usMaxPktSize;

        } else if (usTmp == __USB_DESCRIPTOR_TYPE_OTG) {                /*  OTG������                   */
            pucTmp = (USB_INT8U *)&GusbOtgDescr;
            usbMemCopy(pucTmp, pucBuf + usCount, sizeof(USB_OTG_DESCRIPTOR));

        } else {                                                        /*  ��������������������,������ */
            ;
        }
        usCount = (USB_INT16U)(usCount + *(pucBuf + usCount));          /*  ���¼�����                  */
    } while (usCount < usConfigDescLen);

    if (cInterfaceIndex < 0) {                                          /*  δ�ҵ��ӿ�������            */
        return FALSE;
    }

    /*
     *  �������еĽӿ�����������д��ÿ���ӿ��������ṹ�������ucInterfaceNum�ֶ���
     */
    for (usCount = 0; usCount <= cInterfaceIndex; usCount++) {
        GusbInterEpDescr[usCount].ucInterfaceNum = (USB_INT8U)(cInterfaceIndex + 1);
    }

    return TRUE;
}

/*********************************************************************************************************
** Function name:       usbGetEpMaxPktSize
** Descriptions:        ��ȡ�˵��������С
** input parameters:    ucInterfaceIndex �ӿ�����
**                      ucEpNum          �˵��
** output parameters:   None
** Returned value:      > 0 : ��Ҫ�˵��������С,  0 : ʧ��,��ʾ�˵�ucEpNum������
*********************************************************************************************************/
USB_INT16U usbGetEpMaxPktSize (USB_INT8U ucInterfaceIndex, USB_INT8U ucEpNum)
{
    ucEpNum = (USB_INT8U)(((ucEpNum >> 3) | ucEpNum) & 0x1F);

    return __GusEpMaxPktSize[ucInterfaceIndex][ucEpNum];
}

/*********************************************************************************************************
** Function name:       usbIsDeviceReady
** Descriptions:        �ж��豸�Ƿ�׼����
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : �豸׼����   FALSE : �豸δ׼����
*********************************************************************************************************/
USB_BOOL usbIsDeviceReady (void)
{
    return (USB_BOOL)(GusbHostFlags.ucEnumed == 1);
}

/*********************************************************************************************************
** Function name:       usbIsDeviceAttach
** Descriptions:        �ж��豸�Ƿ����
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : �豸�Ѳ���   FALSE : �豸δ׼������
*********************************************************************************************************/
USB_BOOL usbIsDeviceAttach (void)
{
    return (USB_BOOL)(GusbHostFlags.ucAttached == 1);
}

/*********************************************************************************************************
** Function name:       usbGetDeviceType
** Descriptions:        ��ȡ�豸������Ϣ,�����豸����,��֧��Э���
** input parameters:    ucInterfaceIndex �ӿں�
** output parameters:   pusbDeviceType   �豸����
** Returned value:      TRUE : �ɹ�   FALSE : ʧ��
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
** Descriptions:        ��ȡ�ǿ��ƶ˵�Ķ˵��
** input parameters:    ucInterface �ӿں�
**                      ucTranType  ��������
** output parameters:   None
** Returned value:      > 0 : �˵��,  = 0 : ��ȡʧ��,�������豸�����ڸ����Ͷ˵�
*********************************************************************************************************/
USB_INT8U usbGetEp (USB_INT8U ucInterface, USB_INT8U ucTranType)
{
    USB_INT8U ucEpNum = 0;

    switch (ucTranType) {

    case USB_TRAN_TYPE_BULK_OUT:                                        /*  Bulk OUT ����               */
        ucEpNum = GusbInterEpDescr[ucInterface].usbBulkOutEpDescr[0].bEndpointAddress;
        break;

    case USB_TRAN_TYPE_BULK_IN:                                         /*  Bulk IN ����                */
        ucEpNum = GusbInterEpDescr[ucInterface].usbBulkInEpDescr[0].bEndpointAddress;
        break;

    case USB_TRAN_TYPE_INTR_OUT:                                        /*  Intrrupt OUT  ����          */
        ucEpNum = GusbInterEpDescr[ucInterface].usbIntrOutEpDescr[0].bEndpointAddress;
        break;

    case USB_TRAN_TYPE_INTR_IN:                                         /*  Intrrupt IN ����            */
        ucEpNum = GusbInterEpDescr[ucInterface].usbIntrInEpDescr[0].bEndpointAddress;
        break;

    default:                                                            /*  ��֧�ֵĴ�������            */
        ucEpNum = 0;
    }

    return ucEpNum;
}

/*********************************************************************************************************
** Function name:       usbGetVer
** Descriptions:        ȡ��USBЭ��ջ�İ汾��
** input parameters:    None
** output parameters:   None
** Returned value:      32λ�汾��
*********************************************************************************************************/
INT32U usbGetVer (void)
{
#define __CPU_TYPE          3200

#define __USB_TYPE_DEVICE   0
#define __USB_TYPE_HOST     1U
#define __USB_TYPE_OTG      2U

#define __USB_TYPE          __USB_TYPE_HOST

#define __USB_VERSION       0x0110

	return (__USB_TYPE_HOST << 30) | (__CPU_TYPE << 16) | __USB_VERSION;/*  ����Э��ջ,for LPC3250,     */
														                /*  ...�汾 0x0110              */
}

#ifdef __cplusplus
 }
#endif

/*********************************************************************************************************
** Function name:       __usbdInit
** Descriptions:        USBD��Ļ�����ʼ��
** input parameters:    pEnumSucessCallBack: ö�ٳɹ�ʱ�ص�����
**                      pDevDisconCallBack:  �豸����ʱ�ص�����
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
