/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            isp1301.c
** Latest modified Date: 2009-01-05
** Latest Version:       V1.2
** Description:          ISP1301.c, ISP1301����ز�������
**--------------------------------------------------------------------------------------------------------
** Created By:           Longsui Wu
** Created date:         2007-10-19
** Version:              V1.0
** Descriptions:         ��ʼ�汾
**--------------------------------------------------------------------------------------------------------
** Modified by:   		 LiuWeiyun
** Modified date:    	 2009-01-05
** Version:          	 1.2
** Descriptions:      	 LPC3200��ֲ
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
** Descriptions:        ISP1301��ʼ��, �˳�ʼ����Ͽ���������
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : �����ɹ�, FALSE : ����ʧ��
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
** Descriptions:        ISP1301��ʼ��, �˳�ʼ����Ͽ���������
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : �����ɹ�, FALSE : ����ʧ��
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
** Descriptions:        ISP1301��ʼ��, �˳�ʼ����Ͽ���������
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : �����ɹ�, FALSE : ����ʧ��
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
** Descriptions:        ͨ��I2C��ISP1301д����
** input parameters:    ucReg    �Ĵ�����ַ
**                      ucRegBit �Ĵ����е�λ
** output parameters:   None
** Returned value:      TRUE : �����ɹ�, FALSE : ����ʧ��
*********************************************************************************************************/
USB_BOOL __isp1301Write (USB_INT8U ucReg, USB_INT8U ucRegBit)
{   
    __I2CTx = __ISP1301_ADDR_WRITE | __USB_I2C_START;
    __I2CTx = ucReg;
    __I2CTx = (USB_INT16U)(ucRegBit | __USB_I2C_STOP);

    while (!(__I2CSt & __USB_TDI));                         		    /*  �ȴ��������                */

    __I2CSt = __USB_TDI;                                            	/*  �����־λ                  */

    return TRUE;
}

/*********************************************************************************************************
** Function name:       __isp1301Read
** Descriptions:        ͨ��I2C��ȡISP1301�����Ĵ�������
** input parameters:    ucReg    �Ĵ�����ַ
** output parameters:   None
** Returned value:      ��ȡ���ļĴ���ֵ
*********************************************************************************************************/
USB_INT8U __isp1301Read (USB_INT8U ucReg)
{
    __I2CTx = __ISP1301_ADDR_WRITE | __USB_I2C_START;
    __I2CTx = ucReg;
    __I2CTx = __ISP1301_ADDR_READ | __USB_I2C_START;
    __I2CTx = __USB_I2C_STOP;

    while (!(__I2CSt & __USB_TDI));                                 /*  �ȴ��������                */
    __I2CSt = __USB_TDI;                                            /*  �����־λ                  */

    while (__I2CSt & __ISP1301_RFE);                                /*  �ȴ�FIFO����                */

    return (USB_INT8U)__I2CRx;
}


/*********************************************************************************************************
  END FILE
*********************************************************************************************************/