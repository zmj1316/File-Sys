/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**                                     
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            USBHostError.h
** Latest modified Date: 2008-3-2        
** Latest Version:       V1.0    
** Description:          USB主机软件包的错误代码
**                       
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗   Longsui Wu   
** Created date:         2007-11-14    
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
#ifndef __USB_HOST_ERROR_H
#define __USB_HOST_ERROR_H

#define USB_ERR_SUCESS                          0x00                    /*  成功                        */

#define USB_ERR_DEVICE_NOT_ATTACHED             0x20                    /*  设备没连接上                */
#define USB_ERR_DEVICE_NOT_READY                0x21                    /*  设备没准备好,不能数据传输   */
#define USB_ERR_SOFT_NOT_SUPPORT                0x22                    /*  软件包不支持                */
#define USB_ERR_EXIT_REQ                        0x23                    /*  卸载函数要求退出操作        */

#define USB_ERR_TIMEOUT                         0x30                    /*  数据传输超时                */

/*********************************************************************************************************
  数据传输时可能出现的错误代码，也是传输完成状态代码
*********************************************************************************************************/

#define USB_ERR_NOERR                           0x00                    /*  数据传输正常                */
#define USB_ERR_CRC                             0x01                    /*  发生CRC错误                 */
#define USB_ERR_BITSTUFFING                     0x02                    /*  发生位填充错误              */
#define USB_ERR_DATATOGGLEMISMATCH              0x03                    /*  数据触发位不匹配            */
#define USB_ERR_STALL                           0x04                    /*  端点被禁止                  */
#define USB_ERR_DEVICENOTRESPONDING             0x05                    /*  设备没有响应                */
#define USB_ERR_PIDCHECKFAILURE                 0x06                    /*  PID校验错误                 */
#define USB_ERR_UNEXPECTEDPID                   0x07                    /*  出现未知PID                 */
#define USB_ERR_DATAOVERRUN                     0x08                    /*  数据上溢错误                */
#define USB_ERR_DATAUNDERRUN                    0x09                    /*  数据下溢错误                */

#define USB_ERR_BUFFEROVERRUN                   0x0C                    /*  缓冲区上溢错误              */
#define USB_ERR_BUFFERUNDERRUN                  0x0D                    /*  缓冲区下溢错误              */
#define USB_ERR_NOTACCESSED                     0x0E                    /*  保留                        */


#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
