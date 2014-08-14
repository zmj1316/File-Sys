/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**                                     
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            usbd.h
** Latest modified Date: 2007-11-15        
** Latest Version:       V1.0    
** Description:          USBD.c的头文件
**                       
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗  Longsui Wu   
** Created date:         2007-11-15    
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
#ifndef __USBD_H
#define __USBD_H

#include "..\USBHostConfig.h"


#define USB_TRAN_TYPE_CONTROL                   0x01                    /*  传输类型                    */

#define USB_TRAN_TYPE_BULK_OUT                  0x02
#define USB_TRAN_TYPE_BULK_IN                   0x82

#define USB_TRAN_TYPE_INTR_OUT                  0x03
#define USB_TRAN_TYPE_INTR_IN                   0x83

#define USB_TRAN_TYPE_ISO                       0x04


/*********************************************************************************************************
  USB 主机状态标志位
*********************************************************************************************************/
typedef struct __tagUSB_HOST_FLAGS {
    
    USB_INT8U ucAttached;                                               /*  设备是(1)否(0)已插入        */
    USB_INT8U ucConfiged;                                               /*  是(1)否(0)已配置设备        */
    USB_INT8U ucEnumed;                                                 /*  是(1)否(0)已成功枚举设备    */
    
    USB_INT8U ucCtrlPipe;                                               /*  控制传输管道是(1)否(0)已打开*/ 
    USB_INT8U ucBulkOutPipe;
    USB_INT8U ucBulkInPipe;
    USB_INT8U ucIntrOutPipe;
    USB_INT8U ucIntrInPipe;
    
    USB_BOOL  bExitStdOperaReq;                                         /*  是否请求退出控制传输        */
    USB_BOOL  bExitDataOperaReq;                                        /*  是否请求退出批量和中断传输  */
    
} __USB_HOST_FLAGS, *__PUSB_HOST_FLAGS;

/*********************************************************************************************************
  USB 设备,接口的类型,协议等集合
*********************************************************************************************************/
typedef struct {
    USB_INT8U bDeviceClass;
    USB_INT8U bDeviceProtocol;
    USB_INT8U bInterfaceClass;
    USB_INT8U bInterfaceProtocol;
    USB_INT8U bInterfaceSubClass;
} USB_DEVICE_TYPE, *PUSB_DEVICE_TYPE;  
    
#define __USB_MAX_INTERFACE                         2                   /*  最大接口数                  */
#define __USB_MAX_EP                                32                  /*  最大端点数                  */
#define __USB_MAX_SAME_EP                           2                   /*  每类端点的最大数目          */

typedef struct __tagUSB_INTERFACE_EP_DESCR {
    USB_INT8U                ucInterfaceIndex;                          /*  接口序号                    */
    USB_INT8U                ucInterfaceNum;                            /*  该设备所具有的接口总数      */
    USB_INT8U                ucReserv[2];
    
    USB_INT8U                ucBulkInEpNum;                             /*  该接口具有的 Bulk IN 端点数 */
    USB_INT8U                ucBulkOutEpNum;                            /*  该接口具有的 Bulk OUT 端点数*/
    USB_INT8U                ucIntrInEpNum;                             /*  该接口具有的 Intr IN 端点数 */
    USB_INT8U                ucIntrOutEpNum;                            /*  该接口具有的 Intr Out 端点数*/
    USB_INT8U                ucIsoInEpNum;                              /*  该接口具有的 Iso IN 端点数  */
    USB_INT8U                ucIsoOutEpNum;                             /*  该接口具有的 Iso Out 端点数 */
    USB_INT8U                ucControlInEpNum;                          /*  该接口具有的 控制 IN 端点数 */
    USB_INT8U                ucControlOutEpNum;                         /*  该接口具有的 控制 Out 端点数*/
    
    USB_INTERFACE_DESCRIPTOR usbInterfaceDescr;                         /*  接口描述符                  */
    
    USB_ENDPOINT_DESCRIPTOR  usbBulkInEpDescr[__USB_MAX_SAME_EP];       /*  该接口下的 Bulk IN 描述符   */
    USB_ENDPOINT_DESCRIPTOR  usbBulkOutEpDescr[__USB_MAX_SAME_EP];

    USB_ENDPOINT_DESCRIPTOR  usbIntrInEpDescr[__USB_MAX_SAME_EP];
    USB_ENDPOINT_DESCRIPTOR  usbIntrOutEpDescr[__USB_MAX_SAME_EP];

    USB_ENDPOINT_DESCRIPTOR  usbIsoInEpDescr[__USB_MAX_SAME_EP];
    USB_ENDPOINT_DESCRIPTOR  usbIsoOutEpDescr[__USB_MAX_SAME_EP];
} __USB_INTERFACE_EP_DESCR, __PUSB_INTERFACE_EP_DESCR;


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
                            void      (*pDevDisconCallBack)(void));

