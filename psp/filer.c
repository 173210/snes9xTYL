//#include "main.h"
#include "snes9x.h"
#include "psp.h"
#include "filer.h"
#include "homehook.h"

#define TITLE_COL ((31)|(26<<5)|(31<<10))
#define PATH_COL ((31)|(24<<5)|(28<<10))

#define FILE_COL ((20)|(20<<5)|(31<<10))
#define DIR_COL ((8)|(31<<5)|(8<<10))

#define SEL_COL ((30)|(30<<5)|(31<<10))
#define SELDIR_COL ((28)|(31<<5)|(28<<10))

#define NET_COL (28|(4<<5)|(16<<10))


#define INFOBAR_COL ((31)|(24<<5)|(20<<10))

#define INFOBAR_COL2 ((31)|(28<<5)|(31<<10))
#define INFOBAR_COL3 ((31)|(31<<5)|(28<<10))
#define INFOBAR_COL4 ((28)|(31<<5)|(31<<10))

#define MAXPATH 256		//temp, not confirmed
#define MAX_ENTRY 1024

u32 new_pad,old_pad;

enum { 
    TYPE_DIR=0x10, 
    TYPE_FILE=0x20 
}; 


char jpeg_files[MAX_ENTRY];
SceIoDirent file;
int nfiles;
SceIoDirent files[MAX_ENTRY];

int nfiles_jpeg;
SceIoDirent files_jpeg[MAX_ENTRY];

extern volatile int g_bSleep,g_bLoop;

extern int os9x_cpuclock,os9x_usballowed;
extern int os9x_lowbat;
extern char LaunchDir[256];
extern char SaveDir[256];
extern int os9x_language;
extern int os9x_netplay;
extern int bg_img_mul;

extern int os9x_savesnap();
extern int os9x_loadsnap(char *fname,u16 *snes_image,int *height);

extern void show_background(int mul,int add);

extern void show_batteryinfo(void);
extern void show_usbinfo(void);

int psp_ExitCheck(void);

char LastPath[MAXPATH];
char FilerMsg[256];
char FileName[MAXPATH];
u16 *filer_bg;

#define timercmp(a, b, CMP)	(((a)->tv_sec == (b)->tv_sec) ? ((a)->tv_usec CMP (b)->tv_usec) : ((a)->tv_sec CMP (b)->tv_sec))

struct timeval filer_next,filer_cur;

extern void show_bg(u16 *bg);

////////////////////////////////////////////////////////////////////////
// クイックソート
void SJISCopy(struct SceIoDirent *a, char *file)
{
	char ca;
	int i;

	for(i=0;i<=strlen(a->d_name);i++){
		ca = a->d_name[i];
		if (((0x81 <= ca)&&(ca <= 0x9f))
		|| ((0xe0 <= ca)&&(ca <= 0xef))){
			file[i++] = ca;
			file[i] = a->d_name[i];
		}
		else{
			if(ca>='a' && ca<='z') ca-=0x20;
			file[i] = ca;
		}
	}
}
//#include <curl\stdcheaders.h>
int cmpFile(SceIoDirent *a, SceIoDirent *b) {
	unsigned char file1[0x108];
	unsigned char file2[0x108];
	unsigned char ca, cb;
	int i, n, ret;	
	if(a->d_stat.st_attr==b->d_stat.st_attr) {
		SJISCopy(a, (char *)file1);
		SJISCopy(b, (char *)file2);
		//return strcasecmp(file1,file2);
		n=strlen((char *)file1);
		for(i=0; i<=n; i++){
			ca=file1[i]; cb=file2[i];
			ret = ca-cb;
			if(ret!=0) return ret;
		}
		return 0;
	}
	
	if(a->d_stat.st_attr & FIO_SO_IFDIR)	return -1;
	else					return 1;
}

