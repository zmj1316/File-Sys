/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            USBTransfer.c
** Latest modified Date: 2007-11-14
** Latest Version:       V1.0
** Description:          USB主机软件包的传输函数
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗   Longsui Wu
** Created date:         2007-11-14
** Version:              V1.0
** Descriptions:         初始版本
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
** Descriptions:        发送标准请求, 并取得返回值
** input parameters:    pusbDevReq  标准请求
** output parameters:   pucBuf      设备返回的数据
**                      puiSts      状态,如错误号等
** Returned value:      错误码
*********************************************************************************************************/
USB_INT8U usbStandardReqTransfer (PUSB_DEV_REQ pusbDevReq, USB_INT8U *pucBuf)
{
    USB_INT8U ucErr, ucRet;

    if (GusbHostFlags.ucAttached == 0) {                                /*  设备未插入                  */
        return USB_ERR_DEVICE_NOT_ATTACHED;
    }

    OS_ENTER_CRITICAL();
    if (GusbHostFlags.bExitStdOperaReq) {                               /*  应用程序要求退出操作        */
        OS_EXIT_CRITICAL();
        return USB_ERR_EXIT_REQ;
    }
    __GusbHostEvtCnt.ucStdTranCnt++;                                    /*  控制传输计数器自加1         */
    OS_EXIT_CRITICAL();

    OSSemPend(__GevtUsbCtrlSem, 0, &ucErr);

    ucRet = __usbCtrlTranPhase_Setup(pusbDevReq);                       /*  第一步: SETUP 阶段          */
    if (USB_ERR_SUCESS == ucRet) {
        ucRet = __usbCtrlTranPhase_In(pusbDevReq->wLength, pucBuf);     /*  第二步: IN 阶段             */
        if (USB_ERR_SUCESS == ucRet) {
            if (pusbDevReq->wLength > 0) {                              /*  若IN 阶段有数据传输则执行   */
                (void)__usbCtrlTranPhase_Out();                         /*  第三步: OUT 阶段            */
            }
        }
    }

    OSSemPost(__GevtUsbCtrlSem);

    OS_ENTER_CRITICAL();
    __GusbHostEvtCnt.ucStdTranCnt--;                                    /*  控制传输计数器自减1         */
    OS_EXIT_CRITICAL();

    return ucRet;
}