USB_BOOL usbHostDeInit (void);
USB_BOOL usbHostNotifyExitDataOpera (void);
USB_BOOL usbHostNotifyExitStdOpera (void);

/*********************************************************************************************************
** Function name:       usbPipeOpen
** Descriptions:        打开传输管道
** input parameters:    ucTranType  传输类型
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL usbPipeOpen (USB_INT8U ucTranType);

/*********************************************************************************************************
** Function name:       usbPipeClose
** Descriptions:        关闭传输管道
** input parameters:    ucTranType  传输类型
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL usbPipeClose (USB_INT8U ucTranType);

/*********************************************************************************************************
** Function name:       usbGetStatus
** Descriptions:        读取状态请求
** input parameters:    ucType  要读取的状态的对象类型,指设备(0),接口(1),或端点(2)
**                      wIndex  索引,设备号(固定为0),或接口号,或端点号
** output parameters:   pucData 接收数据缓冲区,用于存放读取到的状态值
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbGetStatus (USB_INT8U ucType , USB_INT16U wIndex, USB_INT8U *pucData);

#define usbGetDeviceStatus(pucData)            usbGetStatus(0, 0, pucData)
#define usbGetInterfaceStatus(wIndex, pucData) usbGetStatus(0x01, wIndex, pucData)
#define usbGetEndPointStatus(wIndex, pucData)  usbGetStatus(0x02, wIndex, pucData)

/*********************************************************************************************************
** Function name:       usbClearFeature
** Descriptions:        清除特性
** input parameters:    ucType  要读取的状态的对象类型,指设备(0),接口(1),或端点(2)
**                      wValue  特性选择符
**                      wIndex  索引,设备号(固定为0),或接口号,或端点号
** output parameters:   None
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbClearFeature (USB_INT8U ucType, USB_INT16U wValue, USB_INT16U wIndex);

#define usbClearFeature_EP(wIndex) usbClearFeature (__USB_RECIPIENT_ENDPOINT, 0, wIndex)

/*********************************************************************************************************
** Function name:       usbSetFeature
** Descriptions:        设置特性
** input parameters:    ucType  要读取的状态的对象类型,指设备(0),接口(1),或端点(2)
**                      wValue  特性选择符
**                      wIndex  索引,设备号(固定为0),或接口号,或端点号
** output parameters:   None
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_BOOL usbSetFeature (USB_INT8U ucType, USB_INT16U wValue, USB_INT16U wIndex);

/*********************************************************************************************************
  OTG 增加的特性
*********************************************************************************************************/
#define usbSetFeature_b_hnp_enable()      usbSetFeature(0, __OTG_B_HNP_ENABLE, 0)
#define usbSetFeature_a_hnp_support()     usbSetFeature(0, __OTG_A_HNP_SUPPORT, 0)
#define usbSetFeature_a_alt_hnp_support() usbSetFeature(0, __OTG_A_ALT_HNP_SUPPORT, 0)

/*********************************************************************************************************
** Function name:       usbSetAddress
** Descriptions:        设置地址
** input parameters:    wValue 地址值
** output parameters:   None
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbSetAddress (USB_INT16U wValue);

/*********************************************************************************************************
** Function name:       usbGetDescription
** Descriptions:        获取描述符
** input parameters:    wValue  类型和索引
**                      wIndex  0 或语言 ID
**                      wLength 描述符长度
** output parameters:   pucData 接收描述符的缓冲区
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbGetDescription (USB_INT16U wValue, USB_INT16U wIndex, USB_INT16U wLength, USB_INT8U *pucData);

#define usbGetDeviceDescription(wLength, pucData)  \
            usbGetDescription((USB_INT16U)(0x01 << 8), 0, wLength, pucData)
        
#define usbGetConfigDescription(wValue, wIndex, wLength, pucData)  \
            usbGetDescription((USB_INT16U)(0x02 << 8) | wValue, wIndex, wLength, pucData)

#define usbGetStringDescription(wValue, wIndex, wLength, pucData)  \
            usbGetDescription((USB_INT16U)((USB_INT16U)(0x03 << 8) | wValue), wIndex, wLength, pucData)

/*********************************************************************************************************
** Function name:       usbSetDescription
** Descriptions:        设置描述符
** input parameters:    wValue  类型和索引
**                      wIndex  0 或语言ID
**                      wLength 描述符长度
** output parameters:   pucData 接收描述符的缓冲区
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbSetDescription (USB_INT16U wValue, USB_INT16U wIndex, USB_INT16U wLength, USB_INT8U *pucData);

/*********************************************************************************************************
** Function name:       usbGetConfiguratiton
** Descriptions:        读取配置值请求
** input parameters:    None
** output parameters:   pucData 配置值
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbGetConfiguratiton (USB_INT8U *pucData);

/*********************************************************************************************************
** Function name:       usbSetConfiguratiton
** Descriptions:        设置配置值请求
** input parameters:    None
** output parameters:   None
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbSetConfiguratiton (void);

/*********************************************************************************************************
** Function name:       usbGetInterface
** Descriptions:        读取指定接口的设置值,即接口描述符中的bAlternateSetting字段值
** input parameters:    wIndex  接口号
** output parameters:   pucData 返回的bAlternateSetting字段值
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbGetInterface (USB_INT16U wIndex, USB_INT8U *pucData);

/*********************************************************************************************************
** Function name:       usbSetInterface
** Descriptions:        设置接口请求,与usbGetInterface相对应
** input parameters:    wValue  可替换的设置值
**                      wIndex  接口号
** output parameters:   None
** Returned value:      传输错误码  成功 : USB_ERR_SUCESS, 其余为错误
*********************************************************************************************************/
USB_INT8U usbSetInterface (USB_INT16U wValue, USB_INT16U wIndex);

