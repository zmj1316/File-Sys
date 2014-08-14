/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            rbc.h
** Latest modified Date: 2007-11-16
** Latest Version:       V1.0
** Description:          rbc.c��ͷ�ļ�,����SCSI������,�б����ݼ�rbc.c��ʵ�ֵ�SCSI����ĺ�������
**
**--------------------------------------------------------------------------------------------------------
** Created By:           ��¡��   Longsui Wu
** Created date:         2007-11-16
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
  CSW��bmCSWFlagsֵ����
*********************************************************************************************************/
#define CSW_FLAG_SUCCESS            0x00                                /* ����ִ�гɹ�                 */
#define CSW_FLAG_FAIL               0x01                                /* ����ִ��ʧ��                 */
#define CSW_FLAG_PHASE_ERR          0x02                                /* �׶δ���                     */

/*********************************************************************************************************
  SCSI ����
*********************************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************************
** Function name:       rbcInquiry
** Descriptions:        ��ѯ�豸��Ϣ
** input parameters:    ucInterfaceIndex : �ӿں�
**                      ucLunIndex       : LUN��
** output parameters:   pucData          : ��ȡ�����豸��Ϣ
** Returned value:      �������,������MS_ERR_SUCESS, ˵��ִ�гɹ�, ����ִ��ʧ��
*********************************************************************************************************/
USB_INT8U rbcInquiry (USB_INT16U usInterface, USB_INT8U ucLunIndex, USB_INT8U *pucData);

/*********************************************************************************************************
** Function name:       rbcReadCapacity
** Descriptions:        ��ȡ�豸����
** input parameters:    usInterface      : �ӿں�
**                      ucLunIndex       : LUN��
** output parameters:   puiMaxLba        : ���LBA
**                      puiBytesPerBlock : ÿ���ֽ���
** Returned value:      �������,������MS_ERR_SUCESS, ˵��ִ�гɹ�, ����ִ��ʧ��
*********************************************************************************************************/
USB_INT8U rbcReadCapacity (USB_INT16U  usInterface,
                           USB_INT8U   ucLunIndex,
                           USB_INT32U *puiMaxLba,
                           USB_INT32U *puiBytesPerBlock);

/*********************************************************************************************************
** Function name:       rbcStartStopUnit
** Descriptions:        ������ֹͣ�豸(LUN)
** input parameters:    ucInterfaceIndex : �ӿں�
**                      ucLunIndex       : LUN��
**                      ucStatus         : ״̬,����(RBC_MAKEREADY)��ֹͣ(RBC_STOPMEDIUM) LUN
** output parameters:   None
** Returned value:      �������,������MS_ERR_SUCESS, ˵��ִ�гɹ�, ����ִ��ʧ��
*********************************************************************************************************/
USB_INT8U rbcStartStopUnit (USB_INT16U ucInterfaceIndex,
                            USB_INT8U  ucLunIndex,
                            USB_INT8U  ucStatus);

/*********************************************************************************************************
** Function name:       rbcRead10
** Descriptions:        ��ȡ����
** input parameters:    usInterface : �ӿں�
**                      ucLunIndex  : LUN��
**                      uiLba       : ���ַ
**                      uiTranLen   : ÿ���ֽ���
** output parameters:   pucData     : �������ݻ�����
** Returned value:      �������,������MS_ERR_SUCESS, ˵��ִ�гɹ�, ����ִ��ʧ��
*********************************************************************************************************/
USB_INT8U rbcRead10 (USB_INT16U  usInterface,
                      USB_INT8U  ucLunIndex,
                      USB_INT32U uiLba,                                 /*  LBA,���߼����ַ            */
                      USB_INT32U uiTranLen,                             /*  Ҫ��ȡ�����ݳ���            */
                      USB_INT8U *pucData);                              /*  �������ݻ�����              */

/*********************************************************************************************************
** Function name:       rbcWrite10
** Descriptions:        ��ȡ����
** input parameters:    usInterface : �ӿں�
**                      ucLunIndex  : LUN��
**                      uiLba       : ���ַ
**                      uiTranLen   : ÿ���ֽ���
** output parameters:   pucData     : �������ݻ�����
** Returned value:      �������,������MS_ERR_SUCESS, ˵��ִ�гɹ�, ����ִ��ʧ��
*********************************************************************************************************/
USB_INT8U rbcWrite10 (USB_INT16U  usInterface,
                       USB_INT8U  ucLunIndex,
                       USB_INT32U uiLba,                                /*  LBA,���߼����ַ            */
                       USB_INT32U uiTranLen,                            /*  Ҫ��ȡ�����ݳ���            */
                       USB_INT8U *pucData);                             /*  �������ݻ�����              */

/*********************************************************************************************************
** Function name:       rbcRequestSense
** Descriptions:        ��ȡ�б�����
** input parameters:    ucLunIndex : LUN��
** output parameters:   None
** Returned value:      �������,������MS_ERR_SUCESS, ˵��ִ�гɹ�, ����ִ��ʧ��
*********************************************************************************************************/
USB_INT8U rbcRequestSense (USB_INT8U ucLunIndex);

/*********************************************************************************************************
** Function name:       rbcTestUintReady
** Descriptions:        �����豸�Ƿ�׼����
** input parameters:    usInterface : �ӿں�
**                      ucLunIndex  : LUN��
** output parameters:   None
** Returned value:      �������,������MS_ERR_SUCESS, ˵��ִ�гɹ�, ����ִ��ʧ��
*********************************************************************************************************/
USB_INT8U rbcTestUintReady (USB_INT16U usInterface, USB_INT8U ucLunIndex);

#ifdef __cplusplus
 }
#endif

/*********************************************************************************************************
** Function name:       __rbcInit
** Descriptions:        RBC�������ʼ��
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __rbcInit (void);

/*********************************************************************************************************
** Function name:       __rbcDeInit
** Descriptions:        RBC�����ж��
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE: �ɹ�, FALSE: ʧ��
*********************************************************************************************************/
USB_BOOL __rbcDeInit (void);

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/



