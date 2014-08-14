/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            usbReg.h
** Latest modified Date: 2009-02-17
** Latest Version:       V1.1
** Description:          usb相关寄存器定义
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗  Longsui Wu
** Created date:         2007-11-06
** Version:              V1.0
** Descriptions:         初始版本
**--------------------------------------------------------------------------------------------------------
** Modified by:          LiuWeiyun
** Modified date:        2009-05-25
** Version:              V1.1
** Description:          Add LPC17000 USB registors define for convenience of transplant
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
*********************************************************************************************************/
#ifndef __USB_REG_H
#define __USB_REG_H

#ifndef   USB_MAX_PORTS
#define   USB_MAX_PORTS         1                                       /*  最大USB下行端口数           */
#endif

/*********************************************************************************************************
    OHCI规范定义的寄存器,详见OHCI规范第7章
*********************************************************************************************************/
typedef struct {
    volatile unsigned long uiHcRevision;                                /*  OHCI_REG_BASE_ADDR + 0x00   */
    volatile unsigned long uiHcControl;                                 /*  OHCI_REG_BASE_ADDR + 0x04   */
    volatile unsigned long uiHcCommandStatus;                           /*  OHCI_REG_BASE_ADDR + 0x08   */
    volatile unsigned long uiHcInterruptStatus;                         /*  OHCI_REG_BASE_ADDR + 0x0C   */
    volatile unsigned long uiHcInterruptEnable;                         /*  OHCI_REG_BASE_ADDR + 0x10   */
    volatile unsigned long uiHcInterruptDisable;                        /*  OHCI_REG_BASE_ADDR + 0x14   */
    volatile unsigned long uiHcHCCA;                                    /*  OHCI_REG_BASE_ADDR + 0x18   */
    volatile unsigned long uiHcPeriodCurrentED;                         /*  OHCI_REG_BASE_ADDR + 0x1C   */
    volatile unsigned long uiHcControlHeadED;                           /*  OHCI_REG_BASE_ADDR + 0x20   */
    volatile unsigned long uiHcControlCurrentED;                        /*  OHCI_REG_BASE_ADDR + 0x24   */
    volatile unsigned long uiHcBulkHeadED;                              /*  OHCI_REG_BASE_ADDR + 0x28   */
    volatile unsigned long uiHcBulkCurrentED;                           /*  OHCI_REG_BASE_ADDR + 0x2C   */
    volatile unsigned long uiHcDoneHead;                                /*  OHCI_REG_BASE_ADDR + 0x30   */
    volatile unsigned long uiHcFmInterval;                              /*  OHCI_REG_BASE_ADDR + 0x34   */
    volatile unsigned long uiHcFmRemaining;                             /*  OHCI_REG_BASE_ADDR + 0x38   */
    volatile unsigned long uiHcFmNumber;                                /*  OHCI_REG_BASE_ADDR + 0x3C   */
    volatile unsigned long uiHcPeriodicStart;                           /*  OHCI_REG_BASE_ADDR + 0x40   */
    volatile unsigned long uiHcLSThreshold;                             /*  OHCI_REG_BASE_ADDR + 0x44   */
    volatile unsigned long uiHcRhDescriptorA;                           /*  OHCI_REG_BASE_ADDR + 0x48   */
    volatile unsigned long uiHcRhDescriptorB;                           /*  OHCI_REG_BASE_ADDR + 0x4C   */
    volatile unsigned long uiHcRhStatus;                                /*  OHCI_REG_BASE_ADDR + 0x50   */
    volatile unsigned long uiHcRhPortStatus[USB_MAX_PORTS];             /*  OHCI_REG_BASE_ADDR + 0x54   */
} __OHCI_REG, *__POHCI_REG;

#define __OHCI_REG_BASE_ADDR    0x5000C000

#define __GpsOhciReg          ((/*volatile const*/ __POHCI_REG)(__OHCI_REG_BASE_ADDR))
/*********************************************************************************************************
    引脚连接模块寄存器
*********************************************************************************************************/
typedef struct {
    /**
     *  PINSELn
     */
    volatile unsigned long sel0;
    volatile unsigned long sel1;
    volatile unsigned long sel2;
    volatile unsigned long sel3;
    volatile unsigned long sel4;
    volatile unsigned long sel5;
    volatile unsigned long sel6;
    volatile unsigned long sel7;
    volatile unsigned long sel8;
    volatile unsigned long sel9;
    volatile unsigned long sel10;
    /**
     *  PINMODEn
     */
    volatile unsigned long mod0;
    volatile unsigned long mod1;
    volatile unsigned long mod2;
    volatile unsigned long mod3;
    volatile unsigned long mod4;
    volatile unsigned long mod5;
    volatile unsigned long mod6;
    volatile unsigned long mod7;
    volatile unsigned long mod8;
    volatile unsigned long mod9;
    /**
     *  PINMODE_ODn
     */
    volatile unsigned long modeOd0;
    volatile unsigned long modeOd1;
    volatile unsigned long modeOd2;
    volatile unsigned long modeOd3;
    volatile unsigned long modeOd4;
    /**
     *  I2CPADCFG
     */
    volatile unsigned long i2cPadCfg;
} __USB_REG_PIN;

#define __USB_PIN_BASE_ADDR     0x4002C000
#define __GpUrPin               ((volatile __USB_REG_PIN *)(__USB_PIN_BASE_ADDR))
//volatile /*const*/ __USB_REG_PIN * const __GpUrPin = (volatile /*const*/ __USB_REG_PIN * const)__USB_PIN_BASE_ADDR;

