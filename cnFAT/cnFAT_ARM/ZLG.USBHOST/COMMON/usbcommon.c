/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            usbcommon.c
** Latest modified Date: 2007-11-04
** Latest Version:       V1.0
** Description:          USB��������㹲ͬ���õĺ�����ȫ�ֱ�������
**--------------------------------------------------------------------------------------------------------
** Created By:           ��¡��  Longsui Wu
** Created date:         2007-11-04
** Version:              V1.0
** Descriptions:         ��ʼ�汾
**--------------------------------------------------------------------------------------------------------
** Modified by:          ��ΰ��
** Modified date:        2009-01-14
** Version:              V1.0
** Description:          ���usbMemCopy��usbMemSetЧ��
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
*********************************************************************************************************/
#include "..\HOST\USBHostIncludes.h"


/*********************************************************************************************************
** Function name:       usbMemCopy
** Descriptions:        �ڴ濽������
** input parameters:    pvSrc   Դ��������ַ
**                      usLen   Ҫ�����Ļ���������
** output parameters:   pvDst   Ŀ�Ļ�������ַ
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
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
** Descriptions:        ��ĳ�λ���������Ϊĳ���ض���ֵ
** input parameters:    pvMem   ��������ַ
**                      ucNum   Ҫ���õ�ֵ
**                      uiLen   ����������
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
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
** Descriptions:		��ʱ���1ms
** input parameters:	uiDly ��ʱ�ĺ�����
** output parameters:	��
** Returned value:		��
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
