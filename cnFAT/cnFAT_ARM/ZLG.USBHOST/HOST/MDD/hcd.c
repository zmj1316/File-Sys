/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            hcd.c
** Latest modified Date: 2007-11-06
** Latest Version:       V1.0
** Description:          HCD������
**--------------------------------------------------------------------------------------------------------
** Created By:           ��¡��         Longsui Wu
** Created date:         2007-11-06
** Version:              V1.0
** Descriptions:         ��ʼ�汾
**--------------------------------------------------------------------------------------------------------
** Modified by:          liuweiyun
** Modified date:        2009-01-09
** Version:              V1.0
** Description:          ��ֲ��LPC3200������__GpsOhciReg�Ķ��壬��ӡ��޸�ע�ͣ��Ľ�һЩ�����Ŀɶ��Ե�
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
*********************************************************************************************************/
#define USB_RAM_ALLOCATE
#include "..\USBHostIncludes.h"


/*********************************************************************************************************
  HCD��,��Ҫ�������¼�������:
  1. ���������,����ED��ED�������,TD��TD�������
  2. HC������
  3. ��������������
  4. USB RAM������
*********************************************************************************************************/

/*********************************************************************************************************
  ���������,����ED��ED�������,TD��TD�������
*********************************************************************************************************/

__POHCI_ED         __GpohciEd;
__POHCI_TD         __GpohciTd;
__OHCI_TD_STAT     __GohciTdStat;
__TRAN_END_TD      __GsTranEndTd;

volatile USB_INT8U __GucUsbSpeed   = 0;                                 /*  �豸�ٶ�,ȫ��(0)�����(1)   */
volatile USB_INT8U __GucUsbDevAddr = 0;                                 /*  �豸��ַ                    */


void __hcdAllocBufferInit (void);

/*********************************************************************************************************
** Function name:       __hcdEdInit
** Descriptions:        ��ʼ���� ED
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
** Descriptions:        TD ��ʼ��
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __hcdTdInit (void)
{
    __GpohciTd     = (__POHCI_TD)__OHCI_TD_BASE_ADDR;
    usbMemSet(&__GohciTdStat, 0, sizeof(__GohciTdStat));                /*  �������TDʹ�ñ�־          */
    usbMemSet(__GpohciTd, 0, sizeof(__OHCI_TD));                        /*  ��������TD                  */
}

/*********************************************************************************************************
** Function name:       __hcdInit
** Descriptions:        HCD������������ʼ��
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
** Descriptions:        ��ȡED�������Ϣ
** input parameters:    ucTranType  ��������
** output parameters:   pedInfo     ��ȡ������Ϣ
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
** Descriptions:        ����TD�ĵ�ַ��ȡTD������
** input parameters:    tdTd: TDָ��,��TD��ַ
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
** Descriptions:        ������� TD
** input parameters:    pEdInfo: TD����ED����Ϣ�ṹ��
** output parameters:   None
** Returned value:      ���뵽�� TD �׵�ַ,������ʧ��,�򷵻� NULL
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
        for (i = 0; i < (pEdInfo->uiMaxTd +31) / 32; i++) {             /*  ���ҿ����õĿ���TD���ڵ��ֶ�*/
            if (*(pEdInfo->puiTdSts + i) != 0xFFFFFFFF) {
                break;
            }
        }
        if (i >= (pEdInfo->uiMaxTd +31) / 32) {                         /*  �޿���TD                    */
            break;
        }
        for (j = 0; j < 32; j++) {
            if ((*(pEdInfo->puiTdSts + i) & (0x01U << j)) == 0) {       /*  ���ҵ���һ��TD,������       */
                break;
            }
        }

        uiTdIndex = i * 32 + j;
        if (uiTdIndex >= pEdInfo->uiMaxTd) {                            /*  �õ���TD��Ŵ��ڵ������TD��*/
            break;                                                      /*  ˵����TD����Ч,����do..while*/
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
** Descriptions:        �ͷ�һ�� TD
** input parameters:    pEdInfo  Ҫ�ͷŵ�TD����ED����Ϣ�ṹ��
**                      phcdFreeTd Ҫ�ͷŵ�TD
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
    i = phcdFreeTd - pEdInfo->pTdHead;                                  /*  Ҫ���յ�TD�����            */
    j = i / 32;                                                         /*  Ҫ�ջص�TD��״̬λ��״̬    */
                                                                        /*  �����е��±�                */
    *(pEdInfo->puiTdSts + j) &= ~(1 << (i - j * 32));                   /*  �ڸ��±�Ԫ���е�λ���      */
    OS_EXIT_CRITICAL();
}

