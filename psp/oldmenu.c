 //#include "main.h"
#define SNAPSHOT_MENU

#define BG_FADE_CST 0x202020 //0xD2D2D2

#include "psp.h"
#include "blitter.h"
#include "snes9x.h"
#include <math.h>

#include "img_jpeg.h"

#include "help_data.c"

#define TITLE_COL ((31)|(26<<5)|(31<<10))
#define PATH_COL ((31)|(24<<5)|(28<<10))

#define FILE_COL ((20)|(20<<5)|(31<<10))
#define DIR_COL ((8)|(31<<5)|(8<<10))

#define SEL_COL ((30)|(30<<5)|(31<<10))
#define SELDIR_COL ((28)|(31<<5)|(28<<10))


#define INFOBAR_COL ((31)|(24<<5)|(20<<10))

extern int pg_drawframe;
extern int os9x_language,os9x_timezone,os9x_autosavesram;
extern int os9x_screenLeft,os9x_screenTop,os9x_screenHeight,os9x_screenWidth;
extern u16 os9x_savestate_mini[128*120];
extern u32 new_pad,old_pad;
extern int psp_vsync;
extern int os9x_BG0,os9x_BG1,os9x_BG2,os9x_BG3,os9x_OBJ,os9x_easy,os9x_fastsprite,os9x_applyhacks,os9x_hack;
extern int os9x_smoothing,os9x_softrendering,os9x_apuenabled,os9x_cpuclock,os9x_fskipvalue,os9x_render,os9x_speedlimit,os9x_sndfreq;
extern int os9x_showfps,os9x_showpass,os9x_getnewfile,os9x_gammavalue,os9x_snesheight,os9x_forcepal_ntsc;
extern int os9x_lowbat,os9x_autosavetimer;
extern char LaunchDir[256];
extern char romPath[256];
extern char lastRom[256];
extern int os9x_inputs[32];
int exit_menu,menu_modified;


extern int ExitCallback(int arg1, int arg2, void *common);

int state_slot=0;
int slot_occupied;

int snap=0;

int do_screencalibrate(){
	u16 *snes_image;
	SceCtrlData paddata;
	char st[100];
	int exit_calib=0;
	
	show_background(BG_FADE_CST,(os9x_lowbat?0x600000:0));				
  pgDrawFrame(12,12,480-12,272-12,28|(28<<5)|(28<<10));
  pgDrawFrame(11,11,480-11,272-11,30|(30<<5)|(30<<10));    
  show_batteryinfo();   
  pgScreenFlipV();
  show_background(BG_FADE_CST,(os9x_lowbat?0x600000:0));				
  pgDrawFrame(12,12,480-12,272-12,28|(28<<5)|(28<<10));
  pgDrawFrame(11,11,480-11,272-11,30|(30<<5)|(30<<10));    
  show_batteryinfo();   
  pgScreenFlipV();
	
	while (get_pad()) pgWaitV();
	
	if (os9x_softrendering<2)	snes_image=(u16*)(0x44000000+512*272*2*2);
	else snes_image=(u16*)(0x44000000+2*512*272*2+256*240*2+2*256*256*2);			
	
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
		sprintf(st,"PAD : SCREEN POSITION  " SJIS_STAR "  ANALOG STICK : SCREEN SIZE  " SJIS_STAR "  " SJIS_TRIANGLE " default  " SJIS_STAR "  " SJIS_CROSS " Exit");
		mh_print(8-1,262-1,st,(8<<10)|(8<<5)|(16<<0));
		mh_print(8,262,st,(29<<10)|(29<<5)|(31<<0));
		
		//wait for a press
			while (1) {
				sceCtrlPeekBufferPositive(&paddata, 1);
				pgWaitV();
				if (paddata.Buttons) break;
				if (paddata.Lx<0x30) break;
				if (paddata.Ly<0x30) break;
				if (paddata.Lx>0xA0) break;
				if (paddata.Ly>0xA0) break;
			}
			
			//wait for release
			//while (get_pad()) pgWaitV();
			pgWaitVn(10);
			//now analyse press
			if (paddata.Buttons & PSP_CTRL_UP) os9x_screenTop--;
			else if (paddata.Buttons & PSP_CTRL_DOWN) os9x_screenTop++;
			else if (paddata.Buttons & PSP_CTRL_LEFT) os9x_screenLeft--;
			else if (paddata.Buttons & PSP_CTRL_RIGHT) os9x_screenLeft++;
			else if (paddata.Ly<0x30) os9x_screenHeight--;
			else if (paddata.Ly>0xA0) os9x_screenHeight++;
			else if (paddata.Lx<0x30) os9x_screenWidth--;
			else if (paddata.Lx>0xA0) os9x_screenWidth++;
			else if (paddata.Buttons & PSP_CTRL_TRIANGLE) {
				os9x_screenLeft=os9x_screenTop=os9x_screenWidth=os9x_screenHeight=0;
			}
			else if (paddata.Buttons & PSP_CTRL_CROSS) exit_calib=1;
				
			if (os9x_screenWidth<-64) os9x_screenWidth=-64;
			if (os9x_screenWidth>64) os9x_screenWidth=64;
			if (os9x_screenHeight<-64) os9x_screenHeight=-64;
			if (os9x_screenHeight>64) os9x_screenHeight=64;
			if (os9x_screenTop<-64) os9x_screenTop=-64;
			if (os9x_screenTop>64) os9x_screenTop=64;
			if (os9x_screenLeft<-64) os9x_screenLeft=-64;
			if (os9x_screenLeft>64) os9x_screenLeft=64;
	}	
	while (get_pad()) pgWaitV();
	return 0;
}

int menu_exitemu(){	
	if (!inputBox("Exit snes9xTYL ?")) return 0;
	ExitCallback(0,0,NULL);
	return 1;
}

int loadstate(){
  char ext[10];
  
  if (slot_occupied) {
  	if (!inputBox("Current game progress will be lost.\nContinue ?")) return 0;  
  	msgBoxLines("Loading state...",10);
  	if (state_slot==10) strcpy(ext,".zat");
  	else {
  		strcpy(ext,".za0");
  		ext[3]=state_slot+48;
  	}
  	os9x_load(ext);
  	return 1;
  }
  msgBoxLines("No state available...",10);
  return 0;
}

int deletestate(){
  char ext[10];
  
  if (slot_occupied) {
  	if (!inputBox("Delete current state ?")) return 0;  
  	msgBoxLines("Deleting...",10);
  	if (state_slot==10) strcpy(ext,".zat");
  	else {
  		strcpy(ext,".za0");
  		ext[3]=state_slot+48;
  	}
  	os9x_remove(ext);  	
  	return 1;
  }
  msgBoxLines("No state available...",10);
  return 0;
}

