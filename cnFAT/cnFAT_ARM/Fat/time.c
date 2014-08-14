 /*
+FHDR------------------------------------------------------------------
Copyright (c),
Tony Yang ¨Cspecialized in usb,fat firmware development
Contact:qq 292942278  e-mail:tony_yang123@sina.com.cn

Abstract:
$Id: fat.c,v 1.14 2007/05/11 03:00:55 design Exp $
-FHDR-------------------------------------------------------------------
*/ 

#include"types.h"
#include"FAT_cfg.h" 

 
#if enable_time_stamp_transaction
u8 get_time(u16 *date_time)
{
  u16 year,month,date,hour,minute,second;


  year = 2009;
  month = 3;
  date = 21;
  hour = 8;
  minute = 29;
  second = 1;

  date_time[0] = ((year - 1980) << 9) | (month << 5) | date;
  date_time[1] = (hour << 11) | (minute << 5) | second;
  return(SUCC);
}
#endif
