/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            rbc.c
** Latest modified Date: 2007-11-16
** Latest Version:       V1.0
** Description:          实现RBC命令
**
**--------------------------------------------------------------------------------------------------------
** Created By:           吴隆穗  Longsui Wu
** Created date:         2007-11-18
** Version:              V1.0
** Descriptions:         初始版本
**
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
**
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
**
*********************************************************************************************************/
#include "mshostconfig.h"


static USB_INT8U __GucMsCmdCnt     = 0;                                 /*  大容量类设备的命令调用次数  */
static USB_BOOL  __GbHostMsExitReq = FALSE;                             /*  是否要求退出操作标志        */

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************************
** Function name:       __rbcInit
** Descriptions:        RBC层软件初始化
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void __rbcInit (void)
{
    OS_ENTER_CRITICAL();
    __GucMsCmdCnt     = 0;                                              /*  大容量类设备的命令调用次数  */
    __GbHostMsExitReq = FALSE;                                          /*  是否要求退出操作标志        */
    OS_EXIT_CRITICAL();
}

/*********************************************************************************************************
** Function name:       __rbcDeInit
** Descriptions:        RBC层软件卸载
** input parameters:    None
** output parameters:   None
** Returned value:      TRUE: 成功, FALSE: 失败
*********************************************************************************************************/
USB_BOOL __rbcDeInit (void)
{
    USB_INT32U i;

    OS_ENTER_CRITICAL();
    __GbHostMsExitReq = TRUE;                                           /*  请求退出操作                */
    OS_EXIT_CRITICAL();

    i = 0;
    while (__GucMsCmdCnt) {                                             /*  等待退出操作完成            */
        OSTimeDly(1);
        if (i++ > OS_TICKS_PER_SEC * 10) {                              /*  最长等待时限为10s           */
            return FALSE;
        }
    }

    return TRUE;
}

/*********************************************************************************************************
  RBC 命令 (SCSI-2规范)
*********************************************************************************************************/

/*********************************************************************************************************
** Function name:       rbcInquiry
** Descriptions:        查询设备信息
** input parameters:    ucInterfaceIndex : 接口号
**                      ucLunIndex       : LUN号
** output parameters:   pucData          : 读取到的设备信息
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U rbcInquiry (USB_INT16U usInterface, USB_INT8U ucLunIndex, USB_INT8U *pucData)
{
    __BULK_ONLY_CBW cbwInq;
    USB_INT8U       ucErrCode;

    if ((pucData == NULL) || (ucLunIndex > MS_MAX_LUN)) {
        return MS_ERR_INVALID_PARAM;
    }

    OS_ENTER_CRITICAL();
    if (__GbHostMsExitReq) {                                            /*  判断上层是否已要求退出      */
        OS_EXIT_CRITICAL();
        return MS_ERR_EXIT_REQ;
    }
    __GucMsCmdCnt++;                                                    /*  进入时执行命令计数器加1     */
    OS_EXIT_CRITICAL();

    usbMemSet(&cbwInq, 0 , sizeof(__BULK_ONLY_CBW));

    /*
     *  单批量传输的特征码:0x43425355(小端模式), 即:"USBC"(大端模式)
     */
    cbwInq.dCBWSignature0 = 0x55;                                       /*  'U'                         */
    cbwInq.dCBWSignature1 = 0x53;                                       /*  'S'                         */
    cbwInq.dCBWSignature2 = 0x42;                                       /*  'B'                         */
    cbwInq.dCBWSignature3 = 0x43;                                       /*  'C'                         */

    /*
     *  传输标志位,应为随机数,
     *  但为加快程序执行速度,此处固定为0x01,0x02,0x03,0x04,而不是产生随机数
     */
    cbwInq.dCBWTag0 = 0x01;
    cbwInq.dCBWTag1 = 0x02;
    cbwInq.dCBWTag2 = 0x03;
    cbwInq.dCBWTag3 = 0x04;

    /*
     *  CBW 和 CSW 间要传输的数据量
     */
    cbwInq.dCBWDataTransferLength0 = 8;                                 /*  将要读取的数据长度          */
    cbwInq.dCBWDataTransferLength1 = 0x00;

    cbwInq.bmCBWFlags   = 0x80;                                         /*  方向: BULK IN               */
    cbwInq.bCBWLUN      = (USB_INT8U)(__msInfo.ucUsedLun[ucLunIndex] & 0x0F);
    cbwInq.bCBWCBLength = 6;

    cbwInq.CBWCB[0] = 0x12;                                             /*  命令号:0x12                 */
    cbwInq.CBWCB[1] = (USB_INT8U)((__msInfo.ucUsedLun[ucLunIndex] & 0x07) << 5);
    cbwInq.CBWCB[2] = 0;

    cbwInq.CBWCB[4] = 8;

    ucErrCode = __boSendCommand(&cbwInq, pucData);
    if (ucErrCode != MS_ERR_SUCESS) {
        ucErrCode = rbcRequestSense(ucLunIndex);
        if ((ucErrCode != MS_ERR_SUCESS) && (ucErrCode != 0x06)) {
            __boMassStorReset(usInterface);
        }
    }

    OS_ENTER_CRITICAL();
    __GucMsCmdCnt--;                                                    /*  退出时执行命令计数器减1     */
    OS_EXIT_CRITICAL();

    return ucErrCode;
}

