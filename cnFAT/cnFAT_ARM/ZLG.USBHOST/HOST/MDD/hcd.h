/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            hcd.h
** Latest modified Date: 2009.01.14
** Latest Version:       V1.1
** Description:          ������������, HCD����� hcd.c ͷ�ļ�
**--------------------------------------------------------------------------------------------------------
** Created By:           ��¡��  Longsui Wu
** Created date:         2007-11-06
** Version:              V1.0
** Descriptions:         ��ʼ�汾
**--------------------------------------------------------------------------------------------------------
** Modified by:          liuweiyun
** Modified date:        2009.01.14
** Version:              V1.1
** Description:          �޸ġ�����ע�ͣ�����HCCA�ṹ���������Ͷ����
**--------------------------------------------------------------------------------------------------------
** Modified by:          Liu Weiyun
** Modified date:        2009.03.13
** Version:              V1.2
** Description:          USB RAM�Զ������ڴ棬�ϳ���ǰ��ʹ�ù̶���ַ�ڴ�
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
*********************************************************************************************************/
#ifndef __HCD_H
#define __HCD_H

#include "..\USBHostConfig.h"

#ifndef   USB_RAM_SIZE
#define   USB_RAM_SIZE              (4 * 1024)                          /* USB RAM size: 8KB default    */
#endif

#if       USB_RAM_SIZE < (4 * 1024)
#error    "USB_RAM_SIZE must be >= (4 * 1024)!!"
#endif

#if       USB_RAM_SIZE % 4 != 0
#error    "(USB_RAM_SIZE % 4) must be 0!!"
#endif

#ifdef    USB_RAM_ALLOCATE
#define   USB_EXT
#else
#define   USB_EXT   extern
#endif

//#pragma data_alignment=256
__align(256)
USB_EXT USB_INT8U __GusbRAM[USB_RAM_SIZE];                 /* Allocate USB RAM in user RAM */

#define __OHCI_HCCA_BASE_ADDR       ((USB_INT32U)__GusbRAM)
#define __OHCI_ED_BASE_ADDR         (__OHCI_HCCA_BASE_ADDR + 256)       /* HCCA size: 256 bytes         */
#define __OHCI_TD_BASE_ADDR         (__OHCI_ED_BASE_ADDR + sizeof(__OHCI_ED))

#define __OHCI_DATA_BASE_ADDR       (__OHCI_TD_BASE_ADDR + sizeof(__OHCI_TD))

#define __OHCI_DATA_CTRL_BUFFER_LEN 0x200
#define __OHCI_DATA_BASE_ADDR_CTRL  __OHCI_DATA_BASE_ADDR               /*  ���ƶ˵����ݻ������׵�ַ    */
#define __OHCI_DATA_BASE_ADDR_GEN   (__OHCI_DATA_BASE_ADDR_CTRL + __OHCI_DATA_CTRL_BUFFER_LEN)
                                                                        /*  BULK, INTR �ȶ˵����ݻ����� */
                                                                        /*  ...�׵�ַ                   */
#define __OHCI_USB_RAM_END_ADDR     ((USB_INT32U)__GusbRAM + USB_RAM_SIZE - 1)

#define __HC_TD_TYPE_GEN            0x01
#define __HC_TD_TYPE_ISO            0x02

#define __HC_CTL_MAXPACKETSIZE      8
#define __HC_BULK_MAXPACKETSIZE     64
#define __HC_ISO_MAXPACKETSIZE      16

#define __OHCI_TD_R                 1

#define __OHCI_TD_TYPE_SETUP        0x00
#define __OHCI_TD_TYPE_OUT          0x01
#define __OHCI_TD_TYPE_IN           0x02

#define __OHCI_TD_T_DATA0           0x02
#define __OHCI_TD_T_DATA1           0x03

#define __HC_SPEED_LOW              1
#define __HC_SPEED_FULL             0

#define __HC_CTRL_LIST_EN           (0x01 << 4)

#define __HCD_GEN_PARAM_MASK        0x01
#define __HC_GEN_PARAM_MASK         0x02

#define __HCD_TD_TYPE_ED_TYPE_MASK  (USB_INT8U)0x03


