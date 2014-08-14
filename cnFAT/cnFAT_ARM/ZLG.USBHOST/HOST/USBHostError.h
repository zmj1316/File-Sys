/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**                                     
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            USBHostError.h
** Latest modified Date: 2008-3-2        
** Latest Version:       V1.0    
** Description:          USB����������Ĵ������
**                       
**--------------------------------------------------------------------------------------------------------
** Created By:           ��¡��   Longsui Wu   
** Created date:         2007-11-14    
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
#ifndef __USB_HOST_ERROR_H
#define __USB_HOST_ERROR_H

#define USB_ERR_SUCESS                          0x00                    /*  �ɹ�                        */

#define USB_ERR_DEVICE_NOT_ATTACHED             0x20                    /*  �豸û������                */
#define USB_ERR_DEVICE_NOT_READY                0x21                    /*  �豸û׼����,�������ݴ���   */
#define USB_ERR_SOFT_NOT_SUPPORT                0x22                    /*  �������֧��                */
#define USB_ERR_EXIT_REQ                        0x23                    /*  ж�غ���Ҫ���˳�����        */

#define USB_ERR_TIMEOUT                         0x30                    /*  ���ݴ��䳬ʱ                */

/*********************************************************************************************************
  ���ݴ���ʱ���ܳ��ֵĴ�����룬Ҳ�Ǵ������״̬����
*********************************************************************************************************/

#define USB_ERR_NOERR                           0x00                    /*  ���ݴ�������                */
#define USB_ERR_CRC                             0x01                    /*  ����CRC����                 */
#define USB_ERR_BITSTUFFING                     0x02                    /*  ����λ������              */
#define USB_ERR_DATATOGGLEMISMATCH              0x03                    /*  ���ݴ���λ��ƥ��            */
#define USB_ERR_STALL                           0x04                    /*  �˵㱻��ֹ                  */
#define USB_ERR_DEVICENOTRESPONDING             0x05                    /*  �豸û����Ӧ                */
#define USB_ERR_PIDCHECKFAILURE                 0x06                    /*  PIDУ�����                 */
#define USB_ERR_UNEXPECTEDPID                   0x07                    /*  ����δ֪PID                 */
#define USB_ERR_DATAOVERRUN                     0x08                    /*  �����������                */
#define USB_ERR_DATAUNDERRUN                    0x09                    /*  �����������                */

#define USB_ERR_BUFFEROVERRUN                   0x0C                    /*  �������������              */
#define USB_ERR_BUFFERUNDERRUN                  0x0D                    /*  �������������              */
#define USB_ERR_NOTACCESSED                     0x0E                    /*  ����                        */


#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
