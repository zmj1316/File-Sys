/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            isp1301.c
** Latest modified Date: 2009-01-05
** Latest Version:       V1.2
** Description:          ISP1301.c, ISP1301的相关操作函数
**--------------------------------------------------------------------------------------------------------
** Created By:           Longsui Wu
** Created date:         2007-10-19
** Version:              V1.0
** Descriptions:         初始版本
**--------------------------------------------------------------------------------------------------------
** Modified by:   		 LiuWeiyun
** Modified date:    	 2009-01-05
** Version:          	 1.2
** Descriptions:      	 LPC3200移植
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
*********************************************************************************************************/
#include "..\host\USBHostIncludes.h"

USB_INT16U __isp1301VendorIdRead (void);
USB_INT16U __isp1301ProductIdRead (void);

/*********************************************************************************************************
** Function name:       __isp1301Init
** Descriptions:        ISP1301初始化, 此初始化结合开发板而设计
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 操作成功, FALSE : 操作失败
*********************************************************************************************************/
USB_BOOL __isp1301Init (void)
{
    if (__isp1301VendorIdRead() != 0x04CC) {
        return FALSE;
    }

    if (__isp1301ProductIdRead() != 0x1301) {
        return FALSE;
    }
    __isp1301DPPURRemove();
    //__isp1301SetModeDAT_SE0();
	__isp1301SetModeVP_VM();
    __isp1301SetSpdSuspCtrl();
    __isp1301SetFullSpeed();

    return TRUE;
}

/*********************************************************************************************************
** Function name:       __isp1301Init
** Descriptions:        ISP1301初始化, 此初始化结合开发板而设计
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 操作成功, FALSE : 操作失败
*********************************************************************************************************/
USB_INT16U __isp1301VendorIdRead (void)
{
    USB_INT16U usId = 0;
    USB_INT8U  ucTmp;

    ucTmp = __isp1301Read(__ISP1301_VENDOR_ID + 1);
    usId  = (USB_INT16U)(ucTmp << 8);
    ucTmp = __isp1301Read(__ISP1301_VENDOR_ID);
    usId  = (USB_INT16U)(usId + ucTmp);

    return usId;
}

/*********************************************************************************************************
** Function name:       __isp1301Init
** Descriptions:        ISP1301初始化, 此初始化结合开发板而设计
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 操作成功, FALSE : 操作失败
*********************************************************************************************************/
USB_INT16U __isp1301ProductIdRead (void)
{
    USB_INT16U usId = 0;
    USB_INT16U ucTmp;

    ucTmp = __isp1301Read(__ISP1301_PRODUCT_ID + 1);
    usId  = (USB_INT16U)(ucTmp << 8);
    ucTmp = __isp1301Read(__ISP1301_PRODUCT_ID);
    usId  = (USB_INT16U)(usId + ucTmp);

    return usId;
}

/*********************************************************************************************************
** Function name:       __isp1301Write
** Descriptions:        通过I2C向ISP1301写数据
** input parameters:    ucReg    寄存器地址
**                      ucRegBit 寄存器中的位
** output parameters:   None
** Returned value:      TRUE : 操作成功, FALSE : 操作失败
*********************************************************************************************************/
USB_BOOL __isp1301Write (USB_INT8U ucReg, USB_INT8U ucRegBit)
{   
    __I2CTx = __ISP1301_ADDR_WRITE | __USB_I2C_START;
    __I2CTx = ucReg;
    __I2CTx = (USB_INT16U)(ucRegBit | __USB_I2C_STOP);

    while (!(__I2CSt & __USB_TDI));                         		    /*  等待发送完毕                */

    __I2CSt = __USB_TDI;                                            	/*  清除标志位                  */

    return TRUE;
}

/*********************************************************************************************************
** Function name:       __isp1301Read
** Descriptions:        通过I2C读取ISP1301单个寄存器数据
** input parameters:    ucReg    寄存器地址
** output parameters:   None
** Returned value:      读取到的寄存器值
*********************************************************************************************************/
USB_INT8U __isp1301Read (USB_INT8U ucReg)
{
    __I2CTx = __ISP1301_ADDR_WRITE | __USB_I2C_START;
    __I2CTx = ucReg;
    __I2CTx = __ISP1301_ADDR_READ | __USB_I2C_START;
    __I2CTx = __USB_I2C_STOP;

    while (!(__I2CSt & __USB_TDI));                                 /*  等待发送完毕                */
    __I2CSt = __USB_TDI;                                            /*  清除标志位                  */

    while (__I2CSt & __ISP1301_RFE);                                /*  等待FIFO不空                */

    return (USB_INT8U)__I2CRx;
}


/*********************************************************************************************************
  END FILE
*********************************************************************************************************/