void sort(SceIoDirent *a, int left, int right) {
	SceIoDirent tmp, pivot;
	int i, p;
	
	if (left < right) {
		pivot = a[left];
		p = left;
		for (i=left+1; i<=right; i++) {
			if (cmpFile(&a[i],&pivot)<0){
				p=p+1;
				tmp=a[p];
				a[p]=a[i];
				a[i]=tmp;
			}
		}
		a[left] = a[p];
		a[p] = pivot;
		sort(a, left, p-1);
		sort(a, p+1, right);
	}
}

// 拡張子管理用
const struct {
	char *szExt;
	int nExtId;
} stExtentions[] = {
    {"zip",EXT_ZIP},
    {"smc",EXT_SMC},
    {"sfc",EXT_SFC},
    {"fig",EXT_FIG},
    {"bin",EXT_BIN},
    {"1",EXT_1},
    {"spc",EXT_SPC},
    {"txt",EXT_TXT},
	{NULL, EXT_UNKNOWN}
};

int getExtId(const char *szFilePath) {
	char *pszExt;
	int i;
	if((pszExt = strrchr(szFilePath, '.'))) {
		pszExt++;
		for (i = 0; stExtentions[i].nExtId != EXT_UNKNOWN; i++) {
			if (!strcasecmp(stExtentions[i].szExt,pszExt)) {
				return stExtentions[i].nExtId;
			}
		}
	}
	return EXT_UNKNOWN;
}


char *find_file(char *pattern,char *path){
	int fd,found;	
	fd = sceIoDopen(path);
	if (fd<0) {
		psp_msg(ERR_READ_MEMSTICK,MSG_DEFAULT);
		return NULL;
	}
	found=0;	
	while(1){
		if(sceIoDread(fd, &file)<=0) break;
		if (strcasestr(file.d_name,pattern)) {found=1;break;}		
	}
	sceIoDclose(fd);
	if (found) return file.d_name;
	return NULL;
}

void getDir(const char *path) {
	int fd = 0;
	int b = 0;
//	char *p;
	
	nfiles = 0;
			
	if(path[5]) {
		strcpy(files[nfiles].d_name,"..");
	} else if (*path == 'm') {
		fd = sceIoDopen("ef0:/");
		if (fd >= 0) {
			sceIoDclose(fd);
			strcpy(files[nfiles].d_name,"ef0:/");
		}
	} else strcpy(files[nfiles].d_name,"ms0:/");

	if (fd >= 0) {
		files[nfiles].d_stat.st_attr = TYPE_DIR;
		nfiles++;
		b=1;
	}
		
	fd = sceIoDopen(path);
	if (fd<0){
		psp_msg(ERR_READ_MEMSTICK,MSG_DEFAULT);
		return ;
	}
	
	while(nfiles<MAX_ENTRY){
		if(sceIoDread(fd, &files[nfiles])<=0) break;
				
		if(files[nfiles].d_name[0] == '.') continue;
				
		if(files[nfiles].d_stat.st_attr == TYPE_DIR){
			strcat(files[nfiles].d_name, "/");
			nfiles++;
			continue;
		}
		if(getExtId(files[nfiles].d_name) != EXT_UNKNOWN) nfiles++;
	}

	sceIoDclose(fd);

	if (nfiles) {
		if(b)
			sort(files+1, 0, nfiles-2);
		else
			sort(files, 0, nfiles-1);
	}
}

void getDirJpeg() {
	int fd;
	nfiles_jpeg = 0;

	fd = sceIoDopen(SaveDir);
	if (fd<0){
		psp_msg(ERR_READ_MEMSTICK,MSG_DEFAULT);
		return ;
	}

	while(nfiles_jpeg<MAX_ENTRY){
		if(sceIoDread(fd, &files_jpeg[nfiles_jpeg])<=0) break;
				
		if(files_jpeg[nfiles_jpeg].d_name[0] == '.') continue;
				
		if(files_jpeg[nfiles_jpeg].d_stat.st_attr == TYPE_DIR) {
			continue;
		}

		if(strstr(files_jpeg[nfiles_jpeg].d_name,".jpg")) nfiles_jpeg++;
	}

	sceIoDclose(fd);
}


