/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            usbcommon.h
** Latest modified Date: 2007-11-06
** Latest Version:       V1.0
** Description:          usbcommon.c的头文件
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
#ifndef __USBCOMMON_H
#define __USBCOMMON_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char       USB_INT8U;
typedef signed   char       USB_INT8;
typedef unsigned short      USB_INT16U;
typedef signed   short      USB_INT16;
typedef unsigned int        USB_INT32U;
typedef signed   int        USB_INT32;
typedef unsigned long       ULONG32;

typedef unsigned short      USB_WORD;
typedef unsigned int        USB_DWORD;

#ifndef USB_BOOL
typedef unsigned char       USB_BOOL;
#endif

#ifndef TRUE
#define TRUE                (1 == 1)
#endif

#ifndef FALSE
#define FALSE               (1 == 0)
#endif

#ifdef NULL
#undef NULL
#endif

#define NULL                (void *)0

#define USB_ENTER_CRITICAL_SECTION()    OS_ENTER_CRITICAL()
#define USB_EXIT_CRITICAL_SECTION()     OS_EXIT_CRITICAL()


#ifdef __cplusplus
  }
#endif


/*********************************************************************************************************
** Function name:       usbMemCopy
** Descriptions:        内存拷贝函数
** input parameters:    pvSrc   源缓冲区地址
**                      usLen   要拷贝的缓冲区长度
** output parameters:   pvDst   目的缓冲区地址
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL usbMemCopy (void *pvDst, const void *pvSrc, USB_INT32U uiLen);

/*********************************************************************************************************
** Function name:       usbMemSet
** Descriptions:        将某段缓冲区设置为某个特定的值
** input parameters:    pvMem   缓冲区地址
**                      ucNum   要设置的值
**                      uiLen   缓冲区长度
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL usbMemSet (void *pvMem, USB_INT8U ucNum, USB_INT32U uiLen);

/*********************************************************************************************************
** Function name:		__sdDelay1ms
** Descriptions:		延时多个1ms
** input parameters:	uiDly 延时的毫秒数
** output parameters:	无
** Returned value:		无
*********************************************************************************************************/
void __usbDelay1ms (unsigned int uiDly);


#define USB_DEBUG                   0                                   /*  是(1)否(0)以DEBUG版编译,    */
#define USBINT_DEBUG                0                                   /*  是(1)否(0)以DEBUG版编译,    */

#if USB_DEBUG > 0
#include "..\..\uart0\uart0.h"
#endif

#if USB_DEBUG > 0
#define USBDEBUG_SENDSTR(str)       uart0StrSend(str)
#define USBDEBUG_SENDCHAR(ch)       uart0ByteSend(ch)
#else
#define USBDEBUG_SENDSTR(str)
#define USBDEBUG_SENDCHAR(ch)
#endif

#if USBINT_DEBUG > 0
#define USBINTDEBUG_SENDSTR(str)    USBDEBUG_SENDSTR(str)
#else
#define USBINTDEBUG_SENDSTR(str)
#endif

#define __USBLSB(uiData,ucIndex)    (USB_INT8U)((uiData >> (ucIndex * 8)) & 0xFF)
                                                                        /*  提取一个字的某个字节值      */

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