/*********************************************************************************************************
** Function name:       rbcReadCapacity
** Descriptions:        读取设备容量
** input parameters:    usInterface      : 接口号
**                      ucLunIndex       : LUN号
** output parameters:   puiMaxLba        : 最大LBA
**                      puiBytesPerBlock : 每块字节数
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U rbcReadCapacity (USB_INT16U  usInterface,
                           USB_INT8U   ucLunIndex,
                           USB_INT32U *puiMaxLba,
                           USB_INT32U *puiBytesPerBlock)
{
    USB_INT8U       ucBufTmp[8] = {0};
    USB_INT8U       ucErrCode;
    __BULK_ONLY_CBW cbwInq;

    if (ucLunIndex > __msInfo.ucMaxLun) {
        return MS_ERR_INVALID_PARAM;
    }

    OS_ENTER_CRITICAL();
    if (__GbHostMsExitReq) {
        OS_EXIT_CRITICAL();
        return MS_ERR_EXIT_REQ;
    }
    __GucMsCmdCnt++;
    OS_EXIT_CRITICAL();

    usbMemSet(&cbwInq, 0 , sizeof(__BULK_ONLY_CBW));

    /*
     *  单批量传输的特征码:0x43425355(小端模式), 即:"USBC"(大端模式)
     */
    cbwInq.dCBWSignature0 = 0x55;                                       /*  'U'                         */
    cbwInq.dCBWSignature1 = 0x53;                                       /*  'S'                         */
    cbwInq.dCBWSignature2 = 0x42;                                       /*  'B'                         */
    cbwInq.dCBWSignature3 = 0x43;                                       /*  'C'                         */

    /*
     *  传输标志位,应为随机数,
     *  但为加快程序执行速度,此处固定为0x01,0x02,0x03,0x04,而不是产生随机数
     */
    cbwInq.dCBWTag0 = 0x01;
    cbwInq.dCBWTag1 = 0x02;
    cbwInq.dCBWTag2 = 0x03;
    cbwInq.dCBWTag3 = 0x04;

    /*
     *  CBW 和 CSW 间要传输的数据量
     */
    cbwInq.dCBWDataTransferLength0 = 8;
    cbwInq.dCBWDataTransferLength1 = 0x00;

    cbwInq.bmCBWFlags   = 0x80;                                         /*  方向: BULK IN               */
    cbwInq.bCBWLUN      = (USB_INT8U)(__msInfo.ucUsedLun[ucLunIndex] & 0x0F);
    cbwInq.bCBWCBLength = 10;

    cbwInq.CBWCB[0] = SCSI_READ_CAPACITY;
    cbwInq.CBWCB[1] = (USB_INT8U)((__msInfo.ucUsedLun[ucLunIndex] & 0x07) << 5);
    cbwInq.CBWCB[2] = 0;

    cbwInq.CBWCB[4] = 0;

    ucErrCode = __boSendCommand(&cbwInq, ucBufTmp);
    if (ucErrCode != MS_ERR_SUCESS) {
        ucErrCode = rbcRequestSense(ucLunIndex);
        if ((ucErrCode != MS_ERR_SUCESS) && (ucErrCode != 0x06)) {
            __boMassStorReset(usInterface);
        }
    }

    /**
     * 该存储介质最后的 LBA
     */
    *puiMaxLba = (ucBufTmp[0] << 24)
               + (ucBufTmp[1] << 16)
               + (ucBufTmp[2] << 8)
               +  ucBufTmp[3];

    /**
     * 每块字节数
     */
    *puiBytesPerBlock = (ucBufTmp[4] << 24)
                      + (ucBufTmp[5] << 16)
                      + (ucBufTmp[6] << 8)
                      +  ucBufTmp[7];


    OS_ENTER_CRITICAL();
    __GucMsCmdCnt--;
    OS_EXIT_CRITICAL();

    return ucErrCode;
}