void getDirNoExt(const char *path) {
	int fd = 0;
	int b = 0;
//	char *p;

	nfiles = 0;

	inputBoxOK(path);

	if(path[5]) {
		strcpy(files[nfiles].d_name,"..");
	} else if (*path == 'm') {
		fd = sceIoDopen("ef0:/");
		if (fd >= 0) {
			sceIoDclose(fd);
			strcpy(files[nfiles].d_name,"ef0:/");
		}
	} else strcpy(files[nfiles].d_name,"ms0:/");

	if (fd >= 0) {
		files[nfiles].d_stat.st_attr = TYPE_DIR;
		nfiles++;
		b=1;
	}
		
	fd = sceIoDopen(path);
	if (fd<0){
		psp_msg(ERR_READ_MEMSTICK,MSG_DEFAULT);
		return ;
	}
	
	while(nfiles<MAX_ENTRY){
		if(sceIoDread(fd, &files[nfiles])<=0) break;
				
		if(files[nfiles].d_name[0] == '.') continue;
				
		if(files[nfiles].d_stat.st_attr == TYPE_DIR){
			strcat(files[nfiles].d_name, "/");
			nfiles++;
			continue;
		}
		/*if(getExtId(files[nfiles].d_name) != EXT_UNKNOWN)*/ nfiles++;
	}
		
	sceIoDclose(fd);
		
	if (nfiles) {
		if(b)
			sort(files+1, 0, nfiles-2);
		else
			sort(files, 0, nfiles-1);
	}
}

void filer_buildbg(int detailed) {
	u16 *dst,*src;
	int i;
	if (detailed) {
		show_background(bg_img_mul,(os9x_lowbat?0x600000:0)|(os9x_netplay?100:0));	
		if (os9x_netplay) pgPrint4(4,4,28|(4<<5)|(16<<10),31|(28<<5)|(30<<10),"NETPLAY");			
	} else show_background(bg_img_mul,(os9x_lowbat?0x600000:0));	
	pgDrawFrame(0,10,479,10,(8<<10)|(8<<5)|8);
	pgDrawFrame(0,11,479,11,(30<<10)|(30<<5)|30);
	pgDrawFrame(0,12,479,12,(8<<10)|(8<<5)|8);
	pgDrawFrame(0,272-13,479,272-13,(8<<10)|(8<<5)|8);
	pgDrawFrame(0,272-12,479,272-12,(30<<10)|(30<<5)|30);
	pgDrawFrame(0,272-11,479,272-11,(8<<10)|(8<<5)|8);
	pgFillBoxHalfer(0,0,479,9);
	pgFillBoxHalfer(0,272-10,479,271);	
	
	if (detailed) {			
		pgFillBoxHalfer(300,180,479,272-13);
		mh_print(310,190,psp_msg_string(FILER_HELP_WINDOW1),INFOBAR_COL2);
		mh_print(310,200,psp_msg_string(FILER_HELP_WINDOW2),INFOBAR_COL2);
		mh_print(310,215,psp_msg_string(FILER_HELP_WINDOW3),INFOBAR_COL3);
		mh_print(310,230,psp_msg_string(FILER_HELP_WINDOW4),INFOBAR_COL4);
		mh_print(310,245,psp_msg_string(FILER_HELP_WINDOW5),INFOBAR_COL4);
	}
		
	dst=filer_bg;
	for (i=0;i<272;i++) {
		src = (u16*)pgGetVramAddr(0,i);
		memcpy(dst,src,480*2);
		dst+=480;
	}	
	
}

