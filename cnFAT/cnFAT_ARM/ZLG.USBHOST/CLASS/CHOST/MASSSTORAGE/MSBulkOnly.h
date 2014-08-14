/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**                                     
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            msmulkonly.h
** Latest modified Date: 2007-11-06        
** Latest Version:       V1.0    
** Description:          
**                       
**--------------------------------------------------------------------------------------------------------
** Created By:           ��¡��         Longsui Wu   
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
#ifndef __MSBULKONLY_H
#define __MSBULKONLY_H

/*********************************************************************************************************
  CBW�ṹ����������
*********************************************************************************************************/
typedef struct __tagBULK_ONLY_CBW {
    USB_INT8U dCBWSignature0;                                           /*  dCBWSignature,Ϊ�����С�˲�*/
    USB_INT8U dCBWSignature1;                                           /*  ��д,0Ϊ����ֽ�,3Ϊ����ֽ�*/
    USB_INT8U dCBWSignature2;
    USB_INT8U dCBWSignature3;
    
    USB_INT8U dCBWTag0;
    USB_INT8U dCBWTag1;
    USB_INT8U dCBWTag2;
    USB_INT8U dCBWTag3;
    
    USB_INT8U dCBWDataTransferLength0;
    USB_INT8U dCBWDataTransferLength1;
    USB_INT8U dCBWDataTransferLength2;
    USB_INT8U dCBWDataTransferLength3;
    
    USB_INT8U bmCBWFlags;
    USB_INT8U bCBWLUN;
    USB_INT8U bCBWCBLength;
    USB_INT8U CBWCB[16];
} __BULK_ONLY_CBW, *__PBULK_ONLY_CBW;

extern OS_EVENT *__GevtBoSem;


USB_BOOL  __boInit (void);
USB_BOOL  __boDeInit (void);
USB_INT8U __boMassStorReset (USB_INT16U wIndex);
USB_INT8U __boGetMaxLun (USB_INT16U wIndex, USB_INT8U *pucData);
USB_INT8U __boSendCommand (__PBULK_ONLY_CBW pcbwInq, USB_INT8U *pucData);

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/