#define __USB_INT_MIE               (1U << 31)
#define __USB_INT_SO                (1 << 0)
#define __USB_INT_WDH               (1 << 1)
#define __USB_INT_SF                (1 << 2)
#define __USB_INT_RD                (1 << 3)
#define __USB_INT_UE                (1 << 4)
#define __USB_INT_FNO               (1 << 5)
#define __USB_INT_RHSC              (1 << 6)
#define __USB_INT_OC                (1 << 30)


/*********************************************************************************************************
  Host Controller Endpoint Descriptor, refer to Section 4.2 of OpenHCI Spec. Rev1.0, Endpoint Descriptor
*********************************************************************************************************/
typedef struct __tagHCCA {
    USB_INT32U uiHccaInterrruptTable[32];               /* �ж�ED�׵�ַ���128�ֽ�(32��)              */
    USB_INT16U usHccaFrameNumber;                       /* ��ǰ֡��ţ�2�ֽ�(����)                      */
    USB_INT16U usHccaPad1;                              /* ������2�ֽ�(����)������ʹHccaDoneHead�ֶ���  */
    USB_INT32U uiHccaDoneHead;                          /* ������������ͷָ�룬4�ֽ�                  */
    USB_INT32U uiReserved[30];                          /* ��������120�ֽڣ�30��                        */
} __HCCA, *__PHCCA;


/*********************************************************************************************************
  Host Controller Transfer Descriptor, refer to Section 4.3 of OpenHCI Spec. Rev1.0, Transfer Descriptors
*********************************************************************************************************/
typedef struct __tagHC_GEN_TRANSFER_DESCRIPTOR {                        /*  ͨ�� TD,for Contrl,bulk,Intr*/
    USB_INT32U                              uiControl;                  /*  ������                      */
    USB_INT8U                              *pucCBP;                     /*  Current Buffer Pointer      */
    struct __tagHC_GEN_TRANSFER_DESCRIPTOR *ptdNextTD;                  /*  phys ptr to                 */
                                                                        /*  ...HC_TRANSFER_DESCRIPTOR   */
    USB_INT8U                              *pucBufEnd;                  /*  Buffer end                  */
} __HC_GEN_TRANSFER_DESCRIPTOR, *__PHC_GEN_TRANSFER_DESCRIPTOR;

/*********************************************************************************************************
  HC ������ͬ������Ĵ��������� TD, ��ϸ�μ�OHCI Sepc Rev 1.0 �е� 4.3.2
*********************************************************************************************************/
typedef struct __tagHC_ISO_TRANSFER_DESCRIPTOR {                        /*  ����ͬ�������TD(ISO TD)    */
    USB_INT32U                              uiControl;                  /*  ������                      */
    USB_INT8U                              *pucBF0;                     /*  Current Buffer Pointer      */
    struct __tagHC_ISO_TRANSFER_DESCRIPTOR *ptdNextTD;                  /*  phys ptr to                 */
                                                                        /*  ...ISO_TD                   */
    USB_INT8U                              *pucBufEnd;                  /*  Buffer end                  */
    USB_INT16U                              usOP0;                      /*  Offsetn/PSWn                */
    USB_INT16U                              usOP1;
    USB_INT16U                              usOP2;
    USB_INT16U                              usOP3;
    USB_INT16U                              usOP4;
    USB_INT16U                              usOP5;
    USB_INT16U                              usOP6;
    USB_INT16U                              usOP7;
} __HC_ISO_TRANSFER_DESCRIPTOR, *__PHC_ISO_TRANSFER_DESCRIPTOR;

/*********************************************************************************************************
  HC �ж˵������� ED, ��ϸ�μ�OHCI Sepc Rev 1.0 �е� 4.2
*********************************************************************************************************/
typedef struct __tagHC_ENDPOINT_DESCRIPTOR {
    USB_INT32U                          uiControl;                      /*  dword 0:����λ              */
    __PHC_GEN_TRANSFER_DESCRIPTOR       ptdTailP;                       /*  TRANSFER_DESCRIPTOR�����ַ */
    __PHC_GEN_TRANSFER_DESCRIPTOR       ptdHeadP;                       /*  flags + phys ptr to TD      */
    struct __tagHC_ENDPOINT_DESCRIPTOR *pedNextED;                      /*  phys ptr to TD              */
} __HC_ENDPOINT_DESCRIPTOR, *__PHC_ENDPOINT_DESCRIPTOR;