int getFilePath(char *out,int can_exit) {
	static int cpt_lowbat=0;
  int counter=0;
	unsigned long color=RGB_WHITE;
	static int sel=0;
	int reload_entries=0;
	int rows=24, top=0, x, y, h, i, /*len,*/ bMsg=0, up=0,pad_cnt=0,nopress=0,pad_cnt_acc=0;
	int retval;
	u16 *snes_image;
	int snesheight;
	int image_loaded;
	int current_smoothing;
	char path[MAXPATH], oldDir[MAXPATH], tmp[255], *p;
	int old_netplay=os9x_netplay;

	snes_image=(u16*)(0x44000000+512*272*2*2);

	filer_bg=(u16*)malloc(480*272*2);
	if (!filer_bg) {
		psp_msg(ERR_OUT_OF_MEM,MSG_DEFAULT);
		return -1;
	}

	filer_buildbg(1);

	strcpy(path, LastPath);

	if(FilerMsg[0])
		bMsg=1;
	getDir(path);
	
	//init jpeg stuff
	getDirJpeg();
	memset(jpeg_files,1,MAX_ENTRY);
	image_loaded=2;

	if (out[0]) { //check if a file was already selected
		for (i=0;i<nfiles;i++){
			if (!strcmp(out,files[i].d_name)) {sel=i;break;}
		}
	}

	pgDrawFrame(16,16,480-16,272-16,28|(28<<5)|(28<<10));

	while (get_pad()) pgWaitV();
  old_pad=0;
  
  sceKernelLibcGettimeofday( &filer_next, 0 );
  filer_next.tv_usec+=33*1000;
  
	for(;;){
		
		if (g_bSleep) {
#ifdef ME_SOUND			
			sceGuDisplay(0);		
//20080420 //
//			scePowerSetClockFrequency(66,66,33); //set to 66Mhz
#endif			
			while(g_bSleep) pgWaitVn(10);			//wait 16*10 ms
#ifdef ME_SOUND
//20080420 //
//			scePowerSetClockFrequency(222,222,111);
			sceGuDisplay(1);
#endif			
		}
		if (!g_bLoop) {retval=0;break;}
		
		for (;;) {
			sceKernelLibcGettimeofday( &filer_cur, 0 );
			if ( timercmp( &filer_next, &filer_cur, < ) ){
				break;
  		}
  	}
  	filer_next=filer_cur;
  	filer_next.tv_usec+=33*1000; //ms
  	while ( filer_next.tv_usec >= 1000000 ){
	  	filer_next.tv_sec += 1;
      filer_next.tv_usec -= 1000000;
	  }
		counter++;
		current_smoothing=3+round(sin(counter*3.14159/30)*3);
		new_pad=0;
    if (!pad_cnt) {
    	new_pad=get_pad();
    	nopress=!new_pad;
    	if (!new_pad) pad_cnt_acc=0;
    }
    else pad_cnt--;
    //pgWaitV();
		if (new_pad) {
			if (old_pad==new_pad) {
				if (pad_cnt_acc<6) {pad_cnt=1;pad_cnt_acc++;}
				else pad_cnt=0;
			}
			else {pad_cnt_acc=0;pad_cnt=5;}
		 	old_pad=new_pad;
		}
		if (!((cpt_lowbat++)&127)) os9x_lowbat=scePowerIsLowBattery();
		if (os9x_netplay!=old_netplay) {
			old_netplay=os9x_netplay;
			//recompute background
			filer_buildbg(1);

		}
		show_bg(filer_bg);

    if ((image_loaded==2)&&( nopress )) { //try new one if not keeping key pressed
    	if ((files[sel].d_stat.st_attr == TYPE_FILE)&&(jpeg_files[sel])) {
				char filename [MAXPATH + 1];
				char *fname;
				const char *src;
				char *dst = filename;
				int n;
				debug_log("check jpeg");
				src = SaveDir;
				while (*src) *dst++ = *src++;
				*dst++ = '/';
				fname = dst;
				src = files[sel].d_name;
				while (*src != '.' && *src) *dst++ = *src++;
				src = ".jpg";
				while ((*dst++ = *src++)) ;

				for (n=0;n<nfiles_jpeg;n++){
					if (!strcasecmp(fname, files_jpeg[n].d_name)) break;
				}
				if (n<nfiles_jpeg)
					image_loaded=os9x_loadsnap(filename,snes_image,&snesheight);
				else image_loaded=0;
				if (!image_loaded) jpeg_files[sel]=0;
			}
		}


		show_batteryinfo();
		show_usbinfo();


		if(new_pad & (PSP_CTRL_CIRCLE|PSP_CTRL_SQUARE)){
			int is_square=new_pad & PSP_CTRL_SQUARE;
			if(files[sel].d_stat.st_attr == TYPE_DIR){
				if(!strcmp(files[sel].d_name,"..")){  up=1; }
				else {
					if (files[sel].d_name[5])
						strcat(path,files[sel].d_name);
					else strcpy(path, files[sel].d_name);
					getDir(path);
					//init jpeg stuff
					getDirJpeg();
					memset(jpeg_files,1,MAX_ENTRY);
					image_loaded=2;
					
					
					sel=0;
					while (get_pad()) pgWaitV();	
				}
			}else{
						strcpy(out, path);
						strcat(out, files[sel].d_name);
						strcpy(LastPath,path);
				
						retval= (is_square?2:1);
						break;
			 }
			}		
        else if(new_pad & PSP_CTRL_CROSS)   { if (can_exit) {retval= 0;break;} }
        else if(new_pad & PSP_CTRL_TRIANGLE){ up=1;     }
        else if(new_pad & PSP_CTRL_UP)      { sel--;image_loaded=2;os9x_beep1();    }
        else if(new_pad & PSP_CTRL_DOWN)    { sel++;image_loaded=2;os9x_beep1();    }
        else if(new_pad & PSP_CTRL_LEFT)    { sel-=10;if (sel<0) sel=0;image_loaded=2;os9x_beep1(); }
        else if(new_pad & PSP_CTRL_RIGHT)   { sel+=10;if(sel >= nfiles) sel=nfiles-1;image_loaded=2;os9x_beep1();  }
#ifdef HOME_HOOK
        if( readHomeButton() > 0 )
        {
        		if (psp_msg(ASK_EXIT,MSG_DEFAULT)) {
        			S9xExit();
        		}
        }
#else
        else if(new_pad & PSP_CTRL_LTRIGGER) {
        	if (new_pad & PSP_CTRL_RTRIGGER) {
        		if (psp_msg(ASK_EXIT,MSG_DEFAULT)) {
        			S9xExit();
        		}
        	}
        }
#endif
        else if(new_pad & PSP_CTRL_SELECT){ 
        		if (psp_msg(ASK_DELETE,MSG_DEFAULT)) {
  						psp_msg(INFO_DELETING,MSG_DEFAULT);
  						strcpy(out, path);
							strcat(out, files[sel].d_name);
							strcpy(LastPath,path);
  						remove(out);
  						sel--;image_loaded=2; 
  						reload_entries=2;
  					}
  			} else if (new_pad & PSP_CTRL_START) {
  				os9x_netplay^=1;
  				pad_cnt=10;
  			}
        else if(new_pad & PSP_CTRL_RTRIGGER) {
#ifndef FW3X
        	os9x_usballowed=!os9x_usballowed;
        	if (os9x_usballowed) {
        		psp_msg(INFO_USB_ON,MSG_DEFAULT);
        		initUSBdrivers();
        	}
        	else {
        		psp_msg(INFO_USB_OFF,MSG_DEFAULT);
        		endUSBdrivers();
        	}
#endif
        }
		
		if(up){
			up=0;
			if(path[5]){
				p=strrchr(path,'/');
				*p=0;
				p=strrchr(path,'/');
				p++;
				strcpy(oldDir,p);
				strcat(oldDir,"/");
				*p=0;
				reload_entries=1;				
			}
		}
		if (reload_entries) {
				int old_sel=-1;
				if (reload_entries==2) old_sel=sel;
				reload_entries=0;
				getDir(path);
				
				//init jpeg stuff
				getDirJpeg();
				memset(jpeg_files,1,MAX_ENTRY);
				image_loaded=2;
				
				sel=0;
				for(i=0; i<nfiles; i++) {
					if(!strcmp(oldDir, files[i].d_name)) {
						sel=i;
						top=sel-3;
						break;
					}
				}
				if (old_sel!=-1) sel=old_sel;
		}					
		
		
		
		if(top > nfiles-rows)	top=nfiles-rows;
		if(top < 0)				top=0;
		if(sel >= nfiles)		sel=0;
		if(sel < 0)				sel=nfiles-1;
		if(sel >= top+rows)		top=sel-rows+1;
		if(sel < top)			top=sel;
		
        if(bMsg) {
          mh_print(1,0,FilerMsg,TITLE_COL);
        }
        else {
          mh_print(8,0,path,PATH_COL);
        }
	sprintf(tmp,
		psp_msg_string(can_exit ? FILER_STATUS_CANEXIT1 : FILER_STATUS_NOEXIT1),
		files[0].d_name[3] == ':' ? files[0].d_name : psp_msg_string(FILER_STATUS_PARDIR));
        mh_print(4, 262, tmp, INFOBAR_COL);
        	
		if(nfiles > rows){
			h = 219;
			pgDrawFrame(461,25,462,243,(0xd<<0)|(0xd<<5)|(0x17<<10));
			pgFillBox(465, h*top/nfiles + 25,477, h*(top+rows)/nfiles + 25,
				(0xd<<0)|(0xd<<5)|(0x17<<10));
			pgFillBox(465+1, h*top/nfiles + 25+1,477-1, h*(top+rows)/nfiles + 25 -1,
				(0x17<<0)|(0x17<<5)|(0x1f<<10));
		}
		
		x=8; y=17;
		for(i=0; i<rows; i++){
			if(top+i >= nfiles) break;
			if(top+i == sel) color = SEL_COL;
			else			 color = FILE_COL;
			if (files[top+i].d_stat.st_attr==TYPE_DIR){
				if (color==SEL_COL) color=SELDIR_COL;
				else color = DIR_COL;
			}
			if ((color==SEL_COL)||(color==SELDIR_COL)) mh_printSel_light(x,y,files[top+i].d_name,color,current_smoothing);//pgPrintSel(x, y, color, files[top+i].d_name);		
			else mh_print(x, y, files[top+i].d_name,color);//pgPrint(x, y, color, files[top+i].d_name);
			y+=1*10;
		}
		
		if (image_loaded==1) { //jpeg already loaded
			int x,y,xmax=128,ymax=snesheight/2;
			u16 *dst=(unsigned short*)pgGetVramAddr(0,0);
			for (y=0;y<ymax;y++)
				for (x=0;x<xmax;x++) {
					int col2a=snes_image[(y*2)*256+(x*2)];
					int col2b=snes_image[(y*2+1)*256+(x*2)];
					int col2c=snes_image[(y*2)*256+(x*2+1)];
					int col2d=snes_image[(y*2+1)*256+(x*2+1)];					
					int col2;
					col2=((((((col2a>>10)&31)+((col2b>>10)&31)+((col2c>>10)&31)+((col2d>>10)&31))>>2)/**2/3*/)<<10);
					col2|=((((((col2a>>5)&31)+((col2b>>5)&31)+((col2c>>5)&31)+((col2d>>5)&31))>>2)/**2/3*/)<<5);
					col2|=((((((col2a>>0)&31)+((col2b>>0)&31)+((col2c>>0)&31)+((col2d>>0)&31))>>2)/**2/3*/)<<0);
												
					dst[(y+17/*272-ymax-12*/)*512+x+480-xmax-24]=col2;//(b1<<10)|(g1<<5)|(r1);
				}
			pgDrawFrame(480-xmax-24-2,17-2,480-24+2,17+ymax+2,12|(12<<5)|(12<<10));
		  pgDrawFrame(480-xmax-24-1,17-1,480-24+1,17+ymax+1,30|(30<<5)|(30<<10));
		}

		pgScreenFlipV2();
		if(psp_ExitCheck()) {retval= -1;break;}
	}
	
	while (get_pad()) pgWaitV();	
	
	free(filer_bg);
	
	return retval;
}



