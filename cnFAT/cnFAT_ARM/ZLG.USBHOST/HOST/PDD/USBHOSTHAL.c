/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            USBHAL.c
** Latest modified Date: 2007-07-08
** Latest Version:       V1.0
** Description:          USBHAL.c, 硬件抽象层,设置 USB 设备运行的硬件条件
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗
** Created date:         2007-10-20
** Version:              V1.0
** Descriptions:         初始版本
**--------------------------------------------------------------------------------------------------------
** Modified by:          刘伟云
** Modified date:        2009-01-09
** Version:              V1.1
** Description:          移植到LPC3200。使用模块内部专用寄存器名
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
*********************************************************************************************************/
#include    "..\USBHostIncludes.h"


/*********************************************************************************************************
    1.USB双向数据D+线,P0.29(PINSEL1)
    2.在PINSSELn中的偏移
*********************************************************************************************************/
#define __USB_DEV_DP                0x01UL
#define __REG_OFF_DEV_DP            26
/*********************************************************************************************************
    1.USB双向数据D-线,P0.30(PINSEL1)
    2.在PINSSELn中的偏移
*********************************************************************************************************/
#define __USB_DEV_DM                0x01UL
#define __REG_OFF_DEV_DM            28

/*********************************************************************************************************
    1.USB UP LED,P1.18(PINSEL3)
    2.在PINSSELn中的偏移
*********************************************************************************************************/
#define __USB_DEV_UP_LED            0x01UL
#define __REG_OFF_DEV_UP_LED        4
/*********************************************************************************************************
    1.USB VBUS,P1.30(PINSEL3)
    2.在PINSSELn中的偏移
*********************************************************************************************************/
#define __USB_DEV_VBUS              0x02UL
#define __REG_OFF_DEV_VBUS          28
/*********************************************************************************************************
    1.USB connect,P2.9(PINSEL4)
    2.在PINSSELn中的偏移
*********************************************************************************************************/
#define __USB_DEV_CONNECT           0x01UL
#define __REG_OFF_DEV_CONNECT       18


/*********************************************************************************************************
    1.USB PPWR,P1.19(PINSEL3)
    2.在PINSSELn中的偏移
*********************************************************************************************************/
#define __USB_PPWR                  0x02UL
#define __REG_OFF_USB_PPWR          6
/*********************************************************************************************************
    1.USB PWRD,P1.22(PINSEL3)
    2.在PINSSELn中的偏移
*********************************************************************************************************/
#define __USB_PWRD                  0x02UL
#define __REG_OFF_USB_PWRD          12
/*********************************************************************************************************
    1.USB VORCR,P1.27(PINSEL3)
    2.在PINSSELn中的偏移
*********************************************************************************************************/
#define __USB_VORCR                 0x02UL
#define __REG_OFF_USB_VORCR         22

/*********************************************************************************************************
    1.USB SDA,P0.27(PINSEL1)
    2.在PINSSELn中的偏移
*********************************************************************************************************/
#define __USB_OTG_SDA               0x02UL
#define __REG_OFF_USB_SDA           22
/*********************************************************************************************************
    1.USB SCL,P0.28(PINSEL0)
    2.在PINSSELn中的偏移
*********************************************************************************************************/
#define __USB_OTG_SCL               0x01UL
#define __REG_OFF_USB_SCL           22
/*********************************************************************************************************
    1.(USB OTG INT)EINT1,P2.11(PINSEL4)
    2.在PINSSELn中的偏移
*********************************************************************************************************/
#define __USB_OTG_INT               0x01UL
#define __REG_OFF_USB_OTG_INT       22

#if 0
#define __USB_DEV_CLK_EN           (0x01UL << 1)
#define __USB_PORTSEL_CLK_EN       (0x01UL << 3)
#define __USB_AHB_CLK_EN           (0x01UL << 4)
#endif

#define __MAX_I2C_RATE              100000


