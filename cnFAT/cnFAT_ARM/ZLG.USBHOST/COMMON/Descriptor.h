/*****************************Copyright(c)****************************************************************
**                    Guangzhou ZHIYUAN electronics Co.,LTD.
**
**                           http://www.embedtools.com
**
**------File Info-----------------------------------------------------------------------------------------
** File Name:            Descriptor.h
** Latest modified Date: 2009-02-02
** Latest Version:       V1.0
** Description:          USB规范第9章定义的一些描述符
**--------------------------------------------------------------------------------------------------------
** Created By:           Longsui Wu
** Created date:         2007-10-20
** Version:              V1.0
** Descriptions:         初始版本
**--------------------------------------------------------------------------------------------------------
** Modified by:          liuweiyun
** Modified date:        2009-02-02
** Version:              V1.0
** Description:          Add and modify some macro defines
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Description:
*********************************************************************************************************/
#ifndef __DESCRIPTOR_H
#define __DESCRIPTOR_H

#include "..\HOST\USBHostConfig.h"


/*********************************************************************************************************
  USB Descriptor structure and their member value
*********************************************************************************************************/

/*********************************************************************************************************
  USB  device descriptor structure
*********************************************************************************************************/
#if 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __USB_DEVICE_DESCRIPTOR_TYPE_
#define __USB_DEVICE_DESCRIPTOR_TYPE_
typedef struct __tagUSB_DEVICE_DESCRIPTOR {
    USB_INT8U bLength;
    USB_INT8U bDescriptorType;
    USB_INT8U bcdUSB0;
    USB_INT8U bcdUSB1;
    USB_INT8U bDeviceClass;
    USB_INT8U bDeviceSubClass;
    USB_INT8U bDeviceProtocol;
    USB_INT8U bMaxPacketSize0;
    USB_INT8U idVendor0;
    USB_INT8U idVendor1;
    USB_INT8U idProduct0;
    USB_INT8U idProduct1;
    USB_INT8U bcdDevice0;
    USB_INT8U bcdDevice1;
    USB_INT8U iManufacturer;
    USB_INT8U iProduct;
    USB_INT8U iSerialNumber;
    USB_INT8U bNumConfigurations;
} USB_DEVICE_DESCRIPTOR, *PUSB_DEVICE_DESCRIPTOR;
#endif

/*********************************************************************************************************
  USB  configuration descriptor structure
*********************************************************************************************************/
#ifndef __USB_CONFIGURATION_DESCRIPTOR_TYPE_
#define __USB_CONFIGURATION_DESCRIPTOR_TYPE_
typedef struct __tagUSB_CONFIGURATION_DESCRIPTOR {
    USB_INT8U bLength;
    USB_INT8U bDescriptorType;
    USB_INT8U wTotalLength0;
    USB_INT8U wTotalLength1;
    USB_INT8U bNumInterfaces;
    USB_INT8U bConfigurationValue;
    USB_INT8U iConfiguration;
    USB_INT8U bmAttributes;
    USB_INT8U MaxPower;
} USB_CONFIGURATION_DESCRIPTOR, *PUSB_CONFIGURATION_DESCRIPTOR;
#endif

/*********************************************************************************************************
  USB  interface descriptor structure
*********************************************************************************************************/
#ifndef __USB_INTERFACE_DESCRIPTOR_TYPE_
#define __USB_INTERFACE_DESCRIPTOR_TYPE_
typedef struct __tagUSB_INTERFACE_DESCRIPTOR {
    USB_INT8U bLength;
    USB_INT8U bDescriptorType;
    USB_INT8U bInterfaceNumber;
    USB_INT8U bAlternateSetting;
    USB_INT8U bNumEndpoints;
    USB_INT8U bInterfaceClass;
    USB_INT8U bInterfaceSubClass;
    USB_INT8U bInterfaceProtocol;
    USB_INT8U iInterface;
} USB_INTERFACE_DESCRIPTOR, *PUSB_INTERFACE_DESCRIPTOR;
#endif

/*********************************************************************************************************
  USB  endpoint descriptor structure
*********************************************************************************************************/
#ifndef __USB_ENDPOINT_DESCRIPTOR_TYPE_
#define __USB_ENDPOINT_DESCRIPTOR_TYPE_
typedef struct __tagUSB_ENDPOINT_DESCRIPTOR {
    USB_INT8U bLength;
    USB_INT8U bDescriptorType;
    USB_INT8U bEndpointAddress;
    USB_INT8U bmAttributes;
    USB_INT8U wMaxPacketSize0;
    USB_INT8U wMaxPacketSize1;
    USB_INT8U bInterval;
} USB_ENDPOINT_DESCRIPTOR, *PUSB_ENDPOINT_DESCRIPTOR;
#endif

/*********************************************************************************************************
  USB string descriptor structure
*********************************************************************************************************/
#ifndef __USB_STRING_DESCRIPTOR_TYPE_
#define __USB_STRING_DESCRIPTOR_TYPE_
typedef struct __tagUSB_STRING_DESCRIPTOR {
    USB_INT8U bLength;
    USB_INT8U bDescriptorType;
    USB_INT8U bString[1];
} USB_STRING_DESCRIPTOR, *PUSB_STRING_DESCRIPTOR;
#endif