/*********************************************************************************************************
** Function name:       rbcStartStopUnit
** Descriptions:        启动或停止设备(LUN)
** input parameters:    ucInterfaceIndex : 接口号
**                      ucLunIndex       : LUN号
**                      ucStatus         : 状态,启动(RBC_MAKEREADY)或停止(RBC_STOPMEDIUM) LUN
** output parameters:   None
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U rbcStartStopUnit (USB_INT16U ucInterfaceIndex,
                            USB_INT8U  ucLunIndex,
                            USB_INT8U  ucStatus)
{
    __BULK_ONLY_CBW cbwInq;
    USB_INT8U       ucErrCode;

    OS_ENTER_CRITICAL();
    if (__GbHostMsExitReq) {
        OS_EXIT_CRITICAL();
        return MS_ERR_EXIT_REQ;
    }
    __GucMsCmdCnt++;
    OS_EXIT_CRITICAL();

    usbMemSet(&cbwInq, 0 , sizeof(__BULK_ONLY_CBW));

    /*
     *  单批量传输的特征码:0x43425355(小端模式), 即:"USBC"(大端模式)
     */
    cbwInq.dCBWSignature0 = 0x55;                                       /*  'U'                         */
    cbwInq.dCBWSignature1 = 0x53;                                       /*  'S'                         */
    cbwInq.dCBWSignature2 = 0x42;                                       /*  'B'                         */
    cbwInq.dCBWSignature3 = 0x43;                                       /*  'C'                         */

    /*
     *  传输标志位,应为随机数,
     *  但为加快程序执行速度,此处固定为0x01,0x02,0x03,0x04,而不是产生随机数
     */
    cbwInq.dCBWTag0 = 0x01;
    cbwInq.dCBWTag1 = 0x02;
    cbwInq.dCBWTag2 = 0x03;
    cbwInq.dCBWTag3 = 0x04;

    /*
     *  CBW 和 CSW 间要传输的数据量
     */
    cbwInq.dCBWDataTransferLength0 = 0;
    cbwInq.dCBWDataTransferLength1 = 0;
    cbwInq.dCBWDataTransferLength2 = 0;
    cbwInq.dCBWDataTransferLength3 = 0;

    cbwInq.bmCBWFlags   = 0x80;                                         /*  方向: BULK IN               */
    cbwInq.bCBWLUN      = (USB_INT8U)(ucLunIndex & 0x0F);
    cbwInq.bCBWCBLength = 6;

    cbwInq.CBWCB[0] = SCSI_START_STOP;                                  /*  操作码                      */
    cbwInq.CBWCB[4] = (USB_INT8U)(ucStatus & 0x03);

    ucErrCode = __boSendCommand (&cbwInq, NULL);
    if (ucErrCode == MS_ERR_CSW) {
        __boMassStorReset(ucInterfaceIndex);
        OS_ENTER_CRITICAL();
        __GucMsCmdCnt--;
        OS_EXIT_CRITICAL();
        return SCSI_ERR_CMDEXE;
    }

    OS_ENTER_CRITICAL();
    __GucMsCmdCnt--;
    OS_EXIT_CRITICAL();

    return ucErrCode;
}

