
//���̶�д
DRESULT disk_read (
	BYTE pdrv,		
	BYTE *buff,		//����
	DWORD sector,	//�������
	UINT count		//��д����
)
{
	DRESULT res;
	char a[255];
	sprintf(a,"%d.img",pdrv);
	fp=fopen(a,"rb+");
	fseek(fp,sector*512,0);
	fread(buff, count,512,fp);
	fclose(fp);
	return 0;
}

DRESULT disk_write (
	BYTE pdrv,			
	const BYTE *buff,	
	DWORD sector,		
	UINT count			
)
{
	DRESULT res;
	char a[255];
	sprintf(a,"%d.img",pdrv);
	fp=fopen(a,"rb+");
	fseek(fp,sector*512,0);
	fwrite(buff, 512,count,fp);
	fflush(fp);
	fclose(fp);
	printf("writed s:%d c:%d\n",sector,count);
	return 0;
}
//��ȡʱ��
DWORD get_fattime(){
	int year,month,date,hour,minute,second;
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

	return 	  ((DWORD)(year - 1980) << 25)
			| ((DWORD)month << 21)
			| ((DWORD)date << 16)
			| (WORD)(hour << 11)
			| (WORD)(minute << 5)
			| (WORD)(second >> 1);
}

//dump
void put_dump (
    const unsigned char* buff, 
    unsigned long addr,         
    int cnt                     
)
{
    int i;


    printf("%08lX:", addr);

    for (i = 0; i < cnt; i++)
        printf(" %02X", buff[i]);

    putchar(' ');
    for (i = 0; i < cnt; i++)
        putchar((TCHAR)((buff[i] >= ' ' && buff[i] <= '~') ? buff[i] : '.'));
    putchar('\n');
}

//�ṹ��Ϣ

/* �ļ�ϵͳ�ṹ */
typedef struct {
	BYTE	fs_type;		/*  �������� */
	BYTE	drv;			/* ��������ţ�012�ļ�����*/
	BYTE	csize;			/* ���������� 1 */
	BYTE	n_fats;			/* fat�� (1 or 2) */
	BYTE	wflag;			/* �Ƿ��л���ȴ�д��*/
	BYTE	fsi_flag;		/*  �Ƿ������ */
	WORD	id;				/* ��д��ʶ*/
	WORD	n_rootdir;		/* ��Ŀ¼�ļ���*/
	DWORD	last_clust;		/* Last allocated cluster �������*/
	DWORD	free_clust;		/* Number of free clusters ���д�*/
	DWORD	n_fatent;		/* fat��¼������������+����+�𻵣�*/
	DWORD	fsize;			/*  fat��¼��������*/
	DWORD	volbase;		/* ��ʼ���� */
	DWORD	fatbase;		/* FAT  ��¼��ʼ*/
	DWORD	dirbase;		/*  ��Ŀ¼��ʼ����*/
	DWORD	database;		/* ������ʼ*/
	DWORD	winsect;		/*��ǰ���������λ��*/
	BYTE	win[512];	/*  ����*/
} FATFS;



/* �ļ��洢�ṹ*/

typedef struct {
	FATFS*	fs;				/* �ļ�ϵͳָ�루ָ������Ľṹ��*/
	DWORD	fsize;			/* ��С*/
	DWORD	sclust;			/* ��ʼ��*/
	//DWORD	clust;			/* ��ǰָ��ָ��Ĵ� */
	DWORD	dsect;			/* ����������*/
	DWORD	dir_sect;		/*  Ŀ¼��¼��������*/
	BYTE*	dir_ptr;		/* ָ��Ŀ¼��¼��ָ��*/
	BYTE	buf[512];	/* File private data read/write window �ļ���д����*/
} FIL;



/*  Ŀ¼�ṹ*/

typedef struct {
	FATFS*	fs;				/*  */
	WORD	index;			/*  ��ǰ��д�ļ�������*/
	DWORD	sclust;			/* ��ʼ��(��Ŀ¼��ʶΪ0)*/
	DWORD	clust;			/*  ��ǰ��*/
	DWORD	sect;			/*  */
	BYTE*	dir;			/* Pointer to the current SFN entry in the win[] �����е�Ŀ¼��¼ָ��*/
	BYTE*	fn;				/* Pointer to the SFN (in/out) {file[8],ext[3],status[1]} �����еı�Ŀ¼�ļ���ָ��*/
} DIR;