/*********************************************************************************************************
  USB OTG descriptor structure
*********************************************************************************************************/
#ifndef __USB_OTG_DESCRIPTOR_TYPE_
#define __USB_OTG_DESCRIPTOR_TYPE_
typedef struct __tagUSB_OTG_DESCRIPTOR {
    USB_INT8U bLength;
    USB_INT8U bDescriptorType;
    USB_INT8U bmAttributes;
} USB_OTG_DESCRIPTOR, *PUSB_OTG_DESCRIPTOR;
#endif

#ifdef __cplusplus
 }
#endif

#endif

#define __USB_MAXIMUM_STRING_LENGTH                 255                 /*  USB max string length       */

/*********************************************************************************************************
  USB power descriptor structure
*********************************************************************************************************/
typedef __packed struct __tagUSB_POWER_DESCRIPTOR {
    USB_INT8U  bLength;
    USB_INT8U  bDescriptorType;
    USB_INT8U  bCapabilitiesFlags;
    USB_INT16U EventNotification;
    USB_INT16U D1LatencyTime;
    USB_INT16U D2LatencyTime;
    USB_INT16U D3LatencyTime;
    USB_INT8U  PowerUnit;
    USB_INT16U D0PowerConsumption;
    USB_INT16U D1PowerConsumption;
    USB_INT16U D2PowerConsumption;
} __USB_POWER_DESCRIPTOR, *__PUSB_POWER_DESCRIPTOR;

/*********************************************************************************************************
  USB Power descriptor value
*********************************************************************************************************/
#define __USB_SUPPORT_D0_COMMAND                    0x01
#define __USB_SUPPORT_D1_COMMAND                    0x02
#define __USB_SUPPORT_D2_COMMAND                    0x04
#define __USB_SUPPORT_D3_COMMAND                    0x08

#define __USB_SUPPORT_D1_WAKEUP                     0x10
#define __USB_SUPPORT_D2_WAKEUP                     0x20


/*********************************************************************************************************
  USB common descriptor structure
*********************************************************************************************************/
typedef struct __tagUSB_COMMON_DESCRIPTOR {
    USB_INT8U bLength;
    USB_INT8U bDescriptorType;
} __USB_COMMON_DESCRIPTOR, *__PUSB_COMMON_DESCRIPTOR;

/*********************************************************************************************************
  USB HUB descriptor structure
*********************************************************************************************************/
typedef __packed struct __tagUSB_HUB_DESCRIPTOR {
    USB_INT8U  bDescriptorLength;                                       /*  Length of this descriptor   */
    USB_INT8U  bDescriptorType;                                         /*  Hub configuration type      */
    USB_INT8U  bNumberOfPorts;                                          /*  number of ports on this hub */
    USB_INT16U wHubCharacteristics;                                     /*  Hub Charateristics          */
    USB_INT8U  bPowerOnToPowerGood;                                     /*  port power on till power    */
                                                                        /*  ...good in 2ms              */
    USB_INT8U  bHubControlCurrent;                                      /*  max current in mA           */
    USB_INT8U  bRemoveAndPowerMask[64];                                 /*  room for 255 ports power    */
                                                                        /*  control and removable bitmask*/
} __USB_HUB_DESCRIPTOR, *__PUSB_HUB_DESCRIPTOR;


/*********************************************************************************************************
  common descriptor struture member value
*********************************************************************************************************/

/*********************************************************************************************************
  USB device request type mask
*********************************************************************************************************/
/*********************************************************************************************************
  (1) Receiver: D4...D0
*********************************************************************************************************/
#define __USB_RECIPIENT                             (USB_INT8U)0x1F
#define __USB_RECIPIENT_DEVICE                      (USB_INT8U)0x00
#define __USB_RECIPIENT_INTERFACE                   (USB_INT8U)0x01
#define __USB_RECIPIENT_ENDPOINT                    (USB_INT8U)0x02

/*********************************************************************************************************
  (2) Type: D6...D5
*********************************************************************************************************/
#define __USB_REQUEST_TYPE_MASK                     (USB_INT8U)0x60
#define __USB_STANDARD_REQUEST                      (USB_INT8U)0x00
#define __USB_CLASS_REQUEST                         (USB_INT8U)0x20
#define __USB_VENDOR_REQUEST                        (USB_INT8U)0x40

/*********************************************************************************************************
  (3) data direction: D7
*********************************************************************************************************/
#define __USB_DEVICE_ADDRESS_MASK                   0x7F
#define __USB_HOST_TO_DEVICE                        (USB_INT8U)0x00
#define __USB_DEVICE_TO_HOST                        (USB_INT8U)0x80

/*********************************************************************************************************
  (4) USB request mask
*********************************************************************************************************/
#define __USB_REQUEST_MASK                          (USB_INT8U)0x0F