/*********************************************************************************************************
** Function name:       rbcRead10
** Descriptions:        读取数据
** input parameters:    usInterface : 接口号
**                      ucLunIndex  : LUN号
**                      uiLba       : 块地址
**                      uiTranLen   : 要读取的块数
** output parameters:   pucData     : 接收数据缓冲区
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U rbcRead10 (USB_INT16U usInterface,
                     USB_INT8U  ucLunIndex,
                     USB_INT32U uiLba,                                  /*  LBA,即逻辑块地址            */
                     USB_INT32U uiTranLen,                              /*  要读取的数据长度(块数)      */
                     USB_INT8U *pucData)                                /*  接收数据缓冲区              */
{
    __BULK_ONLY_CBW  cbwInq;
    USB_INT8U        ucErrCode;
    
    USB_INT32U       uiCurLen;
    USB_INT32U       uiTotalLen;
    
    
    if ((pucData == NULL) || (uiTranLen == 0) || (ucLunIndex >= __msInfo.ucMaxLun) ||
        (uiLba + uiTranLen > __msInfo.msLunInfo[ucLunIndex].uiMaxLba)) {
        return MS_ERR_INVALID_PARAM;
    }

    OS_ENTER_CRITICAL();
    if (__GbHostMsExitReq) {
        OS_EXIT_CRITICAL();
        return MS_ERR_EXIT_REQ;
    }
    __GucMsCmdCnt++;
    OS_EXIT_CRITICAL();

    usbMemSet(&cbwInq, 0 , sizeof(__BULK_ONLY_CBW));

    /*
     *  单批量传输的特征码:0x43425355(小端模式), 即:"USBC"(大端模式)
     */
    cbwInq.dCBWSignature0 = 0x55;                                       /*  'U'                         */
    cbwInq.dCBWSignature1 = 0x53;                                       /*  'S'                         */
    cbwInq.dCBWSignature2 = 0x42;                                       /*  'B'                         */
    cbwInq.dCBWSignature3 = 0x43;                                       /*  'C'                         */

    /*
     *  传输标志位,应为随机数,
     *  但为加快程序执行速度,此处固定为0x01,0x02,0x03,0x04,而不是产生随机数
     */
    cbwInq.dCBWTag0 = 0x01;
    cbwInq.dCBWTag1 = 0x02;
    cbwInq.dCBWTag2 = 0x03;
    cbwInq.dCBWTag3 = 0x04;

    cbwInq.bmCBWFlags   = 0x80;                                         /*  方向: BULK IN               */
    cbwInq.bCBWLUN      = (USB_INT8U)(__msInfo.ucUsedLun[ucLunIndex] & 0x0F);
    cbwInq.bCBWCBLength = 10;

    cbwInq.CBWCB[0] = SCSI_READ10;                                      /*  操作码                      */
    cbwInq.CBWCB[1] = (USB_INT8U)((__msInfo.ucUsedLun[ucLunIndex] & 0x07) << 5);
    
    uiTotalLen = 0;
    do {
        if (__msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock == 512) {
            if (((uiTranLen - uiTotalLen) * 512) > 512 * 1024) {
                uiCurLen = (512 * 1024) / 512;
            } else {
                uiCurLen = uiTranLen - uiTotalLen;
            }
        } else {
            if ((uiTranLen - uiTotalLen) * __msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock 
                 > 512 * 1024) {
                uiCurLen = (512 * 1024) / __msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock;
            } else {
                uiCurLen = uiTranLen - uiTotalLen;
            }
        }            
        
        /*  
         *  CBW 和 CSW 间要传输的数据量
         */
        if (__msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock == 512) {
            cbwInq.dCBWDataTransferLength0 = __MSLSB(uiCurLen * 512, 0);
            cbwInq.dCBWDataTransferLength1 = __MSLSB(uiCurLen * 512, 1);
            cbwInq.dCBWDataTransferLength2 = __MSLSB(uiCurLen * 512, 2);
            cbwInq.dCBWDataTransferLength3 = __MSLSB(uiCurLen * 512, 3);
        } else {
            cbwInq.dCBWDataTransferLength0 = __MSLSB(uiCurLen * \
                                                     __msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock, 0);
            cbwInq.dCBWDataTransferLength1 = __MSLSB(uiCurLen * \
                                                     __msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock, 1);
            cbwInq.dCBWDataTransferLength2 = __MSLSB(uiCurLen * \
                                                     __msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock, 2);
            cbwInq.dCBWDataTransferLength3 = __MSLSB(uiCurLen * \
                                                     __msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock, 3);
        }   
        
        cbwInq.CBWCB[2] = __MSLSB(uiLba, 3);
        cbwInq.CBWCB[3] = __MSLSB(uiLba, 2);
        cbwInq.CBWCB[4] = __MSLSB(uiLba, 1);
        cbwInq.CBWCB[5] = __MSLSB(uiLba, 0);
        
        cbwInq.CBWCB[7] = __MSLSB(uiCurLen, 1);
        cbwInq.CBWCB[8] = __MSLSB(uiCurLen, 0);
        
        ucErrCode = __boSendCommand(&cbwInq, pucData);
        if (ucErrCode != MS_ERR_SUCESS) {
            if (ucErrCode == MS_ERR_CSW) {
                ucErrCode = rbcRequestSense (ucLunIndex);
                if ((ucErrCode != MS_ERR_SUCESS) && (ucErrCode != 0x06)) {
                    __boMassStorReset(usInterface);
                    break;
                }
            } else {
                __boMassStorReset(usInterface);
                break;
            }
        }
        uiTotalLen = (USB_INT32U)(uiTotalLen + uiCurLen);
        uiLba      = (USB_INT32U)(uiLba + uiCurLen);
        pucData    = pucData + uiCurLen * __msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock;
    } while (uiTotalLen < uiTranLen);
    
    OS_ENTER_CRITICAL();
    __GucMsCmdCnt--;
    OS_EXIT_CRITICAL();

    return ucErrCode;
}

