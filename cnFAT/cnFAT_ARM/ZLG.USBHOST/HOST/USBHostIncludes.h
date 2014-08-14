/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            USBHostIncludes.h
** Latest modified Date: 2007-12-26
** Latest Version:       V1.0
** Description:          USB主机软件包所要抉的头文件
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗  Longsui Wu
** Created date:         2007-12-26
** Version:              V1.0
** Descriptions:         初始版本
**--------------------------------------------------------------------------------------------------------
** Modified by:          刘伟云
** Modified date:        2009-01-09
** Version:              V1.1
** Description:
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
*********************************************************************************************************/
#ifndef __USB_HOST_INCLUDES_H
#define __USB_HOST_INCLUDES_H

//#include "..\..\ZY_CODE\uCOS-II\INCLUDES.H"
#include "..\..\config.h"
#include <stdlib.h>

#include "..\COMMON\usbReg.h"
#include ".\USBHostError.h"
#include "..\COMMON\usbcommon.h"
#include "..\COMMON\Descriptor.h"
#include ".\MDD\hostChap9.h"
#include "..\ISP1301\ISP1301.h"
#include ".\PDD\USBHOSTInt.h"
#include ".\MDD\hcd.h"
#include ".\PDD\USBHOSTHAL.h"
#include ".\MDD\usbShed.h"
#include ".\MDD\USBTransfer.h"
#include ".\MDD\usbd.h"

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