int getNoExtFilePath(char *out,int can_exit) {
//    int counter=0;
	static int cpt_lowbat=0;
	unsigned long color=RGB_WHITE;
	static int sel=0;
	int rows=24, top=0, x, y, h, i, /*len,*/ bMsg=0, up=0,pad_cnt=5,pad_cnt_acc=0;
	int retval;
	int current_smoothing;
	int cnt = 0;
	char path[MAXPATH], oldDir[MAXPATH], tmp[255], *p;

	filer_bg=(u16*)malloc(480*272*2);
	if (!filer_bg) {
		psp_msg(ERR_OUT_OF_MEM,MSG_DEFAULT);
		return -1;
	}
	filer_buildbg(0);

	strcpy(path, LastPath);

	if(FilerMsg[0])
		bMsg=1;


	getDirNoExt(path);


	pgDrawFrame(16,16,480-16,272-16,28|(28<<5)|(28<<10));

	while (get_pad()) pgWaitV();
  old_pad=0;

  sceKernelLibcGettimeofday( &filer_next, 0 );
  filer_next.tv_usec+=33*1000;

	for(;;){
		current_smoothing=3+round(sin(cnt*3.14159/30)*3);
		cnt++;

		if (g_bSleep) {
#ifdef ME_SOUND
			sceGuDisplay(0);
//20080420
//			scePowerSetClockFrequency(66,66,33); //set to 66Mhz
#endif
			while(g_bSleep) pgWaitVn(10);			//wait 16*10 ms
#ifdef ME_SOUND						
//20080420
//			scePowerSetClockFrequency(222,222,111);
			sceGuDisplay(1);
#endif			
		}
		if (!g_bLoop) {retval=0;break;}
		
		for (;;) {
			sceKernelLibcGettimeofday( &filer_cur, 0 );
			if ( timercmp( &filer_next, &filer_cur, < ) ){
				break;  		
  		}
  	}
  	filer_next=filer_cur;
  	filer_next.tv_usec+=33*1000; //ms
  	while ( filer_next.tv_usec >= 1000000 ){
	  	filer_next.tv_sec += 1;
      filer_next.tv_usec -= 1000000;
	  }
		
		if (!((cpt_lowbat++)&127)) os9x_lowbat=scePowerIsLowBattery();
		
		show_bg(filer_bg);
  	
		show_batteryinfo();
		show_usbinfo();
				                  		
		new_pad=0;
    if (!pad_cnt) {    	
    	new_pad=get_pad();    	
    	if (!new_pad) pad_cnt_acc=0;
    }
    else pad_cnt--;
    //pgWaitV();        
		if (new_pad) {
			if (old_pad==new_pad) {				
				if (pad_cnt_acc<6) {pad_cnt=1;pad_cnt_acc++;}
				else pad_cnt=0;
			}
			else {pad_cnt_acc=0;pad_cnt=5;}
		 	old_pad=new_pad;
		}
        				        
		if(new_pad & PSP_CTRL_CIRCLE){
			if(files[sel].d_stat.st_attr == TYPE_DIR){
				if(!strcmp(files[sel].d_name,"..")){  up=1; }
                else{
					if (files[sel].d_name[5]) {
						strcat(path, files[sel].d_name);
					} else strcpy(path, files[sel].d_name);
					getDirNoExt(path);					
					sel=0;
					while (get_pad()) pgWaitV();	
				}
			}else{
						strcpy(out, path);
						strcat(out, files[sel].d_name);
						//strcpy(LastPath,path);
						retval= 1;
						break;
			 }
			}		
        else if(new_pad & PSP_CTRL_CROSS)   { if (can_exit) {retval= 0;break;} }
        else if(new_pad & PSP_CTRL_TRIANGLE){ up=1;os9x_beep1();}
        else if(new_pad & PSP_CTRL_UP)      { sel--;os9x_beep1();}
        else if(new_pad & PSP_CTRL_DOWN)    { sel++;os9x_beep1();}
        else if(new_pad & PSP_CTRL_LEFT)    { sel-=10;if (sel<0) sel=0;os9x_beep1();}
        else if(new_pad & PSP_CTRL_RIGHT)   { sel+=10;if(sel >= nfiles) sel=nfiles-1;os9x_beep1();}
        else if(new_pad & PSP_CTRL_LTRIGGER) {
        	if (new_pad & PSP_CTRL_RTRIGGER) {
        		if (psp_msg(ASK_EXIT,MSG_DEFAULT)) {
        			S9xExit();
        		}
        	}
        }
        else if(new_pad & PSP_CTRL_RTRIGGER) {
#ifndef FW3X
        	os9x_usballowed=!os9x_usballowed;
        	if (os9x_usballowed) initUSBdrivers();
        	else endUSBdrivers();
#endif
        }
		
		if(up){
			if(path[5]){
				p=strrchr(path,'/');
				*p=0;
				p=strrchr(path,'/');
				p++;
				strcpy(oldDir,p);
				strcat(oldDir,"/");
				*p=0;
				getDirNoExt(path);
				
				sel=0;
				for(i=0; i<nfiles; i++) {
					if(!strcmp(oldDir, files[i].d_name)) {
						sel=i;
						top=sel-3;
						break;
					}
				}
			}
			up=0;
		}
		
		
		
		if(top > nfiles-rows)	top=nfiles-rows;
		if(top < 0)				top=0;
		if(sel >= nfiles)		sel=0;
		if(sel < 0)				sel=nfiles-1;
		if(sel >= top+rows)		top=sel-rows+1;
		if(sel < top)			top=sel;
		
        if(bMsg) {
          mh_print(1,0,FilerMsg,TITLE_COL);
        }
        else {
          mh_print(8,0,path,PATH_COL);
        }
        
	sprintf(tmp,
		psp_msg_string(can_exit ? FILER_STATUS_CANEXIT2 : FILER_STATUS_NOEXIT2),
		files[0].d_name[3] == ':' ? files[0].d_name : psp_msg_string(FILER_STATUS_PARDIR));
        mh_print(8, 262, tmp, INFOBAR_COL);

		// スクロールバー
		if(nfiles > rows){
			h = 219;
			pgDrawFrame(461,25,462,243,(0xd<<0)|(0xd<<5)|(0x17<<10));
			pgFillBox(465, h*top/nfiles + 25,477, h*(top+rows)/nfiles + 25,
				(0xd<<0)|(0xd<<5)|(0x17<<10));
			pgFillBox(465+1, h*top/nfiles + 25+1,477-1, h*(top+rows)/nfiles + 25 -1,
				(0x17<<0)|(0x17<<5)|(0x1f<<10));

		}
		
		x=8; y=17;
		for(i=0; i<rows; i++){
			if(top+i >= nfiles) break;
			if(top+i == sel) color = SEL_COL;
			else			 color = FILE_COL;
			if (files[top+i].d_stat.st_attr==TYPE_DIR){
				if (color==SEL_COL) color=SELDIR_COL;
				else color = DIR_COL;
			}
			if ((color==SEL_COL)||(color==SELDIR_COL)) mh_printSel_light(x,y,files[top+i].d_name,color,current_smoothing);//pgPrintSel(x, y, color, files[top+i].d_name);		
			else mh_print(x, y, files[top+i].d_name,color);//pgPrint(x, y, color, files[top+i].d_name);
			y+=1*10;
		}
				
		pgScreenFlipV2();
		if(psp_ExitCheck()) {retval= -1;break;}
	}
	
	while (get_pad()) pgWaitV();	
	
	free(filer_bg);
	
	
	return retval;
}


int filer_init(char*msg,char*path)
{
    strcpy(FilerMsg,msg);            
    strcpy(LastPath,path);
    memset(files,0,sizeof(files));
    return 1;
}