/* �ļ���Ϣ�ṹ����Ŀ¼��¼��ȡ��*/

typedef struct {
	DWORD	fsize;			/*  ��С*/
	WORD	fdate;			/*  �༭����*/
	WORD	ftime;			/* ʱ��*/
	BYTE	fattrib;		/*  ����*/
	TCHAR	fname[13];		/* �ļ���*/
} FILINFO;
//Сͷת��
#define	LD_WORD(ptr)		(WORD)(((WORD)*((BYTE*)(ptr)+1)<<8)|(WORD)*(BYTE*)(ptr))
#define	LD_DWORD(ptr)		(DWORD)(((DWORD)*((BYTE*)(ptr)+3)<<24)|((DWORD)*((BYTE*)(ptr)+2)<<16)|((WORD)*((BYTE*)(ptr)+1)<<8)|*(BYTE*)(ptr))
#define	ST_WORD(ptr,val)	*(BYTE*)(ptr)=(BYTE)(val); *((BYTE*)(ptr)+1)=(BYTE)((WORD)(val)>>8)
#define	ST_DWORD(ptr,val)	*(BYTE*)(ptr)=(BYTE)(val); *((BYTE*)(ptr)+1)=(BYTE)((WORD)(val)>>8); *((BYTE*)(ptr)+2)=(BYTE)((DWORD)(val)>>16); *((BYTE*)(ptr)+3)=(BYTE)((DWORD)(val)>>24)


/*  �ļ�����*/

#define	AM_RDO	0x01	/* ֻ�� */
#define	AM_HID	0x02	/* ���� */
#define	AM_SYS	0x04	/* System */
#define	AM_VOL	0x08	/* Volume label */
#define AM_LFN	0x0F	/* LFN entry */
#define AM_DIR	0x10	/* Directory */
#define AM_ARC	0x20	/* Archive */
#define AM_MASK	0x3F	/* Mask of defined bits */
/*ȷ�ϻ���д����ɣ�ͬ����д���ڣ�*/
static
FRESULT sync_window (
	FATFS* fs		/* File system object */
)
{
	DWORD wsect;
	UINT nf;


	if (fs->wflag) {	/* Write back the sector if it is dirty ����Ƿ񻺴�*/
		wsect = fs->winsect;	/* Current sector number ��ǰ�����������ַ*/
		if (disk_write(fs->drv, fs->win, wsect, 1))
			return FR_DISK_ERR;
		fs->wflag = 0;
		if (wsect - fs->fatbase < fs->fsize) {		/* Is it in the FAT area? */
			for (nf = fs->n_fats; nf >= 2; nf--) {	/* Reflect the change to all FAT copies д�뵽����fat*/
				wsect += fs->fsize;
				disk_write(fs->drv, fs->win, wsect, 1);
			}
		}
	}
	return FR_OK;
}

/*�������� ���ڴ�*/
static
FRESULT move_window (
	FATFS* fs,		/* File system object */
	DWORD sector	/* Sector number to make appearance in the fs->win[] �����������*/
)
{
	if (sector != fs->winsect) {	/* Changed current window �ı䵱ǰ��д����*/
		if (sync_window(fs) != FR_OK)
			return FR_DISK_ERR;
		if (disk_read(fs->drv, fs->win, sector, 1))
			return FR_DISK_ERR;
		fs->winsect = sector;
	}

	return FR_OK;
}

/*�Ӵص�����*/
DWORD clust2sect (	
	FATFS* fs,		
	DWORD clst		
)
{
	clst -= 2;
	if (clst >= (fs->n_fatent - 2)) return 0;		
	return clst * fs->csize + fs->database;
}

//��ȡ�ⲿ�ļ���С
DWORD getsize(char * fn){
    DWORD count=0;
    FILE *fp;
    fp=fopen(fn,"rb+");
    while(!feof(fp)){
        fgetc(fp);
        count++;
    }
    fclose(fp);
    printf("%d",count);
    return count;
}                                  

