/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**                                     
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            massstorage.h
** Latest modified Date: 2007-11-26        
** Latest Version:       V1.0    
** Description:          massstorage.c头文件
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
#ifndef __MASSSTORHOST_H
#define __MASSSTORHOST_H

/*********************************************************************************************************
  LUN 设备信息结构体
*********************************************************************************************************/
typedef struct __tagMS_LUN_INFO {
    USB_INT32U uiMaxLba;                                                /*  最大逻辑块地址			    */
    USB_INT32U uiBytesPerBlock;                                         /*  每块字节数				    */
} __MS_LUN_INFO, *__PMS_LUN_INFO;

/*********************************************************************************************************
  大容量类设备信息结构体
*********************************************************************************************************/
typedef struct __tagMS_INFO {
    USB_INT8U     ucMaxLun;                                             /*  准备好的LUN总数             */
    USB_INT8U     ucUsedLun[15];                                        /*  若0~F,则数组大小应为16,     */
                                                                        /*  ...而不是15                 */
    __MS_LUN_INFO msLunInfo[MS_MAX_LUN];
} __MS_INFO, *__PMS_INFO;


#define USBMSC_INTERFACE_CLASS          0x08

#define USBMSC_SUBCLASS_RBC             0x01
#define USBMSC_SUBCLASS_SFF8020I        0x02
#define USBMSC_SUBCLASS_QIC157          0x03
#define USBMSC_SUBCLASS_UFI             0x04
#define USBMSC_SUBCLASS_SFF8070I        0x05
#define USBMSC_SUBCLASS_SCSI            0x06
#define USBMSC_SUBCLASS_RESERVED        0xff

#define USBMSC_INTERFACE_PROTOCOL_CBIT  0x00
#define USBMSC_INTERFACE_PROTOCOL_CBT   0x01
#define USBMSC_INTERFACE_PROTOCOL_BOT   0x50

#define __MSLSB(uiData, ucIndex)        (USB_INT8U)(((uiData) >> ((ucIndex) * 8)) & 0xFF)

#define MS_ERR_SUCESS                   0x00
#define MS_ERR_CSW                      0x40
#define MS_ERR_STATUS                   0x41

#define MS_ERR_INVALID_PARAM            0x42
#define MS_ERR_EXIT_REQ                 0x43                            /*  上层软件已经请求退出操作    */

#define MS_ERR_GET_MAXLUN               0x44
#define MS_ERR_DEVICE_NOT_READY         0x45
#define MS_ERR_NONE_DEVICE              0x46
#define MS_ERR_DEVICE_NOT_SUPPORT       0x47

#define MS_ERR_ENVIRONMENT              0x50


extern __MS_INFO __msInfo;

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
USB_INT8U msHostInit (USB_INT8U ucInterfaceIndex);

/*********************************************************************************************************
** Function name:       msHostDeInit
** Descriptions:        大容量类设备卸载
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE: 成功, FALSE: 失败
*********************************************************************************************************/
USB_BOOL msHostDeInit (void);

#ifdef __cplusplus
 }
#endif

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/



