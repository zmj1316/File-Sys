/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            hcd.c
** Latest modified Date: 2007-11-06
** Latest Version:       V1.0
** Description:          HCD层驱动
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗         Longsui Wu
** Created date:         2007-11-06
** Version:              V1.0
** Descriptions:         初始版本
**--------------------------------------------------------------------------------------------------------
** Modified by:          liuweiyun
** Modified date:        2009-01-09
** Version:              V1.0
** Description:          移植到LPC3200。更改__GpsOhciReg的定义，添加、修改注释，改进一些函数的可读性等
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
*********************************************************************************************************/
#define USB_RAM_ALLOCATE
#include "..\USBHostIncludes.h"


/*********************************************************************************************************
  HCD层,主要包括以下几个部分:
  1. 链表管理部分,包括ED和ED链表管理,TD和TD链表管理
  2. HC管理部分
  3. 根集线器管理部分
  4. USB RAM管理部分
*********************************************************************************************************/

/*********************************************************************************************************
  链表管理部分,包括ED和ED链表管理,TD和TD链表管理
*********************************************************************************************************/

__POHCI_ED         __GpohciEd;
__POHCI_TD         __GpohciTd;
__OHCI_TD_STAT     __GohciTdStat;
__TRAN_END_TD      __GsTranEndTd;

volatile USB_INT8U __GucUsbSpeed   = 0;                                 /*  设备速度,全速(0)或低速(1)   */
volatile USB_INT8U __GucUsbDevAddr = 0;                                 /*  设备地址                    */


void __hcdAllocBufferInit (void);

/*********************************************************************************************************
** Function name:       __hcdEdInit
** Descriptions:        初始化各 ED
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __hcdEdInit (void)
{
    __GpohciEd = (__POHCI_ED)__OHCI_ED_BASE_ADDR;

    __GpohciEd->edsControl.uiControl = (8 << 16)                        /* MPS,MaximumPacketSize        */
                                     | (1 << 14)                        /* K,sKip                       */
                                     | (__GucUsbSpeed << 13);           /* S,Speed                      */
    __GpohciEd->edsControl.pedNextED = NULL;
    __GpohciEd->edsControl.ptdHeadP  = NULL;
    __GpohciEd->edsControl.ptdTailP  = NULL;

    __GpohciEd->edsBulkOut.uiControl = (8 << 16) | (1 << 14);
    __GpohciEd->edsBulkOut.pedNextED = &(__GpohciEd->edsBulkIn);
    __GpohciEd->edsBulkOut.ptdHeadP  = NULL;
    __GpohciEd->edsBulkOut.ptdTailP  = NULL;

    __GpohciEd->edsBulkIn.uiControl  = (8 << 16) | (1 << 14);
    __GpohciEd->edsBulkIn.pedNextED  = NULL;
    __GpohciEd->edsBulkIn.ptdHeadP   = NULL;
    __GpohciEd->edsBulkIn.ptdTailP   = NULL;

    __GpohciEd->edsIntrOut.uiControl = (8 << 16) | (1 << 14);
    __GpohciEd->edsIntrOut.pedNextED = (__PHC_ENDPOINT_DESCRIPTOR)&(__GpohciEd->edsIntrIn);
    __GpohciEd->edsIntrOut.ptdHeadP  = NULL;
    __GpohciEd->edsIntrOut.ptdTailP  = NULL;

    __GpohciEd->edsIntrIn.uiControl = (8 << 16) | (1 << 14);
    __GpohciEd->edsIntrIn.pedNextED = (__PHC_ENDPOINT_DESCRIPTOR)&(__GpohciEd->edsIso);
    __GpohciEd->edsIntrIn.ptdHeadP  = NULL;
    __GpohciEd->edsIntrIn.ptdTailP  = NULL;

    __GpohciEd->edsIso.uiControl = (__HC_ISO_MAXPACKETSIZE << 16) | (3 << 7) | (1 << 14) | (1 << 15);
    __GpohciEd->edsIso.pedNextED = NULL;
    __GpohciEd->edsIso.ptdHeadP  = NULL;
    __GpohciEd->edsIso.ptdTailP  = NULL;
}

/*********************************************************************************************************
** Function name:       __hcdTdInit
** Descriptions:        TD 初始化
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __hcdTdInit (void)
{
    __GpohciTd     = (__POHCI_TD)__OHCI_TD_BASE_ADDR;
    usbMemSet(&__GohciTdStat, 0, sizeof(__GohciTdStat));                /*  清除所有TD使用标志          */
    usbMemSet(__GpohciTd, 0, sizeof(__OHCI_TD));                        /*  清零所有TD                  */
}

