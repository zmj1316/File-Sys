/*
+FHDR------------------------------------------------------------------
Copyright (c),
Tony Yang ¨Cspecialized in fat usb firmware development  
Contact:qq 292942278  e-mail:tony_yang123@sina.com.cn
;;;;;;;;;;
Abstract:
$Id: main.C,v 1.12 2007/05/10 11:13:14 design Exp $
-FHDR-------------------------------------------------------------------
*/
#include<include\types.h>
#include<include\FAT_cfg.h> 
#include <time.h>
 
#if enable_time_stamp_transaction
u8 get_time(u16 *date_time)
{
  u16 year,month,date,hour,minute,second;
  struct tm *local;
  time_t t;
  t=time(NULL);
  local=localtime(&t);

  year = local->tm_year + 1900;
  month = local->tm_mon+1;
  date = local->tm_mday;
  hour = local->tm_hour;
  minute = local->tm_min;
  second = local->tm_sec;

  //year = 2009;
  //month = 3;
  //date = 21;
  //hour = 8;
  //minute = 29;
  //second = 1;

  date_time[0] = ((year - 1980) << 9) | (month << 5) | date;
  date_time[1] = (hour << 11) | (minute << 5) | (second/2);
  return(SUCC);
}
#endif

/*
+FFTR--------------------------------------------------------------------
$Log: main.C,v $

-FFTR--------------------------------------------------------------------
*/