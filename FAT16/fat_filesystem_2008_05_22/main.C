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

#include "stdio.h"

#include "include\types.h"

#include "fat\fat.h"

#include "Flash_Management\Flash_Management.h" 

#include "include\FAT_cfg.h" 

/*===============================================================================
����
main()
�û�����ͨ���޸����еĴ��룬���ﵽ�����ļ�ϵͳ�Ĺ��ܵ�Ŀ��
��ڣ���
���ڣ���
===============================================================================*/ 
FILE *file2;
void main(void)
{
  u32 cc,bb;
  u16 COUNT;
  u8 a,ATTR,j;
  u8 HANDLE1; 
  u8 buf[65535];
  u8 ddd[] = "c:\\ok\\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaAAAAAAAAAAAAAAAaaaaaaaaa.rar";
  
  //��ʼ��������̵�����
  flash_management_sysinit();
  FAT_filesystem_autoformat(0,FAT16,483328);
  FAT_filesystem_initialiation();

  //���ú���volume_inquiry,��ѯ����������ʣ��ռ�
  volume_inquiry('c',&cc,&bb);

  //��ӡ������̷���������ʣ��ռ�
  printf("Volume Capacity: %ld\n",cc);
  printf("Volume FreeSpace: %ld\n",bb);
  scanf("%c",&a);

  /*����Ŀ¼CREATE_FOLDER_TEST�����ڲ��Խ���Ŀ¼����create_floder()*/

  create_floder("CREATE_FOLDER_TEST");
  create_floder("CREATE_FOLDER_TEST");//Ŀ¼�ٽ�ʧ��, �����Խ��ظ���Ŀ¼


  /*Ŀ¼��������*/
  create_floder("a");
  create_floder("a\\b");
  create_floder("a\\b\\c");
  create_floder("a\\b\\c\\d");


  /*���ļ���Ŀ¼CREATE_FOLDER_TEST�£����ڲ��Խ����ļ�����create_file()*/
  
  create_file("C:\\CREATE_FOLDER_TEST\\created_file.txt");

  create_file("C:\\CREATE_FOLDER_TEST\\created_file.txt");//�ļ��ٽ�ʧ��,�����Խ��ظ����ļ�


  /* �������ļ�CREATE_FOLDER_TEST\\created_file.txtΪ"created_file_new.txt",���ڲ����ļ�����������rename_file() */
  rename_file("C:\\CREATE_FOLDER_TEST\\created_file.txt","created_file_new.txt");
	
  /*  ��ȡĿ¼CREATE_FOLDER_TEST���������ENTRY����ӡ��������Ļ*/
  cd_folder("CREATE_FOLDER_TEST",0);
 
  a = 0; //ö��ģʽ��ʼ�趨
  while(folder_enumeration(buf,a,&ATTR) == SUCC)
  { 
      //��ӡ��ȡ��Directory ENTRY����attribute
	  printf("\nreaded entry=%s Attr = %x",buf,ATTR);

     if(a == 0)
	   a = 1;//ö��ģʽ�޸�Ϊö�ټ���
  }

  scanf("%c",&a);	

  cd_folder(" ",1); //���ص���Ŀ¼

  /*  ��ȡ��Ŀ¼������ENTRY����ӡ��������Ļ  */

  a = 0;//ö��ģʽ��ʼ�趨
  while(folder_enumeration(buf,a,&ATTR) == SUCC)
  { 
	  //��ӡ��ȡ��Directory ENTRY����attribute
     printf("\nreaded entry=%s Attr = %x",buf,ATTR);

     if(a == 0)
	   a = 1;//ö��ģʽ�޸�Ϊö�ټ���
  }

  scanf("%c",&a);	 

  /* ����һ��OKĿ¼,֮������д�ļ�readed.rar��ֱ�������̱�д�� */ 
  create_floder("OK");

  j = 0;
  COUNT = 0;
  do{
  for(a = 0;a<25;a++)
  {
  COUNT++;
  //����create_file()�������Ͻ����ļ� 
  if(create_file(ddd) == SUCC)
  {
   printf("Create file %s successfully\n",ddd);
   
   HANDLE1 = open_file(ddd);//�򿪸ո���ɽ������ļ�

   //��readed.rar,��Ϊд��Դ�ļ�
   if ((file2 = fopen("readed.rar","rb+")) == NULL)
     {
      return;
     }
   if(HANDLE1 != FAIL)
	{   
		//��readed.rar����ոս������ļ�   
		printf("\nOpen File %s successfully",ddd);
		do{
            printf("x");
			//��readed.rar�������
            cc = fread(buf,1,40000,file2);  
	        //�����ݺ���ոս������ļ� 
			write_file(HANDLE1,buf, cc);
		    if(cc < 40000)
			    break;
		}while(1);
		//���������ļ�������,֮��ر��ļ�
		close_file(HANDLE1);   
		fclose(file2);
	}

  }
  else
   printf("Create file %s failed\n",ddd);
   ddd[6+j] ++; 
  }
  j++;
 }while(j< 40);



  /*����������ϵ����е��ļ���Ŀ¼���뵽�ףɣΣģϣף���*/
 
  a = 0;//ö��ģʽ��ʼ�趨
  while(disk_enumeration(buf,a,&ATTR) == SUCC)
  { 
     printf("\nreaded entry=%s Attr = %x",buf,ATTR);
     
     if(a == 0)
	   a = 1;	//ö��ģʽ�޸�Ϊö�ټ���

	 if(ATTR & ATTR_DIRECTORY)
	 { 
	   mkdir(buf+3);
	   continue;
	 }
	 else
     {   //��windows��ǰĿ¼�½���һ���ļ�,�ļ����뵱ǰö�ٵ��ļ���һ��*/
		 if ((file2 = fopen(buf+3,"wb+")) == NULL)
		 {  
            return;
		 }    
	     else 
		     HANDLE1 = open_file(buf);
	 }//�����ݺ���ոս������ļ� 
	if(HANDLE1 != FAIL)
		do{	
	    //�ӵ�ǰ���ļ���ȡ����
	    cc = read_file(HANDLE1,buf+400,50000);
		printf("\nreaded chars = %ld ",cc);
        //�����ݺ���ոս������ļ�
		fwrite(buf+400,1,cc,file2);
		if(cc != 50000)
		{ 
			close_file(HANDLE1);
		    break;   
		}   
	
	}while(1);
    else{
	}
    fclose(file2);	 
  }

  scanf("%c",&a);	
}
   