/*********************************************************************************************************
    USB PLL(PLL1)
*********************************************************************************************************/
#define __USB_PLL_BASE_ADDR     0x400FC0A0
#define __USBPllCon             (*(volatile unsigned long *)(__USB_PLL_BASE_ADDR + 0x00))
#define __USBPllCfg             (*(volatile unsigned long *)(__USB_PLL_BASE_ADDR + 0x04))
#define __USBPllState           (*(volatile unsigned long *)(__USB_PLL_BASE_ADDR + 0x08))
#define __USBPllFeed            (*(volatile unsigned long *)(__USB_PLL_BASE_ADDR + 0x0C))
/*********************************************************************************************************
    USB power and clock control register
*********************************************************************************************************/
#define __USBClkCfg             (*(volatile unsigned long *)(0x400FC108))
#define __USBClkCtrl            (*(volatile unsigned long *)(0x5000CFF4))
#define __USBClkSt              (*(volatile unsigned long *)(0x5000CFF8))

#define __PCONP_OFF_USB         31                              /*  USB接口功率/时钟控制位在PCONP中偏移 */
#define __PCONP                 (*(volatile unsigned long *)(0x400FC0C4))

/*********************************************************************************************************
    USB OTG ATX中断(EINT1)相关寄存器
*********************************************************************************************************/
#define __EXTINT                (*(volatile unsigned long *)(0x400fc140))
#define __EXTMODE               (*(volatile unsigned long *)(0x400fc148))
#define __EXTPOLAR              (*(volatile unsigned long *)(0x400fc14C))
/*********************************************************************************************************
    USB 中断状态寄存器
*********************************************************************************************************/
#define __USBIntSt              (*(volatile unsigned long *)(0x400FC1C0))

/*********************************************************************************************************
    USB device register
*********************************************************************************************************/
#define __USB_DEV_BASE_ADDR     0x5000C200

#define __DevIntSt              (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x00))
#define __DevIntEn              (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x04))
#define __DevIntClr             (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x08))
#define __DevIntSet             (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x0C))
#define __DevIntPrio            (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x2C))

#define __EPIntSt               (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x30))
#define __EPIntEn               (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x34))
#define __EPIntClr              (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x38))
#define __EPIntSet              (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x3C))
#define __EPIntPrio             (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x40))

#define __EPRealized            (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x44))
#define __EPIndex               (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x48))
#define __EPMaxPacketSize       (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x4C))

#define __RxData                (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x18))
#define __RxPacketLen           (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x20))
#define __TxData                (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x1C))
#define __TxPacketLen           (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x24))

#define __USBCtrl               (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x28))

#define __CmdCode               (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x10))
#define __CmdData               (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x14))

/*********************************************************************************************************
   USB DMA Registers
*********************************************************************************************************/
#define __DMAReqSt              (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x50))
#define __DMAReqClr             (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x54))
#define __DMAReqSet             (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x58))

#define __UDCAHead              (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x80))

#define __EPDMASt               (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x84))
#define __EPDMAEn               (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x88))
#define __EPDMADis              (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x8C))

#define __DMAIntSt              (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x90))
#define __DMAIntEn              (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0x94))

#define __EOTIntSt              (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0xA0))
#define __EOTIntClr             (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0xA4))
#define __EOTIntSet             (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0xA8))

#define __NDDReqIntSt           (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0xAC))
#define __NDDReqIntClr          (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0xB0))
#define __NDDReqIntSet          (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0xB4))

#define __SysErrIntSt           (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0xB8))
#define __SysErrIntClr          (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0xBC))
#define __SysErrIntSet          (*(volatile unsigned long *)(__USB_DEV_BASE_ADDR + 0xC0))

/*********************************************************************************************************
   USB OTG register
*********************************************************************************************************/
#define __USB_OTG_BASE_ADDR     0x5000C000
#define __OTGIntSt              (*(volatile unsigned long *)(__USB_OTG_BASE_ADDR + 0x100))
#define __OTGIntEn              (*(volatile unsigned long *)(__USB_OTG_BASE_ADDR + 0x104))
#define __OTGIntSet             (*(volatile unsigned long *)(__USB_OTG_BASE_ADDR + 0x108))
#define __OTGIntClr             (*(volatile unsigned long *)(__USB_OTG_BASE_ADDR + 0x10C))

#define __OTGState              (*(volatile unsigned long *)(__USB_OTG_BASE_ADDR + 0x110))
#define __OTGTimer              (*(volatile unsigned long *)(__USB_OTG_BASE_ADDR + 0x114))
#define __OTGClkCtrl            (*(volatile unsigned long *)(__USB_OTG_BASE_ADDR + 0xFF4))
#define __OTGClkSt              (*(volatile unsigned long *)(__USB_OTG_BASE_ADDR + 0xFF8))

/*********************************************************************************************************
   USB OTG I2C register
*********************************************************************************************************/
#define __OTG_I2C_BASE_ADDR     0x5000C300
#define __I2CRx                 (*(volatile unsigned long *)(__OTG_I2C_BASE_ADDR + 0x00))
#define __I2CTx                 (*(volatile unsigned long *)(__OTG_I2C_BASE_ADDR + 0x00))
#define __I2CSt                 (*(volatile unsigned long *)(__OTG_I2C_BASE_ADDR + 0x04))
#define __I2CCtrl               (*(volatile unsigned long *)(__OTG_I2C_BASE_ADDR + 0x08))
#define __I2CClkHigh            (*(volatile unsigned long *)(__OTG_I2C_BASE_ADDR + 0x0C))
#define __I2CClkLow             (*(volatile unsigned long *)(__OTG_I2C_BASE_ADDR + 0x10))


#endif                                                                  /*  __USB_REG_H                 */

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
