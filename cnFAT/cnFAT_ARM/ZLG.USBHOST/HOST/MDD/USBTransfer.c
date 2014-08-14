/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            USBTransfer.c
** Latest modified Date: 2007-11-14
** Latest Version:       V1.0
** Description:          USB����������Ĵ��亯��
**--------------------------------------------------------------------------------------------------------
** Created By:           ��¡��   Longsui Wu
** Created date:         2007-11-14
** Version:              V1.0
** Descriptions:         ��ʼ�汾
**--------------------------------------------------------------------------------------------------------
** Modified by:          liuweiyun
** Modified date:        2009-01-17
** Version:              V1.0
** Description:          Improve some codes
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
*********************************************************************************************************/
#include "..\USBHostIncludes.h"

USB_INT8U __usbCtrlTranPhase_Setup (PUSB_DEV_REQ pusbDevReq);
USB_INT8U __usbCtrlTranPhase_In (USB_INT16U usLength, USB_INT8U *pucBuf);
USB_INT8U __usbCtrlTranPhase_Out (void);

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************************
** Function name:       usbStandardReqTransfer
** Descriptions:        ���ͱ�׼����, ��ȡ�÷���ֵ
** input parameters:    pusbDevReq  ��׼����
** output parameters:   pucBuf      �豸���ص�����
**                      puiSts      ״̬,�����ŵ�
** Returned value:      ������
*********************************************************************************************************/
USB_INT8U usbStandardReqTransfer (PUSB_DEV_REQ pusbDevReq, USB_INT8U *pucBuf)
{
    USB_INT8U ucErr, ucRet;

    if (GusbHostFlags.ucAttached == 0) {                                /*  �豸δ����                  */
        return USB_ERR_DEVICE_NOT_ATTACHED;
    }

    OS_ENTER_CRITICAL();
    if (GusbHostFlags.bExitStdOperaReq) {                               /*  Ӧ�ó���Ҫ���˳�����        */
        OS_EXIT_CRITICAL();
        return USB_ERR_EXIT_REQ;
    }
    __GusbHostEvtCnt.ucStdTranCnt++;                                    /*  ���ƴ���������Լ�1         */
    OS_EXIT_CRITICAL();

    OSSemPend(__GevtUsbCtrlSem, 0, &ucErr);

    ucRet = __usbCtrlTranPhase_Setup(pusbDevReq);                       /*  ��һ��: SETUP �׶�          */
    if (USB_ERR_SUCESS == ucRet) {
        ucRet = __usbCtrlTranPhase_In(pusbDevReq->wLength, pucBuf);     /*  �ڶ���: IN �׶�             */
        if (USB_ERR_SUCESS == ucRet) {
            if (pusbDevReq->wLength > 0) {                              /*  ��IN �׶������ݴ�����ִ��   */
                (void)__usbCtrlTranPhase_Out();                         /*  ������: OUT �׶�            */
            }
        }
    }

    OSSemPost(__GevtUsbCtrlSem);

    OS_ENTER_CRITICAL();
    __GusbHostEvtCnt.ucStdTranCnt--;                                    /*  ���ƴ���������Լ�1         */
    OS_EXIT_CRITICAL();

    return ucRet;
}

