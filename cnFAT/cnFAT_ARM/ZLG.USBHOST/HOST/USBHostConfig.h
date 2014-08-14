/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            usbconfig.h
** Latest modified Date: 2007-11-06
** Latest Version:       V1.0
** Description:          USB����������ļ�
**
**--------------------------------------------------------------------------------------------------------
** Created By:           ��¡��  Longsui Wu
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
#ifndef __USBHOSTCONFIG_H
#define __USBHOSTCONFIG_H

#define ATX_ISP1301_EN              0

#define USB_HOST_PORT               1                                   /*  ʹ�õ������˿�              */
#define USB_MAX_PORTS               1                                   /*  ��������˿���              */

#define USB_MAX_CTRL_TD             0x08                                /*  ���ƴ�������TD��          */
#define USB_MAX_BULKOUT_TD          0x10                                /*  BULK OUT ������� TD ��     */
#define USB_MAX_BULKIN_TD           0x10                                /*  BULK IN ������� TD ��      */
#define USB_MAX_INTROUT_TD          0x02                                /*  INTR OUT ������� TD ��     */
#define USB_MAX_INTRIN_TD           0x02                                /*  INTR IN ������� TD ��      */


#if ((USB_HOST_PORT < 1) || (USB_HOST_PORT > USB_MAX_PORTS))
    #error USBHostConfig.h: USB_HOST_PORT must greater than 1 and smaller than USB_MAX_PORTS
#endif

#if (USB_MAX_CTRL_TD < 1)
    #error USBHostConfig.h: USB_MAX_CTRL_TD must greater than 1
#endif

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