/*********************************************************************************************************
** Function name:       usbSunchFrame
** Descriptions:        同步帧请求,用于设置并报告端点的同步帧号
** input parameters:    wIndex  端点号
** output parameters:   pucData 返回的帧号
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_INT8U usbSunchFrame (USB_INT16U wIndex, USB_INT8U *pucData);

/*********************************************************************************************************
** Function name:       usbInterfaceEpConfig
** Descriptions:        根据获取到的配置描述符数据找出接口描述符及其端点描述符,
**                      并将其填充到相应的描述符结构体中
** input parameters:    pucBuf          获取到的配置描述符数据缓冲区
**                      usConfigDescLen 配置描述符总长度
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL usbInterfaceEpConfig (USB_INT8U *pucBuf, USB_INT16U usConfigDescLen);

/*********************************************************************************************************
** Function name:       usbGetEpMaxPktSize
** Descriptions:        获取端点的最大包大小
** input parameters:    ucInterfaceIndex 接口索引
**                      ucEpNum          端点号
** output parameters:   None
** Returned value:      > 0 : 所要端点的最大包大小,  0 : 失败,表示端点ucEpNum不存在
*********************************************************************************************************/
USB_INT16U usbGetEpMaxPktSize (USB_INT8U ucInterfaceIndex, USB_INT8U ucEpNum);

/*********************************************************************************************************
** Function name:       usbIsDeviceReady
** Descriptions:        判断设备是否准备好
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 设备准备好   FALSE : 设备未准备好
*********************************************************************************************************/
USB_BOOL usbIsDeviceReady (void);

/*********************************************************************************************************
** Function name:       usbIsDeviceAttach
** Descriptions:        判断设备是否插入
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 设备已插入   FALSE : 设备未准备插入
*********************************************************************************************************/
USB_BOOL usbIsDeviceAttach (void);

/*********************************************************************************************************
** Function name:       usbGetDeviceType
** Descriptions:        获取设备类型信息,包括设备类型,所支持协议等
** input parameters:    ucInterfaceIndex 接口号
** output parameters:   pusbDeviceType   设备类型
** Returned value:      TRUE : 成功   FALSE : 失败
*********************************************************************************************************/
USB_BOOL usbGetDeviceType (USB_INT8U ucInterfaceIndex, PUSB_DEVICE_TYPE pusbDeviceType);

/*********************************************************************************************************
** Function name:       usbGetEp
** Descriptions:        获取非控制端点的端点号
** input parameters:    ucInterface 接口号
**                      ucTranType  传输类型
** output parameters:   None
** Returned value:      > 0 : 端点号,  = 0 : 获取失败,可能是设备不存在该类型端点
*********************************************************************************************************/
USB_INT8U usbGetEp (USB_INT8U ucInterface, USB_INT8U ucTranType);

/*********************************************************************************************************
** Function name:       usbGetVer
** Descriptions:        取得USB主机协议栈的版本号
** input parameters:    None
** output parameters:   None
** Returned value:      32位版本号
*********************************************************************************************************/
INT32U usbGetVer (void);

extern volatile                     __USB_HOST_FLAGS GusbHostFlags;
extern __USB_HOST_EVENT_CNT         __GusbHostEvtCnt;                 
extern USB_INT16U                   __GusEpMaxPktSize[__USB_MAX_INTERFACE][32];
extern USB_DEVICE_DESCRIPTOR        GusbDeviceDescr;                    /*  设备描述符                  */
extern USB_CONFIGURATION_DESCRIPTOR GusbConfigDescr;                    /*  配置描述符                  */
extern USB_OTG_DESCRIPTOR           GusbOtgDescr;
extern __USB_INTERFACE_EP_DESCR     GusbInterEpDescr[__USB_MAX_INTERFACE];

extern void                         (*__GpEnumSucessCallBack)(void);    /*  枚举成功时回调函数          */
extern void                         (*__GpDevDisconCallBack)(void);     /*  设备拨出时回调函数          */

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