/*********************************************************************************************************
** Function name:       usbDataTransfer
** Descriptions:        ���ͻ���� Bulk, Intrrupt ���������
** input parameters:    pucData       Ҫ���ͻ���յ����ݻ�����
**                      uiLength      Ҫ���ͻ�������ݵĳ���
**                      uiTranType    ��������: __HC_ED_TYPE_BULK �� __HC_ED_TYPE_INTR
**                      ucMaxTryCount �����ִ���ʱ,�������Դ���
** Returned value:      ������
*********************************************************************************************************/
USB_INT8U usbDataTransfer (USB_INT8U *pucData,                          /*  ���ݻ�����                  */
                           USB_INT32U uiLength,                         /*  Ҫ��������ݳ���            */
                           USB_INT8U  ucTranType,                       /*  ��������                    */
                           USB_INT8U  ucMaxTryCount)                    /*  ��������ʱ����Դ���      */
{
    USB_INT8U                     ucErr;
    USB_INT8U                     ucTryCount;
    USB_INT32U                    uiCode;

    USB_INT16U                    usMaxLenPerPkt;                       /*  ÿ��������󳤶�            */
    USB_INT16U                    usCurTran;
    USB_INT32U                    uiTotalTran;
    USB_INT32U                    uiBufLen;

    USB_INT8U                    *pucBufTmp;
    USB_INT8U                    *pucFristBuf;

    __HC_GEN_TD_PARAM             tdParam;
    __PHC_GEN_TRANSFER_DESCRIPTOR ptdAddTd;
    __ED_INFO                     edInfo;


    if (!usbIsDeviceReady()) {
        return USB_ERR_DEVICE_NOT_READY;
    }

    OS_ENTER_CRITICAL();
    if (GusbHostFlags.bExitDataOperaReq) {                              /*  Ӧ�ó���Ҫ���˳�����        */
        OS_EXIT_CRITICAL();
        return USB_ERR_EXIT_REQ;
    }

    __GusbHostEvtCnt.ucDataTranCnt++;
    OS_EXIT_CRITICAL();

    usbPipeOpen(ucTranType);
    __hcdGetEdInfo(ucTranType, &edInfo);
    usMaxLenPerPkt = edInfo.usMaxPktSize;

    OSSemPend(__GevtUsbDataTranSem, 0, &ucErr);

    ucTryCount = 0;
    do {
        pucBufTmp = pucData;

        /*
         *  ���� TD �Ĳ���
         */
        tdParam.ucBufferRound = 1;
        tdParam.ucDelayIntr   = 0;
        tdParam.ucDataToggle  = 0;

        if (ucTranType & 0x80) {
            tdParam.ucDirect_PID = __OHCI_TD_TYPE_IN;
        } else {
            tdParam.ucDirect_PID = __OHCI_TD_TYPE_OUT;
        }

        /*
         *  Ϊ������Ӵ��������� TD ����������
         *  ���������Ƚϴ�ʱ���ܻ����TD ����,�򻺳�������,��ʱ����Ҫ�ָ�Ϊ���,
         *  �ָ��Ϊ�޷����뵽���� TD �����벻��������ʱ
         */
        uiTotalTran = 0;                                                /*  �����ܴ�����������          */
        do {
            uiBufLen = 0;
            do {
                ptdAddTd = __hcdAllocTd(&edInfo);                       /*  ���� TD                     */
                if (ptdAddTd == NULL) {
                    break;
                }


                if (uiLength - uiTotalTran <= usMaxLenPerPkt) {
                    usCurTran = (USB_INT16U)(uiLength - uiTotalTran);
                } else {
                    usCurTran = usMaxLenPerPkt;
                }

                tdParam.pucCBP = __usbAllocGenBuffer(usCurTran);
                if (tdParam.pucCBP == NULL) {                           /*  ��������޻���������,��,    */
                                                                        /*  ...������ѭ��               */
                    __hcdFreeTd(&edInfo, ptdAddTd);
                    break;
                }

                if (!(ucTranType & 0x80)) {                             /*  OUT ����                    */
                    usbMemCopy(tdParam.pucCBP, pucBufTmp, usCurTran);
                    pucBufTmp += usCurTran;
                } else {                                                /*  IN ����                     */
                    if (uiBufLen == 0) {
                        pucFristBuf = tdParam.pucCBP;                   /*  �����һ�εĻ�����λ��,     */
                    }                                                   /*  ...usbMemCopyʱҪ��         */
                }
                tdParam.usBufLen = usCurTran;
                __hcdAddTd(&tdParam, ptdAddTd, &edInfo);                /*  �����뵽��TD���뵽ED��      */
                uiTotalTran = (USB_INT32U)(uiTotalTran + usCurTran);
                uiBufLen = (USB_INT32U)(uiBufLen + usCurTran);
            } while (uiTotalTran < uiLength);

            __usbEdClearSkip(ucTranType);                               /*  ��� sKip λ                */
            __hcStartSchedule(ucTranType);

            uiCode = (USB_INT32U)OSMboxPend(__GevtUsbDataTranMbox, USB_TIMEOUT_TICK, &ucErr);
                                                                        /*  �ȴ��������������        */
            if (ucErr != OS_NO_ERR) {                                   /*  �ȴ���ʱ                    */
                uiCode = (USB_INT32U)(USB_ERR_TIMEOUT << 24);           /*  ��ʱ������                  */
                ucTryCount = (USB_INT8U)(ucMaxTryCount + 1);            /*  ʹ��ǰ���Դ��������������  */
                                                                        /*  ����,ʹ�����˳�do whileѭ�� */
                __hcDisEnSchedAndWaitNextFrame(ucTranType);
                __hcdEdLetHeadEquTail(&edInfo);                         /*  ǿ�� ʹHeadP = TailP        */
                __hcdFreeAllTd(&edInfo);                                /*  �ͷ����� TD ��Դ            */
                __hcEnableSchedule(ucTranType);
                break;
            }
            if (uiCode == __USB_TRANDEAL_OK) {
                if ((ucTranType & 0x80)) {                              /*  IN ����                     */
                    usbMemCopy(pucBufTmp, pucFristBuf, uiBufLen);       /*  ��USB RAM������ݿ�����     */
                                                                        /*  ...�û�������               */
                    pucBufTmp = pucBufTmp + uiBufLen;
                }
                __usbFreeGenBuffer(uiBufLen);                           /*  �ͷ���ռUSB RAM������       */
            } else if (__USBLSB(uiCode, 0) == __USB_TRANDEAL_ERR_TRY) { /*  ��Ҫ����                    */
                if (__USBLSB(uiCode, 3) == USB_ERR_STALL) {             /*  �˵㱻��ֹ, ��Ҫ���        */
                    usbClearFeature_EP(usbGetEp(0, ucTranType));
                }
                ucTryCount++;
                uiTotalTran = 0;
                break;
            } else {                                                    /*  �������ʹ���,���������˳�   */
                ucTryCount = (USB_INT8U)(ucMaxTryCount + 1);            /*  ...do..while ѭ��           */
                break;
            }
        } while (uiTotalTran < uiLength);                               /*  ֱ�����е����ݴ������      */
        __usbFreeAllGenBuffer();
    } while ((uiTotalTran < uiLength) && (ucTryCount <= ucMaxTryCount));

    OSSemPost(__GevtUsbDataTranSem);

    OS_ENTER_CRITICAL();
    __GusbHostEvtCnt.ucDataTranCnt--;
    OS_EXIT_CRITICAL();

    return __USBLSB(uiCode, 3);
}