/*********************************************************************************************************
** Function name:       rbcWrite10
** Descriptions:        读取数据
** input parameters:    usInterface : 接口号
**                      ucLunIndex  : LUN号
**                      uiLba       : 块地址
**                      uiTranLen   : 要写的块数
** output parameters:   pucData     : 写数据缓冲区
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U rbcWrite10 (USB_INT16U usInterface,
                      USB_INT8U  ucLunIndex,
                      USB_INT32U uiLba,                                 /*  LBA,即逻辑块地址            */
                      USB_INT32U uiTranLen,                             /*  要读取的数据长度            */
                      USB_INT8U *pucData)                               /*  接收数据缓冲区              */
{
    USB_INT8U       ucErrCode;
    __BULK_ONLY_CBW cbwInq;
    
    USB_INT32U      uiCurLen;
    USB_INT32U      uiTotalLen;

    if ((pucData == NULL) || (uiTranLen == 0) || (ucLunIndex >= __msInfo.ucMaxLun) ||
        (uiLba + uiTranLen > __msInfo.msLunInfo[ucLunIndex].uiMaxLba)) {
        return MS_ERR_INVALID_PARAM;
    }

    OS_ENTER_CRITICAL();
    if (__GbHostMsExitReq) {
        OS_EXIT_CRITICAL();
        return MS_ERR_EXIT_REQ;
    }
    __GucMsCmdCnt++;
    OS_EXIT_CRITICAL();

    usbMemSet(&cbwInq, 0 , sizeof(__BULK_ONLY_CBW));

    /*
     *  单批量传输的特征码:0x43425355(小端模式), 即:"USBC"(大端模式)
     */
    cbwInq.dCBWSignature0 = 0x55;                                       /*  'U'                         */
    cbwInq.dCBWSignature1 = 0x53;                                       /*  'S'                         */
    cbwInq.dCBWSignature2 = 0x42;                                       /*  'B'                         */
    cbwInq.dCBWSignature3 = 0x43;                                       /*  'C'                         */

    /*
     *  传输标志位,应为随机数,
     *  但为加快程序执行速度,此处固定为0x01,0x02,0x03,0x04,而不是产生随机数
     */
    cbwInq.dCBWTag0 = 0x01;
    cbwInq.dCBWTag1 = 0x02;
    cbwInq.dCBWTag2 = 0x03;
    cbwInq.dCBWTag3 = 0x04;

    cbwInq.bmCBWFlags   = 0x00;                                         /*  方向: BULK IN               */
    cbwInq.bCBWLUN      = (USB_INT8U)(__msInfo.ucUsedLun[ucLunIndex] & 0x0F);
    cbwInq.bCBWCBLength = 10;

    cbwInq.CBWCB[0] = 0x2A;                                             /*  操作码                      */
    cbwInq.CBWCB[1] = (USB_INT8U)((__msInfo.ucUsedLun[ucLunIndex] & 0x07) << 5);
    
    uiTotalLen = 0;
    do {
        if (__msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock == 512) {
            if (((uiTranLen - uiTotalLen) * 512) > 512 * 1024) {
                uiCurLen = (512 * 1024) / 512;
            } else {
                uiCurLen = uiTranLen - uiTotalLen;
            }
        } else {
            if ((uiTranLen - uiTotalLen) * __msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock 
                > 512 * 1024) {
                uiCurLen = (512 * 1024) / __msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock;
            } else {
                uiCurLen = uiTranLen - uiTotalLen;
            }
        }            
        
        /*  
         *  CBW 和 CSW 间要传输的数据量
         */
        if (__msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock == 512) {
            cbwInq.dCBWDataTransferLength0 = __MSLSB(uiCurLen * 512, 0);
            cbwInq.dCBWDataTransferLength1 = __MSLSB(uiCurLen * 512, 1);
            cbwInq.dCBWDataTransferLength2 = __MSLSB(uiCurLen * 512, 2);
            cbwInq.dCBWDataTransferLength3 = __MSLSB(uiCurLen * 512, 3);
        } else {
            cbwInq.dCBWDataTransferLength0 = __MSLSB(uiCurLen * \
                                                     __msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock, 0);
            cbwInq.dCBWDataTransferLength1 = __MSLSB(uiCurLen * \
                                                     __msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock, 1);
            cbwInq.dCBWDataTransferLength2 = __MSLSB(uiCurLen * \
                                                     __msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock, 2);
            cbwInq.dCBWDataTransferLength3 = __MSLSB(uiCurLen * \
                                                     __msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock, 3);
        }   
        
        cbwInq.CBWCB[2] = __MSLSB(uiLba, 3);
        cbwInq.CBWCB[3] = __MSLSB(uiLba, 2);
        cbwInq.CBWCB[4] = __MSLSB(uiLba, 1);
        cbwInq.CBWCB[5] = __MSLSB(uiLba, 0);
        
        cbwInq.CBWCB[7] = __MSLSB(uiCurLen, 1);
        cbwInq.CBWCB[8] = __MSLSB(uiCurLen, 0);
        
        ucErrCode = __boSendCommand(&cbwInq, pucData);
        if (ucErrCode != MS_ERR_SUCESS) {
            if (ucErrCode == MS_ERR_CSW) {
                ucErrCode = rbcRequestSense (ucLunIndex);
                if ((ucErrCode != MS_ERR_SUCESS) && (ucErrCode != 0x06)) {
                    __boMassStorReset(usInterface);
                    break;
                }
            } else {
                __boMassStorReset(usInterface);
                break;
            } 
        }
        uiTotalLen = (USB_INT32U)(uiTotalLen + uiCurLen);
        uiLba      = (USB_INT32U)(uiLba + uiCurLen);
        pucData    = pucData + uiCurLen * __msInfo.msLunInfo[ucLunIndex].uiBytesPerBlock;
    } while (uiTotalLen < uiTranLen);
    
    OS_ENTER_CRITICAL();
    __GucMsCmdCnt--;
    OS_EXIT_CRITICAL();

    return ucErrCode;
}