/*********************************************************************************************************
** Function name:       usbDataTransfer
** Descriptions:        发送或接收 Bulk, Intrrupt 传输的数据
** input parameters:    pucData       要发送或接收的数据缓冲区
**                      uiLength      要发送或接收数据的长度
**                      uiTranType    传输类型: __HC_ED_TYPE_BULK 或 __HC_ED_TYPE_INTR
**                      ucMaxTryCount 当出现错误时,最大的重试次数
** Returned value:      错误码
*********************************************************************************************************/
USB_INT8U usbDataTransfer (USB_INT8U *pucData,                          /*  数据缓冲区                  */
                           USB_INT32U uiLength,                         /*  要传输的数据长度            */
                           USB_INT8U  ucTranType,                       /*  传输类型                    */
                           USB_INT8U  ucMaxTryCount)                    /*  发生错误时最大尝试次数      */
{
    USB_INT8U                     ucErr;
    USB_INT8U                     ucTryCount;
    USB_INT32U                    uiCode;

    USB_INT16U                    usMaxLenPerPkt;                       /*  每包数据最大长度            */
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
    if (GusbHostFlags.bExitDataOperaReq) {                              /*  应用程序要求退出操作        */
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
         *  设置 TD 的参数
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
         *  为传输添加传输描述符 TD 并启动传输
         *  当数据量比较大时可能会出现TD 不够,或缓冲区不够,此时传输要分割为多次,
         *  分割点为无法申请到空闲 TD 或申请不到缓冲区时
         */
        uiTotalTran = 0;                                                /*  清零总传输量计数器          */
        do {
            uiBufLen = 0;
            do {
                ptdAddTd = __hcdAllocTd(&edInfo);                       /*  申请 TD                     */
                if (ptdAddTd == NULL) {
                    break;
                }


                if (uiLength - uiTotalTran <= usMaxLenPerPkt) {
                    usCurTran = (USB_INT16U)(uiLength - uiTotalTran);
                } else {
                    usCurTran = usMaxLenPerPkt;
                }

                tdParam.pucCBP = __usbAllocGenBuffer(usCurTran);
                if (tdParam.pucCBP == NULL) {                           /*  如果现在无缓冲区可用,则,    */
                                                                        /*  ...跳出本循环               */
                    __hcdFreeTd(&edInfo, ptdAddTd);
                    break;
                }

                if (!(ucTranType & 0x80)) {                             /*  OUT 传输                    */
                    usbMemCopy(tdParam.pucCBP, pucBufTmp, usCurTran);
                    pucBufTmp += usCurTran;
                } else {                                                /*  IN 传输                     */
                    if (uiBufLen == 0) {
                        pucFristBuf = tdParam.pucCBP;                   /*  保存第一次的缓冲区位置,     */
                    }                                                   /*  ...usbMemCopy时要用         */
                }
                tdParam.usBufLen = usCurTran;
                __hcdAddTd(&tdParam, ptdAddTd, &edInfo);                /*  将申请到的TD加入到ED中      */
                uiTotalTran = (USB_INT32U)(uiTotalTran + usCurTran);
                uiBufLen = (USB_INT32U)(uiBufLen + usCurTran);
            } while (uiTotalTran < uiLength);

            __usbEdClearSkip(ucTranType);                               /*  清除 sKip 位                */
            __hcStartSchedule(ucTranType);

            uiCode = (USB_INT32U)OSMboxPend(__GevtUsbDataTranMbox, USB_TIMEOUT_TICK, &ucErr);
                                                                        /*  等待调度任务处理完毕        */
            if (ucErr != OS_NO_ERR) {                                   /*  等待超时                    */
                uiCode = (USB_INT32U)(USB_ERR_TIMEOUT << 24);           /*  超时错误码                  */
                ucTryCount = (USB_INT8U)(ucMaxTryCount + 1);            /*  使当前重试次数大于最大重试  */
                                                                        /*  次数,使程序退出do while循环 */
                __hcDisEnSchedAndWaitNextFrame(ucTranType);
                __hcdEdLetHeadEquTail(&edInfo);                         /*  强制 使HeadP = TailP        */
                __hcdFreeAllTd(&edInfo);                                /*  释放所有 TD 资源            */
                __hcEnableSchedule(ucTranType);
                break;
            }
            if (uiCode == __USB_TRANDEAL_OK) {
                if ((ucTranType & 0x80)) {                              /*  IN 传输                     */
                    usbMemCopy(pucBufTmp, pucFristBuf, uiBufLen);       /*  将USB RAM里的数据拷贝至     */
                                                                        /*  ...用户缓冲区               */
                    pucBufTmp = pucBufTmp + uiBufLen;
                }
                __usbFreeGenBuffer(uiBufLen);                           /*  释放所占USB RAM缓冲区       */
            } else if (__USBLSB(uiCode, 0) == __USB_TRANDEAL_ERR_TRY) { /*  需要重试                    */
                if (__USBLSB(uiCode, 3) == USB_ERR_STALL) {             /*  端点被禁止, 需要解禁        */
                    usbClearFeature_EP(usbGetEp(0, ucTranType));
                }
                ucTryCount++;
                uiTotalTran = 0;
                break;
            } else {                                                    /*  其它类型错误,创造条件退出   */
                ucTryCount = (USB_INT8U)(ucMaxTryCount + 1);            /*  ...do..while 循环           */
                break;
            }
        } while (uiTotalTran < uiLength);                               /*  直到所有的数据传输完成      */
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
** Descriptions:        标准请求的 SETUP 阶段
** input parameters:    pusbDevReq  标准请求数据
** output parameters:   None
** Returned value:      错误码
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
     *  控制传输: SETUP 阶段
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
        __usbEdSetSkipAndWaitNextFrame(USB_TRAN_TYPE_CONTROL);          /*  设置sKip=1,并等待一下帧开始 */

        ptdAddTd = __hcdAllocTd(&edInfo);                               /*  申请 TD                     */
        __hcdAddTd(&tdParam, ptdAddTd, &edInfo);                        /*  增加 SETUP TD               */

        __usbEdClearSkip(USB_TRAN_TYPE_CONTROL);                        /*  清除sKip位                  */
        __hcStartSchedule(USB_TRAN_TYPE_CONTROL);                       /*  启动传输                    */

        uiCode = (USB_INT32U)OSMboxPend(__GevtUsbCtrlMbox, USB_TIMEOUT_TICK, &ucErr);
        if (ucErr != OS_NO_ERR) {                                       /*  超时退出                    */
            uiCode = (USB_INT32U)(USB_ERR_TIMEOUT << 24);
            __hcdEdLetHeadEquTail(&edInfo);                             /*  强制 使HeadP = TailP        */
            __hcdFreeAllTd(&edInfo);                                    /*  释放所有 TD 资源            */
            break;
        }
        if (uiCode == __USB_TRANDEAL_OK) {                              /*  传输成功                    */
            break;
        } else if (__USBLSB(uiCode, 0) == __USB_TRANDEAL_ERR_TRY) {     /*  需要重试传输                */
            break;
        } else {
            break;
        }
    } while (++ucTryCount <= 3);
    __usbFreeCtrlBuffer(tdParam.usBufLen);                              /*  释放所占的USB RAM缓冲区     */
    return __USBLSB(uiCode, 3);
}