/*********************************************************************************************************
** Function name:       __hcdInit
** Descriptions:        HCD层的软件环境初始化
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __hcdInit (void)
{
    __hcdAllocBufferInit();
    __hcdTdInit();
    __hcdEdInit();
}

/*********************************************************************************************************
** Function name:       __hcdGetEdInfo
** Descriptions:        获取ED的相关信息
** input parameters:    ucTranType  传输类型
** output parameters:   pedInfo     获取到的信息
** Returned value:      None
*********************************************************************************************************/
void __hcdGetEdInfo (USB_INT8U ucTranType, __PED_INFO pedInfo)
{
    pedInfo->ucTranType = ucTranType;

    switch (ucTranType) {

    case USB_TRAN_TYPE_CONTROL:
        pedInfo->pEd          = &__GpohciEd->edsControl;
        pedInfo->pTdHead      = &(__GpohciTd->tdCtrlTd[0]);
        pedInfo->puiTdSts     = (volatile  USB_INT32 *)&__GohciTdStat.uiCtrlTdStatus[0];
        pedInfo->uiMaxTd      = USB_MAX_CTRL_TD;
        pedInfo->usMaxPktSize = (USB_INT16U)((pedInfo->pEd->uiControl >> 16) & 0x7FF);
        break;

    case USB_TRAN_TYPE_BULK_OUT:
        pedInfo->pEd          = &__GpohciEd->edsBulkOut;
        pedInfo->pTdHead      = &(__GpohciTd->tdBulkOutTd[0]);
        pedInfo->puiTdSts     = (volatile  USB_INT32 *)&__GohciTdStat.uiBulkOutTdStatus[0];
        pedInfo->uiMaxTd      = USB_MAX_BULKOUT_TD;
        pedInfo->usMaxPktSize = (USB_INT16U)((pedInfo->pEd->uiControl >> 16) & 0x7FF);
        break;

    case USB_TRAN_TYPE_BULK_IN:
        pedInfo->pEd          = &__GpohciEd->edsBulkIn;
        pedInfo->pTdHead      = &(__GpohciTd->tdBulkInTd[0]);
        pedInfo->puiTdSts     = (volatile  USB_INT32 *)&__GohciTdStat.uiBulkInTdStatus[0];
        pedInfo->uiMaxTd      = USB_MAX_BULKIN_TD;
        pedInfo->usMaxPktSize = (USB_INT16U)((pedInfo->pEd->uiControl >> 16) & 0x7FF);
        break;

    case USB_TRAN_TYPE_INTR_OUT:
        pedInfo->pEd          = &__GpohciEd->edsIntrOut;
        pedInfo->pTdHead      = &(__GpohciTd->tdIntrOutTd[0]);
        pedInfo->puiTdSts     = (volatile  USB_INT32 *)&__GohciTdStat.uiIntrOutTdStatus[0];
        pedInfo->uiMaxTd      = USB_MAX_INTROUT_TD;
        pedInfo->usMaxPktSize = (USB_INT16U)((pedInfo->pEd->uiControl >> 16) & 0x7FF);
        break;

    case USB_TRAN_TYPE_INTR_IN:
        pedInfo->pEd          = &__GpohciEd->edsIntrIn;
        pedInfo->pTdHead      = &(__GpohciTd->tdIntrInTd[0]);
        pedInfo->puiTdSts     = (volatile  USB_INT32 *)&__GohciTdStat.uiIntrInTdStatus[0];
        pedInfo->uiMaxTd      = USB_MAX_INTRIN_TD;
        pedInfo->usMaxPktSize = (USB_INT16U)((pedInfo->pEd->uiControl >> 16) & 0x7FF);
        break;

    default:
        pedInfo = NULL;
    }
}

/*********************************************************************************************************
** Function name:       __hcdGetTdType
** Descriptions:        根据TD的地址获取TD的类型
** input parameters:    tdTd: TD指针,即TD地址
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
USB_INT8U __hcdGetTdType (__PHC_GEN_TRANSFER_DESCRIPTOR ptdTd)
{
    USB_INT8U  ucType;
    USB_INT32U uiTmp = (USB_INT32U)ptdTd;

    if ((uiTmp >= __OHCI_TD_CTRL_BASE_ADDR) && (uiTmp <= __OHCI_TD_CTRL_END_ADDR)) {
        ucType = USB_TRAN_TYPE_CONTROL;
    } else if ((uiTmp >= __OHCI_TD_BULKOUT_BASE_ADDR) && (uiTmp <= __OHCI_TD_BULKOUT_END_ADDR)) {
        ucType = USB_TRAN_TYPE_BULK_OUT;
    } else if ((uiTmp >= __OHCI_TD_BULKIN_BASE_ADDR) && (uiTmp <= __OHCI_TD_BULKIN_END_ADDR)) {
        ucType = USB_TRAN_TYPE_BULK_IN;
    } else if ((uiTmp >= __OHCI_TD_INTROUT_BASE_ADDR) && (uiTmp <= __OHCI_TD_INTROUT_END_ADDR)) {
        ucType = USB_TRAN_TYPE_INTR_OUT;
    } else if ((uiTmp >= __OHCI_TD_INTRIN_BASE_ADDR) && (uiTmp <= __OHCI_TD_INTRIN_END_ADDR)) {
        ucType = USB_TRAN_TYPE_INTR_IN;
    } else {
        ucType = 0xff;
    }
    return ucType;
}

/*********************************************************************************************************
** Function name:       __hcdAllocTd
** Descriptions:        申请空闲 TD
** input parameters:    pEdInfo: TD所属ED的信息结构体
** output parameters:   None
** Returned value:      申请到的 TD 首地址,若申请失败,则返回 NULL
*********************************************************************************************************/
__PHC_GEN_TRANSFER_DESCRIPTOR __hcdAllocTd (__PED_INFO pEdInfo)
{
    USB_INT32U                    i, j;
    USB_INT32U                    uiTdIndex;
    __PHC_GEN_TRANSFER_DESCRIPTOR phcdAllocTd;

    if (pEdInfo == NULL) {
        return NULL;
    }

    phcdAllocTd = NULL;
    OS_ENTER_CRITICAL();
    do {
        for (i = 0; i < (pEdInfo->uiMaxTd +31) / 32; i++) {             /*  查找可利用的空闲TD所在的字段*/
            if (*(pEdInfo->puiTdSts + i) != 0xFFFFFFFF) {
                break;
            }
        }
        if (i >= (pEdInfo->uiMaxTd +31) / 32) {                         /*  无空闲TD                    */
            break;
        }
        for (j = 0; j < 32; j++) {
            if ((*(pEdInfo->puiTdSts + i) & (0x01U << j)) == 0) {       /*  查找到第一个TD,则跳出       */
                break;
            }
        }

        uiTdIndex = i * 32 + j;
        if (uiTdIndex >= pEdInfo->uiMaxTd) {                            /*  得到的TD序号大于等于最大TD号*/
            break;                                                      /*  说明该TD号无效,跳出do..while*/
        } else {
            *(pEdInfo->puiTdSts + i) |= 1U << j;
            phcdAllocTd = pEdInfo->pTdHead + uiTdIndex;
        }
    } while (0);
    OS_EXIT_CRITICAL();

#if 0

    if (phcdAllocTd != NULL) {
        usbMemSet(phcdAllocTd, 0, sizeof(__PHC_GEN_TRANSFER_DESCRIPTOR));
    }

#endif

    return phcdAllocTd;
}