typedef struct __tagHC_ISO_ENDPOINT_DESCRIPTOR {
    USB_INT32U                              uiControl;                  /*  dword 0:����λ              */
    __PHC_ISO_TRANSFER_DESCRIPTOR           ptdTailP;                   /*  TRANSFER_DESCRIPTOR�����ַ */
    __PHC_ISO_TRANSFER_DESCRIPTOR           ptdHeadP;                   /*  flags + phys ptr to TD      */
    struct __tagHC_ISO_ENDPOINT_DESCRIPTOR *pedNextED;                  /*  phys ptr to TD              */
} __HC_ISO_ENDPOINT_DESCRIPTOR, *__PHC_ISO_ENDPOINT_DESCRIPTOR;

/*********************************************************************************************************
  HC �ж˵� GEN TD �Ĳ����ṹ��, ��ϸ�μ�OHCI Sepc Rev 1.0 �е� 4.3.1
*********************************************************************************************************/
typedef struct __tagHC_GEN_TD_PARAM {
    USB_INT8U  ucBufferRound;                                           /*  �μ�OHCI Sepc Rev 1.0 �е�  */
                                                                        /*  ... 4.3.1.2                 */
    USB_INT8U  ucDirect_PID;                                            /*  ����������� PID            */
    USB_INT8U  ucDelayIntr;                                             /*  DelayInterrupt              */
    USB_INT8U  ucDataToggle;                                            /*  ͬ������                    */
    USB_INT8U *pucCBP;
    USB_INT16U usBufLen;
    USB_INT16U usReserv;
} __HC_GEN_TD_PARAM, *__PHC_GEN_TD_PARAM;

/*********************************************************************************************************
  �˵������� ED �ļ���
*********************************************************************************************************/
typedef struct __tagOHCI_ED {

    __HC_ENDPOINT_DESCRIPTOR     edsControl;

    __HC_ENDPOINT_DESCRIPTOR     edsBulkOut;
    __HC_ENDPOINT_DESCRIPTOR     edsBulkIn;

    __HC_ENDPOINT_DESCRIPTOR     edsIntrOut;
    __HC_ENDPOINT_DESCRIPTOR     edsIntrIn;

    __HC_ISO_ENDPOINT_DESCRIPTOR edsIso;

} __OHCI_ED, *__POHCI_ED;

/*********************************************************************************************************
 TD����β������������ TD �ļ���
*********************************************************************************************************/
typedef struct __tagOHCI_TD_TAIL {

    __HC_ISO_TRANSFER_DESCRIPTOR tdsIsoTail;
    __HC_GEN_TRANSFER_DESCRIPTOR tdsContrlTail;
    __HC_GEN_TRANSFER_DESCRIPTOR tdsBulkOutTail;
    __HC_GEN_TRANSFER_DESCRIPTOR tdsBulkInTail;
    __HC_GEN_TRANSFER_DESCRIPTOR tdsIntrOutTail;
    __HC_GEN_TRANSFER_DESCRIPTOR tdsIntrInTail;

}__OHCI_TD_TAIL, *__POHCI_TD_TAIL;

/*********************************************************************************************************
  HCD �ж˵� ED �Ĳ����ṹ��, ��ϸ�μ�OHCI Sepc Rev 1.0 �е� 4.2.2
*********************************************************************************************************/
typedef struct __tagHCD_ED_PARAM {                                      /*  ͨ�� TD,for Contrl �� bulk  */
    USB_INT8U                     ucMethod;                             /*  �����ֵΪ0,�����ED�ĺ���  */
                                                                        /*  __hcdAddEd()��������Щ����  */
    USB_INT8U                     ucReserved[3];                        /*  ����.Ҳ��Ϊ�˱���pad����    */
    USB_INT8U                     ucFunAddr;                            /*  �豸(�ӻ�)��ַ              */
    USB_INT8U                     ucEpNum;                              /*  �˵��                      */
    USB_INT8U                     ucDirection;                          /*  ����������,��OHCI Sepc 4.2.2*/
    USB_INT8U                     ucSpeed;                              /*  �ٶ�: ȫ��(0) �� ����(1)    */
    USB_INT8U                     ucSkip;                               /*  ��(1)��(0)�����˶˵�        */
    USB_INT8U                     ucFormat;                             /*  �ö˵��µ� TD ��ʽ,GEN TD(0)*/
                                                                        /*  ...�� ISO TD(1)             */
    USB_INT16U                    usMaxPktSize;                         /*  �ö˵����󻺳�����С      */
    __PHC_GEN_TRANSFER_DESCRIPTOR ptdTailP;                             /*  TRANSFER_DESCRIPTOR�����ַ */
    __PHC_GEN_TRANSFER_DESCRIPTOR ptdHeadP;                             /*  flags + phys ptr to TD      */
    __PHC_ENDPOINT_DESCRIPTOR     pedNextED;                            /*  phys ptr to TD              */
} __HCD_ED_PARAM, *__PHCD_ED_PARAM;

