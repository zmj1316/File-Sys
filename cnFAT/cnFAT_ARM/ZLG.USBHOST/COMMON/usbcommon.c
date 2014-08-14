/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            usbcommon.c
** Latest modified Date: 2007-11-04
** Latest Version:       V1.0
** Description:          USB软件包各层共同引用的函数及全局变量定义
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗  Longsui Wu
** Created date:         2007-11-04
** Version:              V1.0
** Descriptions:         初始版本
**--------------------------------------------------------------------------------------------------------
** Modified by:          刘伟云
** Modified date:        2009-01-14
** Version:              V1.0
** Description:          提高usbMemCopy和usbMemSet效率
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
*********************************************************************************************************/
#include "..\HOST\USBHostIncludes.h"


/*********************************************************************************************************
** Function name:       usbMemCopy
** Descriptions:        内存拷贝函数
** input parameters:    pvSrc   源缓冲区地址
**                      usLen   要拷贝的缓冲区长度
** output parameters:   pvDst   目的缓冲区地址
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL usbMemCopy (void *pvDst, const void *pvSrc, USB_INT32U uiLen)
{
    if ((pvSrc == NULL) || (pvDst == NULL)) {
        return FALSE;
    }

    while (uiLen > 0) {
        uiLen--;
        ((USB_INT8U *)pvDst)[uiLen] = ((const USB_INT8U *)pvSrc)[uiLen];
    }

    return TRUE;
}

/*********************************************************************************************************
** Function name:       usbMemSet
** Descriptions:        将某段缓冲区设置为某个特定的值
** input parameters:    pvMem   缓冲区地址
**                      ucNum   要设置的值
**                      uiLen   缓冲区长度
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL usbMemSet (void *pvMem, USB_INT8U ucNum, USB_INT32U uiLen)
{
    if (pvMem == NULL) {
        return FALSE;
    }

     while (uiLen > 0) {
        ((USB_INT8U *)pvMem)[--uiLen] = ucNum;
    }

    return TRUE;
}

/*********************************************************************************************************
** Function name:		__sdDelay1ms
** Descriptions:		延时多个1ms
** input parameters:	uiDly 延时的毫秒数
** output parameters:	无
** Returned value:		无
*********************************************************************************************************/
void __usbDelay1ms (unsigned int uiDly)
{
	unsigned int i;

	for (; uiDly > 0; uiDly--) {
		for (i = 0; i < 4914; i++) {
			;
		}
	}
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