#define __HOST_POWER_DIS()
#define __HOST_POWER_EN()
/*********************************************************************************************************
    USB PLL

    (1) USBCLK = M * Fosc, or USBCLK = Fcco / (2 * p)
	(2) Fcco = USBCLK * 2 * P, or Fcco = Fosc * M * 2 * P
	(3) 10MHz <= Fosc <= 25MHz
	(4) USBCLK = 48MHz
	(5) 156MHz <= Fcco <= 320MHz
*********************************************************************************************************/
#define __USB_CLOCK                 48000000                            /*  USB 操作频率                */
#define __USB_PLL_FOSC              12000000                            /*  USB PLL输入频率             */
#define __USB_PLL_FCCO              (__USB_CLOCK * 2 * __USB_PLL_P)
#define __USB_PLL_M                 (__USB_CLOCK / __USB_PLL_FOSC)
#define __USB_PLL_P                 (2)

#define __USB_PLL_M_VALUE           (__USB_PLL_M - 1)
#define __USB_PLL_P_VALUE           (0x01)

#define __PLL_CON_EN                0
#define __PLL_CON_CONNECT           1

#define __PLL_CFG_MSEL              0
#define __PLL_CFG_PSEL              5

#define __PLL_STAT_MSEL             0
#define __PLL_STAT_PSEL             5
#define __PLL_STAT_EN_STAT          8
#define __PLL_STAT_CONNET_STAT      9
#define __PLL_STAT_PLOCK            10

#define __PLL_MODE_DIS_UNCON        0x00                                /* PLL disabled and unconnected */
#define __PLL_MODE_EN_UNCON         0x01                                /* PLL enabled but unconnected  */
#define __PLL_MODE_EN_CONN          0x03                                /* PLL enabled and connected    */

#define __PLL_OPT_FEED()            do {\
                                        USB_ENTER_CRITICAL_SECTION();\
                                        __USBPllFeed = 0xAA;\
                                        __USBPllFeed = 0x55;\
                                        USB_EXIT_CRITICAL_SECTION();\
                                    } while (0)
/*********************************************************************************************************
** Function name:       __usbPllConfig
** Descriptions:        USB PLL setting
** input parameters:    无
** output parameters:   无
** Returned value:      无
** NOTE:                (1) USBCLK = M * Fosc, or USBCLK = Fcco / (2 * p)
**                      (2) Fcco = USBCLK * 2 * P, or Fcco = Fosc * M * 2 * P
**                      (3) 10MHz <= Fosc <= 25MHz
**                      (4) USBCLK = 48MHz
**                      (5) 156MHz <= Fcco <= 320MHz
*********************************************************************************************************/
static void  __usbPllConfig (void)
{
    if (0 != (__USBPllState & (0x01UL << __PLL_STAT_CONNET_STAT))) {
        __USBPllCon = __PLL_MODE_DIS_UNCON;
        __PLL_OPT_FEED();
    }

    __USBPllCon = __PLL_MODE_DIS_UNCON;
    __PLL_OPT_FEED();

    __USBPllCfg = (__USB_PLL_P_VALUE << __PLL_CFG_PSEL)
                | (__USB_PLL_M_VALUE << __PLL_CFG_MSEL);
    __PLL_OPT_FEED();

    __USBPllCon = __PLL_MODE_EN_UNCON;
    __PLL_OPT_FEED();

    while (0 == (__USBPllState & (0x01UL << __PLL_STAT_PLOCK)));

    __USBPllCon = __PLL_MODE_EN_CONN;
    __PLL_OPT_FEED();
}