/*********************************************************************************************************
** Function name:       __hcdFreeTd
** Descriptions:        释放一个 TD
** input parameters:    pEdInfo  要释放的TD所属ED的信息结构体
**                      phcdFreeTd 要释放的TD
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __hcdFreeTd (__PED_INFO pEdInfo, __PHC_GEN_TRANSFER_DESCRIPTOR phcdFreeTd)
{
    USB_INT32U i ,j;

    if (phcdFreeTd == NULL) {
        return;
    }

    OS_ENTER_CRITICAL();
    i = phcdFreeTd - pEdInfo->pTdHead;                                  /*  要回收的TD的序号            */
    j = i / 32;                                                         /*  要收回的TD的状态位在状态    */
                                                                        /*  数组中的下标                */
    *(pEdInfo->puiTdSts + j) &= ~(1 << (i - j * 32));                   /*  在该下标元素中的位序号      */
    OS_EXIT_CRITICAL();
}

/*********************************************************************************************************
** Function name:       __hcdFreeAllTd
** Descriptions:        释放所有 TD
** input parameters:    pEdInfo  要释放的TD所属ED的信息结构体
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __hcdFreeAllTd (__PED_INFO pEdInfo)
{
    USB_INT32U i;

    OS_ENTER_CRITICAL();
    for (i = 0; i < (pEdInfo->uiMaxTd +31) / 32; i++) {
        *(pEdInfo->puiTdSts + i) = 0;
    }
    OS_EXIT_CRITICAL();
}


/*********************************************************************************************************
** Function name:       __hcdAddTd
** Descriptions:        在目标 ED 上添加 TD,目标 ED 由 pGenTdParam->ucType 指定
** input parameters:    pGenTdParam: TD 的参数
**                      ptdAddTd     要增加的TD
**                      pEdInfo      ED信息结构体
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __hcdAddTd (__PHC_GEN_TD_PARAM            pGenTdParam,
                     __PHC_GEN_TRANSFER_DESCRIPTOR ptdAddTd,
                     __PED_INFO                    pEdInfo)
{
    USB_INT32U                   *puiHeap;
    USB_INT32U                    i;
    __PHC_GEN_TRANSFER_DESCRIPTOR ptdInsert = NULL;

    /*
     *  配置 TD 参数
     */
    ptdAddTd->uiControl  = (pGenTdParam->ucDataToggle  << 24) |
                           (pGenTdParam->ucDelayIntr   << 21) |
                           (pGenTdParam->ucDirect_PID  << 19) |
                           (pGenTdParam->ucBufferRound << 18);
    ptdAddTd->pucCBP     = pGenTdParam->pucCBP;
    if (pGenTdParam->usBufLen != 0) {
        ptdAddTd->pucBufEnd = pGenTdParam->pucCBP + pGenTdParam->usBufLen - 1;
    } else {                                                            /*  发送空包                    */
        pGenTdParam->pucCBP = NULL;
        ptdAddTd->pucBufEnd = ptdAddTd->pucCBP;
    }

    /*
     *  将TD插入ED.如果该ED为空,则直接将TD插入ED的TD List
     *  否则将TD插入该ED的尾部
     */
    OS_ENTER_CRITICAL();
    if (((USB_INT32U)(pEdInfo->pEd->ptdHeadP) & 0xFFFFFFF0) == \
        ((USB_INT32U)(pEdInfo->pEd->ptdTailP) & 0xFFFFFFF0)) {
                                                                        /*  如果为空ED,则直接将TD插入ED */
        puiHeap                = (USB_INT32U *)&(pEdInfo->pEd->ptdHeadP);
        i                      = *puiHeap;
        pEdInfo->pEd->ptdHeadP = ptdAddTd;
        *puiHeap              |= i & 0x02;
    } else {                                                            /*  该 ED 已经含有 TD           */
        ptdInsert = (__PHC_GEN_TRANSFER_DESCRIPTOR)((USB_INT32U)(pEdInfo->pEd->ptdHeadP) & 0xFFFFFFF0);
        while (((USB_INT32U)ptdInsert->ptdNextTD & 0xFFFFFFF0) !=       /*  寻找最后一个 TD             */
               ((USB_INT32U)pEdInfo->pEd->ptdTailP & 0xFFFFFFF0)) {
            ptdInsert = (__PHC_GEN_TRANSFER_DESCRIPTOR)((USB_INT32U)ptdInsert->ptdNextTD & 0xFFFFFFF0);
        }
        ptdInsert->ptdNextTD = ptdAddTd;
    }
    ptdAddTd->ptdNextTD = pEdInfo->pEd->ptdTailP;
    if (pEdInfo->ucTranType == USB_TRAN_TYPE_CONTROL) {
        __GsTranEndTd.pCtrlEndTd = ptdAddTd;
    } else {
        __GsTranEndTd.pDataEndTd = ptdAddTd;
    }
    OS_EXIT_CRITICAL();

    return TRUE;
}

