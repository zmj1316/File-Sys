/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**                                     
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            massstorage.c
** Latest modified Date: 2007-11-06        
** Latest Version:       V1.0    
** Description:          实现大容量设备操作
**                       
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗   Longsui Wu   
** Created date:         2007-11-26    
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

__MS_INFO __msInfo;                                                     /*  大容量类设备的信息结构体    */

USB_INT8U __msHostDeviceInit (USB_INT8U ucInterfaceIndex);
USB_INT8U __msHostLunInit (USB_INT16U usInterface, USB_INT8U ucLunIndex);

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************************
** Function name:       msHostInit
** Descriptions:        大容量类设备初始化
** input parameters:    ucInterfaceIndex : 接口号
** output parameters:   None
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
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
** Descriptions:        大容量类设备卸载
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE: 成功, FALSE: 失败
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
** Descriptions:        大容量类设备级初始化
** input parameters:    ucInterfaceIndex : 接口号
** output parameters:   None
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U __msHostDeviceInit (USB_INT8U ucInterfaceIndex)
{
    USB_INT8U       i, n;
    USB_INT8U       ucErrCode;
    USB_INT8U       ucTryCount;
    USB_INT8U       ucMaxLuns = 0;
    
    USB_DEVICE_TYPE usbDeviceType;
    
    USBDEBUG_SENDSTR("\r\nmsHostInit...\r\n");
    
    usbGetDeviceType(ucInterfaceIndex, &usbDeviceType);                 /*  获取设备类型等信息          */
    if ((usbDeviceType.bInterfaceClass != __USB_DEVICE_CLASS_STORAGE) ||/*  判断是否为大容量类设备      */
        (usbDeviceType.bInterfaceProtocol != 0x50) || 
        (usbDeviceType.bInterfaceSubClass != 0x06)) {                   /*  SCSI类                      */
        USBDEBUG_SENDSTR("\r\nmsHostInit: device not support\r\n");
        return MS_ERR_DEVICE_NOT_SUPPORT;
    }
    
    usbMemSet(&__msInfo, 0, sizeof(__msInfo));
    
    if (!__boInit()) {
        USBDEBUG_SENDSTR("\r\n__boInit failed!\r\n");
        return MS_ERR_ENVIRONMENT;
    }
    __rbcInit();
    __boMassStorReset(ucInterfaceIndex);                                /*  复位大容量类设备            */
    
    ucTryCount = 0;
    do {
        ucErrCode = __boGetMaxLun (ucInterfaceIndex, &ucMaxLuns);       /*  获取最大LUN号               */
        if (ucErrCode != MS_ERR_SUCESS) {
            if (ucErrCode == USB_ERR_STALL) {                           /*  如果设备返回STALL,则可能    */
                                                                        /*  只有一个LUN                 */
                usbClearFeature_EP(0);
                ucMaxLuns = 0;
                break;
            }
            USBDEBUG_SENDSTR("\r\n__boGetMaxLun failed!\r\n");
            return ucErrCode;
        }
        if (ucMaxLuns <= 0x0F) {                                        /*  LUN最大只能为0x0F,如果得到的*/
            break;                                                      /*  LUN大于0x0F,则认为错误,     */
        }                                                               /*  需重新获取                  */
        OSTimeDly(1);
    } while (++ucTryCount <= 3);
    if (ucTryCount > 3) {
        USBDEBUG_SENDSTR("\r\n__boGetMaxLun failed! Had try 3 times\r\n");
        return MS_ERR_GET_MAXLUN;                                       /*  获取 LUN 失败               */
    }
    
    if (ucMaxLuns > MS_MAX_LUN) {
        ucMaxLuns = MS_MAX_LUN;
    }
    
    /*
     *  等待设备就绪,并判断该设备处于哪个LUN号
     */
    n = 0;
    for (i = 0; i <= ucMaxLuns; i++) {
        ucTryCount = 0;
        do {
            ucErrCode = rbcTestUintReady(ucInterfaceIndex, i);          /*  测试设备是否准备好,如果准备 */
                                                                        /*  好,则认为该LUN下有存储介质  */
            if (ucErrCode == MS_ERR_SUCESS) {
                __msInfo.ucMaxLun++;                                    /*  准备好的LUN数自加1          */
                __msInfo.ucUsedLun[n++] = i;                            /*  保存该LUN号                 */
                break;
            } else {
                OSTimeDly(1);
            }
        } while (++ucTryCount <= 10);
    }
    if (__msInfo.ucMaxLun == 0) {
        USBDEBUG_SENDSTR("\r\nMS_ERR_NONE_DEVICE!\r\n");
        return MS_ERR_NONE_DEVICE;                                      /*  没有就绪的设备              */
    }
    
    return MS_ERR_SUCESS;
}

/*********************************************************************************************************
** Function name:       __msHostLunInit
** Descriptions:        大容量类的某个LUN初始化
** input parameters:    ucInterfaceIndex : 接口号
**                      ucLunIndex       : LUN号
** output parameters:   None
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U __msHostLunInit (USB_INT16U usInterface, USB_INT8U ucLunIndex)
{
    USB_INT8U  ucBufTmp[40];
    USB_INT8U  ucErrCode;
    USB_INT8U  ucTryCount;
    USB_INT32U uiMaxLba        = 0;
    USB_INT32U uiBytesPerBlock = 0;

    USBDEBUG_SENDSTR("\r\nmsHostLunInit...\r\n");

    ucErrCode = rbcInquiry(usInterface, ucLunIndex, ucBufTmp);/*  查询设备                   */
    if (ucErrCode != MS_ERR_SUCESS) {
        USBDEBUG_SENDSTR("\r\nInquiry failed\r\n");
        return ucErrCode;
    }

    /*
     *  读取设备容量
     */
    ucTryCount = 0;
    do {
        ucErrCode = rbcReadCapacity(usInterface, ucLunIndex, &uiMaxLba, &uiBytesPerBlock);
                                                                        /*  读取设备容量                */
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
        __msInfo.msLunInfo[ucLunIndex].uiMaxLba        = uiMaxLba;      /*  最大 LBA                    */
        __msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock = uiBytesPerBlock;
    }                                                                   /*  每块字节数                  */
    
    return MS_ERR_SUCESS;
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