void xcopy(DIR * dp,char * ptr){/*��ȡ���ⲿ�ļ�*/
    BYTE buff[512];
    BYTE name[20];
    DWORD ofs=dp->fs->dirbase;
    WORD fsect,dsect;
    DWORD size,remain;
    FILE *tar;
    BYTE idx=0,ofsect=0;
    char * fn;
    int i=0,count =0;
    char a[50]="data-";
    if(dp->sclust)
        disk_read(0,buff,clust2sect(dp->fs,dp->sclust),1);
    else
        disk_read(0,buff,dp->fs->dirbase,1);
    for(count=0;count<8&&*ptr;count++){
        if (*ptr>47&&*ptr<58||*ptr>=65&&*ptr<=90||*ptr>=97&&*ptr<=122)
            if(*ptr>=97&&*ptr<=122)
                name[count]=*ptr-32;
            else
                name[count]=*ptr;
        else
            if (*(ptr++)=='.')
                break;
        ptr++;
    }
    for(;count<8;count++){
        name[count]=' ';
    }
    for(count=0;count<3&&*ptr;count++){
        if (*ptr>47&&*ptr<58||*ptr>=65&&*ptr<=90||*ptr>=97&&*ptr<=122)
            if(*ptr>=97&&*ptr<=122)
                name[count+8]=*ptr-32;
            else
                name[count+8]=*ptr;
        ptr++;
    }
    for(;count<3;count++){
        name[count+8]=' ';
    }
    while(1){
             
        if (!memcmp(buff+32*idx,name,11)) {
            printf("File  Found!\n");
            break;
        }
 
        else idx++;
    }
    fsect=LD_WORD(buff+26+32*idx);
    remain=size=LD_DWORD(buff+28+32*idx);
    fn=buff+32*idx;
    count=0;
    for(i=0;i<8;i++){
        if (*(fn+i)!=' ')
            name[count++]=*(fn+i);
    }
    name[count++]='.';
    for(;i<11;i++){
        if (*(fn+i)!=' ')
            name[count++]=*(fn+i);
    }
    name[count]=0;
    printf("start sector: %lx\n", fsect);
    getch();
    strcat(a,name);
    tar=fopen(a,"wb+");
    dsect=fsect;
    while(remain>512){
        disk_read(0,buff,FatFs[0].database+dsect-2,1);
        /*printf("%lx\n", dsect);*/
        fseek(tar,size-remain,0);
        /*printf("%lx\n",size-remain );*/
        fwrite(buff,512,1,tar);
        dsect=get_fat(&FatFs[0],dsect);
        remain-=512;
        /*printf("%s", buff);*/
    }
    disk_read(0,buff,FatFs[0].database+dsect-2,1);
    fseek(tar,size-remain,0);
    /*printf("%lx\n",size-remain );*/
    fwrite(buff,remain,1,tar);
    /*printf("%s", buff);*/
    fclose(tar);

}

