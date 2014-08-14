/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            msBulkOnly.c
** Latest modified Date: 2007-11-06
** Latest Version:       V1.0
** Description:          �������豸������(BulkOnly)����Э���ʵ��
**
**--------------------------------------------------------------------------------------------------------
** Created By:           ��¡��  Longsui Wu
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
#include "MSHostConfig.h"


OS_EVENT *__GevtBoSem;

/*********************************************************************************************************
** Function name:       __boInit
** Descriptions:        BulkOnly������������ʼ��
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : �ɹ�,  FALSE : ʧ��
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
** Descriptions:        BulkOnly����������ж��
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : �ɹ�,  FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL __boDeInit (void)
{
    USB_INT8U ucErr;

    __GevtBoSem = OSSemDel(__GevtBoSem, OS_DEL_ALWAYS, &ucErr);
    return TRUE;
}

/*********************************************************************************************************
** Function name:       __boMassStorReset
** Descriptions:        ��λ���������豸
** input parameters:    wIndex : �ӿں�
** output parameters:   None
** Returned value:      �������,������MS_ERR_SUCESS, ˵��ִ�гɹ�, ����ִ��ʧ��
*********************************************************************************************************/
USB_INT8U __boMassStorReset (USB_INT16U wIndex)
{
    USB_DEV_REQ usbDeviceRequest;

    usbMemSet(&usbDeviceRequest, 0, sizeof (USB_DEV_REQ));              /*  ����usbDeviceRequest        */

    usbDeviceRequest.bmRequestType = 0x21;                              /*  ��������                    */
    usbDeviceRequest.bRequest      = 0xFF;                              /*  �����                      */
    usbDeviceRequest.wValue        = 0;
    usbDeviceRequest.wIndex        = wIndex;                            /*  �ӿں� Interface            */
    usbDeviceRequest.wLength       = 0;                                 /*  ���ص����ݳ��ȹ̶�Ϊ2�ֽ�   */

    return usbStandardReqTransfer(&usbDeviceRequest, NULL);             /*  ��������,��ȡ���豸����ֵ   */
}

/*********************************************************************************************************
** Function name:       __boGetMaxLun
** Descriptions:        ��ȡ���LUN
** input parameters:    wIndex : �ӿں�
** output parameters:   pucData: ���LUN
** Returned value:      �������,������MS_ERR_SUCESS, ˵��ִ�гɹ�, ����ִ��ʧ��
*********************************************************************************************************/
USB_INT8U __boGetMaxLun (USB_INT16U wIndex, USB_INT8U *pucData)
{
    USB_DEV_REQ usbDeviceRequest;

    usbMemSet(&usbDeviceRequest, 0, sizeof (USB_DEV_REQ));              /*  ����usbDeviceRequest        */

    usbDeviceRequest.bmRequestType = 0xA1;                              /*  ��������                    */
    usbDeviceRequest.bRequest      = 0xFE;                              /*  �����                      */
    usbDeviceRequest.wValue        = 0;
    usbDeviceRequest.wIndex        = wIndex;                            /*  �ӿں� Interface            */
    usbDeviceRequest.wLength       = 1;                                 /*  ���ص����ݳ��ȹ̶�Ϊ2�ֽ�   */

    return usbStandardReqTransfer(&usbDeviceRequest, pucData);          /*  ��������,��ȡ���豸����ֵ   */
}

/*********************************************************************************************************
** Function name:       __boSendCommand
** Descriptions:        ��������,����(��ȡ)���ݲ���ȡ����״ֵ̬
** input parameters:    pcbwInq: CBW�ṹ�����
**                      pucData: Ҫ���ͻ���յ����ݻ�����
** output parameters:   None
** Returned value:      �������,������MS_ERR_SUCESS, ˵��ִ�гɹ�, ����ִ��ʧ��
*********************************************************************************************************/
USB_INT8U __boSendCommand (__PBULK_ONLY_CBW pcbwInq, USB_INT8U *pucData)
{
    USB_INT8U  ucErrCode;
    USB_INT8U  ucBufTmp[13] = {0};
    USB_INT32U uiLength     = 0;

    if (!usbIsDeviceReady()) {                                          /*  ����豸�Ƿ���ö�����      */
        return USB_ERR_DEVICE_NOT_READY;
    }

    if (!pcbwInq) {
        return MS_ERR_INVALID_PARAM;
    }

    OSSemPend(__GevtBoSem, 0, &ucErrCode);

    /*
     *  ��һ��: ����CBW
     */
    ucErrCode = usbHostBulkWrite((USB_INT8U *)pcbwInq, 31, 1);
    if (ucErrCode != USB_ERR_SUCESS) {
        OSSemPost(__GevtBoSem);
        return USB_ERR_DEVICE_NOT_READY;
    }

    /*
     *  �ڶ���: ��ȡ��������
     */
    uiLength = (pcbwInq->dCBWDataTransferLength3 << 24)
             + (pcbwInq->dCBWDataTransferLength2 << 16)
             + (pcbwInq->dCBWDataTransferLength1 << 8)
             +  pcbwInq->dCBWDataTransferLength0;

    if (uiLength) {
        if (pcbwInq->bmCBWFlags & 0x80) {                               /*  IN ����                     */
            ucErrCode = usbHostBulkRead(pucData, uiLength, 0);
        } else {                                                        /*  OUT ����                    */
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
     *  ������: ��ȡCSW
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
     *  �жϷ��ص�״ֵ̬�ĵ��������Ƿ�Ϊ"USBS"
     */
    if ((ucBufTmp[0] != 'U') || (ucBufTmp[1] != 'S') || (ucBufTmp[2] != 'B') || (ucBufTmp[3] != 'S')) {
        OSSemPost(__GevtBoSem);
        return MS_ERR_CSW;
    }

    OSSemPost(__GevtBoSem);

    return ucBufTmp[12];                                                /*  ����״ֵ̬                  */
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
