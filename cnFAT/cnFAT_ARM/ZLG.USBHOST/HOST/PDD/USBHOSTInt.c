/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            USBInt.c
** Latest modified Date: 2009-01-09
** Latest Version:       V1.0
** Description:          USB�ж����ü�����
**--------------------------------------------------------------------------------------------------------
** Created By:           ��¡��
** Created date:         2007-10-20
** Version:              V1.0
** Descriptions:         ��ʼ�汾
**--------------------------------------------------------------------------------------------------------
** Modified by:          ��ΰ��
** Modified date:        2009-01-09
** Version:              V1.1
** Description:          ��ֲ��LPC3200��ʹ��ģ���ڲ�ר�üĴ��������޸Ĵ����ʽ������ע��
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
** Descriptions:        ��ʼ���жϼĴ���
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __usbHostIntInit (void)
{
    __GpsOhciReg->uiHcInterruptDisable = 0x7F | (0x3UL << 30);          /*  ��ֹ�����ж�, ����ֹ MIE λ */
    __GpsOhciReg->uiHcInterruptStatus  = 0x7F | (0x3UL << 30);          /*  ��������жϱ�־            */

    __OTGIntClr = __OTGIntSt;                                           /*  ����жϱ�־                */
    __OTGIntEn  = 0x1U << 3;                                            /*  ʹ�������ж�                */

    __USBIntSt |= (0x01UL << 31);                                       /*  ����USB�ж�                 */

    //__GpsOhciReg->uiHcInterruptEnable = 0x7F | (0x3UL << 30);
}

/*********************************************************************************************************
** Function name:       usbHostException
** Descriptions:        �����жϷ������
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
        OSQPost(__GevtUsbMsgQeue, (void *)uiTmp);                       /*  ������Ϣ����������          */
        USBINTDEBUG_SENDSTR("\r\nSched OR\r\n");
    }

    if ( uiIntSt & 0x02 ) {	                                            /*  Write back done             */
        uiTmp = *(USB_INT32U *)(__GpsOhciReg->uiHcHCCA + 0x84);         /*  Get HccaDoneHead            */
        __GpsOhciReg->uiHcInterruptStatus |= 0x02;

        uiTmp = (uiTmp >> 4) | (__USB_SHED_DONEHEAD << 24);             /*  TDΪ16�ֽڶ���,��ַ��4λΪ0 */
                                                                        /*  �����������ͷָ���28λ��Ч*/
                                                                        /*  bit27~bit0Ϊ������������  */
                                                                        /*  ��28λ,bit31~bit28Ϊ�¼�����*/

        OSQPost(__GevtUsbMsgQeue, (void *)uiTmp);                       /*  ���Ϳ�����Ϣ����������      */
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
        OSQPost(__GevtUsbMsgQeue, (void *)uiTmp);                       /*  ������Ϣ����������          */
        USBINTDEBUG_SENDSTR("\r\nUnrecov err\r\n");
    }

    if ( uiIntSt & 0x20 ) {	                                            /*  Frame Number overflow       */
        __GpsOhciReg->uiHcInterruptStatus |= 0x20;
    }

    if ( uiIntSt & 0x40 ) {                                             /*  Root hub status change      */

        __GpsOhciReg->uiHcInterruptStatus |= 0x40;

#if USB_MAX_PORTS == 1                                                  /*  ֻ��һ�����ж˿ڵ����      */

        if (__GpsOhciReg->uiHcRhPortStatus[0] & (1U << 16)) {           /*  �ж��Ƿ������Ӹı�        */

            uiTmp = (__USB_SHED_CONCHANGE << 24)
                  | (1U << 8)
                  | (__GpsOhciReg->uiHcRhPortStatus[0] & 0x01);

            OSQPost(__GevtUsbMsgQeue, (void *)uiTmp);                   /*  ������Ϣ����������          */
        }

#else                                                                   /*  ����2�����ж˿ڵ����       */

        for (i = 0; i < USB_MAX_PORTS; i++) {                           /*  �����ж��ĸ��˿ڷ������Ӹı�*/

            if (__GpsOhciReg->uiHcRhPortStatus[i] & (1U << 16)) {       /*  �ж��Ƿ������Ӹı�        */

                uiTmp = (__USB_SHED_CONCHANGE << 24)
                      | ((i + 1) << 8)
                      | (__GpsOhciReg->uiHcRhPortStatus[i] & 0x01);

                OSQPost(__GevtUsbMsgQeue, (void *)uiTmp);               /*  ������Ϣ����������          */

                break;
            }
        }
#endif                                                                  /*  USB_MAX_PORTS == 1          */
        __ohciDisEnInt(__USB_INT_RHSC);                                 /*  ��ֹ���Ӹı��ж�            */
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