/*********************************************************************************************************
** Function name:       __hcdFreeAllTd
** Descriptions:        �ͷ����� TD
** input parameters:    pEdInfo  Ҫ�ͷŵ�TD����ED����Ϣ�ṹ��
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
** Descriptions:        ��Ŀ�� ED ����� TD,Ŀ�� ED �� pGenTdParam->ucType ָ��
** input parameters:    pGenTdParam: TD �Ĳ���
**                      ptdAddTd     Ҫ���ӵ�TD
**                      pEdInfo      ED��Ϣ�ṹ��
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL __hcdAddTd (__PHC_GEN_TD_PARAM            pGenTdParam,
                     __PHC_GEN_TRANSFER_DESCRIPTOR ptdAddTd,
                     __PED_INFO                    pEdInfo)
{
    USB_INT32U                   *puiHeap;
    USB_INT32U                    i;
    __PHC_GEN_TRANSFER_DESCRIPTOR ptdInsert = NULL;

    /*
     *  ���� TD ����
     */
    ptdAddTd->uiControl  = (pGenTdParam->ucDataToggle  << 24) |
                           (pGenTdParam->ucDelayIntr   << 21) |
                           (pGenTdParam->ucDirect_PID  << 19) |
                           (pGenTdParam->ucBufferRound << 18);
    ptdAddTd->pucCBP     = pGenTdParam->pucCBP;
    if (pGenTdParam->usBufLen != 0) {
        ptdAddTd->pucBufEnd = pGenTdParam->pucCBP + pGenTdParam->usBufLen - 1;
    } else {                                                            /*  ���Ϳհ�                    */
        pGenTdParam->pucCBP = NULL;
        ptdAddTd->pucBufEnd = ptdAddTd->pucCBP;
    }

    /*
     *  ��TD����ED.�����EDΪ��,��ֱ�ӽ�TD����ED��TD List
     *  ����TD�����ED��β��
     */
    OS_ENTER_CRITICAL();
    if (((USB_INT32U)(pEdInfo->pEd->ptdHeadP) & 0xFFFFFFF0) == \
        ((USB_INT32U)(pEdInfo->pEd->ptdTailP) & 0xFFFFFFF0)) {
                                                                        /*  ���Ϊ��ED,��ֱ�ӽ�TD����ED */
        puiHeap                = (USB_INT32U *)&(pEdInfo->pEd->ptdHeadP);
        i                      = *puiHeap;
        pEdInfo->pEd->ptdHeadP = ptdAddTd;
        *puiHeap              |= i & 0x02;
    } else {                                                            /*  �� ED �Ѿ����� TD           */
        ptdInsert = (__PHC_GEN_TRANSFER_DESCRIPTOR)((USB_INT32U)(pEdInfo->pEd->ptdHeadP) & 0xFFFFFFF0);
        while (((USB_INT32U)ptdInsert->ptdNextTD & 0xFFFFFFF0) !=       /*  Ѱ�����һ�� TD             */
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
** Descriptions:        ��ȡ ED �׵�ַ
** input parameters:    ucTranType  ��������
** output parameters:   None
** Returned value:      ED �׵�ַ
*********************************************************************************************************/
__PHC_ENDPOINT_DESCRIPTOR __hcdGetEdPtr (USB_INT8U ucTranType)
{
    __PHC_ENDPOINT_DESCRIPTOR pedEd;

    switch (ucTranType) {

    case USB_TRAN_TYPE_CONTROL:                                         /*  Control �˵�                */
        pedEd = &(__GpohciEd->edsControl);
        break;

    case USB_TRAN_TYPE_BULK_OUT:                                        /*  Bulk Out �˵�               */
        pedEd = &(__GpohciEd->edsBulkOut);
        break;

    case USB_TRAN_TYPE_BULK_IN:                                         /*  Bulk In �˵�                */
        pedEd = &(__GpohciEd->edsBulkIn);
        break;

    case USB_TRAN_TYPE_INTR_OUT:                                        /*  Intrrupt Out �˵�           */
        pedEd = &(__GpohciEd->edsIntrOut);
        break;

    case USB_TRAN_TYPE_INTR_IN:                                         /*  Intrrupt In �˵�            */
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
** Descriptions:        ����ED�е�skipλ���ȴ���һ֡��ʼ
** input parameters:    ucType : ����: ���ƴ���, �鴫��, �жϴ���, ͬ������
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL __usbEdSetSkipAndWaitNextFrame (USB_INT8U ucType)
{
    __PHC_ENDPOINT_DESCRIPTOR pedEd;

    pedEd             = __hcdGetEdPtr(ucType);                          /*  ��ȡ�˵��������׵�ַ        */
    pedEd->uiControl |= 1 << 14;                                        /*  ���� sKip = 1               */
    __hcWaitNextFrame();                                                /*  �ȴ���һ֡                  */
    return TRUE;
}

/*********************************************************************************************************
** Function name:       __usbEdSetSkipAndWaitNextFrame
** Descriptions:        ����ED�е�skipλ���ȴ���һ֡��ʼ
** input parameters:    ucType : ����: ���ƴ���, �鴫��, �жϴ���, ͬ������
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
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
** Descriptions:        �ж� ED �е� HeadP �Ƿ���� TailP
** input parameters:    ED����Ϣ�ṹ��
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
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
** Descriptions:        ǿ��ʹ ED �е� HeadP ���� TailP, ���ڴ�����ִ���ʱ����
** input parameters:    ED����Ϣ�ṹ��
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
    *puiHeadp |= uiTmp & 0x02;                                          /*  ���� ToggleCarry λ         */
    OS_EXIT_CRITICAL();
}


/*********************************************************************************************************
** Function name:       __hcdEnableEd
** Descriptions:        ʹ��ED
** input parameters:    ucEdType: ED ����
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
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
    OHCI �Ĵ�����ַָ�룬�Ѿ��滻�ɺ궨�壬��usbreg.h
*********************************************************************************************************/
#if 0
volatile const __POHCI_REG __GpsOhciReg = (volatile const __POHCI_REG)(__OHCI_REG_BASE_ADDR);
#endif



/*********************************************************************************************************
  HC������
*********************************************************************************************************/

/*********************************************************************************************************
** Function name:       __ohciInit
** Descriptions:        OHCI��ʼ��,ϵͳ��ʼ��ʱ����
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL __ohciInit (void)
{
    USB_INT8U  i;
    USB_INT32U uiTmp = 0x0000ffff;

    __GpsOhciReg->uiHcControl = 0;                                      /*  ��λ                        */
    OSTimeDly(1);

    __GpsOhciReg->uiHcCommandStatus = 0x01;                             /*  ��λ HC                     */
    while ((__GpsOhciReg->uiHcCommandStatus & 0x01) && --uiTmp);        /*  ��λ������10uS�����        */
    if (uiTmp == 0) {
        return FALSE;
    }

    uiTmp = __GpsOhciReg->uiHcRevision;                                 /*  ��ȡHC�汾��                */
    if ( (uiTmp & 0xFF) != 0x10) {
        return( FALSE );
    }

    __GpsOhciReg->uiHcInterruptDisable = 0x7FUL | (0x3UL << 30);        /*  ��ֹ�����ж�, ����ֹ MIE λ */
    __GpsOhciReg->uiHcRhStatus = (0x1UL << 16) | (0x1UL << 15);         /*  ʹ��ȫ�ֵ�Դ��LPSCλ        */

    OSTimeDlyHMSM(0, 0, 0, 200);

    /*
     *  ��������豸����
     */
    for (i = 0; i < USB_MAX_PORTS; i++) {
        if (__GpsOhciReg->uiHcRhPortStatus[i] & (1 << 0)) {
            uiTmp = (0x10 << 24) | ((i + 1) << 8) | (__GpsOhciReg->uiHcRhPortStatus[i] & 0x01);
            OSQPost(__GevtUsbMsgQeue, (void *)uiTmp);
            break;
        }
    }

    __GpsOhciReg->uiHcInterruptStatus = 0xC000007F;                     /*  ��������жϱ�־            */
    __ohciEnableInt(__USB_INT_RHSC);

    return TRUE;
}

/*********************************************************************************************************
** Function name:       __ohciInit2
** Descriptions:        OHCI��ʼ��,��⵽�豸����ʱ����
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL __ohciInit2 (USB_INT8U ucPort)
{
    USB_INT32U  uiTmp;
    USB_INT32U *puiTmp = NULL;

    puiTmp = (USB_INT32U *)__OHCI_HCCA_BASE_ADDR;

    /**
     *  ����HCCA����,256�ֽ�
     */
    (void)usbMemSet(puiTmp, 0, 256);
    /**
     *  HccaInterruptTable init,32 * 4 bytes
     */
    for (uiTmp = 0; uiTmp < 32; uiTmp++) {
        puiTmp[uiTmp] = (USB_INT32U)&(__GpohciEd->edsIntrOut);
    }

    __GpsOhciReg->uiHcControl = 0;                                      /*  ��λ                        */
    OSTimeDly(1);

    uiTmp = 0x0000ffff;
    __GpsOhciReg->uiHcCommandStatus = 0x01;                             /*  ��λ HC                     */
    while ((__GpsOhciReg->uiHcCommandStatus & 0x01) && --uiTmp);
    if (uiTmp == 0) {
        return FALSE;
    }

    __GpsOhciReg->uiHcInterruptDisable = 0x7F | (0x03U << 30);          /*  ��ֹ�����ж�, ����ֹ MIE λ */

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
     *  ���������ϵ�ʱ���ʸ�������ǰ��ȴ�ʱ������
     */
    uiTmp = (__OHCI_PWRON_TO_PWRGOOD << 24) & 0xFF000000;               /*  PowerOnToPowerGoodTime      */
    __GpsOhciReg->uiHcRhDescriptorA = uiTmp;
    uiTmp = (uiTmp >> 24) << 1;                                         /*  ʵ�ʵȴ�ʱ��:POTPGT * 2 ms  */
    OSTimeDlyHMSM(0, 0, 0, (USB_INT16U)uiTmp);

    __GpsOhciReg->uiHcRhDescriptorB = 0x0000;

    __GpsOhciReg->uiHcRhStatus = (0x01 << 16) | (1 << 15);              /*  ʹ��ȫ�ֵ�Դ��LPSCλ        */

    OSTimeDlyHMSM(0, 0, 0, 200);
	__ohciPortOpen(ucPort);

    /*
     *  ʹ HC ���� Operational ״̬,CBSR = 3 (�� 4 : 1)
     */
    __GpsOhciReg->uiHcControl = (0x02 << 6) | (0x03 << 0);

    __GpsOhciReg->uiHcInterruptStatus = 0xC000007F;                     /*  ��������жϱ�־            */
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
** Descriptions:        ʹ���ж�
** input parameters:    uiIntIndex �ж�����
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL __ohciEnableInt (USB_INT32U uiIntIndex)
{
    INT32U i, j;

    __GpsOhciReg->uiHcInterruptStatus |= uiIntIndex;

    /*
     *  ʹ���ж�, ����ʹ����󽫼Ĵ���ֵ���ؽ���У��,���У��ʧ��������
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
** Descriptions:        ��ֹ�ж�
** input parameters:    uiIntIndex �ж�����
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __ohciDisEnInt (USB_INT32U uiIntIndex)
{
    __GpsOhciReg->uiHcInterruptDisable = uiIntIndex;
}

/*********************************************************************************************************
** Function name:       __ohciDisEnIntAll
** Descriptions:        ��ֹ�����ж�
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
** Descriptions:        ʹ�ܵ���
** input parameters:    ucType : ����: ���ƴ���, �鴫��, �жϴ���, ͬ������
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
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
** Descriptions:        ��ֹ����
** input parameters:    ucType : ����: ���ƴ���, �鴫��, �жϴ���, ͬ������
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
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
** Descriptions:        �ȴ���һ֡��ʼ
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
void __hcWaitNextFrame (void)
{
    USB_INT32U uiTmp     = 0x0000ffff;
    USB_INT16U usFrameNum;

    usFrameNum = __usbGetFrameNumber();                                 /*  ��ȡ��ǰ֡��                */
    while ((usFrameNum == __usbGetFrameNumber()) && (--uiTmp));         /*  �ȴ���һ֡��ʼ              */
}

/*********************************************************************************************************
** Function name:       __hcDisEnSchedAndWaitNextFrame
** Descriptions:        ��ֹ���Ȳ��ȴ���һ֡��ʼ
** input parameters:    ucType : ����: ���ƴ���, �鴫��, �жϴ���, ͬ������
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
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
** Descriptions:        ��������
** input parameters:    ucType : ����: ���ƴ���, �鴫��, �жϴ���, ͬ������
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
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
** Descriptions:        ʹ�ܲ���������
** input parameters:    ucType : ����: ���ƴ���, �鴫��, �жϴ���, ͬ������
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL __hcEnableAndStartSchedule (USB_INT8U ucType)
{
    if (!__hcEnableSchedule(ucType)) {                                  /*  ʹ�ܵ���                    */
        return FALSE;
    }
    return __hcStartSchedule(ucType);                                   /*  ��������                    */
}


