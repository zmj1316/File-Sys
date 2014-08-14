/****************************************Copyright (c)****************************************************
**                            Guangzhou ZHIYUAN electronics Co.,LTD.
**                                      
**                                 http://www.embedtools.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               firmware.h
** Latest modified Date:    2008-1-17
** Latest Version:          1.0
** Descriptions:            固件接口
**
**--------------------------------------------------------------------------------------------------------
** Created by:              Chenmingji
** Created date:            2008-1-17
** Version:                 1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Version:                 
** Descriptions:            
**
*********************************************************************************************************/
#ifndef __FIRMWARE_H 
#define __FIRMWARE_H

#ifdef __cplusplus
extern "C" {
#endif                                                                  /*  __cplusplus                 */

/*********************************************************************************************************
  通用宏定义
*********************************************************************************************************/
#ifndef TRUE
#define TRUE        1
#endif                                                                  /*  TRUE                        */

#ifndef FALSE
#define FALSE       0
#endif                                                                  /*  FALSE                       */

#ifndef NULL
#define NULL        0ul
#endif                                                                  /*  NULL                       */

/*********************************************************************************************************
  realView编译器需要添加的头文件
*********************************************************************************************************/
#include    <ctype.h>
#include    <stdlib.h>
#include    <setjmp.h>
#include    <rt_misc.h>

/*********************************************************************************************************
  uC/OS-II的特殊代码
*********************************************************************************************************/
#include ".\uCOS-II\cpu\os_cpu.h"
#include "..\cfg_file\uCOSII\os_cfg.h"
#include ".\uCOS-II\uCOS_II.H"

/*********************************************************************************************************
  固件提供的功能引入的头文件
*********************************************************************************************************/
#include    ".\Startup\lpc1700.h"
#include    ".\Startup\target.h"
#include    ".\zy_if\zy_if.h"

/*********************************************************************************************************
  其它需要引入的头文件
*********************************************************************************************************/

#ifdef __cplusplus
}
#endif                                                                  /*  __cplusplus                 */

#endif                                                                  /*  __FIRMWARE_H                */
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
