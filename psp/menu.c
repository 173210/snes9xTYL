//#include "main.h"

 

#define SNAPSHOT_MENU

#define PANEL_TEXTCMD_COL ((31)|(28<<5)|(31<<10))
#define PANEL_BUTTONCMD_COL ((20)|(31<<5)|(18<<10))

#define MENU_CHGVAL() \
	os9x_beep2();menu_modified=1;
	
#ifdef SNAPSHOT_MENU   
#define SNAPSHOT_CODE() \	
    else if (new_pad & PSP_CTRL_START) {    	\
    	pg_drawframe^=1;    	\
    	sprintf(str_tmp,"%ssnap%d.jpg",LaunchDir,snap++);\
    	write_JPEG_file ((char*)str_tmp,75,pgGetVramAddr(0,0),480,272,512); \
    	pg_drawframe^=1;    	\
    }
#else    
	#define SNAPSHOT_CODE()
#endif


#include "psp.h"

#include "psputility_osk.h"
#include "danzeff.h"

#include "blitter.h"
#include "snes9x.h"
#include "cheats.h"
#include <math.h>
#include "unzip.h"

#include "os9xZ_openspc.h"

#include "img_jpeg.h"

#include "help_data.c"

#include "menu_fx.h"

#define TITLE_COL ((31)|(26<<5)|(31<<10))
#define PATH_COL ((31)|(24<<5)|(28<<10))

#define FILE_COL ((20)|(20<<5)|(31<<10))
#define DIR_COL ((8)|(31<<5)|(8<<10))

#define SEL_COL ((30)|(30<<5)|(31<<10))
#define SELDIR_COL ((28)|(31<<5)|(28<<10))

#define CHEATS_ACTIVE_COL ((31)|(24<<5)|(28<<10))
#define CHEATS_DISABLED_COL ((16)|(12<<5)|(14<<10))

#define INFOBAR_COL ((31)|(24<<5)|(20<<10))


#define MENU_VAL_COL	((28)|(31<<5)|(26<<10))

#define CODE_COL		((4)|(31<<5)|(4<<10))
#define CODE2_COL		((8)|(28<<5)|(8<<10))
#define CODE3_COL		((4)|(12<<5)|(4<<10))
#define GFX_COL		  ((26)|(26<<5)|(31<<10))
#define GFX2_COL		  ((28)|(28<<5)|(31<<10))
#define GFX3_COL		  ((8)|(8<<5)|(12<<10))
#define GREETINGS0_COL ((31)|(27<<5)|(27<<10))		 		
#define GREETINGS_COL ((20)|(20<<5)|(20<<10))

extern volatile int g_bSleep,g_bLoop;

extern IMAGE *bg_img;
extern int bg_img_mul,bg_img_num;
extern int pg_drawframe;
extern int os9x_language,os9x_timezone,os9x_autosavesram,os9x_daylsavings;
extern int os9x_screenLeft,os9x_screenTop,os9x_screenHeight,os9x_screenWidth;
extern u16 os9x_savestate_mini[128*120];
extern u32 new_pad,old_pad;
extern int os9x_vsync,os9x_padindex,os9x_usballowed;
extern int os9x_BG0,os9x_BG1,os9x_BG2,os9x_BG3,os9x_OBJ,os9x_easy,os9x_fastsprite,os9x_applyhacks,os9x_hack;
extern int os9x_smoothing,os9x_softrendering,os9x_apuenabled,os9x_cpuclock,os9x_fskipvalue,os9x_render,os9x_speedlimit,os9x_sndfreq;
extern int os9x_showfps,os9x_showpass,os9x_getnewfile,os9x_gammavalue,os9x_snesheight,os9x_forcepal_ntsc;
extern int os9x_lowbat,os9x_autosavetimer,os9x_menumusic,os9x_menufx,os9x_menupadbeep;
extern char LaunchDir[256];
extern char romPath[256];
extern char lastRom[256];
extern char LastPath[256];
extern char str_tmp[256];
extern char os9x_nickname[256];
extern int os9x_inputs[32],os9x_inputs_analog,os9x_fpslimit,os9x_apu_ratio;
int exit_menu,menu_modified,cheats_modified;
int menu_music;
int menu_musiclen;
uint8 *menu_musicdata;
int menu_musicidx=-1;

extern char os9x_viewfile_path[256];

extern int ExitCallback(int arg1, int arg2, void *common);
extern void os9x_savecheats();
extern int scroll_message(char **msg_lines,int lines,int start_pos,int exit_only_at_end,char *title);
extern struct SCheatData Cheat;

int state_slot=0;
int slot_occupied;
int menu_cnt,menu_cnt2,menu_cnt3,pad_cnt,pad_cnt_acc,menu_panel_pos,menu_scrolling;
int menu_free_ram;
u16 *menu_bg;
int menu_music_panel_pos,menu_music_panel_mode,menu_music_panel_size;
char menu_music_songname[33],menu_music_gametitle[33],menu_music_author[33];

int menu_current_smoothing=0;
int menu_current_smoothing_icon=0;

int snap=0;

uint32 g_ROMCRC32;

void show_bg(u16 *bg);
void menu_inputName(char *name);
void show_batteryinfo(void);
void show_usbinfo(void);

void menu_alertmsg(char *msg) {
	int x,cx,co1,co1b,co2,co2b,co3,co3b;
	if (!msg) return;	
	pgFillBoxHalfer(13,13,13+5+mh_length(msg)+5,13+5+20+5);
	
	co1=16+(round(sin((menu_cnt+50*2/3)*3.14159/50)*15));
	co1b=co1>>1;	
	co2=16+(round(sin((menu_cnt+50*4/3)*3.14159/50)*15));
	co2b=co2>>1;
	co3=16+(round(sin(menu_cnt*3.14159/50)*15));
	co3b=co3>>1;
	
	
	co1=(co1<<10)|(co1<<5)|co1;
	co1b=(co1b<<10)|(co1b<<5)|co1b;
	co2=(co2<<10)|(co2<<5)|co2;
	co2b=(co2b<<10)|(co2b<<5)|co2b;
	co3=(co3<<10)|(co3<<5)|co3;
	co3b=(co3b<<10)|(co3b<<5)|co3b;
	x=mh_length(msg);
	cx=x/2;
	pgDrawFrame(13+5+cx-x/2,13+5+13,13+5+cx+x/2,13+5+13,co1);
	pgDrawFrame(13+5+cx-x/2,13+5+13+1,13+5+cx+x/2,13+5+13+1,co1b);
	pgDrawFrame(13+5+cx-x/2,13+5+13+3,13+5+cx+x/2,13+5+13+3,co2);
	pgDrawFrame(13+5+cx-x/2,13+5+13+4,13+5+cx+x/2,13+5+13+4,co2b);
	pgDrawFrame(13+5+cx-x/2,13+5+13+6,13+5+cx+x/2,13+5+13+6,co3);
	pgDrawFrame(13+5+cx-x/2,13+5+13+7,13+5+cx+x/2,13+5+13+7,co3b);
	
	mh_print_light(13+5,13+5,msg,31|(31<<5)|(31<<10),menu_current_smoothing);
}

int menu_screencalibrate(char *mode){
	u16 *snes_image;
	SceCtrlData paddata,oldpaddata;
	char st[100];
	int exit_calib=0;
	
	if (mode) {mode[0]=0; return 0;}
	
	blit_reinit();
		
	show_bg(menu_bg);
	show_batteryinfo();
	show_usbinfo();
  pgScreenFlipV2();  
  show_bg(menu_bg);  
  show_batteryinfo();   
  show_usbinfo();
  pgScreenFlipV2();
	
	while (get_pad()) pgWaitV();
	
	if (os9x_softrendering<2)	snes_image=(u16*)(0x44000000+512*272*2*2);
	else snes_image=(u16*)(0x44000000+2*512*272*2+256*240*2+2*256*256*2);			
	
	sceCtrlPeekBufferPositive(&paddata, 1);
	pad_cnt=8;
	pad_cnt_acc=0;
	while (!exit_calib) {
		
		
		switch (os9x_render) {
			case 0:
				//could be faster since no stretch, use copyimage
				guClear(256,os9x_snesheight);
				guDrawBuffer(snes_image,256,os9x_snesheight,256,256,os9x_snesheight);			
				break;
			case 1:	
				guClear(256*272/os9x_snesheight,272);
				guDrawBuffer(snes_image,256,os9x_snesheight,256,256*272/os9x_snesheight,272);
				break;
			case 2:	
				guClear(272*4/3,272);
				guDrawBuffer(snes_image,256,os9x_snesheight,256,272*4/3,272);
				break;
			case 3:
				guClear(320*272/os9x_snesheight,272);
				guDrawBuffer(snes_image,256,os9x_snesheight,256,320*272/os9x_snesheight,272);
				break;
			case 4:
				guClear(480,272);
				guDrawBuffer(snes_image,256,os9x_snesheight,256,480,272);
				break;
			case 5:
				guClear(480,272);
				guDrawBuffer(snes_image,256,os9x_snesheight-16,256,480,272);
				break;
		}
		sprintf(st,psp_msg_string(MENU_STATUS_VIDEO_SCRCALIB));
		mh_print(8-1,262-1,st,(8<<10)|(8<<5)|(16<<0));
		mh_print(8,262,st,(29<<10)|(29<<5)|(31<<0));

		//wait for a press

			oldpaddata.Buttons=paddata.Buttons;
			oldpaddata.Lx=paddata.Lx;
			oldpaddata.Ly=paddata.Ly;
			while (1) {
				sceCtrlPeekBufferPositive(&paddata, 1);
				if (paddata.Buttons) break;
				if (paddata.Lx<0x30) break;
				if (paddata.Ly<0x30) break;
				if (paddata.Lx>0xA0) break;
				if (paddata.Ly>0xA0) break;
				pad_cnt=8;
				pad_cnt_acc=0;
				pgWaitV();
			}
						
			//wait for release
			//while (get_pad()) pgWaitV();
			pgWaitVn(pad_cnt);
			//input scan
			
			if ((oldpaddata.Buttons==paddata.Buttons)&&((oldpaddata.Lx>>2)==(paddata.Lx>>2))&&((oldpaddata.Ly>>2)==(paddata.Ly>>2))) {
				if (pad_cnt_acc<8) {pad_cnt=4;pad_cnt_acc++;}
				else if (pad_cnt_acc<40) {pad_cnt=2;pad_cnt_acc++;}
				else pad_cnt=0;
  		} else {pad_cnt_acc=0;pad_cnt=8;}
			
			//now analyse press
			if (paddata.Buttons & PSP_CTRL_UP) {os9x_screenTop--;menu_modified=1;}
			else if (paddata.Buttons & PSP_CTRL_DOWN) {os9x_screenTop++;menu_modified=1;}
			else if (paddata.Buttons & PSP_CTRL_LEFT) {os9x_screenLeft--;menu_modified=1;}
			else if (paddata.Buttons & PSP_CTRL_RIGHT) {os9x_screenLeft++;menu_modified=1;}
			else if (paddata.Ly<0x30) {os9x_screenHeight--;menu_modified=1;}
			else if (paddata.Ly>0xA0) {os9x_screenHeight++;menu_modified=1;}
			else if (paddata.Lx<0x30) {os9x_screenWidth--;menu_modified=1;}
			else if (paddata.Lx>0xA0) {os9x_screenWidth++;menu_modified=1;}
			else if (paddata.Buttons & PSP_CTRL_TRIANGLE) {
				os9x_screenLeft=os9x_screenTop=os9x_screenWidth=os9x_screenHeight=0;
				menu_modified=1;
			}
			else if (paddata.Buttons & PSP_CTRL_CROSS) exit_calib=1;

			if (os9x_screenWidth<-128) {os9x_screenWidth=-128;}
			if (os9x_screenWidth>128) {os9x_screenWidth=128;}
			if (os9x_screenHeight<-128) {os9x_screenHeight=-128;}
			if (os9x_screenHeight>128) {os9x_screenHeight=128;}
			if (os9x_screenTop<-128) {os9x_screenTop=-128;}
			if (os9x_screenTop>128) {os9x_screenTop=128;}
			if (os9x_screenLeft<-128) {os9x_screenLeft=-128;}
			if (os9x_screenLeft>128) {os9x_screenLeft=128;}
	}	
	while (get_pad()) pgWaitV();
	
	fx_init();
	
	return 0;
}

int menu_exitemu(char *mode){	
	if (mode) {mode[0]=0;return 0;}
					
	if (!(psp_msg(ASK_EXIT,MSG_DEFAULT))) return 0;
	StopSoundThread();	
	ExitCallback(0,0,NULL);
	return 1;
}

int loadstate(){
  char ext[10];
  
  if (slot_occupied) {
  	if (!psp_msg(MENU_STATE_CONFIRMLOAD,MSG_DEFAULT)) return 0;
  	psp_msg(MENU_STATE_ISLOADING,MSG_DEFAULT);
  	if (state_slot==10) strcpy(ext,".zat");
  	else {
  		strcpy(ext,".za0");
  		ext[3]=state_slot+48;
  	}
  	os9x_load(ext);
  	return 1;
  }
  psp_msg(MENU_STATE_NOSTATE,MSG_DEFAULT);
  return 0;
}

int deletestate(){
  char ext[10];
  
  if (slot_occupied) {
  	if (!psp_msg(MENU_STATE_CONFIRMDELETE,MSG_DEFAULT)) return 0;
  	psp_msg(MENU_STATE_ISDELETING,MSG_DEFAULT);
  	if (state_slot==10) strcpy(ext,".zat");
  	else {
  		strcpy(ext,".za0");
  		ext[3]=state_slot+48;
  	}
  	os9x_remove(ext);
  	return 1;
  }
  psp_msg(MENU_STATE_NOSTATE,MSG_DEFAULT);
  return 0;
}

int savestate(){
  char ext[10];
  if (os9x_lowbat) return;
  if (slot_occupied) {
  	if (!psp_msg(MENU_STATE_CONFIRMSAVE,MSG_DEFAULT)) return 0;
  }
  psp_msg(MENU_STATE_ISSAVING,MSG_DEFAULT);
  if (state_slot==10) strcpy(ext,".zat");
  else {
  	strcpy(ext,".za0");
  	ext[3]=state_slot+48;
  	os9x_save(ext);
  }
  return 1;
}

int menu_reset(char *mode){	
	if (mode) {mode[0]=0;return 0;}
	if (!psp_msg(MENU_GAME_CONFIRMRESET,MSG_DEFAULT)) return 0;
	if (!os9x_lowbat) os9x_savesram();
  S9xReset();
  return 1;
}

int menu_browser(char *mode){
	if (mode) {mode[0]=0;return 0;}
	os9x_getnewfile=1;
	return 1;
}

int menu_snapshot(char *mode) {
	if (mode) {mode[0]=0;return 0;}
	psp_msg(MENU_MISC_SAVINGJPEG,MSG_DEFAULT);
	if (!os9x_lowbat) os9x_savesnap();
	return 0;
}

int menu_importstate(char *mode) {
	char statefilename[256];
	if (mode) {mode[0]=0;return 0;}

	if (getNoExtFilePath(statefilename,1)==1) {
		debug_log(statefilename);
		if (!psp_msg(MENU_STATE_CONFIRMLOAD,MSG_DEFAULT)) return 0;
		psp_msg(MENU_STATE_ISIMPORTING,MSG_DEFAULT);
		return os9x_loadfname(statefilename);
	}
	return 0;
}

int menu_exportS9Xstate(char *mode) {
	if (mode) {mode[0]=0;return 0;}

  if (os9x_lowbat) return;
  psp_msg(MENU_STATE_ISEXPORTINGS9X,MSG_DEFAULT);
  os9x_S9Xsave(".000");
  return 0;
}


int viewfile_pos=0;
char viewfile_oldfilename[256];
char lastpath_save[256];
char filename[256];	
int menu_viewfile(char *mode) {
	FILE *f;
	char *txtdata,*p;
	int fsize;
	int i,j,lines,newlines,dosmode;
	char **msg_lines;
	char *pszExt;			
	
	if (mode) {mode[0]=0;return 0;}
	
	strncpy(lastpath_save,LastPath,255);
	strncpy(LastPath,os9x_viewfile_path,255);
	if (getNoExtFilePath(filename,1)==1) {
		
		if((pszExt = strrchr(filename, '.'))) {
			pszExt++;
			if (!strcasecmp("txt",pszExt)) {
				//read file
				
				f=fopen(filename,"rb");
				if (!f) return 0;
				fseek(f,0,SEEK_END);
				fsize=ftell(f);
				fseek(f,0,SEEK_SET);
				txtdata=(char*)malloc(fsize+2);
				if (!txtdata) {
					fclose(f);
					psp_msg(ERR_OUT_OF_MEM,MSG_DEFAULT);
					return 0;
				}
				fread(txtdata,1,fsize,f);
				fclose(f);
				txtdata[fsize]=0;
				txtdata[fsize+1]=1;
				
				p=txtdata;
				// 'lineify' it
				lines=0;		
				dosmode=0;
				while (*p) {
					i=0;
					//get new line
					while ((p[i]!=0x0D)&&(p[i]!=0x0A)&&(p[i])) i++;
					//if line carriage return, skip it & put a '0' / end of string 
					if (p[i]==0x0D) {//DOS or MAC
						p[i]=0;i++;
						if (p[i]==0x0A) {//DOS
							p[i]=0; //0x0D 0x0A
							i++;
							dosmode=1;
						}
					}
					if (p[i]==0x0A) {//UNIX
						p[i]=0;i++;
					}
					lines++;
					p=p+i;
				}
				
				p=txtdata;
				//1st pass, check lines for too long ones
				newlines=0;
				for (i=0;i<lines;i++){
					if (strlen(p)) {						
						int pos;
						char *q=p;
						//while lines too long, newline
						while (mh_length(q)>480) {
							pos=mh_trimlength(q);
							q=q+pos;
							newlines++;							
						}
					}
					p=p+strlen(p)+(dosmode?2:1);
				}
				if (newlines) {
					char *newtxtdata;
					newtxtdata=(char*)malloc(fsize+2+newlines);
					if (!newtxtdata) {
						psp_msg(ERR_OUT_OF_MEM,MSG_DEFAULT);
						return 0;
					}
					memcpy(newtxtdata,txtdata,fsize+2);
					free(txtdata);
					txtdata=newtxtdata;
				}
				
				lines+=2+newlines;
				msg_lines=(char**)malloc(lines*sizeof(char*));
				if (!msg_lines) {
					free(txtdata);
					psp_msg(ERR_OUT_OF_MEM,MSG_DEFAULT);	
					return 0;
				}
				msg_lines[0]=msg_lines[1]=NULL;				
				p=txtdata;				
				for (i=2;i<lines;i++){
					if (strlen(p)) {
						if (mh_length(p)<=480) {
							msg_lines[i]=p;							
							p=p+strlen(p)+(dosmode?2:1);							
						} else {
							int pos;
							char *r,*q=p;								
							//while lines too long, newline
							while (mh_length(q)>480) {
								pos=mh_trimlength(q);
								msg_lines[i++]=q;
								q=q+pos;
								r=q;
								//1st, go to the end
								for (;;) {
									if ((*r==0)&&(*(r+1)==1)) break;
									r++;
								}
								r+=2;
								//now move
								for (;;) {									
									*r=*(r-1);
									r--;
									if (r==q) break;									
								}															
								*q=0;
								q++;
							}
							if (strlen(q)) {
								msg_lines[i]=q;								
								p=q+strlen(q)+(dosmode?2:1);
							} else {
								msg_lines[i]=NULL;
								p=p+(dosmode?2:1);
							}
						}
					}	else {
						msg_lines[i]=NULL;					
						p=p+(dosmode?2:1);
					}
				}
				
				if (viewfile_pos) {//help to init oldfilename first time
					if (strcmp(viewfile_oldfilename,filename)) viewfile_pos=0; //new file, reset pos
				}
				strcpy(viewfile_oldfilename,filename);
				strncpy(os9x_viewfile_path,LastPath,255);
				
				p=strrchr(filename,'/');
				if (!p) p=filename;
				else p=p+1;


				viewfile_pos=scroll_message(msg_lines,lines,viewfile_pos,0,p);


				//free 'linified' message			
				free(txtdata);
				free(msg_lines);
				fx_init();
			}
		}
	}
	strncpy(LastPath,lastpath_save,255);
	return 0;
}

int menu_savedefaultsetting(char *mode){
	if (mode) {mode[0]=0;return 0;}

	psp_msg(MENU_GAME_SAVINGDEFAULTSETTINGS,MSG_DEFAULT);
	if (!os9x_lowbat) save_rom_settings(0,"default");
	return 0;
}


typedef struct {
		char label[64];
		s32	help_index;
		int (*menu_func)(char *mode);
		int	*value_int;
		int	values_list_size;
		int values_list[30];
		int value_index;	
		char *values_list_label[30];
} menu_time_t;


int os9x_ignore_fixcol,os9x_ignore_winclip,os9x_ignore_addsub,os9x_ignore_palwrite,os9x_gfx_fastmode7,os9x_fix_hires;
int os9x_apufix;int os9x_old_accel;
extern int os9x_SA1_exec;
#define DEBUGMENU_ITEMS 20

menu_time_t os9xpsp_debugmenu[DEBUGMENU_ITEMS]={	
	{"Show rendering passes : ",10,NULL,&os9x_showpass,2,{0,1},0,{"Off","On"}},
	{"--------",-1,NULL,NULL,0,NULL,0,NULL},
	{"Speed hacks : ",11,NULL,&os9x_applyhacks,2,{0,1},0,{"Off","On"}},
	{"--------",-1,NULL,NULL,0,NULL,0,NULL},
	{"Ignore Fixed Colour : ",12,NULL,&os9x_ignore_fixcol,2,{0,1},0,{"Off","On"}},
	{"Ignore Windows clipping : ",13,NULL,&os9x_ignore_winclip,2,{0,1},0,{"Off","On"}},
	{"Ignore Add/Sub modes : ",14,NULL,&os9x_ignore_addsub,2,{0,1},0,{"Off","On"}},
	{"Ignore Palette writes : ",15,NULL,&os9x_ignore_palwrite,2,{0,1},0,{"Off","On"}},
	{"Simple Palette writes : ",-1,NULL,&os9x_fix_hires,2,{0,1},0,{"Off","On"}},
	{"Old PSP accel.: ",12,NULL,&os9x_old_accel,2,{0,1},0,{"Off","On"}},
	{"--------",-1,NULL,NULL,0,NULL,0,NULL},	
	{"No Transparency : ",22,NULL,&os9x_easy,2,{0,1},0,{"Off","On"}},	
	{"Fast sprites : ",23,NULL,&os9x_fastsprite,2,{0,1},0,{"Off","On"}},	
	{"--------",-1,NULL,NULL,0,NULL,0,NULL},
	{"OBJ : ",17,NULL,&os9x_OBJ,2,{0,1},0,{"Off","On"}},
	{"BG0 : ",18,NULL,&os9x_BG0,2,{0,1},0,{"Off","On"}},	
	{"BG1 : ",19,NULL,&os9x_BG1,2,{0,1},0,{"Off","On"}},	
	{"BG2 : ",20,NULL,&os9x_BG2,2,{0,1},0,{"Off","On"}},	
	{"BG3 : ",21,NULL,&os9x_BG3,2,{0,1},0,{"Off","On"}},
	{"SA1 Option : ",-1,NULL,&os9x_SA1_exec,9,{1,2,3,4,5,6,7,8,9},1,{"1","2","3","4","5","6","7","8","9"}}	
};