/*********************************************************************************************************
** Function name:       rbcRequestSense
** Descriptions:        读取判别数据
** input parameters:    ucLunIndex : LUN号
** output parameters:   None
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U rbcRequestSense (USB_INT8U ucLunIndex)
{
    __BULK_ONLY_CBW cbwInq;
    USB_INT8U       ucErrCode;
    USB_INT8U       ucBufTmp[18] = {0};
    USB_INT8U       ucTryCount;

    OS_ENTER_CRITICAL();
    if (__GbHostMsExitReq) {
        OS_EXIT_CRITICAL();
        return MS_ERR_EXIT_REQ;
    }
    __GucMsCmdCnt++;
    OS_EXIT_CRITICAL();

    usbMemSet(&cbwInq, 0 , sizeof(__BULK_ONLY_CBW));

    /*
     *  单批量传输的特征码:0x43425355(小端模式), 即:"USBC"(大端模式)
     */
    cbwInq.dCBWSignature0 = 0x55;                                       /*  'U'                         */
    cbwInq.dCBWSignature1 = 0x53;                                       /*  'S'                         */
    cbwInq.dCBWSignature2 = 0x42;                                       /*  'B'                         */
    cbwInq.dCBWSignature3 = 0x43;                                       /*  'C'                         */

    /*
     *  传输标志位,应为随机数,
     *  但为加快程序执行速度,此处固定为0x01,0x02,0x03,0x04,而不是产生随机数
     */
    cbwInq.dCBWTag0 = 0x01;
    cbwInq.dCBWTag1 = 0x02;
    cbwInq.dCBWTag2 = 0x03;
    cbwInq.dCBWTag3 = 0x04;

    /*
     *  CBW 和 CSW 间要传输的数据量
     */
    cbwInq.dCBWDataTransferLength0 = 18;
    cbwInq.dCBWDataTransferLength1 = 0;
    cbwInq.dCBWDataTransferLength2 = 0;
    cbwInq.dCBWDataTransferLength3 = 0;

    cbwInq.bmCBWFlags   = 0x80;                                         /*  方向: BULK IN               */
    cbwInq.bCBWLUN      = (USB_INT8U)(__msInfo.ucUsedLun[ucLunIndex] & 0x0F);
    cbwInq.bCBWCBLength = 6;

    cbwInq.CBWCB[0] = 0x03;                                             /*  操作码                      */
    cbwInq.CBWCB[4] = 18;

    ucTryCount = 0;
    do {
        ucErrCode = __boSendCommand(&cbwInq, ucBufTmp);
        if ((ucErrCode == MS_ERR_CSW) || (ucErrCode == USB_ERR_DEVICE_NOT_READY)) {
            OS_ENTER_CRITICAL();
            __GucMsCmdCnt--;
            OS_EXIT_CRITICAL();
            return SCSI_ERR_CMDEXE;
        }

        ucErrCode = (USB_INT8U)(ucBufTmp[2] & 0x0F);
    } while ((ucErrCode == 0x0B) && (++ucTryCount <= 2));


    OS_ENTER_CRITICAL();
    __GucMsCmdCnt--;
    OS_EXIT_CRITICAL();

    return ucErrCode;
}

