/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**                                     
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            hostChap9.c
** Latest modified Date: 2007-11-10        
** Latest Version:       V1.0    
** Description:          �ھ���Э���е���ض���
**                       
**--------------------------------------------------------------------------------------------------------
** Created By:           ��¡��  Longsui Wu   
** Created date:         2007-11-10    
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
#ifndef __HOSTCHAP9_H
#define __HOSTCHAP9_H

/*********************************************************************************************************
  USB ����
*********************************************************************************************************/
typedef struct __tagUSB_DEV_REQ {
    USB_INT8U  bmRequestType;                                           /*  ��������                    */
    USB_INT8U  bRequest;                                                /*  �����                      */
    USB_INT16U wValue;
    USB_INT16U wIndex;
    USB_INT16U wLength;
} USB_DEV_REQ, *PUSB_DEV_REQ;


#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/