int show_debugmenu(char *mode) {
	//int counter=0;
	unsigned long color=RGB_WHITE;
	static int sel=0;
	int rows=28,x, y, h, i,j,top=0;
	int cpt;
	int retval;		
	
	if (mode) {mode[0]=0; return 0;}
				
	while (get_pad()) pgWaitV();	
	
	os9x_ignore_fixcol=(os9x_hack&PPU_IGNORE_FIXEDCOLCHANGES?1:0);
	os9x_ignore_winclip=(os9x_hack&PPU_IGNORE_WINDOW?1:0);
	os9x_ignore_addsub=(os9x_hack&PPU_IGNORE_ADDSUB?1:0);
	os9x_ignore_palwrite=(os9x_hack&PPU_IGNORE_PALWRITE?1:0);
	os9x_gfx_fastmode7=(os9x_hack&GFX_FASTMODE7?1:0);
	os9x_apufix=(os9x_hack&APU_FIX?1:0);
	os9x_fix_hires=(os9x_hack&HIRES_FIX?1:0);
	os9x_old_accel=(os9x_hack&OLD_PSP_ACCEL?1:0);
	
	//init menu indexes
			
	for (j=0;j<DEBUGMENU_ITEMS;j++) {
		if (os9xpsp_debugmenu[j].value_int) {			//menu entry has a variable
			int cur=*(os9xpsp_debugmenu[j].value_int);    //current value
			for (i=0;i<os9xpsp_debugmenu[j].values_list_size;i++){ //browse list to find index
    		if (cur==os9xpsp_debugmenu[j].values_list[i]) {    			
    			os9xpsp_debugmenu[j].value_index=i;
    			break;
    		}
      }
    }
	}		
	
	
	
	old_pad=0;
	cpt=0;
	for(;;){		
		show_bg(menu_bg);
    show_batteryinfo();       
    show_usbinfo();
        	
		new_pad=0;
    if (!pad_cnt) new_pad=get_pad();
    else pad_cnt--;
    pgWaitV();        
		if (new_pad) {
			if (old_pad==new_pad) pad_cnt=1/*2*/;
			else pad_cnt=5;
		 	old_pad=new_pad;
		}
		    
		if(new_pad & PSP_CTRL_CIRCLE){
			if (os9xpsp_debugmenu[sel].menu_func) 
				if ((*os9xpsp_debugmenu[sel].menu_func)(0)) {retval=0;break;}
		} else if(new_pad & PSP_CTRL_TRIANGLE)   {
			if (os9xpsp_debugmenu[sel].help_index>=0) {
				inputBoxOK(help_data[os9x_language][os9xpsp_debugmenu[sel].help_index]);
			}
		} else if(new_pad & PSP_CTRL_CROSS)   { retval= 0;break; }   
    else if(new_pad & PSP_CTRL_UP)      { sel--;os9x_beep1();    }
    else if(new_pad & PSP_CTRL_DOWN)    { sel++;os9x_beep1();    }
    else if(new_pad & PSP_CTRL_LTRIGGER)      { sel-=10;if (sel<0) sel=0;os9x_beep1();    }
    else if(new_pad & PSP_CTRL_RTRIGGER)    { sel+=10;if (sel>=DEBUGMENU_ITEMS) sel=DEBUGMENU_ITEMS-1;os9x_beep1();    }
    else if(new_pad & PSP_CTRL_LEFT)    {
    	if (os9xpsp_debugmenu[sel].value_int){
    		MENU_CHGVAL();
    		if (os9xpsp_debugmenu[sel].value_index>0) os9xpsp_debugmenu[sel].value_index--;
    		*(os9xpsp_debugmenu[sel].value_int)=os9xpsp_debugmenu[sel].values_list[os9xpsp_debugmenu[sel].value_index];    		
    	}
    }    
    else if(new_pad & PSP_CTRL_RIGHT)    {
    	if (os9xpsp_debugmenu[sel].value_int){
    		MENU_CHGVAL();
    		if (os9xpsp_debugmenu[sel].value_index<os9xpsp_debugmenu[sel].values_list_size-1) os9xpsp_debugmenu[sel].value_index++;
    		*(os9xpsp_debugmenu[sel].value_int)=os9xpsp_debugmenu[sel].values_list[os9xpsp_debugmenu[sel].value_index];    		
    	}
    } SNAPSHOT_CODE()
    

		
		if(top > DEBUGMENU_ITEMS-rows)	top=DEBUGMENU_ITEMS-rows;
		if(top < 0)				top=0;
		if(sel >= DEBUGMENU_ITEMS)		sel=0;
		if(sel < 0)				sel=DEBUGMENU_ITEMS-1;
		if(sel >= top+rows)		top=sel-rows+1;
		if(sel < top)			top=sel;
		
       
    //pgPrint(1,0,TITLE_COL,"[" EMUNAME_VERSION "] - Menu");
    //pgPrint(1,33,INFOBAR_COL,"\1 OK \2 Main menu \5,\7 to select ,\6,\b to change value");
    //pgPrint(1,0,TITLE_COL,"[" EMUNAME_VERSION "] - Menu");
    mh_print(8,0,psp_msg_string(MENU_TITLE_MISC_HACKDEBUG),TITLE_COL);
    //pgPrint(1,33,INFOBAR_COL,"\1 OK \2 Return game \5,\7 to select ,\6,\b to change value");
    {
    	char status_bar[100];
    	if (os9xpsp_debugmenu[sel].menu_func) strcpy(status_bar,psp_msg_string(MENU_STATUS_MISC_HACKDEBUG_FUNC));
    	else strcpy(status_bar,psp_msg_string(MENU_STATUS_MISC_HACKDEBUG));
    	if (os9xpsp_debugmenu[sel].help_index>=0) {
    		switch ((cpt>>3)&7) {
			case 0:
    			case 7:
    				strcat(status_bar,psp_msg_string(MENU_STATUS_MISC_HACKDEBUG_HELP_0));break;
			case 1:
    			case 6:
    				strcat(status_bar,psp_msg_string(MENU_STATUS_MISC_HACKDEBUG_HELP_1));break;
    			case 2:
			case 5:
    				strcat(status_bar,psp_msg_string(MENU_STATUS_MISC_HACKDEBUG_HELP_2));break;
    			case 3:
			case 4:
    				strcat(status_bar,psp_msg_string(MENU_STATUS_MISC_HACKDEBUG_HELP_3));break;
    		}
    	}
    	else strcat(status_bar,psp_msg_string(MENU_STATUS_MISC_HACKDEBUG_0));
    	mh_print(8,262,  status_bar,INFOBAR_COL);
		}

		if(DEBUGMENU_ITEMS > rows){
			h = 219;
			pgDrawFrame(445,25,446,243,(0xd<<0)|(0xd<<5)|(0x17<<10));
			pgFillBox(448, h*top/DEBUGMENU_ITEMS + 25,460, h*(top+rows)/DEBUGMENU_ITEMS + 25,
				(0xd<<0)|(0xd<<5)|(0x17<<10));
			pgFillBox(448+1, h*top/DEBUGMENU_ITEMS + 25+1,460-1, h*(top+rows)/DEBUGMENU_ITEMS + 25 -1,
				(0x17<<0)|(0x17<<5)|(0x1f<<10));
		}
		
		x=4; y=3;
		for(i=0; i<rows; i++){
			if(top+i >= DEBUGMENU_ITEMS) break;
			if(top+i == sel) color = SEL_COL;
			else			 color = FILE_COL;
												
			if (color==SEL_COL) {
				
				if (os9xpsp_debugmenu[top+i].value_int) {
					pgPrintSel(x, y, ((30)|(30<<5)|(31<<10)), os9xpsp_debugmenu[top+i].label);
					if (os9xpsp_debugmenu[top+i].values_list_label[0]) {
						int ind=os9xpsp_debugmenu[top+i].value_index;
						pgPrint(x+strlen(os9xpsp_debugmenu[top+i].label), y, ((31)|(29<<5)|(30<<10)), os9xpsp_debugmenu[top+i].values_list_label[ind]);					
					}	else pgPrintDecTrim(x+strlen(os9xpsp_debugmenu[top+i].label), y, ((31)|(29<<5)|(30<<10)), *(os9xpsp_debugmenu[top+i].value_int));
				} else if (os9xpsp_debugmenu[top+i].menu_func) pgPrintSel(x, y, ((28)|(31<<5)|(28<<10)), os9xpsp_debugmenu[top+i].label);
				else pgPrintSel(x, y, ((24)|(24<<5)|(24<<10)), os9xpsp_debugmenu[top+i].label);
				
			}
			else {				
				if (os9xpsp_debugmenu[top+i].value_int) {
					pgPrint(x, y, ((20)|(20<<5)|(31<<10)), os9xpsp_debugmenu[top+i].label);
					if (os9xpsp_debugmenu[top+i].values_list_label[0]) {
						int ind=os9xpsp_debugmenu[top+i].value_index;
						pgPrint(x+strlen(os9xpsp_debugmenu[top+i].label), y, ((31)|(24<<5)|(24<<10)), os9xpsp_debugmenu[top+i].values_list_label[ind]);					
					}	else pgPrintDecTrim(x+strlen(os9xpsp_debugmenu[top+i].label), y, ((31)|(24<<5)|(24<<10)), *(os9xpsp_debugmenu[top+i].value_int));
				} else if (os9xpsp_debugmenu[top+i].menu_func) pgPrint(x, y, ((16)|(24<<5)|(16<<10)), os9xpsp_debugmenu[top+i].label);
				else pgPrint(x, y, ((20)|(20<<5)|(20<<10)), os9xpsp_debugmenu[top+i].label);
			}																	
			y+=1;
		}
						
		pgScreenFlipV2();		
		cpt++;
	}
	
	while (get_pad()) pgWaitV();		
	
	os9x_hack=0;
	os9x_hack|=(os9x_ignore_fixcol?PPU_IGNORE_FIXEDCOLCHANGES:0);
	os9x_hack|=(os9x_ignore_winclip?PPU_IGNORE_WINDOW:0);
	os9x_hack|=(os9x_ignore_addsub?PPU_IGNORE_ADDSUB:0);
	os9x_hack|=(os9x_ignore_palwrite?PPU_IGNORE_PALWRITE:0);	
	os9x_hack|=(os9x_gfx_fastmode7?GFX_FASTMODE7:0);
	os9x_hack|=(os9x_apufix?APU_FIX:0);
	os9x_hack|=(os9x_fix_hires?HIRES_FIX:0);
	os9x_hack|=(os9x_old_accel?OLD_PSP_ACCEL:0);
	return retval;	
}


