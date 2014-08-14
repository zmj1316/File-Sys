/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**                                     
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            massstorage.c
** Latest modified Date: 2007-11-06        
** Latest Version:       V1.0    
** Description:          ʵ�ִ������豸����
**                       
**--------------------------------------------------------------------------------------------------------
** Created By:           ��¡��   Longsui Wu   
** Created date:         2007-11-26    
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

__MS_INFO __msInfo;                                                     /*  ���������豸����Ϣ�ṹ��    */

USB_INT8U __msHostDeviceInit (USB_INT8U ucInterfaceIndex);
USB_INT8U __msHostLunInit (USB_INT16U usInterface, USB_INT8U ucLunIndex);

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************************
** Function name:       msHostInit
** Descriptions:        ���������豸��ʼ��
** input parameters:    ucInterfaceIndex : �ӿں�
** output parameters:   None
** Returned value:      �������,������MS_ERR_SUCESS, ˵��ִ�гɹ�, ����ִ��ʧ��
*********************************************************************************************************/
USB_INT8U msHostInit (USB_INT8U ucInterfaceIndex)
{
    USB_INT8U i, ucRet;
    
    ucRet = __msHostDeviceInit(ucInterfaceIndex);
    if (ucRet != MS_ERR_SUCESS) {
        return ucRet;
    }
    
    for (i = 0; i < __msInfo.ucMaxLun; i++) {
        ucRet = __msHostLunInit((USB_INT16U)ucInterfaceIndex, i);
        if (ucRet != MS_ERR_SUCESS) {
            return ucRet;
        }
    }
    return MS_ERR_SUCESS;
 }

/*********************************************************************************************************
** Function name:       msHostDeInit
** Descriptions:        ���������豸ж��
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE: �ɹ�, FALSE: ʧ��
*********************************************************************************************************/
USB_BOOL msHostDeInit (void)
{
    if (!__rbcDeInit()) { 
        return FALSE;
    }
    __boDeInit();
    return TRUE;
}

#ifdef __cplusplus
 }
#endif

/*********************************************************************************************************
** Function name:       __msHostDeviceInit
** Descriptions:        ���������豸����ʼ��
** input parameters:    ucInterfaceIndex : �ӿں�
** output parameters:   None
** Returned value:      �������,������MS_ERR_SUCESS, ˵��ִ�гɹ�, ����ִ��ʧ��
*********************************************************************************************************/
USB_INT8U __msHostDeviceInit (USB_INT8U ucInterfaceIndex)
{
    USB_INT8U       i, n;
    USB_INT8U       ucErrCode;
    USB_INT8U       ucTryCount;
    USB_INT8U       ucMaxLuns = 0;
    
    USB_DEVICE_TYPE usbDeviceType;
    
    USBDEBUG_SENDSTR("\r\nmsHostInit...\r\n");
    
    usbGetDeviceType(ucInterfaceIndex, &usbDeviceType);                 /*  ��ȡ�豸���͵���Ϣ          */
    if ((usbDeviceType.bInterfaceClass != __USB_DEVICE_CLASS_STORAGE) ||/*  �ж��Ƿ�Ϊ���������豸      */
        (usbDeviceType.bInterfaceProtocol != 0x50) || 
        (usbDeviceType.bInterfaceSubClass != 0x06)) {                   /*  SCSI��                      */
        USBDEBUG_SENDSTR("\r\nmsHostInit: device not support\r\n");
        return MS_ERR_DEVICE_NOT_SUPPORT;
    }
    
    usbMemSet(&__msInfo, 0, sizeof(__msInfo));
    
    if (!__boInit()) {
        USBDEBUG_SENDSTR("\r\n__boInit failed!\r\n");
        return MS_ERR_ENVIRONMENT;
    }
    __rbcInit();
    __boMassStorReset(ucInterfaceIndex);                                /*  ��λ���������豸            */
    
    ucTryCount = 0;
    do {
        ucErrCode = __boGetMaxLun (ucInterfaceIndex, &ucMaxLuns);       /*  ��ȡ���LUN��               */
        if (ucErrCode != MS_ERR_SUCESS) {
            if (ucErrCode == USB_ERR_STALL) {                           /*  ����豸����STALL,�����    */
                                                                        /*  ֻ��һ��LUN                 */
                usbClearFeature_EP(0);
                ucMaxLuns = 0;
                break;
            }
            USBDEBUG_SENDSTR("\r\n__boGetMaxLun failed!\r\n");
            return ucErrCode;
        }
        if (ucMaxLuns <= 0x0F) {                                        /*  LUN���ֻ��Ϊ0x0F,����õ���*/
            break;                                                      /*  LUN����0x0F,����Ϊ����,     */
        }                                                               /*  �����»�ȡ                  */
        OSTimeDly(1);
    } while (++ucTryCount <= 3);
    if (ucTryCount > 3) {
        USBDEBUG_SENDSTR("\r\n__boGetMaxLun failed! Had try 3 times\r\n");
        return MS_ERR_GET_MAXLUN;                                       /*  ��ȡ LUN ʧ��               */
    }
    
    if (ucMaxLuns > MS_MAX_LUN) {
        ucMaxLuns = MS_MAX_LUN;
    }
    
    /*
     *  �ȴ��豸����,���жϸ��豸�����ĸ�LUN��
     */
    n = 0;
    for (i = 0; i <= ucMaxLuns; i++) {
        ucTryCount = 0;
        do {
            ucErrCode = rbcTestUintReady(ucInterfaceIndex, i);          /*  �����豸�Ƿ�׼����,���׼�� */
                                                                        /*  ��,����Ϊ��LUN���д洢����  */
            if (ucErrCode == MS_ERR_SUCESS) {
                __msInfo.ucMaxLun++;                                    /*  ׼���õ�LUN���Լ�1          */
                __msInfo.ucUsedLun[n++] = i;                            /*  �����LUN��                 */
                break;
            } else {
                OSTimeDly(1);
            }
        } while (++ucTryCount <= 10);
    }
    if (__msInfo.ucMaxLun == 0) {
        USBDEBUG_SENDSTR("\r\nMS_ERR_NONE_DEVICE!\r\n");
        return MS_ERR_NONE_DEVICE;                                      /*  û�о������豸              */
    }
    
    return MS_ERR_SUCESS;
}