int savestate(){
  char ext[10];
  if (os9x_lowbat) return;
  if (slot_occupied) {
  	if (!inputBox("Overwrite current savestate ?")) return 0;
  }
  msgBoxLines("Saving state...",10);
  if (state_slot==10) strcpy(ext,".zat");
  else {
  	strcpy(ext,".za0");
  	ext[3]=state_slot+48;
  	os9x_save(ext);
  }
  return 1;
}

int menu_reset(){	
	if (!inputBox("Reset Snes ?")) return 0;
	if (!os9x_lowbat) os9x_savesram();
  S9xReset();
  return 1;
}

int menu_browser(){
	os9x_getnewfile=1;
	return 1;
}

int menu_snapshot() {	
	msgBoxLines("Saving JPEG snapshot...",10);
	if (!os9x_lowbat) os9x_savesnap();	
	return 0;
}

int menu_importstate() {
	char statefilename[256];	
	if (getNoExtFilePath(statefilename,1)==1) {
		debug_log(statefilename);
		if (!inputBox("Current game progress will be lost.\nContinue ?")) return 0;
		msgBoxLines("Importing state...",10);
		os9x_loadfname(statefilename);
	}
	return 1;
}

int menu_savedefaultsetting(void){
	msgBoxLines("Saving current settings as default profile",30);
	if (!os9x_lowbat) save_rom_settings(0,"default");
	return 0;
}


typedef struct {
		char label[64];
		s32	help_index;
		int (*menu_func)(void);
		int	*value_int;
		int	values_list_size;
		int values_list[20];		
		int value_index;	
		char *values_list_label[20];
} menu_time_t;


int os9x_ignore_fixcol,os9x_ignore_winclip,os9x_ignore_addsub,os9x_ignore_palwrite,os9x_gfx_fastmode7,os9x_fix_hires;
int os9x_apufix;
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
	{"Fix HiRes hidden details : ",-1,NULL,&os9x_fix_hires,2,{0,1},0,{"Off","On"}},
	{"--------",-1,NULL,NULL,0,NULL,0,NULL},
	{"APU speed : ",-1,NULL,&os9x_apu_ratio,9,{128,144,160,176,192,208,224,240,256},0,{"50","56.25","62.5","68.75","75","81.25","87.5","93.75","100"}},			
	{"--------",-1,NULL,NULL,0,NULL,0,NULL},
	{"No Transparency : ",22,NULL,&os9x_easy,2,{0,1},0,{"Off","On"}},	
	{"Fast sprites : ",23,NULL,&os9x_fastsprite,2,{0,1},0,{"Off","On"}},	
	{"--------",-1,NULL,NULL,0,NULL,0,NULL},
	{"OBJ : ",17,NULL,&os9x_OBJ,2,{0,1},0,{"Off","On"}},
	{"BG0 : ",18,NULL,&os9x_BG0,2,{0,1},0,{"Off","On"}},	
	{"BG1 : ",19,NULL,&os9x_BG1,2,{0,1},0,{"Off","On"}},	
	{"BG2 : ",20,NULL,&os9x_BG2,2,{0,1},0,{"Off","On"}},	
	{"BG3 : ",21,NULL,&os9x_BG3,2,{0,1},0,{"Off","On"}}	
};