/*********************************************************************************************************
** Function name:       rbcTestUintReady
** Descriptions:        测试设备是否准备好
** input parameters:    usInterface : 接口号
**                      ucLunIndex  : LUN号
** output parameters:   None
** Returned value:      错误代码,若返回MS_ERR_SUCESS, 说明执行成功, 否则执行失败
*********************************************************************************************************/
USB_INT8U rbcTestUintReady (USB_INT16U usInterface, USB_INT8U ucLunIndex)
{
    __BULK_ONLY_CBW cbwInq;
    USB_INT8U       ucErrCode;

    OS_ENTER_CRITICAL();
    if (__GbHostMsExitReq) {                                            /*  检查是否被上层软件要求退出  */
        OS_EXIT_CRITICAL();
        return MS_ERR_EXIT_REQ;
    }
    __GucMsCmdCnt++;                                                    /*  进入时执行命令计数器加1     */
    OS_EXIT_CRITICAL();

    usbMemSet(&cbwInq, 0 , sizeof(__BULK_ONLY_CBW));

    /*
     *  单批量传输的特征码:0x43425355(小端模式), 即:"USBC"(大端模式)
     */
    cbwInq.dCBWSignature0 = 0x55;                                       /*  'U'                         */
    cbwInq.dCBWSignature1 = 0x53;                                       /*  'S'                         */
    cbwInq.dCBWSignature2 = 0x42;                                       /*  'B'                         */
    cbwInq.dCBWSignature3 = 0x43;                                       /*  'C'                         */

    /*
     *  传输标志位,应为随机数,
     *  但为加快程序执行速度,此处固定为0x01,0x02,0x03,0x04,而不是产生随机数
     */
    cbwInq.dCBWTag0 = 0x01;
    cbwInq.dCBWTag1 = 0x02;
    cbwInq.dCBWTag2 = 0x03;
    cbwInq.dCBWTag3 = 0x04;

    /*
     *  CBW 和 CSW 间要传输的数据量
     */
    cbwInq.dCBWDataTransferLength0 = 0;
    cbwInq.dCBWDataTransferLength1 = 0;
    cbwInq.dCBWDataTransferLength2 = 0;
    cbwInq.dCBWDataTransferLength3 = 0;

    cbwInq.bmCBWFlags   = 0x80;                                         /*  方向: BULK IN               */
    cbwInq.bCBWLUN      = (USB_INT8U)(ucLunIndex & 0x0F);               /*  LUN号                       */
    cbwInq.bCBWCBLength = 6;                                            /*  命令块有效长度:6            */

    cbwInq.CBWCB[0] = SCSI_TEST_UNIT_READY;                             /*  操作码:测试设备是否准备好   */

    ucErrCode = __boSendCommand (&cbwInq, NULL);                        /*  发送命令                    */
    if ((ucErrCode == MS_ERR_CSW) || (ucErrCode == USB_ERR_DEVICE_NOT_READY)) {
        __boMassStorReset(usInterface);
        OS_ENTER_CRITICAL();
        __GucMsCmdCnt--;
        OS_EXIT_CRITICAL();
        return SCSI_ERR_CMDEXE;
    } else if (ucErrCode == CSW_FLAG_FAIL) {
        rbcRequestSense (ucLunIndex);
    }

    if (ucErrCode == SCSI_CHECK_CONDITION) {
        ;
    }

    OS_ENTER_CRITICAL();
    __GucMsCmdCnt--;                                                    /*  退出时执行命令计数器减1     */
    OS_EXIT_CRITICAL();

    return ucErrCode;
}

#ifdef __cplusplus
 }
#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
