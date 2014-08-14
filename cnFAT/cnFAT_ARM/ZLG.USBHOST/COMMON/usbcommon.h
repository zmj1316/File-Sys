/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            usbcommon.h
** Latest modified Date: 2007-11-06
** Latest Version:       V1.0
** Description:          usbcommon.c��ͷ�ļ�
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
** Descriptions:        �ڴ濽������
** input parameters:    pvSrc   Դ��������ַ
**                      usLen   Ҫ�����Ļ���������
** output parameters:   pvDst   Ŀ�Ļ�������ַ
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL usbMemCopy (void *pvDst, const void *pvSrc, USB_INT32U uiLen);

/*********************************************************************************************************
** Function name:       usbMemSet
** Descriptions:        ��ĳ�λ���������Ϊĳ���ض���ֵ
** input parameters:    pvMem   ��������ַ
**                      ucNum   Ҫ���õ�ֵ
**                      uiLen   ����������
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL usbMemSet (void *pvMem, USB_INT8U ucNum, USB_INT32U uiLen);

/*********************************************************************************************************
** Function name:		__sdDelay1ms
** Descriptions:		��ʱ���1ms
** input parameters:	uiDly ��ʱ�ĺ�����
** output parameters:	��
** Returned value:		��
*********************************************************************************************************/
void __usbDelay1ms (unsigned int uiDly);


#define USB_DEBUG                   0                                   /*  ��(1)��(0)��DEBUG�����,    */
#define USBINT_DEBUG                0                                   /*  ��(1)��(0)��DEBUG�����,    */

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
                                                                        /*  ��ȡһ���ֵ�ĳ���ֽ�ֵ      */

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
