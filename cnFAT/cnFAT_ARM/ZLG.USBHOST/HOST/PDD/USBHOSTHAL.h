/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**                                     
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            USBHAL.h
** Latest modified Date: 2007-07-08        
** Latest Version:       V1.0    
** Description:          USBHAL.h, Ӳ�������ͷ�ļ�,���� USB �豸����������
**                       Hardware abstract layer Header file
**--------------------------------------------------------------------------------------------------------
** Created By:           ��¡�� Longsui Wu
** Created date:         2007-10-20    
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
#ifndef __USBHAL_H
#define __USBHAL_H

#define __USB_HOST_CLK_EN         (0x01 << 0)
#define __USB_DEV_CLK_EN          (0x01 << 1)
#define __USB_I2C_CLK_EN          (0x01 << 2)
#define __USB_OTG_CLK_EN          (0x01 << 3)
#define __USB_AHB_CLK_EN          (0x01 << 4)
#define __USB_TDI                 (0x01 << 0)

extern void __usbInitHardware (void);

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/

