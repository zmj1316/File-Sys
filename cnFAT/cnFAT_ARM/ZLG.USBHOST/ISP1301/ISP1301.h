/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**                                     
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            ISP1301.h
** Latest modified Date: 2007-10-19        
** Latest Version:       V1.0    
** Description:          ISP1301.h, ISP1301的相关操作头文件
**                       
**--------------------------------------------------------------------------------------------------------
** Created By:           Longsui Wu
** Created date:         2007-10-19    
** Version:              V1.0    
** Descriptions:         初始版本
**
**--------------------------------------------------------------------------------------------------------
** Modified by:          
** Modified date:        
** Version:             
** Description:          
**                      
**
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
**
*********************************************************************************************************/
#ifndef __ISP1301_H
#define __ISP1301_H

#define __USB_TDI                           (0x01 << 0)
#define __USB_I2C_START                     (0x01 << 8)
#define __USB_I2C_STOP                      (0x01 << 9)

#define __ISP1301_RFE                       (0x01 << 9)
#define __ISP1301_TFF                       (0x01 << 10)

#define __ISP1301_ADDR	                    0x5A
#define __ISP1301_ADDR_WRITE                (__ISP1301_ADDR | 0)    
#define __ISP1301_ADDR_READ                 (__ISP1301_ADDR | 1)
#define __ISP1301_START	                    (1 << 8)
#define __ISP1301_STOP	                    (1 << 9)

#define __ISP1301_VENDOR_ID                 0x00
#define __ISP1301_PRODUCT_ID                0x02
#define __ISP1301_MODE1_SET		            0x04
#define __ISP1301_MODE1_CLR		            0x05
#define __ISP1301_OTG_CTRL_SET	            0x06
#define __ISP1301_OTG_CTRL_CLR	            0x07
#define __ISP1301_OTG_STATUS		        0x10
#define __ISP1301_MODE2_SET		            0x12
#define __ISP1301_MODE2_CLR		            0x13


#define __ISP1301_VENDOR_ID                 0x00
#define __ISP1301_PRODUCT_ID                0x02
#define __ISP1301_VERSION_ID                0x14

#define __MODE_CTL1_SET                     0x04      
#define __MODE_CTL1_CLR                     0x05
#define __MODE_CTL2_SET                     0x12      
#define __MODE_CTL2_CLR                     0x13

#define __OTG_CTL1_SET                      0x06      
#define __OTG_CTL1_CLR                      0x07
#define __OTG_STAT                          0x10      

#define __INT_SRC                           0x08      
#define __INT_LATCH_SET                     0x0A
#define __INT_LATCH_CLR                     0x0B
#define __INT_MASK_FALSE_SET                0x0C
#define __INT_MASK_FALSE_CLR                0x0D
#define __INT_MASK_TRUE_SET                 0x0E
#define __INT_MASK_TRUE_CLR                 0x0F
      
#define __MODE_CTL1_SPEED                   (1 << 0)
#define __MODE_CTL1_SUSPENDED               (1 << 1)
#define __MODE_CTL1_DAT_SE0                 (1 << 2)
#define __MODE_CTL1_TRANSP_EN               (1 << 3)
#define __MODE_CTL1_BDIS_ACON_EN            (1 << 4)
#define __MODE_CTL1_OE_INT_EN               (1 << 5)

#define __MODE_CTL2_GLOBAL_PWR_DN           (1 << 0)
#define __MODE_CTL2_SPD_SUSP_CTRL           (1 << 1)
#define __MODE_CTL2_BI_DI                   (1 << 2)
#define __MODE_CTL2_TRANSP_BDIR0            (1 << 3)
#define __MODE_CTL2_TRANSP_BDIR1            (1 << 4)
#define __MODE_CTL2_AUDIO_EN                (1 << 5)
#define __MODE_CTL2_PSW_OE                  (1 << 6)
#define __MODE_CTL2_EN2V7                   (1 << 7)

#define __OTG_CTL1_DP_PULLUP                (1 << 0)
#define __OTG_CTL1_DM_PULLUP                (1 << 1)
#define __OTG_CTL1_DP_PULLDOWN              (1 << 2)
#define __OTG_CTL1_DM_PULLDOWN              (1 << 3)
#define __OTG_CTL1_ID_PULLDOWN              (1 << 4)
#define __OTG_CTL1_VBUS_DRV                 (1 << 5)
#define __OTG_CTL1_VBUS_DISCHRG             (1 << 6)
#define __OTG_CTL1_VBUS_CHRG                (1 << 7)

#define __OTG_STAT_B_SESS_VLD               (1 << 7)
#define __OTG_STAT_B_SESS_END               (1 << 6)