/*********************************************************************************************************
** Function name:       __hcdGetEdPtr
** Descriptions:        获取 ED 首地址
** input parameters:    ucTranType  传输类型
** output parameters:   None
** Returned value:      ED 首地址
*********************************************************************************************************/
__PHC_ENDPOINT_DESCRIPTOR __hcdGetEdPtr (USB_INT8U ucTranType)
{
    __PHC_ENDPOINT_DESCRIPTOR pedEd;

    switch (ucTranType) {

    case USB_TRAN_TYPE_CONTROL:                                         /*  Control 端点                */
        pedEd = &(__GpohciEd->edsControl);
        break;

    case USB_TRAN_TYPE_BULK_OUT:                                        /*  Bulk Out 端点               */
        pedEd = &(__GpohciEd->edsBulkOut);
        break;

    case USB_TRAN_TYPE_BULK_IN:                                         /*  Bulk In 端点                */
        pedEd = &(__GpohciEd->edsBulkIn);
        break;

    case USB_TRAN_TYPE_INTR_OUT:                                        /*  Intrrupt Out 端点           */
        pedEd = &(__GpohciEd->edsIntrOut);
        break;

    case USB_TRAN_TYPE_INTR_IN:                                         /*  Intrrupt In 端点            */
        pedEd = &(__GpohciEd->edsIntrIn);
        break;

    default:
        pedEd = NULL;
        break;
    }

    return pedEd;
 }

/*********************************************************************************************************
** Function name:       __usbEdSetSkipAndWaitNextFrame
** Descriptions:        设置ED中的skip位并等待下一帧开始
** input parameters:    ucType : 类型: 控制传输, 块传输, 中断传输, 同步传输
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __usbEdSetSkipAndWaitNextFrame (USB_INT8U ucType)
{
    __PHC_ENDPOINT_DESCRIPTOR pedEd;

    pedEd             = __hcdGetEdPtr(ucType);                          /*  获取端点描述符首地址        */
    pedEd->uiControl |= 1 << 14;                                        /*  设置 sKip = 1               */
    __hcWaitNextFrame();                                                /*  等待下一帧                  */
    return TRUE;
}

/*********************************************************************************************************
** Function name:       __usbEdSetSkipAndWaitNextFrame
** Descriptions:        设置ED中的skip位并等待下一帧开始
** input parameters:    ucType : 类型: 控制传输, 块传输, 中断传输, 同步传输
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __usbEdClearSkip (USB_INT8U ucType)
{
    __PHC_ENDPOINT_DESCRIPTOR pedEd;

    pedEd             = __hcdGetEdPtr(ucType);
    pedEd->uiControl &= ~(1 << 14);

    return TRUE;
}
/*********************************************************************************************************
** Function name:       __hcdIsEdHeadEquTail
** Descriptions:        判断 ED 中的 HeadP 是否等于 TailP
** input parameters:    ED的信息结构体
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __hcdIsEdHeadEquTail (__PED_INFO pEdInfo)
{
    USB_INT32U uiTmp1, uiTmp2;

    uiTmp1 = (USB_INT32U)pEdInfo->pEd->ptdHeadP;
    uiTmp2 = (USB_INT32U)pEdInfo->pEd->ptdTailP;

    if ((uiTmp1 & ~0x0F) == (uiTmp2 & ~0x0F)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/*********************************************************************************************************
** Function name:       __hcdEdLetHeadEquTail
** Descriptions:        强制使 ED 中的 HeadP 等于 TailP, 这在传输出现错误时调用
** input parameters:    ED的信息结构体
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __hcdEdLetHeadEquTail (__PED_INFO pEdInfo)
{
    USB_INT32U  uiTmp;
    USB_INT32U *puiHeadp;

    __usbEdSetSkipAndWaitNextFrame(pEdInfo->ucTranType);

    OS_ENTER_CRITICAL();
    puiHeadp = (USB_INT32U *)&(pEdInfo->pEd->ptdHeadP);
    uiTmp    = *puiHeadp;
    pEdInfo->pEd->ptdHeadP = pEdInfo->pEd->ptdTailP;
    *puiHeadp |= uiTmp & 0x02;                                          /*  保留 ToggleCarry 位         */
    OS_EXIT_CRITICAL();
}


/*********************************************************************************************************
** Function name:       __hcdEnableEd
** Descriptions:        使能ED
** input parameters:    ucEdType: ED 类型
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __hcdEnableEd (USB_INT8U ucEdType)
{
    __PHC_ENDPOINT_DESCRIPTOR pedEd = __hcdGetEdPtr(ucEdType);

    if (pedEd) {
        pedEd->uiControl &= ~(1 << 14);
    }
    return TRUE;
}


/*********************************************************************************************************
    OHCI 寄存器地址指针，已经替换成宏定义，见usbreg.h
*********************************************************************************************************/
#if 0
volatile const __POHCI_REG __GpsOhciReg = (volatile const __POHCI_REG)(__OHCI_REG_BASE_ADDR);
#endif



/*********************************************************************************************************
  HC管理部分
*********************************************************************************************************/