#ifdef __cplusplus
    }
#endif

/*********************************************************************************************************
** Function name:       __usbCtrlTranPhase_Setup
** Descriptions:        ��׼����� SETUP �׶�
** input parameters:    pusbDevReq  ��׼��������
** output parameters:   None
** Returned value:      ������
*********************************************************************************************************/
USB_INT8U __usbCtrlTranPhase_Setup (PUSB_DEV_REQ pusbDevReq)
{
    USB_INT8U                     ucErr;
    USB_INT8U                     ucTryCount;
    USB_INT32U                    uiCode;

    USB_INT8U                    *pucBufReq = (USB_INT8U *)pusbDevReq;
    __HC_GEN_TD_PARAM             tdParam;
    __PHC_GEN_TRANSFER_DESCRIPTOR ptdAddTd;
    __ED_INFO                     edInfo;


    /*
     *  ���ƴ���: SETUP �׶�
     */
    tdParam.pucCBP       = pucBufReq;
    tdParam.usBufLen     = 8;

    tdParam.ucBufferRound = 1;
    tdParam.ucDirect_PID  = __OHCI_TD_TYPE_SETUP;
    tdParam.ucDelayIntr   = 0;
    tdParam.ucDataToggle  = __OHCI_TD_T_DATA0;

    tdParam.pucCBP = __usbAllocCtrlBuffer(tdParam.usBufLen);
    usbMemCopy(tdParam.pucCBP, pucBufReq, tdParam.usBufLen);

    __hcdGetEdInfo(USB_TRAN_TYPE_CONTROL, &edInfo);

    ucTryCount = 0;
    do {
        __usbEdSetSkipAndWaitNextFrame(USB_TRAN_TYPE_CONTROL);          /*  ����sKip=1,���ȴ�һ��֡��ʼ */

        ptdAddTd = __hcdAllocTd(&edInfo);                               /*  ���� TD                     */
        __hcdAddTd(&tdParam, ptdAddTd, &edInfo);                        /*  ���� SETUP TD               */

        __usbEdClearSkip(USB_TRAN_TYPE_CONTROL);                        /*  ���sKipλ                  */
        __hcStartSchedule(USB_TRAN_TYPE_CONTROL);                       /*  ��������                    */

        uiCode = (USB_INT32U)OSMboxPend(__GevtUsbCtrlMbox, USB_TIMEOUT_TICK, &ucErr);
        if (ucErr != OS_NO_ERR) {                                       /*  ��ʱ�˳�                    */
            uiCode = (USB_INT32U)(USB_ERR_TIMEOUT << 24);
            __hcdEdLetHeadEquTail(&edInfo);                             /*  ǿ�� ʹHeadP = TailP        */
            __hcdFreeAllTd(&edInfo);                                    /*  �ͷ����� TD ��Դ            */
            break;
        }
        if (uiCode == __USB_TRANDEAL_OK) {                              /*  ����ɹ�                    */
            break;
        } else if (__USBLSB(uiCode, 0) == __USB_TRANDEAL_ERR_TRY) {     /*  ��Ҫ���Դ���                */
            break;
        } else {
            break;
        }
    } while (++ucTryCount <= 3);
    __usbFreeCtrlBuffer(tdParam.usBufLen);                              /*  �ͷ���ռ��USB RAM������     */
    return __USBLSB(uiCode, 3);
}

