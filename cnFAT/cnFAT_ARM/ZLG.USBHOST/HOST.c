
#define  USB_RAM_ALLOCATE

#include "COMMON\usbcommon.c"

#if 0
#include "DEVICE\Chap_9.c"
#include "DEVICE\usbDevCI.c"
#include "DEVICE\usbDevDescriptor.c"
#include "DEVICE\usbDevDMA.c"
#include "DEVICE\usbDevDriver.c"
#include "DEVICE\usbDevHal.c"

#include "ISP1301\ISP1301.c"
#endif

#include "HOST\MDD\hcd.c"
#include "HOST\MDD\usbd.c"
#include "HOST\MDD\usbShed.c"
#include "HOST\MDD\usbTransfer.c"

#include "HOST\PDD\usbHostHal.c"
#include "HOST\PDD\usbHostInt.c"

#include "CLASS\CHOST\MASSSTORAGE\HostRBC.c"
#include "CLASS\CHOST\MASSSTORAGE\massStorHost.c"

#include "CLASS\CHOST\MASSSTORAGE\msBulkOnly.c"

#if 0
#include "OTG\OTGDriver.c"
#include "OTG\OTGHal.c"
#endif