/*********************************************************************************************************
  HCD �ж˵� GEN TD �Ĳ����ṹ��, ��ϸ�μ�OHCI Sepc Rev 1.0 �е� 4.3.1
*********************************************************************************************************/
typedef struct __tagHCD_GEN_TD_PARAM {
    USB_INT8U                     ucType;
    USB_INT8U                     ucTDStatus;
    USB_INT16U                    usBufLen;                             /*  ���ݻ�����ĩ��ַ            */
    USB_INT8U                    *pucBuf;                               /*  ���ݻ������׵�ַ            */
    USB_INT16U                    usThisIndex;
    USB_INT16U                    usTotalIndex;

    __HC_GEN_TD_PARAM             tdHcParam;
    __PHC_GEN_TRANSFER_DESCRIPTOR ptdNextTD;                            /*  ָ����һ�� TD               */
} __HCD_GEN_TD_PARAM, *__PHCD_GEN_TD_PARAM;

/*********************************************************************************************************
  HCD �� TD
*********************************************************************************************************/
typedef struct __tagHCD_GEN_TD {
    USB_INT8U                     ucType;
    USB_INT8U                     ucTDStatus;
    USB_INT16U                    usBufLen;                             /*  ���ݻ�����ĩ��ַ            */
    USB_INT8U                    *pucBuf;                               /*  ���ݻ������׵�ַ            */
    USB_INT16U                    usThisIndex;
    USB_INT16U                    usTotalIndex;
    __PHC_GEN_TRANSFER_DESCRIPTOR ptdNextTD;

    __HC_GEN_TRANSFER_DESCRIPTOR  tdTD;
} __HCD_GEN_TD, *__PHCD_GEN_TD;

/*********************************************************************************************************
  ����������TD�ļ���
*********************************************************************************************************/
typedef struct __tagOHCI_TD {
    __HC_GEN_TRANSFER_DESCRIPTOR tdCtrlTd[USB_MAX_CTRL_TD];             /*  ����������TD, Ҫ��16�ֽڶ���*/

    __HC_GEN_TRANSFER_DESCRIPTOR tdBulkOutTd[USB_MAX_BULKOUT_TD];       /*  ����������TD, Ҫ��16�ֽڶ���*/
    __HC_GEN_TRANSFER_DESCRIPTOR tdBulkInTd[USB_MAX_BULKIN_TD];         /*  ����������TD, Ҫ��16�ֽڶ���*/

    __HC_GEN_TRANSFER_DESCRIPTOR tdIntrOutTd[USB_MAX_INTROUT_TD];       /*  ����������TD, Ҫ��16�ֽڶ���*/
    __HC_GEN_TRANSFER_DESCRIPTOR tdIntrInTd[USB_MAX_INTRIN_TD];         /*  ����������TD, Ҫ��16�ֽڶ���*/
} __OHCI_TD, *__POHCI_TD;

/*********************************************************************************************************
  TD�Ƿ�����ʹ�õ�״̬��־
*********************************************************************************************************/
typedef struct __tagOHCI_TD_STAT {
    USB_INT32U uiCtrlTdStatus[(USB_MAX_CTRL_TD + 31) / 32];

    USB_INT32U uiBulkOutTdStatus[(USB_MAX_BULKOUT_TD + 31) / 32];
    USB_INT32U uiBulkInTdStatus[(USB_MAX_BULKIN_TD + 31) / 32];

    USB_INT32U uiIntrOutTdStatus[(USB_MAX_INTROUT_TD + 31) / 32];
    USB_INT32U uiIntrInTdStatus[(USB_MAX_INTRIN_TD + 31) / 32];
} __OHCI_TD_STAT, *__POHCI_TD_STAT;

/*********************************************************************************************************
  ��ǰTD���������һ��TD��ַ
*********************************************************************************************************/
typedef struct __tagTRAN_END_TD {
    __PHC_GEN_TRANSFER_DESCRIPTOR pCtrlEndTd;
    __PHC_GEN_TRANSFER_DESCRIPTOR pDataEndTd;
} __TRAN_END_TD, *__PTRAN_END_TD;