/*********************************************************************************************************
** Function name:       __ohciInit
** Descriptions:        OHCI初始化,系统初始化时调用
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __ohciInit (void)
{
    USB_INT8U  i;
    USB_INT32U uiTmp = 0x0000ffff;

    __GpsOhciReg->uiHcControl = 0;                                      /*  复位                        */
    OSTimeDly(1);

    __GpsOhciReg->uiHcCommandStatus = 0x01;                             /*  复位 HC                     */
    while ((__GpsOhciReg->uiHcCommandStatus & 0x01) && --uiTmp);        /*  复位操作在10uS内完成        */
    if (uiTmp == 0) {
        return FALSE;
    }

    uiTmp = __GpsOhciReg->uiHcRevision;                                 /*  读取HC版本号                */
    if ( (uiTmp & 0xFF) != 0x10) {
        return( FALSE );
    }

    __GpsOhciReg->uiHcInterruptDisable = 0x7FUL | (0x3UL << 30);        /*  禁止所有中断, 并禁止 MIE 位 */
    __GpsOhciReg->uiHcRhStatus = (0x1UL << 16) | (0x1UL << 15);         /*  使能全局电源及LPSC位        */

    OSTimeDlyHMSM(0, 0, 0, 200);

    /*
     *  检查有无设备插入
     */
    for (i = 0; i < USB_MAX_PORTS; i++) {
        if (__GpsOhciReg->uiHcRhPortStatus[i] & (1 << 0)) {
            uiTmp = (0x10 << 24) | ((i + 1) << 8) | (__GpsOhciReg->uiHcRhPortStatus[i] & 0x01);
            OSQPost(__GevtUsbMsgQeue, (void *)uiTmp);
            break;
        }
    }

    __GpsOhciReg->uiHcInterruptStatus = 0xC000007F;                     /*  清除所有中断标志            */
    __ohciEnableInt(__USB_INT_RHSC);

    return TRUE;
}

/*********************************************************************************************************
** Function name:       __ohciInit2
** Descriptions:        OHCI初始化,检测到设备插入时调用
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __ohciInit2 (USB_INT8U ucPort)
{
    USB_INT32U  uiTmp;
    USB_INT32U *puiTmp = NULL;

    puiTmp = (USB_INT32U *)__OHCI_HCCA_BASE_ADDR;

    /**
     *  清零HCCA区域,256字节
     */
    (void)usbMemSet(puiTmp, 0, 256);
    /**
     *  HccaInterruptTable init,32 * 4 bytes
     */
    for (uiTmp = 0; uiTmp < 32; uiTmp++) {
        puiTmp[uiTmp] = (USB_INT32U)&(__GpohciEd->edsIntrOut);
    }

    __GpsOhciReg->uiHcControl = 0;                                      /*  复位                        */
    OSTimeDly(1);

    uiTmp = 0x0000ffff;
    __GpsOhciReg->uiHcCommandStatus = 0x01;                             /*  复位 HC                     */
    while ((__GpsOhciReg->uiHcCommandStatus & 0x01) && --uiTmp);
    if (uiTmp == 0) {
        return FALSE;
    }

    __GpsOhciReg->uiHcInterruptDisable = 0x7F | (0x03U << 30);          /*  禁止所有中断, 并禁止 MIE 位 */

	__GpsOhciReg->uiHcBulkHeadED    = (USB_INT32U)&(__GpohciEd->edsBulkOut);
    __GpsOhciReg->uiHcControlHeadED = (USB_INT32U)&(__GpohciEd->edsControl);

    __GpsOhciReg->uiHcControlCurrentED = 0;
    __GpsOhciReg->uiHcBulkCurrentED    = 0;

    __GpsOhciReg->uiHcPeriodCurrentED  = 0;


    __GpsOhciReg->uiHcHCCA = __OHCI_HCCA_BASE_ADDR;

    __GpsOhciReg->uiHcFmInterval = (__OHCI_FM_INTERVAL_FSMPS << 16) | __OHCI_FM_INTERVAL_FI;

    __GpsOhciReg->uiHcPeriodicStart = __OHCI_FM_INTERVAL_FI / 5;
    __GpsOhciReg->uiHcLSThreshold   = __OHCI_LS_THRESHOLD;

    /**
     *  根集线器上电时访问根集线器前需等待时间设置
     */
    uiTmp = (__OHCI_PWRON_TO_PWRGOOD << 24) & 0xFF000000;               /*  PowerOnToPowerGoodTime      */
    __GpsOhciReg->uiHcRhDescriptorA = uiTmp;
    uiTmp = (uiTmp >> 24) << 1;                                         /*  实际等待时间:POTPGT * 2 ms  */
    OSTimeDlyHMSM(0, 0, 0, (USB_INT16U)uiTmp);

    __GpsOhciReg->uiHcRhDescriptorB = 0x0000;

    __GpsOhciReg->uiHcRhStatus = (0x01 << 16) | (1 << 15);              /*  使能全局电源及LPSC位        */

    OSTimeDlyHMSM(0, 0, 0, 200);
	__ohciPortOpen(ucPort);

    /*
     *  使 HC 进入 Operational 状态,CBSR = 3 (即 4 : 1)
     */
    __GpsOhciReg->uiHcControl = (0x02 << 6) | (0x03 << 0);

    __GpsOhciReg->uiHcInterruptStatus = 0xC000007F;                     /*  清除所有中断标志            */
    __GpsOhciReg->uiHcInterruptEnable = ((1U << 31)
                                      |  (1U << 6)
                                      |  (1U << 4)
                                      |  (1U << 3)
                                      |  (1U << 1)
                                      |  (1U << 1)
                                      |  (1U << 0));

    return TRUE;
}

/*********************************************************************************************************
** Function name:       __ohciEnableInt
** Descriptions:        使能中断
** input parameters:    uiIntIndex 中断索引
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __ohciEnableInt (USB_INT32U uiIntIndex)
{
    INT32U i, j;

    __GpsOhciReg->uiHcInterruptStatus |= uiIntIndex;

    /*
     *  使能中断, 并在使能完后将寄存器值读回进行校验,如果校验失败则重试
     */
    i = 0;
    do {
        __GpsOhciReg->uiHcInterruptEnable |= __USB_INT_MIE | uiIntIndex;
        if ((__GpsOhciReg->uiHcInterruptEnable & __USB_INT_MIE | uiIntIndex) == __USB_INT_MIE | uiIntIndex) {
            break;
        }
        USBDEBUG_SENDSTR("\r\n!!Enable Interrupt failed!\r\n");
        for (j = 0; j < 0x200; j++);
    } while (++i <= 3);

    if (i <= 3) {
        return TRUE;
    } else {
        USBDEBUG_SENDSTR("\r\n!!Enable Interrupt failed!\r\n");
        return FALSE;
    }
}

