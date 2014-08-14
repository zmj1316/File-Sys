/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**                                     
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            usbd.h
** Latest modified Date: 2007-11-15        
** Latest Version:       V1.0    
** Description:          USBD.c��ͷ�ļ�
**                       
**--------------------------------------------------------------------------------------------------------
** Created By:           ��¡��  Longsui Wu   
** Created date:         2007-11-15    
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
#ifndef __USBD_H
#define __USBD_H

#include "..\USBHostConfig.h"


#define USB_TRAN_TYPE_CONTROL                   0x01                    /*  ��������                    */

#define USB_TRAN_TYPE_BULK_OUT                  0x02
#define USB_TRAN_TYPE_BULK_IN                   0x82

#define USB_TRAN_TYPE_INTR_OUT                  0x03
#define USB_TRAN_TYPE_INTR_IN                   0x83

#define USB_TRAN_TYPE_ISO                       0x04


/*********************************************************************************************************
  USB ����״̬��־λ
*********************************************************************************************************/
typedef struct __tagUSB_HOST_FLAGS {
    
    USB_INT8U ucAttached;                                               /*  �豸��(1)��(0)�Ѳ���        */
    USB_INT8U ucConfiged;                                               /*  ��(1)��(0)�������豸        */
    USB_INT8U ucEnumed;                                                 /*  ��(1)��(0)�ѳɹ�ö���豸    */
    
    USB_INT8U ucCtrlPipe;                                               /*  ���ƴ���ܵ���(1)��(0)�Ѵ�*/ 
    USB_INT8U ucBulkOutPipe;
    USB_INT8U ucBulkInPipe;
    USB_INT8U ucIntrOutPipe;
    USB_INT8U ucIntrInPipe;
    
    USB_BOOL  bExitStdOperaReq;                                         /*  �Ƿ������˳����ƴ���        */
    USB_BOOL  bExitDataOperaReq;                                        /*  �Ƿ������˳��������жϴ���  */
    
} __USB_HOST_FLAGS, *__PUSB_HOST_FLAGS;

/*********************************************************************************************************
  USB �豸,�ӿڵ�����,Э��ȼ���
*********************************************************************************************************/
typedef struct {
    USB_INT8U bDeviceClass;
    USB_INT8U bDeviceProtocol;
    USB_INT8U bInterfaceClass;
    USB_INT8U bInterfaceProtocol;
    USB_INT8U bInterfaceSubClass;
} USB_DEVICE_TYPE, *PUSB_DEVICE_TYPE;  
    
#define __USB_MAX_INTERFACE                         2                   /*  ���ӿ���                  */
#define __USB_MAX_EP                                32                  /*  ���˵���                  */
#define __USB_MAX_SAME_EP                           2                   /*  ÿ��˵�������Ŀ          */

typedef struct __tagUSB_INTERFACE_EP_DESCR {
    USB_INT8U                ucInterfaceIndex;                          /*  �ӿ����                    */
    USB_INT8U                ucInterfaceNum;                            /*  ���豸�����еĽӿ�����      */
    USB_INT8U                ucReserv[2];
    
    USB_INT8U                ucBulkInEpNum;                             /*  �ýӿھ��е� Bulk IN �˵��� */
    USB_INT8U                ucBulkOutEpNum;                            /*  �ýӿھ��е� Bulk OUT �˵���*/
    USB_INT8U                ucIntrInEpNum;                             /*  �ýӿھ��е� Intr IN �˵��� */
    USB_INT8U                ucIntrOutEpNum;                            /*  �ýӿھ��е� Intr Out �˵���*/
    USB_INT8U                ucIsoInEpNum;                              /*  �ýӿھ��е� Iso IN �˵���  */
    USB_INT8U                ucIsoOutEpNum;                             /*  �ýӿھ��е� Iso Out �˵��� */
    USB_INT8U                ucControlInEpNum;                          /*  �ýӿھ��е� ���� IN �˵��� */
    USB_INT8U                ucControlOutEpNum;                         /*  �ýӿھ��е� ���� Out �˵���*/
    
    USB_INTERFACE_DESCRIPTOR usbInterfaceDescr;                         /*  �ӿ�������                  */
    
    USB_ENDPOINT_DESCRIPTOR  usbBulkInEpDescr[__USB_MAX_SAME_EP];       /*  �ýӿ��µ� Bulk IN ������   */
    USB_ENDPOINT_DESCRIPTOR  usbBulkOutEpDescr[__USB_MAX_SAME_EP];

    USB_ENDPOINT_DESCRIPTOR  usbIntrInEpDescr[__USB_MAX_SAME_EP];
    USB_ENDPOINT_DESCRIPTOR  usbIntrOutEpDescr[__USB_MAX_SAME_EP];

    USB_ENDPOINT_DESCRIPTOR  usbIsoInEpDescr[__USB_MAX_SAME_EP];
    USB_ENDPOINT_DESCRIPTOR  usbIsoOutEpDescr[__USB_MAX_SAME_EP];
} __USB_INTERFACE_EP_DESCR, __PUSB_INTERFACE_EP_DESCR;