#define INPUTSMENU_ITEMS 18
int inputs_up,inputs_down,inputs_left,inputs_right,inputs_A,inputs_B,inputs_X,inputs_Y;
int inputs_TL,inputs_TR,inputs_START,inputs_SELECT,inputs_MENU,inputs_TURBO;
int inputs_FSKIPINC,inputs_FSKIPDEC,inputs_GFXENGINE;
menu_time_t os9xpsp_inputsmenu[INPUTSMENU_ITEMS]={	
	{"Analog stick mapped to pad : ",-1,NULL,&os9x_inputs_analog,2,{0,1},0,{"Yes","No"}},
	{"UP : ",-1,NULL,&inputs_up,23,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","LTrg","RTrg","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","LTrg+RTrg","LTrg+START","RTrg+START","LTrg+SELECT","RTrg+SELECT","START+SELECT","None"}},
	{"DOWN : ",-1,NULL,&inputs_down,23,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","LTrg","RTrg","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","LTrg+RTrg","LTrg+START","RTrg+START","LTrg+SELECT","RTrg+SELECT","START+SELECT","None"}},
	{"LEFT : ",-1,NULL,&inputs_left,23,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","LTrg","RTrg","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","LTrg+RTrg","LTrg+START","RTrg+START","LTrg+SELECT","RTrg+SELECT","START+SELECT","None"}},
	{"RIGHT : ",-1,NULL,&inputs_right,23,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","LTrg","RTrg","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","LTrg+RTrg","LTrg+START","RTrg+START","LTrg+SELECT","RTrg+SELECT","START+SELECT","None"}},
	{"A : ",-1,NULL,&inputs_A,23,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","LTrg","RTrg","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","LTrg+RTrg","LTrg+START","RTrg+START","LTrg+SELECT","RTrg+SELECT","START+SELECT","None"}},
	{"B : ",-1,NULL,&inputs_B,23,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","LTrg","RTrg","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","LTrg+RTrg","LTrg+START","RTrg+START","LTrg+SELECT","RTrg+SELECT","START+SELECT","None"}},
	{"X : ",-1,NULL,&inputs_X,23,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","LTrg","RTrg","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","LTrg+RTrg","LTrg+START","RTrg+START","LTrg+SELECT","RTrg+SELECT","START+SELECT","None"}},
	{"Y : ",-1,NULL,&inputs_Y,23,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","LTrg","RTrg","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","LTrg+RTrg","LTrg+START","RTrg+START","LTrg+SELECT","RTrg+SELECT","START+SELECT","None"}},
	{"L : ",-1,NULL,&inputs_TL,23,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","LTrg","RTrg","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","LTrg+RTrg","LTrg+START","RTrg+START","LTrg+SELECT","RTrg+SELECT","START+SELECT","None"}},
	{"R : ",-1,NULL,&inputs_TR,23,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","LTrg","RTrg","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","LTrg+RTrg","LTrg+START","RTrg+START","LTrg+SELECT","RTrg+SELECT","START+SELECT","None"}},
	{"START : ",-1,NULL,&inputs_START,23,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","LTrg","RTrg","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","LTrg+RTrg","LTrg+START","RTrg+START","LTrg+SELECT","RTrg+SELECT","START+SELECT","None"}},
	{"SELECT : ",-1,NULL,&inputs_SELECT,23,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","LTrg","RTrg","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","LTrg+RTrg","LTrg+START","RTrg+START","LTrg+SELECT","RTrg+SELECT","START+SELECT","None"}},
	{"MENU : ",-1,NULL,&inputs_MENU,23,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","LTrg","RTrg","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","LTrg+RTrg","LTrg+START","RTrg+START","LTrg+SELECT","RTrg+SELECT","START+SELECT","None"}},
	{"TURBO : ",-1,NULL,&inputs_TURBO,23,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","LTrg","RTrg","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","LTrg+RTrg","LTrg+START","RTrg+START","LTrg+SELECT","RTrg+SELECT","START+SELECT","None"}},
	{"FRAMESKIP + : ",-1,NULL,&inputs_FSKIPINC,23,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","LTrg","RTrg","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","LTrg+RTrg","LTrg+START","RTrg+START","LTrg+SELECT","RTrg+SELECT","START+SELECT","None"}},
	{"FRAMESKIP - : ",-1,NULL,&inputs_FSKIPDEC,23,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","LTrg","RTrg","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","LTrg+RTrg","LTrg+START","RTrg+START","LTrg+SELECT","RTrg+SELECT","START+SELECT","None"}},
	{"CHANGE GPUPack.GFX ENGINE : ",-1,NULL,&inputs_GFXENGINE,23,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","LTrg","RTrg","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","LTrg+RTrg","LTrg+START","RTrg+START","LTrg+SELECT","RTrg+SELECT","START+SELECT","None"}}
};


int show_inputsmenu(char *mode) {
	//int counter=0;
	unsigned long color=RGB_WHITE;
	static int sel=0;
	int actual_slot;	
	int rows=28,x, y, h, i,j,top=0;
	int cpt,rebuild_entries;
	int retval;		
	char ext[5],slot_fileinfo[32];
	
	if (mode) {mode[0]=0; return 0;}
		
	while (get_pad()) pgWaitV();	
	
	rebuild_entries=1;
	
	
	
	old_pad=0;
	cpt=0;
	for(;;){		
		show_bg(menu_bg);
		show_batteryinfo();   
		show_usbinfo();
    
    if (rebuild_entries) {
    	rebuild_entries=0;
    	inputs_up=inputs_down=inputs_left=inputs_right=inputs_A=inputs_B=inputs_X=inputs_Y=PSP_BUTTONS_TOTAL;
			inputs_TL=inputs_TR=inputs_START=inputs_SELECT=inputs_MENU=inputs_TURBO=PSP_BUTTONS_TOTAL;
			inputs_FSKIPINC=inputs_FSKIPDEC=inputs_GFXENGINE=PSP_BUTTONS_TOTAL;
			
			for (i=0;i<32;i++){
				if (os9x_inputs[i]&SNES_UP_MASK) inputs_up=i;
				if (os9x_inputs[i]&SNES_DOWN_MASK) inputs_down=i;
				if (os9x_inputs[i]&SNES_LEFT_MASK) inputs_left=i;
				if (os9x_inputs[i]&SNES_RIGHT_MASK) inputs_right=i;
				if (os9x_inputs[i]&SNES_A_MASK) inputs_A=i;
				if (os9x_inputs[i]&SNES_B_MASK) inputs_B=i;
				if (os9x_inputs[i]&SNES_X_MASK) inputs_X=i;
				if (os9x_inputs[i]&SNES_Y_MASK) inputs_Y=i;
				if (os9x_inputs[i]&SNES_TL_MASK) inputs_TL=i;
				if (os9x_inputs[i]&SNES_TR_MASK) inputs_TR=i;
				if (os9x_inputs[i]&SNES_START_MASK) inputs_START=i;
				if (os9x_inputs[i]&SNES_SELECT_MASK) inputs_SELECT=i;
				if (os9x_inputs[i]&OS9X_MENUACCESS) inputs_MENU=i;
				if (os9x_inputs[i]&OS9X_TURBO) inputs_TURBO=i;
				if (os9x_inputs[i]&OS9X_FRAMESKIP_DOWN) inputs_FSKIPDEC=i;
				if (os9x_inputs[i]&OS9X_FRAMESKIP_UP) inputs_FSKIPINC=i;		
				if (os9x_inputs[i]&OS9X_GFXENGINE) inputs_GFXENGINE=i;		
			}
				
			//init menu indexes
					
			for (j=0;j<INPUTSMENU_ITEMS;j++) {
				if (os9xpsp_inputsmenu[j].value_int) {			//menu entry has a variable
					int cur=*(os9xpsp_inputsmenu[j].value_int);    //current value
					for (i=0;i<os9xpsp_inputsmenu[j].values_list_size;i++){ //browse list to find index
		    		if (cur==os9xpsp_inputsmenu[j].values_list[i]) {    			
		    			os9xpsp_inputsmenu[j].value_index=i;
		    			break;
		    		}
		      }
		    }
			}		
    }
    
        	
		new_pad=0;
    if (!pad_cnt) new_pad=get_pad();
    else pad_cnt--;
    pgWaitV();
        
		if (new_pad) {
			if (old_pad==new_pad) pad_cnt=1/*2*/;
			else pad_cnt=5;
		 	old_pad=new_pad;
		}
		   		
		if ((sel>0)&&(new_pad & PSP_CTRL_CIRCLE)){
			//if (os9xpsp_inputsmenu[sel].menu_func) 
			//	if ((*os9xpsp_inputsmenu[sel].menu_func)()) {retval=0;break;}
			char st[64];
			int pad;
			SceCtrlData paddata;
			menu_modified=1;
			// wait for no button pressed
			while (get_pad()) pgWaitV();
			//message asking a button press
			sprintf(st,psp_msg_string(MENU_CONTROLS_INPUT_PRESS),os9xpsp_inputsmenu[sel].label);
			msgBoxLines(st,0);
			//wait for a press
			while (1) {
				sceCtrlPeekBufferPositive(&paddata, 1);
				if (!os9x_inputs_analog) {
					if (paddata.Ly >= 0xD0) paddata.Buttons|=PSP_CTRL_DOWN;  // DOWN
  				if (paddata.Ly <= 0x30) paddata.Buttons|=PSP_CTRL_UP;    // UP
  				if (paddata.Lx <= 0x30) paddata.Buttons|=PSP_CTRL_LEFT;  // LEFT
  				if (paddata.Lx >= 0xD0) paddata.Buttons|=PSP_CTRL_RIGHT; // RIGHT
  				paddata.Lx=paddata.Ly=0;
				}
				pgWaitV();
				if (paddata.Buttons) break;
				if (os9x_inputs_analog) {
					if (paddata.Lx<0x30) break;
					if (paddata.Ly<0x30) break;
					if (paddata.Lx>0xD0) break;
					if (paddata.Ly>0xD0) break;
				}
			}
			//wait for release
			while (pad=get_pad()) {
				paddata.Buttons|=pad;
				pgWaitV();
			}
			//now analyse press
			if (paddata.Ly<0x30) *(os9xpsp_inputsmenu[sel].value_int)=PSP_AUP;
			else if (paddata.Ly>0xA0) *(os9xpsp_inputsmenu[sel].value_int)=PSP_ADOWN;
			else if (paddata.Lx<0x30) *(os9xpsp_inputsmenu[sel].value_int)=PSP_ALEFT;
			else if (paddata.Lx>0xA0) *(os9xpsp_inputsmenu[sel].value_int)=PSP_ARIGHT;				
			else if (paddata.Buttons & PSP_CTRL_UP) *(os9xpsp_inputsmenu[sel].value_int)=PSP_UP;
			else if (paddata.Buttons & PSP_CTRL_DOWN) *(os9xpsp_inputsmenu[sel].value_int)=PSP_DOWN;
			else if (paddata.Buttons & PSP_CTRL_LEFT) *(os9xpsp_inputsmenu[sel].value_int)=PSP_LEFT;
			else if (paddata.Buttons & PSP_CTRL_RIGHT) *(os9xpsp_inputsmenu[sel].value_int)=PSP_RIGHT;				
			else if (paddata.Buttons & PSP_CTRL_CIRCLE) *(os9xpsp_inputsmenu[sel].value_int)=PSP_CIRCLE;
			else if (paddata.Buttons & PSP_CTRL_CROSS) *(os9xpsp_inputsmenu[sel].value_int)=PSP_CROSS;
			else if (paddata.Buttons & PSP_CTRL_SQUARE) *(os9xpsp_inputsmenu[sel].value_int)=PSP_SQUARE;
			else if (paddata.Buttons & PSP_CTRL_TRIANGLE) *(os9xpsp_inputsmenu[sel].value_int)=PSP_TRIANGLE;				
			else if (paddata.Buttons == PSP_CTRL_START) *(os9xpsp_inputsmenu[sel].value_int)=PSP_START;
			else if (paddata.Buttons == PSP_CTRL_SELECT) *(os9xpsp_inputsmenu[sel].value_int)=PSP_SELECT;
			else if (paddata.Buttons == PSP_CTRL_LTRIGGER) *(os9xpsp_inputsmenu[sel].value_int)=PSP_TL;
			else if (paddata.Buttons == PSP_CTRL_RTRIGGER) *(os9xpsp_inputsmenu[sel].value_int)=PSP_TR;
			else if (paddata.Buttons == (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER)) *(os9xpsp_inputsmenu[sel].value_int)=PSP_TL_TR;
			else if (paddata.Buttons == (PSP_CTRL_LTRIGGER|PSP_CTRL_START)) *(os9xpsp_inputsmenu[sel].value_int)=PSP_TL_START;
			else if (paddata.Buttons == (PSP_CTRL_RTRIGGER|PSP_CTRL_START)) *(os9xpsp_inputsmenu[sel].value_int)=PSP_TR_START;
			else if (paddata.Buttons == (PSP_CTRL_LTRIGGER|PSP_CTRL_SELECT)) *(os9xpsp_inputsmenu[sel].value_int)=PSP_TL_SELECT;
			else if (paddata.Buttons == (PSP_CTRL_RTRIGGER|PSP_CTRL_SELECT)) *(os9xpsp_inputsmenu[sel].value_int)=PSP_TR_SELECT;
			else if (paddata.Buttons == (PSP_CTRL_START|PSP_CTRL_SELECT)) *(os9xpsp_inputsmenu[sel].value_int)=PSP_SELECT_START;			
			//else if (paddata.Buttons & PSP_CTRL_NOTE) *(os9xpsp_inputsmenu[sel].value_int)=PSP_NOTE;			
			os9xpsp_inputsmenu[sel].value_index=*(os9xpsp_inputsmenu[sel].value_int);
		}			
    else if(new_pad & PSP_CTRL_CROSS)   { 
#ifdef HOME_HOOK // [Shoey]
      retval = 0;
      break;
#else
      //check menu access button
    	if (inputs_MENU==PSP_BUTTONS_TOTAL) {
    		psp_msg(MENU_CONTROLS_INPUT_NOFORMENU,MSG_DEFAULT);
    	}
      else {
        retval= 0;
        break;
      }
#endif
    }
    else if(new_pad & PSP_CTRL_UP)      { sel--; os9x_beep1();   }
    else if(new_pad & PSP_CTRL_DOWN)    { sel++; os9x_beep1();   }
    else if(new_pad & PSP_CTRL_LTRIGGER)      { sel-=10;if (sel<0) sel=0; os9x_beep1();   }
    else if(new_pad & PSP_CTRL_RTRIGGER)    { sel+=10;if (sel>=INPUTSMENU_ITEMS) sel=INPUTSMENU_ITEMS-1; os9x_beep1();   }
    else if(new_pad & PSP_CTRL_LEFT)    {
    	if (os9xpsp_inputsmenu[sel].value_int){
    		MENU_CHGVAL();
    		if (os9xpsp_inputsmenu[sel].value_index>0) os9xpsp_inputsmenu[sel].value_index--;
    		*(os9xpsp_inputsmenu[sel].value_int)=os9xpsp_inputsmenu[sel].values_list[os9xpsp_inputsmenu[sel].value_index];    		
    		
    		if (sel==0) {    			
    			if (!os9x_inputs_analog) {
    				rebuild_entries=1;
    				os9x_inputs[PSP_AUP]=os9x_inputs[PSP_ADOWN]=os9x_inputs[PSP_ALEFT]=os9x_inputs[PSP_ARIGHT]=0;
    			}
    		}
    	}
    }    
    else if(new_pad & PSP_CTRL_RIGHT)    {
    	if (os9xpsp_inputsmenu[sel].value_int){
    		MENU_CHGVAL();
    		if (os9xpsp_inputsmenu[sel].value_index<os9xpsp_inputsmenu[sel].values_list_size-1) os9xpsp_inputsmenu[sel].value_index++;
    		*(os9xpsp_inputsmenu[sel].value_int)=os9xpsp_inputsmenu[sel].values_list[os9xpsp_inputsmenu[sel].value_index];    		
    		
    		if (sel==0) {
    			if (!os9x_inputs_analog) {
    				rebuild_entries=1;
    				os9x_inputs[PSP_AUP]=os9x_inputs[PSP_ADOWN]=os9x_inputs[PSP_ALEFT]=os9x_inputs[PSP_ARIGHT]=0;
    			}
    		}
    	}
    }
    else if(new_pad & PSP_CTRL_SELECT) { //default profile
    	char st[256];
    	SceCtrlData paddata;
    	psp_msg(MENU_CONTROLS_INPUT_DEFAULT,MSG_DEFAULT);
    	// wait for no button pressed
			while (get_pad()) pgWaitV();
			//wait for a press
			while (1) {
				sceCtrlPeekBufferPositive(&paddata, 1);
				pgWaitV();
				if (paddata.Buttons & (PSP_CTRL_CROSS|PSP_CTRL_CIRCLE|PSP_CTRL_TRIANGLE|PSP_CTRL_SQUARE)) break;				
			}
			//wait for release
			while (get_pad()) pgWaitV();
			//now analyse press
			if (paddata.Buttons & PSP_CTRL_CROSS) { //default pad
				rebuild_entries=1;
				memset(os9x_inputs,0,sizeof(os9x_inputs));
				os9x_inputs[PSP_UP]=SNES_UP_MASK;
				os9x_inputs[PSP_DOWN]=SNES_DOWN_MASK;
				os9x_inputs[PSP_LEFT]=SNES_LEFT_MASK;
				os9x_inputs[PSP_RIGHT]=SNES_RIGHT_MASK;
				os9x_inputs[PSP_START]=SNES_START_MASK;
				os9x_inputs[PSP_SELECT]=SNES_SELECT_MASK;
				os9x_inputs[PSP_CIRCLE]=SNES_A_MASK;
				os9x_inputs[PSP_CROSS]=SNES_B_MASK;
				os9x_inputs[PSP_SQUARE]=SNES_Y_MASK;
				os9x_inputs[PSP_TRIANGLE]=SNES_X_MASK;
				os9x_inputs[PSP_TL]=SNES_TL_MASK;
				os9x_inputs[PSP_TR]=SNES_TR_MASK;
				os9x_inputs[PSP_AUP]=OS9X_FRAMESKIP_UP;
				os9x_inputs[PSP_ADOWN]=OS9X_FRAMESKIP_DOWN;
#ifndef HOME_HOOK
				os9x_inputs[PSP_ALEFT]=OS9X_MENUACCESS;
#endif
				os9x_inputs[PSP_ARIGHT]=OS9X_TURBO;
				
				os9x_inputs_analog=1;
			} else if (paddata.Buttons & PSP_CTRL_CIRCLE) { //default analog stick
				rebuild_entries=1;
				memset(os9x_inputs,0,sizeof(os9x_inputs));
				os9x_inputs[PSP_AUP]=SNES_UP_MASK;
				os9x_inputs[PSP_ADOWN]=SNES_DOWN_MASK;
				os9x_inputs[PSP_ALEFT]=SNES_LEFT_MASK;
				os9x_inputs[PSP_ARIGHT]=SNES_RIGHT_MASK;
				os9x_inputs[PSP_START]=SNES_START_MASK;
				os9x_inputs[PSP_SELECT]=SNES_SELECT_MASK;
				os9x_inputs[PSP_CIRCLE]=SNES_A_MASK;
				os9x_inputs[PSP_CROSS]=SNES_B_MASK;
				os9x_inputs[PSP_SQUARE]=SNES_Y_MASK;
				os9x_inputs[PSP_TRIANGLE]=SNES_X_MASK;
				os9x_inputs[PSP_TL]=SNES_TL_MASK;
				os9x_inputs[PSP_TR]=SNES_TR_MASK;
				os9x_inputs[PSP_UP]=OS9X_FRAMESKIP_UP;
				os9x_inputs[PSP_DOWN]=OS9X_FRAMESKIP_DOWN;
#ifndef HOME_HOOK
				os9x_inputs[PSP_ALEFT]=OS9X_MENUACCESS;
#endif
				os9x_inputs[PSP_RIGHT]=OS9X_TURBO;
				
				os9x_inputs_analog=1;
			} else if (paddata.Buttons & PSP_CTRL_SQUARE) { //default analog stick
				rebuild_entries=1;
				memset(os9x_inputs,0,sizeof(os9x_inputs));
				os9x_inputs[PSP_START]=SNES_START_MASK;
				os9x_inputs[PSP_SELECT]=SNES_SELECT_MASK;
				os9x_inputs[PSP_CIRCLE]=SNES_A_MASK;
				os9x_inputs[PSP_CROSS]=SNES_B_MASK;
				os9x_inputs[PSP_SQUARE]=SNES_Y_MASK;
				os9x_inputs[PSP_TRIANGLE]=SNES_X_MASK;
				os9x_inputs[PSP_TL]=SNES_TL_MASK;
				os9x_inputs[PSP_TR]=SNES_TR_MASK;
				os9x_inputs[PSP_UP]=SNES_UP_MASK;
				os9x_inputs[PSP_DOWN]=SNES_DOWN_MASK;
				os9x_inputs[PSP_LEFT]=SNES_LEFT_MASK;
				os9x_inputs[PSP_RIGHT]=SNES_RIGHT_MASK;
				
#ifndef HOME_HOOK
				os9x_inputs[PSP_TL_TR]=OS9X_MENUACCESS; // [Shoey]
#endif
				
				os9x_inputs_analog=0;
			}
    } SNAPSHOT_CODE()
    

		
		if(top > INPUTSMENU_ITEMS-rows)	top=INPUTSMENU_ITEMS-rows;
		if(top < 0)				top=0;
		if(sel >= INPUTSMENU_ITEMS)		sel=0;
		if(sel < 0)				sel=INPUTSMENU_ITEMS-1;
		if(sel >= top+rows)		top=sel-rows+1;
		if(sel < top)			top=sel;
		
       
    //pgPrint(1,0,TITLE_COL,"[" EMUNAME_VERSION "] - Menu");
    //pgPrint(1,33,INFOBAR_COL,"\1 OK \2 Main menu \5,\7 to select ,\6,\b to change value");
    //pgPrint(1,0,TITLE_COL,"[" EMUNAME_VERSION "] - Menu");
    mh_print(8,0,psp_msg_string(MENU_TITLE_CONTROLS_INPUT),TITLE_COL);
    //pgPrint(1,33,INFOBAR_COL,"\1 OK \2 Return game \5,\7 to select ,\6,\b to change value");
    //mh_print(8,262,SJIS_CIRCLE " OK " SJIS_CROSS " Back to Game       " SJIS_STAR "       " SJIS_TRIANGLE " Help        " SJIS_STAR "       " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value",INFOBAR_COL);
    {
    	char status_bar[100];
    	/*if (os9xpsp_debugmenu[sel].menu_func) strcpy(status_bar,SJIS_CIRCLE " OK ");
    	else strcpy(status_bar,"");*/
    	strcpy(status_bar,psp_msg_string(MENU_STATUS_CONTROLS_INPUT));
    	if (os9xpsp_inputsmenu[sel].help_index>=0) {
    		switch ((cpt>>3)&7) {
			case 0:
    			case 7:
    				strcat(status_bar,psp_msg_string(MENU_STATUS_CONTROLS_INPUT_HELP_0));break;
    			case 1:
			case 6:
    				strcat(status_bar,psp_msg_string(MENU_STATUS_CONTROLS_INPUT_HELP_1));break;
    			case 2:
			case 5:
    				strcat(status_bar,psp_msg_string(MENU_STATUS_CONTROLS_INPUT_HELP_2));break;
    			case 3:
			case 4:
    				strcat(status_bar,psp_msg_string(MENU_STATUS_CONTROLS_INPUT_HELP_3));break;
    		}
    	}
    	else strcat(status_bar,psp_msg_string(MENU_STATUS_CONTROLS_INPUT_0));
    	mh_print(8,262,  status_bar,INFOBAR_COL);
		}


		if(INPUTSMENU_ITEMS > rows){
			h = 219;
			pgDrawFrame(445,25,446,243,(0xd<<0)|(0xd<<5)|(0x17<<10));
			pgFillBox(448, h*top/INPUTSMENU_ITEMS + 25,460, h*(top+rows)/INPUTSMENU_ITEMS + 25,
				(0xd<<0)|(0xd<<5)|(0x17<<10));
			pgFillBox(448+1, h*top/INPUTSMENU_ITEMS + 25+1,460-1, h*(top+rows)/INPUTSMENU_ITEMS + 25 -1,
				(0x17<<0)|(0x17<<5)|(0x1f<<10));
		}
		
		x=4; y=3;
		for(i=0; i<rows; i++){
			if(top+i >= INPUTSMENU_ITEMS) break;
			if(top+i == sel) color = SEL_COL;
			else			 color = FILE_COL;
												
			if (color==SEL_COL) {
				
				if (os9xpsp_inputsmenu[top+i].value_int) {
					pgPrintSel(x, y, ((30)|(30<<5)|(31<<10)), os9xpsp_inputsmenu[top+i].label);
					if (os9xpsp_inputsmenu[top+i].values_list_label[0]) {
						int ind=os9xpsp_inputsmenu[top+i].value_index;
						pgPrint(x+strlen(os9xpsp_inputsmenu[top+i].label), y, ((31)|(29<<5)|(30<<10)), os9xpsp_inputsmenu[top+i].values_list_label[ind]);					
					}	else pgPrintDecTrim(x+strlen(os9xpsp_inputsmenu[top+i].label), y, ((31)|(29<<5)|(30<<10)), *(os9xpsp_inputsmenu[top+i].value_int));
				} else if (os9xpsp_inputsmenu[top+i].menu_func) pgPrintSel(x, y, ((28)|(31<<5)|(28<<10)), os9xpsp_inputsmenu[top+i].label);
				else pgPrintSel(x, y, ((24)|(24<<5)|(24<<10)), os9xpsp_inputsmenu[top+i].label);
				
			}
			else {				
				if (os9xpsp_inputsmenu[top+i].value_int) {
					pgPrint(x, y, ((20)|(20<<5)|(31<<10)), os9xpsp_inputsmenu[top+i].label);
					if (os9xpsp_inputsmenu[top+i].values_list_label[0]) {
						int ind=os9xpsp_inputsmenu[top+i].value_index;
						pgPrint(x+strlen(os9xpsp_inputsmenu[top+i].label), y, ((31)|(24<<5)|(24<<10)), os9xpsp_inputsmenu[top+i].values_list_label[ind]);					
					}	else pgPrintDecTrim(x+strlen(os9xpsp_inputsmenu[top+i].label), y, ((31)|(24<<5)|(24<<10)), *(os9xpsp_inputsmenu[top+i].value_int));
				} else if (os9xpsp_inputsmenu[top+i].menu_func) pgPrint(x, y, ((16)|(24<<5)|(16<<10)), os9xpsp_inputsmenu[top+i].label);
				else pgPrint(x, y, ((20)|(20<<5)|(20<<10)), os9xpsp_inputsmenu[top+i].label);
			}																	
			y+=1;
		}
						
		pgScreenFlipV2();				
		cpt++;
	}
	
	while (get_pad()) pgWaitV();
	
	for (i=0;i<32;i++){
		os9x_inputs[i]=0;
		if (inputs_up==i) os9x_inputs[i]|=SNES_UP_MASK;
		if (inputs_down==i) os9x_inputs[i]|=SNES_DOWN_MASK;
		if (inputs_left==i) os9x_inputs[i]|=SNES_LEFT_MASK;
		if (inputs_right==i) os9x_inputs[i]|=SNES_RIGHT_MASK;
		if (inputs_A==i) os9x_inputs[i]|=SNES_A_MASK;
		if (inputs_B==i) os9x_inputs[i]|=SNES_B_MASK;
		if (inputs_X==i) os9x_inputs[i]|=SNES_X_MASK;
		if (inputs_Y==i) os9x_inputs[i]|=SNES_Y_MASK;
		if (inputs_TL==i) os9x_inputs[i]|=SNES_TL_MASK;
		if (inputs_TR==i) os9x_inputs[i]|=SNES_TR_MASK;
		if (inputs_START==i) os9x_inputs[i]|=SNES_START_MASK;
		if (inputs_SELECT==i) os9x_inputs[i]|=SNES_SELECT_MASK;
		if (inputs_MENU==i) os9x_inputs[i]|=OS9X_MENUACCESS;
		if (inputs_TURBO==i) os9x_inputs[i]|=OS9X_TURBO;
		if (inputs_FSKIPDEC==i) os9x_inputs[i]|=OS9X_FRAMESKIP_DOWN;
		if (inputs_FSKIPINC==i) os9x_inputs[i]|=OS9X_FRAMESKIP_UP;
		if (inputs_GFXENGINE==i) os9x_inputs[i]|=OS9X_GFXENGINE;		
	}
	
	
	return retval;	
}


void show_batteryinfo(void){
	static int update_infos=0;
	static char bat_info[128];

	if ( !((update_infos>>3)&1) ) {
		struct tm *tsys;
	  time_t cur_time;
	  //get current time
		sceKernelLibcTime(&cur_time);
		cur_time+=os9x_timezone*60+os9x_daylsavings*3600;
		tsys=localtime(&cur_time);
		if (scePowerIsBatteryExist()) {
			//if exists battery, gather infos
			char bat_time[16];
	  	int batteryLifeTime = scePowerGetBatteryLifeTime();
	  	if (batteryLifeTime > 0) sprintf(bat_time,psp_msg_string(MENU_TITLE_GENERIC_BAT_TIME),batteryLifeTime/60, batteryLifeTime-(batteryLifeTime/60*60));
		  else bat_time[0]=0;
	   	sprintf(bat_info,psp_msg_string(MENU_TITLE_GENERIC_BAT),
		   		tsys->tm_hour,(tsys->tm_sec&1?':':' '),tsys->tm_min,
	   			(scePowerIsPowerOnline()?psp_msg_string(MENU_TITLE_GENERIC_BAT_PLG):""),
	   			(scePowerIsBatteryCharging()?psp_msg_string(MENU_TITLE_GENERIC_BAT_CHRG):""),
		   		(scePowerIsLowBattery()?psp_msg_string(MENU_TITLE_GENERIC_BAT_LOW):""),
		   		scePowerGetBatteryLifePercent(),
		   		bat_time,
		  		scePowerGetBatteryTemp());
		} else {
			sprintf(bat_info,"%02d%c%02d",tsys->tm_hour,(tsys->tm_sec&1?':':' '),tsys->tm_min);
		}

		menu_free_ram=ramAvailable()/1024;
  }
  update_infos++;
  mh_print(479-strlen(bat_info)*5,0,bat_info,(22<<0)|(31<<5)|(22<<10));
}

void show_usbinfo(void){
#ifndef FW3X
	static int update_infos=0;
	static int state=0;
	
	if (!os9x_usballowed) return;
	
	if ( !((update_infos>>3)&1) ) {
		state=sceUsbGetState();
	}
	update_infos++;
	// Display USB Connection Status
	if ((state&PSP_USB_CONNECTION_ESTABLISHED)==PSP_USB_CONNECTION_ESTABLISHED) {
		mh_print(240,0,"USB",(8<<0)|(31<<5)|(8<<10));
	} else {
		mh_print(240,0,"USB",(31<<0)|(8<<5)|(8<<10));
	}
#endif
}

/////////////////////////////////////////
/////////////////////////////////////////
typedef struct {
	int menu_family_id;
	int icon_handle;
	int state;
	int menu_entries_nb;
	//char label[32];
	int label_id;
} menu_xmb_icon_t;

typedef struct {
	int menu_family_id;
	int menu_id;	
	int (*menu_func)(char *mode);
	//char label[32];
	int label_id;
	int help_id;
} menu_xmb_entry_t;

#define MENU_XMB_ICONS_NB 8
#define MENU_XMB_CHEATS 6
menu_xmb_icon_t menu_xmb_icons[MENU_XMB_ICONS_NB]={
	{0,0,0,4,MENU_ICONS_GAME},
	{1,0,0,7,MENU_ICONS_LOADSAVE},
	{2,0,0,2,MENU_ICONS_CONTROLS},
	{3,0,0,11,MENU_ICONS_VIDEO},
	{4,0,0,3,MENU_ICONS_SOUND},
	{5,0,0,8,MENU_ICONS_MISC},
	{6,0,0,10,MENU_ICONS_CHEATS},	
	{7,0,0,2,MENU_ICONS_ABOUT},	
};


#include "psp/imageio.h"
START_EXTERN_C
extern IMAGE *icons[7];
extern int icons_col[7];
END_EXTERN_C
int menu_current_xmb_index=0;
int menu_current_xmb_index_entry[MENU_XMB_ICONS_NB]={
	0,0,0,0,0,0,0,0
};


////////////////////////////////////////
// menu_basic : handle frame drawing,
//  input reading & counters update
///////////////////////////////////////
void menu_basic(int selected) {
	static int cpt_lowbat=0;
	if (!((cpt_lowbat++)&63)) {
		int oldvalue=os9x_lowbat;
		os9x_lowbat=scePowerIsLowBattery();
		if (oldvalue!=os9x_lowbat) {
			menu_buildbg();
		}
	}
	
	menu_current_smoothing=3+round(sin(menu_cnt*3.14159/30)*3);
	menu_current_smoothing_icon=4+round(sin(menu_cnt*3.14159/30)*4);
	
	if (menu_scrolling<0) {
		menu_scrolling=round(-64*cos(menu_cnt2*3.14159/16));
		if (menu_scrolling>0) menu_scrolling=0;
	}
	if (menu_scrolling>0) {
		menu_scrolling=round(64*cos(menu_cnt2*3.14159/16));
		if (menu_scrolling<0) menu_scrolling=0;
	}
	
	menu_cnt++;
	menu_cnt2++;
	menu_cnt3++;
	//draw menu
	menu_drawFrame(selected);
		
  //input scan
	new_pad=0;
  if (!pad_cnt) {
   	new_pad=get_pad();    	
   	
		if (old_pad==new_pad) {
			if (pad_cnt_acc<6) {pad_cnt=1;pad_cnt_acc++;}
			else pad_cnt=0;
		}
		else {pad_cnt_acc=0;pad_cnt=5;}
		old_pad=new_pad;   	
  }
  else pad_cnt--;

  //add the 'X to return' message at bottom
	mh_printCenter(262,psp_msg_string(MENU_STATUS_GENERIC_MSG1),INFOBAR_COL);

	{
		sprintf(str_tmp,psp_msg_string(MENU_STATUS_GENERIC_FREERAM),menu_free_ram);
		mh_print(480-mh_length(str_tmp),262,str_tmp,INFOBAR_COL);
	}
	if (menu_music) {
		sprintf(str_tmp,psp_msg_string(MENU_STATUS_GENERIC_CHANGEMUSIC));
		mh_print(13,262,str_tmp,INFOBAR_COL);
	}

  //right panel
  if (selected==2) { //in
		if (menu_panel_pos>280) {
			menu_panel_pos=479-round( (479-280 + 10)*sin(3.14159*menu_cnt2/12)*sin(3.14159*menu_cnt2/12)*sin(3.14159*menu_cnt2/12) );
			if (menu_panel_pos<280) menu_panel_pos=280;
		}
    pgFillBoxHalfer(menu_panel_pos,14,479,272-15);
	  pgDrawFrame(menu_panel_pos-1,14,menu_panel_pos-1,272-15,12|(12<<5)|(12<<10));
	  pgDrawFrame(menu_panel_pos-2,14,menu_panel_pos-2,272-15,24|(24<<5)|(24<<10));
	  pgDrawFrame(menu_panel_pos-3,14,menu_panel_pos-3,272-15,31|(31<<5)|(31<<10));
  }
  if (selected==3) { //out
  	//add the 'X to return' message at bottom

		if (menu_panel_pos<479) {
			menu_panel_pos=479-round( (479-280)*cos(3.14159*menu_cnt2/12)*cos(3.14159*menu_cnt2/12)*cos(3.14159*menu_cnt2/12) );
			if (menu_panel_pos>479) menu_panel_pos=479;
		}
    pgFillBoxHalfer(menu_panel_pos,14,479,272-15);
    pgDrawFrame(menu_panel_pos-1,14,menu_panel_pos-1,272-15,12|(12<<5)|(12<<10));
	  pgDrawFrame(menu_panel_pos-2,14,menu_panel_pos-2,272-15,24|(24<<5)|(24<<10));
	  pgDrawFrame(menu_panel_pos-3,14,menu_panel_pos-3,272-15,31|(31<<5)|(31<<10));
  }
  
  
  if (menu_music) {
  	if (menu_music_panel_mode) {
  		//same for music playback
	  	if (menu_music_panel_mode==1) { //panel in  		
	  		if (menu_music_panel_pos<menu_music_panel_size) menu_music_panel_pos=round( (menu_music_panel_size+10)*sin(3.14159*menu_cnt3/40)*sin(3.14159*menu_cnt3/40)*sin(3.14159*menu_cnt3/40) );
	  		if (menu_music_panel_pos>=menu_music_panel_size) {menu_music_panel_mode=2;menu_music_panel_pos=menu_music_panel_size;}
	  	} else if (menu_music_panel_mode<2+60*2) { //panel shown, small delay to read
	  		menu_music_panel_mode++;
	  		if (menu_music_panel_mode>=2+60*2) menu_cnt3=0;
	  	} else { //panel out  		
	  		if (menu_music_panel_pos>0) menu_music_panel_pos=round( (menu_music_panel_size)*cos(3.14159*menu_cnt3/40)*cos(3.14159*menu_cnt3/40)*cos(3.14159*menu_cnt3/40) );
	  		if (menu_music_panel_pos<=0) {menu_music_panel_pos=0;menu_music_panel_mode=0;}
	  	}
	    pgFillBoxHalfer(0,271-31,menu_music_panel_pos,271);
	    
	    pgDrawFrame(0,271-34,menu_music_panel_pos+3,271-34,31|(31<<5)|(31<<10));
	    pgDrawFrame(0,271-33,menu_music_panel_pos+2,271-33,24|(24<<5)|(24<<10));
	    pgDrawFrame(0,271-32,menu_music_panel_pos+1,271-32,12|(12<<5)|(12<<10));	    
	    pgDrawFrame(menu_music_panel_pos+1,271-32,menu_music_panel_pos+1,271,12|(12<<5)|(12<<10));
	    pgDrawFrame(menu_music_panel_pos+2,271-33,menu_music_panel_pos+2,271,24|(24<<5)|(24<<10));
	    pgDrawFrame(menu_music_panel_pos+3,271-34,menu_music_panel_pos+3,271,31|(31<<5)|(31<<10));	    
	    
	    mh_print(menu_music_panel_pos-menu_music_panel_size+5,271-30,psp_msg_string(MENU_MISC_BGMUSIC_GAMETITLE),PANEL_TEXTCMD_COL);
	    mh_print(menu_music_panel_pos-menu_music_panel_size+5+mh_length(psp_msg_string(MENU_MISC_BGMUSIC_GAMETITLE)),271-30,menu_music_gametitle,PANEL_BUTTONCMD_COL);    
	    mh_print(menu_music_panel_pos-menu_music_panel_size+5,271-20,psp_msg_string(MENU_MISC_BGMUSIC_TITLE),PANEL_TEXTCMD_COL);
	    mh_print(menu_music_panel_pos-menu_music_panel_size+5+mh_length(psp_msg_string(MENU_MISC_BGMUSIC_TITLE)),271-20,menu_music_songname,PANEL_BUTTONCMD_COL);    	    
	    mh_print(menu_music_panel_pos-menu_music_panel_size+5,271-10,psp_msg_string(MENU_MISC_BGMUSIC_AUTHOR),PANEL_TEXTCMD_COL);
	    mh_print(menu_music_panel_pos-menu_music_panel_size+5+mh_length(psp_msg_string(MENU_MISC_BGMUSIC_AUTHOR)),271-10,menu_music_author,PANEL_BUTTONCMD_COL);    
	  }
	  //check music
	  if (OSPC_IsFinished()) {
	  	menu_stopmusic();
	  	menu_startmusic();
	  }
  }
}

int menu_clockspeed(char *mode) {	
	int retval=0;
	int to_exit=0;
	int new_value=os9x_cpuclock;
	
	if (mode) {sprintf(mode,"%dMHz",os9x_cpuclock);return 0;}
	
	menu_panel_pos=479;
	menu_cnt2=0;
	for (;;) {		
		menu_basic(2+to_exit);
		if (!g_bLoop) {retval=1;break;}
		
    sprintf(str_tmp,"%dMHz",new_value);
    mh_printLimit(menu_panel_pos+5,104,479,272,str_tmp,((31)|(24<<5)|(24<<10)));
    mh_printLimit(menu_panel_pos+5,130,479,272,psp_msg_string(MENU_CHANGE_VALUE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,130,479,272,SJIS_UP " " SJIS_DOWN,PANEL_BUTTONCMD_COL);    
    mh_printLimit(menu_panel_pos+5,140,479,272,psp_msg_string(MENU_CANCEL_VALIDATE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,140,479,272,SJIS_LEFT " " SJIS_CROSS "              " SJIS_CIRCLE,PANEL_BUTTONCMD_COL);    
    
    if (to_exit) {
    	if (menu_panel_pos>=479) return 0;
    } else {
	    if (new_pad&(PSP_CTRL_CROSS|PSP_CTRL_LEFT)) {
	    	os9x_beep1();
	    	to_exit=1;
	    	menu_cnt2=0;

	    } else if (new_pad&PSP_CTRL_CIRCLE) {
	    	os9x_beep1();
	    	to_exit=1;
	    	menu_cnt2=0;
	    	os9x_cpuclock=new_value;
	    } else if (new_pad&PSP_CTRL_DOWN) {    	
	    	switch (new_value) {
	    		case 222:break;
	    		case 266:new_value=222;MENU_CHGVAL();break;
	    		case 300:new_value=266;MENU_CHGVAL();break;
	    		case 333:new_value=300;MENU_CHGVAL();break;
	    	}
	    } else if (new_pad&PSP_CTRL_UP) {    	
	    	switch (new_value) {
	    		case 333:break;
	    		case 300:new_value=333;break;
	    		case 266:new_value=300;MENU_CHGVAL();break;
	    		case 222:new_value=266;MENU_CHGVAL();break;
	    	}
	    } else if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
			 } SNAPSHOT_CODE()
	  }
    //swap screen
		pgScreenFlipV2();
	}
	return retval;
}

#define MENU_ONOFF(label) \
	int retval=0;\
	int to_exit=0;\
	int new_value=label;\
	if (mode) {sprintf(mode,"%s",(label?psp_msg_string(MENU_YES):psp_msg_string(MENU_NO)));return 0;}\
	menu_panel_pos=479;\
	menu_cnt2=0;\
	for (;;) {		\
		menu_basic(2+to_exit);\
		if (!g_bLoop) {retval=1;break;} \
		\
    sprintf(str_tmp,"%s",(new_value?psp_msg_string(MENU_YES):psp_msg_string(MENU_NO)));\
    mh_printLimit(menu_panel_pos+5,104,479,272,str_tmp,((31)|(24<<5)|(24<<10)));\
    mh_printLimit(menu_panel_pos+5,130,479,272,psp_msg_string(MENU_CHANGE_VALUE),PANEL_TEXTCMD_COL);\
    mh_printLimit(menu_panel_pos+5,130,479,272,SJIS_UP " " SJIS_DOWN,PANEL_BUTTONCMD_COL);    \
    mh_printLimit(menu_panel_pos+5,140,479,272,psp_msg_string(MENU_CANCEL_VALIDATE),PANEL_TEXTCMD_COL);\
    mh_printLimit(menu_panel_pos+5,140,479,272,SJIS_LEFT " " SJIS_CROSS "              " SJIS_CIRCLE,PANEL_BUTTONCMD_COL);    \
        \
    if (to_exit) {\
    	if (menu_panel_pos>=479) return 0;\
    } else {\
    	if (new_pad&(PSP_CTRL_CROSS|PSP_CTRL_LEFT)) {\
    		os9x_beep1();\
    		to_exit=1;\
    		menu_cnt2=0;\
    	} else if (new_pad&(PSP_CTRL_CIRCLE)) {\
    		os9x_beep1();\
    		to_exit=1;\
    		menu_cnt2=0;\
    		label=new_value;\
    	} else if (new_pad&PSP_CTRL_DOWN) {    	\
	    	switch (new_value) {\
    			case 0:break;\
    			case 1:new_value=0;MENU_CHGVAL();break;    		\
    		}\
    	} else if (new_pad&PSP_CTRL_UP) {    	\
	    	switch (new_value) {\
    			case 1:break;\
    			case 0:new_value=1;MENU_CHGVAL();break;    		\
    		}\
    	}  else if (new_pad & PSP_CTRL_SELECT) {\
					if (os9x_menumusic) {\
						menu_stopmusic();\
						menu_startmusic();\
					}\
			 } SNAPSHOT_CODE() \
    }\
    /*swap screen*/ \
		pgScreenFlipV2();\
	} \
	return retval; \

int menu_showfps(char *mode){
	MENU_ONOFF(os9x_showfps)
}

int menu_smoothing(char *mode){
	MENU_ONOFF(os9x_smoothing)
}

int menu_speedlimit(char *mode){
	MENU_ONOFF(os9x_speedlimit)
}

int menu_vsync(char *mode){
	MENU_ONOFF(os9x_vsync)
}

int menu_palntsc(char *mode){
	MENU_ONOFF(os9x_forcepal_ntsc)
}

int menu_videomode(char *mode) {	
	int retval=0;
	int to_exit=0;
	int new_value=os9x_render;
	if (mode) {
			switch (os9x_render) {
			case 0:sprintf(mode,psp_msg_string(MENU_VIDEO_MODE_1_1));break;
			case 1:sprintf(mode,psp_msg_string(MENU_VIDEO_MODE_ZOOM_FIT));break;
			case 2:sprintf(mode,psp_msg_string(MENU_VIDEO_MODE_ZOOM_4_3RD));break;
			case 3:sprintf(mode,psp_msg_string(MENU_VIDEO_MODE_ZOOM_WIDE));break;
			case 4:sprintf(mode,psp_msg_string(MENU_VIDEO_MODE_FULLSCREEN));break;
			case 5:sprintf(mode,psp_msg_string(MENU_VIDEO_MODE_FULLSCREEN_CLIPPED));break;
		}
		return 0;
	}
	
	menu_panel_pos=479;
	menu_cnt2=0;
	for (;;) {		
		menu_basic(2+to_exit);
		if (!g_bLoop) {retval=1;break;}
		
		switch (new_value) {
			case 0:sprintf(str_tmp,psp_msg_string(MENU_VIDEO_MODE_1_1));break;
			case 1:sprintf(str_tmp,psp_msg_string(MENU_VIDEO_MODE_ZOOM_FIT));break;
			case 2:sprintf(str_tmp,psp_msg_string(MENU_VIDEO_MODE_ZOOM_4_3RD));break;
			case 3:sprintf(str_tmp,psp_msg_string(MENU_VIDEO_MODE_ZOOM_WIDE));break;
			case 4:sprintf(str_tmp,psp_msg_string(MENU_VIDEO_MODE_FULLSCREEN));break;
			case 5:sprintf(str_tmp,psp_msg_string(MENU_VIDEO_MODE_FULLSCREEN_CLIPPED));break;
		}
    
    mh_printLimit(menu_panel_pos+5,104,479,272,str_tmp,((31)|(24<<5)|(24<<10)));
    mh_printLimit(menu_panel_pos+5,130,479,272,psp_msg_string(MENU_CHANGE_VALUE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,130,479,272,SJIS_UP " " SJIS_DOWN,PANEL_BUTTONCMD_COL);    
    mh_printLimit(menu_panel_pos+5,140,479,272,psp_msg_string(MENU_CANCEL_VALIDATE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,140,479,272,SJIS_LEFT " " SJIS_CROSS "              " SJIS_CIRCLE,PANEL_BUTTONCMD_COL);    
        
    if (to_exit) {
    	if (menu_panel_pos>=479) return 0;
    } else {
    	if (new_pad&(PSP_CTRL_CROSS|PSP_CTRL_LEFT)) {
    		os9x_beep1();
    		to_exit=1;
    		menu_cnt2=0;
    	} else if (new_pad&PSP_CTRL_CIRCLE) {
	    	os9x_beep1();
	    	to_exit=1;
	    	menu_cnt2=0;
	    	os9x_render=new_value;
	    } else if (new_pad&PSP_CTRL_DOWN) {    	
    		if (new_value) {new_value--;MENU_CHGVAL();}
    	} else if (new_pad&PSP_CTRL_UP) {
	    	if (new_value<5) {new_value++;MENU_CHGVAL();}
    	}  else if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
			 } SNAPSHOT_CODE()
    }
    //swap screen
		pgScreenFlipV2();
	}
	return retval;
}

int menu_engine(char *mode) {	
	int retval=0;
	int to_exit=0;
	int new_value=os9x_softrendering;
	if (mode) {		
		switch (os9x_softrendering) {
			case 0:sprintf(mode,psp_msg_string(MENU_VIDEO_ENGINE_APPROX));break;
			case 1:sprintf(mode,psp_msg_string(MENU_VIDEO_ENGINE_ACCUR));break;
			case 2:sprintf(mode,psp_msg_string(MENU_VIDEO_ENGINE_ACCEL));break;
			case 3:sprintf(mode,psp_msg_string(MENU_VIDEO_ENGINE_ACCEL_ACCUR));break;
			case 4:sprintf(mode,psp_msg_string(MENU_VIDEO_ENGINE_ACCEL_APPROX));break;
		}		
		return 0;
	}
	
	menu_panel_pos=479;
	menu_cnt2=0;
	for (;;) {		
		menu_basic(2+to_exit);
		if (!g_bLoop) {retval=1;break;}
		
		switch (new_value) {
			case 0:sprintf(str_tmp,psp_msg_string(MENU_VIDEO_ENGINE_APPROX));break;
			case 1:sprintf(str_tmp,psp_msg_string(MENU_VIDEO_ENGINE_ACCUR));break;
			case 2:sprintf(str_tmp,psp_msg_string(MENU_VIDEO_ENGINE_ACCEL));break;
			case 3:sprintf(str_tmp,psp_msg_string(MENU_VIDEO_ENGINE_ACCEL_ACCUR));break;
			case 4:sprintf(str_tmp,psp_msg_string(MENU_VIDEO_ENGINE_ACCEL_APPROX));break;
		}
    
    mh_printLimit(menu_panel_pos+5,104,479,272,str_tmp,((31)|(24<<5)|(24<<10)));
    mh_printLimit(menu_panel_pos+5,130,479,272,psp_msg_string(MENU_CHANGE_VALUE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,130,479,272,SJIS_UP " " SJIS_DOWN,PANEL_BUTTONCMD_COL);    
    mh_printLimit(menu_panel_pos+5,140,479,272,psp_msg_string(MENU_CANCEL_VALIDATE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,140,479,272,SJIS_LEFT " " SJIS_CROSS "              " SJIS_CIRCLE,PANEL_BUTTONCMD_COL);    
        
    if (to_exit) {
    	if (menu_panel_pos>=479) return 0;
    } else {
    	if (new_pad&(PSP_CTRL_CROSS|PSP_CTRL_LEFT)) {
    		os9x_beep1();
    		to_exit=1;
    		menu_cnt2=0;
    	} else if (new_pad&PSP_CTRL_CIRCLE) {
	    	os9x_beep1();
	    	to_exit=1;
	    	menu_cnt2=0;
	    	os9x_softrendering=new_value;
	    } else if (new_pad&PSP_CTRL_DOWN) {    	
    		if (new_value) {new_value--;MENU_CHGVAL();}
    	} else if (new_pad&PSP_CTRL_UP) {    	
	    	if (new_value<4) {new_value++;MENU_CHGVAL();}
    	}  else if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
			 } SNAPSHOT_CODE()
    }
    //swap screen
		pgScreenFlipV2();		
	}
	return retval;
}

int menu_soundmode(char *mode) {	
	int retval=0;
	int to_exit=0;	
	int oldmode;
	int new_value=os9x_apuenabled;
	
	if (mode) {		
		switch (os9x_apuenabled) {
			case 0:sprintf(mode,psp_msg_string(MENU_SOUND_MODE_NOTEMUL));break;
			case 1:sprintf(mode,psp_msg_string(MENU_SOUND_MODE_EMULOFF));break;
			case 2:sprintf(mode,psp_msg_string(MENU_SOUND_MODE_EMULON));break;			
		}
		return 0;
	}
	
	menu_panel_pos=479;
	menu_cnt2=0;
	
	oldmode=new_value;
	for (;;) {		
		menu_basic(2+to_exit);
		if (!g_bLoop) {retval=1;break;}
		
		if ( ((!oldmode)&&new_value) || (oldmode&&(!new_value)) ) {
			menu_alertmsg(psp_msg_string(MENU_STATUS_GENERIC_NEEDRESET));			
		}
		
		switch (new_value) {
			case 0:sprintf(str_tmp,psp_msg_string(MENU_SOUND_MODE_NOTEMUL));break;
			case 1:sprintf(str_tmp,psp_msg_string(MENU_SOUND_MODE_EMULOFF));break;
			case 2:sprintf(str_tmp,psp_msg_string(MENU_SOUND_MODE_EMULON));break;			
		}
				    
    mh_printLimit(menu_panel_pos+5,104,479,272,str_tmp,((31)|(24<<5)|(24<<10)));
    mh_printLimit(menu_panel_pos+5,130,479,272,psp_msg_string(MENU_CHANGE_VALUE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,130,479,272,SJIS_UP " " SJIS_DOWN,PANEL_BUTTONCMD_COL);    
    mh_printLimit(menu_panel_pos+5,140,479,272,psp_msg_string(MENU_CANCEL_VALIDATE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,140,479,272,SJIS_LEFT " " SJIS_CROSS "              " SJIS_CIRCLE,PANEL_BUTTONCMD_COL);    
        
    if (to_exit) {
    	if (menu_panel_pos>=479) return 0;
    } else {
    	if (new_pad&(PSP_CTRL_CROSS|PSP_CTRL_LEFT)) {
    		os9x_beep1();
    		to_exit=1;
    		menu_cnt2=0;
    	}else if (new_pad&PSP_CTRL_CIRCLE) {
	    	os9x_beep1();
	    	to_exit=1;
	    	menu_cnt2=0;
	    	os9x_apuenabled=new_value;
	    } else if (new_pad&PSP_CTRL_DOWN) {    	
    		if (new_value) {new_value--;MENU_CHGVAL();}
    	} else if (new_pad&PSP_CTRL_UP) {    	
	    	if (new_value<2) {new_value++;MENU_CHGVAL();}
    	}  else if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
			 } SNAPSHOT_CODE()
    }
    //swap screen
		pgScreenFlipV2();		
	}
	
	return retval;
}

int menu_soundfreq(char *mode){	
	int retval=0;
	int to_exit=0;
	int new_value=os9x_sndfreq;
	if (mode) {				
		sprintf(mode,"%dHz",os9x_sndfreq);
		return 0;
	}
	
	menu_panel_pos=479;
	menu_cnt2=0;
	for (;;) {		
		menu_basic(2+to_exit);
		if (!g_bLoop) {retval=1;break;} 
		
		sprintf(str_tmp,"%dHz",new_value);
		mh_printLimit(menu_panel_pos+5,104,479,272,str_tmp,((31)|(24<<5)|(24<<10)));
    mh_printLimit(menu_panel_pos+5,130,479,272,psp_msg_string(MENU_CHANGE_VALUE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,130,479,272,SJIS_UP " " SJIS_DOWN,PANEL_BUTTONCMD_COL);    
    mh_printLimit(menu_panel_pos+5,140,479,272,psp_msg_string(MENU_CANCEL_VALIDATE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,140,479,272,SJIS_LEFT " " SJIS_CROSS "              " SJIS_CIRCLE,PANEL_BUTTONCMD_COL);    
        
    if (to_exit) {
    	if (menu_panel_pos>=479) return 0;
    } else {
    	if (new_pad&(PSP_CTRL_CROSS|PSP_CTRL_LEFT)) {
    		os9x_beep1();
    		to_exit=1;
    		menu_cnt2=0;
    	} else if (new_pad&PSP_CTRL_CIRCLE) {
	    	os9x_beep1();
	    	to_exit=1;
	    	menu_cnt2=0;
	    	os9x_sndfreq=new_value;
	    } else if (new_pad&PSP_CTRL_DOWN) {    	
    		switch (new_value) {
    			case 11025:break;
    			case 22050:new_value=11025;MENU_CHGVAL();break;
    			case 33075:new_value=22050;MENU_CHGVAL();break;
    			case 44100:new_value=33075;MENU_CHGVAL();break;
    		}
    	} else if (new_pad&PSP_CTRL_UP) {    	
	    	switch (new_value) {
    			case 44100:break;
    			case 33075:new_value=44100;MENU_CHGVAL();break;
    			case 22050:new_value=33075;MENU_CHGVAL();break;
    			case 11025:new_value=22050;MENU_CHGVAL();break;
    		}
    	}  else if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
			 } SNAPSHOT_CODE()
    }
    //swap screen
		pgScreenFlipV2();		
	}
	return retval;
}

int menu_gamma(char *mode){	
	int retval=0;
	int to_exit=0;
	int new_value=os9x_gammavalue;
	if (mode) {				
		sprintf(mode,"%d",os9x_gammavalue);
		return 0;
	}
	
	menu_panel_pos=479;
	menu_cnt2=0;
	for (;;) {		
		menu_basic(2+to_exit);
		if (!g_bLoop) {retval=1;break;} 
		
		sprintf(str_tmp,"%d",new_value);
		mh_printLimit(menu_panel_pos+5,104,479,272,str_tmp,((31)|(24<<5)|(24<<10)));
    mh_printLimit(menu_panel_pos+5,130,479,272,psp_msg_string(MENU_CHANGE_VALUE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,130,479,272,SJIS_UP " " SJIS_DOWN,PANEL_BUTTONCMD_COL);
    mh_printLimit(menu_panel_pos+5,140,479,272,psp_msg_string(MENU_DEFAULT_VALUE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,140,479,272,SJIS_TRIANGLE ,PANEL_BUTTONCMD_COL);
    mh_printLimit(menu_panel_pos+5,150,479,272,psp_msg_string(MENU_CANCEL_VALIDATE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,150,479,272,SJIS_LEFT " " SJIS_CROSS "              " SJIS_CIRCLE,PANEL_BUTTONCMD_COL);    
        
    if (to_exit) {
    	if (menu_panel_pos>=479) return 0;
    } else {
    	if (new_pad&(PSP_CTRL_CROSS|PSP_CTRL_LEFT)) {
    		os9x_beep1();
    		to_exit=1;
    		menu_cnt2=0;
    	} else if (new_pad&PSP_CTRL_CIRCLE) {
	    	os9x_beep1();
	    	to_exit=1;
	    	menu_cnt2=0;
	    	os9x_gammavalue=new_value;
	    } else if (new_pad&PSP_CTRL_DOWN) {    	
    		if (new_value) {new_value--;MENU_CHGVAL();}
    	} else if (new_pad&PSP_CTRL_UP) {    	
	    	if (new_value<9) {new_value++;MENU_CHGVAL();}
    	} else if (new_pad&PSP_CTRL_TRIANGLE) {
    		new_value=0;
    	}  else if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
			 } SNAPSHOT_CODE()
    }
    //swap screen
		pgScreenFlipV2();		
	}
	return retval;
}

int menu_fskip(char *mode){	
	int retval=0;
	int to_exit=0;
	int new_value=os9x_fskipvalue;
	if (mode) {				
		if (os9x_fskipvalue<10) sprintf(mode,"%d",os9x_fskipvalue);
		else sprintf(mode,psp_msg_string(MENU_VIDEO_FSKIP_AUTO));
		return 0;
	}
	
	menu_panel_pos=479;
	menu_cnt2=0;
	for (;;) {		
		menu_basic(2+to_exit);
		if (!g_bLoop) {retval=1;break;} 
		
		if (new_value<10) sprintf(str_tmp,"%d",new_value);
		else sprintf(str_tmp,psp_msg_string(MENU_VIDEO_FSKIP_AUTO));
		mh_printLimit(menu_panel_pos+5,104,479,272,str_tmp,((31)|(24<<5)|(24<<10)));
    mh_printLimit(menu_panel_pos+5,130,479,272,psp_msg_string(MENU_CHANGE_VALUE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,130,479,272,SJIS_UP " " SJIS_DOWN,PANEL_BUTTONCMD_COL);
    mh_printLimit(menu_panel_pos+5,140,479,272,psp_msg_string(MENU_DEFAULT_VALUE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,140,479,272,SJIS_TRIANGLE ,PANEL_BUTTONCMD_COL);
    mh_printLimit(menu_panel_pos+5,150,479,272,psp_msg_string(MENU_CANCEL_VALIDATE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,150,479,272,SJIS_LEFT " " SJIS_CROSS "              " SJIS_CIRCLE,PANEL_BUTTONCMD_COL);    
        
    if (to_exit) {
    	if (menu_panel_pos>=479) return 0;
    } else {
    	if (new_pad&(PSP_CTRL_CROSS|PSP_CTRL_LEFT)) {
    		os9x_beep1();
    		to_exit=1;
    		menu_cnt2=0;
    	} else if (new_pad&PSP_CTRL_CIRCLE) {
	    	os9x_beep1();
	    	to_exit=1;
	    	menu_cnt2=0;
	    	os9x_fskipvalue=new_value;
	    } else if (new_pad&PSP_CTRL_DOWN) {    	
    		if (new_value) {new_value--;MENU_CHGVAL();}
    	} else if (new_pad&PSP_CTRL_UP) {    	
	    	if (new_value<10) {new_value++;MENU_CHGVAL();}
    	} else if (new_pad&PSP_CTRL_TRIANGLE) {    	
	    	new_value=10;
    	}  else if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
			 } SNAPSHOT_CODE()
    }
    //swap screen
		pgScreenFlipV2();		
	}
	return retval;
}

int menu_emulinput(char *mode){		
	int retval=0;
	int to_exit=0;
	int new_value=os9x_padindex;
	
	if (mode) {				
		sprintf(mode,psp_msg_string(MENU_CONTROLS_INPUT),new_value);
		return 0;
	}
	
	menu_panel_pos=479;
	menu_cnt2=0;			
	for (;;) {		
		menu_basic(2+to_exit);
		if (!g_bLoop) {retval=1;break;} 
		
		sprintf(str_tmp,psp_msg_string(MENU_CONTROLS_INPUT),new_value);
		mh_printLimit(menu_panel_pos+5,104,479,272,str_tmp,((31)|(24<<5)|(24<<10)));
    mh_printLimit(menu_panel_pos+5,130,479,272,psp_msg_string(MENU_CHANGE_VALUE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,130,479,272,SJIS_UP " " SJIS_DOWN,PANEL_BUTTONCMD_COL);
    mh_printLimit(menu_panel_pos+5,140,479,272,psp_msg_string(MENU_CANCEL_VALIDATE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,140,479,272,SJIS_LEFT " " SJIS_CROSS "              " SJIS_CIRCLE,PANEL_BUTTONCMD_COL);    
        
    if (to_exit) {
    	if (menu_panel_pos>=479) return 0;
    } else {
    	if (new_pad&(PSP_CTRL_CROSS|PSP_CTRL_LEFT)) {
    		os9x_beep1();
    		to_exit=1;
    		menu_cnt2=0;
    	} else if (new_pad&PSP_CTRL_CIRCLE) {
	    	os9x_beep1();
	    	to_exit=1;
	    	menu_cnt2=0;
	    	os9x_padindex=new_value;
	    }	else if (new_pad&PSP_CTRL_DOWN) {    	
    		if (new_value) {new_value--;MENU_CHGVAL();}
    	} else if (new_pad&PSP_CTRL_UP) {    	
	    	if (new_value<5) {new_value++;MENU_CHGVAL();}
    	}  else if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
			 } SNAPSHOT_CODE()
    }
    //swap screen
		pgScreenFlipV2();		
	}
	return retval;
}

int menu_autosaveUpdSRAM(char *mode){
	MENU_ONOFF(os9x_autosavesram)
}

int menu_menumusic(char *mode) {
	MENU_ONOFF(os9x_menumusic)	
}

int menu_menufx(char *mode) {
	MENU_ONOFF(os9x_menufx)	
}

int menu_menupadbeep(char *mode) {
	MENU_ONOFF(os9x_menupadbeep)	
}


int menu_autosavetimer(char *mode){	
	int retval=0;
	int to_exit=0;
	int new_value=os9x_autosavetimer;
	if (mode) {				
		if (os9x_autosavetimer) sprintf(mode,psp_msg_string(MENU_STATE_AUTOSAVETIMER),os9x_autosavetimer);
		else sprintf(mode,psp_msg_string(MENU_STATE_AUTOSAVETIMER_OFF));
		return 0;
	}
	
	menu_panel_pos=479;
	menu_cnt2=0;
	for (;;) {		
		menu_basic(2+to_exit);
		if (!g_bLoop) {retval=1;break;} 
		
		if (new_value) sprintf(str_tmp,psp_msg_string(MENU_STATE_AUTOSAVETIMER),new_value);
		else sprintf(str_tmp,psp_msg_string(MENU_STATE_AUTOSAVETIMER_OFF));
		mh_printLimit(menu_panel_pos+5,104,479,272,str_tmp,((31)|(24<<5)|(24<<10)));
    mh_printLimit(menu_panel_pos+5,130,479,272,psp_msg_string(MENU_CHANGE_VALUE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,130,479,272,SJIS_UP " " SJIS_DOWN,PANEL_BUTTONCMD_COL);
    mh_printLimit(menu_panel_pos+5,140,479,272,psp_msg_string(MENU_DEFAULT_VALUE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,140,479,272,SJIS_TRIANGLE ,PANEL_BUTTONCMD_COL);
    mh_printLimit(menu_panel_pos+5,150,479,272,psp_msg_string(MENU_CANCEL_VALIDATE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,150,479,272,SJIS_LEFT " " SJIS_CROSS "              " SJIS_CIRCLE,PANEL_BUTTONCMD_COL);    
        
    if (to_exit) {
    	if (menu_panel_pos>=479) return 0;
    } else {
    	if (new_pad&(PSP_CTRL_CROSS|PSP_CTRL_LEFT)) {
    		os9x_beep1();
    		to_exit=1;
    		menu_cnt2=0;
    	} else if (new_pad&PSP_CTRL_CIRCLE) {
	    	os9x_beep1();
	    	to_exit=1;
	    	menu_cnt2=0;
	    	os9x_autosavetimer=new_value;
	    } else if (new_pad&PSP_CTRL_DOWN) {    	
    		if (new_value) {new_value--;MENU_CHGVAL();}
    	} else if (new_pad&PSP_CTRL_UP) {    	
	    	if (new_value<5) {new_value++;MENU_CHGVAL();}
    	} else if (new_pad&PSP_CTRL_TRIANGLE) {
    		new_value=0;
    	}  else if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
			 } SNAPSHOT_CODE()
    }
    //swap screen
		pgScreenFlipV2();		
	}
	return retval;
}

u16 os9x_savestate_mini_[10][128*120];
int menu_loadstate(char *mode) {
	FILE *f;
	char slot_occupied_[10];		
	char ext[5],slot_fileinfo_[10][32];	
	int col1,col2,i,x,y,retval,px,py;	
	
	if (mode) {mode[0]=0;return 0;}
	
	menu_cnt2=0;	
	psp_msg(MENU_STATE_SCANNING,MSG_DEFAULT);	
	for (i=0;i<10;i++) {
		if (i==10) strcpy(ext,".zat");
  	else {
  		strcpy(ext,".za0");
  		ext[3]=i+48;
  	}				
		slot_occupied_[i]=os9x_getstate(ext,slot_fileinfo_[i]);
		if (!slot_occupied_[i]) {
			memset(os9x_savestate_mini_[i],0,128*120*2);
			for (y=0;y<120;y++)
			for (x=0;x<128;x++) os9x_savestate_mini_[i][y*128+x]=( (((y+x)>>2)&1)&&(((y+127-x)>>2)&1) ?(15<<10)|(8<<5)|(8<<0):(7<<10)|(3<<5)|(3<<0));
		}
		else {			
			memcpy(os9x_savestate_mini_[i],os9x_savestate_mini,128*120*2);			
		}
	}

	retval=0;
	
	for (;;) {		
		menu_basic(-1);
		if (!g_bLoop) {retval=1;break;} 
		
		mh_print_light((480-22*5)/2,14,psp_msg_string(MENU_STATE_CHOOSELOAD),31|(31<<5)|(31<<10),menu_current_smoothing);
		
		//now draw each slot
		if (state_slot<5) px=40;
		else px=56;
		py=30;
		for (i=0;i<10;i++) {					  	
			py=19+(i/5)*118;
			if (i==5) {
				if (state_slot>=5) px=40;
				else px=56;
			}
				
		  if (i==state_slot) {
		  	if (slot_occupied_[i]) {
		  		u16 *dst=pgGetVramAddr(px+8,py+8);
		  		for (y=0;y<os9x_snesheight*3/8;y++)
						for (x=0;x<96;x++) {							
							dst[y*512+x]=os9x_savestate_mini_[i][(y*4/3)*128+(x*4/3)];
						}
				} else {
					u16 *dst=pgGetVramAddr(px+8,py+8);
		  		for (y=0;y<os9x_snesheight*3/8;y++)
						for (x=0;x<96;x++) {	
							int r1,g1,b1,r2,g2,b2;
							int col1=dst[y*512+x];
							int col2=os9x_savestate_mini_[i][(y*4/3)*128+(x*4/3)];
							b1=(col1>>10)&31;g1=(col1>>5)&31;r1=(col1>>0)&31;
							b2=(col2>>10)&31;g2=(col2>>5)&31;r2=(col2>>0)&31;
							b1=b1/3+b2; if (b1>31) b1=31;
							g1=g1/3+g2; if (g1>31) g1=31;
							r1=r1/3+r2; if (r1>31) r1=31;												
							dst[y*512+x]=(b1<<10)|(g1<<5)|(r1);
						}
				}
				col1=12|(12<<5)|(12<<10);
				col2=31|(31<<5)|(31<<10);
				
				pgDrawFrame(px+6,py+6,px+6+96+4,py+6+os9x_snesheight*3/8+4,col2);
		  	pgDrawFrame(px+7,py+7,px+7+96+2,py+7+os9x_snesheight*3/8+2,col1);
		  	
		  	if (slot_occupied_[i]) {		  		
		  		int j=0;
		  		strcpy(str_tmp,slot_fileinfo_[i]);
		  		while (str_tmp[j]) {if (str_tmp[j]==' ') break; j++;}
		  		if (str_tmp[j]) {
		  			str_tmp[j]=0;
		  			mh_print(px+8+(96-mh_length(str_tmp))/2,py+os9x_snesheight*3/8+10+4,str_tmp,col2);		  		
		  			mh_print(px+8+(96-mh_length(str_tmp+j+1))/2,py+os9x_snesheight*3/8+20+4,str_tmp+j+1,col2);
		  		} else mh_print(px+8+(96-mh_length(str_tmp))/2,py+os9x_snesheight*3/8+10+4,str_tmp,col2);
		  	} else {
		  		sprintf(str_tmp,psp_msg_string(MENU_STATE_FREESLOT));
		  		mh_print(px+8+(96-mh_length(str_tmp))/2,py+os9x_snesheight*3/8+10+4,str_tmp,col2);
		  	}
		  	px+=96+12;
		  }	else {
		  	if (slot_occupied_[i]) {
		  		u16 *dst=pgGetVramAddr(px+8,py+8);
		  		for (y=0;y<os9x_snesheight/4;y++)
						for (x=0;x<64;x++) {
							int r1,g1,b1,r2,g2,b2;
							int col1=dst[y*512+x];
							int col2=os9x_savestate_mini_[i][y*2*128+x*2];
							b1=(col1>>10)&31;g1=(col1>>5)&31;r1=(col1>>0)&31;
							b2=(col2>>10)&31;g2=(col2>>5)&31;r2=(col2>>0)&31;
							b1=b1/3+b2; if (b1>31) b1=31;
							g1=g1/3+g2; if (g1>31) g1=31;
							r1=r1/3+r2; if (r1>31) r1=31;												
							dst[y*512+x]=(b1<<10)|(g1<<5)|(r1);
						}
				} else {
					u16 *dst=pgGetVramAddr(px+8,py+8);
		  		for (y=0;y<os9x_snesheight/4;y++)
						for (x=0;x<64;x++) {	
							int r1,g1,b1,r2,g2,b2;
							int col1=dst[y*512+x];
							int col2=os9x_savestate_mini_[i][(y*2)*128+(x*2)];
							b1=(col1>>10)&31;g1=(col1>>5)&31;r1=(col1>>0)&31;
							b2=(col2>>10)&31;g2=(col2>>5)&31;r2=(col2>>0)&31;
							b1=b1/2+b2*2/3; if (b1>31) b1=31;
							g1=g1/2+g2*2/3; if (g1>31) g1=31;
							r1=r1/2+r2*2/3; if (r1>31) r1=31;												
							dst[y*512+x]=(b1<<10)|(g1<<5)|(r1);
						}
				}
				col1=8|(8<<5)|(8<<10);
				col2=15|(15<<5)|(15<<10);				
				pgDrawFrame(px+6,py+6,px+6+64+4,py+6+os9x_snesheight/4+4,col2);
		  	pgDrawFrame(px+7,py+7,px+7+64+2,py+7+os9x_snesheight/4+2,col1);
		  	
		  	if (slot_occupied_[i]) {		  		
		  		int j=0;
		  		strcpy(str_tmp,slot_fileinfo_[i]);
		  		while (str_tmp[j]) {if (str_tmp[j]==' ') break; j++;}
		  		if (str_tmp[j]) {
		  			str_tmp[j]=0;
		  			mh_print(px+8+(64-mh_length(str_tmp))/2,py+os9x_snesheight/4+10+4,str_tmp,col2);		  		
		  			mh_print(px+8+(64-mh_length(str_tmp+j+1))/2,py+os9x_snesheight/4+20+4,str_tmp+j+1,col2);
		  		} else mh_print(px+8+(64-mh_length(str_tmp))/2,py+os9x_snesheight/4+10+4,str_tmp,col2);
		  	} else {
		  		sprintf(str_tmp,psp_msg_string(MENU_STATE_FREESLOT));
		  		mh_print(px+8+(64-mh_length(str_tmp))/2,py+os9x_snesheight/4+10+4,str_tmp,col2);
		  	}
		  	px+=64+12;
			}			
		}	
		
		if (new_pad&PSP_CTRL_CROSS) {
    	os9x_beep1();
    	retval=0;
    	break;
    }
    
    if (new_pad&PSP_CTRL_LEFT) {
    	if (state_slot%5) state_slot--;
    }
    if (new_pad&PSP_CTRL_RIGHT) {
    	if ((state_slot%5)<4) state_slot++;
    }
    if (new_pad&PSP_CTRL_UP) {
    	if (state_slot>4) state_slot-=5;
    }
    if (new_pad&PSP_CTRL_DOWN) {
    	if (state_slot<5) state_slot+=5;
    }
    if (new_pad&PSP_CTRL_CIRCLE) {
    	if (slot_occupied_[state_slot]) {
    		slot_occupied=1;
    		if (loadstate()) {
    			retval=1;
    			break;
    		}
    	}
    }  else if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
			 } SNAPSHOT_CODE()
    //swap screen
		pgScreenFlipV2();		
	}
	return retval;
}


int menu_savestate(char *mode) {
	FILE *f;
	char slot_occupied_[10];		
	char ext[5],slot_fileinfo_[10][32];	
	int col1,col2,i,x,y,retval,px,py;		
	
	if (mode) {mode[0]=0;return 0;}
	
	menu_cnt2=0;
	
	psp_msg(MENU_STATE_SCANNING,MSG_DEFAULT);
	
	for (i=0;i<10;i++) {
		if (i==10) strcpy(ext,".zat");
  	else {
  		strcpy(ext,".za0");
  		ext[3]=i+48;
  	}				
		slot_occupied_[i]=os9x_getstate(ext,slot_fileinfo_[i]);
		if (!slot_occupied_[i]) {
			memset(os9x_savestate_mini_[i],0,128*120*2);
			for (y=0;y<120;y++)
			for (x=0;x<128;x++) os9x_savestate_mini_[i][y*128+x]=( (((y+x)>>2)&1)&&(((y+127-x)>>2)&1) ?(15<<10)|(8<<5)|(8<<0):(7<<10)|(3<<5)|(3<<0));
		}
		else {			
			memcpy(os9x_savestate_mini_[i],os9x_savestate_mini,128*120*2);			
		}
	}

	retval=0;
	
	for (;;) {		
		menu_basic(-1);
		if (!g_bLoop) {retval=1;break;} 
		
		mh_print_light((480-22*5)/2,14,psp_msg_string(MENU_STATE_CHOOSESAVE),31|(31<<5)|(31<<10),menu_current_smoothing);
		
		//now draw each slot
		if (state_slot<5) px=40;
		else px=56;
		py=30;
		for (i=0;i<10;i++) {					  	
			py=19+(i/5)*118;
			if (i==5) {
				if (state_slot>=5) px=40;
				else px=56;
			}
				
		  if (i==state_slot) {
		  	if (slot_occupied_[i]) {
		  		u16 *dst=pgGetVramAddr(px+8,py+8);
		  		for (y=0;y<os9x_snesheight*3/8;y++)
						for (x=0;x<96;x++) {							
							dst[y*512+x]=os9x_savestate_mini_[i][(y*4/3)*128+(x*4/3)];
						}
				} else {
					u16 *dst=pgGetVramAddr(px+8,py+8);
		  		for (y=0;y<os9x_snesheight*3/8;y++)
						for (x=0;x<96;x++) {	
							int r1,g1,b1,r2,g2,b2;
							int col1=dst[y*512+x];
							int col2=os9x_savestate_mini_[i][(y*4/3)*128+(x*4/3)];
							b1=(col1>>10)&31;g1=(col1>>5)&31;r1=(col1>>0)&31;
							b2=(col2>>10)&31;g2=(col2>>5)&31;r2=(col2>>0)&31;
							b1=b1/3+b2; if (b1>31) b1=31;
							g1=g1/3+g2; if (g1>31) g1=31;
							r1=r1/3+r2; if (r1>31) r1=31;												
							dst[y*512+x]=(b1<<10)|(g1<<5)|(r1);
						}
				}
				col1=12|(12<<5)|(12<<10);
				col2=31|(31<<5)|(31<<10);
				
				pgDrawFrame(px+6,py+6,px+6+96+4,py+6+os9x_snesheight*3/8+4,col2);
		  	pgDrawFrame(px+7,py+7,px+7+96+2,py+7+os9x_snesheight*3/8+2,col1);
		  	
		  	if (slot_occupied_[i]) {		  		
		  		int j=0;
		  		strcpy(str_tmp,slot_fileinfo_[i]);
		  		while (str_tmp[j]) {if (str_tmp[j]==' ') break; j++;}
		  		if (str_tmp[j]) {
		  			str_tmp[j]=0;
		  			mh_print(px+8+(96-mh_length(str_tmp))/2,py+os9x_snesheight*3/8+10+4,str_tmp,col2);		  		
		  			mh_print(px+8+(96-mh_length(str_tmp+j+1))/2,py+os9x_snesheight*3/8+20+4,str_tmp+j+1,col2);
		  		} else mh_print(px+8+(96-mh_length(str_tmp))/2,py+os9x_snesheight*3/8+10+4,str_tmp,col2);
		  	} else {
		  		sprintf(str_tmp,psp_msg_string(MENU_STATE_FREESLOT));
		  		mh_print(px+8+(96-mh_length(str_tmp))/2,py+os9x_snesheight*3/8+10+4,str_tmp,col2);
		  	}
		  	px+=96+12;
		  }	else {
		  	if (slot_occupied_[i]) {
		  		u16 *dst=pgGetVramAddr(px+8,py+8);
		  		for (y=0;y<os9x_snesheight/4;y++)
						for (x=0;x<64;x++) {
							int r1,g1,b1,r2,g2,b2;
							int col1=dst[y*512+x];
							int col2=os9x_savestate_mini_[i][y*2*128+x*2];
							b1=(col1>>10)&31;g1=(col1>>5)&31;r1=(col1>>0)&31;
							b2=(col2>>10)&31;g2=(col2>>5)&31;r2=(col2>>0)&31;
							b1=b1/3+b2; if (b1>31) b1=31;
							g1=g1/3+g2; if (g1>31) g1=31;
							r1=r1/3+r2; if (r1>31) r1=31;												
							dst[y*512+x]=(b1<<10)|(g1<<5)|(r1);
						}
				} else {
					u16 *dst=pgGetVramAddr(px+8,py+8);
		  		for (y=0;y<os9x_snesheight/4;y++)
						for (x=0;x<64;x++) {	
							int r1,g1,b1,r2,g2,b2;
							int col1=dst[y*512+x];
							int col2=os9x_savestate_mini_[i][(y*2)*128+(x*2)];
							b1=(col1>>10)&31;g1=(col1>>5)&31;r1=(col1>>0)&31;
							b2=(col2>>10)&31;g2=(col2>>5)&31;r2=(col2>>0)&31;
							b1=b1/2+b2*2/3; if (b1>31) b1=31;
							g1=g1/2+g2*2/3; if (g1>31) g1=31;
							r1=r1/2+r2*2/3; if (r1>31) r1=31;												
							dst[y*512+x]=(b1<<10)|(g1<<5)|(r1);
						}
				}
				col1=8|(8<<5)|(8<<10);
				col2=15|(15<<5)|(15<<10);				
				pgDrawFrame(px+6,py+6,px+6+64+4,py+6+os9x_snesheight/4+4,col2);
		  	pgDrawFrame(px+7,py+7,px+7+64+2,py+7+os9x_snesheight/4+2,col1);
		  	
		  	if (slot_occupied_[i]) {		  		
		  		int j=0;
		  		strcpy(str_tmp,slot_fileinfo_[i]);
		  		while (str_tmp[j]) {if (str_tmp[j]==' ') break; j++;}
		  		if (str_tmp[j]) {
		  			str_tmp[j]=0;
		  			mh_print(px+8+(64-mh_length(str_tmp))/2,py+os9x_snesheight/4+10+4,str_tmp,col2);		  		
		  			mh_print(px+8+(64-mh_length(str_tmp+j+1))/2,py+os9x_snesheight/4+20+4,str_tmp+j+1,col2);
		  		} else mh_print(px+8+(64-mh_length(str_tmp))/2,py+os9x_snesheight/4+10+4,str_tmp,col2);
		  	} else {
		  		sprintf(str_tmp,psp_msg_string(MENU_STATE_FREESLOT));
		  		mh_print(px+8+(64-mh_length(str_tmp))/2,py+os9x_snesheight/4+10+4,str_tmp,col2);
		  	}
		  	px+=64+12;
			}			
		}	
		
		if (new_pad&PSP_CTRL_CROSS) {
    	os9x_beep1();
    	retval=0;
    	break;
    }
    
    if (new_pad&PSP_CTRL_LEFT) {
    	if (state_slot%5) state_slot--;
    }
    if (new_pad&PSP_CTRL_RIGHT) {
    	if ((state_slot%5)<4) state_slot++;
    }
    if (new_pad&PSP_CTRL_UP) {
    	if (state_slot>4) state_slot-=5;
    }
    if (new_pad&PSP_CTRL_DOWN) {
    	if (state_slot<5) state_slot+=5;
    }
    if (new_pad&PSP_CTRL_CIRCLE) {    	
    	slot_occupied=slot_occupied_[state_slot];
    	if (savestate()) {
    		retval=1;
    		break;
    	}
    }  else if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
			 } SNAPSHOT_CODE()
    //swap screen
		pgScreenFlipV2();		
	}
	return retval;
}

int menu_deletestate(char *mode) {
	FILE *f;
	char slot_occupied_[10];		
	char ext[5],slot_fileinfo_[10][32];	
	int col1,col2,i,x,y,retval,px,py;
	
	if (mode) {mode[0]=0;return 0;}
	
	menu_cnt2=0;
	
	psp_msg(MENU_STATE_SCANNING,MSG_DEFAULT);
	
	for (i=0;i<10;i++) {
		if (i==10) strcpy(ext,".zat");
  	else {
  		strcpy(ext,".za0");
  		ext[3]=i+48;
  	}
		slot_occupied_[i]=os9x_getstate(ext,slot_fileinfo_[i]);
		if (!slot_occupied_[i]) {
			memset(os9x_savestate_mini_[i],0,128*120*2);
			for (y=0;y<120;y++)
			for (x=0;x<128;x++) os9x_savestate_mini_[i][y*128+x]=( (((y+x)>>2)&1)&&(((y+127-x)>>2)&1) ?(15<<10)|(8<<5)|(8<<0):(7<<10)|(3<<5)|(3<<0));
		}
		else {			
			memcpy(os9x_savestate_mini_[i],os9x_savestate_mini,128*120*2);			
		}
	}

	retval=0;
	
	for (;;) {		
		menu_basic(-1);
		
		mh_print_light((480-22*5)/2,14,psp_msg_string(MENU_STATE_CHOOSEDEL),31|(31<<5)|(31<<10),menu_current_smoothing);
		
		//now draw each slot
		if (state_slot<5) px=40;
		else px=56;
		py=30;
		for (i=0;i<10;i++) {					  	
			py=19+(i/5)*118;
			if (i==5) {
				if (state_slot>=5) px=40;
				else px=56;
			}
				
		  if (i==state_slot) {
		  	if (slot_occupied_[i]) {
		  		u16 *dst=pgGetVramAddr(px+8,py+8);
		  		for (y=0;y<os9x_snesheight*3/8;y++)
						for (x=0;x<96;x++) {							
							dst[y*512+x]=os9x_savestate_mini_[i][(y*4/3)*128+(x*4/3)];
						}
				} else {
					u16 *dst=pgGetVramAddr(px+8,py+8);
		  		for (y=0;y<os9x_snesheight*3/8;y++)
						for (x=0;x<96;x++) {	
							int r1,g1,b1,r2,g2,b2;
							int col1=dst[y*512+x];
							int col2=os9x_savestate_mini_[i][(y*4/3)*128+(x*4/3)];
							b1=(col1>>10)&31;g1=(col1>>5)&31;r1=(col1>>0)&31;
							b2=(col2>>10)&31;g2=(col2>>5)&31;r2=(col2>>0)&31;
							b1=b1/3+b2; if (b1>31) b1=31;
							g1=g1/3+g2; if (g1>31) g1=31;
							r1=r1/3+r2; if (r1>31) r1=31;												
							dst[y*512+x]=(b1<<10)|(g1<<5)|(r1);
						}
				}
				col1=12|(12<<5)|(12<<10);
				col2=31|(31<<5)|(31<<10);
				
				pgDrawFrame(px+6,py+6,px+6+96+4,py+6+os9x_snesheight*3/8+4,col2);
		  	pgDrawFrame(px+7,py+7,px+7+96+2,py+7+os9x_snesheight*3/8+2,col1);
		  	
		  	if (slot_occupied_[i]) {		  		
		  		int j=0;
		  		strcpy(str_tmp,slot_fileinfo_[i]);
		  		while (str_tmp[j]) {if (str_tmp[j]==' ') break; j++;}
		  		if (str_tmp[j]) {
		  			str_tmp[j]=0;
		  			mh_print(px+8+(96-mh_length(str_tmp))/2,py+os9x_snesheight*3/8+10+4,str_tmp,col2);		  		
		  			mh_print(px+8+(96-mh_length(str_tmp+j+1))/2,py+os9x_snesheight*3/8+20+4,str_tmp+j+1,col2);
		  		} else mh_print(px+8+(96-mh_length(str_tmp))/2,py+os9x_snesheight*3/8+10+4,str_tmp,col2);
		  	} else {
		  		sprintf(str_tmp,psp_msg_string(MENU_STATE_FREESLOT));
		  		mh_print(px+8+(96-mh_length(str_tmp))/2,py+os9x_snesheight*3/8+10+4,str_tmp,col2);
		  	}
		  	px+=96+12;
		  }	else {
		  	if (slot_occupied_[i]) {
		  		u16 *dst=pgGetVramAddr(px+8,py+8);
		  		for (y=0;y<os9x_snesheight/4;y++)
						for (x=0;x<64;x++) {
							int r1,g1,b1,r2,g2,b2;
							int col1=dst[y*512+x];
							int col2=os9x_savestate_mini_[i][y*2*128+x*2];
							b1=(col1>>10)&31;g1=(col1>>5)&31;r1=(col1>>0)&31;
							b2=(col2>>10)&31;g2=(col2>>5)&31;r2=(col2>>0)&31;
							b1=b1/3+b2; if (b1>31) b1=31;
							g1=g1/3+g2; if (g1>31) g1=31;
							r1=r1/3+r2; if (r1>31) r1=31;												
							dst[y*512+x]=(b1<<10)|(g1<<5)|(r1);
						}
				} else {
					u16 *dst=pgGetVramAddr(px+8,py+8);
		  		for (y=0;y<os9x_snesheight/4;y++)
						for (x=0;x<64;x++) {	
							int r1,g1,b1,r2,g2,b2;
							int col1=dst[y*512+x];
							int col2=os9x_savestate_mini_[i][(y*2)*128+(x*2)];
							b1=(col1>>10)&31;g1=(col1>>5)&31;r1=(col1>>0)&31;
							b2=(col2>>10)&31;g2=(col2>>5)&31;r2=(col2>>0)&31;
							b1=b1/2+b2*2/3; if (b1>31) b1=31;
							g1=g1/2+g2*2/3; if (g1>31) g1=31;
							r1=r1/2+r2*2/3; if (r1>31) r1=31;												
							dst[y*512+x]=(b1<<10)|(g1<<5)|(r1);
						}
				}
				col1=8|(8<<5)|(8<<10);
				col2=15|(15<<5)|(15<<10);				
				pgDrawFrame(px+6,py+6,px+6+64+4,py+6+os9x_snesheight/4+4,col2);
		  	pgDrawFrame(px+7,py+7,px+7+64+2,py+7+os9x_snesheight/4+2,col1);
		  	
		  	if (slot_occupied_[i]) {		  		
		  		int j=0;
		  		strcpy(str_tmp,slot_fileinfo_[i]);
		  		while (str_tmp[j]) {if (str_tmp[j]==' ') break; j++;}
		  		if (str_tmp[j]) {
		  			str_tmp[j]=0;
		  			mh_print(px+8+(64-mh_length(str_tmp))/2,py+os9x_snesheight/4+10+4,str_tmp,col2);		  		
		  			mh_print(px+8+(64-mh_length(str_tmp+j+1))/2,py+os9x_snesheight/4+20+4,str_tmp+j+1,col2);
		  		} else mh_print(px+8+(64-mh_length(str_tmp))/2,py+os9x_snesheight/4+10+4,str_tmp,col2);
		  	} else {
		  		sprintf(str_tmp,psp_msg_string(MENU_STATE_FREESLOT));
		  		mh_print(px+8+(64-mh_length(str_tmp))/2,py+os9x_snesheight/4+10+4,str_tmp,col2);
		  	}
		  	px+=64+12;
			}			
		}	
		
		if (new_pad&PSP_CTRL_CROSS) {
    	os9x_beep1();
    	retval=0;
    	break;
    }
    
    if (new_pad&PSP_CTRL_LEFT) {
    	if (state_slot%5) state_slot--;
    }
    if (new_pad&PSP_CTRL_RIGHT) {
    	if ((state_slot%5)<4) state_slot++;
    }
    if (new_pad&PSP_CTRL_UP) {
    	if (state_slot>4) state_slot-=5;
    }
    if (new_pad&PSP_CTRL_DOWN) {
    	if (state_slot<5) state_slot+=5;
    }
    if (new_pad&PSP_CTRL_CIRCLE) {    	
    	slot_occupied=slot_occupied_[state_slot];
    	if (deletestate()) {
    		retval=0;
    		break;
    	}
    } else if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
			 }  SNAPSHOT_CODE()
    //swap screen
		pgScreenFlipV2();		
	}
	return retval;
}

int menu_addRAWcode(char *mode) {
	int retval=0;
	char my_code[16];
	uint32 data = 0;
			
	if (mode) {mode[0]=0;return 0;}
			
	//wait for no input
	while (get_pad()) pgWaitV();
	sprintf(my_code,"00000000");
	retval=InputCodeBox(psp_msg_string(MENU_CHEATS_ENTERRAW),"%X%X%X%X%X%X - %X%X",my_code);	
	if (!retval) {
		sscanf (my_code, "%x", &data);		
		uint32 address = data >> 8;
    uint8 byte = (uint8) data;
		cheats_modified=1;
		if (S9xAddCheat(1,1,address,byte)) {
			psp_msg(ERR_ADD_CODE,MSG_DEFAULT);
		} else {
			sprintf(Cheat.c[Cheat.num_cheats-1].name,"cht%d",Cheat.num_cheats);
			menu_inputName(Cheat.c[Cheat.num_cheats-1].name);
			S9xEnableCheat(Cheat.num_cheats-1);
		}
	}
	
	retval=0;
	return retval;
}

int menu_addGGcode(char *mode) {
	int retval=0;
	char my_code[16],ggcode[16],*res_str;
	uint8 byte;
	uint32 address;
	
	if (mode) {mode[0]=0;return 0;}
			
	//wait for no input
	while (get_pad()) pgWaitV();
	sprintf(my_code,"00000000");
	retval=InputCodeBox(psp_msg_string(MENU_CHEATS_ENTERGG),"%X%X%X%X-%X%X%X%X",my_code);	
	
	if (!retval) {
		memcpy(ggcode,my_code,4);
		memcpy(ggcode+5,my_code+4,5);
		ggcode[4]='-';
		res_str=S9xGameGenieToRaw(ggcode,&address,&byte);
		if (res_str) msgBoxLines(res_str,60);
		else {
			//add code to list
			cheats_modified=1;
			if (S9xAddCheat(1,1,address,byte)) {
				psp_msg(ERR_ADD_CODE,MSG_DEFAULT);
			} else {
				sprintf(Cheat.c[Cheat.num_cheats-1].name,"cht%d",Cheat.num_cheats);
				menu_inputName(Cheat.c[Cheat.num_cheats-1].name);
				S9xEnableCheat(Cheat.num_cheats-1);
			}
		}
	}
	retval=0;
	return retval;
}

int menu_addPARcode(char *mode) {
	int retval=0;
	char my_code[16];
	uint32 address;
	uint8 byte;
	char *res_str;
	
	if (mode) {mode[0]=0;return 0;}
			
	//wait for no input
	while (get_pad()) pgWaitV();
	sprintf(my_code,"00000000");
	retval=InputCodeBox(psp_msg_string(MENU_CHEATS_ENTERPAR),"%X%X%X%X%X%X%X%X",my_code);	
	
	if (!retval) {		
		res_str=S9xProActionReplayToRaw(my_code,&address,&byte);
		if (res_str) msgBoxLines(res_str,60);
		else {
			//add code to list
			cheats_modified=1;
			if (S9xAddCheat(1,1,address,byte)) {
				psp_msg(ERR_ADD_CODE,MSG_DEFAULT);
			} else {
				sprintf(Cheat.c[Cheat.num_cheats-1].name,"cht%d",Cheat.num_cheats);
				menu_inputName(Cheat.c[Cheat.num_cheats-1].name);
				S9xEnableCheat(Cheat.num_cheats-1);
			}
		}
	}
	retval=0;
	return retval;
}

int menu_addGFcode(char *mode) {
	int retval=0;
	char my_code[16];
	uint32 address;
	uint8 num_bytes,sram,bytes[3],c;
	char *res_str;
	
	if (mode) {mode[0]=0;return 0;}
			
	//wait for no input
	while (get_pad()) pgWaitV();
	sprintf(my_code,"00000000000000");
	retval=InputCodeBox(psp_msg_string(MENU_CHEATS_ENTERGF),"%X%X%X%X%X%X%X%X%X%X%X%X%X%X",my_code);	
	
	if (!retval) {		
		res_str=S9xGoldFingerToRaw(my_code,&address,&sram,&num_bytes,bytes);
		if (res_str) msgBoxLines(res_str,60);
		else {
			//add code to list			
			cheats_modified=1;
			for (c = 0; c < num_bytes; c++) {
			  if (S9xAddCheat (1, 1, address + c, bytes [c])) {
			  	psp_msg(ERR_ADD_CODE,MSG_DEFAULT); break;
				} else {
					sprintf(Cheat.c[Cheat.num_cheats-1].name,"cht%d",Cheat.num_cheats);
					menu_inputName(Cheat.c[Cheat.num_cheats-1].name);
					S9xEnableCheat(Cheat.num_cheats-1);
				}
			}
		}
	}
	retval=0;
	return retval;
}

int menu_disablecode(char *mode) {
	int retval=0,i,sel=0,cpt;	
	
	if (mode) {mode[0]=0;return 0;}
	
	if (!(Cheat.num_cheats)) return retval;
	
	for (;;) {
		menu_basic(1);
		cpt++;
				
		pgFillBoxHalfer(280,13,479,18+Cheat.num_cheats*10+3);
		for (i=0;i<Cheat.num_cheats;i++) {
			if (i==sel) sprintf(str_tmp,"%c%s:%06X -> %02X",((cpt>>2)&1?'>':' '),Cheat.c[i].name,Cheat.c[i].address,Cheat.c[i].byte);
			else sprintf(str_tmp,"%s:%06X -> %02X",Cheat.c[i].name,Cheat.c[i].address,Cheat.c[i].byte);
									
			if (Cheat.c[i].enabled) mh_print(290,18+i*10,str_tmp,CHEATS_ACTIVE_COL);
			else mh_print(290,18+i*10,str_tmp,CHEATS_DISABLED_COL);
		}
		
		if (new_pad&PSP_CTRL_UP) {
			if (sel) {sel--;os9x_beep1();}
		}
		if (new_pad&PSP_CTRL_DOWN) {
			if (sel<Cheat.num_cheats-1) {sel++;os9x_beep1();}
		}
		
		if (new_pad&PSP_CTRL_CIRCLE) {			
			S9xDisableCheat(sel);
			cheats_modified=1;
    	os9x_beep1();
    	retval=0;
    	break;
    }
		
		if (new_pad&PSP_CTRL_CROSS) {
    	os9x_beep1();
    	retval=0;
    	break;
    } else if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
			 }  SNAPSHOT_CODE()
		//swap screen
		pgScreenFlipV2();		
	}
	
	return retval;
}

int menu_enablecode(char *mode) {
	int retval=0,i,sel=0,cpt;	
	
	if (mode) {mode[0]=0;return 0;}
	
	if (!(Cheat.num_cheats)) return retval;
	
	for (;;) {
		menu_basic(1);
		cpt++;
				
		pgFillBoxHalfer(280,13,479,18+Cheat.num_cheats*10+3);
		for (i=0;i<Cheat.num_cheats;i++) {
			if (i==sel) sprintf(str_tmp,"%c%s:%06X -> %02X",((cpt>>2)&1?'>':' '),Cheat.c[i].name,Cheat.c[i].address,Cheat.c[i].byte);
			else sprintf(str_tmp,"%s:%06X -> %02X",Cheat.c[i].name,Cheat.c[i].address,Cheat.c[i].byte);
									
			if (Cheat.c[i].enabled) mh_print(290,18+i*10,str_tmp,CHEATS_ACTIVE_COL);
			else mh_print(290,18+i*10,str_tmp,CHEATS_DISABLED_COL);
		}
		
		if (new_pad&PSP_CTRL_UP) {
			if (sel) {sel--;os9x_beep1();}
		}
		if (new_pad&PSP_CTRL_DOWN) {
			if (sel<Cheat.num_cheats-1) {sel++;os9x_beep1();}
		}
		
		if (new_pad&PSP_CTRL_CIRCLE) {			
			S9xEnableCheat(sel);
			cheats_modified=1;
    	os9x_beep1();
    	retval=0;
    	break;
    }
		
		if (new_pad&PSP_CTRL_CROSS) {
    	os9x_beep1();
    	retval=0;
    	break;
    } else if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
			 }  SNAPSHOT_CODE()
		//swap screen
		pgScreenFlipV2();		
	}
	return retval;
}

int menu_removecode(char *mode) {
	int retval=0,i,sel=0,cpt;	
	
	if (mode) {mode[0]=0;return 0;}
	
	if (!(Cheat.num_cheats)) return retval;
	
	for (;;) {
		menu_basic(1);
		cpt++;
				
		pgFillBoxHalfer(280,13,479,18+Cheat.num_cheats*10+3);
		for (i=0;i<Cheat.num_cheats;i++) {
			if (i==sel) sprintf(str_tmp,"%c%s:%06X -> %02X",((cpt>>2)&1?'>':' '),Cheat.c[i].name,Cheat.c[i].address,Cheat.c[i].byte);
			else sprintf(str_tmp,"%s:%06X -> %02X",Cheat.c[i].name,Cheat.c[i].address,Cheat.c[i].byte);
									
			if (Cheat.c[i].enabled) mh_print(290,18+i*10,str_tmp,CHEATS_ACTIVE_COL);
			else mh_print(290,18+i*10,str_tmp,CHEATS_DISABLED_COL);
		}
		
		if (new_pad&PSP_CTRL_UP) {
			if (sel) {sel--;os9x_beep1();}
		}
		if (new_pad&PSP_CTRL_DOWN) {
			if (sel<Cheat.num_cheats-1) {sel++;os9x_beep1();}
		}
		
		if (new_pad&PSP_CTRL_CIRCLE) {
			S9xDeleteCheat(sel);
			cheats_modified=1;
    	os9x_beep1();
    	retval=0;
    	break;
    }
		
		if (new_pad&PSP_CTRL_CROSS) {
    	os9x_beep1();
    	retval=0;
    	break;
    } else if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
			 }  SNAPSHOT_CODE()
		//swap screen
		pgScreenFlipV2();		
	}
	
	return retval;
}

int menu_disableallcodes(char *mode) {
	int i;
	if (mode) {mode[0]=0;return 0;}
	
	if (!(Cheat.num_cheats)) return 0;
	for (i=0;i<Cheat.num_cheats;i++) S9xDisableCheat(i);
	cheats_modified=1;
	return 0;
}

int menu_enableallcodes(char *mode) {
	int i;
	if (mode) {mode[0]=0;return 0;}
	
	if (!(Cheat.num_cheats)) return 0;
	for (i=0;i<Cheat.num_cheats;i++) S9xEnableCheat(i);
	cheats_modified=1;
	return 0;
}

int menu_removeallcodes(char *mode) {
	int i;
	if (mode) {mode[0]=0;return 0;}
		
	if (!(Cheat.num_cheats)) return 0;
	if (!psp_msg(MENU_CHEATS_CONFIRMREMALL,MSG_DEFAULT)) return 0;
	cheats_modified=1;
	S9xDeleteCheats();
	return 0;
}

void menu_startmusic(){
	unzFile zip_file;
	unz_file_info unzinfo;
	
	int l;
	sprintf(str_tmp,"%sDATA/music.zip",LaunchDir);
		
	zip_file = 0;    
	menu_music=0;
	
	zip_file = unzOpen(str_tmp);
	if (zip_file) {
		unz_global_info pglobal_info;
		int num;
		struct timeval now;
		sceKernelLibcGettimeofday( &now, 0 );		
		srand((now.tv_usec+now.tv_sec*1000000));
		
		unzGetGlobalInfo(zip_file,&pglobal_info);		
		do {
			num=rand()%pglobal_info.number_entry;
		} while (num==menu_musicidx);
		menu_musicidx=num;
						
		unzGoToFirstFile(zip_file);
		while (num--) {
			unzGoToNextFile(zip_file);
		}
		if (unzGetCurrentFileInfo(zip_file, &unzinfo, str_tmp, sizeof(str_tmp), NULL, NULL, NULL, NULL) != UNZ_OK) {
			return;
		}
		unzOpenCurrentFile (zip_file);
		
		
		
		menu_musiclen = unzinfo.uncompressed_size;
		menu_musicdata = (uint8*)malloc(menu_musiclen);
		unzReadCurrentFile(zip_file,(void*)(menu_musicdata), menu_musiclen, NULL);
    unzCloseCurrentFile (zip_file);
    unzClose (zip_file);
							
		menu_music=1;
#ifndef ME_SOUND
		scePowerSetClockFrequency(300,300,150);
#endif		
		OSPC_PlayBuffer(menu_musicdata,menu_musiclen,1,MAXVOLUME*2/3);
		
		if (strlen(OSPC_GameTitle())) strcpy(menu_music_gametitle,OSPC_GameTitle());
		else strncpy(menu_music_gametitle,str_tmp,33);
		if (strlen(OSPC_SongName())) strcpy(menu_music_songname,OSPC_SongName());
		else strcpy(menu_music_songname,psp_msg_string(MENU_MISC_BGMUSIC_UNKNOWN));
		if (strlen(OSPC_Author())) strcpy(menu_music_author,OSPC_Author());
		else strcpy(menu_music_author,psp_msg_string(MENU_MISC_BGMUSIC_UNKNOWN));
		
		menu_music_panel_size=mh_length(menu_music_gametitle)+mh_length(psp_msg_string(MENU_MISC_BGMUSIC_GAMETITLE))+10;
		l=mh_length(menu_music_songname)+mh_length(psp_msg_string(MENU_MISC_BGMUSIC_TITLE))+10;
		if (l>menu_music_panel_size) menu_music_panel_size=l;			
		l=mh_length(menu_music_author)+mh_length(psp_msg_string(MENU_MISC_BGMUSIC_AUTHOR))+10;
		if (l>menu_music_panel_size) menu_music_panel_size=l;
		
		menu_music_panel_pos=-menu_music_panel_size;
		menu_music_panel_mode=1;
		menu_cnt3=0;
	}
}

void menu_stopmusic() {
	if (!menu_music) return;
	OSPC_StopPlay();
	free(menu_musicdata);
	menu_music=0;
#ifndef ME_SOUND
	scePowerSetClockFrequency(222,222,111);		
#endif
}

int menu_credits(char *mode) {		
	int retval=0;
	int to_exit=0;	
	int oldmenu_music;
	if (mode) {mode[0]=0;return 0;}
	
	menu_panel_pos=479;
	menu_cnt2=0;
	
	oldmenu_music=menu_music;
	if (!menu_music) menu_startmusic();	
	for (;;) {		
		menu_basic(2+to_exit);
		if (!g_bLoop) {retval=1;break;} 
		
		mh_printLimit(menu_panel_pos+5,15,479,272,"Code : YoyoFR & Laxer3a",CODE_COL);
		mh_printLimit(menu_panel_pos+5,25,479,272,"Mod : 173210, Ruka & 33(76)",CODE_COL);
		if (menu_panel_pos+5+40<479-40) pgDrawFrame(menu_panel_pos+5+40,40,479-40,40,CODE2_COL);
		if (menu_panel_pos+5+40<479-40) pgDrawFrame(menu_panel_pos+5+40,41,479-40,41,CODE3_COL);
		
		mh_printLimit(menu_panel_pos+5,50,479,272,"Splines based on PSPSDK samples",GFX_COL);
		mh_printLimit(menu_panel_pos+5,60,479,272,"Logo, some bgs : Pochi",GFX_COL);
		mh_printLimit(menu_panel_pos+5,70,479,272,"Other bgs from internet",GFX_COL);
		mh_printLimit(menu_panel_pos+5,80,479,272,"(various artists)",GFX_COL);
		mh_printLimit(menu_panel_pos+5,90,479,272,"Icons : David Vignoni",GFX_COL);
		mh_printLimit(menu_panel_pos+5,100,479,272,"        (KDE nuvola theme)",GFX_COL);
		if (menu_panel_pos+5+40<479-40) pgDrawFrame(menu_panel_pos+5+40,115,479-40,115,GFX2_COL);
		if (menu_panel_pos+5+40<479-40) pgDrawFrame(menu_panel_pos+5+40,116,479-40,116,GFX3_COL);

		mh_printLimit(menu_panel_pos+5,123,479,272,"Greetings to (no order) : ",GREETINGS0_COL);
		mh_printLimit(menu_panel_pos+5,135,479,272,"snes9x authors",GREETINGS_COL);
		mh_printLimit(menu_panel_pos+5,145,479,272,"bifuteki (first snes9x port on PSP)",GREETINGS_COL);
		mh_printLimit(menu_panel_pos+5,155,479,272,"zelurker",GREETINGS_COL);		
		mh_printLimit(menu_panel_pos+5,165,479,272,"tinnus",GREETINGS_COL);
		mh_printLimit(menu_panel_pos+5,175,479,272,"thunderz",GREETINGS_COL);
		mh_printLimit(menu_panel_pos+5,185,479,272,"smith",GREETINGS_COL);
		mh_printLimit(menu_panel_pos+5,195,479,272,"Y",GREETINGS_COL);
		mh_printLimit(menu_panel_pos+5,205,479,272,"radius",GREETINGS_COL);
		mh_printLimit(menu_panel_pos+5,215,479,272,"Chris Swindle",GREETINGS_COL);
		mh_printLimit(menu_panel_pos+5,225,479,272,"Danzel",GREETINGS_COL);		
		mh_printLimit(menu_panel_pos+5,235,479,272,"ps2dev community, psp homebrew coders",GREETINGS_COL);
		mh_printLimit(menu_panel_pos+5,245,479,272,"donators & supporters ^=^",GREETINGS_COL);



    if (to_exit) {
    	if (menu_panel_pos>=479) break;
    } else {
    	if (new_pad&(PSP_CTRL_CROSS|PSP_CTRL_LEFT)) {
    		os9x_beep1();
    		to_exit=1;
    		menu_cnt2=0;
    	}

    	if (new_pad & PSP_CTRL_SELECT) {
					menu_stopmusic();
					menu_startmusic();
			} SNAPSHOT_CODE()
    }
    //swap screen
		pgScreenFlipV2();
	}

	if (!oldmenu_music) menu_stopmusic();
	return retval;
}

int menu_versioninfos(char *mode) {
	int to_exit=0;
	int oldmode;
	int retval=0;

	if (mode) {mode[0]=0;return 0;}

	menu_panel_pos=479;
	menu_cnt2=0;

	oldmode=os9x_apuenabled;
	for (;;) {
		menu_basic(2+to_exit);
		if (!g_bLoop) {retval=1;break;}

		mh_printLimit(menu_panel_pos+5,104,479,272,EMUNAME_VERSION,CODE_COL);
		sprintf(str_tmp,psp_msg_string(MENU_ABOUT_VERSION_TIMESTAMP),__TIMESTAMP__);
		mh_printLimit(menu_panel_pos+5,104+15,479,272,str_tmp,GFX_COL);
		sprintf(str_tmp,psp_msg_string(MENU_ABOUT_VERSION_GCCVER),__VERSION__);
		mh_printLimit(menu_panel_pos+5,104+30,479,272,str_tmp,GREETINGS0_COL);
		sprintf(str_tmp,"CRC32 : %#X",g_ROMCRC32);
		mh_printLimit(menu_panel_pos+5,104+45,479,272,str_tmp,GREETINGS0_COL);

		if (to_exit) {
			if (menu_panel_pos>=479) return 0;
		} else {
			if (new_pad&(PSP_CTRL_CROSS|PSP_CTRL_LEFT)) {
				os9x_beep1();
				to_exit=1;
				menu_cnt2=0;
			} SNAPSHOT_CODE()
		}
		//swap screen
		pgScreenFlipV2();
	}

	return retval;
}

int menu_fpslimit(char *mode) {
	int retval=0;
	int to_exit=0;
	int new_value=os9x_fpslimit;
	if (mode) {				
		if (os9x_fpslimit) sprintf(mode,"%dfps",os9x_fpslimit);
		else sprintf(mode,psp_msg_string(MENU_VIDEO_SLIMITVALUE_AUTO));
		return 0;
	}
	
	menu_panel_pos=479;
	menu_cnt2=0;
	for (;;) {		
		menu_basic(2+to_exit);
		if (!g_bLoop) {retval=1;break;} 
		
		if (new_value) sprintf(str_tmp,"%dfps",new_value);
		else sprintf(str_tmp,psp_msg_string(MENU_VIDEO_SLIMITVALUE_AUTO));
		mh_printLimit(menu_panel_pos+5,104,479,272,str_tmp,((31)|(24<<5)|(24<<10)));
    mh_printLimit(menu_panel_pos+5,130,479,272,psp_msg_string(MENU_CHANGE_VALUE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,130,479,272,SJIS_UP " " SJIS_DOWN "                L,R",PANEL_BUTTONCMD_COL);    
    mh_printLimit(menu_panel_pos+5,140,479,272,psp_msg_string(MENU_DEFAULT_VALUE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,140,479,272,SJIS_TRIANGLE ,PANEL_BUTTONCMD_COL);    
    mh_printLimit(menu_panel_pos+5,150,479,272,psp_msg_string(MENU_CANCEL_VALIDATE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,150,479,272,SJIS_LEFT "   " SJIS_CROSS "              " SJIS_CIRCLE,PANEL_BUTTONCMD_COL);    
        
    if (to_exit) {
    	if (menu_panel_pos>=479) return 0;
    } else {
    	if (new_pad&(PSP_CTRL_CROSS|PSP_CTRL_LEFT)) {
    		os9x_beep1();
    		to_exit=1;
    		menu_cnt2=0;
    	}	else if (new_pad&PSP_CTRL_CIRCLE) {
	    	os9x_beep1();
	    	to_exit=1;
	    	menu_cnt2=0;
	    	os9x_fpslimit=new_value;
	    } else if (new_pad&PSP_CTRL_DOWN) {    	
    		if (new_value) {new_value--;MENU_CHGVAL();}
    		if (new_value<10) new_value=0;
    	} else if (new_pad&PSP_CTRL_UP) {    	
	    	if (new_value<120) {new_value++;MENU_CHGVAL();}
	    	if (new_value<10) new_value=10;
    	} else if (new_pad&PSP_CTRL_LTRIGGER) {    	
    		if (new_value>=5) {new_value-=5;MENU_CHGVAL();}
    		if (new_value<10) new_value=0;
    	} else if (new_pad&PSP_CTRL_RTRIGGER) {    	
	    	if (new_value<120) {new_value+=5;MENU_CHGVAL();}
	    	if (new_value<10) new_value=10;
	    		if (new_value>120) new_value=120;
    	} else if (new_pad&PSP_CTRL_TRIANGLE) {    	
	    	new_value=0;
    	} else if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
			 }  SNAPSHOT_CODE()
    }
    //swap screen
		pgScreenFlipV2();		
	}
	return retval;
}

int menu_apuratio(char *mode) {		
	int retval=0;
	int to_exit=0;
	int new_value=os9x_apu_ratio;
	if (mode) {				
		sprintf(mode,"%.2f%%",(float)os9x_apu_ratio*100.0f/256.0f);
		return 0;
	}
	
	menu_panel_pos=479;
	menu_cnt2=0;
	for (;;) {		
		menu_basic(2+to_exit);
		if (!g_bLoop) {retval=1;break;} 
		
		sprintf(str_tmp,"%.2f%%",(float)new_value*100.0f/256.0f);
		
		mh_printLimit(menu_panel_pos+5,104,479,272,str_tmp,((31)|(24<<5)|(24<<10)));
    mh_printLimit(menu_panel_pos+5,130,479,272,psp_msg_string(MENU_CHANGE_VALUE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,130,479,272,SJIS_UP " " SJIS_DOWN "                L,R",PANEL_BUTTONCMD_COL);    
    mh_printLimit(menu_panel_pos+5,140,479,272,psp_msg_string(MENU_DEFAULT_VALUE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,140,479,272,SJIS_TRIANGLE ,PANEL_BUTTONCMD_COL);    
    mh_printLimit(menu_panel_pos+5,150,479,272,psp_msg_string(MENU_CANCEL_VALIDATE),PANEL_TEXTCMD_COL);
    mh_printLimit(menu_panel_pos+5,150,479,272,SJIS_LEFT "   " SJIS_CROSS "              " SJIS_CIRCLE,PANEL_BUTTONCMD_COL);    
        
    if (to_exit) {
    	if (menu_panel_pos>=479) return 0;
    } else {
    	if (new_pad&(PSP_CTRL_CROSS|PSP_CTRL_LEFT)) {
    		os9x_beep1();
    		to_exit=1;
    		menu_cnt2=0;
    	} else if (new_pad&PSP_CTRL_CIRCLE) {
	    	os9x_beep1();
	    	to_exit=1;
	    	menu_cnt2=0;
	    	os9x_apu_ratio=new_value;
	    } else if (new_pad&PSP_CTRL_DOWN) {    	
    		if (new_value>16) {new_value--;MENU_CHGVAL();}    		
    	} else if (new_pad&PSP_CTRL_UP) {    	
	    	if (new_value<512) {new_value++;MENU_CHGVAL();}	    	
    	} else if (new_pad&PSP_CTRL_LTRIGGER) {    	
    		if (new_value>16) {new_value-=16;MENU_CHGVAL();}    		
    		if (new_value<16) {new_value=16;}
    	} else if (new_pad&PSP_CTRL_RTRIGGER) {    	
	    	if (new_value<512) {new_value+=16;MENU_CHGVAL();}	    	
	    	if (new_value>512) {new_value=512;}
    	}  else if (new_pad&PSP_CTRL_TRIANGLE) {    	
	    	new_value=256;
    	} else if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
			 }  SNAPSHOT_CODE()
    }
    //swap screen
		pgScreenFlipV2();		
	}
	return retval;
}

int menu_buildbg() {
	u16 *src,*dst;
	int i;

	dst=menu_bg;
	show_background(bg_img_mul,(os9x_lowbat?0x600000:0));
	pgDrawFrame(0,10,479,10,(4<<10)|(8<<5)|8);
	pgDrawFrame(0,11,479,12,(30<<10)|(30<<5)|30);
	pgDrawFrame(0,13,479,13,(4<<10)|(8<<5)|8);
	pgDrawFrame(0,272-14,479,272-14,(4<<10)|(2<<5)|4);
	pgDrawFrame(0,272-13,479,272-12,(30<<10)|(30<<5)|30);
	pgDrawFrame(0,272-11,479,272-11,(4<<10)|(2<<5)|4);

	pgFillBoxHalfer(0,0,479,9);
	pgFillBoxHalfer(0,272-10,479,271);
	sprintf(str_tmp,"%s",os9x_nickname);
	mh_print(i,0,str_tmp,(24<<0)|(24<<5)|(24<<10));
	sprintf(str_tmp,"%s",EMUNAME_VERSION);
	mh_print(i+15,0,str_tmp,(31<<0)|(30<<5)|(20<<10));

	for (i=0;i<272;i++) {
		src = (u16*)pgGetVramAddr(0,i);
		memcpy(dst,src,480*2);
		dst+=480;
	}
	return 0;
}

int menu_swapbg(char *mode) {
	if (mode) {mode[0]=0;return 0;}

	if (bg_img) {
		free(bg_img->pixels);
		free(bg_img);
		bg_img=NULL;
		bg_img_num=-1;
		load_background();
		menu_buildbg();
	}
	return 0;
}

#define MENU_XMB_ENTRIES_NB (4+7+2+11+3+8+10+2)
menu_xmb_entry_t menu_xmb_entries[MENU_XMB_ENTRIES_NB]={
	// GAME
	{0,0,menu_browser,MENU_ICONS_GAME_NEW,0},
	{0,1,menu_reset,MENU_ICONS_GAME_RESET,0},
	{0,2,menu_savedefaultsetting,MENU_ICONS_GAME_DEFAULTSETTINGS,MENU_ICONS_GAME_DEFAULTSETTINGS_HELP},
	{0,3,menu_exitemu,MENU_ICONS_GAME_EXIT,0},
	// LOAD/SAVE
	{1,0,menu_loadstate,MENU_ICONS_LOADSAVE_LOADSTATE,MENU_ICONS_LOADSAVE_LOADSTATE_HELP},
	{1,1,menu_savestate,MENU_ICONS_LOADSAVE_SAVESTATE,MENU_ICONS_LOADSAVE_SAVESTATE_HELP},
	{1,2,menu_deletestate,MENU_ICONS_LOADSAVE_DELETESTATE,MENU_ICONS_LOADSAVE_DELETESTATE_HELP},
	{1,3,menu_importstate,MENU_ICONS_LOADSAVE_IMPORTSTATE,MENU_ICONS_LOADSAVE_IMPORTSTATE_HELP},
	{1,4,menu_exportS9Xstate,MENU_ICONS_LOADSAVE_EXPORTS9XSTATE,MENU_ICONS_LOADSAVE_EXPORTS9XSTATE_HELP},	
	{1,5,menu_autosavetimer,MENU_ICONS_LOADSAVE_AUTOSAVETIMER,MENU_ICONS_LOADSAVE_AUTOSAVETIMER_HELP},
	{1,6,menu_autosaveUpdSRAM,MENU_ICONS_LOADSAVE_AUTOSRAM,MENU_ICONS_LOADSAVE_AUTOSRAM_HELP},
	
	// CONTROLS	
	{2,0,show_inputsmenu,MENU_ICONS_CONTROLS_REDEFINE,MENU_ICONS_CONTROLS_REDEFINE_HELP},
	{2,1,menu_emulinput,MENU_ICONS_CONTROLS_INPUT,MENU_ICONS_CONTROLS_INPUT_HELP},	
	// VIDEO OPTIONS
	{3,0,menu_engine,MENU_ICONS_VIDEO_ENGINE,MENU_ICONS_VIDEO_ENGINE_HELP},	
	{3,1,menu_videomode,MENU_ICONS_VIDEO_MODE,MENU_ICONS_VIDEO_MODE_HELP},
	{3,2,menu_smoothing,MENU_ICONS_VIDEO_SMOOTHING,MENU_ICONS_VIDEO_SMOOTHING_HELP},	
	{3,3,menu_speedlimit,MENU_ICONS_VIDEO_SLIMITONOFF,MENU_ICONS_VIDEO_SLIMITONOFF_HELP},
	{3,4,menu_fpslimit,MENU_ICONS_VIDEO_SLIMITVALUE,MENU_ICONS_VIDEO_SLIMITVALUE_HELP},
	{3,5,menu_fskip,MENU_ICONS_VIDEO_FSKIP,MENU_ICONS_VIDEO_FSKIP_HELP},			
	{3,6,menu_vsync,MENU_ICONS_VIDEO_VSYNC,MENU_ICONS_VIDEO_VSYNC_HELP},
	{3,7,menu_palntsc,MENU_ICONS_VIDEO_PALASNTSC,MENU_ICONS_VIDEO_PALASNTSC_HELP},
	{3,8,menu_gamma,MENU_ICONS_VIDEO_GAMMA,MENU_ICONS_VIDEO_GAMMA_HELP},
	{3,9,menu_screencalibrate,MENU_ICONS_VIDEO_SCRCALIB,MENU_ICONS_VIDEO_SCRCALIB_HELP},	
	{3,10,menu_showfps,MENU_ICONS_VIDEO_SHOWFPS,MENU_ICONS_VIDEO_SHOWFPS_HELP},
	// SOUND OPTIONS
	{4,0,menu_soundmode,MENU_ICONS_SOUND_MODE,MENU_ICONS_SOUND_MODE_HELP},
	{4,1,menu_soundfreq,MENU_ICONS_SOUND_FREQ,MENU_ICONS_SOUND_FREQ_HELP},
	{4,2,menu_apuratio,MENU_ICONS_SOUND_APURATIO,MENU_ICONS_SOUND_APURATIO_HELP},	
	// MISC OPTIONS
	{5,0,menu_clockspeed,MENU_ICONS_MISC_PSPCLOCK,MENU_ICONS_MISC_PSPCLOCK_HELP},	
	{5,1,menu_viewfile,MENU_ICONS_MISC_FILEVIEWER,MENU_ICONS_MISC_FILEVIEWER_HELP},		
	{5,2,show_debugmenu,MENU_ICONS_MISC_HACKDEBUG,MENU_ICONS_MISC_HACKDEBUG_HELP},
	{5,3,menu_snapshot,MENU_ICONS_MISC_SNAPSHOT,MENU_ICONS_MISC_SNAPSHOT_HELP},
	{5,4,menu_swapbg,MENU_ICONS_MISC_RANDBG,MENU_ICONS_MISC_RANDBG_HELP},
	{5,5,menu_menumusic,MENU_ICONS_MISC_BGMUSIC,MENU_ICONS_MISC_BGMUSIC_HELP},
	{5,6,menu_menufx,MENU_ICONS_MISC_BGFX,MENU_ICONS_MISC_BGFX_HELP},	
	{5,7,menu_menupadbeep,MENU_ICONS_MISC_PADBEEP,MENU_ICONS_MISC_PADBEEP_HELP},	
	// CHEATS
	{6,0,menu_addRAWcode,MENU_ICONS_CHEATS_ADDRAW,MENU_ICONS_CHEATS_ADDRAW_HELP},
	{6,1,menu_addGGcode,MENU_ICONS_CHEATS_ADDGG,MENU_ICONS_CHEATS_ADDGG_HELP},
	{6,2,menu_addPARcode,MENU_ICONS_CHEATS_ADDPAR,MENU_ICONS_CHEATS_ADDPAR_HELP},
	{6,3,menu_addGFcode,MENU_ICONS_CHEATS_ADDGF,MENU_ICONS_CHEATS_ADDGF_HELP},
	{6,4,menu_disablecode,MENU_ICONS_CHEATS_DISABLE,MENU_ICONS_CHEATS_DISABLE_HELP},
	{6,5,menu_disableallcodes,MENU_ICONS_CHEATS_DISABLEALL,MENU_ICONS_CHEATS_DISABLEALL_HELP},
	{6,6,menu_enablecode,MENU_ICONS_CHEATS_ENABLE,MENU_ICONS_CHEATS_ENABLE_HELP},
	{6,7,menu_enableallcodes,MENU_ICONS_CHEATS_ENABLEALL,MENU_ICONS_CHEATS_ENABLEALL_HELP},
	{6,8,menu_removecode,MENU_ICONS_CHEATS_REMOVE,MENU_ICONS_CHEATS_REMOVE_HELP},
	{6,9,menu_removeallcodes,MENU_ICONS_CHEATS_REMOVEALL,MENU_ICONS_CHEATS_REMOVEALL_HELP},
	// ABOUT
	{7,0,menu_credits,MENU_ICONS_ABOUT_CREDITS,0},
	{7,1,menu_versioninfos,MENU_ICONS_ABOUT_VERSION,0},
};


void show_bg(u16 *src){
	u16 *dst;
	int i;
	if (!src) return;	
	for (i=0;i<272;i++) {
		dst = (u16*)pgGetVramAddr(0,i);						
		memcpy(dst,src,480*2);
		src+=480;
	}
}

void menu_drawFrame(int selected) {
	static int batusb_infos;
	int i,sel,x,y,j,yold,col;
	
	if (g_bSleep) {
#ifdef ME_SOUND			
		if (os9x_menumusic) menu_stopmusic();
		sceGuDisplay(0);
//20080420
//		scePowerSetClockFrequency(111,111,55); //set to 66MHz
#endif			
		while(g_bSleep) pgWaitVn(10);			//wait 16*10 ms
#ifdef ME_SOUND						
//20080420
//		scePowerSetClockFrequency(222,222,111);
		sceGuDisplay(1);
		if (os9x_menumusic) menu_startmusic();
#endif			
	}	
	
	
	show_bg(menu_bg);
			
	if (os9x_menufx) fx_main(pgGetVramAddr(0,0));
				       
  show_batteryinfo();
  show_usbinfo();
    
  if (selected==-1) return;
  
  //draw icons  
  for (i=-1;i<7;i++) {
	//for (i=-1;i<=MENU_XMB_ICONS_NB;i++) {
		sel=(i+menu_current_xmb_index+MENU_XMB_ICONS_NB)%MENU_XMB_ICONS_NB;				
		if ((sel>=0)&&(sel<MENU_XMB_ICONS_NB)) {
			x=(i+1)*64+15+menu_scrolling;
			if ((x<480-64)&&(x>12)) {				
				if (i==0) {
					if (selected) x-=64;
					
					//mh_print_light(x+5,50,menu_xmb_icons[sel].label,31|(31<<5)|(31<<10),menu_current_smoothing);										
					if (icons[sel]) {
						if (menu_scrolling) {
							int zoom_val=(menu_scrolling>0?menu_scrolling:-menu_scrolling);							
							col=zoom_val*1;
							col=(col<<16)|(col<<8)|col;
							zoom_val=256-zoom_val*2;
							image_put_transp(x+8+(48-48*zoom_val/256)/2,14+8+(48-48*zoom_val/256)/2,icons[sel],col,0,icons_col[sel],zoom_val);
						} else image_put_transp_light(x,14,icons[sel],0,0,icons_col[sel],menu_current_smoothing_icon);
					}
					
					mh_print(x+5,74,psp_msg_string(menu_xmb_icons[sel].label_id),31|(31<<5)|(31<<10));
										
					pgDrawFrame(x,70,x+60,70,30|(30<<5)|(30<<10));
					pgDrawFrame(x,71,x+60,71,20|(15<<5)|(15<<10));
					
				}
				else {
					if (selected&&(i==-1)) continue;
						
					if (icons[sel]) {						
						if (  ((menu_scrolling<0)&&(i==1)) || ((menu_scrolling>0)&&(i==-1)) ) {
							int zoom_val=(menu_scrolling>0?menu_scrolling:-menu_scrolling);							
							col=0x40-zoom_val*1;
							col=(col<<16)|(col<<8)|col;
							zoom_val=128+zoom_val*2;
							image_put_transp(x+8+(48-48*zoom_val/256)/2,14+8+(48-48*zoom_val/256)/2,icons[sel],col,0,icons_col[sel],zoom_val);
						}
						else image_put_transp(x+8+12,14+8+12,icons[sel],0x404040,0,icons_col[sel],128);
					}
						
					//mh_print(x+5,74,menu_xmb_icons[sel].label,16|(16<<5)|(16<<10));
				}
			}
		}
	}
	
	//dont draw menu netries if scrolling
	if (menu_scrolling) return;
	
	//draw menu entries
	y=-12;
	if (selected) x=30+menu_scrolling;
	else x=80+menu_scrolling;
	j=menu_current_xmb_index_entry[menu_current_xmb_index];
	
	col=abs(round(27*cos(menu_cnt2*3.14159/64)));
	if (j>=2) {//top is hidden		
		col=((col+4)<<5)+((col)<<10)|((col)<<0);
		mh_print(x-15,88,SJIS_UP " ",col);
	}
	j=0; //used to detect bottom clipping
	for (i=0;i<MENU_XMB_ENTRIES_NB;i++) {
		if (menu_xmb_entries[i].menu_family_id==menu_current_xmb_index) {
			if (menu_xmb_entries[i].menu_id<menu_current_xmb_index_entry[menu_current_xmb_index]-1) continue;
			if (menu_xmb_entries[i].menu_id==menu_current_xmb_index_entry[menu_current_xmb_index]) {
				if (y>=130) {j=1;break;}
				if (y<0) y+=12;
								
				if (menu_xmb_entries[i].menu_func) { //menu func available ?
					//check func type : submenu or value
					menu_xmb_entries[i].menu_func(str_tmp);
					if (str_tmp[0]) {//a value has to be shown
						mh_print_light(x,100+y+4,psp_msg_string(menu_xmb_entries[i].label_id),31|(31<<5)|(31<<10),menu_current_smoothing);
						mh_print(x+160,100+y+4,str_tmp,31|(31<<5)|(31<<10));
						pgDrawFrame(x,100+y+4+12,x+160+mh_length(str_tmp),100+y+4+12,(28|(28<<5)|(28<<10)));
						pgDrawFrame(x+1,100+y+4+13,x+161+mh_length(str_tmp),100+y+4+13,(12|(12<<5)|(12<<10)));
					} else {
						mh_print_light(x,100+y+4,psp_msg_string(menu_xmb_entries[i].label_id),31|(31<<5)|(24<<10),menu_current_smoothing);
					}
				}
				//check for help to display
				if (menu_xmb_entries[i].help_id) { //help label available
					msgBoxLinesRawPosLimit(280,190,200,67,psp_msg_string(menu_xmb_entries[i].help_id));
				}
								
				yold=y;
				y+=20;
			}else{
				if (y>=130) {j=1;break;}
								
				if (menu_xmb_entries[i].menu_func) { //menu func available ?
					//check func type : submenu or value
					menu_xmb_entries[i].menu_func(str_tmp);
					if (str_tmp[0]) {//a value has to be shown
						mh_print(x,100+y,psp_msg_string(menu_xmb_entries[i].label_id),20|(20<<5)|(20<<10));
						mh_print(x+160,100+y,str_tmp,20|(20<<5)|(20<<10));
					} else {
						mh_print(x,100+y,psp_msg_string(menu_xmb_entries[i].label_id),20|(20<<5)|(12<<10));
					}
				}
				
				yold=y;
				y+=12;
			}
		}
	}
	
	if (j) {//bottom is hidden
		col=(col<<5)+(10<<10)|(10<<0);
		mh_print(x-20,100+yold,SJIS_DOWN " ",col);
	}
}


void menu_inputName(char *name) {
	/*int done = 0,i,j,oldmenufx;
	// INIT OSK
	unsigned short intext[128]  = { 0 }; // text already in the edit box on start
	unsigned short outtext[128] = { 0 }; // text after input
	unsigned short desc[128]    = { 'E', 'n', 't', 'e', 'r', ' ', 'N', 'a', 'm', 'e', 0 }; // description
	SceUtilityOskData data;
	SceUtilityOskParams osk;
	
	memset(&data, 0, sizeof(data));
	data.language = os9x_language; // english
	data.lines = 1; // just online
	data.unk_24 = 1; // set to 1
	data.desc = desc;
	data.intext = intext;
	data.outtextlength = 128; // sizeof(outtext) / sizeof(unsigned short)
	data.outtextlimit = 21; // just allow n chars
	data.outtext = outtext;

	memset(intext,0,128*2);	
	for(i = 0; name[i]; i++) {
		intext[i]=name[i];
	}
	
	memset(&osk, 0, sizeof(osk));
	osk.size = sizeof(osk);
	osk.language = os9x_language;
	//if (os9x_language==LANGUAGE_JAPANESE)
	osk.buttonswap = 0;
	//else osk.buttonswap = 1;
		
	osk.unk_12 = 17; // What
	osk.unk_16 = 19; // the
	osk.unk_20 = 18; // fuck
	osk.unk_24 = 16; // ???
	osk.unk_48 = 1;
	osk.data = &data;

	// Only ascii code is handled so only the input of the small letters is printed

	int rc = sceUtilityOskInitStart(&osk);
	if(rc) {
		return 0;
	}
	oldmenufx=os9x_menufx;
	os9x_menufx=1;
	while(!done) {
		menu_basic(-1);		
		
		switch(sceUtilityOskGetStatus()) {
		case PSP_OSK_INIT :
			j=mh_length("Initializing OSK...");
			i=(480-j)/2;
			pgDrawFrame(i-5-1,125-1,i+j+5+1,145+1,12|(2<<5)|(2<<10));
  		pgDrawFrame(i-5-2,125-2,i+j+5+2,145+2,28|(10<<5)|(10<<10));      
			pgFillBox(i-5,125,i+j+5,145,(20)|(4<<5)|(4<<10));
			mh_print(i,130,"Initializing OSK...",31|(28<<5)|(24<<10));
			break;
		case PSP_OSK_VISIBLE :
			sceUtilityOskUpdate(2); // 2 is taken from ps2dev.org recommendation
			break;
		case PSP_OSK_QUIT :
			sceUtilityOskShutdownStart();
			break;
		case PSP_OSK_FINISHED :
			done = 1;
			break;
		case PSP_OSK_NONE :
		default :
			break;
		}

		pgScreenFlipV2();
	}
	
	if (data.rc==2) {				//new value input
		j=0;
		for(i = 0; data.outtext[i]; i++) {
			unsigned c = data.outtext[i];
			if(32 <= c && c <= 127) {
				//pspDebugScreenPrintf("%c", data.outtext[i]); // print ascii only
				name[j++]=c;
				if (j>=21) break;
			}
		}
		name[j]=0;
	}
	
	os9x_menufx=oldmenufx;	
	*/
	SceCtrlData paddata;
	//int oldmenufx;
	int exit_osk;
	unsigned char key,name_pos;
	//oldmenufx=os9x_menufx;
	//os9x_menufx=1;
	
//	danzeff_load16(LaunchDir);
	if (!danzeff_isinitialized()) {
		psp_msg(ERR_INIT_OSK,MSG_DEFAULT);
	} else {
		danzeff_moveTo(20,20);
		exit_osk=0;
		name_pos=0;
		while (name[name_pos]) name_pos++;		
		while (!exit_osk) {			
			menu_basic(-1);					
			sceCtrlPeekBufferPositive(&paddata, 1);
			switch (key=danzeff_readInput(paddata)) {
				case DANZEFF_START:exit_osk=1;break;
				case DANZEFF_SELECT:exit_osk=2;break;
				case 8://backspace
					if (name_pos>0) {
						name_pos--;						
					}
					name[name_pos]=0;
					break;
				default:
					if (key>=32) {
						name[name_pos]=key;
						if (name_pos<127) name_pos++;
							name[name_pos]=0;
					}
					break;
			}						
			mh_printSel_light(200,20,name,0xFFFF,menu_current_smoothing);			
			danzeff_render();				
			pgScreenFlipV2();
		}				
	}	
	//os9x_menufx=oldmenufx;
	
//	danzeff_free();
}


int root_menu(void) {
	int retval;	
	int cnt=0;
	int selected=0;
	int i;
	u16 *dst,*src;
	
	//pgFillAllvram(0);
	//wait for no input
	while (get_pad()) pgWaitV();		
	
			
	//swap screen
	pgScreenFlipV2();
	
	menu_bg=(u16*)malloc(480*272*2);
	if (!menu_bg) {
		psp_msg(ERR_OUT_OF_MEM,MSG_DEFAULT);	
		return -1;
	}
	menu_buildbg();
				
	{
		int current_fx;
		int old_menufx;		
				
	 	current_fx=rand();
#ifndef ME_SOUND
		scePowerSetClockFrequency(300,300,150);
#endif	 	
	 	old_menufx=os9x_menufx;
	 	os9x_menufx=0;
		menu_basic(0);				
		pg_drawframe^=1;
	 	
	 	switch (current_fx&1) {
	 		case 0: {		
				u16 *screen=(u16*)malloc(480*272*2);						
				if (screen) {						
					u32 *src2,*src1;
					u32 *dst32;
					int x,y,col1,col2;
					
					
					for (y=0;y<272;y++) memcpy(&(screen[y*480]),pgGetVramAddr(0,y),480*2);										
					for (i=0;i<32;i+=3) {
						for (y=0;y<272;y++) {
							pg_drawframe^=1;
							src1=(u32*)(pgGetVramAddr(0,y));
							pg_drawframe^=1;
							src2=(u32*)(&(screen[y*480]));
							dst32=(u32*)(pgGetVramAddr(0,y));
							for (x=0;x<480/2;x++) {
								col1=*src1++;
								col2=*src2++;
								*dst32++=(((((col1&(0x1f001f<<0))*i)+((col2&(0x1f001f<<0))*(i^31)))>>5)&(0x1f001f<<0)) |
									 (((((col1&(0x1f001f<<5))*i)+((col2&(0x1f001f<<5))*(i^31)))>>5)&(0x1f001f<<5)) |
									 ((((((col1&(0x1f001f<<10))>>5)*i)+(((col2&(0x1f001f<<10))>>5)*(i^31))))&(0x1f001f<<10));
							}
						}
					}
					free(screen);					
				}
				break;
			}
			case 1:{	
				int x,y;
				u16 *dst,*dst2,*src16,*src162;
				
				for (i=0;i<552;i+=2) {
					if (!((i>>1)&7)) pgWaitV();				
					dst=pgGetVramAddr(0,0);
					dst2=pgGetVramAddr(0,271);
					pg_drawframe^=1;
					src16=pgGetVramAddr(0,0);
					src162=pgGetVramAddr(0,271);
					pg_drawframe^=1;
					for (y=0;y<272;y++) {																
						x=i-y;
						if ((x>=0)&&(x<=479)) {						
							dst[x]=src16[x];
							dst[479-x]=src16[479-x];
							dst2[x]=src162[x];
							dst2[479-x]=src162[479-x];					
						}
						dst+=512;
						dst2-=512;
						src16+=512;
						src162-=512;
					}
				}
				break;
			}			
		}
#ifndef ME_SOUND
		scePowerSetClockFrequency(222,222,111);
#endif
		os9x_menufx=old_menufx;
		pg_drawframe^=1;
		pgCopyScreen();					
	}
	
	fx_init();	
	
	if (os9x_menumusic) menu_startmusic();
	
	menu_free_ram=ramAvailable()/1024;
	
	menu_scrolling=0;
	retval=0;
	pad_cnt=old_pad=new_pad=pad_cnt_acc=0;
	cheats_modified=0;
	menu_modified=0;
				
	for (;;) {		
		menu_basic(selected);
		if (!g_bLoop) {retval=1;break;}
						
		if (!menu_scrolling) {
			
			if (Cheat.num_cheats&&(menu_current_xmb_index==MENU_XMB_CHEATS)) { //cheats menu, scrolling off => drawcode list 					
				menu_alertmsg(psp_msg_string(MENU_STATUS_GENERIC_NEEDRELOAD));
									
				pgFillBoxHalfer(280,13,479,18+Cheat.num_cheats*10+3);				
				for (i=0;i<Cheat.num_cheats;i++) {
					sprintf(str_tmp,"%s:%06X -> %02X",Cheat.c[i].name,Cheat.c[i].address,Cheat.c[i].byte);
					if (Cheat.c[i].enabled) mh_print(290,18+i*10,str_tmp,CHEATS_ACTIVE_COL);
					else mh_print(290,18+i*10,str_tmp,CHEATS_DISABLED_COL);
				}
			}
									
			
			if (!selected) {
				//main menu

#ifdef HOME_HOOK
        if( readHomeButton() > 0 )
        {
			if (psp_msg(ASK_EXIT,MSG_DEFAULT)) {StopSoundThread();ExitCallback(0,0,NULL);;}
        }
#endif				
				//process input									
				if (new_pad & PSP_CTRL_SELECT) {
					if (os9x_menumusic) {
						menu_stopmusic();
						menu_startmusic();
					}
				} else if (new_pad & PSP_CTRL_CROSS){
					os9x_beep1();
					retval=0;break;
				} else if (new_pad & PSP_CTRL_CIRCLE){
					os9x_beep1();
					selected=1;
					//wait for button release
					while (get_pad()) pgWaitV();
				} else if (new_pad & PSP_CTRL_RIGHT){					
					menu_current_xmb_index++;
					if (menu_current_xmb_index>=MENU_XMB_ICONS_NB) menu_current_xmb_index=0;
					os9x_beep1();
					if (!(new_pad & PSP_CTRL_LTRIGGER)) menu_scrolling=64;
					menu_cnt2=0;					
				} else if (new_pad & PSP_CTRL_LEFT){					
					menu_current_xmb_index--;
					if (menu_current_xmb_index<0) menu_current_xmb_index=MENU_XMB_ICONS_NB-1;
					os9x_beep1();
					if (!(new_pad & PSP_CTRL_LTRIGGER)) menu_scrolling=-64;
					menu_cnt2=0;					
				} else	if (new_pad & PSP_CTRL_RTRIGGER){
					/*menu_current_xmb_index++;
					if (menu_current_xmb_index>=MENU_XMB_ICONS_NB) menu_current_xmb_index=0;
					os9x_beep1();
					menu_cnt2=0;*/
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
        	menu_modified=1;
				} else if (new_pad & PSP_CTRL_LTRIGGER){					
					/*menu_current_xmb_index--;
					if (menu_current_xmb_index<0) menu_current_xmb_index=MENU_XMB_ICONS_NB-1;
					os9x_beep1();
					menu_cnt2=0;*/
				} else if (new_pad & PSP_CTRL_UP){					
					menu_current_xmb_index_entry[menu_current_xmb_index]--;
					if (menu_current_xmb_index_entry[menu_current_xmb_index]<0) {
						if (pad_cnt>1) {
							menu_current_xmb_index_entry[menu_current_xmb_index]=menu_xmb_icons[menu_current_xmb_index].menu_entries_nb-1; 
							os9x_beep1();
						}
						else menu_current_xmb_index_entry[menu_current_xmb_index]=0;
					} else os9x_beep1();
					/*if (menu_current_xmb_index_entry[menu_current_xmb_index]>0) {
          	menu_current_xmb_index_entry[menu_current_xmb_index]--;
            os9x_beep1();
       		} else {
            if (menu_current_xmb_index>0) {
            	menu_current_xmb_index--;
            } else {
            	menu_current_xmb_index = MENU_XMB_ICONS_NB-1;
            }
            // Warning : index changed from here.
            menu_current_xmb_index_entry[menu_current_xmb_index] = menu_xmb_icons[menu_current_xmb_index].menu_entries_nb-1;
            os9x_beep1();
            menu_scrolling=-64;
            menu_cnt2=0;
       		}*/
				} else if (new_pad & PSP_CTRL_DOWN){					
					menu_current_xmb_index_entry[menu_current_xmb_index]++;
					if (menu_current_xmb_index_entry[menu_current_xmb_index]>=menu_xmb_icons[menu_current_xmb_index].menu_entries_nb) {
						if (pad_cnt>1) {
							os9x_beep1();
							menu_current_xmb_index_entry[menu_current_xmb_index]=0;
						} else menu_current_xmb_index_entry[menu_current_xmb_index]=menu_xmb_icons[menu_current_xmb_index].menu_entries_nb-1; 					
					} else os9x_beep1();
					/*if (menu_current_xmb_index_entry[menu_current_xmb_index]<menu_xmb_icons[menu_current_xmb_index].menu_entries_nb-1) {
						menu_current_xmb_index_entry[menu_current_xmb_index]++;
            os9x_beep1();
       		} else {
						// SAME CODE AS RIGHT BUT... Set last item as selected.
            if (menu_current_xmb_index<MENU_XMB_ICONS_NB-1) {
            	menu_current_xmb_index++;
            } else {
            	menu_current_xmb_index = 0;
            }
            // Warning : index changed from here.
            menu_current_xmb_index_entry[menu_current_xmb_index] = 0;
            os9x_beep1();
            menu_scrolling=64;
            menu_cnt2=0;
       		}*/
				} SNAPSHOT_CODE()
			} else {
				//swap screen : show submenu
				uint16 *src,*dst;
				src=pgGetVramAddr(0,0);
				//add the 'X to return' message at bottom
				mh_printCenter(262,psp_msg_string(MENU_STATUS_GENERIC_MSG1),INFOBAR_COL);
				pgScreenFlipV2();
				//and initiate new screen
				dst=pgGetVramAddr(0,0);
				memcpy(dst,src,LINESIZE*272*2);
				//add the 'X to return' message at bottom
				mh_printCenter(262,psp_msg_string(MENU_STATUS_GENERIC_MSG1),INFOBAR_COL);
				
				
				//call the sub-function
				int i;
				for (i=0;i<MENU_XMB_ENTRIES_NB;i++) {
					if (menu_xmb_entries[i].menu_family_id==menu_current_xmb_index) {
						if (menu_xmb_entries[i].menu_id==menu_current_xmb_index_entry[menu_current_xmb_index]) break;
					}
				}
				if (i<MENU_XMB_ENTRIES_NB) {
					if (menu_xmb_entries[i].menu_func) {
						retval=menu_xmb_entries[i].menu_func(0);
						if (os9x_menumusic&&(!menu_music)) menu_startmusic();
						if ((!os9x_menumusic)&&menu_music) menu_stopmusic();
					}
					else {
						retval=0;
						psp_msg(MENU_NOT_IMPLEMENTED,MSG_DEFAULT);
						for (;;) {
							new_pad=get_pad();
							if (new_pad&PSP_CTRL_CROSS) break;
						}
					}
				}
				//wait for release
				while (get_pad()) pgWaitV();			
				selected=0;		
				if (retval) break;	
			}
		}
			
		//swap screen
		pgScreenFlipV2();				
	}
	
	//wait for no input
	while (get_pad()) pgWaitV();		
	
	fx_close();
	free(menu_bg);
	
	menu_stopmusic();
	
	if (cheats_modified) {
		os9x_savecheats();		
	}
	

	{
		int current_fx;		
	 	current_fx=rand();
#ifndef ME_SOUND
		scePowerSetClockFrequency(300,300,150);	
#endif
		pg_drawframe^=1;						
	 	
	 	switch (current_fx&1) {
	 		case 0: {			 		
				int x,y;
				u16 *dst,*dst2;				
				for (i=0;i<552;i+=2) {
					if (!((i>>1)&7)) pgWaitV();
					dst=pgGetVramAddr(0,0);
					dst2=pgGetVramAddr(0,271);
					for (y=0;y<272;y++) {														
						x=i-y;
						if ((x>=0)&&(x<=479)) {					
							dst[x]=0;
							dst[479-x]=0;
							dst2[x]=0;
							dst2[479-x]=0;					
						}
						dst+=512;
						dst2-=512;
					}
				}
				break;
			}			
			case 1:{
				int x,y,col1;
				u32 *src1,*dst32;
				for (i=31;i>=0;i-=3) {
					for (y=0;y<272;y++) {
						pg_drawframe^=1;
						src1=(u32*)(pgGetVramAddr(0,y));
						pg_drawframe^=1;						
						dst32=(u32*)(pgGetVramAddr(0,y));
						for (x=0;x<480/2;x++) {
							col1=*src1++;							
							*dst32++=((((col1&(0x1f001f<<0))*i)>>5)&(0x1f001f<<0)) |
								 ((((col1&(0x1f001f<<5))*i)>>5)&(0x1f001f<<5)) |
								 ((((col1&(0x1f001f<<10))>>5)*i)&(0x1f001f<<10));
						}
					}
				}
				break;
			}
		}
#ifndef ME_SOUND
		scePowerSetClockFrequency(222,222,111);
#endif
		pg_drawframe^=1;
		pgCopyScreen();		
	}
		
	//exit
	return retval;
}