/*********************************************************************************************************
  ED����Ϣ�ṹ��,�����ö˵���������ַ,�˵���������,������TD�׵�ַ,��ʾ������TD״̬�ı����׵�ַ��
*********************************************************************************************************/
typedef struct __tagED_INFO {
    __PHC_ENDPOINT_DESCRIPTOR     pEd;                                  /*  ָ��ö˵�������ED          */
    __PHC_GEN_TRANSFER_DESCRIPTOR pTdHead;                              /*  ָ�����ED�µ�TD���׵�ַ    */
    volatile  USB_INT32          *puiTdSts;                             /*  ��־TD״̬�ı������׵�ַ    */
    USB_INT32U                    uiMaxTd;                              /*  ���������TD��              */
    USB_INT16U                    usMaxPktSize;                         /*  �˵���������              */
    USB_INT8U                     ucTranType;                           /*  ��������                    */
    USB_INT8U                     ucReserv;
} __ED_INFO, *__PED_INFO;


/*********************************************************************************************************
  ���������͵�TD�Ļ���ַ��ĩ��ַ
*********************************************************************************************************/
#define __OHCI_TD_CTRL_BASE_ADDR    (__OHCI_TD_BASE_ADDR + (USB_INT32U)(&(((__POHCI_TD)0)->tdCtrlTd[0])))
#define __OHCI_TD_CTRL_END_ADDR     (__OHCI_TD_CTRL_BASE_ADDR + \
                                     sizeof(__HC_GEN_TRANSFER_DESCRIPTOR) * USB_MAX_CTRL_TD - 1)
#define __OHCI_TD_BULKOUT_BASE_ADDR (__OHCI_TD_BASE_ADDR + \
                                     (USB_INT32U)(&(((__POHCI_TD)0)->tdBulkOutTd[0])))
#define __OHCI_TD_BULKOUT_END_ADDR  (__OHCI_TD_BULKOUT_BASE_ADDR + \
                                     sizeof(__HC_GEN_TRANSFER_DESCRIPTOR) * USB_MAX_BULKOUT_TD - 1)
#define __OHCI_TD_BULKIN_BASE_ADDR  (__OHCI_TD_BASE_ADDR + \
                                     (USB_INT32U)(&(((__POHCI_TD)0)->tdBulkInTd[0])))
#define __OHCI_TD_BULKIN_END_ADDR   (__OHCI_TD_BULKIN_BASE_ADDR + \
                                     sizeof(__HC_GEN_TRANSFER_DESCRIPTOR) * USB_MAX_BULKIN_TD - 1)
#define __OHCI_TD_INTROUT_BASE_ADDR (__OHCI_TD_BASE_ADDR + \
                                     (USB_INT32U)(&(((__POHCI_TD)0)->tdIntrOutTd[0])))
#define __OHCI_TD_INTROUT_END_ADDR  (__OHCI_TD_INTROUT_BASE_ADDR + \
                                     sizeof(__HC_GEN_TRANSFER_DESCRIPTOR) * USB_MAX_INTROUT_TD - 1)
#define __OHCI_TD_INTRIN_BASE_ADDR  (__OHCI_TD_BASE_ADDR + \
                                     (USB_INT32U)(&(((__POHCI_TD)0)->tdIntrInTd[0])))
#define __OHCI_TD_INTRIN_END_ADDR   (__OHCI_TD_INTRIN_BASE_ADDR + \
                                     sizeof(__HC_GEN_TRANSFER_DESCRIPTOR) * USB_MAX_INTRIN_TD - 1)

#define PACK_CTRL_PARAM(tdHcParam)  ((tdHcParam.ucDataToggle << 24) | (tdHcParam.ucDelayIntr << 21) | \
                                     (tdHcParam.ucDirect_PID << 19) | (tdHcParam.ucBufferRound << 18))
                                                                        /*  ���TD��uiControl�ֶβ���   */
extern __TRAN_END_TD      __GsTranEndTd;

extern __POHCI_ED         __GpohciEd;
extern __POHCI_TD_TAIL    __GpohciTdTail;
extern __OHCI_TD_STAT     __GohciTdStat;

extern volatile USB_INT8U __GucUsbSpeed;
extern volatile USB_INT8U __GucUsbDevAddr;


