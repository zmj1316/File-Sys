
//磁盘读写
DRESULT disk_read (
	BYTE pdrv,		
	BYTE *buff,		//缓存
	DWORD sector,	//扇区编号
	UINT count		//读写计数
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
//获取时间
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

//结构信息

/* 文件系统结构 */
typedef struct {
	BYTE	fs_type;		/*  分区类型 */
	BYTE	drv;			/* 驱动器编号（012文件名）*/
	BYTE	csize;			/* 单簇扇区数 1 */
	BYTE	n_fats;			/* fat数 (1 or 2) */
	BYTE	wflag;			/* 是否有缓存等待写入*/
	BYTE	fsi_flag;		/*  是否分区表 */
	WORD	id;				/* 读写标识*/
	WORD	n_rootdir;		/* 根目录文件数*/
	DWORD	last_clust;		/* Last allocated cluster 最后分配簇*/
	DWORD	free_clust;		/* Number of free clusters 空闲簇*/
	DWORD	n_fatent;		/* fat记录类型数（簇数+空闲+损坏）*/
	DWORD	fsize;			/*  fat记录的扇区数*/
	DWORD	volbase;		/* 卷开始扇区 */
	DWORD	fatbase;		/* FAT  记录开始*/
	DWORD	dirbase;		/*  根目录起始扇区*/
	DWORD	database;		/* 数据起始*/
	DWORD	winsect;		/*当前缓存的扇区位置*/
	BYTE	win[512];	/*  缓存*/
} FATFS;



/* 文件存储结构*/

typedef struct {
	FATFS*	fs;				/* 文件系统指针（指向上面的结构）*/
	DWORD	fsize;			/* 大小*/
	DWORD	sclust;			/* 起始簇*/
	//DWORD	clust;			/* 当前指针指向的簇 */
	DWORD	dsect;			/* 缓存中扇区*/
	DWORD	dir_sect;		/*  目录记录所在扇区*/
	BYTE*	dir_ptr;		/* 指向目录记录的指针*/
	BYTE	buf[512];	/* File private data read/write window 文件读写缓存*/
} FIL;



/*  目录结构*/

typedef struct {
	FATFS*	fs;				/*  */
	WORD	index;			/*  当前读写文件索引号*/
	DWORD	sclust;			/* 起始簇(根目录标识为0)*/
	DWORD	clust;			/*  当前簇*/
	DWORD	sect;			/*  */
	BYTE*	dir;			/* Pointer to the current SFN entry in the win[] 缓存中的目录记录指针*/
	BYTE*	fn;				/* Pointer to the SFN (in/out) {file[8],ext[3],status[1]} 缓存中的本目录文件名指针*/
} DIR;



/* 文件信息结构（从目录记录读取）*/

typedef struct {
	DWORD	fsize;			/*  大小*/
	WORD	fdate;			/*  编辑日期*/
	WORD	ftime;			/* 时间*/
	BYTE	fattrib;		/*  属性*/
	TCHAR	fname[13];		/* 文件名*/
} FILINFO;
//小头转换
#define	LD_WORD(ptr)		(WORD)(((WORD)*((BYTE*)(ptr)+1)<<8)|(WORD)*(BYTE*)(ptr))
#define	LD_DWORD(ptr)		(DWORD)(((DWORD)*((BYTE*)(ptr)+3)<<24)|((DWORD)*((BYTE*)(ptr)+2)<<16)|((WORD)*((BYTE*)(ptr)+1)<<8)|*(BYTE*)(ptr))
#define	ST_WORD(ptr,val)	*(BYTE*)(ptr)=(BYTE)(val); *((BYTE*)(ptr)+1)=(BYTE)((WORD)(val)>>8)
#define	ST_DWORD(ptr,val)	*(BYTE*)(ptr)=(BYTE)(val); *((BYTE*)(ptr)+1)=(BYTE)((WORD)(val)>>8); *((BYTE*)(ptr)+2)=(BYTE)((DWORD)(val)>>16); *((BYTE*)(ptr)+3)=(BYTE)((DWORD)(val)>>24)


/*  文件属性*/

#define	AM_RDO	0x01	/* 只读 */
#define	AM_HID	0x02	/* 隐藏 */
#define	AM_SYS	0x04	/* System */
#define	AM_VOL	0x08	/* Volume label */
#define AM_LFN	0x0F	/* LFN entry */
#define AM_DIR	0x10	/* Directory */
#define AM_ARC	0x20	/* Archive */
#define AM_MASK	0x3F	/* Mask of defined bits */
/*确认缓存写入完成（同步读写窗口）*/
static
FRESULT sync_window (
	FATFS* fs		/* File system object */
)
{
	DWORD wsect;
	UINT nf;


	if (fs->wflag) {	/* Write back the sector if it is dirty 检查是否缓存*/
		wsect = fs->winsect;	/* Current sector number 当前缓存的扇区地址*/
		if (disk_write(fs->drv, fs->win, wsect, 1))
			return FR_DISK_ERR;
		fs->wflag = 0;
		if (wsect - fs->fatbase < fs->fsize) {		/* Is it in the FAT area? */
			for (nf = fs->n_fats; nf >= 2; nf--) {	/* Reflect the change to all FAT copies 写入到其他fat*/
				wsect += fs->fsize;
				disk_write(fs->drv, fs->win, wsect, 1);
			}
		}
	}
	return FR_OK;
}

/*缓存扇区 至内存*/
static
FRESULT move_window (
	FATFS* fs,		/* File system object */
	DWORD sector	/* Sector number to make appearance in the fs->win[] 缓存扇区编号*/
)
{
	if (sector != fs->winsect) {	/* Changed current window 改变当前读写窗口*/
#if !_FS_READONLY
		if (sync_window(fs) != FR_OK)
			return FR_DISK_ERR;
#endif
		if (disk_read(fs->drv, fs->win, sector, 1))
			return FR_DISK_ERR;
		fs->winsect = sector;
	}

	return FR_OK;
}

/*从簇得扇区*/
DWORD clust2sect (	
	FATFS* fs,		
	DWORD clst		
)
{
	clst -= 2;
	if (clst >= (fs->n_fatent - 2)) return 0;		
	return clst * fs->csize + fs->database;
}

//获取外部文件大小
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

void xcopy(DIR * dp,char * ptr){/*读取到外部文件*/
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

/*从外部复制文件进磁盘目录*/
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
//新建目录
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


/*从簇得扇区*/
DWORD clust2sect (	
	FATFS* fs,		
	DWORD clst		
)
{
	clst -= 2;
	if (clst >= (fs->n_fatent - 2)) return 0;		
	return clst * fs->csize + fs->database;
}

//fat操作
/*簇读取fat中入口并返回默认为fat16*/
DWORD get_fat (	
	FATFS* fs,	
	DWORD clst	
)
{
	UINT wc, bc;
	BYTE *p;


	if (clst < 2 || clst >= fs->n_fatent)	/* Check range */
		return 1;


		move_window(fs, fs->fatbase + (clst / (SS(fs) / 2))) ;/*16位的fat所以除2*/
		p = &fs->win[clst * 2 % SS(fs)];
		return LD_WORD(p);


	

	return 0xFFFFFFFF;	/* An error occurred at the disk I/O layer */
}




/*簇写入fat入口*/


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
			ST_WORD(p, (WORD)val);/*16位小头存储*/

		fs->wflag = 1;/*标记缓存未写入状态*/
		sync_window(fs);
	}

	return res;
}

