/*+FHDR------------------------------------------------------------------
��Ȩ����:

���ı�-רעUSB��FAT�ļ�ϵͳ�Ĺ̼��о�
��ϵ��ʽ:qq 292942278  ����:tony_yang123@sina.com.cn

����FAT16����Ѵ���,����Բ���,������о���
������FAT32����,�շѵİ汾,�����������ϵ������

Copyright (c),
Tony Yang �CSpecialized in the USB and FAT's firmware research and design
Contact method:qq 292942278  e-mail:tony_yang123@sina.com.cn

This code of FAT16 is free code, you can test, design, research of it 
as your freedom, also the code with FAT32 code vision is for charge version
pls contact with author when you want it to buy of it.

Abstract:
$Id: main.C,v 1.1.1.1 2007/01/01 10:35:32 tony Exp $
-FHDR-------------------------------------------------------------------*/

/*===============================================================================
����
��ȡ�����ϵ�һ������
��ڣ�u8 *buf:���������׵�ַ,u32 sector:�����������
���ڣ�SUCC
===============================================================================*/
extern u8 read_flash_sector(u8 *buf,u32 sector);

/*===============================================================================
����
�����ϵ�һ������д��
��ڣ�u8 *buf:���������׵�ַ,u32 sector:�����������
���ڣ�SUCC
===============================================================================*/
extern u8 write_flash_sector(u8 *buf,u32 sector);

/*===============================================================================
����
��ʼ��ģ����̵�����
��ڣ���
���ڣ���
===============================================================================*/
extern u8 flash_management_sysinit();