/*********************************************************************************************************
** Function name:       __ohciDisEnInt
** Descriptions:        禁止中断
** input parameters:    uiIntIndex 中断索引
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __ohciDisEnInt (USB_INT32U uiIntIndex)
{
    __GpsOhciReg->uiHcInterruptDisable = uiIntIndex;
}

/*********************************************************************************************************
** Function name:       __ohciDisEnIntAll
** Descriptions:        禁止所有中断
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __ohciDisEnIntAll (void)
{
    __GpsOhciReg->uiHcInterruptDisable = 0x80000000;
}

/*********************************************************************************************************
** Function name:       __hcEnableSchedule
** Descriptions:        使能调度
** input parameters:    ucType : 类型: 控制传输, 块传输, 中断传输, 同步传输
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __hcEnableSchedule (USB_INT8U ucType)
{
    if (ucType == USB_TRAN_TYPE_CONTROL) {
        __GpsOhciReg->uiHcControl |= 1 << 4;
        return TRUE;
    } else if ((ucType == USB_TRAN_TYPE_BULK_OUT) || (ucType == USB_TRAN_TYPE_BULK_IN)) {
        __GpsOhciReg->uiHcControl |= 1 << 5;
        return TRUE;
    } else if ((ucType == USB_TRAN_TYPE_INTR_OUT) || (ucType == USB_TRAN_TYPE_INTR_IN)) {
        __GpsOhciReg->uiHcControl |= 1 << 2;
        return TRUE;
    } else if (ucType == USB_TRAN_TYPE_ISO) {
        __GpsOhciReg->uiHcControl |= (1 << 2) | (1 << 3);
        return TRUE;
    }

    return FALSE;
}

/*********************************************************************************************************
** Function name:       __hcDisEnSchedule
** Descriptions:        禁止调度
** input parameters:    ucType : 类型: 控制传输, 块传输, 中断传输, 同步传输
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __hcDisEnSchedule (USB_INT8U ucType)
{
    if (ucType == USB_TRAN_TYPE_CONTROL) {
        __GpsOhciReg->uiHcControl &= ~(1 << 4);
        return TRUE;
    } else if ((ucType == USB_TRAN_TYPE_BULK_OUT) || (ucType == USB_TRAN_TYPE_BULK_IN)) {
        __GpsOhciReg->uiHcControl &= ~(1 << 5);
        return TRUE;
    } else if ((ucType == USB_TRAN_TYPE_INTR_OUT) || (ucType == USB_TRAN_TYPE_INTR_IN)) {
        __GpsOhciReg->uiHcControl &= ~(1 << 2);
        return TRUE;
    } else if (ucType == USB_TRAN_TYPE_ISO) {
        __GpsOhciReg->uiHcControl &= ~((1 << 2) | (1 << 3));
        return TRUE;
    }

    return FALSE;
}

/*********************************************************************************************************
** Function name:       __hcWaitNextFrame
** Descriptions:        等待下一帧开始
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
void __hcWaitNextFrame (void)
{
    USB_INT32U uiTmp     = 0x0000ffff;
    USB_INT16U usFrameNum;

    usFrameNum = __usbGetFrameNumber();                                 /*  获取当前帧号                */
    while ((usFrameNum == __usbGetFrameNumber()) && (--uiTmp));         /*  等待下一帧开始              */
}

/*********************************************************************************************************
** Function name:       __hcDisEnSchedAndWaitNextFrame
** Descriptions:        禁止调度并等待下一帧开始
** input parameters:    ucType : 类型: 控制传输, 块传输, 中断传输, 同步传输
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __hcDisEnSchedAndWaitNextFrame (USB_INT8U ucType)
{
    if (TRUE != __hcDisEnSchedule(ucType)) {
        return FALSE;
    }
    __hcWaitNextFrame();
    return TRUE;
}

/*********************************************************************************************************
** Function name:       __hcStartSchedule
** Descriptions:        启动调度
** input parameters:    ucType : 类型: 控制传输, 块传输, 中断传输, 同步传输
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __hcStartSchedule (USB_INT8U ucType)
{
    if (ucType == USB_TRAN_TYPE_CONTROL) {
        __GpsOhciReg->uiHcCommandStatus |= 1 << 1;
        return TRUE;
    } else if ((ucType == USB_TRAN_TYPE_BULK_OUT) || (ucType == USB_TRAN_TYPE_BULK_IN)) {
        __GpsOhciReg->uiHcCommandStatus |= 1 << 2;
        return TRUE;
    } else if ((ucType == USB_TRAN_TYPE_INTR_OUT) || (ucType == USB_TRAN_TYPE_INTR_IN)) {
        return TRUE;
    }
    return FALSE;
}

/*********************************************************************************************************
** Function name:       __hcEnableAndStartSchedule
** Descriptions:        使能并启动调度
** input parameters:    ucType : 类型: 控制传输, 块传输, 中断传输, 同步传输
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __hcEnableAndStartSchedule (USB_INT8U ucType)
{
    if (!__hcEnableSchedule(ucType)) {                                  /*  使能调度                    */
        return FALSE;
    }
    return __hcStartSchedule(ucType);                                   /*  启动调度                    */
}


