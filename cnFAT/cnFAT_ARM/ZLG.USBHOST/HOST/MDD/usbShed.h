/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**                                     
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            usbShed.h
** Latest modified Date: 2007-11-24        
** Latest Version:       V1.0    
** Description:          
**                       
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗   Longsui Wu   
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
#ifndef __USBSHED_H
#define __USBSHED_H

#include "..\USBHostConfig.h"


/*********************************************************************************************************
  调度任务处理结果
*********************************************************************************************************/
#define __USB_TRANDEAL_OK                           0x01
#define __USB_TRANDEAL_ERR_HALT                     0x02
#define __USB_TRANDEAL_ERR_TRY                      0x03

#define __USB_MSGQEUE_LENGTH                        16                  /*  调度任务用的消息队列长度    */

/*********************************************************************************************************
  调度类型(事件类型,调度使用的消息的bit31~bit24)
*********************************************************************************************************/
#define __USB_SHED_CONCHANGE                        0x10                /*  RootHubStatusChange         */
#define __USB_SHED_DONEHEAD                         0x20                /*  WritebackDoneHead           */
#define __USB_SHED_SHEDOVERRUN                      0x30                /*  SchedulingOverrun           */
#define __USB_SHED_UNRECERROR                       0x40                /*  Unrecoverable Error         */
#define __USB_SHED_DELSELF                          0x50                /*  Delete Shelf                */
    
extern OS_EVENT *__GevtUsbMsgQeue;
extern void     *__GevtUsbMsgQeueTbl[__USB_MSGQEUE_LENGTH];

extern OS_EVENT *__GevtUsbCtrlMbox;
extern OS_EVENT *__GevtUsbDataTranMbox;

extern OS_EVENT *__GevtUsbCtrlSem;
extern OS_EVENT *__GevtUsbDataTranSem;


USB_BOOL __usbShedInit (void);
USB_BOOL __usbCreateShedTask(USB_INT8U ucShedPrio, USB_INT8U ucEnumPrio);


#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