#define __INT_VBUS_VLD                      (1 << 0)
#define __INT_SESS_VLD                      (1 << 1)
#define __INT_DP_HI                         (1 << 2)
#define __INT_ID_GND                        (1 << 3)
#define __INT_DM_HI                         (1 << 4)
#define __INT_ID_FLOAT                      (1 << 5)
#define __INT_BDIS_ACON                     (1 << 6)
#define __INT_CAR_KIT                       (1 << 7)


#define __isp1301DPPURAdd()                 __isp1301Write(__OTG_CTL1_SET, __OTG_CTL1_DP_PULLUP)
#define __isp1301DPPURRemove()              __isp1301Write(__OTG_CTL1_CLR, __OTG_CTL1_DP_PULLUP)
#define __isp1301EnablePswOE()              __isp1301Write(__MODE_CTL2_SET, __MODE_CTL2_PSW_OE) 
#define __isp1301DisEnPswOE()               __isp1301Write(__MODE_CTL2_CLR, __MODE_CTL2_PSW_OE) 
#define __isp1301SetBDIS_ACON_EN()          __isp1301Write(__MODE_CTL1_SET, __MODE_CTL1_BDIS_ACON_EN)
#define __isp1301ClrBDIS_ACON_EN()          __isp1301Write(__MODE_CTL1_CLR, __MODE_CTL1_BDIS_ACON_EN)

#define __isp1301SetVbusDrv()               __isp1301Write(__OTG_CTL1_SET, __OTG_CTL1_VBUS_DRV)
#define __isp1301ClrVbusDrv()               __isp1301Write(__OTG_CTL1_CLR, __OTG_CTL1_VBUS_DRV)

#define __isp1301SetVbusDisCharge()         __isp1301Write(__OTG_CTL1_SET, __OTG_CTL1_VBUS_DISCHRG)
#define __isp1301ClrVbusDisCharge()         __isp1301Write(__OTG_CTL1_CLR, __OTG_CTL1_VBUS_DISCHRG)

#define __isp1301SetSpdSuspCtrl()           __isp1301Write(__MODE_CTL2_SET, __MODE_CTL2_SPD_SUSP_CTRL)
#define __isp1301SetFullSpeed()             __isp1301Write(__MODE_CTL1_SET, __MODE_CTL1_SPEED)
#define __isp1301SetSusp()                  __isp1301Write(__MODE_CTL1_SET, __MODE_CTL1_SUSPENDED)
#define __isp1301SetModeDAT_SE0()           __isp1301Write(__MODE_CTL1_SET, __MODE_CTL1_DAT_SE0)
    
#define __isp1301IntEnLow(ucDat)            __isp1301Write(__INT_MASK_FALSE_SET, ucDat)
#define __isp1301IntEnHigh(ucDat)           __isp1301Write(__INT_MASK_TRUE_SET, ucDat)

#define __isp1301IntDisLow(ucDat)           __isp1301Write(__INT_MASK_FALSE_CLR, ucDat)
#define __isp1301IntDisHigh(ucDat)          __isp1301Write(__INT_MASK_TRUE_CLR, ucDat)

#define __isp1301IntSrcRead()               __isp1301Read(__INT_SRC)
#define __isp1301IntClr(ucDat)              __isp1301Write(__INT_LATCH_CLR, ucDat)
#define __isp1301IntClrAll()                __isp1301Write(__INT_LATCH_CLR, 0xff)

/*********************************************************************************************************
** Function name:       __isp1301Init
** Descriptions:        ISP1301初始化, 此初始化结合开发板而设计
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 操作成功, FALSE : 操作失败
*********************************************************************************************************/
USB_BOOL __isp1301Init (void);

/*********************************************************************************************************
** Function name:       __isp1301Write
** Descriptions:        通过I2C向ISP1301写数据
** input parameters:    ucReg    寄存器地址
**                      ucRegBit 寄存器中的位
** output parameters:   None
** Returned value:      TRUE : 操作成功, FALSE : 操作失败
*********************************************************************************************************/
USB_BOOL __isp1301Write (USB_INT8U ucReg, USB_INT8U ucRegBit);

/*********************************************************************************************************
** Function name:       __isp1301Read
** Descriptions:        通过I2C读取ISP1301单个寄存器数据
** input parameters:    ucReg    寄存器地址
** output parameters:   None
** Returned value:      读取到的寄存器值
*********************************************************************************************************/
USB_INT8U __isp1301Read (USB_INT8U ucReg);

#endif                                                                  /*  #endif __ISP1301_H          */

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/


