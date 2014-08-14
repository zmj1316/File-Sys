/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**                                     
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            USBTransfer.h
** Latest modified Date: 2007-11-14        
** Latest Version:       V1.0    
** Description:          USB主机软件包的传输函数USBTransfer.c头文件
**                       
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗   Longsui Wu   
** Created date:         2007-11-14    
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
#ifndef __USBTRANSFER_H
#define __USBTRANSFER_H

#include "..\USBHostConfig.h"

typedef struct __tagUSB_HOST_EVENT_CNT {
    USB_INT8U ucStdTranCnt;                                            /*  标准请求计数器               */
    USB_INT8U ucDataTranCnt;                                           /*  非控制传输计数器             */
} __USB_HOST_EVENT_CNT, *__PUSB_HOST_EVENT_CNT;
    
/*********************************************************************************************************
** Function name:       usbStandardReqTransfer
** Descriptions:        发送标准请求, 并取得返回值
** input parameters:    pusbDevReq  标准请求
** output parameters:   pucBuf      设备返回的数据
**                      puiSts      状态,如错误号等
** Returned value:      错误码
*********************************************************************************************************/
USB_INT8U usbStandardReqTransfer (PUSB_DEV_REQ pusbDevReq, USB_INT8U *pucBuf);

/*********************************************************************************************************
** Function name:       usbDataTransfer
** Descriptions:        发送或接收 Bulk, Intrrupt 传输的数据
** input parameters:    pucData       要发送或接收的数据缓冲区
**                      uiLength      要发送或接收数据的长度
**                      uiTranType    传输类型: __HC_ED_TYPE_BULK 或 __HC_ED_TYPE_INTR
**                      ucMaxTryCount 当出现错误时,最大的重试次数
** Returned value:      错误码
*********************************************************************************************************/
USB_INT8U usbDataTransfer (USB_INT8U *pucData,                          /*  数据缓冲区                  */
                           USB_INT32U uiLength,                         /*  要传输的数据长度            */
                           USB_INT8U  ucTranType,                       /*  传输类型                    */
                           USB_INT8U  ucMaxTryCount);                   /*  发生错误时最大尝试次数      */


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
                                                                        /*  信号量等待时间最长为 2s     */
#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/