/*********************************************************************************************************
** Function name:       __usbGetFrameNumber
** Descriptions:        获取当前帧号
** input parameters:    None
** output parameters:   None
** Returned value:      16位的帧吨
*********************************************************************************************************/
USB_INT16U __usbGetFrameNumber (void)
{
#if 0

    return *((USB_INT16U *)(__GpsOhciReg->uiHcHCCA + 0x80));            /*  返回 HcFrameNumber 里的帧号 */

#endif

    return (USB_INT16U)(__GpsOhciReg->uiHcFmNumber & 0xFFFF);           /*  返回 HcFrameNumber 里的帧号 */
}


/*********************************************************************************************************
  根集线器管理部分
*********************************************************************************************************/

/*********************************************************************************************************
** Function name:       __ohciPortOpen
** Descriptions:        打开端口
** input parameters:    ucPortNum 端口号
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __ohciPortOpen (USB_INT8U ucPortNum)
{
    USB_INT32U uiTmp;

    if ((ucPortNum < 1) || (ucPortNum > USB_MAX_PORTS)) {
        USBDEBUG_SENDSTR("\r\n!!PortOpen:parameter error!\r\n");
        return FALSE;
    }

    OSTimeDlyHMSM(0, 0, 0, 100);

    if (__GpsOhciReg->uiHcRhPortStatus[ucPortNum - 1] & 0x01) {         /*  是否有设备插入              */

        __GpsOhciReg->uiHcRhPortStatus[ucPortNum - 1] |= (0x1 << 4);    /*  Port Reset (status)         */
        OSTimeDly(1);

        uiTmp = 0xffff;
        while (!(__GpsOhciReg->uiHcRhPortStatus[ucPortNum - 1] & (0x10 << 16)) && --uiTmp);
        if (uiTmp == 0) {
            return FALSE;
        }
        if ((__GpsOhciReg->uiHcRhPortStatus[ucPortNum - 1] & __OHCI_RH_LSDA) == __OHCI_SPEED_LOW) {
            __GucUsbSpeed = __HC_SPEED_LOW;
        } else {
            __GucUsbSpeed = __HC_SPEED_FULL;
        }
        /**
         *  开启 ucPortNum 端口电源,并使能该端口
         */
        __GpsOhciReg->uiHcRhPortStatus[ucPortNum - 1] = (1UL << 8)      /*  PPS,PortPowerStatus         */
                                                      | (1UL << 1);     /*  PES,PowerEnableStatus       */
    }

    /**
     *  HC root hub descriptor register A set.the value equal to 0x0B01
     */
    __GpsOhciReg->uiHcRhDescriptorA = (0UL << 24)                   /* POTPGT,PowerOnToPowerGoodTime    */
                                    | (1UL << 12)                   /* NOCP,NoOverCurrentProtection     */
                                    | (0UL << 11)                   /* OCPM,OverCurrentProtectionMode   */
                                    | (1UL << 10)                   /* DT,DeviceType                    */
                                    | (1UL << 9)                    /* PSM,PowerSwitchingMode           */
                                    | (0UL << 8)                    /* NPS,NoPowerSwitching             */
                                    | (1UL << 0);                   /* NDP,NumberDownstreamPorts        */

    __GpsOhciReg->uiHcRhDescriptorB = 0UL;

    return TRUE;
}

/*********************************************************************************************************
** Function name:       __ohciPortClose
** Descriptions:        关闭端口
** input parameters:    ucPortNum 端口号
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __ohciPortClose (USB_INT8U ucPortNum)
{
    if ((ucPortNum < 1) || (ucPortNum > USB_MAX_PORTS)) {
        USBDEBUG_SENDSTR("\r\n!!PortClose:parameter error!\r\n");
        return FALSE;
    }

    OSTimeDly(1);

    __GpsOhciReg->uiHcRhPortStatus[ucPortNum - 1] &= ~((1U << 8) | (1U << 1));
                                                                        /*  PPS,PortPowerStatus         */
                                                                        /*  PES,PowerEnableStatus       */
    return TRUE;
}

/*********************************************************************************************************
** Function name:       __ohciAffirmAttach
** Descriptions:        重新确认是否有设备插入
** input parameters:    ucRhNum 端口号
** output parameters:   None
** Returned value:      TRUE : 成功  FALSE : 失败
*********************************************************************************************************/
USB_BOOL __ohciAffirmAttach (USB_INT8U ucRhNum)
{
    if (ucRhNum < 1) {
        return FALSE;
    }

    if (__GpsOhciReg->uiHcRhPortStatus[ucRhNum - 1] & (1 << 0)) {
        return TRUE;
    }
    return FALSE;
}

/*********************************************************************************************************
** Function name:       usbBusReset
** Descriptions:        复位总线
** input parameters:    None
** output parameters:   None
** Returned value:      None
** -------------------------------------------------------------------------------------------------------
** modify by            liuweiyun
** modify date          2009.01.14
** description          Write '0' to register HcRhPortStatus has no effect.
**                      So no need to use code like : HcRhPortStatus |= 1 << 4;
*********************************************************************************************************/
void usbBusReset (void)
{
    __GpsOhciReg->uiHcRhPortStatus[USB_HOST_PORT - 1] = 1 << 4;         /*  reset bus                   */
    USBDEBUG_SENDSTR("\r\nHost reset the bus\r\n");
}

/*********************************************************************************************************
** Function name:       __usbBusSusp
** Descriptions:        挂起总线
** input parameters:    None
** output parameters:   None
** Returned value:      None
** -------------------------------------------------------------------------------------------------------
** modify by            liuweiyun
** modify date          2009.01.14
** description          Write '0' to register HcRhPortStatus has no effect.
**                      So no need to use code like : HcRhPortStatus |= 1 << 4;
*********************************************************************************************************/
void usbBusSusp (void)
{
    __GpsOhciReg->uiHcRhPortStatus[USB_HOST_PORT - 1] = 1 << 2;         /*  挂起总线                    */
    USBDEBUG_SENDSTR("\r\nHost Suspend the bus\r\n");
}