void __hcdEdInit (void);
__PHC_GEN_TRANSFER_DESCRIPTOR __hcdAllocTd(__PED_INFO pEdInfo);
void     __hcdFreeTd(__PED_INFO pEdInfo, __PHC_GEN_TRANSFER_DESCRIPTOR phcdFreeTd);
void     __hcdFreeAllTd (__PED_INFO pEdInfo);
USB_BOOL __hcdAddTd (__PHC_GEN_TD_PARAM pGenTdParam, __PHC_GEN_TRANSFER_DESCRIPTOR ptdAddTd, __PED_INFO pEdInfo);

USB_BOOL                  __hcdIsEdHeadEquTail (__PED_INFO pEdInfo);
void                      __hcdEdLetHeadEquTail (__PED_INFO pEdInfo);
__PHC_ENDPOINT_DESCRIPTOR __hcdGetEdPtr (USB_INT8U ucTranType);

USB_INT8U __hcdEnableEd (USB_INT8U ucEdType);
USB_BOOL  __hcEnableSchedule (USB_INT8U ucType);
USB_BOOL  __hcStartSchedule (USB_INT8U ucType);
USB_BOOL  __hcDisEnSchedule (USB_INT8U ucType);
void      __hcWaitNextFrame (void);
USB_BOOL  __hcDisEnSchedAndWaitNextFrame(USB_INT8U ucType);
USB_BOOL  __hcEnableAndStartSchedule (USB_INT8U ucType);

USB_BOOL  __usbEdSetSkipAndWaitNextFrame (USB_INT8U ucType);
USB_BOOL  __usbEdClearSkip (USB_INT8U ucType);

void      __hcdInit (void);
void      __hcdGetEdInfo (USB_INT8U ucTranType, __PED_INFO pedInfo);
USB_INT8U __hcdGetTdType (__PHC_GEN_TRANSFER_DESCRIPTOR tdTd);

USB_INT16U __usbGetFrameNumber (void);

USB_INT8U *__usbAllocCtrlBuffer (USB_INT32U uiLen);
USB_INT8U *__usbAllocGenBuffer (USB_INT32U uiLen);
void       __usbFreeCtrlBuffer (USB_INT32U uiLen);
void       __usbFreeGenBuffer (USB_INT32U uiLen);
void       __usbFreeAllCtrlBuffer (void);
void       __usbFreeAllGenBuffer (void);


#define __OHCI_FM_INTERVAL_FSMPS		0x2374                          /*  FSLargestDataPacket         */
#define __OHCI_FM_INTERVAL_FI			0x2EDF                          /*  FrameIntervalToggle         */

#define __OHCI_PWRON_TO_PWRGOOD		    10                              /*  PowerOnToPowerGoodTime      */
#define __OHCI_LS_THRESHOLD			    0x0628
#define __OHCI_PERIODIC_START			((__OHCI_FM_INTERVAL_FI - __OHCI_LS_THRESHOLD) / 10 \
                                         + __OHCI_LS_THRESHOLD)

#define __OHCI_RH_LSDA                  (1 << 9)                        /*  uiHcRhPortStatus�е��ٶ�λ  */
#define __OHCI_SPEED_LOW                (1 << 9)                        /*  �����豸Ϊ����              */
#define __OHCI_SPEED_FULL               (0 << 9)                        /*  �����豸Ϊȫ��              */

/*********************************************************************************************************
** Function name:       usbBusSusp
** Descriptions:        ��������
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void usbBusSusp (void);

/*********************************************************************************************************
** Function name:       usbBusResume
** Descriptions:        ��������
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void usbBusResume (void);

/*********************************************************************************************************
** Function name:       usbBusReset
** Descriptions:        ��λ����
** input parameters:    None
** output parameters:   None
** Returned value:      None
*********************************************************************************************************/
void usbBusReset (void);

USB_BOOL __ohciInit (void);
USB_BOOL __ohciInit2 (USB_INT8U ucPort);

USB_BOOL __ohciPortOpen (USB_INT8U ucPortNum);
USB_BOOL __ohciPortClose (USB_INT8U ucPortNum);

USB_BOOL __ohciEnableInt (USB_INT32U);
void     __ohciDisEnInt (USB_INT32U uiIntIndex);
void     __ohciDisEnIntAll (void);
USB_BOOL __ohciAffirmAttach (USB_INT8U ucRhNum);


#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
