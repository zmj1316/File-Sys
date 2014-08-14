/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            rbc.h
** Latest modified Date: 2007-11-16
** Latest Version:       V1.0
** Description:          rbc.c的头文件,包含SCSI的命令,判别数据及rbc.c所实现的SCSI命令的函数声明
**
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗   Longsui Wu
** Created date:         2007-11-16
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
#ifndef __HOST_RBC_H
#define __HOST_RBC_H

#include "..\..\..\common\usbcommon.h"
    
/*********************************************************************************************************
  SCSI-2 commands
*********************************************************************************************************/
#define SCSI_TEST_UNIT_READY        0x00
#define SCSI_REQUEST_SENSE          0x03
#define SCSI_INQUIRY                0x12
#define SCSI_SEND_DIAGNOSTIC        0x1D

/*********************************************************************************************************
  mandatory device-specific SCSI-2 commands
*********************************************************************************************************/
#define SCSI_READ10                 0x28
#define SCSI_READ_CAPACITY          0x25

/*********************************************************************************************************
  optional device-specific SCSI-2 commands
*********************************************************************************************************/
#define SCSI_MODE_SELECT6           0x15
#define SCSI_MODE_SENSE6            0x1A
#define SCSI_START_STOP             0x1B
#define SCSI_WRITE10                0x2A
#define SCSI_MODE_SELECT10          0x55
#define SCSI_MODE_SENSE10           0x5A

/*********************************************************************************************************
  ATAPI (CD-ROM) commands
*********************************************************************************************************/
#define SCSI_CD_READ_TOC            0x43
#define SCSI_CD_PLAY10              0x45
#define SCSI_CD_PLAY_MSF            0x47
#define SCSI_CD_PAUSE_RESUME        0x4B
#define SCSI_CD_STOP                0x4E

/*********************************************************************************************************
  mode pages
*********************************************************************************************************/
#define MODE_PAGE_FLEXIBLE_DISK     0x05
#define MODE_PAGE_CDROM             0x0D
#define MODE_PAGE_CDROM_AUDIO       0x0E
#define MODE_PAGE_CDROM_CAPS        0x2A

/*********************************************************************************************************
  SCSI-2 sense keys
*********************************************************************************************************/
#define SENSE_NONE                  0x00
#define SENSE_RECOVERED_ERROR       0x01
#define SENSE_NOT_READY             0x02
#define SENSE_MEDIUM_ERROR          0x03
#define SENSE_HARDWARE_ERROR        0x04
#define SENSE_ILLEGAL_REQUEST       0x05
#define SENSE_UNIT_ATTENTION        0x06
#define SENSE_DATA_PROTECT          0x07
#define SENSE_BLANK_CHECK           0x08

/*********************************************************************************************************
  SCSI-2 ASC
*********************************************************************************************************/
#define ASC_LUN                     0x04
#define ASC_INVALID_COMMAND_FIELD   0x24
#define ASC_MEDIA_CHANGED           0x28
#define ASC_RESET                   0x29
#define ASC_COMMANDS_CLEARED        0x2F
#define ASC_MEDIUM_NOT_PRESENT      0x3A

#define RBC_STOPMEDIUM              0x00
#define RBC_MAKEREADY               0x01
#define RBC_UNLOADMEDIUM            0x02
#define RBC_LOADMEDIUM              0x03

#define SCSI_CHECK_CONDITION        0x02


#define SCSI_ERR_CMDEXE             0x40

/*********************************************************************************************************
  CSW中bmCSWFlags值定义
*********************************************************************************************************/
#define CSW_FLAG_SUCCESS            0x00                                /* 命令执行成功                 */
#define CSW_FLAG_FAIL               0x01                                /* 命令执行失败                 */
#define CSW_FLAG_PHASE_ERR          0x02                                /* 阶段错误                     */

/*********************************************************************************************************
  SCSI 命令
*********************************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************************
** Function name:       rbcInquiry
** Descriptions:        查询设备信息
** input parameters:    ucInterfaceIndex : 接口号
**                      ucLunIndex       : LUN号
** output parameters:   pucData          : 读取到的设备信息
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U rbcInquiry (USB_INT16U usInterface, USB_INT8U ucLunIndex, USB_INT8U *pucData);

/*********************************************************************************************************
** Function name:       rbcReadCapacity
** Descriptions:        读取设备容量
** input parameters:    usInterface      : 接口号
**                      ucLunIndex       : LUN号
** output parameters:   puiMaxLba        : 最大LBA
**                      puiBytesPerBlock : 每块字节数
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U rbcReadCapacity (USB_INT16U  usInterface,
                           USB_INT8U   ucLunIndex,
                           USB_INT32U *puiMaxLba,
                           USB_INT32U *puiBytesPerBlock);

/*********************************************************************************************************
** Function name:       rbcStartStopUnit
** Descriptions:        启动或停止设备(LUN)
** input parameters:    ucInterfaceIndex : 接口号
**                      ucLunIndex       : LUN号
**                      ucStatus         : 状态,启动(RBC_MAKEREADY)或停止(RBC_STOPMEDIUM) LUN
** output parameters:   None
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U rbcStartStopUnit (USB_INT16U ucInterfaceIndex,
                            USB_INT8U  ucLunIndex,
                            USB_INT8U  ucStatus);

/*********************************************************************************************************
** Function name:       rbcRead10
** Descriptions:        读取数据
** input parameters:    usInterface : 接口号
**                      ucLunIndex  : LUN号
**                      uiLba       : 块地址
**                      uiTranLen   : 每块字节数
** output parameters:   pucData     : 接收数据缓冲区
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U rbcRead10 (USB_INT16U  usInterface,
                      USB_INT8U  ucLunIndex,
                      USB_INT32U uiLba,                                 /*  LBA,即逻辑块地址            */
                      USB_INT32U uiTranLen,                             /*  要读取的数据长度            */
                      USB_INT8U *pucData);                              /*  接收数据缓冲区              */

/*********************************************************************************************************
** Function name:       rbcWrite10
** Descriptions:        读取数据
** input parameters:    usInterface : 接口号
**                      ucLunIndex  : LUN号
**                      uiLba       : 块地址
**                      uiTranLen   : 每块字节数
** output parameters:   pucData     : 接收数据缓冲区
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U rbcWrite10 (USB_INT16U  usInterface,
                       USB_INT8U  ucLunIndex,
                       USB_INT32U uiLba,                                /*  LBA,即逻辑块地址            */
                       USB_INT32U uiTranLen,                            /*  要读取的数据长度            */
                       USB_INT8U *pucData);                             /*  接收数据缓冲区              */

/*********************************************************************************************************
** Function name:       rbcRequestSense
** Descriptions:        读取判别数据
** input parameters:    ucLunIndex : LUN号
** output parameters:   None
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U rbcRequestSense (USB_INT8U ucLunIndex);

/*********************************************************************************************************
** Function name:       rbcTestUintReady
** Descriptions:        测试设备是否准备好
** input parameters:    usInterface : 接口号
**                      ucLunIndex  : LUN号
** output parameters:   None
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U rbcTestUintReady (USB_INT16U usInterface, USB_INT8U ucLunIndex);

#ifdef __cplusplus
 }
#endif

/*********************************************************************************************************
** Function name:       __rbcInit
** Descriptions:        RBC层软件初始化
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __rbcInit (void);

/*********************************************************************************************************
** Function name:       __rbcDeInit
** Descriptions:        RBC层软件卸载
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE: 成功, FALSE: 失败
*********************************************************************************************************/
USB_BOOL __rbcDeInit (void);

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/