/*********************************************************************************************************
** Function name:       usbBusResume
** Descriptions:        唤醒总线
** input parameters:    None
** output parameters:   None
** Returned value:      None
** -------------------------------------------------------------------------------------------------------
** modify by            liuweiyun
** modify date          2009.01.14
** description          Write '0' to register HcRhPortStatus has no effect.
**                      So no need to use code like : HcRhPortStatus |= 1 << 4;
*********************************************************************************************************/
void usbBusResume (void)
{
    __GpsOhciReg->uiHcRhPortStatus[USB_HOST_PORT - 1] = 1 << 3;         /*  唤醒总线                    */
    USBDEBUG_SENDSTR("\r\nHost resume the bus\r\n");
}


/*********************************************************************************************************
  USB RAM管理部分
  下面的函数主要用于管理USB RAM中的数据缓冲区
  由于调用此类函数的父函数已经限制为互斥访问,故此处没有添加额外的保证互斥访问手段
*********************************************************************************************************/

static USB_INT8U *__GpucCtrlBufCurrPos;                                 /*  控制端点数据缓冲区当前位置  */
static USB_INT8U *__GpucGenBufCurrPos;                                  /*  批量与中断端点缓冲区当前位置*/

/*********************************************************************************************************
** Function name:       __hcdAllocBufferInit
** Descriptions:        初始化数据缓冲区指针
** input parameters:    None
** output parameters:   None
** Returned value:
*********************************************************************************************************/
void __hcdAllocBufferInit (void)
{
    __GpucCtrlBufCurrPos = (USB_INT8U *)__OHCI_DATA_BASE_ADDR_CTRL;
    __GpucGenBufCurrPos  = (USB_INT8U *)__OHCI_DATA_BASE_ADDR_GEN;
}

/*********************************************************************************************************
** Function name:       __usbAllocCtrlBuffer
** Descriptions:        申请控制端点缓冲区
** input parameters:    uiLen   缓冲区长度
** output parameters:   None
** Returned value:      申请到的缓冲区指针
*********************************************************************************************************/
USB_INT8U *__usbAllocCtrlBuffer (USB_INT32U uiLen)
{
    USB_INT8U *pucTmp;

    if ((USB_INT32U)__GpucCtrlBufCurrPos + uiLen >
         __OHCI_DATA_BASE_ADDR_CTRL + __OHCI_DATA_CTRL_BUFFER_LEN) {
        pucTmp = NULL;
    } else {
        pucTmp = __GpucCtrlBufCurrPos;
        usbMemSet(pucTmp, 0, uiLen);
        __GpucCtrlBufCurrPos = __GpucCtrlBufCurrPos + uiLen;
    }

    return pucTmp;
}

/*********************************************************************************************************
** Function name:       __usbAllocGenBuffer
** Descriptions:        申请批量,中断端点缓冲区
** input parameters:    uiLen   缓冲区长度
** output parameters:   None
** Returned value:      申请到的缓冲区指针
*********************************************************************************************************/
USB_INT8U *__usbAllocGenBuffer (USB_INT32U uiLen)
{
    USB_INT8U *pucTmp;

    if ((USB_INT32U)__GpucGenBufCurrPos + uiLen > __OHCI_USB_RAM_END_ADDR) {
        pucTmp = NULL;
    } else {
        pucTmp = __GpucGenBufCurrPos;
        __GpucGenBufCurrPos = __GpucGenBufCurrPos + uiLen;
    }

    return pucTmp;
}

/*********************************************************************************************************
** Function name:       __usbFreeCtrlBuffer
** Descriptions:        释放控制端点的数据缓冲区
** input parameters:    uiLen   缓冲区长度
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __usbFreeCtrlBuffer (USB_INT32U uiLen)
{
    if ((USB_INT32U)__GpucCtrlBufCurrPos - uiLen >= __OHCI_DATA_BASE_ADDR_CTRL) {
        __GpucCtrlBufCurrPos = __GpucCtrlBufCurrPos - uiLen;
    } else {
        __GpucCtrlBufCurrPos = (USB_INT8U *)__OHCI_DATA_BASE_ADDR_CTRL;
    }
}

/*********************************************************************************************************
** Function name:       __usbFreeGenBuffer
** Descriptions:        释放批量,中断端点的数据缓冲区
** input parameters:    uiLen   缓冲区长度
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __usbFreeGenBuffer (USB_INT32U uiLen)
{
    if ((USB_INT32U)__GpucGenBufCurrPos - uiLen >= __OHCI_DATA_BASE_ADDR_GEN) {
        __GpucGenBufCurrPos = __GpucGenBufCurrPos - uiLen;
    }
}

/*********************************************************************************************************
** Function name:       __usbFreeAllCtrlBuffer
** Descriptions:        释放所有控制端点的数据缓冲区
** input parameters:    None
** output parameters:   None
** Returned value:
*********************************************************************************************************/
void __usbFreeAllCtrlBuffer (void)
{
    __GpucCtrlBufCurrPos = (USB_INT8U *)__OHCI_DATA_BASE_ADDR_CTRL;
}

/*********************************************************************************************************
** Function name:       __usbFreeAllGenBuffer
** Descriptions:        释放所有批量,中断端点的数据缓冲区
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __usbFreeAllGenBuffer (void)
{
    __GpucGenBufCurrPos  = (USB_INT8U *)__OHCI_DATA_BASE_ADDR_GEN;
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