/*********************************************************************************************************
** Function name:       __usbCtrlTranPhase_In
** Descriptions:        标准请求的 IN 阶段
** input parameters:    usLength  要传输的数据长度
** output parameters:   pucBuf    用于接收读取到的数据的缓冲区
** Returned value:      错误码
*********************************************************************************************************/
USB_INT8U __usbCtrlTranPhase_In (USB_INT16U usLength, USB_INT8U *pucBuf)
{
    USB_INT8U                     ucErr;
    USB_INT32U                    uiCode;

    USB_INT8U                    *pucBufTmp;
    USB_INT8U                    *pucFristBuf;

    USB_INT16U                    usCurTran;                            /*  当前 TD 所要传输的数据长度  */
    USB_INT16U                    usTotalTran;                          /*  所有 TD 传输的数据长度      */
    USB_INT16U                    usBufLen;                             /*  一轮传输的数据长度          */

    USB_INT8U                     ucToggle;                             /*  触发位(DATA0 or DATA1)      */
    __HC_GEN_TD_PARAM             tdParam;                              /*  TD 参数列表                 */
    __PHC_GEN_TRANSFER_DESCRIPTOR ptdAddTd;
    __ED_INFO                     edInfo;


    tdParam.ucBufferRound = 1;
    tdParam.ucDirect_PID  = __OHCI_TD_TYPE_IN;
    tdParam.ucDelayIntr   = 0;

    /*
     *  控制传输: IN 阶段
     */
    ucToggle    = 0;
    usCurTran   = 0;
    usTotalTran = 0;

    __hcdGetEdInfo(USB_TRAN_TYPE_CONTROL, &edInfo);                     /*  获取端点信息                */

    pucBufTmp = pucBuf;

    do {
        usBufLen = 0;

        __usbEdSetSkipAndWaitNextFrame(USB_TRAN_TYPE_CONTROL);

        do {
            ptdAddTd = __hcdAllocTd(&edInfo);                           /*  申请 TD                     */
            if (ptdAddTd == NULL) {
                break;
            }

            if (usLength - usTotalTran <= edInfo.usMaxPktSize) {
                usCurTran = (USB_INT16U)(usLength - usTotalTran);
            } else {
                usCurTran = edInfo.usMaxPktSize;
            }
            tdParam.pucCBP = __usbAllocCtrlBuffer(usCurTran);
            if (tdParam.pucCBP == NULL) {                               /*  如果现在无缓冲区可用,则,    */
                                                                        /*  ...跳出本循环               */
                __hcdFreeTd(&edInfo, ptdAddTd);
                break;
            }
            if (usBufLen == 0) {
                pucFristBuf = tdParam.pucCBP;                           /*  保存第一次的缓冲区位置,     */
            }                                                           /*  ...usbMemCopy时要用         */
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
** Descriptions:        标准请求的 OUT 阶段
** input parameters:    None
** output parameters:   None
** Returned value:      错误码
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
    ptdAddTd = __hcdAllocTd(&edInfo);                                   /*  申请 TD                     */
    __hcdAddTd(&tdParam, ptdAddTd, &edInfo);                            /*  增加 SETUP TD               */
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
