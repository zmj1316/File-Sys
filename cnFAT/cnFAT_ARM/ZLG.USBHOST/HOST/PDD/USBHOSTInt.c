/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            USBInt.c
** Latest modified Date: 2009-01-09
** Latest Version:       V1.0
** Description:          USB中断设置及处理
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗
** Created date:         2007-10-20
** Version:              V1.0
** Descriptions:         初始版本
**--------------------------------------------------------------------------------------------------------
** Modified by:          刘伟云
** Modified date:        2009-01-09
** Version:              V1.1
** Description:          移植到LPC3200。使用模块内部专用寄存器名，修改代码格式及部分注释
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
**
*********************************************************************************************************/
#include "..\USBHostIncludes.h"


/*********************************************************************************************************
** Function name:       __usbHostIntInit
** Descriptions:        初始化中断寄存器
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __usbHostIntInit (void)
{
    __GpsOhciReg->uiHcInterruptDisable = 0x7F | (0x3UL << 30);          /*  禁止所有中断, 并禁止 MIE 位 */
    __GpsOhciReg->uiHcInterruptStatus  = 0x7F | (0x3UL << 30);          /*  清除所有中断标志            */

    __OTGIntClr = __OTGIntSt;                                           /*  清除中断标志                */
    __OTGIntEn  = 0x1U << 3;                                            /*  使能主机中断                */

    __USBIntSt |= (0x01UL << 31);                                       /*  允许USB中断                 */

    //__GpsOhciReg->uiHcInterruptEnable = 0x7F | (0x3UL << 30);
}

/*********************************************************************************************************
** Function name:       usbHostException
** Descriptions:        主机中断服务程序
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void usbHostException(void)
{
    USB_INT32U uiIntSt = 0;
    USB_INT32U uiTmp;

#if USB_MAX_PORTS > 1
    USB_INT32U i;
#endif
	OSIntEnter();
    uiIntSt = __GpsOhciReg->uiHcInterruptStatus & __GpsOhciReg->uiHcInterruptEnable;

    if ( uiIntSt & 0x01 ) {	                                            /*  Scheduling overrun          */
        __GpsOhciReg->uiHcInterruptStatus |= 0x01;
        uiTmp = __USB_SHED_SHEDOVERRUN << 24;
        OSQPost(__GevtUsbMsgQeue, (void *)uiTmp);                       /*  发送消息给调度任务          */
        USBINTDEBUG_SENDSTR("\r\nSched OR\r\n");
    }

    if ( uiIntSt & 0x02 ) {	                                            /*  Write back done             */
        uiTmp = *(USB_INT32U *)(__GpsOhciReg->uiHcHCCA + 0x84);         /*  Get HccaDoneHead            */
        __GpsOhciReg->uiHcInterruptStatus |= 0x02;

        uiTmp = (uiTmp >> 4) | (__USB_SHED_DONEHEAD << 24);             /*  TD为16字节对齐,地址低4位为0 */
                                                                        /*  传输完成链表头指针高28位有效*/
                                                                        /*  bit27~bit0为传输完成链表的  */
                                                                        /*  高28位,bit31~bit28为事件类型*/

        OSQPost(__GevtUsbMsgQeue, (void *)uiTmp);                       /*  发送控制信息给调度任务      */
    }

    if ( uiIntSt & 0x04 ) {	                                            /*  SOF packet                  */
        __GpsOhciReg->uiHcInterruptStatus |= 0x04;
    }

    if ( uiIntSt & 0x08 ) {	                                            /*  Resume detected             */
        __GpsOhciReg->uiHcInterruptStatus |= 0x08;
        USBINTDEBUG_SENDSTR("\r\nResume det\r\n");
    }

    if ( uiIntSt & 0x10 ) {	                                            /*  Unrecoverable error         */
        __GpsOhciReg->uiHcInterruptStatus |= 0x10;
        uiTmp = __USB_SHED_UNRECERROR << 24;
        OSQPost(__GevtUsbMsgQeue, (void *)uiTmp);                       /*  发送消息给调度任务          */
        USBINTDEBUG_SENDSTR("\r\nUnrecov err\r\n");
    }

    if ( uiIntSt & 0x20 ) {	                                            /*  Frame Number overflow       */
        __GpsOhciReg->uiHcInterruptStatus |= 0x20;
    }

    if ( uiIntSt & 0x40 ) {                                             /*  Root hub status change      */

        __GpsOhciReg->uiHcInterruptStatus |= 0x40;

#if USB_MAX_PORTS == 1                                                  /*  只有一个下行端口的情况      */

        if (__GpsOhciReg->uiHcRhPortStatus[0] & (1U << 16)) {           /*  判断是否发生连接改变        */

            uiTmp = (__USB_SHED_CONCHANGE << 24)
                  | (1U << 8)
                  | (__GpsOhciReg->uiHcRhPortStatus[0] & 0x01);

            OSQPost(__GevtUsbMsgQeue, (void *)uiTmp);                   /*  发送消息给调度任务          */
        }

#else                                                                   /*  多于2个下行端口的情况       */

        for (i = 0; i < USB_MAX_PORTS; i++) {                           /*  依次判断哪个端口发生连接改变*/

            if (__GpsOhciReg->uiHcRhPortStatus[i] & (1U << 16)) {       /*  判断是否发生连接改变        */

                uiTmp = (__USB_SHED_CONCHANGE << 24)
                      | ((i + 1) << 8)
                      | (__GpsOhciReg->uiHcRhPortStatus[i] & 0x01);

                OSQPost(__GevtUsbMsgQeue, (void *)uiTmp);               /*  发送消息给调度任务          */

                break;
            }
        }
#endif                                                                  /*  USB_MAX_PORTS == 1          */
        __ohciDisEnInt(__USB_INT_RHSC);                                 /*  禁止连接改变中断            */
        USBINTDEBUG_SENDSTR("\r\nstate change\r\n");
    }

    if ( uiIntSt & 0x80 ) {                                             /*  Ownership change            */
        __GpsOhciReg->uiHcInterruptStatus |= 0x80;
        USBINTDEBUG_SENDSTR("\r\nOwnership change\r\n");
    }
	OSIntExit();
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