int show_debugmenu() {
	//int counter=0;
	unsigned long color=RGB_WHITE;
	static int sel=0;
	int rows=28,x, y, h, i,j,top=0,pad_cnt=0;
	int new_pad,cpt;
	int retval;		
				
	while (get_pad()) pgWaitV();	
	
	os9x_ignore_fixcol=(os9x_hack&PPU_IGNORE_FIXEDCOLCHANGES?1:0);
	os9x_ignore_winclip=(os9x_hack&PPU_IGNORE_WINDOW?1:0);
	os9x_ignore_addsub=(os9x_hack&PPU_IGNORE_ADDSUB?1:0);
	os9x_ignore_palwrite=(os9x_hack&PPU_IGNORE_PALWRITE?1:0);
	os9x_gfx_fastmode7=(os9x_hack&GFX_FASTMODE7?1:0);
	os9x_apufix=(os9x_hack&APU_FIX?1:0);
	os9x_fix_hires=(os9x_hack&HIRES_FIX?1:0);
	
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
		show_background(BG_FADE_CST,(os9x_lowbat?0x600000:0));
				
    pgDrawFrame(12,12,480-12,272-12,28|(28<<5)|(28<<10));
    pgDrawFrame(11,11,480-11,272-11,30|(30<<5)|(30<<10));
    
    show_batteryinfo();   
    
        	
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
				if ((*os9xpsp_debugmenu[sel].menu_func)()) {retval=0;break;}
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
    		os9x_beep2();
    		menu_modified=1;
    		if (os9xpsp_debugmenu[sel].value_index>0) os9xpsp_debugmenu[sel].value_index--;
    		*(os9xpsp_debugmenu[sel].value_int)=os9xpsp_debugmenu[sel].values_list[os9xpsp_debugmenu[sel].value_index];    		
    	}
    }    
    else if(new_pad & PSP_CTRL_RIGHT)    {
    	if (os9xpsp_debugmenu[sel].value_int){
    		os9x_beep2();
    		menu_modified=1;
    		if (os9xpsp_debugmenu[sel].value_index<os9xpsp_debugmenu[sel].values_list_size-1) os9xpsp_debugmenu[sel].value_index++;
    		*(os9xpsp_debugmenu[sel].value_int)=os9xpsp_debugmenu[sel].values_list[os9xpsp_debugmenu[sel].value_index];    		
    	}
    }
    #ifdef SNAPSHOT_MENU    
    else if (new_pad & PSP_CTRL_START) {    	
    	char st[256];
    	pg_drawframe^=1;    	
    	sprintf(st,"%ssnap%d.jpg",LaunchDir,snap++);
    	write_JPEG_file ((char*)st,75,pgGetVramAddr(0,0),480,272,512); 
    	pg_drawframe^=1;    	
    }
		#endif

		
		if(top > DEBUGMENU_ITEMS-rows)	top=DEBUGMENU_ITEMS-rows;
		if(top < 0)				top=0;
		if(sel >= DEBUGMENU_ITEMS)		sel=0;
		if(sel < 0)				sel=DEBUGMENU_ITEMS-1;
		if(sel >= top+rows)		top=sel-rows+1;
		if(sel < top)			top=sel;
		
       
    //pgPrint(1,0,TITLE_COL,"[" EMUNAME_VERSION "] - Menu");
    //pgPrint(1,33,INFOBAR_COL,"\1 OK \2 Main menu \5,\7 to select ,\6,\b to change value");
    //pgPrint(1,0,TITLE_COL,"[" EMUNAME_VERSION "] - Menu");
    mh_print(8,0,"[" EMUNAME_VERSION "] - Menu",TITLE_COL);
    //pgPrint(1,33,INFOBAR_COL,"\1 OK \2 Return game \5,\7 to select ,\6,\b to change value");
    {
    	char status_bar[100];
    	if (os9xpsp_debugmenu[sel].menu_func) strcpy(status_bar,SJIS_CIRCLE " OK     " SJIS_STAR "    " SJIS_CROSS " Main Menu   ");
    	else strcpy(status_bar,SJIS_CROSS " Main Menu   ");
    	if (os9xpsp_debugmenu[sel].help_index>=0) {
    		switch ((cpt>>3)&7) {
    			case 7:
    				strcat(status_bar,SJIS_STAR "      " SJIS_TRIANGLE " Help       " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 6:
    				strcat(status_bar,SJIS_STAR "     " SJIS_TRIANGLE "   Help      " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 5:
    				strcat(status_bar,SJIS_STAR "    " SJIS_TRIANGLE "     Help     " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 4:
    				strcat(status_bar,SJIS_STAR "   " SJIS_TRIANGLE "       Help    " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 3:
    				strcat(status_bar,SJIS_STAR "   " SJIS_TRIANGLE "       Help    " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 2:
    				strcat(status_bar,SJIS_STAR "    " SJIS_TRIANGLE "     Help     " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 1:
    				strcat(status_bar,SJIS_STAR "     " SJIS_TRIANGLE "   Help      " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 0:
    				strcat(status_bar,SJIS_STAR "      " SJIS_TRIANGLE " Help       " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    		}
    	}
    	else strcat(status_bar,SJIS_STAR "    " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");
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
						
		pgScreenFlipV();
		cpt++;

    if(psp_ExitCheck()) {
    	retval=-1;break;
    }
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
	return retval;	
}


#define INPUTSMENU_ITEMS 17
int inputs_up,inputs_down,inputs_left,inputs_right,inputs_A,inputs_B,inputs_X,inputs_Y;
int inputs_TL,inputs_TR,inputs_START,inputs_SELECT,inputs_MENU,inputs_TURBO;
int inputs_FSKIPINC,inputs_FSKIPDEC,inputs_GFXENGINE;
menu_time_t os9xpsp_inputsmenu[INPUTSMENU_ITEMS]={	
	{"UP : ",-1,NULL,&inputs_up,18,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","Left TRIGGER","Right TRIGGER","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","NOTE","None"}},
	{"DOWN : ",-1,NULL,&inputs_down,18,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","Left TRIGGER","Right TRIGGER","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","NOTE","None"}},
	{"LEFT : ",-1,NULL,&inputs_left,18,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","Left TRIGGER","Right TRIGGER","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","NOTE","None"}},
	{"RIGHT : ",-1,NULL,&inputs_right,18,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","Left TRIGGER","Right TRIGGER","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","NOTE","None"}},
	{"A : ",-1,NULL,&inputs_A,18,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","Left TRIGGER","Right TRIGGER","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","NOTE","None"}},
	{"B : ",-1,NULL,&inputs_B,18,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","Left TRIGGER","Right TRIGGER","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","NOTE","None"}},
	{"X : ",-1,NULL,&inputs_X,18,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","Left TRIGGER","Right TRIGGER","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","NOTE","None"}},
	{"Y : ",-1,NULL,&inputs_Y,18,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","Left TRIGGER","Right TRIGGER","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","NOTE","None"}},
	{"L : ",-1,NULL,&inputs_TL,18,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","Left TRIGGER","Right TRIGGER","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","NOTE","None"}},
	{"R : ",-1,NULL,&inputs_TR,18,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","Left TRIGGER","Right TRIGGER","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","NOTE","None"}},
	{"START : ",-1,NULL,&inputs_START,18,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","Left TRIGGER","Right TRIGGER","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","NOTE","None"}},
	{"SELECT : ",-1,NULL,&inputs_SELECT,18,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","Left TRIGGER","Right TRIGGER","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","NOTE","None"}},
	{"MENU : ",-1,NULL,&inputs_MENU,18,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","Left TRIGGER","Right TRIGGER","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","NOTE","None"}},
	{"TURBO : ",-1,NULL,&inputs_TURBO,18,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","Left TRIGGER","Right TRIGGER","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","NOTE","None"}},
	{"FRAMESKIP + : ",-1,NULL,&inputs_FSKIPINC,18,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","Left TRIGGER","Right TRIGGER","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","NOTE","None"}},
	{"FRAMESKIP - : ",-1,NULL,&inputs_FSKIPDEC,18,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","Left TRIGGER","Right TRIGGER","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","NOTE","None"}},
	{"CHANGE GPUPack.GFX ENGINE : ",-1,NULL,&inputs_GFXENGINE,18,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},0,{"UP(pad)","DOWN(pad)","LEFT(pad)","RIGHT(pad)","TRIANGLE","CIRCLE","CROSS","SQUARE","START","SELECT","Left TRIGGER","Right TRIGGER","UP(analog)","DOWN(analog)","LEFT(analog)","RIGHT(analog)","NOTE","None"}}
};


int show_inputsmenu() {
	//int counter=0;
	unsigned long color=RGB_WHITE;
	static int sel=0;
	int actual_slot;	
	int rows=28,x, y, h, i,j,top=0,pad_cnt=5;
	int new_pad,cpt,rebuild_entries;
	int retval;		
	char ext[5],slot_fileinfo[32];
	
	
		
	while (get_pad()) pgWaitV();	
	
	rebuild_entries=1;
	
	
	
	old_pad=0;
	cpt=0;
	for(;;){
		show_background(BG_FADE_CST,(os9x_lowbat?0x600000:0));
				
    pgDrawFrame(12,12,480-12,272-12,28|(28<<5)|(28<<10));
    pgDrawFrame(11,11,480-11,272-11,30|(30<<5)|(30<<10));
    
    show_batteryinfo();   
    
    if (rebuild_entries) {
    	rebuild_entries=0;
    	inputs_up=inputs_down=inputs_left=inputs_right=inputs_A=inputs_B=inputs_X=inputs_Y=17;
			inputs_TL=inputs_TR=inputs_START=inputs_SELECT=inputs_MENU=inputs_TURBO=17;
			inputs_FSKIPINC=inputs_FSKIPDEC=inputs_GFXENGINE=17;
			
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
		    
		if(new_pad & PSP_CTRL_CIRCLE){
			//if (os9xpsp_inputsmenu[sel].menu_func) 
			//	if ((*os9xpsp_inputsmenu[sel].menu_func)()) {retval=0;break;}
			char st[64];
			SceCtrlData paddata;
			menu_modified=1;
			// wait for no button pressed
			while (get_pad()) pgWaitV();
			//message asking a button press
			sprintf(st,"Press a button for %s",os9xpsp_inputsmenu[sel].label);
			msgBoxLines(st,0);
			//wait for a press
			while (1) {
				sceCtrlPeekBufferPositive(&paddata, 1);
				pgWaitV();
				if (paddata.Buttons) break;
				if (paddata.Lx<0x30) break;
				if (paddata.Ly<0x30) break;
				if (paddata.Lx>0xA0) break;
				if (paddata.Ly>0xA0) break;
			}
			//wait for release
			while (get_pad()) pgWaitV();
			//now analyse press
			if (paddata.Buttons & PSP_CTRL_UP) *(os9xpsp_inputsmenu[sel].value_int)=PSP_UP;
			else if (paddata.Buttons & PSP_CTRL_DOWN) *(os9xpsp_inputsmenu[sel].value_int)=PSP_DOWN;
			else if (paddata.Buttons & PSP_CTRL_LEFT) *(os9xpsp_inputsmenu[sel].value_int)=PSP_LEFT;
			else if (paddata.Buttons & PSP_CTRL_RIGHT) *(os9xpsp_inputsmenu[sel].value_int)=PSP_RIGHT;	
			else if (paddata.Ly<0x30) *(os9xpsp_inputsmenu[sel].value_int)=PSP_AUP;
			else if (paddata.Ly>0xA0) *(os9xpsp_inputsmenu[sel].value_int)=PSP_ADOWN;
			else if (paddata.Lx<0x30) *(os9xpsp_inputsmenu[sel].value_int)=PSP_ALEFT;
			else if (paddata.Lx>0xA0) *(os9xpsp_inputsmenu[sel].value_int)=PSP_ARIGHT;				
			else if (paddata.Buttons & PSP_CTRL_CIRCLE) *(os9xpsp_inputsmenu[sel].value_int)=PSP_CIRCLE;
			else if (paddata.Buttons & PSP_CTRL_CROSS) *(os9xpsp_inputsmenu[sel].value_int)=PSP_CROSS;
			else if (paddata.Buttons & PSP_CTRL_SQUARE) *(os9xpsp_inputsmenu[sel].value_int)=PSP_SQUARE;
			else if (paddata.Buttons & PSP_CTRL_TRIANGLE) *(os9xpsp_inputsmenu[sel].value_int)=PSP_TRIANGLE;				
			else if (paddata.Buttons & PSP_CTRL_START) *(os9xpsp_inputsmenu[sel].value_int)=PSP_START;
			else if (paddata.Buttons & PSP_CTRL_SELECT) *(os9xpsp_inputsmenu[sel].value_int)=PSP_SELECT;
			else if (paddata.Buttons & PSP_CTRL_LTRIGGER) *(os9xpsp_inputsmenu[sel].value_int)=PSP_TL;
			else if (paddata.Buttons & PSP_CTRL_RTRIGGER) *(os9xpsp_inputsmenu[sel].value_int)=PSP_TR;
			else if (paddata.Buttons & PSP_CTRL_NOTE) *(os9xpsp_inputsmenu[sel].value_int)=PSP_NOTE;			
			os9xpsp_inputsmenu[sel].value_index=*(os9xpsp_inputsmenu[sel].value_int);
		}			
    else if(new_pad & PSP_CTRL_CROSS)   { 
    	//check menu access button
    	if (inputs_MENU==17) {
    		msgBoxLines("No button defined for MENU Access!\nPlease choose one",30);
    	} else {retval= 0;break; }
    }
    else if(new_pad & PSP_CTRL_UP)      { sel--; os9x_beep1();   }
    else if(new_pad & PSP_CTRL_DOWN)    { sel++; os9x_beep1();   }
    else if(new_pad & PSP_CTRL_LTRIGGER)      { sel-=10;if (sel<0) sel=0; os9x_beep1();   }
    else if(new_pad & PSP_CTRL_RTRIGGER)    { sel+=10;if (sel>=INPUTSMENU_ITEMS) sel=INPUTSMENU_ITEMS-1; os9x_beep1();   }
    else if(new_pad & PSP_CTRL_LEFT)    {
    	if (os9xpsp_inputsmenu[sel].value_int){
    		os9x_beep2();
    		menu_modified=1;
    		if (os9xpsp_inputsmenu[sel].value_index>0) os9xpsp_inputsmenu[sel].value_index--;
    		*(os9xpsp_inputsmenu[sel].value_int)=os9xpsp_inputsmenu[sel].values_list[os9xpsp_inputsmenu[sel].value_index];    		
    	}
    }    
    else if(new_pad & PSP_CTRL_RIGHT)    {
    	if (os9xpsp_inputsmenu[sel].value_int){
    		os9x_beep2();
    		menu_modified=1;
    		if (os9xpsp_inputsmenu[sel].value_index<os9xpsp_inputsmenu[sel].values_list_size-1) os9xpsp_inputsmenu[sel].value_index++;
    		*(os9xpsp_inputsmenu[sel].value_int)=os9xpsp_inputsmenu[sel].values_list[os9xpsp_inputsmenu[sel].value_index];    		
    	}
    }
    else if(new_pad & PSP_CTRL_SELECT) { //default profile
    	char st[256];
    	SceCtrlData paddata;
    	sprintf(st,"Choose a default profile : \n\n"\
    	 SJIS_CROSS " - Default, SNES pad mapped to PSP pad\n\n"\
    	 SJIS_CIRCLE " - Default, SNES pad mapped to PSP analog stick\n\n"\
    	 SJIS_TRIANGLE " Cancel\n\n");
    	msgBoxLines(st,0);
    	// wait for no button pressed
			while (get_pad()) pgWaitV();			
			//wait for a press
			while (1) {
				sceCtrlPeekBufferPositive(&paddata, 1);
				pgWaitV();
				if (paddata.Buttons & (PSP_CTRL_CROSS|PSP_CTRL_CIRCLE|PSP_CTRL_TRIANGLE)) break;				
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
				os9x_inputs[PSP_ALEFT]=OS9X_MENUACCESS;
				os9x_inputs[PSP_ARIGHT]=OS9X_TURBO;
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
				os9x_inputs[PSP_LEFT]=OS9X_MENUACCESS;
				os9x_inputs[PSP_RIGHT]=OS9X_TURBO;
			}			
    }
    #ifdef SNAPSHOT_MENU    
    else if (new_pad & PSP_CTRL_START) {    	
    	char st[256];
    	pg_drawframe^=1;    	
    	sprintf(st,"%ssnap%d.jpg",LaunchDir,snap++);
    	write_JPEG_file ((char*)st,75,pgGetVramAddr(0,0),480,272,512); 
    	pg_drawframe^=1;    	
    }
		#endif

		
		if(top > INPUTSMENU_ITEMS-rows)	top=INPUTSMENU_ITEMS-rows;
		if(top < 0)				top=0;
		if(sel >= INPUTSMENU_ITEMS)		sel=0;
		if(sel < 0)				sel=INPUTSMENU_ITEMS-1;
		if(sel >= top+rows)		top=sel-rows+1;
		if(sel < top)			top=sel;
		
       
    //pgPrint(1,0,TITLE_COL,"[" EMUNAME_VERSION "] - Menu");
    //pgPrint(1,33,INFOBAR_COL,"\1 OK \2 Main menu \5,\7 to select ,\6,\b to change value");
    //pgPrint(1,0,TITLE_COL,"[" EMUNAME_VERSION "] - Menu");
    mh_print(8,0,"[" EMUNAME_VERSION "] - Inputs Config",TITLE_COL);
    //pgPrint(1,33,INFOBAR_COL,"\1 OK \2 Return game \5,\7 to select ,\6,\b to change value");
    //mh_print(8,262,SJIS_CIRCLE " OK " SJIS_CROSS " Back to Game       " SJIS_STAR "       " SJIS_TRIANGLE " Help        " SJIS_STAR "       " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value",INFOBAR_COL);
    {
    	char status_bar[100];
    	/*if (os9xpsp_debugmenu[sel].menu_func) strcpy(status_bar,SJIS_CIRCLE " OK ");
    	else strcpy(status_bar,"");*/
    	strcpy(status_bar,SJIS_CIRCLE " Detect mode   " SJIS_STAR "  " SJIS_CROSS " Menu  ");
    	if (os9xpsp_inputsmenu[sel].help_index>=0) {
    		switch ((cpt>>3)&7) {
    			case 7:
    				strcat(status_bar,SJIS_STAR "      " SJIS_TRIANGLE " Help       " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 6:
    				strcat(status_bar,SJIS_STAR "     " SJIS_TRIANGLE "   Help      " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 5:
    				strcat(status_bar,SJIS_STAR "    " SJIS_TRIANGLE "     Help     " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 4:
    				strcat(status_bar,SJIS_STAR "   " SJIS_TRIANGLE "       Help    " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 3:
    				strcat(status_bar,SJIS_STAR "   " SJIS_TRIANGLE "       Help    " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 2:
    				strcat(status_bar,SJIS_STAR "    " SJIS_TRIANGLE "     Help     " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 1:
    				strcat(status_bar,SJIS_STAR "     " SJIS_TRIANGLE "   Help      " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 0:
    				strcat(status_bar,SJIS_STAR "      " SJIS_TRIANGLE " Help       " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    		}
    	}
    	else strcat(status_bar,SJIS_STAR "  " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value  " SJIS_STAR "  SELECT default profiles");
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
						
		pgScreenFlipV();
		cpt++;

    if(psp_ExitCheck()) {
    	retval=-1;break;
    }
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


#define MENU_ITEMS 36
#define MENU_ITEMS_STATE_MIN 2
#define MENU_ITEMS_STATE_MAX 5
menu_time_t os9xpsp_menu[MENU_ITEMS]={
	{"ROM browser",-1,menu_browser,NULL,0,NULL,0,NULL},
	{"--------",-1,NULL,NULL,0,NULL,0,NULL},			
	{"Current slot : ",0,NULL,&state_slot,11,{0,1,2,3,4,5,6,7,8,9,10},0,{"Slot 0","Slot 1","Slot 2","Slot 3","Slot 4","Slot 5","Slot 6","Slot 7","Slot 8","Slot 9","Autosave (timer) Slot"}},
	{"Load State",1,loadstate,NULL,0,NULL,0,NULL},
	{"Save State",2,savestate,NULL,0,NULL,0,NULL},
	{"Delete State",-1,deletestate,NULL,0,NULL,0,NULL},
	{"Import savestate",3,menu_importstate,NULL,0,NULL,0,NULL},	
	{"Autosavestate timer : ",25,NULL,&os9x_autosavetimer,11,{0,1,2,3,4,5,6,7,8,9,10},0,{"off","1min","2min","3min","4min","5min","6min","7min","8min","9min","10min"}},
	{"Autosave updated SRAM : ",26,NULL,&os9x_autosavesram,2,{0,1},0,{"off","on"}},	
	{"--------",-1,NULL,NULL,0,NULL,0,NULL},		
	{"Snapshot",4,menu_snapshot,NULL,0,NULL,0,NULL},		
	{"Reset SNES",5,menu_reset,NULL,0,NULL,0,NULL},	
	{"--------",-1,NULL,NULL,0,NULL,0,NULL},			
	{"Sound : ",6,NULL,&os9x_apuenabled,4,{0,1,2},0,{"Off","Emulated/No output","ON"}},
	{"Frequency : ",7,NULL,&os9x_sndfreq,4,{11025,22050,/*32000*/33075,44100},0,{"11Khz","22Khz","32Khz","44Khz"}},	
	{"--------",-1,NULL,NULL,0,NULL,0,NULL},
	{"GPUPack.GFX engine : ",8,NULL,&os9x_softrendering,5,{0,1,2,3,4},0,{"Approx. software","Accurate software","PSP Accelerated","PSP accel.+accu.Soft.","PSP accel.+appr.Soft."}},	
	{"Frameskip : ",9,NULL,&os9x_fskipvalue,11,{0,1,2,3,4,5,6,7,8,9,10},0,{"0","1","2","3","4","5","6","7","8","9","Auto"}},	
	{"Rendering : ",-1,NULL,&os9x_render,6,{0,1,2,3,4,5},0,{"1:1","Zoom Fit","Zoom 4/3","Zoom Wide","Full","Full Clipped"}},
	{"Render PAL as NTSC : ",24,NULL,&os9x_forcepal_ntsc,2,{0,1},0,{"Off","On"}},	
	{"Smoothing : ",-1,NULL,&os9x_smoothing,2,{0,1},0,{"Off","On"}},
	{"Speed limit : ",-1,NULL,&os9x_speedlimit,2,{0,1},0,{"Off","On"}},
	{"Video sync : ",-1,NULL,&psp_vsync,2,{0,1},0,{"Off","On"}},	
	{"Gamma correction : ",-1,NULL,&os9x_gammavalue,10,{0,1,2,3,4,5,6,7,8,9},0,NULL},
	{"Calibrate screen",-1,do_screencalibrate,NULL,0,NULL,0,NULL},
	{"--------",-1,NULL,NULL,0,NULL,0,NULL},		
	{"CPU clock : ",-1,NULL,&os9x_cpuclock,3,{222,266,333},0,{"222Mhz","266Mhz","333Mhz"}},	
	{"Show FPS : ",-1,NULL,&os9x_showfps,2,{0,1},0,{"Off","On"}},	
	{"--------",-1,NULL,NULL,0,NULL,0,NULL},	
	{"Hack/Debug menu",-1,show_debugmenu,NULL,0,NULL,0,NULL},
	{"--------",-1,NULL,NULL,0,NULL,0,NULL},	
	{"Redefine controls",-1,show_inputsmenu,NULL,0,NULL,0,NULL},
	{"--------",-1,NULL,NULL,0,NULL,0,NULL},			
	{"Save settings as DEFAULT settings",-1,menu_savedefaultsetting,NULL,0,NULL,0,NULL},
	{"--------",-1,NULL,NULL,0,NULL,0,NULL},				
	{"Exit emulator",-1,menu_exitemu,NULL,0,NULL,0,NULL}
};

void show_batteryinfo(void){
	if (scePowerIsBatteryExist()){
    	struct tm *tsys;
    	time_t cur_time;
			sceKernelLibcTime(&cur_time);						
			cur_time+=os9x_timezone*60;
			tsys=localtime(&cur_time);			
    	char bat_info[128],bat_time[16];
    	int batteryLifeTime = scePowerGetBatteryLifeTime();
    	if (batteryLifeTime>=0) sprintf(bat_time,"(%02dh%02dm)",batteryLifeTime/60, batteryLifeTime-(batteryLifeTime/60*60));
    	else bat_time[0]=0;
    	sprintf(bat_info,"%02d%c%02d Bat.:%s%s%s%02d%%%s Tmp.%dC",    		
    	tsys->tm_hour,(tsys->tm_sec&1?':':' '),tsys->tm_min,
    	(scePowerIsPowerOnline()?"Plg.":""),
    	(scePowerIsBatteryCharging()?"Chrg.":""),
	    (scePowerIsLowBattery()?"Low!":""),
	    scePowerGetBatteryLifePercent(),
	    bat_time,
	    scePowerGetBatteryTemp());	    
	    mh_print(479-strlen(bat_info)*5,0,bat_info,(22<<0)|(31<<5)|(22<<10));
		}	
}

int showmenu(void) {
	//int counter=0;
	unsigned long color=RGB_WHITE;
	static int sel=0;
	int actual_slot;	
	int rows=28,x, y, h, i,j,top=0,pad_cnt=0;
	int new_pad,cpt;
	int retval;		
	char ext[5],slot_fileinfo[32];
	int cur_engine=os9x_softrendering;
	
	u16 *minisnes_image=(u16*)malloc(128*128*2);
	
	{
		u16 *snes_image;
		int x,y,xmax=256/2,ymax=os9x_snesheight/2;
		if (cur_engine<2)	snes_image=(u16*)(0x44000000+512*272*2*2);
		else snes_image=(u16*)(0x44000000+2*512*272*2+256*240*2+2*256*256*2);			
		u16 *dst=pgGetVramAddr(0,0);
		for (y=0;y<ymax;y++)
			for (x=0;x<xmax;x++) {					
				int col2a=snes_image[(y*2)*256+(x*2)];
				int col2b=snes_image[(y*2+1)*256+(x*2)];
				int col2c=snes_image[(y*2)*256+(x*2+1)];
				int col2d=snes_image[(y*2+1)*256+(x*2+1)];					
				int col2;
				col2=((((((col2a>>10)&31)+((col2b>>10)&31)+((col2c>>10)&31)+((col2d>>10)&31))>>2)*2/3)<<10);
				col2|=((((((col2a>>5)&31)+((col2b>>5)&31)+((col2c>>5)&31)+((col2d>>5)&31))>>2)*2/3)<<5);
				col2|=((((((col2a>>0)&31)+((col2b>>0)&31)+((col2c>>0)&31)+((col2d>>0)&31))>>2)*2/3)<<0);
				minisnes_image[y*128+x]=col2;
			}
	}
	
	
			
	while (get_pad()) pgWaitV();		
	//init menu indexes			
	for (j=0;j<MENU_ITEMS;j++) {
		if (os9xpsp_menu[j].value_int) {			//menu entry has a variable
			int cur=*(os9xpsp_menu[j].value_int);    //current value
			for (i=0;i<os9xpsp_menu[j].values_list_size;i++){ //browse list to find index
    		if (cur==os9xpsp_menu[j].values_list[i]) {    			
    			os9xpsp_menu[j].value_index=i;
    			break;
    		}
      }
    }
	}		
	
	{
		if (state_slot==10) strcpy(ext,".zat");
  	else {
  		strcpy(ext,".za0");
  		ext[3]=state_slot+48;
  	}		
		slot_occupied=os9x_getstate(ext,slot_fileinfo);
		actual_slot=state_slot;
	}
	
	menu_modified=0;
	old_pad=0;
	cpt=0;
	for(;;){
		show_background(BG_FADE_CST,(os9x_lowbat?0x600000:0));
				
    pgDrawFrame(12,12,480-12,272-12,28|(28<<5)|(28<<10));
    pgDrawFrame(11,11,480-11,272-11,30|(30<<5)|(30<<10));
    
    show_batteryinfo();   
    
		new_pad=0;
    if (!pad_cnt) {
    	new_pad=get_pad();
    	
    	if (new_pad) {
				if (old_pad==new_pad) pad_cnt=1/*2*/;
				else pad_cnt=5;
			} 
			old_pad=new_pad;   	
    }
    else pad_cnt--;
    pgWaitV();
        
		
		
		    
		if(new_pad & PSP_CTRL_CIRCLE){
			if (os9xpsp_menu[sel].menu_func) 
				if ((*os9xpsp_menu[sel].menu_func)()) {retval=0;break;}
		} else if(new_pad & PSP_CTRL_TRIANGLE)   {
			if (os9xpsp_menu[sel].help_index>=0) {
				inputBoxOK(help_data[os9x_language][os9xpsp_menu[sel].help_index]);
			}
		} else if(new_pad & PSP_CTRL_CROSS)   { retval= 0;break; }
    else if(new_pad & PSP_CTRL_UP)      { sel--;os9x_beep1();    }
    else if(new_pad & PSP_CTRL_DOWN)    { sel++; os9x_beep1();   }
    else if(new_pad & PSP_CTRL_LTRIGGER)      {
    	if (sel>0) {
    	 sel--;
    	 os9x_beep1();
    	 while (sel>0) {
    	 	sel--;
    	 	if (os9xpsp_menu[sel].label[0]=='-') {sel++;break;}
    	 }
    	}
    }
    else if(new_pad & PSP_CTRL_RTRIGGER)    {
    	if (sel<MENU_ITEMS-1){
    		os9x_beep1();
    	while (sel<MENU_ITEMS-1) {
    	 	sel++;
    	 	if (os9xpsp_menu[sel].label[0]=='-') {sel++;break;}
    	 }
    	}
    }
    else if(new_pad & PSP_CTRL_LEFT)    {
    	if (os9xpsp_menu[sel].value_int){
    		os9x_beep2();
    		menu_modified=1;
    		if (os9xpsp_menu[sel].value_index>0) os9xpsp_menu[sel].value_index--;
    		else os9xpsp_menu[sel].value_index=os9xpsp_menu[sel].values_list_size-1;
    		*(os9xpsp_menu[sel].value_int)=os9xpsp_menu[sel].values_list[os9xpsp_menu[sel].value_index];    		
    	}
    }    
    else if(new_pad & PSP_CTRL_RIGHT)    {
    	if (os9xpsp_menu[sel].value_int){
    		os9x_beep2();
    		menu_modified=1;
    		if (os9xpsp_menu[sel].value_index<os9xpsp_menu[sel].values_list_size-1) os9xpsp_menu[sel].value_index++;
    		else os9xpsp_menu[sel].value_index=0;
    		*(os9xpsp_menu[sel].value_int)=os9xpsp_menu[sel].values_list[os9xpsp_menu[sel].value_index];    		
    	}
    }
#ifdef SNAPSHOT_MENU    
    else if (new_pad & PSP_CTRL_START) {    	
    	char st[256];
    	pg_drawframe^=1;    	
    	sprintf(st,"%ssnap%d.jpg",LaunchDir,snap++);
    	write_JPEG_file ((char*)st,75,pgGetVramAddr(0,0),480,272,512); 
    	pg_drawframe^=1;    	
    }
#endif
		
		if(top > MENU_ITEMS-rows)	top=MENU_ITEMS-rows;
		if(top < 0)				top=0;
		if(sel >= MENU_ITEMS)		sel=0;
		if(sel < 0)				sel=MENU_ITEMS-1;
		if(sel >= top+rows)		top=sel-rows+1;
		if(sel < top)			top=sel;
		
       
    //pgPrint(1,0,TITLE_COL,"[" EMUNAME_VERSION "] - Menu");
    mh_print(8,0,"[" EMUNAME_VERSION "] - Menu",TITLE_COL);
    
    
    //pgPrint(1,33,INFOBAR_COL,"\1 OK \2 Return game \5,\7 to select ,\6,\b to change value");
    //mh_print(8,262,SJIS_CIRCLE " OK " SJIS_CROSS " Back to Game       " SJIS_STAR "       " SJIS_TRIANGLE " Help        " SJIS_STAR "       " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value",INFOBAR_COL);    
    {
    	char status_bar[100];
    	if (os9xpsp_menu[sel].menu_func) strcpy(status_bar,SJIS_CIRCLE " OK    " SJIS_STAR "   " SJIS_CROSS " Back to Game   ");
    	else strcpy(status_bar,SJIS_CROSS " Back to Game   ");
    	
    	if (os9xpsp_menu[sel].help_index>=0) {
    		switch ((cpt>>3)&7) {
    			case 7:
    				strcat(status_bar,SJIS_STAR "      " SJIS_TRIANGLE " Help       " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 6:
    				strcat(status_bar,SJIS_STAR "     " SJIS_TRIANGLE "   Help      " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 5:
    				strcat(status_bar,SJIS_STAR "    " SJIS_TRIANGLE "     Help     " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 4:
    				strcat(status_bar,SJIS_STAR "   " SJIS_TRIANGLE "       Help    " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 3:
    				strcat(status_bar,SJIS_STAR "   " SJIS_TRIANGLE "       Help    " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 2:
    				strcat(status_bar,SJIS_STAR "    " SJIS_TRIANGLE "     Help     " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 1:
    				strcat(status_bar,SJIS_STAR "     " SJIS_TRIANGLE "   Help      " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    			case 0:
    				strcat(status_bar,SJIS_STAR "      " SJIS_TRIANGLE " Help       " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");break;
    		}
    	}
    	else strcat(status_bar,SJIS_STAR "    " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value");
    	mh_print(8,262,  status_bar,INFOBAR_COL);
		}


		if(MENU_ITEMS > rows){
			h = 219;
			pgDrawFrame(445,25,446,243,(0xd<<0)|(0xd<<5)|(0x17<<10));
			pgFillBox(448, h*top/MENU_ITEMS + 25,460, h*(top+rows)/MENU_ITEMS + 25,
				(0xd<<0)|(0xd<<5)|(0x17<<10));
			pgFillBox(448+1, h*top/MENU_ITEMS + 25+1,460-1, h*(top+rows)/MENU_ITEMS + 25 -1,
				(0x17<<0)|(0x17<<5)|(0x1f<<10));
		}
		
		x=4; y=3;
		for(i=0; i<rows; i++){
			if(top+i >= MENU_ITEMS) break;
			if(top+i == sel) color = SEL_COL;
			else			 color = FILE_COL;
												
			if (color==SEL_COL) {
				
				if (os9xpsp_menu[top+i].value_int) {
					pgPrintSel(x, y, ((30)|(30<<5)|(31<<10)), os9xpsp_menu[top+i].label);
					if (os9xpsp_menu[top+i].values_list_label[0]) {
						int ind=os9xpsp_menu[top+i].value_index;
						pgPrint(x+strlen(os9xpsp_menu[top+i].label), y, ((31)|(29<<5)|(30<<10)), os9xpsp_menu[top+i].values_list_label[ind]);					
					}	else pgPrintDecTrim(x+strlen(os9xpsp_menu[top+i].label), y, ((31)|(29<<5)|(30<<10)), *(os9xpsp_menu[top+i].value_int));
				} else if (os9xpsp_menu[top+i].menu_func) pgPrintSel(x, y, ((28)|(31<<5)|(28<<10)), os9xpsp_menu[top+i].label);
				else pgPrintSel(x, y, ((24)|(24<<5)|(24<<10)), os9xpsp_menu[top+i].label);
				
			}
			else {				
				if (os9xpsp_menu[top+i].value_int) {
					pgPrint(x, y, ((20)|(20<<5)|(31<<10)), os9xpsp_menu[top+i].label);
					if (os9xpsp_menu[top+i].values_list_label[0]) {
						int ind=os9xpsp_menu[top+i].value_index;
						pgPrint(x+strlen(os9xpsp_menu[top+i].label), y, ((31)|(24<<5)|(24<<10)), os9xpsp_menu[top+i].values_list_label[ind]);					
					}	else pgPrintDecTrim(x+strlen(os9xpsp_menu[top+i].label), y, ((31)|(24<<5)|(24<<10)), *(os9xpsp_menu[top+i].value_int));
				} else if (os9xpsp_menu[top+i].menu_func) pgPrint(x, y, ((16)|(24<<5)|(16<<10)), os9xpsp_menu[top+i].label);
				else pgPrint(x, y, ((20)|(20<<5)|(20<<10)), os9xpsp_menu[top+i].label);
			}																	
			y+=1;
		}
		
		
		{
			u16 *snes_image;
			int x,y,xmax=256/2,ymax=os9x_snesheight/2;
			if (cur_engine<2)	snes_image=(u16*)(0x44000000+512*272*2*2);
			else snes_image=(u16*)(0x44000000+2*512*272*2+256*240*2+2*256*256*2);			
			u16 *dst=pgGetVramAddr(0,0);
			if (!((sel>=MENU_ITEMS_STATE_MIN)&&(sel<=MENU_ITEMS_STATE_MAX))) {
				for (y=0;y<ymax;y++)
					for (x=0;x<xmax;x++) {
						int col1=dst[(y+13/*272-ymax-12*/)*512+x+480-xmax-12];
						/*int col2a=snes_image[(y*2)*256+(x*2)];
						int col2b=snes_image[(y*2+1)*256+(x*2)];
						int col2c=snes_image[(y*2)*256+(x*2+1)];
						int col2d=snes_image[(y*2+1)*256+(x*2+1)];					
						int col2;
						col2=((((((col2a>>10)&31)+((col2b>>10)&31)+((col2c>>10)&31)+((col2d>>10)&31))>>2)*2/3)<<10);
						col2|=((((((col2a>>5)&31)+((col2b>>5)&31)+((col2c>>5)&31)+((col2d>>5)&31))>>2)*2/3)<<5);
						col2|=((((((col2a>>0)&31)+((col2b>>0)&31)+((col2c>>0)&31)+((col2d>>0)&31))>>2)*2/3)<<0);*/
						int col2=minisnes_image[y*128+x];
						int r1,g1,b1,r2,g2,b2;
						b1=(col1>>10)&31;g1=(col1>>5)&31;r1=(col1>>0)&31;
						b2=(col2>>10)&31;g2=(col2>>5)&31;r2=(col2>>0)&31;
						b1=b1/3+b2; if (b1>31) b1=31;
						g1=g1/3+g2; if (g1>31) g1=31;
						r1=r1/3+r2; if (r1>31) r1=31;
													
						dst[(y+13/*272-ymax-12*/)*512+x+480-xmax-12]=(b1<<10)|(g1<<5)|(r1);
					}
				int r,g,b;
				float fl=sinf((float)cpt*3.14159f/64.0f);
				b=5+abs(25.0f*fl);
				g=5+abs(25.0f*fl);
				fl=sinf((float)cpt*3.14159f/48.0f);
				r=25+abs(5.0f*fl);
				int col1=r|(g<<5)|(b<<10);
				r=r*9/10;g=g*9/10;b=b*9/10;
				int col2=r|(g<<5)|(b<<10);
				/*pgDrawFrame(480-xmax-12-2,272-ymax-12-2,480-12,272-12,col2);
		  	pgDrawFrame(480-xmax-12-1,272-ymax-12-1,480-12,272-12,col1);		  	*/
		  	pgDrawFrame(480-128-12-2,11,480-12,13+os9x_snesheight/2,col2);
		  	pgDrawFrame(480-128-12-1,12,480-12,14+os9x_snesheight/2,col1);
		  	
		  	mh_print(480-128-12-5-5*strlen("[Current game]"),272-ymax-12-2-32,"[Current game]",(29<<0)|(29<<5)|(29<<10));
			} else {
				
		  	mh_print(480-128-12-5-5*strlen("[Current game]"),272-ymax-12-2-32,"[Current game]",(12<<0)|(12<<5)|(12<<10));
		  	
			}
			
		  
		  if (actual_slot!=state_slot) {
				char ext[5];
				if (state_slot==10) strcpy(ext,".zat");
  			else {
  				strcpy(ext,".za0");
  				ext[3]=state_slot+48;
  			}				
				slot_occupied=os9x_getstate(ext,slot_fileinfo);
				actual_slot=state_slot;
			}
		  
			if (slot_occupied) {
		  	u16 *dst=pgGetVramAddr(0,0);
		  	if ((sel>=MENU_ITEMS_STATE_MIN)&&(sel<=MENU_ITEMS_STATE_MAX)) {		  	
		  		for (y=0;y<os9x_snesheight/2;y++)
						for (x=0;x<128;x++) {
							int r1,g1,b1,r2,g2,b2;
							int col1=dst[(y+13)*512+x+480-128-12];
							int col2=os9x_savestate_mini[y*128+x];
							b1=(col1>>10)&31;g1=(col1>>5)&31;r1=(col1>>0)&31;
							b2=(col2>>10)&31;g2=(col2>>5)&31;r2=(col2>>0)&31;
							b1=b1/3+b2; if (b1>31) b1=31;
							g1=g1/3+g2; if (g1>31) g1=31;
							r1=r1/3+r2; if (r1>31) r1=31;												
							dst[(y+13)*512+x+480-128-12]=(b1<<10)|(g1<<5)|(r1);						
						}
					int r,g,b;
					float fl=sinf((float)cpt*3.14159f/64.0f);
					r=5+abs(25.0f*fl);
					b=5+abs(25.0f*fl);
					fl=sinf((float)cpt*3.14159f/48.0f);					
					g=25+abs(5.0f*fl);
					int col1=r|(g<<5)|(b<<10);
					b=r*9/10;g=g*9/10;b=b*9/10;
					int col2=r|(g<<5)|(b<<10);
					pgDrawFrame(480-128-12-2,11,480-12,13+os9x_snesheight/2,col2);
		  		pgDrawFrame(480-128-12-1,12,480-12,14+os9x_snesheight/2,col1);
		  		mh_print(480-128-12-5-5*strlen("[Current slot]"),14,"[Current slot]",(29<<0)|(29<<5)|(29<<10));
		  		mh_print(480-128-12-5-5*strlen(slot_fileinfo),24,slot_fileinfo,(29<<0)|(29<<5)|(29<<10));
				} else {
				
		  		mh_print(480-128-12-5-5*strlen("[Current slot]"),14,"[Current slot]",(12<<0)|(12<<5)|(12<<10));
		  	mh_print(480-128-12-5-5*strlen(slot_fileinfo),24,slot_fileinfo,(12<<0)|(12<<5)|(12<<10));
				}
		  	
		  	
		  }
		}
		
		pgScreenFlipV();
		cpt++;

    if(psp_ExitCheck()) {
    	retval=-1;break;
    }
	}
	
	while (get_pad()) pgWaitV();		
	
	//do some checking & correct values if needed
	//should be done within emu so that user can notice changes...
	if (os9x_fskipvalue==10/*AUTO_FSKIP*/) os9x_speedlimit=1;
	
	free(minisnes_image);
	
	return retval;
}




