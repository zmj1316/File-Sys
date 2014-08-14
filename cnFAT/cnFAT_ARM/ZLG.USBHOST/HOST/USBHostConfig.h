/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            usbconfig.h
** Latest modified Date: 2007-11-06
** Latest Version:       V1.0
** Description:          USB软件包配置文件
**
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗  Longsui Wu
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
#ifndef __USBHOSTCONFIG_H
#define __USBHOSTCONFIG_H

#define ATX_ISP1301_EN              0

#define USB_HOST_PORT               1                                   /*  使用的主机端口              */
#define USB_MAX_PORTS               1                                   /*  最大主机端口数              */

#define USB_MAX_CTRL_TD             0x08                                /*  控制传输的最大TD数          */
#define USB_MAX_BULKOUT_TD          0x10                                /*  BULK OUT 传输最大 TD 数     */
#define USB_MAX_BULKIN_TD           0x10                                /*  BULK IN 传输最大 TD 数      */
#define USB_MAX_INTROUT_TD          0x02                                /*  INTR OUT 传输最大 TD 数     */
#define USB_MAX_INTRIN_TD           0x02                                /*  INTR IN 传输最大 TD 数      */


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