/*********************************************************************************************************
** Function name:       __msHostLunInit
** Descriptions:        ���������ĳ��LUN��ʼ��
** input parameters:    ucInterfaceIndex : �ӿں�
**                      ucLunIndex       : LUN��
** output parameters:   None
** Returned value:      �������,������MS_ERR_SUCESS, ˵��ִ�гɹ�, ����ִ��ʧ��
*********************************************************************************************************/
USB_INT8U __msHostLunInit (USB_INT16U usInterface, USB_INT8U ucLunIndex)
{
    USB_INT8U  ucBufTmp[40];
    USB_INT8U  ucErrCode;
    USB_INT8U  ucTryCount;
    USB_INT32U uiMaxLba        = 0;
    USB_INT32U uiBytesPerBlock = 0;

    USBDEBUG_SENDSTR("\r\nmsHostLunInit...\r\n");

    ucErrCode = rbcInquiry(usInterface, ucLunIndex, ucBufTmp);/*  ��ѯ�豸                   */
    if (ucErrCode != MS_ERR_SUCESS) {
        USBDEBUG_SENDSTR("\r\nInquiry failed\r\n");
        return ucErrCode;
    }

    /*
     *  ��ȡ�豸����
     */
    ucTryCount = 0;
    do {
        ucErrCode = rbcReadCapacity(usInterface, ucLunIndex, &uiMaxLba, &uiBytesPerBlock);
                                                                        /*  ��ȡ�豸����                */
        if ((ucErrCode != MS_ERR_SUCESS) && (ucErrCode != 0x06)) {
            USBDEBUG_SENDSTR("\r\nReadCapacity failed\r\n");
            return ucErrCode;
        } 
        
        if (uiMaxLba != 0) {
            break;
        }
        OSTimeDly(1);
    } while (++ucTryCount < 0x03);   
    
    if (ucTryCount >= 0x03) {
        USBDEBUG_SENDSTR("\r\nReadCapacity failed, Had try 3 times\r\n");
        return MS_ERR_DEVICE_NOT_READY;
    } else {
        __msInfo.msLunInfo[ucLunIndex].uiMaxLba        = uiMaxLba;      /*  ��� LBA                    */
        __msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock = uiBytesPerBlock;
    }                                                                   /*  ÿ���ֽ���                  */
    
    return MS_ERR_SUCESS;
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
