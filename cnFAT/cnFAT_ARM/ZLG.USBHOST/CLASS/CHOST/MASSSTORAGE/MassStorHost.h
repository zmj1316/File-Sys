/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**                                     
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            massstorage.h
** Latest modified Date: 2007-11-26        
** Latest Version:       V1.0    
** Description:          massstorage.cͷ�ļ�
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
#ifndef __MASSSTORHOST_H
#define __MASSSTORHOST_H

/*********************************************************************************************************
  LUN �豸��Ϣ�ṹ��
*********************************************************************************************************/
typedef struct __tagMS_LUN_INFO {
    USB_INT32U uiMaxLba;                                                /*  ����߼����ַ			    */
    USB_INT32U uiBytesPerBlock;                                         /*  ÿ���ֽ���				    */
} __MS_LUN_INFO, *__PMS_LUN_INFO;

/*********************************************************************************************************
  ���������豸��Ϣ�ṹ��
*********************************************************************************************************/
typedef struct __tagMS_INFO {
    USB_INT8U     ucMaxLun;                                             /*  ׼���õ�LUN����             */
    USB_INT8U     ucUsedLun[15];                                        /*  ��0~F,�������СӦΪ16,     */
                                                                        /*  ...������15                 */
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
#define MS_ERR_EXIT_REQ                 0x43                            /*  �ϲ�����Ѿ������˳�����    */

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
** Descriptions:        ���������豸��ʼ��
** input parameters:    ucInterfaceIndex : �ӿں�
** output parameters:   None
** Returned value:      �������,������MS_ERR_SUCESS, ˵��ִ�гɹ�, ����ִ��ʧ��
*********************************************************************************************************/
USB_INT8U msHostInit (USB_INT8U ucInterfaceIndex);

/*********************************************************************************************************
** Function name:       msHostDeInit
** Descriptions:        ���������豸ж��
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE: �ɹ�, FALSE: ʧ��
*********************************************************************************************************/
USB_BOOL msHostDeInit (void);

#ifdef __cplusplus
 }
#endif

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/