/*���ⲿ�����ļ�������Ŀ¼*/
void xput(DIR* dp,char * ptr){
    BYTE buff[512];
    BYTE file[512];
    BYTE name[15];
    BYTE ofsect=0;
    DWORD ofs=dp->fs->dirbase;
    WORD fsect,dsect;
    DWORD size,remain;
    FILE *tar;
    BYTE idx=0;
    int count=0;
    char * fn=ptr;
    if(dp->sclust)
    	disk_read(0,buff,clust2sect(dp->fs,dp->sclust),1);
    else
    	disk_read(0,buff,dp->fs->dirbase,1);
    tar=fopen(fn,"rb");
    remain=size=getsize(fn);
    for(count=0;count<8&&*ptr;count++){
        if (*ptr>47&&*ptr<58||*ptr>=65&&*ptr<=90||*ptr>=97&&*ptr<=122)
            if(*ptr>=97&&*ptr<=122)
                name[count]=*ptr-32;
            else
                name[count]=*ptr;
        else
            if (*(ptr++)=='.')
                break;
        ptr++;
    }
    for(;count<8;count++){
    	name[count]=' ';
    }
    for(count=0;count<3&&*ptr;count++){
        if (*ptr>47&&*ptr<58||*ptr>=65&&*ptr<=90||*ptr>=97&&*ptr<=122)
            if(*ptr>=97&&*ptr<=122)
                name[count+8]=*ptr-32;
            else
                name[count+8]=*ptr;
        ptr++;
    }
    for(;count<3;count++){
    	name[count+8]=' ';
    }
    while(1){
             
    	if (!memcmp(buff+32*idx,name,11)) {
    		fclose(tar);
    		
    		printf("There has been a File %s!\n",fn);
    		return 1;
    	}
        if(buff[32*idx]==0) break;

        else idx++;

    }
    memcpy(buff+32*idx,name,11);
    buff[32*idx+11]=0x20;
    dsect=fsect=create_chain(dp->fs,0);
    while(remain>512){
        fread(file,512,1,tar);
        disk_write(0,file,dp->fs->database+dsect-2,1);
        dsect=create_chain(dp->fs,dsect);
        remain-=512;
        /*fseek(tar,512,SEEK_CUR);*/
    }
    fread(file,remain,1,tar);
    for (count=remain+1;count<512;count++){
    	*(file+remain)=0;
    }
    disk_write(0,file,dp->fs->database+dsect-2,1);
    ST_DWORD(32*idx+buff+22,get_fattime());
    ST_DWORD(32*idx+buff+28,size);
    ST_WORD(32*idx+buff+26,fsect);
    printf("%lx\n",buff+28 );
    if(dp->sclust)
	    disk_write(0,buff,clust2sect(dp->fs,dp->sclust),1);
	else 
		disk_write(0,buff,dp->fs->dirbase,1);
}
//�½�Ŀ¼
void mkdir(FATFS* fs,char * ptr){
    BYTE buff[512];
    BYTE file[512];
    BYTE name[15];
    DWORD ofs=fs->dirbase;
    WORD fsect,dsect;
    DWORD size,remain;
    BYTE idx=0,count=0;
    char * fn=ptr;
    disk_read(0,buff,fs->dirbase,1);
    for(count=0;count<8&&*ptr;count++){
        if (*ptr>47&&*ptr<58||*ptr>=65&&*ptr<=90||*ptr>=97&&*ptr<=122)
            if(*ptr>=97&&*ptr<=122)
                name[count]=*ptr-32;
            else
                name[count]=*ptr;
        else
            if (*(ptr++)=='.')
                break;
        ptr++;
    }
    for(;count<8;count++){
        name[count]=' ';
    }
    for(count=0;count<3&&*ptr;count++){
        if (*ptr>47&&*ptr<58||*ptr>=65&&*ptr<=90||*ptr>=97&&*ptr<=122)
            if(*ptr>=97&&*ptr<=122)
                name[count+8]=*ptr-32;
            else
                name[count+8]=*ptr;
        ptr++;
    }
    for(;count<3;count++){
        name[count+8]=' ';
    }
    while(1){
             
        if (!memcmp(buff+32*idx,name,11)) {

            printf("There has been a File %s!\n",fn);
            return 1;
        }
        if(buff[32*idx]==0) break;

        else idx++;
    }
    memcpy(buff+32*idx,name,11);
    buff[32*idx+11]=0x10;
    dsect=fsect=create_chain(fs,0);
    ST_DWORD(32*idx+buff+22,get_fattime());
    ST_DWORD(32*idx+buff+28,size);
    ST_WORD(32*idx+buff+26,fsect);
    printf("%lx\n",buff+28 );
    disk_write(0,buff,fs->dirbase,1);
}


/*�Ӵص�����*/
DWORD clust2sect (	
	FATFS* fs,		
	DWORD clst		
)
{
	clst -= 2;
	if (clst >= (fs->n_fatent - 2)) return 0;		
	return clst * fs->csize + fs->database;
}

//fat����
/*�ض�ȡfat����ڲ�����Ĭ��Ϊfat16*/
DWORD get_fat (	
	FATFS* fs,	
	DWORD clst	
)
{
	UINT wc, bc;
	BYTE *p;


	if (clst < 2 || clst >= fs->n_fatent)	/* Check range */
		return 1;


		move_window(fs, fs->fatbase + (clst / (SS(fs) / 2))) ;/*16λ��fat���Գ�2*/
		p = &fs->win[clst * 2 % SS(fs)];
		return LD_WORD(p);


	

	return 0xFFFFFFFF;	/* An error occurred at the disk I/O layer */
}




/*��д��fat���*/