/*创建链表*/
static
DWORD create_chain (	
	FATFS* fs,			
	DWORD clst			
)
{
	DWORD cs, ncl, scl;
	FRESULT res;


	if (clst == 0) {		/* 创建新链 */
		scl = fs->last_clust;			/* 从最前的从未分配空簇开始 */
		if (!scl || scl >= fs->n_fatent) scl = 1;/*所有簇都已经分配过*/
	}
	else {					/* 连接 */
		cs = get_fat(fs, clst);			/*  *检查开始簇是否空*/
		if (cs < 2) return 1;			/* Invalid value */
		if (cs == 0xFFFFFFFF) return cs;	/* A disk error occurred */
		if (cs < fs->n_fatent) return cs;	/* It is already followed by next cluster 已经占用返回下一个*/
		scl = clst;
	}

	ncl = scl;				/* Start cluster */
	/*循环直到找到下一个空簇*/
	for (;;) {
		ncl++;							/* Next cluster */
		if (ncl >= fs->n_fatent) {		/* 回到开头 */
			ncl = 2;
			if (ncl > scl) return 0;	
		}
		cs = get_fat(fs, ncl);			
		if (cs == 0) break;				/*  找到空簇*/
		if (ncl == scl) return 0;		/* No free cluster */
	}

	res = put_fat(fs, ncl, 0x0FFFFFFF);	/* 标记为文件结束 */
	if (res == FR_OK && clst != 0) {
		res = put_fat(fs, clst, ncl);	/* 与上一个连接 */
	}
	if (res == FR_OK) {
		fs->last_clust = ncl;			/* 更新文件系统 */
		if (fs->free_clust != 0xFFFFFFFF) {
			fs->free_clust--;
			fs->fsi_flag |= 1;
		}
	} else {
		ncl = (res == FR_DISK_ERR) ? 0xFFFFFFFF : 1;
	}

	return ncl;		/* 返回新簇*/
}
//获取目录信息
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

//获取文件信息
static
void get_fileinfo (		/* No return code */
	DIR* dp,			/* Pointer to the directory object */
	FILINFO* fno	 	/* Pointer to the file information to be filled */
)
{
	UINT i;
	TCHAR *p, c;


	p = fno->fname;
	if (dp->sect) {		/* 获取名称*/
		BYTE *dir = dp->dir;/*目录索引目标名称*/
		i = 0;
		while (i < 11) {		/*  文件名和扩展名*/
			c = (TCHAR)dir[i++];
			if (c == ' ') continue;			/* 跳过空格 */
			if (i == 9) *p++ = '.';			/*  扩展名*/
			*p++ = c;
		}
		fno->fattrib = dir[DIR_Attr];				/*  读取属性*/
		fno->fsize = LD_DWORD(dir+DIR_FileSize);	/*  大小*/
		fno->fdate = LD_WORD(dir+DIR_WrtDate);		/* Date */
		fno->ftime = LD_WORD(dir+DIR_WrtTime);		/* Time */
	}
	*p = 0;		/* Terminate SFN string by a \0 文件名结束*/
}