/*********************************************************************************************************
** Function name:       usbHostInitialize
** Descriptions:        USB ������ʼ��
** input parameters:    ucShedPrio           ������������ȼ�
**                      ucEnumPrio           ö����������ȼ�
**                      pEnumSucessCallBack: ö�ٳɹ��ص�����,��û��������ΪNULL
**                      pDevDisconCallBack:  �豸�����ص�����,��û��������ΪNULL
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
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
** Descriptions:        �򿪴���ܵ�
** input parameters:    ucTranType  ��������
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL usbPipeOpen (USB_INT8U ucTranType);

/*********************************************************************************************************
** Function name:       usbPipeClose
** Descriptions:        �رմ���ܵ�
** input parameters:    ucTranType  ��������
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL usbPipeClose (USB_INT8U ucTranType);

/*********************************************************************************************************
** Function name:       usbGetStatus
** Descriptions:        ��ȡ״̬����
** input parameters:    ucType  Ҫ��ȡ��״̬�Ķ�������,ָ�豸(0),�ӿ�(1),��˵�(2)
**                      wIndex  ����,�豸��(�̶�Ϊ0),��ӿں�,��˵��
** output parameters:   pucData �������ݻ�����,���ڴ�Ŷ�ȡ����״ֵ̬
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
*********************************************************************************************************/
USB_INT8U usbGetStatus (USB_INT8U ucType , USB_INT16U wIndex, USB_INT8U *pucData);

#define usbGetDeviceStatus(pucData)            usbGetStatus(0, 0, pucData)
#define usbGetInterfaceStatus(wIndex, pucData) usbGetStatus(0x01, wIndex, pucData)
#define usbGetEndPointStatus(wIndex, pucData)  usbGetStatus(0x02, wIndex, pucData)

/*********************************************************************************************************
** Function name:       usbClearFeature
** Descriptions:        �������
** input parameters:    ucType  Ҫ��ȡ��״̬�Ķ�������,ָ�豸(0),�ӿ�(1),��˵�(2)
**                      wValue  ����ѡ���
**                      wIndex  ����,�豸��(�̶�Ϊ0),��ӿں�,��˵��
** output parameters:   None
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
*********************************************************************************************************/
USB_INT8U usbClearFeature (USB_INT8U ucType, USB_INT16U wValue, USB_INT16U wIndex);

#define usbClearFeature_EP(wIndex) usbClearFeature (__USB_RECIPIENT_ENDPOINT, 0, wIndex)

/*********************************************************************************************************
** Function name:       usbSetFeature
** Descriptions:        ��������
** input parameters:    ucType  Ҫ��ȡ��״̬�Ķ�������,ָ�豸(0),�ӿ�(1),��˵�(2)
**                      wValue  ����ѡ���
**                      wIndex  ����,�豸��(�̶�Ϊ0),��ӿں�,��˵��
** output parameters:   None
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
*********************************************************************************************************/
USB_BOOL usbSetFeature (USB_INT8U ucType, USB_INT16U wValue, USB_INT16U wIndex);

/*********************************************************************************************************
  OTG ���ӵ�����
*********************************************************************************************************/
#define usbSetFeature_b_hnp_enable()      usbSetFeature(0, __OTG_B_HNP_ENABLE, 0)
#define usbSetFeature_a_hnp_support()     usbSetFeature(0, __OTG_A_HNP_SUPPORT, 0)
#define usbSetFeature_a_alt_hnp_support() usbSetFeature(0, __OTG_A_ALT_HNP_SUPPORT, 0)

/*********************************************************************************************************
** Function name:       usbSetAddress
** Descriptions:        ���õ�ַ
** input parameters:    wValue ��ֵַ
** output parameters:   None
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
*********************************************************************************************************/
USB_INT8U usbSetAddress (USB_INT16U wValue);

/*********************************************************************************************************
** Function name:       usbGetDescription
** Descriptions:        ��ȡ������
** input parameters:    wValue  ���ͺ�����
**                      wIndex  0 ������ ID
**                      wLength ����������
** output parameters:   pucData �����������Ļ�����
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
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
** Descriptions:        ����������
** input parameters:    wValue  ���ͺ�����
**                      wIndex  0 ������ID
**                      wLength ����������
** output parameters:   pucData �����������Ļ�����
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
*********************************************************************************************************/
USB_INT8U usbSetDescription (USB_INT16U wValue, USB_INT16U wIndex, USB_INT16U wLength, USB_INT8U *pucData);

/*********************************************************************************************************
** Function name:       usbGetConfiguratiton
** Descriptions:        ��ȡ����ֵ����
** input parameters:    None
** output parameters:   pucData ����ֵ
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
*********************************************************************************************************/
USB_INT8U usbGetConfiguratiton (USB_INT8U *pucData);

/*********************************************************************************************************
** Function name:       usbSetConfiguratiton
** Descriptions:        ��������ֵ����
** input parameters:    None
** output parameters:   None
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
*********************************************************************************************************/
USB_INT8U usbSetConfiguratiton (void);

