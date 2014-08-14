/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**                                     
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            USBTransfer.h
** Latest modified Date: 2007-11-14        
** Latest Version:       V1.0    
** Description:          USB����������Ĵ��亯��USBTransfer.cͷ�ļ�
**                       
**--------------------------------------------------------------------------------------------------------
** Created By:           ��¡��   Longsui Wu   
** Created date:         2007-11-14    
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
#ifndef __USBTRANSFER_H
#define __USBTRANSFER_H

#include "..\USBHostConfig.h"

typedef struct __tagUSB_HOST_EVENT_CNT {
    USB_INT8U ucStdTranCnt;                                            /*  ��׼���������               */
    USB_INT8U ucDataTranCnt;                                           /*  �ǿ��ƴ��������             */
} __USB_HOST_EVENT_CNT, *__PUSB_HOST_EVENT_CNT;
    
/*********************************************************************************************************
** Function name:       usbStandardReqTransfer
** Descriptions:        ���ͱ�׼����, ��ȡ�÷���ֵ
** input parameters:    pusbDevReq  ��׼����
** output parameters:   pucBuf      �豸���ص�����
**                      puiSts      ״̬,�����ŵ�
** Returned value:      ������
*********************************************************************************************************/
USB_INT8U usbStandardReqTransfer (PUSB_DEV_REQ pusbDevReq, USB_INT8U *pucBuf);

/*********************************************************************************************************
** Function name:       usbDataTransfer
** Descriptions:        ���ͻ���� Bulk, Intrrupt ���������
** input parameters:    pucData       Ҫ���ͻ���յ����ݻ�����
**                      uiLength      Ҫ���ͻ�������ݵĳ���
**                      uiTranType    ��������: __HC_ED_TYPE_BULK �� __HC_ED_TYPE_INTR
**                      ucMaxTryCount �����ִ���ʱ,�������Դ���
** Returned value:      ������
*********************************************************************************************************/
USB_INT8U usbDataTransfer (USB_INT8U *pucData,                          /*  ���ݻ�����                  */
                           USB_INT32U uiLength,                         /*  Ҫ��������ݳ���            */
                           USB_INT8U  ucTranType,                       /*  ��������                    */
                           USB_INT8U  ucMaxTryCount);                   /*  ��������ʱ����Դ���      */


#define usbHostIntrRead(pucData, uiLength, ucMaxTry)    usbDataTransfer(pucData, uiLength, \
                                                                        USB_TRAN_TYPE_INTR_IN, ucMaxTry)
#define usbHostIntrWrite(pucData, uiLength, ucMaxTry)   usbDataTransfer(pucData, uiLength, \
                                                                        USB_TRAN_TYPE_INTR_OUT, ucMaxTry)

#define usbHostBulkRead(pucData, uiLength, ucMaxTry)    usbDataTransfer(pucData, uiLength, \
                                                                        USB_TRAN_TYPE_BULK_IN, ucMaxTry)
#define usbHostBulkWrite(pucData, uiLength, ucMaxTry)   usbDataTransfer(pucData, uiLength, \
                                                                        USB_TRAN_TYPE_BULK_OUT, ucMaxTry)


#define usbHostIntrIn(pucData, uiLength, ucMaxTry)      usbHostIntrRead(pucData, uiLength, ucMaxTry)
#define usbHostIntrOut(pucData, uiLength, ucMaxTry)     usbHostIntrWrite(pucData, uiLength, ucMaxTry)

#define usbHostBulkIn(pucData, uiLength, ucMaxTry)      usbHostBulkRead(pucData, uiLength, ucMaxTry)
#define usbHostBulkOut(pucData, uiLength, ucMaxTry)     usbHostBulkWrite(pucData, uiLength, ucMaxTry)


#define USB_TIMEOUT_TICK                                (OS_TICKS_PER_SEC * 2)
                                                                        /*  �ź����ȴ�ʱ���Ϊ 2s     */
#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/