/*********************************************************************************************************
** Function name:       __usbHostPinInit
** Descriptions:        USB主机引脚初始化
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
static void __usbHostPinInit (void)
{
    USB_ENTER_CRITICAL_SECTION();
    __GpUrPin->sel1 &= ~((0x03UL << __REG_OFF_DEV_DP) |
                         (0x03UL << __REG_OFF_DEV_DM));
    __GpUrPin->sel1 |=  ((__USB_DEV_DP << __REG_OFF_DEV_DP) |
                         (__USB_DEV_DM << __REG_OFF_DEV_DM));

    __GpUrPin->sel3 &= ~((0x03UL << __REG_OFF_DEV_UP_LED) |
                         (0x03UL << __REG_OFF_USB_VORCR)  |
                         (0x03UL << __REG_OFF_USB_PWRD)   |
                         (0x03UL << __REG_OFF_USB_PPWR));
    __GpUrPin->sel3 |=  ((__USB_DEV_UP_LED << __REG_OFF_DEV_UP_LED) |
                         (__USB_VORCR      << __REG_OFF_USB_VORCR)  |
                         (__USB_PWRD       << __REG_OFF_USB_PWRD)   |
                         (__USB_PPWR       << __REG_OFF_USB_PPWR));
    USB_EXIT_CRITICAL_SECTION();
}

/******************************************************************************
** Function name:		__USBDevEnable
**
** Descriptions:		Enable USB HCLK, enable path from USB PLL
**				to USB device block, enable USB Device clock.
**				This module is called after USBPLL_Config()
**				before adding pull-up on DP on ISP1301.
**				See comment in USB_ISP1301Config().
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void __USBHostEnable( void )
{
#if ATX_ISP1301_EN
    USB_INT32U  ulClkCtrl = (__USB_HOST_CLK_EN
                          |  __USB_AHB_CLK_EN
                          |  __USB_I2C_CLK_EN
                          |  __USB_OTG_CLK_EN);
#else
    USB_INT32U  ulClkCtrl = (__USB_HOST_CLK_EN
                          |  __USB_OTG_CLK_EN
                          |  __USB_AHB_CLK_EN);
#endif


    __PCONP |= 0x01UL << __PCONP_OFF_USB;

    __usbPllConfig();

    __OTGClkCtrl = ulClkCtrl;                                           /*  配置USB时钟控制寄存器       */
    while (!(__OTGClkSt & ulClkCtrl));

    __OTGState |= 0x01;

    __usbHostPinInit();
}
/*********************************************************************************************************
** Function name:       __usbInitHardware
** Descriptions:        初始化 USB 设备控制器硬件
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __usbInitHardware (void)
{
#ifndef Fpclk
#define Fpclk               ~0UL
#endif
#define __MAX_I2C_RATE      100000

    __USBHostEnable();

#if ATX_ISP1301_EN

    __OTGState |= 0x01;

    /**
     * Set USB I2C rate
     */
    __I2CCtrl   |= 0x100;

#if ((Fpclk / __MAX_I2C_RATE) / 2) > 0xff
    __I2CClkHigh  = 0xff;
    __I2CClkLow   = 0xff;
#else
    __I2CClkHigh  = (Fpclk / __MAX_I2C_RATE) / 2;
    __I2CClkLow   = (Fpclk / __MAX_I2C_RATE) / 2;
#endif

    __isp1301Write(__MODE_CTL2_CLR,0xff);
    __isp1301Write(__MODE_CTL1_CLR,0xff);
    __isp1301Write(__MODE_CTL2_SET, __MODE_CTL2_BI_DI);
    __isp1301Write(__MODE_CTL1_SET, __MODE_CTL1_DAT_SE0);

    __isp1301Write(__OTG_CTL1_CLR, __OTG_CTL1_DP_PULLUP | __OTG_CTL1_DM_PULLUP);
    __isp1301Write(__OTG_CTL1_SET, __OTG_CTL1_DM_PULLDOWN | __OTG_CTL1_DP_PULLDOWN);


    __isp1301Write(__OTG_CTL1_CLR, __OTG_CTL1_VBUS_DRV |__OTG_CTL1_VBUS_CHRG);
    __isp1301Write(__OTG_CTL1_SET, __OTG_CTL1_VBUS_DISCHRG);
#endif

    OSTimeDly(OS_TICKS_PER_SEC / 40);
    __HOST_POWER_EN();
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
