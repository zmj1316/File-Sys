/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            msBulkOnly.c
** Latest modified Date: 2007-11-06
** Latest Version:       V1.0
** Description:          大容量设备单批量(BulkOnly)传输协议的实现
**
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗  Longsui Wu
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
#include "MSHostConfig.h"


OS_EVENT *__GevtBoSem;

/*********************************************************************************************************
** Function name:       __boInit
** Descriptions:        BulkOnly层的软件环境初始化
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 成功,  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __boInit (void)
{
    __GevtBoSem = OSSemCreate(1);
    if (__GevtBoSem == NULL) {
        return FALSE;
    } else {
        return TRUE;
    }
}

/*********************************************************************************************************
** Function name:       __boDeInit
** Descriptions:        BulkOnly层的软件环境卸载
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 成功,  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __boDeInit (void)
{
    USB_INT8U ucErr;

    __GevtBoSem = OSSemDel(__GevtBoSem, OS_DEL_ALWAYS, &ucErr);
    return TRUE;
}

/*********************************************************************************************************
** Function name:       __boMassStorReset
** Descriptions:        复位大容量类设备
** input parameters:    wIndex : 接口号
** output parameters:   None
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U __boMassStorReset (USB_INT16U wIndex)
{
    USB_DEV_REQ usbDeviceRequest;

    usbMemSet(&usbDeviceRequest, 0, sizeof (USB_DEV_REQ));              /*  清零usbDeviceRequest        */

    usbDeviceRequest.bmRequestType = 0x21;                              /*  请求类型                    */
    usbDeviceRequest.bRequest      = 0xFF;                              /*  请求号                      */
    usbDeviceRequest.wValue        = 0;
    usbDeviceRequest.wIndex        = wIndex;                            /*  接口号 Interface            */
    usbDeviceRequest.wLength       = 0;                                 /*  返回的数据长度固定为2字节   */

    return usbStandardReqTransfer(&usbDeviceRequest, NULL);             /*  发送请求,并取得设备返回值   */
}

/*********************************************************************************************************
** Function name:       __boGetMaxLun
** Descriptions:        获取最大LUN
** input parameters:    wIndex : 接口号
** output parameters:   pucData: 最大LUN
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U __boGetMaxLun (USB_INT16U wIndex, USB_INT8U *pucData)
{
    USB_DEV_REQ usbDeviceRequest;

    usbMemSet(&usbDeviceRequest, 0, sizeof (USB_DEV_REQ));              /*  清零usbDeviceRequest        */

    usbDeviceRequest.bmRequestType = 0xA1;                              /*  请求类型                    */
    usbDeviceRequest.bRequest      = 0xFE;                              /*  请求号                      */
    usbDeviceRequest.wValue        = 0;
    usbDeviceRequest.wIndex        = wIndex;                            /*  接口号 Interface            */
    usbDeviceRequest.wLength       = 1;                                 /*  返回的数据长度固定为2字节   */

    return usbStandardReqTransfer(&usbDeviceRequest, pucData);          /*  发送请求,并取得设备返回值   */
}

/*********************************************************************************************************
** Function name:       __boSendCommand
** Descriptions:        发送命令,发送(读取)数据并获取最后的状态值
** input parameters:    pcbwInq: CBW结构体变量
**                      pucData: 要发送或接收的数据缓冲区
** output parameters:   None
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U __boSendCommand (__PBULK_ONLY_CBW pcbwInq, USB_INT8U *pucData)
{
    USB_INT8U  ucErrCode;
    USB_INT8U  ucBufTmp[13] = {0};
    USB_INT32U uiLength     = 0;

    if (!usbIsDeviceReady()) {                                          /*  检查设备是否已枚举完毕      */
        return USB_ERR_DEVICE_NOT_READY;
    }

    if (!pcbwInq) {
        return MS_ERR_INVALID_PARAM;
    }

    OSSemPend(__GevtBoSem, 0, &ucErrCode);

    /*
     *  第一步: 发送CBW
     */
    ucErrCode = usbHostBulkWrite((USB_INT8U *)pcbwInq, 31, 1);
    if (ucErrCode != USB_ERR_SUCESS) {
        OSSemPost(__GevtBoSem);
        return USB_ERR_DEVICE_NOT_READY;
    }

    /*
     *  第二步: 读取或发送数据
     */
    uiLength = (pcbwInq->dCBWDataTransferLength3 << 24)
             + (pcbwInq->dCBWDataTransferLength2 << 16)
             + (pcbwInq->dCBWDataTransferLength1 << 8)
             +  pcbwInq->dCBWDataTransferLength0;

    if (uiLength) {
        if (pcbwInq->bmCBWFlags & 0x80) {                               /*  IN 传输                     */
            ucErrCode = usbHostBulkRead(pucData, uiLength, 0);
        } else {                                                        /*  OUT 传输                    */
            ucErrCode = usbHostBulkWrite(pucData, uiLength, 0);
        }

        if (ucErrCode != USB_ERR_SUCESS) {
            if (ucErrCode == USB_ERR_STALL) {
                ;
            } else {
                OSSemPost(__GevtBoSem);
                return USB_ERR_DEVICE_NOT_READY;
            }
        }
    }

    /*
     *  第三步: 读取CSW
     */
    ucErrCode = usbHostBulkRead(ucBufTmp, 13, 0);
    if (ucErrCode != USB_ERR_SUCESS) {
        if (ucErrCode == USB_ERR_STALL) {
            ;
        } else {
            OSSemPost(__GevtBoSem);
            return USB_ERR_DEVICE_NOT_READY;
        }
    }

    /*
     *  判断返回的状态值的的特征码是否为"USBS"
     */
    if ((ucBufTmp[0] != 'U') || (ucBufTmp[1] != 'S') || (ucBufTmp[2] != 'B') || (ucBufTmp[3] != 'S')) {
        OSSemPost(__GevtBoSem);
        return MS_ERR_CSW;
    }

    OSSemPost(__GevtBoSem);

    return ucBufTmp[12];                                                /*  返回状态值                  */
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