/*********************************************************************************************************
** Function name:       __usbGetFrameNumber
** Descriptions:        ��ȡ��ǰ֡��
** input parameters:    None
** output parameters:   None
** Returned value:      16λ��֡��
*********************************************************************************************************/
USB_INT16U __usbGetFrameNumber (void)
{
#if 0

    return *((USB_INT16U *)(__GpsOhciReg->uiHcHCCA + 0x80));            /*  ���� HcFrameNumber ���֡�� */

#endif

    return (USB_INT16U)(__GpsOhciReg->uiHcFmNumber & 0xFFFF);           /*  ���� HcFrameNumber ���֡�� */
}


/*********************************************************************************************************
  ��������������
*********************************************************************************************************/

/*********************************************************************************************************
** Function name:       __ohciPortOpen
** Descriptions:        �򿪶˿�
** input parameters:    ucPortNum �˿ں�
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
*********************************************************************************************************/
USB_BOOL __ohciPortOpen (USB_INT8U ucPortNum)
{
    USB_INT32U uiTmp;

    if ((ucPortNum < 1) || (ucPortNum > USB_MAX_PORTS)) {
        USBDEBUG_SENDSTR("\r\n!!PortOpen:parameter error!\r\n");
        return FALSE;
    }

    OSTimeDlyHMSM(0, 0, 0, 100);

    if (__GpsOhciReg->uiHcRhPortStatus[ucPortNum - 1] & 0x01) {         /*  �Ƿ����豸����              */

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
         *  ���� ucPortNum �˿ڵ�Դ,��ʹ�ܸö˿�
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
** Descriptions:        �رն˿�
** input parameters:    ucPortNum �˿ں�
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
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
** Descriptions:        ����ȷ���Ƿ����豸����
** input parameters:    ucRhNum �˿ں�
** output parameters:   None
** Returned value:      TRUE : �ɹ�  FALSE : ʧ��
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
** Descriptions:        ��λ����
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
** Descriptions:        ��������
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
    __GpsOhciReg->uiHcRhPortStatus[USB_HOST_PORT - 1] = 1 << 2;         /*  ��������                    */
    USBDEBUG_SENDSTR("\r\nHost Suspend the bus\r\n");
}

/*********************************************************************************************************
** Function name:       usbBusResume
** Descriptions:        ��������
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
    __GpsOhciReg->uiHcRhPortStatus[USB_HOST_PORT - 1] = 1 << 3;         /*  ��������                    */
    USBDEBUG_SENDSTR("\r\nHost resume the bus\r\n");
}