/*********************************************************************************************************
** Function name:       usbGetInterface
** Descriptions:        ��ȡָ���ӿڵ�����ֵ,���ӿ��������е�bAlternateSetting�ֶ�ֵ
** input parameters:    wIndex  �ӿں�
** output parameters:   pucData ���ص�bAlternateSetting�ֶ�ֵ
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
*********************************************************************************************************/
USB_INT8U usbGetInterface (USB_INT16U wIndex, USB_INT8U *pucData);

/*********************************************************************************************************
** Function name:       usbSetInterface
** Descriptions:        ���ýӿ�����,��usbGetInterface���Ӧ
** input parameters:    wValue  ���滻������ֵ
**                      wIndex  �ӿں�
** output parameters:   None
** Returned value:      ���������  �ɹ� : USB_ERR_SUCESS, ����Ϊ����
*********************************************************************************************************/
USB_INT8U usbSetInterface (USB_INT16U wValue, USB_INT16U wIndex);

/*********************************************************************************************************
** Function name:       usbSunchFrame
** Descriptions:        ͬ��֡����,�������ò�����˵��ͬ��֡��
** input parameters:    wIndex  �˵��
** output parameters:   pucData ���ص�֡��
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
USB_INT8U usbSunchFrame (USB_INT16U wIndex, USB_INT8U *pucData);

/*********************************************************************************************************
** Function name:       usbInterfaceEpConfig
** Descriptions:        ���ݻ�ȡ�������������������ҳ��ӿ�����������˵�������,
**                      ��������䵽��Ӧ���������ṹ����
** input parameters:    pucBuf          ��ȡ�����������������ݻ�����
**                      usConfigDescLen �����������ܳ���
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL usbInterfaceEpConfig (USB_INT8U *pucBuf, USB_INT16U usConfigDescLen);

/*********************************************************************************************************
** Function name:       usbGetEpMaxPktSize
** Descriptions:        ��ȡ�˵��������С
** input parameters:    ucInterfaceIndex �ӿ�����
**                      ucEpNum          �˵��
** output parameters:   None
** Returned value:      > 0 : ��Ҫ�˵��������С,  0 : ʧ��,��ʾ�˵�ucEpNum������
*********************************************************************************************************/
USB_INT16U usbGetEpMaxPktSize (USB_INT8U ucInterfaceIndex, USB_INT8U ucEpNum);

/*********************************************************************************************************
** Function name:       usbIsDeviceReady
** Descriptions:        �ж��豸�Ƿ�׼����
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : �豸׼����   FALSE : �豸δ׼����
*********************************************************************************************************/
USB_BOOL usbIsDeviceReady (void);

/*********************************************************************************************************
** Function name:       usbIsDeviceAttach
** Descriptions:        �ж��豸�Ƿ����
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : �豸�Ѳ���   FALSE : �豸δ׼������
*********************************************************************************************************/
USB_BOOL usbIsDeviceAttach (void);

/*********************************************************************************************************
** Function name:       usbGetDeviceType
** Descriptions:        ��ȡ�豸������Ϣ,�����豸����,��֧��Э���
** input parameters:    ucInterfaceIndex �ӿں�
** output parameters:   pusbDeviceType   �豸����
** Returned value:      TRUE : �ɹ�   FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL usbGetDeviceType (USB_INT8U ucInterfaceIndex, PUSB_DEVICE_TYPE pusbDeviceType);

/*********************************************************************************************************
** Function name:       usbGetEp
** Descriptions:        ��ȡ�ǿ��ƶ˵�Ķ˵��
** input parameters:    ucInterface �ӿں�
**                      ucTranType  ��������
** output parameters:   None
** Returned value:      > 0 : �˵��,  = 0 : ��ȡʧ��,�������豸�����ڸ����Ͷ˵�
*********************************************************************************************************/
USB_INT8U usbGetEp (USB_INT8U ucInterface, USB_INT8U ucTranType);

/*********************************************************************************************************
** Function name:       usbGetVer
** Descriptions:        ȡ��USB����Э��ջ�İ汾��
** input parameters:    None
** output parameters:   None
** Returned value:      32λ�汾��
*********************************************************************************************************/
INT32U usbGetVer (void);

extern volatile                     __USB_HOST_FLAGS GusbHostFlags;
extern __USB_HOST_EVENT_CNT         __GusbHostEvtCnt;                 
extern USB_INT16U                   __GusEpMaxPktSize[__USB_MAX_INTERFACE][32];
extern USB_DEVICE_DESCRIPTOR        GusbDeviceDescr;                    /*  �豸������                  */
extern USB_CONFIGURATION_DESCRIPTOR GusbConfigDescr;                    /*  ����������                  */
extern USB_OTG_DESCRIPTOR           GusbOtgDescr;
extern __USB_INTERFACE_EP_DESCR     GusbInterEpDescr[__USB_MAX_INTERFACE];

extern void                         (*__GpEnumSucessCallBack)(void);    /*  ö�ٳɹ�ʱ�ص�����          */
extern void                         (*__GpDevDisconCallBack)(void);     /*  �豸����ʱ�ص�����          */

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