/*********************************************************************************************************
  (5) USB endpoint direction mask
*********************************************************************************************************/
#define __USB_ENDPOINT_DIRECTION_MASK               0x80
#define __USB_ENDPOINT_DIRECTION_OUT(addr)          (!((addr) & __USB_ENDPOINT_DIRECTION_MASK))
#define __USB_ENDPOINT_DIRECTION_IN(addr)           ((addr) & __USB_ENDPOINT_DIRECTION_MASK)

/*********************************************************************************************************
  USB device descriptor type
*********************************************************************************************************/
#define __USB_DESCRIPTOR_TYPE_DEVICE                ((USB_INT8U)0x01)
#define __USB_DESCRIPTOR_TYPE_CONFIGURATION         ((USB_INT8U)0x02)
#define __USB_DESCRIPTOR_TYPE_STRING                ((USB_INT8U)0x03)
#define __USB_DESCRIPTOR_TYPE_INTERFACE             ((USB_INT8U)0x04)
#define __USB_DESCRIPTOR_TYPE_ENDPOINT              ((USB_INT8U)0x05)
#define __USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER      ((USB_INT8U)0x06)
#define __USB_DESCRIPTOR_TYPE_OTHER_SPEED_CFG       ((USB_INT8U)0x07)
#define __USB_DESCRIPTOR_TYPE_INTERFACE_POWER       ((USB_INT8U)0x08)
#define __USB_DESCRIPTOR_TYPE_OTG                   ((USB_INT8U)0x09)
#define __USB_DESCRIPTOR_TYPE_DEBUG                 ((USB_INT8U)0x0A)
#define __USB_DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION ((USB_INT8U)0x0B)

/*********************************************************************************************************
  correct value based on USB1.0 specification
*********************************************************************************************************/
#define __USB_REQUEST_GET_STATUS                    0x00
#define __USB_REQUEST_CLEAR_FEATURE                 0x01

#define __USB_REQUEST_SET_FEATURE                   0x03

#define __USB_REQUEST_SET_ADDRESS                   0x05
#define __USB_REQUEST_GET_DESCRIPTOR                0x06
#define __USB_REQUEST_SET_DESCRIPTOR                0x07
#define __USB_REQUEST_GET_CONFIGURATION             0x08
#define __USB_REQUEST_SET_CONFIGURATION             0x09
#define __USB_REQUEST_GET_INTERFACE                 0x0A
#define __USB_REQUEST_SET_INTERFACE                 0x0B
#define __USB_REQUEST_SYNC_FRAME                    0x0C

/*********************************************************************************************************
  USB feather select
*********************************************************************************************************/
#define __USB_FEATURE_ENDPOINT_STALL                0x0000
#define __USB_FEATURE_REMOTE_WAKEUP                 0x0001
#define __USB_FEATURE_POWER_D0                      0x0002
#define __USB_FEATURE_POWER_D1                      0x0003
#define __USB_FEATURE_POWER_D2                      0x0004
#define __USB_FEATURE_POWER_D3                      0x0005

/*********************************************************************************************************
  USB OTG Supplement feather select
*********************************************************************************************************/
#define __OTG_B_HNP_ENABLE                          3
#define __OTG_A_HNP_SUPPORT                         4
#define __OTG_A_ALT_HNP_SUPPORT                     5


#define __USB_DESCRIPTOR_MAKE_TYPE_AND_INDEX(d, i)  ((USB_INT16U)((USB_INT16U)d<<8 | i))

/*********************************************************************************************************
  USB configuration descriptor
*********************************************************************************************************/
#define __USB_CONFIG_POWERED_MASK                   0xc0

#define __USB_CONFIG_BUS_POWERED                    0x80
#define __USB_CONFIG_SELF_POWERED                   0x40
#define __USB_CONFIG_REMOTE_WAKEUP                  0x20

#define __BUS_POWERED                               0x80
#define __SELF_POWERED                              0x40
#define __REMOTE_WAKEUP                             0x20

/*********************************************************************************************************
  USB endpoint descriptor structure: endpoint attribute
*********************************************************************************************************/
#define __USB_ENDPOINT_TYPE_MASK                    0x03

#define __USB_ENDPOINT_TYPE_CONTROL                 0x00
#define __USB_ENDPOINT_TYPE_ISOCHRONOUS             0x01
#define __USB_ENDPOINT_TYPE_BULK                    0x02
#define __USB_ENDPOINT_TYPE_INTERRUPT               0x03

/*********************************************************************************************************
  USB  device class
*********************************************************************************************************/
#define __USB_DEVICE_CLASS_RESERVED                 0x00
#define __USB_DEVICE_CLASS_AUDIO                    0x01
#define __USB_DEVICE_CLASS_COMMUNICATIONS           0x02
#define __USB_DEVICE_CLASS_HUMAN_INTERFACE          0x03
#define __USB_DEVICE_CLASS_MONITOR                  0x04
#define __USB_DEVICE_CLASS_PHYSICAL_INTERFACE       0x05
#define __USB_DEVICE_CLASS_POWER                    0x06
#define __USB_DEVICE_CLASS_PRINTER                  0x07
#define __USB_DEVICE_CLASS_STORAGE                  0x08
#define __USB_DEVICE_CLASS_HUB                      0x09
#define __USB_DEVICE_CLASS_VENDOR_SPECIFIC          0xFF

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/