/*********************************************************************************************************
  USB RAM������
  ����ĺ�����Ҫ���ڹ���USB RAM�е����ݻ�����
  ���ڵ��ô��ຯ���ĸ������Ѿ�����Ϊ�������,�ʴ˴�û����Ӷ���ı�֤��������ֶ�
*********************************************************************************************************/

static USB_INT8U *__GpucCtrlBufCurrPos;                                 /*  ���ƶ˵����ݻ�������ǰλ��  */
static USB_INT8U *__GpucGenBufCurrPos;                                  /*  �������ж϶˵㻺������ǰλ��*/

/*********************************************************************************************************
** Function name:       __hcdAllocBufferInit
** Descriptions:        ��ʼ�����ݻ�����ָ��
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
** Descriptions:        ������ƶ˵㻺����
** input parameters:    uiLen   ����������
** output parameters:   None
** Returned value:      ���뵽�Ļ�����ָ��
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
** Descriptions:        ��������,�ж϶˵㻺����
** input parameters:    uiLen   ����������
** output parameters:   None
** Returned value:      ���뵽�Ļ�����ָ��
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
** Descriptions:        �ͷſ��ƶ˵�����ݻ�����
** input parameters:    uiLen   ����������
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
** Descriptions:        �ͷ�����,�ж϶˵�����ݻ�����
** input parameters:    uiLen   ����������
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
** Descriptions:        �ͷ����п��ƶ˵�����ݻ�����
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
** Descriptions:        �ͷ���������,�ж϶˵�����ݻ�����
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