/*********************************************************************************************************
** Function name:       __usbCtrlTranPhase_In
** Descriptions:        ��׼����� IN �׶�
** input parameters:    usLength  Ҫ��������ݳ���
** output parameters:   pucBuf    ���ڽ��ն�ȡ�������ݵĻ�����
** Returned value:      ������
*********************************************************************************************************/
USB_INT8U __usbCtrlTranPhase_In (USB_INT16U usLength, USB_INT8U *pucBuf)
{
    USB_INT8U                     ucErr;
    USB_INT32U                    uiCode;

    USB_INT8U                    *pucBufTmp;
    USB_INT8U                    *pucFristBuf;

    USB_INT16U                    usCurTran;                            /*  ��ǰ TD ��Ҫ��������ݳ���  */
    USB_INT16U                    usTotalTran;                          /*  ���� TD ��������ݳ���      */
    USB_INT16U                    usBufLen;                             /*  һ�ִ�������ݳ���          */

    USB_INT8U                     ucToggle;                             /*  ����λ(DATA0 or DATA1)      */
    __HC_GEN_TD_PARAM             tdParam;                              /*  TD �����б�                 */
    __PHC_GEN_TRANSFER_DESCRIPTOR ptdAddTd;
    __ED_INFO                     edInfo;


    tdParam.ucBufferRound = 1;
    tdParam.ucDirect_PID  = __OHCI_TD_TYPE_IN;
    tdParam.ucDelayIntr   = 0;

    /*
     *  ���ƴ���: IN �׶�
     */
    ucToggle    = 0;
    usCurTran   = 0;
    usTotalTran = 0;

    __hcdGetEdInfo(USB_TRAN_TYPE_CONTROL, &edInfo);                     /*  ��ȡ�˵���Ϣ                */

    pucBufTmp = pucBuf;

    do {
        usBufLen = 0;

        __usbEdSetSkipAndWaitNextFrame(USB_TRAN_TYPE_CONTROL);

        do {
            ptdAddTd = __hcdAllocTd(&edInfo);                           /*  ���� TD                     */
            if (ptdAddTd == NULL) {
                break;
            }

            if (usLength - usTotalTran <= edInfo.usMaxPktSize) {
                usCurTran = (USB_INT16U)(usLength - usTotalTran);
            } else {
                usCurTran = edInfo.usMaxPktSize;
            }
            tdParam.pucCBP = __usbAllocCtrlBuffer(usCurTran);
            if (tdParam.pucCBP == NULL) {                               /*  ��������޻���������,��,    */
                                                                        /*  ...������ѭ��               */
                __hcdFreeTd(&edInfo, ptdAddTd);
                break;
            }
            if (usBufLen == 0) {
                pucFristBuf = tdParam.pucCBP;                           /*  �����һ�εĻ�����λ��,     */
            }                                                           /*  ...usbMemCopyʱҪ��         */
            tdParam.usBufLen = usCurTran;
            if (++ucToggle & 0x01) {
                tdParam.ucDataToggle = __OHCI_TD_T_DATA1;
            } else {
                tdParam.ucDataToggle = __OHCI_TD_T_DATA0;
            }
            usTotalTran = (USB_INT16U)(usTotalTran + usCurTran);
            usBufLen = (USB_INT8U)(usBufLen + usCurTran);
            __hcdAddTd(&tdParam, ptdAddTd, &edInfo);
        } while (usTotalTran < usLength);

        __usbEdClearSkip(USB_TRAN_TYPE_CONTROL);
        __hcStartSchedule(USB_TRAN_TYPE_CONTROL);

        uiCode = (USB_INT32U)OSMboxPend(__GevtUsbCtrlMbox, USB_TIMEOUT_TICK, &ucErr);
        if (ucErr != OS_NO_ERR) {
            uiCode = (USB_INT32U)(USB_ERR_TIMEOUT << 24);
            __hcdEdLetHeadEquTail(&edInfo);
            __hcdFreeAllTd(&edInfo);
            break;
        }
        if (uiCode == __USB_TRANDEAL_OK) {
            usbMemCopy(pucBufTmp, pucFristBuf, usBufLen);
            pucBufTmp = pucBufTmp + usBufLen;
            __usbFreeCtrlBuffer(usBufLen);
        } else if (__USBLSB(uiCode, 0) == __USB_TRANDEAL_ERR_TRY) {
            break;
        } else {
            break;
        }
    } while (usTotalTran < usLength);

    __usbFreeAllCtrlBuffer();
    return __USBLSB(uiCode, 3);
}