FRESULT put_fat (
	FATFS* fs,	
	DWORD clst,	
	DWORD val
)
{
	UINT bc;
	BYTE *p;
	FRESULT res;


	if (clst < 2 || clst >= fs->n_fatent) {	/* Check range */
		res = FR_INT_ERR;

	} else {

			res = move_window(fs, fs->fatbase + (clst / (SS(fs) / 2)));
			res != FR_OK;
			p = &fs->win[clst * 2 % SS(fs)];
			ST_WORD(p, (WORD)val);/*16λСͷ�洢*/

		fs->wflag = 1;/*��ǻ���δд��״̬*/
		sync_window(fs);
	}

	return res;
}

/*��������*/
static
DWORD create_chain (	
	FATFS* fs,			
	DWORD clst			
)
{
	DWORD cs, ncl, scl;
	FRESULT res;


	if (clst == 0) {		/* �������� */
		scl = fs->last_clust;			/* ����ǰ�Ĵ�δ����մؿ�ʼ */
		if (!scl || scl >= fs->n_fatent) scl = 1;/*���дض��Ѿ������*/
	}
	else {					/* ���� */
		cs = get_fat(fs, clst);			/*  *��鿪ʼ���Ƿ��*/
		if (cs < 2) return 1;			/* Invalid value */
		if (cs == 0xFFFFFFFF) return cs;	/* A disk error occurred */
		if (cs < fs->n_fatent) return cs;	/* It is already followed by next cluster �Ѿ�ռ�÷�����һ��*/
		scl = clst;
	}

	ncl = scl;				/* Start cluster */
	/*ѭ��ֱ���ҵ���һ���մ�*/
	for (;;) {
		ncl++;							/* Next cluster */
		if (ncl >= fs->n_fatent) {		/* �ص���ͷ */
			ncl = 2;
			if (ncl > scl) return 0;	
		}
		cs = get_fat(fs, ncl);			
		if (cs == 0) break;				/*  �ҵ��մ�*/
		if (ncl == scl) return 0;		/* No free cluster */
	}

	res = put_fat(fs, ncl, 0x0FFFFFFF);	/* ���Ϊ�ļ����� */
	if (res == FR_OK && clst != 0) {
		res = put_fat(fs, clst, ncl);	/* ����һ������ */
	}
	if (res == FR_OK) {
		fs->last_clust = ncl;			/* �����ļ�ϵͳ */
		if (fs->free_clust != 0xFFFFFFFF) {
			fs->free_clust--;
			fs->fsi_flag |= 1;
		}
	} else {
		ncl = (res == FR_DISK_ERR) ? 0xFFFFFFFF : 1;
	}

	return ncl;		/* �����´�*/
}
//��ȡĿ¼��Ϣ
FRESULT f_opendir (
	DIR* dp,			
	const TCHAR* path	
)
{
	FRESULT res;
	FATFS* fs;
	DEF_NAMEBUF;


	if (!dp) return FR_INVALID_OBJECT;
	if (res == FR_OK) {
		dp->fs = fs;
		if (res == FR_OK) {					
			if (dp->dir) {						
				if (dp->dir[DIR_Attr] & AM_DIR)	
					dp->sclust = ld_clust(fs, dp->dir);
				else							/* The object is a file */
					res = FR_NO_PATH;
			}
			if (res == FR_OK) {
				dp->id = fs->id;
				res = dir_sdi(dp, 0);			

			}
		}
		
	}
}

//��ȡ�ļ���Ϣ
static
void get_fileinfo (		/* No return code */
	DIR* dp,			/* Pointer to the directory object */
	FILINFO* fno	 	/* Pointer to the file information to be filled */
)
{
	UINT i;
	TCHAR *p, c;


	p = fno->fname;
	if (dp->sect) {		/* ��ȡ����*/
		BYTE *dir = dp->dir;/*Ŀ¼����Ŀ������*/
		i = 0;
		while (i < 11) {		/*  �ļ�������չ��*/
			c = (TCHAR)dir[i++];
			if (c == ' ') continue;			/* �����ո� */
			if (i == 9) *p++ = '.';			/*  ��չ��*/
			*p++ = c;
		}
		fno->fattrib = dir[DIR_Attr];				/*  ��ȡ����*/
		fno->fsize = LD_DWORD(dir+DIR_FileSize);	/*  ��С*/
		fno->fdate = LD_WORD(dir+DIR_WrtDate);		/* Date */
		fno->ftime = LD_WORD(dir+DIR_WrtTime);		/* Time */
	}
	*p = 0;		/* Terminate SFN string by a \0 �ļ�������*/
}

