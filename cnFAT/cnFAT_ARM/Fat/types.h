 /*
+FHDR------------------------------------------------------------------
Copyright (c),
Tony Yang �Cspecialized in usb,fat firmware development
Contact:qq 292942278  e-mail:tony_yang123@sina.com.cn

Abstract:
$Id: fat.c,v 1.14 2007/05/11 03:00:55 design Exp $
-FHDR-------------------------------------------------------------------
*/ 
#define SUCC 0x0
#define FAIL 0Xff

typedef unsigned char  u8;         /* �޷���8λ���ͱ���*/
typedef signed char  s8;           /* �з���8λ���ͱ���*/
typedef unsigned int u16;          /* �޷���16λ���ͱ���*/
typedef signed int s16;            /* �з���16λ���ͱ���*/
typedef unsigned long u32;         /* �޷���32λ���ͱ���*/
typedef signed long s32;           /* �з���32λ���ͱ���*/


/*
+FFTR--------------------------------------------------------------------
$Log: types.h,v $
Revision 1.3  2007/03/06 12:26:56  yangwenbin
����cd_folder()��ɣ� mode 0/mode1����ģʽ����ͨ��

Revision 1.2  2007/02/28 13:39:52  Design
����open_file�򿪾���·��+���ļ�������ͨ��
ʹ��C�������£�
open_file("C:\\HELLO.TXT")

Revision 1.1.1.1  2007/02/26 14:01:12  Design
volume_inquire�������Գɹ�

Revision 1.1.1.1  2007/02/24 07:59:45  yangwenbin
����FAT16�ļ�ϵͳ��������

-FFTR--------------------------------------------------------------------
*/