/*********************************************************************************************************
** Function name:       __usbCtrlTranPhase_Out
** Descriptions:        ��׼����� OUT �׶�
** input parameters:    None
** output parameters:   None
** Returned value:      ������
*********************************************************************************************************/
USB_INT8U __usbCtrlTranPhase_Out (void)
{
    USB_INT8U                     ucErr;
    USB_INT32U                    uiCode;

    __HC_GEN_TD_PARAM             tdParam;
    __PHC_GEN_TRANSFER_DESCRIPTOR ptdAddTd;
    __ED_INFO                     edInfo;


    tdParam.pucCBP   = NULL;
    tdParam.usBufLen = 0;

    tdParam.ucDataToggle  = __OHCI_TD_T_DATA1;
    tdParam.ucBufferRound = 1;
    tdParam.ucDirect_PID  = __OHCI_TD_TYPE_OUT;
    tdParam.ucDelayIntr   = 0;

    __hcdGetEdInfo(USB_TRAN_TYPE_CONTROL, &edInfo);
    __usbEdSetSkipAndWaitNextFrame(USB_TRAN_TYPE_CONTROL);
    ptdAddTd = __hcdAllocTd(&edInfo);                                   /*  ���� TD                     */
    __hcdAddTd(&tdParam, ptdAddTd, &edInfo);                            /*  ���� SETUP TD               */
    __usbEdClearSkip(USB_TRAN_TYPE_CONTROL);
    __hcStartSchedule(USB_TRAN_TYPE_CONTROL);
    uiCode = (USB_INT32U)OSMboxPend(__GevtUsbCtrlMbox, USB_TIMEOUT_TICK, &ucErr);
    if (ucErr != OS_NO_ERR) {
        uiCode = (USB_INT32U)(USB_ERR_TIMEOUT << 24);
        __hcdEdLetHeadEquTail(&edInfo);
        __hcdFreeAllTd(&edInfo);
    }

    return __USBLSB(uiCode, 3);
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
