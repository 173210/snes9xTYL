 #define PASSWORD_XORED "watashihaos9xpspdesu."

/*******************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002 Gary Henderson (gary.henderson@ntlworld.com) and
                            Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2001 - 2004 John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2004 Brad Jorsch (anomie@users.sourceforge.net),
                            funkyass (funkyass@spam.shaw.ca),
                            Joel Yliluoma (http://iki.fi/bisqwit/)
                            Kris Bleakley (codeviolation@hotmail.com),
                            Matthew Kendora,
                            Nach (n-a-c-h@users.sourceforge.net),
                            Peter Bortas (peter@bortas.org) and
                            zones (kasumitokoduck@yahoo.com)

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003 zsKnight (zsknight@zsnes.com),
                            _Demo_ (_demo_@zsnes.com), and Nach

  C4 C++ code
  (c) Copyright 2003 Brad Jorsch

  DSP-1 emulator code
  (c) Copyright 1998 - 2004 Ivar (ivar@snes9x.com), _Demo_, Gary Henderson,
                            John Weidman, neviksti (neviksti@hotmail.com),
                            Kris Bleakley, Andreas Naive

  DSP-2 emulator code
  (c) Copyright 2003 Kris Bleakley, John Weidman, neviksti, Matthew Kendora, and
                     Lord Nightmare (lord_nightmare@users.sourceforge.net

  OBC1 emulator code
  (c) Copyright 2001 - 2004 zsKnight, pagefault (pagefault@zsnes.com) and
                            Kris Bleakley
  Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002 Matthew Kendora with research by
                     zsKnight, John Weidman, and Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003 Brad Jorsch with research by
                     Andreas Naive and John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003 Feather, Kris Bleakley, John Weidman and Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003 zsKnight, _Demo_, and pagefault

  Super FX C emulator code
  (c) Copyright 1997 - 1999 Ivar, Gary Henderson and John Weidman


  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004 Marcus Comstedt (marcus@mc.pp.se)


  Specific ports contains the works of other authors. See headers in
  individual files.

  Snes9x homepage: http://www.snes9x.com

  Permission to use, copy, modify and distribute Snes9x in both binary and
  source form, for non-commercial purposes, is hereby granted without fee,
  providing that this license information and copyright notice appear with
  all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes
  charging money for Snes9x or software derived from Snes9x.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
*******************************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "snes9x.h"
#include "memmap.h"
#include "debug.h"
#include "cpuexec.h"
#include "ppu.h"
#include "snapshot.h"
#include "apu.h"
#include "display.h"
#include "gfx.h"
#include "soundux.h"
//#define SOUNDDUX_151
#include "spc700.h"
#include "psp/counter.h"
//#include "spc7110.h"

#include "psp/psp.h"
#include "psp/filer.h"
#include "psp/menu.h"
#include "psp/imageio.h"

#include "danzeff.h"


#include "psp/openspc++/os9xZ_openspc.h"

#include "tile_psp.h"

#include <pspaudio.h>

/* Define the module info section */
//PSP_MODULE_INFO("snes9xTYL", 0x1000, 0, 4);
PSP_MODULE_INFO("snes9xTYL", 0, 0, 4);
/* Define the main thread's attribute value (optional) */
//PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU);
PSP_MAIN_THREAD_STACK_SIZE_KB(256); /* smaller stack for kernel thread */
//7000 not enough
//9000 not enough rom ok next ng
//11000 user ok me anime ng
//12000 ok
//14000x
//28000x
//PSP_HEAP_SIZE_KB(12000);

#ifdef PROFILE
profile_t profile_data;
#endif
#include "psp_net.h"
extern "C" {
#include "cheats.h"

#ifdef USE_ADHOC
#include "pspadhoc.h"
#endif

#include "img_jpeg.h"

#include "psp/blitter.h"

#include "message.c"
#include "decrypt.h"

#include "cptbl.h"

#ifdef HOME_HOOK
#include "homehook.h"
#endif
#include "clock.h"
#ifdef ME_SOUND
#include "me.h"
#endif

uint32 caCRC32(uint8 *array, uint32 size, register uint32 crc32 = 0xFFFFFFFF);
int net_waitpause_state(int show_menu);
void net_send_state();
void net_receive_settings();
void net_send_settings();
void net_flush_net(int to_send);
void before_pause();
void after_pause();
void set_cpu_clock();
#ifndef FW3X
int initUSBdrivers();
int endUSBdrivers();
#endif


float vfpumulsin(float mul,float angle,float range);

int inputBoxOK(const char *msg);
void msgBox(const char *msg,int delay_vblank=10);
int msgBoxLines(const char *msg,int delay_vblank=10);
int load_rom_settings(int game_crc32);
int save_rom_settings(int game_crc32,const char *name);
void check_battery();

extern int pg_drawframe;

int menu_modified;

int bypass_rom_settings;

char os9x_devicename[256];

u16 *clut256;


int swap_buf;


static struct timeval next1 = { 0, 0 };
static struct timeval next1_autofs = { 0, 0 };

static struct timeval os9x_autosavetimer_tv = {0,0};

char *os9x_shortfilename(char *filename);
char *os9x_filename_ext(char *filename);

extern void image_put(int x0,int y0,IMAGE* img,int fade,int add);

int os9x_load(const char *ext);
int os9x_remove(const char *ext);
int os9x_loadfname(const char *fname);
int os9x_save(const char *ext);
int os9x_S9Xsave(const char *ext);
int os9x_ZSsave(const char *ext);
}


extern struct SCheatData Cheat;


const char *S9xGetSaveFilename( const char *e );

int os9x_apu_ratio,os9x_fpslimit;
int os9x_usballowed;

char str_tmp[256];
char LaunchDir[256];
char romPath[256];
char lastRom[256];
char os9x_viewfile_path[256];

char os9x_nickname[256];
int os9x_timezone,os9x_daylsavings;
int os9x_language=PSP_SYSTEMPARAM_LANGUAGE_ENGLISH; //need to be initialized for early error messages! (before calling getsysparam
int os9x_menumusic,os9x_menufx,os9x_menupadbeep;
int os9x_autostart,os9x_osk;

IMAGE* bg_img;
int bg_img_mul;
int bg_img_num;

extern "C" {
IMAGE *icons[8];
int icons_col[8];
}
u16 os9x_savestate_mini[128*120];

int os9x_notfirstlaunch;

unsigned int __attribute__((aligned(64))) list[262144*4];

void InitSoundThread();
void StopSoundThread();

int debug_count=0;
int debug_counts[100];
char debug_str_value[128]="";
char * debug_str=debug_str_value;
char me_debug_str_value[128]="";
char * me_debug_str=(char*)UNCACHE_PTR(&me_debug_str_value);
#include "gammatab.h"

#define AUTO_FSKIP 10
#define SKEEZIX_FSKIP 11
#define MAX_AUTO_SKIP 9
#define MIN_AUTOFRAME_LAG 200  //0,2 ms

int os9x_netplay,os9x_conId; //conId : 2 is client, 1 is server


uint8  __attribute__((aligned(64))) pkt_recv[NET_PKT_LEN],pkt_send[NET_PKT_LEN];

int os9x_updatepadcpt,os9x_updatepadFrame;
int os9x_padfirstcall;
int os9x_adhoc_active;
int os9x_padindex,os9x_netpadindex;
char os9x_IPaddr[32];
int os9x_vsync;
int os9x_screenLeft,os9x_screenTop,os9x_screenHeight,os9x_screenWidth;
int os9x_renderingpass,os9x_snesheight,os9x_forcepal_ntsc;
int os9x_externstate_mode,os9x_autosavesram;
int os9x_lowbat,os9x_autosavetimer;
int os9x_TurboMode;

int os9x_BG0,os9x_BG1,os9x_BG2,os9x_BG3,os9x_OBJ,os9x_easy,os9x_hack,os9x_applyhacks;
int os9x_SA1_exec;

int os9x_inputs[32],os9x_inputs_analog;
int os9x_snespad,os9x_oldsnespad,os9x_specialaction,os9x_specialaction_old;
int os9x_netsnespad[NET_DELAY][5];
int os9x_netcrc32[NET_DELAY][5];
int os9x_netsynclost;
int os9x_oldframe;

int os9x_getnewfile=0;
int os9x_showfps,os9x_showpass;
int os9x_padvalue,os9x_padvalue_ax,os9x_padvalue_ay;

int os9x_cpuclock;
extern "C" {uint32 os9x_ColorsChanged;}
uint32 os9x_gammavalue;
int os9x_fastsprite;
int32 os9x_ShowSub;
int os9x_render;
int os9x_softrendering,os9x_smoothing;

int os9x_fskipvalue;
uint32 os9x_autofskip_SkipFrames;
int os9x_CyclesPercentage;
int os9x_apuenabled;
int* os9x_apuenabled_ptr;
int os9x_DisableHDMA;
#ifdef DEBUGGER
int os9x_DisableIRQ;
#endif
int os9x_speedlimit;

int os9x_sndfreq;

char rom_filename[256];
char shortrom_filename[64];

volatile int me_resume_flag_value=0;
volatile int *me_resume_flag_p=&me_resume_flag_value;
#define me_resumeing (*me_resume_flag_p)

//int testval;

int in_emu;

/*******************************/

static struct timeval	s_tvStart;
static int				s_iFrame,s_iFrameAuto,s_iFrameReal;
int				s_TotalFrame;


int os9x_paused=0;
int *os9x_paused_ptr;

volatile int			g_bLoop = true;
volatile int				g_sndthread = -1,g_updatethread= -1,g_mainthread=-1;
volatile int g_bSleep;

static int samples_error;
static int current_SoundBuffer=0;
static u32 snd_freqratio,snd_freqerr;
volatile uint16 __attribute__((aligned(64))) SoundBuffer[4][MAX_BUFFER_SIZE];

#ifdef ME_SOUND
typedef struct {
	uint8 *buffer[4];
	int buffer_idx;
	int sample_count;
	int freqratio;
	int freqerr;
	int *os9x_apuenabled_ptr;
	int *os9x_paused_ptr;
	int exit;
	int mixsample_flag;
	//uint8 *apu_ram;
} me_sound_t;

volatile me_sound_t __attribute__((aligned(64))) me_sound_data;
volatile struct me_struct *me_data;
#endif

//static uint8 __attribute__((aligned(64))) 	GFX_Screen[SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2];
static uint8 __attribute__((aligned(64))) 	GFX_SubScreen[SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2];
static uint8 __attribute__((aligned(64))) 	GFX_ZBuffer[SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2];
static uint8 __attribute__((aligned(64))) 	GFX_SubZBuffer[SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2];


#define timercmp(a, b, CMP)	(((a)->tv_sec == (b)->tv_sec) ? ((a)->tv_usec CMP (b)->tv_usec) : ((a)->tv_sec CMP (b)->tv_sec))


void S9xCloseSoundDevice();

extern "C"{

////////////////////////////////////////////////////////////////////////////////////////
// Check for exiting flag
////////////////////////////////////////////////////////////////////////////////////////
int psp_ExitCheck(){
	return !g_bLoop;
}
void ErrorExit(const char* msg)
{
	FILE* f;
	char tmp_str[256];
	sprintf(tmp_str,"%serorr.txt",LaunchDir);
	f = fopen(tmp_str,"wb");			
	if (!f){
		//("cannot save settings");
	}
	fwrite(msg,1,strlen(msg),f);
	fclose(f);
    sceKernelExitGame();
}
void FileLog(char* msg)
{
	FILE* f;
	char tmp_str[256];
//	sprintf(tmp_str,"%serorr.txt",LaunchDir);
	sprintf(tmp_str,"%serorr.txt","ms0:/PSP/GAME380/snes9xTYL/");
	f = fopen(tmp_str,"ab");			
	if (!f){
		//("cannot save settings");
	}
	fwrite(msg,1,strlen(msg),f);
	fclose(f);
    
}
void FileDump(char* filename,void* data,int len)
{
	FILE* f;
	char tmp_str[256];
//	sprintf(tmp_str,"%serorr.txt",LaunchDir);
	sprintf(tmp_str,"%s%s","ms0:/PSP/GAME380/snes9xTYL/",filename);
	f = fopen(tmp_str,"w");			
	if (!f){
		//("cannot save settings");
	}
	fwrite(data,1,len,f);
	fclose(f);
    
}
////////////////////////////////////////////////////////////////////////////////////////
// Read inputs
////////////////////////////////////////////////////////////////////////////////////////
void update_pad(){
	SceCtrlData	ctl;
	int i,j;
	//sceCtrlReadBufferPositive( &ctl, 1 );
	sceCtrlPeekBufferPositive( &ctl, 1 );

	if (!os9x_inputs_analog) {
		if (ctl.Ly >= 0xD0) ctl.Buttons|=PSP_CTRL_DOWN;  // DOWN
  	if (ctl.Ly <= 0x30) ctl.Buttons|=PSP_CTRL_UP;    // UP
  	if (ctl.Lx <= 0x30) ctl.Buttons|=PSP_CTRL_LEFT;  // LEFT
  	if (ctl.Lx >= 0xD0) ctl.Buttons|=PSP_CTRL_RIGHT; // RIGHT
	}

	os9x_padvalue=ctl.Buttons;
	os9x_padvalue_ax=ctl.Lx;
	os9x_padvalue_ay=ctl.Ly;



	os9x_oldsnespad=os9x_snespad;
	if (os9x_netplay) {
		for (i=0;i<5;i++) {
			for (j=0;j<NET_DELAY-1;j++) {
				os9x_netsnespad[j][i]=os9x_netsnespad[j+1][i];
				os9x_netcrc32[j][i]=os9x_netcrc32[j+1][i];
			}
		}
	}

	os9x_snespad=0;
	os9x_specialaction=0;

#ifndef HOME_HOOK
	if ((os9x_inputs[PSP_TL_TR])&&(os9x_padvalue & PSP_CTRL_LTRIGGER)&&(os9x_padvalue & PSP_CTRL_RTRIGGER)) {
		os9x_snespad|=os9x_inputs[PSP_TL_TR];
		os9x_padvalue&=~(PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER);
	}
#endif
	if ((os9x_inputs[PSP_TL_START])&&(os9x_padvalue & PSP_CTRL_LTRIGGER)&&(os9x_padvalue & PSP_CTRL_START)) {
		os9x_snespad|=os9x_inputs[PSP_TL_START];
		os9x_padvalue&=~(PSP_CTRL_LTRIGGER|PSP_CTRL_START);
	}
	if ((os9x_inputs[PSP_TR_START])&&(os9x_padvalue & PSP_CTRL_RTRIGGER)&&(os9x_padvalue & PSP_CTRL_START)) {
		os9x_snespad|=os9x_inputs[PSP_TR_START];
		os9x_padvalue&=~(PSP_CTRL_RTRIGGER|PSP_CTRL_START);
	}
	if ((os9x_inputs[PSP_TL_SELECT])&&(os9x_padvalue & PSP_CTRL_LTRIGGER)&&(os9x_padvalue & PSP_CTRL_SELECT)) {
		os9x_snespad|=os9x_inputs[PSP_TL_SELECT];
		os9x_padvalue&=~(PSP_CTRL_LTRIGGER|PSP_CTRL_SELECT);
	}
	if ((os9x_inputs[PSP_TR_SELECT])&&(os9x_padvalue & PSP_CTRL_RTRIGGER)&&(os9x_padvalue & PSP_CTRL_SELECT)) {
		os9x_snespad|=os9x_inputs[PSP_TR_SELECT];
		os9x_padvalue&=~(PSP_CTRL_RTRIGGER|PSP_CTRL_SELECT);
	}
	if ((os9x_inputs[PSP_SELECT_START])&&(os9x_padvalue & PSP_CTRL_SELECT)&&(os9x_padvalue & PSP_CTRL_START)) {
		os9x_snespad|=os9x_inputs[PSP_SELECT_START];
		os9x_padvalue&=~(PSP_CTRL_SELECT|PSP_CTRL_START);
	}

	if (os9x_padvalue & PSP_CTRL_UP) os9x_snespad|=os9x_inputs[PSP_UP];
	if (os9x_padvalue & PSP_CTRL_DOWN) os9x_snespad|=os9x_inputs[PSP_DOWN];
	if (os9x_padvalue & PSP_CTRL_LEFT) os9x_snespad|=os9x_inputs[PSP_LEFT];
	if (os9x_padvalue & PSP_CTRL_RIGHT) os9x_snespad|=os9x_inputs[PSP_RIGHT];

	if (os9x_inputs_analog) {
		if (os9x_padvalue_ay<0x30) os9x_snespad|=os9x_inputs[PSP_AUP];
		if (os9x_padvalue_ay>0xA0) os9x_snespad|=os9x_inputs[PSP_ADOWN];
		if (os9x_padvalue_ax<0x30) os9x_snespad|=os9x_inputs[PSP_ALEFT];
		if (os9x_padvalue_ax>0xA0) os9x_snespad|=os9x_inputs[PSP_ARIGHT];
	}

	if (os9x_padvalue & PSP_CTRL_CIRCLE) os9x_snespad|=os9x_inputs[PSP_CIRCLE];
	if (os9x_padvalue & PSP_CTRL_CROSS) os9x_snespad|=os9x_inputs[PSP_CROSS];
	if (os9x_padvalue & PSP_CTRL_SQUARE) os9x_snespad|=os9x_inputs[PSP_SQUARE];
	if (os9x_padvalue & PSP_CTRL_TRIANGLE) os9x_snespad|=os9x_inputs[PSP_TRIANGLE];

	if (os9x_padvalue & PSP_CTRL_START) os9x_snespad|=os9x_inputs[PSP_START];
	if (os9x_padvalue & PSP_CTRL_SELECT) os9x_snespad|=os9x_inputs[PSP_SELECT];
	if (os9x_padvalue & PSP_CTRL_LTRIGGER) os9x_snespad|=os9x_inputs[PSP_TL];
	if (os9x_padvalue & PSP_CTRL_RTRIGGER) os9x_snespad|=os9x_inputs[PSP_TR];
	//if (os9x_padvalue & PSP_CTRL_NOTE) os9x_snespad|=os9x_inputs[PSP_NOTE];


	os9x_specialaction=os9x_snespad&0xFFFF0000;
	os9x_snespad&=0xFFFF;

	// NET STUFF HERE
	//  Standard PACKET is 8 bytes.
	//  Offset   What
	//  0        1 byte : command
	//  cmd = 0
	//  1				 frame number & 255.
	//	2        2*3 bytes : snes pad frame-3,snes pad frame-2,snes pad frame-1
	//
	//  cmd = 1  resync. If sent by client, ask server to do a resync
	//						  		 If sent by server, the savestate follows.
	//
	//  cmd = 2 => going to menu => pause
	//  cmd = 3 => unpause => load state
	//

	if (os9x_netplay) {
#ifdef USE_ADHOC
		char str[16];

		os9x_netcrc32[NET_DELAY-1][os9x_netpadindex]=caCRC32((uint8*)&CPUPack.Registers,sizeof(SRegisters))&0xFFFF;
		os9x_netsnespad[NET_DELAY-1][os9x_netpadindex]=os9x_snespad;

		if(os9x_conId==1) { //server
			unsigned int length;
			int ret;

			if (os9x_padfirstcall) { //first call, initiate with ingame packet

				//frame counter ok, send pad data
				pkt_send[0]=0;
				pkt_send[1]=os9x_updatepadFrame&0xFF;
				for (j=0;j<NET_DELAY-1;j++) {
					pkt_send[2+j*4]=(os9x_netsnespad[j+1][os9x_netpadindex]>>8)&0xFF;
					pkt_send[3+j*4]=os9x_netsnespad[j+1][os9x_netpadindex]&0xFF;
					pkt_send[4+j*4]=(os9x_netcrc32[j+1][os9x_netpadindex]>>8)&0xFF;
					pkt_send[5+j*4]=os9x_netcrc32[j+1][os9x_netpadindex]&0xFF;
				}

				adhocSend(pkt_send, NET_PKT_LEN);
				os9x_padfirstcall=0;
			} else { //not first call

				//1st - receive a packet
				length=NET_PKT_LEN;
				if ((ret=adhocRecvBlocked(pkt_recv, &length,RECV_MAX_RETRY))<=0) {
					//pb while receiving data
					//sprintf(str,"Lost connection! %d",ret);
					//msgBoxLines(str,60);
					//os9x_netplay=0;adhocTerm();os9x_adhoc_active=0;
					os9x_netsynclost+=NET_SYNCLOST_TS;
				} else if (length==NET_PKT_LEN) {
					//2nd - received a packet, now act accordingly
					switch (pkt_recv[0]) {
						case 0: //ingame, check frame counter
							if (pkt_recv[1]!=(os9x_oldframe&0xFF))								{
								//lost sync, ask for a reset
								//char st[64];sprintf(st,"s lost sync : %d / %d",pkt_recv[1],os9x_oldframe&0xFF);
								//msgBoxLines(st,10);
								os9x_netsynclost++;
							} else {
								//frame counter ok, get pad data
									//os9x_netsnespad[1][1]=(((int)pkt_recv[2])<<8)|pkt_recv[3];
									//os9x_netsnespad[0][1]=(((int)pkt_recv[4])<<8)|pkt_recv[5];
								for (j=0;j<NET_DELAY-1;j++) {
									os9x_netsnespad[j][1]=(((int)pkt_recv[2+j*4])<<8)|pkt_recv[3+j*4];
									os9x_netcrc32[j][1]=(((int)pkt_recv[4+j*4])<<8)|pkt_recv[5+j*4];
								}

								//compare crc32
								//only 2 players right now
								if (os9x_netcrc32[0][0]!=os9x_netcrc32[0][1]) {os9x_netsynclost++;}
								else os9x_netsynclost=0;

								pkt_send[0]=0;
								pkt_send[1]=os9x_updatepadFrame&0xFF;

								//pkt_send[2]=(os9x_netsnespad[2][os9x_netpadindex]>>8)&0xFF;pkt_send[3]=os9x_netsnespad[2][os9x_netpadindex]&0xFF;
								//pkt_send[4]=(os9x_netsnespad[1][os9x_netpadindex]>>8)&0xFF;pkt_send[5]=os9x_netsnespad[1][os9x_netpadindex]&0xFF;
								for (j=0;j<NET_DELAY-1;j++) {
									pkt_send[2+j*4]=(os9x_netsnespad[j+1][os9x_netpadindex]>>8)&0xFF;
									pkt_send[3+j*4]=os9x_netsnespad[j+1][os9x_netpadindex]&0xFF;
									pkt_send[4+j*4]=(os9x_netcrc32[j+1][os9x_netpadindex]>>8)&0xFF;
									pkt_send[5+j*4]=os9x_netcrc32[j+1][os9x_netpadindex]&0xFF;
								}

								adhocSend(pkt_send, NET_PKT_LEN);
							}
							break;
						case 1: //ask for a resync
							net_waitpause_state(0);
							break;
						case 2: { //ask for a pause
							net_waitpause_state(1);
							net_receive_settings();
							break;
						}
						case 3:
							os9x_netsynclost=NET_SYNCLOST_TS; //ask for a reset
							break;
						default: //unknown command, probably something went wrong in network link
							{char st[64];
								sprintf(st,psp_msg_string(ADHOC_UNKNOWNCOMMAND),pkt_recv[0]);
								msgBoxLines(st,60);
								//os9x_netplay=0;
								//os9x_netplay=0;adhocTerm();os9x_adhoc_active=0;
							}
					}
				}
				//handle here special stuff : reset, resync, ....
				if (os9x_netplay) {//if connection still available
					if (os9x_netsynclost>=NET_SYNCLOST_TS) {
						psp_msg(ADHOC_SYNCLOST_SERVER, MSG_DEFAULT);
						net_flush_net(1);
						before_pause();
						set_cpu_clock();
						net_send_state();
						after_pause();
					}
				}
			}
		}	else { //client
			unsigned int length;
			int ret;

			if (os9x_padfirstcall) { //first call, initiate with ingame packet

				//frame counter ok, send pad data
				pkt_send[0]=0;
				pkt_send[1]=os9x_updatepadFrame&0xFF;
				//pkt_send[2]=(os9x_netsnespad[2][os9x_netpadindex]>>8)&0xFF;pkt_send[3]=os9x_netsnespad[2][os9x_netpadindex]&0xFF;
				//pkt_send[4]=(os9x_netsnespad[1][os9x_netpadindex]>>8)&0xFF;pkt_send[5]=os9x_netsnespad[1][os9x_netpadindex]&0xFF;
				for (j=0;j<NET_DELAY-1;j++) {
					pkt_send[2+j*4]=(os9x_netsnespad[j+1][os9x_netpadindex]>>8)&0xFF;
					pkt_send[3+j*4]=os9x_netsnespad[j+1][os9x_netpadindex]&0xFF;
					pkt_send[4+j*4]=(os9x_netcrc32[j+1][os9x_netpadindex]>>8)&0xFF;
					pkt_send[5+j*4]=os9x_netcrc32[j+1][os9x_netpadindex]&0xFF;
				}

				adhocSend(pkt_send, NET_PKT_LEN);
				os9x_padfirstcall=0;
			} else {//not first call
				//1st - receive a packet

				length=NET_PKT_LEN;
				if ((ret=adhocRecvBlocked(pkt_recv, &length,RECV_MAX_RETRY))<=0) {
					//pb while receiving data
					//sprintf(str,"Lost connection! %d",ret);
					//msgBoxLines(str,60);
					//os9x_netplay=0;adhocTerm();os9x_adhoc_active=0;
					os9x_netsynclost+=NET_SYNCLOST_TS;
				} else if (length==NET_PKT_LEN) {
					//2nd - received a packet, now act accordingly

					switch (pkt_recv[0]) {
						case 0: //ingame, check frame counter
							if ((pkt_recv[1])!=(os9x_oldframe&0xFF)) {
								//lost sync, ask for a reset
								//char st[64];sprintf(st,"c lost sync : %d / %d",pkt_recv[1],os9x_oldframe&0xFF);
								//msgBoxLines(st,10);
								os9x_netsynclost++;
							} else {
								//frame counter ok, send pad data
								//frame counter ok, send pad data
								//os9x_netsnespad[1][0]=(((int)pkt_recv[2])<<8)|pkt_recv[3];
								//os9x_netsnespad[0][0]=(((int)pkt_recv[4])<<8)|pkt_recv[5];
								for (j=0;j<NET_DELAY-1;j++) {
									os9x_netsnespad[j][0]=(((int)pkt_recv[2+j*4])<<8)|pkt_recv[3+j*4];
									os9x_netcrc32[j][0]=(((int)pkt_recv[4+j*4])<<8)|pkt_recv[5+j*4];
								}

								//compare crc32
								//only 2 players right now
								if (os9x_netcrc32[0][0]!=os9x_netcrc32[0][1]) {os9x_netsynclost++;}
								else os9x_netsynclost=0;

								pkt_send[0]=0;
								pkt_send[1]=os9x_updatepadFrame&0xFF;
								//pkt_send[2]=(os9x_netsnespad[2][os9x_netpadindex]>>8)&0xFF;pkt_send[3]=os9x_netsnespad[2][os9x_netpadindex]&0xFF;
								//pkt_send[4]=(os9x_netsnespad[1][os9x_netpadindex]>>8)&0xFF;pkt_send[5]=os9x_netsnespad[1][os9x_netpadindex]&0xFF;
								for (j=0;j<NET_DELAY-1;j++) {
									pkt_send[2+j*4]=(os9x_netsnespad[j+1][os9x_netpadindex]>>8)&0xFF;
									pkt_send[3+j*4]=os9x_netsnespad[j+1][os9x_netpadindex]&0xFF;
									pkt_send[4+j*4]=(os9x_netcrc32[j+1][os9x_netpadindex]>>8)&0xFF;
									pkt_send[5+j*4]=os9x_netcrc32[j+1][os9x_netpadindex]&0xFF;
								}

								adhocSend(pkt_send,NET_PKT_LEN);
							}
							break;
						case 1: //received a resync
							net_waitpause_state(0);
							break;
						case 2: //ask for a pause
							net_waitpause_state(1);
							net_receive_settings();
							break;
						default: //unknown command, probably something went wrong in network link
							{char st[64];
								sprintf(st,psp_msg_string(ADHOC_UNKNOWNCOMMAND),pkt_recv[0]);
								msgBoxLines(st,60);
								//os9x_netplay=0;
								//os9x_netplay=0;adhocTerm();os9x_adhoc_active=0;
							}
					}
				}
				//handle here special stuff : reset, resync, ....
				if (os9x_netplay) {//if connection still available
					if (os9x_netsynclost>=NET_SYNCLOST_TS) {
						psp_msg(ADHOC_SYNCLOST_CLIENT, MSG_DEFAULT);
						pkt_send[0]=3;
						adhocSend(pkt_send,NET_PKT_LEN);
					}
				}
			}
		}
#endif
	}

	os9x_oldframe=os9x_updatepadFrame;
}
}

////////////////////////////////////////////////////////////////////////////////////////
// Power callback
////////////////////////////////////////////////////////////////////////////////////////
int PowerCallback(int unknown, int pwrflags,void *common){
	//debug_log("Power Callback");

	if (pwrflags & PSP_POWER_CB_HOLD_SWITCH) {
		//can be used for any purpose, debugging, profiling,...
	}

#ifdef ME_SOUND
	if (pwrflags & PSP_POWER_CB_POWER_SWITCH) {
		if (!g_bSleep) { //going to sleep
			g_bSleep=1;
			/**os9x_paused_ptr=1;
			StopSoundThread();
			sceGuDisplay(0);
			scePowerSetClockFrequency(33,33,16); //set to 12Mhz
			sceKernelDelayThread(1000000*3);*/
		} else { // resuming
			g_bSleep=0;
			//set_cpu_clock();
			/*scePowerSetClockFrequency(222,222,111);
			sceGuDisplay(1);

			Settings.Paused=false;*/
		}
	}
#else
	if ((!g_bSleep)&&((pwrflags & PSP_POWER_CB_POWER_SWITCH) || (pwrflags & PSP_POWER_CB_SUSPENDING))) {
		//msgBoxLines("Going into sleep mode\n\nPlease wait...",60);

		g_bSleep=1;
		*os9x_paused_ptr=1;
		StopSoundThread();
		scePowerSetClockFrequency(222,222,111);
		Settings.Paused = TRUE;
	}else if (g_bSleep&&(pwrflags & (PSP_POWER_CB_RESUME_COMPLETE|PSP_POWER_CB_RESUMING))) {
		//msgBoxLines("Resuming from sleep mode\n\nPlease wait...",60);
		g_bSleep=0;
		Settings.Paused=false;
	}
#endif
	int cbid = sceKernelCreateCallback("Power Callback", PowerCallback,NULL);
	scePowerRegisterCallback(0, cbid);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// Thread to create the callbacks and then begin polling
////////////////////////////////////////////////////////////////////////////////////////
int CallbackThread(SceSize args, void *argp){
	scePowerRegisterCallback( 0, sceKernelCreateCallback( "Power Callback",  PowerCallback,NULL ) );
	sceKernelSleepThreadCB();
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// Sets up the callback thread and returns its thread id
////////////////////////////////////////////////////////////////////////////////////////
int SetupCallbacks()
{
	int thid = 0;
	thid = sceKernelCreateThread( "update_thread", CallbackThread, 0x10, 0xFA0, PSP_THREAD_ATTR_USER, 0 );
	if( thid >= 0 ){
		sceKernelStartThread(thid, 0, 0);
	}
	return thid;
}

////////////////////////////////////////////////////////////////////////////////////////
// set psp cpu clock
////////////////////////////////////////////////////////////////////////////////////////
void set_cpu_clock(){
#ifndef ME_SOUND
		switch (os9x_cpuclock){
			case 266:scePowerSetClockFrequency(266,266,133);break;
			case 300:scePowerSetClockFrequency(300,300,150);break;
			case 333:scePowerSetClockFrequency(333,333,166);break;
			default :scePowerSetClockFrequency(222,222,111);
		}
#else
		//void PowerSetClockFrequency(int clock);
		//PowerSetClockFrequency(os9x_cpuclock);

#endif
}


extern "C" {

////////////////////////////////////////////////////////////////////////////////////////
// debug print
////////////////////////////////////////////////////////////////////////////////////////
void debug_log( const char* message ){
#ifndef RELEASE
	static int	sy = 1;

	pgPrintAllBG( SNES_WIDTH / 8, sy, 0xffff, message );
	sy++;

	if ( sy >= CMAX_Y ){
		int 	x, y;
		uint16*	dest;

		dest = (uint16*)pgGetVramAddr( SNES_WIDTH, 0 );

		for ( y = 0; y < SCREEN_HEIGHT; y++ ){
			for ( x = 0; x < (SCREEN_WIDTH - SNES_WIDTH); x++ ){
				*dest++ = 0;
			}
			dest += (512 - (SCREEN_WIDTH - SNES_WIDTH));
		}
		sy = 1;
	}
#endif // RELEASE
}

void debug_int( const char* message, int value ){
	strcpy( String, message );
	format_int( &String[strlen( String )], value );

	debug_log( String );
}
////////////////////////////////////////////////////////////////////////////////////////
// debug print hex value
////////////////////////////////////////////////////////////////////////////////////////
void debug_hex( int value ){
	int		shift;
	int		val;
	int		i;

	shift = 28;
	for ( i = 0; i < 8; i++ ){
		val = (value >> shift) & 0x0f;
		if ( val < 10 ){
			String[i] = val + '0';
		} else {
			String[i] = val - 10 + 'A';
		}
		shift -= 4;
	}
	String[i] = 0;

	debug_log( String );
}

};

//
// C++ Language
//

////////////////////////////////////////////////////////////////////////////////////////
// SRAM autosaver
////////////////////////////////////////////////////////////////////////////////////////
void S9xAutoSaveSRAM() {
	if (os9x_autosavesram&& (!os9x_lowbat)) Memory.SaveSRAM ((char*)S9xGetSaveFilename(".SRM"));
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
bool8 S9xOpenSoundDevice( int mode, bool8 stereo, int buffer_size )
{
	(stSoundStatus.mute_sound)  = TRUE;
	if ( buffer_size <= 0 ){
		return FALSE;
	}
	(stSoundStatus.sound_switch) = 255;
	(stSoundStatus.buffer_size)  = buffer_size;
	(stSoundStatus.encoded)      = FALSE;
	// Initialize channel and allocate buffer
	(stSoundStatus.sound_fd) = sceAudioChReserve( -1, buffer_size, 0 );
	if ( (stSoundStatus.sound_fd) < 0 ){
		return FALSE;
	}
		(stSoundStatus.buffer_size) *= 2;
		(stSoundStatus.buffer_size) *= 2;
	if ( (stSoundStatus.buffer_size) > MAX_BUFFER_SIZE ){
		(stSoundStatus.buffer_size) = MAX_BUFFER_SIZE;
	}
	samples_error = 0;
	current_SoundBuffer = 0;
	S9xSetPlaybackRate( Settings.SoundPlaybackRate  );
	(stSoundStatus.mute_sound)  = FALSE;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xCloseSoundDevice()
{
	if ( (stSoundStatus.sound_fd) >= 0 ){
		sceAudioChRelease( (stSoundStatus.sound_fd) );
		(stSoundStatus.sound_fd) = -1;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// Synchronous process of me-CPU and main-CPU by ruka

#ifdef ME_SOUND

#define	EVENT_SUSPEND	0
#define EVENT_RESUME	1

#define QUEUE_MASK		0x0F

typedef struct {
	uint8 wSuspend;			// me:w main:r
	uint8 wCallCount;		// me:w mian:r
	uint8  bMainQueuePtr;	// me:r main rw
	uint8  bMeQueuePtr;		// me rw
	uint32 dwTimeout;		// me:rw
	uint8 bMeWorking;		// me:w main:r
	uint8 abQueueData[31];	// me:r main:w
	uint32 dwDummy[6];		// aligned dummy
}PROCESS_EVENT;

volatile PROCESS_EVENT __attribute__((aligned(64))) g_stProcessEvent = {0}; // uncache access only
#endif




////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
#ifdef ME_SOUND

int me_Dummy(int v)
{
	return v;
}
void me_sleep(volatile int * p)
{
	volatile int dummy=0;
	if(*p){me_Dummy(*p);dummy=*p+dummy;}
}
#ifdef GFX_TILE_ME
void me_GFX_Execute();
#else
#define me_GFX_Execute()
#endif
#if 1
int me_MixSound(me_sound_t *p){
	int i;
	u32 *src,*dst;
	memcpy(&APUPack,(void*)UNCACHE_PTR(&APUPack),sizeof(struct SAPUPACK));
	memcpy(&SoundData,(void*)UNCACHE_PTR(&SoundData),sizeof(SSoundData));
	memcpy((void*)NORMAL_PTR(IAPU.RAM),(void*)UNCACHE_PTR(IAPU.RAM),0x10000);
	int apuenabled = (*(p->os9x_apuenabled_ptr)==2)&&(!(*(p->os9x_paused_ptr)));
	APUPack.apu_event1_cpt1_me=apu_event1_cpt1;
	APUPack.apu_ram_write_cpt1_me=apu_ram_write_cpt1;
		if(me_resumeing)
		{
			//continue;
		}
		//check if reset or load snapshot occured
		if (apu_init_after_load) {
			int intc = pspSdkDisableInterrupts();
   // 		if ((apu_init_after_load)&1) {
			//	S9xResetSound (TRUE);
			//	//do it each time since it's updatable from menu
			//	for (i = 0; i < 256; i++) S9xAPUCycles [i] = (int)S9xAPUCycleLengths [i] * (int)(IAPU.OneCycle) *os9x_apu_ratio / 256;
	  //  		S9xSetEchoEnable (0);
   // 		}
			//if ((apu_init_after_load)&2) {
			//	//(IAPU.PC) = (IAPU.RAM) + (APURegisters.PC);

			//	//S9xAPUUnpackStatus ();
			//	//if (APUCheckDirectPage ()) (IAPU.DirectPage) = (IAPU.RAM) + 0x100;
			//	//else(IAPU.DirectPage) = (IAPU.RAM);
			//	//S9xFixSoundAfterSnapshotLoad ();
			//	//if ((*(p->os9x_apuenabled_ptr)==2)) S9xSetSoundMute( false );
			//}
			apu_init_after_load=0;
			pspSdkEnableInterrupts( intc );
		}
		for (;;) {
		if (p->exit) break;
		me_GFX_Execute();

		if (p->mixsample_flag) {
			int next_buffer_idx = p->buffer_idx + 1;
			//p->buffer_idx++;
			if (next_buffer_idx>2) next_buffer_idx=0;

			src=(u32*)(p->buffer[3]);
			dst=(u32*)(p->buffer[next_buffer_idx]);
//			sprintf(me_debug_str,"a%d,p%d\n",*(p->os9x_apuenabled_ptr),*(p->os9x_paused_ptr));
			if (apuenabled){
			//	if(apuenabled){
				if (p->freqratio==(1<<16)) { // 1/1
					S9xMixSamples((uint8*)dst,p->sample_count);
				} else if (p->freqratio==(1<<15)) { // 1/2
					S9xMixSamples((uint8*)src,p->sample_count>>1);
	 				for (i=p->sample_count>>2;i;i--) {
			 			*dst++=*src;
		 				*dst++=*src++;
				 	}
				} else if (p->freqratio==(1<<14)) { // 1/4
			 		S9xMixSamples((uint8*)src,p->sample_count>>2);
				 	for (i=p->sample_count>>3;i;i--) {
			 			*dst++=*src;
			 			*dst++=*src;
			 			*dst++=*src;
			 			*dst++=*src++;
				 	}
				} else { //generic case, missing some real interpolation
					int samples_count_corrected;
					samples_count_corrected=((p->sample_count*(p->freqratio)+(p->freqerr))>>16);
					S9xMixSamples((uint8*)src,samples_count_corrected);
					for (i=p->sample_count>>1;i;i--) {
						*dst++=*src;
						p->freqerr+=p->freqratio;
						if ((p->freqerr)&(1<<16)) {
			  			p->freqerr&=(1<<16)-1;
				 			src++;
			 			}
				 	}
				}
			} else { // paused or apu not enabled
				//sprintf(me_debug_str,"a%d,p%d\n",*(p->os9x_apuenabled_ptr),*(p->os9x_paused_ptr));
				register int i,j;
				for (i=0;i<1024*1024;i++) j=j+1;
			}

			p->buffer_idx = next_buffer_idx;
			p->mixsample_flag=0;
		}

		APU_EXECUTE3 ();


	}

	APURegisters.PC = IAPU.PC - IAPU.RAM;
	S9xAPUPackStatus ();	
	memcpy((void*)UNCACHE_PTR(&APUPack),&APUPack,sizeof(struct SAPUPACK));
	memcpy((void*)UNCACHE_PTR(IAPU.RAM),(void*)NORMAL_PTR(IAPU.RAM),0x10000);
	memcpy((void*)UNCACHE_PTR(&SoundData),&SoundData,sizeof(SSoundData));
	apu_event1_cpt1=APUPack.apu_event1_cpt1_me;
	apu_ram_write_cpt1=APUPack.apu_ram_write_cpt1_me;
	return 1;
}
#else
int me_MixSound(me_sound_t *p){
	int i;
	u32 *src,*dst;
	uint8 *apu_ram_save;
//	Uncache_APU_Cycles=Uncache_APU_Cycles+1000;
//apu_init_after_load=0;
//	testval=testval+50000;
//return 0;

	//memcpy(&APUPack,(void*)UNCACHE_PTR(&save_APUPACK_value),sizeof(struct SAPUPACK));
	//memcpy((void*)NORMAL_PTR(IAPU.RAM),(void*)UNCACHE_PTR(save_IAPU_RAM),0x10000);
	volatile int dummy=0;
	for (;;) {
//		while(*os9x_paused_ptr){dummy=dummy+1;}
		if (p->exit) break;
		//if(*p->os9x_paused_ptr){me_sleep(p->os9x_paused_ptr);continue;}
		if(me_resumeing)continue;
		//if(stSoundStatus.mute_sound)
		//	sprintf(me_debug_str,"stSoundStatus.mute_sound:%d\n",stSoundStatus.mute_sound);	

		//check if reset or load snapshot occured
		if (apu_init_after_load) {
			int intc = pspSdkDisableInterrupts();
   // 		if ((apu_init_after_load)&1) {
			//	S9xResetSound (TRUE);
			//	//do it each time since it's updatable from menu
			//	for (i = 0; i < 256; i++) S9xAPUCycles [i] = (int)S9xAPUCycleLengths [i] * (int)(IAPU.OneCycle) *os9x_apu_ratio / 256;
	  //  		S9xSetEchoEnable (0);
   // 		}
			//if ((apu_init_after_load)&2) {
			//	(IAPU.PC) = (IAPU.RAM) + (APURegisters.PC);

			//	S9xAPUUnpackStatus ();
			//	if (APUCheckDirectPage ()) (IAPU.DirectPage) = (IAPU.RAM) + 0x100;
			//	else(IAPU.DirectPage) = (IAPU.RAM);
			//	S9xFixSoundAfterSnapshotLoad ();
			//	if ((*(p->os9x_apuenabled_ptr)==2)) S9xSetSoundMute( false );
			//}
			apu_init_after_load=0;
			pspSdkEnableInterrupts( intc );
		}


		if (p->mixsample_flag) {
			int next_buffer_idx = p->buffer_idx + 1;
			//p->buffer_idx++;
			if (next_buffer_idx>2) next_buffer_idx=0;

			src=(u32*)(p->buffer[3]);
			dst=(u32*)(p->buffer[next_buffer_idx]);
//			sprintf(me_debug_str,"a%d,p%d\n",*(p->os9x_apuenabled_ptr),*(p->os9x_paused_ptr));
			if ((*(p->os9x_apuenabled_ptr)==2)&&(!(*(p->os9x_paused_ptr))) ){
				if (p->freqratio==(1<<16)) { // 1/1
					S9xMixSamples((uint8*)dst,p->sample_count);
				} else if (p->freqratio==(1<<15)) { // 1/2
					S9xMixSamples((uint8*)src,p->sample_count>>1);
	 				for (i=p->sample_count>>2;i;i--) {
			 			*dst++=*src;
		 				*dst++=*src++;
				 	}
				} else if (p->freqratio==(1<<14)) { // 1/4
			 		S9xMixSamples((uint8*)src,p->sample_count>>2);
				 	for (i=p->sample_count>>3;i;i--) {
			 			*dst++=*src;
			 			*dst++=*src;
			 			*dst++=*src;
			 			*dst++=*src++;
				 	}
				} else { //generic case, missing some real interpolation
					int samples_count_corrected;
					samples_count_corrected=((p->sample_count*(p->freqratio)+(p->freqerr))>>16);
					S9xMixSamples((uint8*)src,samples_count_corrected);
					for (i=p->sample_count>>1;i;i--) {
						*dst++=*src;
						p->freqerr+=p->freqratio;
						if ((p->freqerr)&(1<<16)) {
			  			p->freqerr&=(1<<16)-1;
				 			src++;
			 			}
				 	}
				}
			} else { // paused or apu not enabled
				//sprintf(me_debug_str,"a%d,p%d\n",*(p->os9x_apuenabled_ptr),*(p->os9x_paused_ptr));
				register int i,j;
				for (i=0;i<1024*1024;i++) j=j+1;
			}

			p->buffer_idx = next_buffer_idx;
			p->mixsample_flag=0;
		}
		//else{sprintf(me_debug_str,"mixsample_flag:%d\n",p->mixsample_flag);}

		APU_EXECUTE3 ();


	}
	APURegisters.PC = IAPU.PC - IAPU.RAM;
	S9xAPUPackStatus ();	
	memcpy((void*)UNCACHE_PTR(&APUPack),&APUPack,sizeof(struct SAPUPACK));
	memcpy((void*)UNCACHE_PTR(IAPU.RAM),(void*)NORMAL_PTR(IAPU.RAM),0x10000);
	memcpy((void*)UNCACHE_PTR(&SoundData),&SoundData,sizeof(SSoundData));
	//memcpy((void*)UNCACHE_PTR(&save_APUPACK_value),&APUPack,sizeof(struct SAPUPACK));
	//memcpy((void*)UNCACHE_PTR(save_IAPU_RAM),(void*)NORMAL_PTR(IAPU.RAM),0x10000);
	return 1;
}
#endif

void debug_dump(const char* filename);
////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void me_StartSound (){

	RESET_APU_COUNTER();

	//debug_dump("");
	memcpy((void*)UNCACHE_PTR(&APUPack),&APUPack,sizeof(struct SAPUPACK));
	memcpy((void*)UNCACHE_PTR(IAPU.RAM),(void*)NORMAL_PTR(IAPU.RAM),0x10000);
	memcpy((void*)UNCACHE_PTR(&SoundData),&SoundData,sizeof(SSoundData));
	
	me_sound_t *p=(me_sound_t *)((int)(&me_sound_data)|0x40000000);

	//サウンドバグ暫定3
	sceKernelDcacheWritebackInvalidateAll();
	CallME(me_data, (int)(&me_Dummy),0, -1, 0, 0, 0); // all we want is the ME to inv its dcache

	p->buffer[0]=(uint8 *)( ((int)SoundBuffer[0])|0x40000000 );
	p->buffer[1]=(uint8 *)( ((int)SoundBuffer[1])|0x40000000 );
	p->buffer[2]=(uint8 *)( ((int)SoundBuffer[2])|0x40000000 );
	p->buffer[3]=(uint8 *)( ((int)SoundBuffer[3])|0x40000000 );
	p->buffer_idx=2;
	p->sample_count=((stSoundStatus.buffer_size))>>1;
	p->freqratio=snd_freqratio;
	p->os9x_apuenabled_ptr=(int*)( ((int)&os9x_apuenabled)|0x40000000);
	p->os9x_paused_ptr=(int*)( ((int)&os9x_paused)|0x40000000);
	p->exit=0;

	//debug_count++;
	BeginME(me_data, (int)(&me_MixSound), (int)p,-1,0,0, 0);
}
#endif
char* hexFormat(char* buf,void* debug_dump,int size)
{
	int id=0;
	int is=0;
	int col=0;
	char text[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	while ( size>0)
	{
		uint8 t=((uint8*)debug_dump)[is++];
		buf[id++]=text[t>>4];
		buf[id++]=text[t & 0xF];
		size--;
			if(col ==0xFF)
			{
				//buf[id++]='\r';
				buf[id++]='\n';
				col=0;
			}
			else
			{
				buf[id++]=' ';
				col++;
			}		
		
	}
	buf[id++]=0;
	return buf;
}
ClipDataFix debug_ClipDataFix;
//int    debug_MainColorCount=0;
//uint8  debug_MainColorLines[256];
//uint16 debug_MainColorLog[256];
//int    debug_FixColorCount;
//uint8  debug_FixColorLines[256];
//uint16 debug_FixColorLog[256];
//int debug_start;
//int debug_end;
//int debug_Line[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//int debug_Value[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void debug_dump(const char* filename)
{
return;
#if 0
    char str [2000];
    FILE * f;
	static int idx=0;
	if(strlen(filename)==0)
		sprintf(str,"%sAPUDump%d.txt",LaunchDir,debug_count);
	else
		sprintf(str,"%s%s%d.txt",LaunchDir,filename,idx++);

    f = fopen (str, "wb");
#endif

#if 0
	fprintf (f, "*************PPU 0x2100*************\n");
	for(int i=0;i<20;i++)
	{
		fprintf (f, "%d=%X\n",debug_Line[i],debug_Value[i]);
	}
#endif

#if 0
	fprintf (f, "*************FixColor*************\n");
	fprintf (f, "Count=%d\n",debug_FixColorCount);
	//fprintf (f, "Start=%d:End=%d\n",debug_counts[41],debug_counts[42]);

	for(int i=0;i<debug_FixColorCount;i++)
	{
		fprintf(f,"%3d:%3d:%4X\n",i,debug_FixColorLines[i],debug_FixColorLog[i]);
	}
#endif
#if 0
	fprintf (f, "*************Color*************\n");
	fprintf (f, "Count=%d\n",debug_MainColorCount);
	//fprintf (f, "Start=%d:End=%d\n",debug_counts[41],debug_counts[42]);

	for(int i=0;i<debug_MainColorCount;i++)
	{
		fprintf(f,"%3d:%3d:%4X\n",i,debug_MainColorLines[i],debug_MainColorLog[i]);
	}
#endif

#if 0
	extern ClipDataFix debug_ClipDataFix;
	ClipDataFix* pCurrentClipFix=&debug_ClipDataFix;
	fprintf (f, "Start=%d:End=%d\n",debug_start,debug_end);
	for (uint32 bg = 0; bg < 6 ; bg++) {
	for (uint32 g = 0; g < pCurrentClipFix->GroupCount [bg]; g++) {
  		for (uint32 c = 0; c < pCurrentClipFix->Count [bg][g]; c++) {
			fprintf (f, "[%d][%d][%d]=%d,%d,%d,%d\n",c,bg,g,pCurrentClipFix->Left [c][bg][g],pCurrentClipFix->Right [c][bg][g],pCurrentClipFix->Start [bg][g],pCurrentClipFix->End [bg][g]);
		}
	}
	}
#endif

#if 0
	fprintf (f, "*************APU*************\n");
	fprintf (f, "Cycles:%d\n",APUuncached.Cycles);
	fprintf (f, "ShowROM:%d\n",APUuncached.ShowROM);
	fprintf (f, "Flags:%d\n",APUuncached.Flags);
	fprintf (f, "KeyedChannels:%d\n",APUuncached.KeyedChannels);
	fprintf (f, "dummy_for_align:%d\n",APUuncached.dummy_for_align);
	fprintf (f, "OutPorts:%d\n",*(int*)APUuncached.OutPorts);
    fprintf (f, "DSP\n");
	fprintf (f, hexFormat(str,APUuncached.DSP,0x80));
    fprintf (f, "ExtraRAM\n");
	fprintf (f, hexFormat(str,APUuncached.ExtraRAM,60));
	fprintf (f, "Timer:%d,%d,%d\n",APUuncached.Timer[0],APUuncached.Timer[1],APUuncached.Timer[2]);
	fprintf (f, "TimerTarget:%d,%d,%d\n",APUuncached.TimerTarget[0],APUuncached.TimerTarget[1],APUuncached.TimerTarget[2]);
	fprintf (f, "TimerEnabled:%d,%d,%d\n",APUuncached.TimerEnabled[0],APUuncached.TimerEnabled[1],APUuncached.TimerEnabled[2]);
	fprintf (f, "TimerValueWritten:%d,%d,%d\n\n",APUuncached.TimerValueWritten[0],APUuncached.TimerValueWritten[1],APUuncached.TimerValueWritten[2]);

	fprintf (f, "*************SIAPU*************\n");
	fprintf (f, "PC:%d\n",IAPU.PC);
	fprintf (f, "DirectPage:%d\n",IAPU.DirectPage);
	fprintf (f, "APUExecuting:%d\n",IAPU.APUExecuting);
	fprintf (f, "Address:%d\n",IAPU.Address);
	fprintf (f, "WaitAddress1:%d\n",IAPU.WaitAddress1);
	fprintf (f, "WaitCounter:%d\n",IAPU.WaitCounter);
	fprintf (f, "WaitAddress2:%d\n",IAPU.WaitAddress2);
	fprintf (f, "Bit:%d\n",IAPU.Bit);
	fprintf (f, "_Carry:%d\n",IAPU._Carry);
	fprintf (f, "_Zero:%d\n",IAPU._Zero);
	fprintf (f, "_Overflow:%d\n",IAPU._Overflow);
	fprintf (f, "TimerErrorCounter:%d\n",IAPU.TimerErrorCounter);
	fprintf (f, "NextAPUTimerPos:%d\n",IAPU.NextAPUTimerPos);
	fprintf (f, "APUTimerCounter:%d\n",IAPU.APUTimerCounter);
	fprintf (f, "Scanline:%d\n",IAPU.Scanline);
	fprintf (f, "OneCycle:%d\n",IAPU.OneCycle);
	fprintf (f, "TwoCycles:%d\n\n",IAPU.TwoCycles);


	fprintf (f, "*************SAPURegisters*************\n");
	fprintf (f, "P:%d\n",APURegisters.P);
	fprintf (f, "Bit:%d\n",APURegisters.X);
	fprintf (f, "Address:%d\n",APURegisters.S);
	fprintf (f, "_Carry:%d\n",APURegisters.YA);
	fprintf (f, "_Zero:%d\n",APURegisters.PC);


	fprintf (f, "*************UNCashe*******************\n");
	fprintf (f, "apu_cycles_left:%d\n",apu_cycles_left);
	fprintf (f, "apu_glob_cycles:%d\n",*apu_glob_cycles_p);
	fprintf (f, "apu_glob_cycles_Main:%d\n",apu_glob_cycles_Main);
	fprintf (f, "apu_event1_cpt1:%d\n",apu_event1_cpt1);
	fprintf (f, "apu_event2_cpt1:%d\n",apu_event2_cpt1);
	fprintf (f, "apu_event1_cpt2:%d\n",apu_event1_cpt2);
	fprintf (f, "apu_event2_cpt2:%d\n",apu_event2_cpt2);
	fprintf (f, "apu_init_after_load:%d\n",apu_init_after_load);
	fprintf (f, "apu_can_execute:%d\n",apu_can_execute);
	fprintf (f, "apu_ram_write_cpt1:%d\n",apu_ram_write_cpt1);
	fprintf (f, "apu_ram_write_cpt2:%d\n",apu_ram_write_cpt2);
	fprintf (f, "Uncache_APU_Cycles:%d\n",Uncache_APU_Cycles);
	fprintf (f, "Uncache_APU_OutPorts:%d\n",*(int*)Uncache_APU_OutPorts);
	fprintf (f, "IAPU_APUExecuting:%d\n",IAPU_APUExecuting);

	
	fprintf (f, "*************SoundData*******************\n");
	fprintf (f, "master_volume_left:%d\n",SoundData.master_volume_left);
	fprintf (f, "master_volume_right:%d\n",SoundData.master_volume_right);
	fprintf (f, "echo_volume_left:%d\n",SoundData.echo_volume_left);
	fprintf (f, "echo_volume_right:%d\n",SoundData.echo_volume_right);
	fprintf (f, "echo_enable:%d\n",SoundData.echo_enable);
	fprintf (f, "echo_feedback:%d\n",SoundData.echo_feedback);
	fprintf (f, "echo_ptr:%d\n",SoundData.echo_ptr);
	fprintf (f, "echo_buffer_size:%d\n",SoundData.echo_buffer_size);
	fprintf (f, "echo_write_enabled:%d\n",SoundData.echo_write_enabled);
	fprintf (f, "echo_channel_enable:%d\n",SoundData.echo_channel_enable);
	fprintf (f, "pitch_mod:%d\n",SoundData.pitch_mod);
	fprintf (f, "dummy:%d,%d,%d\n",SoundData.dummy[0],SoundData.dummy[1],SoundData.dummy[2]);
	fprintf (f, "master_volume:%d,%d\n",SoundData.master_volume[0],SoundData.master_volume[1]);
	fprintf (f, "echo_volume:%d,%d\n",SoundData.echo_volume[0],SoundData.echo_volume[1]);
	fprintf (f, "noise_hertz:%d\n",SoundData.noise_hertz);
	fprintf (f, "no_filter:%d\n\n",SoundData.no_filter);
//NUM_CHANNELS
#define o(b)\
	fprintf (f, "state:%d\n",SoundData.channels[b].state);\
	fprintf (f, "type:%d\n",SoundData.channels[b].type);\
	fprintf (f, "volume_left:%d\n",SoundData.channels[b].volume_left);\
	fprintf (f, "volume_right:%d\n",SoundData.channels[b].volume_right);\
	fprintf (f, "hertz:%d\n",SoundData.channels[b].hertz);\
	fprintf (f, "frequency:%d\n",SoundData.channels[b].frequency);\
	fprintf (f, "count:%d\n",SoundData.channels[b].count);\
	fprintf (f, "envx:%d\n",SoundData.channels[b].envx);\
	fprintf (f, "left_vol_level:%d\n",SoundData.channels[b].left_vol_level);\
	fprintf (f, "right_vol_level:%d\n",SoundData.channels[b].right_vol_level);\
	fprintf (f, "envx_target:%d\n",SoundData.channels[b].envx_target);\
	fprintf (f, "sample:%d\n",SoundData.channels[b].sample);\
    fprintf (f, "decoded\n");\
	fprintf (f, hexFormat(str,SoundData.channels[b].decoded,32));\
	fprintf (f, "\nprevious16:%d,%d\n",SoundData.channels[b].previous16[0],SoundData.channels[b].previous16[1]);\
	fprintf (f, "block:%d\n",SoundData.channels[b].block);\
	fprintf (f, "env_error:%d\n",SoundData.channels[b].env_error);\
	fprintf (f, "erate:%d\n",SoundData.channels[b].erate);\
	fprintf (f, "direction:%d\n",SoundData.channels[b].direction);\
	fprintf (f, "attack_rate:%d\n",SoundData.channels[b].attack_rate);\
	fprintf (f, "decay_rate:%d\n",SoundData.channels[b].decay_rate);\
	fprintf (f, "sustain_rate:%d\n",SoundData.channels[b].sustain_rate);\
    fprintf (f, "release_rate:%d\n",SoundData.channels[b].release_rate);\
    fprintf (f, "sustain_level:%d\n",SoundData.channels[b].sustain_level);\
    fprintf (f, "sample_number:%d\n",SoundData.channels[b].sample_number);\
    fprintf (f, "last_block:%d\n",SoundData.channels[b].last_block);\
    fprintf (f, "needs_decode:%d\n",SoundData.channels[b].needs_decode);\
    fprintf (f, "block_pointer:%d\n",SoundData.channels[b].block_pointer);\
    fprintf (f, "sample_pointer:%d\n",SoundData.channels[b].sample_pointer);\
    fprintf (f, "echo_buf_ptr:%d\n",SoundData.channels[b].echo_buf_ptr);\
    fprintf (f, "mode:%d\n",SoundData.channels[b].mode);\
    fprintf (f, "envxx:%d\n",SoundData.channels[b].envxx);\
    fprintf (f, "next_sample:%d\n",SoundData.channels[b].next_sample);\
    fprintf (f, "interpolate:%d\n",SoundData.channels[b].interpolate);\
    fprintf (f, "previous:%d,%d\n",SoundData.channels[b].previous[0],SoundData.channels[b].previous[1]);\
    fprintf (f, "next_sample:%d\n",SoundData.channels[b].next_sample);\
    fprintf (f, "dummy:%d,%d,%d,%d,%d,%d,%d,%d\n",SoundData.channels[b].dummy[0],SoundData.channels[b].dummy[1],SoundData.channels[b].dummy[2],SoundData.channels[b].dummy[3],SoundData.channels[b].dummy[4],SoundData.channels[b].dummy[5],SoundData.channels[b].dummy[6],SoundData.channels[b].dummy[7]);\
    fprintf (f, "loop:%d\n\n",SoundData.channels[b].loop);

	o(0);o(1);o(2);o(3);o(4);o(5);o(6);o(7);
#undef o 
#endif
#if 0
	fclose(f);
#endif
}
////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int S9xProcessSound (SceSize ,void *) {
	int i;
#ifndef ME_SOUND
	uint8 *apu_ram_save;
#endif
	u32 sample_count=(stSoundStatus.buffer_size)>>1;
//debug_log( "Thread start!" );
	memset((char*)SoundBuffer[0],0,sample_count*2);
	memset((char*)SoundBuffer[1],0,sample_count*2);

#ifndef ME_SOUND
	apu_ram_save=IAPU.RAM;
	memcpy(&IAPU,&IAPUuncached,sizeof(struct SIAPU));
	IAPU.RAM=apu_ram_save;
	memcpy(IAPU.RAM,IAPUuncached.RAM,0x10000);

	memcpy(&APUPack.APU,&APUuncached,sizeof(struct SAPU));
#endif
	//do it each time since it's updatable from menu
	for (i = 0; i < 256; i++) S9xAPUCycles [i] = (int)S9xAPUCycleLengths [i] * (int)(IAPU.OneCycle) *os9x_apu_ratio / 256;
#ifndef ME_SOUND
	memcpy(&APURegisters,&APURegistersUncached,sizeof(struct SAPURegisters));

	IAPU.DirectPage+=(IAPU.RAM)-(IAPUuncached.RAM);
	IAPU.PC+=(IAPU.RAM)-(IAPUuncached.RAM);
#endif
#ifdef ME_SOUND
	//sceKernelDcacheWritebackInvalidateAll();  
	me_StartSound();
	me_sound_t *p=(me_sound_t *)((int)(&me_sound_data)|0x40000000);
	//sceKernelDelayThread(1000);
	do {
		if (!Settings.ThreadSound) break;
		//play buffer
		i=p->buffer_idx;
		p->mixsample_flag=1;

		if ((os9x_apuenabled==2)&&(!os9x_paused)) {
			sceAudioOutputPannedBlocking( (stSoundStatus.sound_fd), MAXVOLUME, MAXVOLUME, (char*)((int)SoundBuffer[i]|0x00000000));
		} else {
			// [Shoey] Uncommented next 2 lines
			//memset((char*)SoundBuffer[current_SoundBuffer],0,sample_count*2);
			//sceAudioOutputPannedBlocking( (so->sound_fd), MAXVOLUME, MAXVOLUME, (char*)((int)SoundBuffer[i]|0x00000000));
			sceKernelDelayThread(200*1000);//200ms wait
		}
	} while (Settings.ThreadSound);
	p->exit=1;
	WaitME(me_data);
	memcpy(&APUPack,(void*)UNCACHE_PTR(&APUPack),sizeof(struct SAPUPACK));
	memcpy(&SoundData,(void*)UNCACHE_PTR(&SoundData),sizeof(SSoundData));
	memcpy((void*)NORMAL_PTR(IAPU.RAM),(void*)UNCACHE_PTR(IAPU.RAM),0x10000);

#else
	do {
		int i;
		u32 *src,*dst;

		//check if reset or load snapshot occured
		if (apu_init_after_load) {
			if ((apu_init_after_load)&1) {
				S9xResetSound (TRUE);

				apu_ram_save=IAPU.RAM;
				memcpy(&IAPU,&IAPUuncached,sizeof(struct SIAPU));
				IAPU.RAM=apu_ram_save;
				memcpy(IAPU.RAM,IAPUuncached.RAM,0x10000);

//				memcpy(&APUPack.APU,&APUuncached,sizeof(struct SAPU));
				//do it each time since it's updatable from menu
				for (i = 0; i < 256; i++) S9xAPUCycles [i] = (int)S9xAPUCycleLengths [i] * (int)(IAPU.OneCycle) *os9x_apu_ratio / 256;

				memcpy(&APURegisters,&APURegistersUncached,sizeof(struct SAPURegisters));
				IAPU.DirectPage+=(IAPU.RAM)-(IAPUuncached.RAM);
  			IAPU.PC+=(IAPU.RAM)-(IAPUuncached.RAM);

    		S9xSetEchoEnable (0);
    	}
			if ((apu_init_after_load)&2) {
				(IAPU.PC) = (IAPU.RAM) + (APURegisters.PC);

				S9xAPUUnpackStatus ();
				if (APUCheckDirectPage ()) (IAPU.DirectPage) = (IAPU.RAM) + 0x100;
				else(IAPU.DirectPage) = (IAPU.RAM);
				S9xFixSoundAfterSnapshotLoad ();
				if (os9x_apuenabled==2) S9xSetSoundMute( false );
			}
			apu_init_after_load=0;
		}

		if ((os9x_apuenabled==2)&&(!os9x_paused)&&((apu_init_after_load)==0) ) {
			if (snd_freqratio==(1<<16)) { // 1/1
	  			S9xMixSamples((uint8*)SoundBuffer[current_SoundBuffer],sample_count);//stereo & 16bits
	  		} else if (snd_freqratio==(1<<15)) { // 1/2
	  			S9xMixSamples((uint8*)SoundBuffer[2],sample_count>>1);//stereo & 16bits
				src=(u32*)SoundBuffer[2];
	  			dst=(u32*)SoundBuffer[current_SoundBuffer];
	  			for (i=sample_count>>2;i;i--) {
	  				*dst++=*src;
	  				*dst++=*src++;
	  			}
	  		} else { //generic case, missing some real interpolation
	  			S9xMixSamples((uint8*)SoundBuffer[2],(sample_count*snd_freqratio)>>16);//stereo & 16bits
				src=(u32*)SoundBuffer[2];
	  			dst=(u32*)SoundBuffer[current_SoundBuffer];
	  			for (i=sample_count>>1;i;i--) {
	  				*dst++=*src;
	  				snd_freqerr+=snd_freqratio;
	  				if (snd_freqerr&(1<<16)) {
		  				snd_freqerr&=(1<<16)-1;
	  					src++;
	  				}
	  			}
			}
	  		i=current_SoundBuffer;
  			sceAudioOutputPannedBlocking( (stSoundStatus.sound_fd), MAXVOLUME, MAXVOLUME, (char*)SoundBuffer[i]);
  			current_SoundBuffer^=1;
		} else {
  			//memset((char*)SoundBuffer[current_SoundBuffer],0,sample_count*2);
  			//sceAudioOutputPannedBlocking( (so->sound_fd), MAXVOLUME, MAXVOLUME, (char*)SoundBuffer[i]);
  			sceKernelDelayThread(200*1000);//200ms wait
  		}

  	//i=current_SoundBuffer;
  	//sceAudioOutputPannedBlocking( (so->sound_fd), MAXVOLUME, MAXVOLUME, (char*)SoundBuffer[i]);
  	//current_SoundBuffer^=1;

  } while (Settings.ThreadSound);



#endif

#ifndef ME_SOUND
	(APURegisters.PC) = (IAPU.PC) - (IAPU.RAM);
	S9xAPUPackStatus ();
	memcpy(&APURegistersUncached,&APURegisters,sizeof(struct SAPURegisters));

	apu_ram_save=IAPUuncached.RAM;
	memcpy(&IAPUuncached,&IAPU,sizeof(struct SIAPU));
	IAPUuncached.RAM=apu_ram_save;
	memcpy(IAPUuncached.RAM,IAPU.RAM,0x10000);

	IAPUuncached.DirectPage+=(IAPUuncached.RAM)-(IAPU.RAM);
	IAPUuncached.PC+=(IAPUuncached.RAM)-(IAPU.RAM);


	//*((int*)(APUPack.APU.OutPorts))=*((int*)(APUPack.APU.OutPorts));//to avoid losing OutPorts value
	memcpy(&APUuncached,&APUPack.APU,sizeof(struct SAPU));
	//	debug_log( "thread end");
#endif
  return (0);
}
// main->call
void S9xSuspendSoundProcess(void)
{
#ifdef ME_SOUND
	me_resumeing=true;
	/*
	volatile PROCESS_EVENT *pEvent = (PROCESS_EVENT *)UNCACHE_PTR(&g_stProcessEvent);
	pEvent->abQueueData[pEvent->bMainQueuePtr] = EVENT_SUSPEND;
	pEvent->bMainQueuePtr = (pEvent->bMainQueuePtr + 1) &QUEUE_MASK;
	if (pEvent->bMeWorking) {
		// waits until me starts suspending.
		while (!pEvent->wSuspend) {
			{
				pgPrintBG(0,7,0xFFFF,"maybe deadlock");
				pgScreenFlipV();
			}
			;
		}
	}
*/
#endif
}

// main->call
void S9xResumeSoundProcess(void)
{
#ifdef ME_SOUND
	sceKernelDcacheWritebackInvalidateAll();
	me_resumeing=false;
	/*PROCESS_EVENT *pEvent = (PROCESS_EVENT *)UNCACHE_PTR(&g_stProcessEvent);
	pEvent->abQueueData[pEvent->bMainQueuePtr] = EVENT_RESUME;
	pEvent->bMainQueuePtr = (pEvent->bMainQueuePtr + 1) &QUEUE_MASK;
	*/
#endif
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
extern "C" void InitSoundThread(){
//	if (os9x_apuenabled<2) return;
	if (g_sndthread!=-1) return;
	//me_startproc((u32)me_function, (u32)me_data); // [jonny]

	g_sndthread = sceKernelCreateThread( "sound thread", (SceKernelThreadEntry)S9xProcessSound, 0x8, 256*1024, 0, 0 );
	if ( g_sndthread < 0 ){
		ErrorExit( "Thread failed" );
		return;
	}
	Settings.SoundPlaybackRate = os9x_sndfreq;
	snd_freqratio = (u32)(Settings.SoundPlaybackRate)*(1<<16) / 44100;
	snd_freqerr=0;
	samples_error=0;
	current_SoundBuffer=0;
	S9xSetPlaybackRate( Settings.SoundPlaybackRate  );
	//sceKernelDcacheWritebackInvalidateAll();

	Settings.ThreadSound = true;
	sceKernelStartThread( g_sndthread, 0, 0 );

	sceKernelDelayThread(100*1000);
	while (apu_init_after_load) {
		sceKernelDelayThread(100*1000);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
extern "C" void StopSoundThread(){
	if ( g_sndthread !=-1 ){
		Settings.ThreadSound = false;
		sceKernelWaitThreadEnd( g_sndthread, NULL );
		sceKernelDeleteThread( g_sndthread );
		//me_stopproc();
		g_sndthread=-1;
	}
}


////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xSetPalette(){
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xMessage( int type, int number, const char* message ){
	msgBoxLines(message,30);
	//S9xSetInfoString( message );
	if ((type==-1)||(type==S9X_ERROR)) pgwaitPress();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
bool8 S9xReadSuperScopePosition (int &x, int &y, uint32 &buttons)
{
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
bool8 S9xReadMousePosition (int which, int &x, int &y, uint32 &buttons)
{
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
uint32 S9xReadJoypad( int which1 ) {
	int val;
//#ifdef USE_ADHOC
	if (os9x_netplay) {
		if (which1<5) val=os9x_netsnespad[0][which1];
		else val=0;
	} else 
//#endif
	{
		if ( which1 == os9x_padindex) val=os9x_snespad;
		else val=0;
	}
	return val | 0x80000000;
}



////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xSyncSpeed()
{
	static int waited=1;
	struct timeval now;

	s_TotalFrame++;

	os9x_updatepadcpt++;

	if (os9x_netplay) {
		if (os9x_updatepadcpt>=NET_UPDATE_INTERVAL) {
			os9x_updatepadcpt=0;
			os9x_updatepadFrame++;
			update_pad(); //1 frame / 4
		}
	} else {
		if (os9x_updatepadcpt>=OFFLINE_UPDATE_INTERVAL) {
			update_pad(); //1 frame / 2
			os9x_updatepadcpt=0;
		}
	}

	s_iFrame++;
	s_iFrameAuto++;

	S9xProcessEvents( FALSE );

	sceKernelLibcGettimeofday( &now, 0 );
	if ( next1.tv_sec == 0 ){
		next1 = now;
		++next1.tv_usec;
	}
	if ( next1_autofs.tv_sec == 0 ){
		next1_autofs = now;
		++next1_autofs.tv_usec;
	}

	if ( os9x_TurboMode ){
		if( ++IPPU.FrameSkip >= Settings.TurboSkipFrames ){
			IPPU.FrameSkip = 0;
			IPPU.SkippedFrames = 0;
			IPPU.RenderThisFrame = TRUE;
		} else {
			++IPPU.SkippedFrames;
			IPPU.RenderThisFrame = FALSE;
		}
		return;
	}

	if (os9x_speedlimit){
		waited=0;

		if (IPPU.RenderThisFrame) {//if we have draw a frame, sync speed
		  if ( timercmp( &next1, &now, < ) ){
		  	//too slow
				/*unsigned int lag;
				lag = (now.tv_sec - next1.tv_sec) * 1000000 + now.tv_usec - next1.tv_usec;
				if ( lag >= 1000000*1 ){ //1s lag => reset
					next1 = now;
				} else if (lag <= MIN_AUTOFRAME_LAG) waited=1;*/
				//reset time
				next1 = now;
		  } else {
		  	//too fast,
		  	//wait to sync
		  	waited = (now.tv_sec - next1.tv_sec) * 1000000 + now.tv_usec - next1.tv_usec;
		  	while ( timercmp( &next1, &now, > ) ){
					sceKernelLibcGettimeofday( &now, 0 );
		  	}
		  }
		}

	  // update next timer value
	  if (!os9x_fpslimit) {
	  	next1.tv_usec += ((Settings.PAL?1000000/50:1000000/60));
	  } else {
	  	next1.tv_usec += 1000000/os9x_fpslimit;
	  }
	  while ( next1.tv_usec >= 1000000 ){
	  	next1.tv_sec += 1;
      next1.tv_usec -= 1000000;
	  }
	}

	if (Settings.SkipFrames==AUTO_FRAMERATE) {
		//AUTO FRAME SKIPPING
		if (IPPU.RenderThisFrame) {
			//WE HAVE RENDERED A FRAME, so sync was performed
	  	if (!waited && (os9x_autofskip_SkipFrames<MAX_AUTO_SKIP) ) {
	  		//it was too slow
				os9x_autofskip_SkipFrames++;
			} else {
				//it was too fast
				//perhaps add a limit to wait => if (wait>LIMIT) os9x_autofskip_SkipFrames--...
				if (os9x_autofskip_SkipFrames) os9x_autofskip_SkipFrames--;
			}
		}
		IPPU.RenderThisFrame = ++IPPU.SkippedFrames > os9x_autofskip_SkipFrames;
		if ( IPPU.RenderThisFrame ) IPPU.SkippedFrames = 0;
	} else {
		IPPU.RenderThisFrame = ++IPPU.SkippedFrames > Settings.SkipFrames;
		if ( IPPU.RenderThisFrame ) IPPU.SkippedFrames = 0;
	}


}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
const char *S9xGetFilename( const char *e )
{
	static char filename [_MAX_PATH + 1];
	char drive [_MAX_DRIVE + 1];
	char dir [_MAX_DIR + 1];
	char fname [_MAX_FNAME + 1];
	char ext [_MAX_EXT + 1];

	_splitpath (Memory.ROMFilename, drive, dir, fname, ext);
	_makepath (filename, drive, dir, fname, e);

	return (filename);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
const char *S9xGetSaveFilename( const char *e ) {
	static char filename [_MAX_PATH + 1];
	char drive [_MAX_DRIVE + 1];
	char dir [_MAX_DIR + 1];
	char fname [_MAX_FNAME + 1];
	char ext [_MAX_EXT + 1];

	_splitpath (Memory.ROMFilename, drive, dir, fname, ext);
	sprintf(dir,"%sSAVES",LaunchDir);
	_makepath (filename, drive, dir, fname, e);

	return (filename);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
bool8 S9xInitUpdate() {
	os9x_renderingpass=0;

	if (os9x_softrendering>=2) {//GU mode
		sceGuStart(0,list);



		/* setup view port */
		//sceGuOffset(2048 - (256/2),2048 - (240/2));
		//sceGuViewport(2048,2048,256,240);
		sceGuDrawBufferList(GU_PSM_5551,(void*)(512*272*2*2+256*240*2+2*256*256*2),256);

		/*clear screen */
		sceGuScissor(0,0,256,os9x_snesheight);
		sceGuClearColor(0);
		//sceGuClearDepth(0);
		sceGuClear(GU_COLOR_BUFFER_BIT/*|GU_DEPTH_BUFFER_BIT*/);

		sceGuFinish();
  	sceGuSync(0,0);

	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void resync_var(){

    if (os9x_fskipvalue==AUTO_FSKIP) {
    	Settings.SkipFrames=AUTO_FRAMERATE;
    	os9x_autofskip_SkipFrames=0;
    	os9x_speedlimit=1;
    } else Settings.SkipFrames=os9x_fskipvalue;

    //uncaching stuff
    int tmp=os9x_apuenabled;
    *os9x_apuenabled_ptr=tmp;
		Settings.NextAPUEnabled = Settings.APUEnabled = (os9x_apuenabled==1)||(os9x_apuenabled==2);

		/*if ((!os9x_apuenabled)||(os9x_apuenabled==3))*/ os9x_hack|=APU_FIX;

		os9x_snesheight=(Settings.PAL&&(!os9x_forcepal_ntsc)?240:224);

		set_cpu_clock();

		sceKernelLibcGettimeofday( &s_tvStart, 0 );
		os9x_autosavetimer_tv=s_tvStart;

		//reset timer for synchro stuff
		next1.tv_sec = next1.tv_usec = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void before_pause(){
	//os9x_paused=1;
	os9x_paused=*os9x_paused_ptr=1;
//#ifndef ME_SOUND
	StopSoundThread();
//#endif
#ifndef ME_SOUND
	scePowerSetClockFrequency(222,222,111);
#endif
	//scePowerSetClockFrequency(300,300,150);
	Settings.Paused = TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void after_pause(){
	pgFillAllvram(0);
	Settings.Paused = false;
	//os9x_paused=0;
	os9x_paused=*os9x_paused_ptr=0;
	//reinit blitter
	blit_reinit();
	//invalidate all gfx caches
	ZeroMemory (IPPU.TileCached [TILE_2BIT], MAX_2BIT_TILES<<1);
  ZeroMemory (IPPU.TileCached [TILE_4BIT], MAX_4BIT_TILES<<1);
  ZeroMemory (IPPU.TileCached [TILE_8BIT], MAX_8BIT_TILES<<1);
  tile_askforreset(-1);
  //
	resync_var();
#ifdef ME_SOUND
	if (os9x_apuenabled==2)	S9xSetSoundMute( false );
	else S9xSetSoundMute( true );
#endif
	InitSoundThread();
#ifndef ME_SOUND
	if (os9x_apuenabled==2)	S9xSetSoundMute( false );
	else S9xSetSoundMute( true );
#endif
}
////////////////////////////////////////////////////////////////////////////////////////
// Ge Callback
////////////////////////////////////////////////////////////////////////////////////////
struct timeval	now;
	unsigned long long	diff;
	static int fps_val=0;
	static int real_fps_val=0;
int g_cbid;
#define pg_vramtop ((char *)0x04000000)
// Ge render callback
void GeCallback(int id, void *arg)
{
	char			buf[128];

	swap_buf^=1;
	pg_drawframe=swap_buf^1;

	if (os9x_showpass){
		sprintf(buf,"%03d",os9x_renderingpass);
		pgPrintBG(CMAX_X-8-strlen(buf),0,0xffff,buf);

#ifndef	RELEASE
		int Y=1;
		char str[64];
		#define LOG_PROFILE_FUNC(func,type) \
			sprintf(str,"%25s (...) =%u usecs\n", #func, type.time_##func); \
	    pgPrintBG (0,Y++,0xFFFF,str);

			LOG_PROFILE_FUNC (S9xMainLoop, profile_data)
			RESET_PROFILE_FUNC(S9xMainLoop)
#endif
	}
	os9x_renderingpass=0;


	if (os9x_TurboMode) {
		if ((s_TotalFrame>>6)&1) pgPrintBG(CMAX_X-5,33,0xffff,"TURBO");
		else pgPrintBG(CMAX_X-5,33,0xffff,"     ");
	}

	if (os9x_lowbat) {
		if (!((s_TotalFrame>>7)&15)) pgPrintBG(0,33,0xffff,"Low Battery/Saving disactivated");
		else pgPrintBGRev(0,33,0xffff,"                               ");
	}

	s_iFrameReal++;
	sceKernelLibcGettimeofday( &now, 0 );
	if (os9x_showfps) {
		diff  = (now.tv_sec - s_tvStart.tv_sec) * 1000000 + now.tv_usec - s_tvStart.tv_usec;
		diff /= 1000000;
		if ( diff>=2 ) {
				fps_val = s_iFrame/diff;
				real_fps_val = s_iFrameReal/diff;
				s_tvStart = now;
				s_iFrame  = 0;
				s_iFrameReal=0;
		}

		if (fps_val) {
			buf[0] = ((fps_val / 100)%10) + '0';
			buf[1] = ((fps_val / 10)%10) + '0';
			buf[2] = (fps_val % 10) + '0';
			buf[3] = 'F';
			buf[4] = 'P';
			buf[5] = 'S';
			buf[6] = '\0';
			pgPrintBG( CMAX_X - 7, 0, 0xffff, buf );
		}
		if (real_fps_val) {
			buf[0] = ((real_fps_val / 100)%10) + '0';
			buf[1] = ((real_fps_val / 10)%10) + '0';
			buf[2] = (real_fps_val % 10) + '0';
			pgPrintBG( CMAX_X - 7, 1, 0xffff, buf );
		}
	}
	//MyCounter_drawCount();
/*	{
		char st[108];
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		volatile PROCESS_EVENT *pEvent = (PROCESS_EVENT *)UNCACHE_PTR(&g_stProcessEvent);
//		sprintf(st,"%02X, %02X, %02X, %02X, %02X",
//			pEvent->wCallCount, pEvent->wSuspend, pEvent->bMeWorking, pEvent->bMeQueuePtr, pEvent->bMainQueuePtr);
//		pgPrintBG(0,2,0xFFFF,st);
		SAPUEVENTS *pEvent2 = (SAPUEVENTS *)UNCACHE_PTR(&stAPUEvents);\
		sprintf(st,"E1[%04X,%04X] E2[%04X,%04X], RW[%04X,%04X]",
			pEvent2->apu_event1_cpt1&0xFFFF, pEvent2->apu_event1_cpt2&0xFFFF,
			pEvent2->apu_event2_cpt1&0xFFFF, pEvent2->apu_event2_cpt2&0xFFFF,
			pEvent2->apu_ram_write_cpt1&0xFFFF, pEvent2->apu_ram_write_cpt2&0xFFFF);
		pgPrintBG(0,2,0xFFFF,st);
		sprintf(st,"%08X, %08X, %08X",
			pEvent2->APU_Cycles, pEvent2->apu_glob_cycles, cpu_glob_cycles);
		pgPrintBG(0,3,0xFFFF,st);
		sprintf(st,"[%08X,%08X,%08X,%08X]",
			pEvent2->adwParam[0], pEvent2->adwParam[1], pEvent2->adwParam[2], pEvent2->adwParam[3]);
		pgPrintBG(0,4,0xFFFF,st);
	}*/

	/*{
		char st[32];
		sprintf(st,"%08X",os9x_updatepadFrame);
		pgPrintBG(CMAX_X-10,7,0xFFFF,st);
	}*/

	sceDisplaySetFrameBuf(pg_vramtop+(pg_drawframe?FRAMESIZE:0),LINESIZE,1,
		os9x_vsync ? PSP_DISPLAY_SETBUF_NEXTFRAME: PSP_DISPLAY_SETBUF_IMMEDIATE);
}

void SetGeCallback(void)
{
	PspGeCallbackData cb;

	cb.signal_func = NULL;
	cb.signal_arg = NULL;
	cb.finish_func = GeCallback;
	cb.finish_arg = NULL;
	g_cbid = sceGeSetCallback(&cb);

}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
bool8 S9xDeinitUpdate (int Width, int Height, bool8 sixteen_bit) {


	//if (os9x_hack & HIRES_FIX) {
	//	hires_swap++;
	//	if (hires_swap>=3) hires_swap=0;
	//	switch (hires_swap) {
	//		case 0:hires_offset=0;hires_offset2=0;hires_offsetV=0;hires_offsetV2=0;
	//			break;
	//		case 1:hires_offset=1;hires_offset2=0;hires_offsetV=1;hires_offsetV2=0;
	//			break;
	//		case 2:hires_offset=0;hires_offset2=1;hires_offsetV=0;hires_offsetV2=1;
	//			break;
	//	}
	//}

	if (os9x_softrendering>=2) {
		switch (os9x_render) {
			case 0:
  						//could be faster since no stretch, use copyimage
  						guDrawBuffer((u16*)(0x44000000+512*272*2*2+256*240*2+2*256*256*2),256,os9x_snesheight,256,256,os9x_snesheight);
  						break;
			case 1:
							guDrawBuffer((u16*)(0x44000000+512*272*2*2+256*240*2+2*256*256*2),256,os9x_snesheight,256,256*272/os9x_snesheight,272);
				break;
			case 2:
							guDrawBuffer((u16*)(0x44000000+512*272*2*2+256*240*2+2*256*256*2),256,os9x_snesheight,256,272*4/3,272);
							//pgBitBltFull((unsigned long*)(0x44000000+512*272*2*2+256*240*2+2*256*256*2),os9x_snesheight,0,272);
				break;
			case 3:
							guDrawBuffer((u16*)(0x44000000+512*272*2*2+256*240*2+2*256*256*2),256,os9x_snesheight,256,320*272/os9x_snesheight,272);
				break;
			case 4:
							guDrawBuffer((u16*)(0x44000000+512*272*2*2+256*240*2+2*256*256*2),256,os9x_snesheight,256,480,272);
				break;
			case 5:
							guDrawBuffer((u16*)(0x44000000+512*272*2*2+256*240*2+2*256*256*2),256,os9x_snesheight-16,256,480,272);
				break;
		}
  } else {
		switch (os9x_render) {
			case 0:
				//could be faster since no stretch, use copyimage
				guDrawBuffer((u16*)(GPUPack.GFX.Screen),256,os9x_snesheight,256,256,os9x_snesheight);
				break;
			case 1:	guDrawBuffer((u16*)(GPUPack.GFX.Screen),256,os9x_snesheight,256,256*272/os9x_snesheight,272);
				break;
			case 2:	guDrawBuffer((u16*)(GPUPack.GFX.Screen),256,os9x_snesheight,256,272*4/3,272);
				break;
			case 3:	guDrawBuffer((u16*)(GPUPack.GFX.Screen),256,os9x_snesheight,256,320*272/os9x_snesheight,272);
				break;
			case 4:	guDrawBuffer((u16*)(GPUPack.GFX.Screen),256,os9x_snesheight,256,480,272);
				break;
			case 5:	guDrawBuffer((u16*)(GPUPack.GFX.Screen),256,os9x_snesheight-16,256,480,272);
				break;
		}
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xInitDisplay(  )
{
//	Settings.Transparency = TRUE;
	Settings.SixteenBit   = TRUE;
	Settings.SupportHiRes = 0; //interpolate;
	memset( GFX_SubScreen,  0, SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2 );
	memset( GFX_ZBuffer,    0, SNES_WIDTH * SNES_HEIGHT_EXTENDED  );
	memset( GFX_SubZBuffer, 0, SNES_WIDTH * SNES_HEIGHT_EXTENDED  );
	GPUPack.GFX.Pitch      = 256 * 2;
	//screen & zbuffer share the same memory location since they should not be used
	//at the same time for the same location
  GPUPack.GFX.Screen   = (uint8*)(0x44000000+512*272*2*2);
  GPUPack.GFX.SubScreen  = (uint8*)GFX_SubScreen;
	GPUPack.GFX.ZBuffer    = (uint8*)GFX_ZBuffer;
	GPUPack.GFX.SubZBuffer = (uint8*)GFX_SubZBuffer;
}




////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xProcessEvents( bool8 block ) {
	static int cpt=0;
	if (!(cpt&2047)) scePowerTick(0); //send powertick to avoid auto off
	if (!(cpt&2047)) {//check every 34s
		//check if battery is low
		check_battery();
	}
	cpt++;
	if (os9x_specialaction&OS9X_MENUACCESS) {
		if ((in_emu==1)&&os9x_netplay) {		//net pause
			net_flush_net(2);
		}
		before_pause();
		if (!os9x_lowbat) {
			if (CPUPack.CPU.SRAMModified) {
				Memory.SaveSRAM( (char*)S9xGetSaveFilename(".SRM") );
				CPUPack.CPU.SRAMModified=0;
			}
		}
		//initUSBdrivers();

		root_menu();

#ifndef FW3X
		endUSBdrivers();
#endif
		os9x_specialaction=0;
		if (!os9x_lowbat) {
			if (menu_modified) {
				save_rom_settings(Memory.ROMCRC32,Memory.ROMName);
				save_settings();
			}
		}
		if ((in_emu==1)&&os9x_netplay&&os9x_getnewfile) {
			psp_msg(ADHOC_CLOSING, MSG_DEFAULT);
#ifdef USE_ADHOC
			os9x_netplay=0;adhocTerm();os9x_adhoc_active=0;
#else
			os9x_netplay=0;os9x_adhoc_active=0;
#endif
		}

		if ((in_emu==1)&&os9x_netplay) {		//net unpause
			set_cpu_clock();
			net_send_state();
			net_send_settings();
		}
		after_pause();
	}
	if ((os9x_specialaction&OS9X_GFXENGINE)&& (!(os9x_specialaction_old&OS9X_GFXENGINE))) {
		os9x_softrendering++;
		if (os9x_softrendering==5) os9x_softrendering=0;
		//invalidate all cache
		ZeroMemory (IPPU.TileCached [TILE_2BIT], MAX_2BIT_TILES<<1);
  	ZeroMemory (IPPU.TileCached [TILE_4BIT], MAX_4BIT_TILES<<1);
  	ZeroMemory (IPPU.TileCached [TILE_8BIT], MAX_8BIT_TILES<<1);
  	tile_askforreset(-1);
  	//
		switch (os9x_softrendering) {
			case 0:psp_msg(VIDEO_ENGINE_APPROX, MSG_DEFAULT); break;
			case 1:psp_msg(VIDEO_ENGINE_ACCUR, MSG_DEFAULT); break;
			case 2:psp_msg(VIDEO_ENGINE_ACCEL, MSG_DEFAULT); break;
			case 3:psp_msg(VIDEO_ENGINE_ACCEL_ACCUR, MSG_DEFAULT); break;
			case 4:psp_msg(VIDEO_ENGINE_ACCEL_APPROX, MSG_DEFAULT); break;
		}
		//reset timer for synchro stuff
		next1.tv_sec = next1.tv_usec = 0;
  }
	if ((os9x_specialaction&OS9X_FRAMESKIP_DOWN)&& (!(os9x_specialaction_old&OS9X_FRAMESKIP_DOWN))) {
		char st[64];
		if (os9x_fskipvalue) os9x_fskipvalue--;
		else os9x_fskipvalue=AUTO_FSKIP;

		if (os9x_fskipvalue==AUTO_FSKIP) {
			Settings.SkipFrames=AUTO_FRAMERATE;
			os9x_autofskip_SkipFrames=0;
			os9x_speedlimit=1;
			strcpy(st,psp_msg_string(VIDEO_FSKIP_AUTO));
		} else {
			Settings.SkipFrames=os9x_fskipvalue;
			sprintf(st,psp_msg_string(VIDEO_FSKIP_MANUAL),os9x_fskipvalue);
		}
		msgBoxLines(st,10);
		//reset timer for synchro stuff
		next1.tv_sec = next1.tv_usec = 0;
	}
	if ((os9x_specialaction&OS9X_FRAMESKIP_UP) && (!(os9x_specialaction_old&OS9X_FRAMESKIP_UP))) {
		char st[64];
		if (os9x_fskipvalue<AUTO_FSKIP) os9x_fskipvalue++;
		else os9x_fskipvalue=0;
		if (os9x_fskipvalue==AUTO_FSKIP) {
			Settings.SkipFrames=AUTO_FRAMERATE;
			os9x_speedlimit=1;
			os9x_autofskip_SkipFrames=0;
			strcpy(st,psp_msg_string(VIDEO_FSKIP_AUTO));
		} else {
			Settings.SkipFrames=os9x_fskipvalue;
			sprintf(st,psp_msg_string(VIDEO_FSKIP_MANUAL),os9x_fskipvalue);
		}
		msgBoxLines(st,10);
		//reset timer for synchro stuff
		next1.tv_sec = next1.tv_usec = 0;
	}
	if ((os9x_specialaction&OS9X_TURBO)&& (!(os9x_specialaction_old&OS9X_TURBO))) {
		if (os9x_TurboMode) {
			pgPrintAllBG(CMAX_X-5,33,0xffff,"     ");
			//reset timer for synchro stuff
			next1.tv_sec = next1.tv_usec = 0;
		}
		os9x_TurboMode^=1;
	}
	if ((os9x_specialaction&OS9X_SAVE_STATE)&& (!(os9x_specialaction_old&OS9X_SAVE_STATE))) {
		if ((in_emu==1)&&os9x_netplay) {		//net pause
			net_flush_net(2);
		}
		before_pause();

		switch (os9x_lowbat) {
			case 1:
				if(!psp_msg(MENU_STATE_WARNING_LOWBAT, MSG_DEFAULT)) break;
			default:
				psp_msg(MENU_STATE_ISSAVING,MSG_DEFAULT);
				os9x_save(".zat");
		}

		if ((in_emu==1)&&os9x_netplay) {		//net unpause
			set_cpu_clock();
			net_send_state();
			net_send_settings();
		}
		after_pause();
	}
	if ((os9x_specialaction&OS9X_LOAD_STATE)&& (!(os9x_specialaction_old&OS9X_LOAD_STATE))) {
		SceIoStat stat;

		if ((in_emu==1)&&os9x_netplay) {		//net pause
			net_flush_net(2);
		}
		before_pause();

		if (sceIoGetstat(S9xGetSaveFilename(".zat"),&stat)>=0) {
			if (psp_msg(MENU_STATE_CONFIRMLOAD,MSG_DEFAULT)) {
				psp_msg(MENU_STATE_ISLOADING,MSG_DEFAULT);
				os9x_load(".zat");
			}
		} else psp_msg(MENU_STATE_NOSTATE,MSG_DEFAULT);

		if ((in_emu==1)&&os9x_netplay) {		//net unpause
			set_cpu_clock();
			net_send_state();
			net_send_settings();
		}
		after_pause();
	}
	if (os9x_autosavetimer) {
		struct timeval now;
		int diff;
		sceKernelLibcGettimeofday( &now, 0 );
		diff  = (now.tv_sec - os9x_autosavetimer_tv.tv_sec) * 1000000 + now.tv_usec - os9x_autosavetimer_tv.tv_usec;
		diff/=1000000;
		if ( diff>=60*os9x_autosavetimer ) {
			os9x_autosavetimer_tv=now;
			if (!os9x_lowbat) {
				psp_msg(LOADSAVE_AUTOSAVETIMER, MSG_DEFAULT);
				os9x_save(".zat");
				//reset timer for synchro stuff
				next1.tv_sec = next1.tv_usec = 0;
			}
		}
	}
	os9x_specialaction_old=os9x_specialaction;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xExit (){
	g_bLoop=0;
#if 0
	// Cleanup the games resources etc (if required)
	//debug_log("Exit Callback");

	//Settings.Paused = TRUE;

	//*os9x_paused_ptr=1;
	//StopSoundThread();
	//scePowerSetClockFrequency(222,222,111);
	sceKernelDelayThread(1000000); //a try to fix hang on 2.5
	if (g_sndthread>=0) sceKernelTerminateThread(g_sndthread);
	if (g_updatethread>=0) sceKernelTerminateThread(g_updatethread);
	//if (g_mainthread>=0) sceKernelTerminateThread(g_mainthread);

	if (!os9x_lowbat) {
		save_settings();
		if (in_emu==1) {
			Memory.SaveSRAM( (char*)S9xGetSaveFilename(".SRM") );
			//S9xSaveCheatFile( (char*)S9xGetSaveFilename( ".cht" ) );
			save_rom_settings(Memory.ROMCRC32,Memory.ROMName);
		}
		//pgWaitVn(60*1);//give some times to save files
	}

	// S9xCloseSoundDevice();
	//g_bLoop = false;
	// Exit game

	sceKernelExitGame();
#endif
	return;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
const char *S9xBasename (const char *f)
{
  const char *p;
  if ((p = strrchr (f, '/')) != NULL || (p = strrchr (f, '\\')) != NULL)
    return (p + 1);
  return (f);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
bool8 S9xOpenSnapshotFile (const char *fname, bool8 read_only, STREAM *file) {
  if (read_only) {
      if (!(*file = OPEN_STREAM(fname,"rb"))) return(false);
	} else {
    if (!(*file = OPEN_STREAM(fname,"wb"))) return(false);
	}
	char *ext;
	ext=strrchr(fname,'.');
	if (ext&&(strlen(ext)==4)) {
		if ((ext[1]=='z')&&(ext[2]=='a')) {
			if (os9x_externstate_mode) {
				psp_msg(LOADSAVE_EXPORTS9XSTATE, MSG_DEFAULT);
				os9x_externstate_mode=0;
			}
		}
  }


  if (os9x_externstate_mode) return true;

  if (read_only) {//reading savestate
		READ_STREAM(os9x_savestate_mini,128*120*2,*file);
	} else { //writing savestate
		int x,y;
		u16 *snes_image;
		if (os9x_softrendering<2)	snes_image=(u16*)(0x44000000+512*272*2*2);
		else snes_image=(u16*)(0x44000000+2*512*272*2+256*240*2+2*256*256*2);
		for (y=0;y<os9x_snesheight/2;y++)
			for (x=0;x<128;x++) {
				int col2a=snes_image[(y*2)*256+(x*2)];
				int col2b=snes_image[(y*2+1)*256+(x*2)];
				int col2c=snes_image[(y*2)*256+(x*2+1)];
				int col2d=snes_image[(y*2+1)*256+(x*2+1)];
				int col2;
				col2=((((((col2a>>10)&31)+((col2b>>10)&31)+((col2c>>10)&31)+((col2d>>10)&31))>>2)/**2/3*/)<<10);
				col2|=((((((col2a>>5)&31)+((col2b>>5)&31)+((col2c>>5)&31)+((col2d>>5)&31))>>2)/**2/3*/)<<5);
				col2|=((((((col2a>>0)&31)+((col2b>>0)&31)+((col2c>>0)&31)+((col2d>>0)&31))>>2)/**2/3*/)<<0);
				os9x_savestate_mini[y*128+x]=col2;
			}
		WRITE_STREAM(os9x_savestate_mini,128*120*2,*file);
	}
  return (true);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xCloseSnapshotFile (STREAM file) {
  CLOSE_STREAM (file);
}

extern "C" {

#include "psp_state.c"

#include "psp_utils.c"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void initvar_withdefault() {
	bg_img_num=-1; //bg will be randomized

	os9x_menumusic=0; //no music
	os9x_menufx=0; //menu background FX disabled
	os9x_menupadbeep=1; //beep when selecting something
	os9x_autostart = 0; //do not auto-boot
	os9x_osk = 1; //danzeff
	os9x_usballowed=0; //no usb

	os9x_apu_ratio=256; //100%
	os9x_fpslimit=0; //AUTO
	os9x_padindex=0;

	g_bSleep=0;
	//////
	os9x_padvalue=0;
	os9x_padvalue_ax=0x80;
	os9x_padvalue_ay=0x80;
	//
	os9x_screenLeft=os9x_screenTop=os9x_screenWidth=os9x_screenHeight=0;

	os9x_autosavetimer=0;
	os9x_autosavesram=0;
	os9x_lowbat=0;
	check_battery();
	//if (scePowerIsBatteryExist()) os9x_lowbat=scePowerIsLowBattery();
	os9x_applyhacks=1;
	os9x_BG0=1;
	os9x_BG1=1;
	os9x_BG2=1;
	os9x_BG3=1;
	os9x_OBJ=1;
	os9x_easy=0;
	os9x_render=2;  //zoom 4/3 (tv mode)
	os9x_showfps=0;
	os9x_showpass=0;
	os9x_vsync=0;
	os9x_cpuclock=300;
	os9x_SA1_exec=1;

	os9x_apuenabled=2;

	os9x_gammavalue=0;
	os9x_fastsprite=0;
	os9x_softrendering=4;//psp accel+approx soft
	os9x_smoothing=1;
	os9x_fskipvalue=0;
	os9x_autofskip_SkipFrames=0;
	os9x_speedlimit=1;

	os9x_forcepal_ntsc=1; //most pal games have black bottom borders
	os9x_sndfreq = 44100;
	/** not in menu at the moment **/
	os9x_ShowSub=0;
	os9x_CyclesPercentage=100;
	os9x_DisableHDMA=0;
#ifdef DEBUGGER
	os9x_DisableIRQ=0;
#endif

	// special hack
	os9x_hack=0;

	//default inputs
	os9x_inputs_analog=0;
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
	/*os9x_inputs[PSP_AUP]=OS9X_FRAMESKIP_UP;
	os9x_inputs[PSP_ADOWN]=OS9X_FRAMESKIP_DOWN;
	os9x_inputs[PSP_ALEFT]=OS9X_MENUACCESS;
	os9x_inputs[PSP_ARIGHT]=OS9X_TURBO;*/
#ifndef HOME_HOOK
  os9x_inputs[PSP_TL_TR]=OS9X_MENUACCESS; // [Shoey]
#endif
	os9x_inputs[PSP_TL_SELECT]=OS9X_TURBO;
	os9x_inputs[PSP_TL_START]=OS9X_GFXENGINE;
	os9x_inputs[PSP_TR_SELECT]=OS9X_FRAMESKIP_UP;
	os9x_inputs[PSP_TR_START]=OS9X_FRAMESKIP_DOWN;
	//os9x_inputs[PSP_NOTE]=OS9X_GFXENGINE;

	os9x_specialaction=os9x_specialaction_old=0;

	//
}







#ifndef NOKERNEL
void MyExceptionHandler(PspDebugRegBlock *regs)
{
	// Do normal initial dump, setup screen etc
	pspDebugScreenInit();

	// I always felt BSODs were more interesting that white on black
	pspDebugScreenSetBackColor(0x00FF0000);
	pspDebugScreenSetTextColor(0xFFFFFFFF);
	pspDebugScreenClear();

	pspDebugScreenPrintf("I regret to inform you your psp has just crashed\n");
	pspDebugScreenPrintf("\nShit happens... :-(\n");
	pspDebugScreenPrintf("Exception Details:\n");
	pspDebugDumpException(regs);
	pspDebugScreenPrintf("\nWill exit to PSP menu in 10 seconds\n");

	pgWaitVn(60*10);

	sceKernelExitGame();
}
#endif



void intro_anim();

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int user_main(SceSize args, void* argp);


////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int main(int argc,char **argv) {
	// Kernel mode thread
	/* Install our custom exception handler. If this was NULL then the default would be used */
#ifndef NOKERNEL
	pspDebugInstallErrorHandler(MyExceptionHandler);
#endif
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(0);
	/************/
  //pspDebugScreenInit();
  pgFillAllvram(0);
	sceDisplaySetMode( 0, SCREEN_WIDTH, SCREEN_HEIGHT );
	sceDisplaySetFrameBuf( (char*)VRAM_ADDR, 512, 1, 1 );

	pgScreenFrame(2,0);
	pgFillAllvram(0);

	strncpy(LaunchDir,argv[0],sizeof(LaunchDir)-1);
	LaunchDir[sizeof(LaunchDir)-1]=0;
	char *str_ptr=strrchr(LaunchDir,'/');
	if (str_ptr){
		str_ptr++;
		*str_ptr=0;
	}
  /* Clear the existing profile regs */
	//pspDebugProfilerClear();
	/* Enable profiling */
	//pspDebugProfilerEnable();
#if defined(ME_SOUND) || defined(HOME_HOOK)
	char str[256];
	int devkit_version = sceKernelDevkitVersion();
	SceUID mod;
#endif
#ifdef ME_SOUND // [Shoey/Chilly]
	// have to do this before ME enabled or BOOOOOOOMMMMMM!!!!!!
	scePowerSetClockFrequency(333,333,166);


    sprintf(str,"%s%s",LaunchDir,"mediaengine.prx");
    if( (mod = pspSdkLoadStartModule(str, PSP_MEMORY_PARTITION_KERNEL)) < 0 )
    {
		ErrorExit(" Error  loading/mediaengine");
//        sceKernelDelayThread(3*1000*1000);
 		return 0;
    }

    me_data = (volatile struct me_struct*)malloc_64( sizeof( struct me_struct ) );  // [Shoey]
	if(!me_data)
	{
		ErrorExit(" malloc fail ME\n" );
		return 0;
	}
    me_data = (volatile struct me_struct*)(((int) me_data) | 0x40000000 );          // [Shoey]
    if( InitME( me_data, devkit_version ) )
    {
		ErrorExit(" Error Initializing ME\n" );
		return 0;
    }
#endif
#ifdef HOME_HOOK
//#ifndef ME_SOUND
//    // Don't call this if mediaengine.prx is already loaded
//    scePowerSetClockFrequency(333,333,166);
//#endif
    // Might want to set frequency here as well
    sprintf(str,"%s%s",LaunchDir,"homehook.prx");
	  if ( (mod = pspSdkLoadStartModule(str, PSP_MEMORY_PARTITION_KERNEL)) < 0)
	  {
		ErrorExit(" Error loading/homehook");
//        sceKernelDelayThread(3*1000*1000);
 		return 0;
	  }
    initHomeButton(devkit_version);
#endif
//    sprintf(str,"%s%s",LaunchDir,"clock.prx");
//	  if ( (mod = pspSdkLoadStartModule(str, PSP_MEMORY_PARTITION_KERNEL)) < 0)
//	  {
//		ErrorExit(" Error loading/clock");
////        sceKernelDelayThread(3*1000*1000);
// 		return 0;
//	  }
#ifndef FW3X
	//usb stuff
	//----------------
	loadUSBdrivers();
	//----------------
#endif

#ifdef USE_ADHOC
	//network stuff
	if (adhocLoadDrivers(&module_info) != 0) {
		//pspDebugScreenInit();
		//printf("Net driver load error\n");
		//pgWaitVn(60*2);
    //return 0;
    psp_msg(ADHOC_DRIVERLOADERR, MSG_DEFAULT);
  }
#endif
#ifdef ME_SOUND
//  me_data = me_struct_init();               // [jonny]
//  me_startproc((u32)me_function, (u32)me_data); // [jonny]
#endif
	// create user thread, tweek stack size here if necessary
#ifdef ME_SOUND
//me_stopproc();
#endif			
	//user_main(0,NULL);
  //user thread for network
	SceUID g_mainthread = sceKernelCreateThread("User Mode Thread", user_main,
            0x9,
            2*256 * 1024, // stack size (256KB is regular default)
            PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, NULL);

  // start user thread, then wait for it to do everything else
  sceKernelStartThread(g_mainthread, 0, NULL);
  sceKernelWaitThreadEnd(g_mainthread, NULL);

  sceKernelExitGame();
  return 0;
}



////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void low_level_init(){
	// init psp stuff
	pgFillAllvram(0);
	sceDisplaySetMode( 0, SCREEN_WIDTH, SCREEN_HEIGHT );
	sceDisplaySetFrameBuf( (char*)VRAM_ADDR, 512, 1, 1 );
	pgScreenFrame(2,0);
	pgFillAllvram(0);
	sceCtrlSetSamplingCycle( 0 );
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
#ifndef ME_SOUND
	scePowerSetClockFrequency(222,222,111);
#endif
#ifdef ME_SOUND
	scePowerLock(0); //sleep mode cannot be triggered with this :-)
#endif

#ifndef HOME_HOOK
	g_updatethread=SetupCallbacks();
#endif

	// create dirs if needed
	checkdirs();

	//init timezone, language, ...
	getsysparam();

	//blitter
	blit_init();


	os9x_adhoc_active=0;
	os9x_getnewfile=1;//start by choosing a file
	in_emu=0;
	os9x_notfirstlaunch=0;
	os9x_netplay=0;
	os9x_netpadindex=0;
	//default romPath is launch directory
	strcpy(romPath,LaunchDir);
	lastRom[0]=0;
	//do some uncaching stuff
	os9x_paused_ptr=(int*)UNCACHE_PTR(&os9x_paused);
	os9x_apuenabled_ptr=(int*)UNCACHE_PTR(&os9x_apuenabled);

	sceKernelDcacheWritebackInvalidateAll();
	(stSoundStatus.sound_fd) = -1;
#ifndef SOUNDDUX_151
	S9xAllocSound();
#endif
	S9xInitAPU();

	SetGeCallback();

	//
	//sprintf(str_tmp,"%sDATA/msg.ini",LaunchDir);
	//save_msg_list(str_tmp);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void low_level_deinit(){
	blit_shutdown();

	msgBoxLines(psp_msg_string(INFO_EXITING),0);

	for (int i=0;i<6;i++) {
		sceAudioChRelease(snd_beep1_handle[i]);
		//sceAudioChRelease(snd_beep2_handle[i]);
	}
	snd_beep1_current=0;
	//snd_beep2_current=0;

	S9xDeinitAPU();
#ifndef SOUNDDUX_151
	S9xFreeSound();
#endif
	//OSK
	if (os9x_osk) danzeff_free();


	//network
#ifdef USE_ADHOC
	adhocTerm();
#endif
	sceKernelDelayThread(1000000); //a try to fix hang on 2.5
}

int scroll_message_input(char *name,int limit) {
	struct Vertex *vertices,*vertices_ptr;
	u16 *scr_bg=(u16*)(0x44000000+(512*272*2)*2);

	SceCtrlData paddata;
	//int oldmenufx;
	int exit_osk;
	unsigned char key,name_pos;
	//oldmenufx=os9x_menufx;
	//os9x_menufx=1;

//	danzeff_load16(LaunchDir);
	if (os9x_osk) {
		if (!danzeff_isinitialized()) {
			psp_msg(ERR_INIT_OSK, MSG_DEFAULT);
			return 0;
		} else {
			danzeff_moveTo(20,20);
			exit_osk=0;
			name_pos=0;
			while (name[name_pos]) name_pos++;
			while (!exit_osk) {

				sceGuStart(GU_DIRECT,list);
				sceGuEnable(GU_TEXTURE_2D);
				sceGuTexFilter(GU_NEAREST,GU_NEAREST);
				sceGuDisable(GU_DEPTH_TEST);
  				sceGuDisable(GU_ALPHA_TEST);
  				//sceGuDepthMask(GU_TRUE);
				sceGuTexMode(GU_PSM_5551,0,0,0); //16bit texture
				sceGuScissor(0,0,480,272);
				sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
				sceGuTexImage(0,512,512,512,(u8*)scr_bg);
  				vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
  				vertices_ptr=vertices;
				vertices_ptr[0].u = 0; vertices_ptr[0].v = 0;
				vertices_ptr[0].x = 0; vertices_ptr[0].y = 0; vertices_ptr[0].z = 0;
				vertices_ptr[1].u = 480; vertices_ptr[1].v = 272;
				vertices_ptr[1].x = 480; vertices_ptr[1].y = 272; vertices_ptr[1].z = 0;
				sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);
				sceGuFinish();
  				sceGuSync(0,0);

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
							if (name_pos<limit-1) name_pos++;
								name[name_pos]=0;
						}
						break;
				}
				mh_printSel(200,20,name,0xFFFF);
				danzeff_render();

				sceDisplayWaitVblankStart();
  				sceGuSwapBuffers();
				pg_drawframe++;
				pg_drawframe&=1;
			}

			while (get_pad()) pgWaitV();

			return (exit_osk==1);
		}
	} else {
		int done = 0,i,j,k;
		// INIT OSK
		unsigned short intext[128]  = { 0 }; // text already in the edit box on start
		unsigned short outtext[128] = { 0 }; // text after input
		unsigned short desc[128]; // description
		SceUtilityOskData data[1];
		SceUtilityOskParams osk;
		struct Vertex *vertices,*vertices_ptr;
		u16 *scr_bg=(u16*)(0x44000000+(512*272*2)*2);

		unsigned char *src = (unsigned char *)psp_msg_string(SCROLL_OSK_DESC);
		for (i = 0; *src; i++) {
			if ((0x80 < *src && *src < 0xA0) || (0xDF < *src && *src < 0xF0)) {
				k = *src++;
				k += *src++ << 8;
			}
			else k = *src++;
			for (j = 0; j < TOTAL_TBL; j++)
				if (map[j][0] == k)
					break;
			if (j == TOTAL_TBL) {
				desc[i] = '?';
				continue;
			}
			desc[i] = map[j][1];
		}
		desc[i] = 0;

		memset(&data, 0, sizeof(data));
		data[0].language = os9x_language; // english
		data[0].lines = 1; // just online
		data[0].unk_24 = 1; // set to 1
		data[0].desc = desc;
		data[0].intext = intext;
		data[0].outtextlength = 128; // sizeof(outtext) / sizeof(unsigned short)
		data[0].outtextlimit = limit; // just allow n chars
		data[0].outtext = outtext;

		memset(intext,0,128*2);
		for(i = 0; name[i]; i++) {
			intext[i]=name[i];
		}

		memset(&osk, 0, sizeof(osk));
		osk.base.size = sizeof(osk);
		sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &osk.base.language);
		sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &osk.base.buttonSwap);
		osk.base.graphicsThread = 17;
		osk.base.accessThread = 19;
		osk.base.fontThread = 18;
		osk.base.soundThread = 16;
		osk.datacount = 1;
		osk.data = data;

	// Only ascii code is handled so only the input of the small letters is printed

		int rc = sceUtilityOskInitStart(&osk);
		if(rc) {
			return 0;
		}
		while(!done) {

			sceGuStart(GU_DIRECT,list);
			sceGuEnable(GU_SCISSOR_TEST);
			sceGuEnable(GU_TEXTURE_2D);
			sceGuTexFilter(GU_NEAREST,GU_NEAREST);
			sceGuDisable(GU_DEPTH_TEST);
  			sceGuDisable(GU_ALPHA_TEST);
  			//sceGuDepthMask(GU_TRUE);
			sceGuTexScale(1.0f/512.0f,1.0f/512.0f);
			sceGuTexOffset(0,0);
			sceGuTexMode(GU_PSM_5551,0,0,0); //16bit texture
			sceGuScissor(0,0,480,272);
			sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
			sceGuTexImage(0,512,512,512,(u8*)scr_bg);
  			vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
  			vertices_ptr=vertices;
			vertices_ptr[0].u = 0; vertices_ptr[0].v = 0;
			vertices_ptr[0].x = 0; vertices_ptr[0].y = 0; vertices_ptr[0].z = 0;
			vertices_ptr[1].u = 480; vertices_ptr[1].v = 272;
			vertices_ptr[1].x = 480; vertices_ptr[1].y = 272; vertices_ptr[1].z = 0;
			sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);
			sceGuFinish();
  			sceGuSync(0,0);

			switch(sceUtilityOskGetStatus()) {
			case PSP_UTILITY_DIALOG_INIT :
				j=mh_length("Initializing OSK...");
				i=(480-j)/2;
				pgDrawFrame(i-5-1,125-1,i+j+5+1,145+1,12|(2<<5)|(2<<10));
  				pgDrawFrame(i-5-2,125-2,i+j+5+2,145+2,28|(10<<5)|(10<<10));
				pgFillBox(i-5,125,i+j+5,145,(20)|(4<<5)|(4<<10));
				mh_print(i,130,"Initializing OSK...",31|(28<<5)|(24<<10));
				break;
			case PSP_UTILITY_DIALOG_VISIBLE :
				sceUtilityOskUpdate(2); // 2 is taken from ps2dev.org recommendation
				break;
			case PSP_UTILITY_DIALOG_QUIT :
				sceUtilityOskShutdownStart();
				break;
			case PSP_UTILITY_DIALOG_FINISHED :
				done = 1;
				break;
			case PSP_UTILITY_DIALOG_NONE :
			default :
				break;
			}

			sceDisplayWaitVblankStart();
			sceGuSwapBuffers();
			pg_drawframe++;
			pg_drawframe&=1;
		}

		if (data[0].result!=PSP_UTILITY_OSK_RESULT_CHANGED) return 0;

		j=0;
		for(i = 0; data[0].outtext[i]; i++) {
			for (k=0;k<TOTAL_TBL;k++) {
				if (data[0].outtext[i]==map[k][1]) {
					if (map[k][0] > 0xFF) name[j++]=map[k][0]>>8;
					name[j++]=map[k][0]&0xFF;
					break;
				}
			}
			if (k == TOTAL_TBL) name[j++]='?';
		}
		name[j]=0;

		return 1;
	}

	//os9x_menufx=oldmenufx;

//	danzeff_free();
}

int scroll_message(char **msg_lines,int lines,int start_pos,int intro_message,char *title) {
		int i,j,l,pos,end_pos,oldpos,fakedpos,col1,savedf;
		u16 *scr_bg,*src,*dst,*srctxt,*srctxt2;
		struct Vertex* vertices,*vertices_ptr;
		int found=0;
		char tofind[32];
		tofind[0]=0;
		//2 pages, draw frame is 0
		pgScreenFrame(2,1);

		//cheap scroller, using 3 screens

		srctxt=(u16*)(0x44000000+(512*272*3)*2);
		memset(srctxt,0x0,272*512*2*2);
		for (i=0;(i<lines)&&(i<27);i++) if (msg_lines[i]) {
			if (intro_message) {
				if (i<(lines-2)) col1=0xffff;
				else if (i==(lines-2)) col1=10|(31<<5)|(10<<10);
				else col1=31|(10<<5)|(10<<10);
			} else col1=31|(31<<5)|(31<<10);

			mh_printLimit(0,i*10+272*3,480,272*5,msg_lines[i],col1);
		}
		for (i=0;i<270*512*2;i++) {
			if (!(srctxt[i])) srctxt[i]=0xFFFF;
		}

		//now show messages & ask to scroll until the end
		scr_bg=(u16*)(0x04000000+(512*272*2)*2);//(u16*)malloc(480*272*2);
		dst=scr_bg;
		show_background(bg_img_mul,(os9x_lowbat?0x600000:0));

		pgFillBoxHalfer(0,0,479,9);
		pgDrawFrame(0,9,479,9,(12<<10)|(8<<5)|5);
		pgDrawFrame(0,10,479,10,(16<<10)|(14<<5)|14);
		pgDrawFrame(0,11,479,11,(12<<10)|(8<<5)|5);


		pgFillBoxHalfer(0,261,479,271);
		pgDrawFrame(0,259,479,259,(12<<10)|(8<<5)|5);
		pgDrawFrame(0,260,479,260,(16<<10)|(14<<5)|14);
		pgDrawFrame(0,261,479,261,(12<<10)|(8<<5)|5);


		mh_print(0,0,title,31|(31<<5)|(31<<10));
		strcpy(str_tmp,psp_msg_string(SCROLL_TITLE));
		mh_print(479-mh_length(str_tmp),0,(char*)str_tmp,31|(31<<5)|(31<<10));
		sprintf(str_tmp,"  " SJIS_UP "  " SJIS_DOWN "           L R              ");
		mh_print(479-mh_length(str_tmp),0,(char*)str_tmp,20|(31<<5)|(18<<10));

		if (!intro_message) {
			strcpy(str_tmp,psp_msg_string(SCROLL_STATUS_1));
			mh_print(479-mh_length(str_tmp),262,(char*)str_tmp,31|(31<<5)|(31<<10));
			sprintf(str_tmp,SJIS_CROSS "       SELECT       ");
			mh_print(479-mh_length(str_tmp),262,(char*)str_tmp,20|(31<<5)|(18<<10));
		}


		for (i=0;i<272;i++) {
			src = (u16*)pgGetVramAddr(0,i);
			memcpy(dst,src,480*2);
			dst+=512;
		}


		blit_reinit();
		oldpos=-10;
		pos=start_pos;

		int exit_message=0;
		int scroll_speed,scroll_accel;
		int pad_val,oldpad_val,lx,ly;
		scroll_accel=0;
		scroll_speed=1;
		pad_val=0;
		end_pos=(lines-26)*10;
		if (end_pos<0) end_pos=0;

		while (!exit_message) {

			fakedpos=pos%270;

			if (oldpos!=pos) {
				//scroll down / scroll down
				if (abs(oldpos-pos)>9) {
					pos=(pos/10)*10;
					fakedpos=pos%270;
					i=pos/10;
					j=fakedpos/10;
					srctxt=(u16*)(0x44000000+(512*(272*3+ j*10) )*2);
					memset(srctxt,0x0,272*512*2);

					savedf=pg_drawframe;
					pg_drawframe=0;
					for (l=i;(l<lines)&&(l<i+27);l++,j++) if (msg_lines[l]) {
						if (intro_message) {
							if (l<(lines-2)) col1=0xffff;
							else if (l==(lines-2)) col1=10|(31<<5)|(10<<10);
							else col1=31|(10<<5)|(10<<10);
						} else col1=31|(31<<5)|(31<<10);
						mh_printLimit(0,j*10+272*3,480,272*5,msg_lines[l],col1);
						//highlight searched string by drawing over a string with non searched part blanked
						if (found==l) {
							char *p,*q;
							strcpy(str_tmp,msg_lines[l]);
							p=str_tmp;
							while ((q=strstr(strupr(p),tofind))) {
								while (p<q) *p++=' ';
								p=q+strlen(tofind);
							}
							while (*p) *p++=' ';
							col1=10|(10<<5)|(31<<10);
							mh_printLimit(0,j*10+272*3,480,272*5,str_tmp,col1);
						}
					}
					pg_drawframe=savedf;
					for (i=0;i<270*512;i++) {
						if (!(srctxt[i])) srctxt[i]=0xFFFF;
					}
					if (fakedpos) {
						srctxt=(u16*)(0x44000000+(512*(272*3+ fakedpos) )*2);
						srctxt2=(u16*)(0x44000000+(512*(272*3+ 270+fakedpos) )*2);
						memcpy(srctxt2,srctxt,(270-fakedpos)*512*2);

						srctxt=(u16*)(0x44000000+(512*(272*3+ 270) )*2);
						srctxt2=(u16*)(0x44000000+(512*(272*3+ 0) )*2);
						memcpy(srctxt2,srctxt,(270-fakedpos)*512*2);
					} else {
						srctxt=(u16*)(0x44000000+(512*(272*3)*2));
						srctxt2=(u16*)(0x44000000+(512*(272*3+ 270) )*2);
						memcpy(srctxt2,srctxt,270*512*2);
					}
				}
				i=pos/10+(oldpos>pos?0:27);
				j=fakedpos/10+(oldpos>pos?0:27);
				l=fakedpos/10+(oldpos>pos?27:0);
				if (i<lines) {
					if (intro_message) {
						if (i<(lines-2)) col1=0xffff;
						else if (i==(lines-2)) col1=10|(31<<5)|(10<<10);
						else col1=31|(10<<5)|(10<<10);
					} else col1=31|(31<<5)|(31<<10);

					srctxt=(u16*)(0x44000000+(512*(272*3+ j*10) )*2);
					srctxt2=(u16*)(0x44000000+(512*(272*3+l*10 ))*2);
					memset(srctxt,0x00,512*11*2);
					if (msg_lines[i]) {
						savedf=pg_drawframe;
						pg_drawframe=0;
						mh_printLimit(0,j*10+272*3,480,272*5,msg_lines[i],col1);
						pg_drawframe=savedf;
					}
					for (j=0;j<11*512;j++) {
						if (!(srctxt[j])) srctxt[j]=0xFFFF;
						//if (!(srctxt2[j])) srctxt2[j]=0xFFFF;
					}
					memcpy(srctxt2,srctxt,512*11*2);
				}
			}

			oldpos=pos;
			sceGuStart(GU_DIRECT,list);

			sceGuEnable(GU_TEXTURE_2D);
			sceGuTexFilter(GU_NEAREST,GU_NEAREST);
			sceGuDisable(GU_DEPTH_TEST);
  		sceGuDisable(GU_ALPHA_TEST);
  		//sceGuDepthMask(GU_TRUE);

			sceGuTexMode(GU_PSM_5551,0,0,0); //16bit texture
			sceGuScissor(0,0,480,272);


			sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
			sceGuTexImage(0,512,512,512,(u8*)scr_bg);

  		vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
  		vertices_ptr=vertices;
		  vertices_ptr[0].u = 0; vertices_ptr[0].v = 0;
		  vertices_ptr[0].x = 0; vertices_ptr[0].y = 0; vertices_ptr[0].z = 0;
		  vertices_ptr[1].u = 480; vertices_ptr[1].v = 272;
		  vertices_ptr[1].x = 480; vertices_ptr[1].y = 272; vertices_ptr[1].z = 0;
		  sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);

		  //sceGuTexFunc(GU_TFX_ADD,GU_TCC_RGBA);
		  sceGuEnable(GU_ALPHA_TEST);
		  sceGuAlphaFunc(GU_EQUAL,0,0x1);

		  sceGuScissor(0,12,480,259);
		  srctxt=(u16*)(0x44000000+(512*(272*3+fakedpos))*2);
		  sceGuTexImage(0,512,512,512,(u8*)srctxt);
		  sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);

		  sceGuFinish();
  		sceGuSync(0,0);


  		//memset(pgGetVramAddr(0,272-10),0,512*10*2);
		  sprintf(str_tmp,psp_msg_string(SCROLL_STATUS_0),pos/10+26,lines,(pos/10+26)/27,lines/27);
		  mh_print(0,272-10,str_tmp,((31)|(28<<5)|(31<<10)));

  		sceDisplayWaitVblankStart();
  		sceGuSwapBuffers();
			pg_drawframe++;
			pg_drawframe&=1;

			for (;;) {
				oldpad_val=pad_val;
				while (!(pad_val=get_pad2(&lx,&ly))) {
					if (abs(lx-128)>20) break;
					if (abs(ly-128)>20) break;
					pgWaitV();scroll_speed=0;scroll_accel=0;
				}

				if ((pad_val&PSP_CTRL_RTRIGGER)&&(pos<end_pos)) {
					if (oldpad_val==pad_val) {
						if (scroll_accel<1024) {
							scroll_accel++;
							scroll_speed=(scroll_accel>>3)+1;
						}
					} else {
						scroll_speed=0;scroll_accel=0;
					}
					pos+=5*27*scroll_speed;
					if (pos>=end_pos) pos=end_pos;
					break;
				} else if ((pad_val&PSP_CTRL_LTRIGGER)&&(pos>0)) {
					if (oldpad_val==pad_val) {
						if (scroll_accel<1024) {
							scroll_accel++;
							scroll_speed=(scroll_accel>>3)+1;
						}
					} else {
						scroll_speed=0;scroll_accel=0;
					}
					pos-=5*27*scroll_speed;
					if (pos<0) pos=0;
					break;
				} else if ((pad_val&PSP_CTRL_DOWN)&&(pos<end_pos)) {
					if (oldpad_val==pad_val) {
						if (scroll_accel<256) {
							scroll_accel++;
							scroll_speed=(scroll_accel>>5)+1;
						}
					} else {
						scroll_speed=0;scroll_accel=0;
					}
					pos+=scroll_speed;
					if (pos>=end_pos) pos=end_pos;
					break;
				} else if ((pad_val&PSP_CTRL_UP)&&(pos>=0)) {
					if (oldpad_val==pad_val) {
						if (scroll_accel<256) {
							scroll_accel++;
							scroll_speed=(scroll_accel>>5)+1;
						}
					} else {
						scroll_speed=0;scroll_accel=0;
					}
					pos-=scroll_speed;
					if (pos<0) pos=0;
					break;
				} else if ((ly>=128+40)&&(pos<end_pos)) {
					int r=ly-128;
					if (r<127) pos+= r/16;
					else pos += 270;
					if (pos>end_pos) pos=end_pos;
					break;
				} else if ((ly<=128-20)&&(pos>0)) {
					int r=128-ly;
					if (r<127) pos-= r/16;
					else pos -= 270;
					if (pos<0) pos=0;
					break;
				}

				if (intro_message) {
					if (pos>=end_pos) { //reached the end of message
						if (pad_val&(PSP_CTRL_CIRCLE|PSP_CTRL_CROSS)) {exit_message=1;break;}
						if (pad_val&(PSP_CTRL_TRIANGLE|PSP_CTRL_SQUARE|PSP_CTRL_SQUARE/*|PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER*/)) {
							S9xExit();
							exit_message=1;break;
						}
					}
				} else {
					if (pad_val&PSP_CTRL_CROSS) { //exit
						exit_message=1;
						break;
					}	else if (pad_val&PSP_CTRL_SELECT) { //minihelp
						psp_msg(SCROLL_HELP, MSG_DEFAULT);
						while (!(get_pad()&PSP_CTRL_CROSS));
						while (get_pad());
						break;
					} else if (pad_val&PSP_CTRL_TRIANGLE) { //search from position
						if (scroll_message_input(tofind,31)) {
							found=0;
							if (tofind[0]) {
								psp_msg(SCROLL_SEARCHING, MSG_DEFAULT);
								strcpy(tofind,strupr(tofind));
								j=pos/10-1;
								if (j<0) j=0;
								for (i=j;i<lines;i++)
									if (msg_lines[i]) {
										if (strstr(strupr(msg_lines[i]),tofind)) {found=i;pos=(i-2)*10;if (pos<0) pos=0;break;}
									}
							}
							if (!found) psp_msg(SCROLL_STRNOTFOUND, MSG_DEFAULT);
						}
						break;
					}	else if ((pad_val&PSP_CTRL_CIRCLE)&&found) { //search again from position & loop if needed
						psp_msg(SCROLL_SEARCHING, MSG_DEFAULT);
						i=pos/10+2;
						if (i>=lines) i=0;
						j=i; //just to be safe, should not be needed
						for (;;) {
							i++;
							if (i>=lines) i=0;
							if (i==j) break; //just to be safe, should not be needed
							if (msg_lines[i]) {
								if (strstr(strupr(msg_lines[i]),tofind))  {found=i;pos=(i-2)*10;if (pos<0) pos=0;break;}
							}
						}
						break;
					}	else if ((pad_val&PSP_CTRL_SQUARE)&&found) { //search again from position & loop if needed
						psp_msg(SCROLL_SEARCHING, MSG_DEFAULT);
						i=pos/10+2;
						if (i>=lines) i=0;
						j=i; //just to be safe, should not be needed
						for (;;) {
							i--;
							if (i<0) i=lines-1;
							if (i==j) break; //just to be safe, should not be needed
							if (msg_lines[i]) {
								if (strstr(strupr(msg_lines[i]),tofind))  {found=i;pos=(i-2)*10;if (pos<0) pos=0;break;}
							}
						}
						break;
					}
				}
			}
		}
		while (get_pad()) pgWaitV();
		return pos;
}
////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void show_message() {
#define BLANK_LINES 18
		char *decrypted_message,*p;
		int message_size;
		char password[32];
		char **msg_lines;
		int i,lines;


		strcpy(password,PASSWORD_XORED);
		message_size=sizeof(message);
		decrypted_message=(char*)malloc(message_size+1);
		decrypt((char*)message,decrypted_message,message_size,password);

		//pgFillBox(240-150+1,136-60+1,240+150-1,136+50-1,(16<<10)|(10<<5)|7);
		//pgDrawFrame(240-150,136-60,240+150,136+50,(12<<10)|(8<<5)|5);

		//prepare message to be diplayed

		//decrypt
		p=decrypted_message;
		p[message_size]=0;
		// 'lineify' it
		lines=0;
		while (*p) {
			i=0;
			//get new line
			while ((p[i]!=0x0D)&&(p[i])) i++;
			//if line carriage return, skip it & put a '0' / end of string
			if (p[i]==0x0D) {
				p[i]=0;p[i+1]=0; //0x0D 0x0A
				i+=2;
			}
			lines++;
			p=p+i;
		}

		lines+=BLANK_LINES;
		msg_lines=(char**)malloc(sizeof(char*)*lines);
		for (i=0;i<BLANK_LINES ;i++) {
			msg_lines[i]=NULL;
		}
		p=decrypted_message;
		for (i=BLANK_LINES ;i<lines;i++){
			msg_lines[i]=(char*)malloc(strlen(p)+1);
			strcpy(msg_lines[i],p);
			p=p+strlen(p)+2;
		}
		//free decrypted raw message
		free(decrypted_message);

		scroll_message(msg_lines,lines,0,1,psp_msg_string(SCROLL_DISCLAIMER));

		//free 'linified' message
		for (i=0;i<lines;i++) if (msg_lines[i]) free(msg_lines[i]);
		free(msg_lines);

		//free(scr_bg);
}


////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void welcome_message(){
	char str[256];
	int res,show_msg=1;
	SceIoStat stat;
	ScePspDateTime *tfile;

	sprintf(str,"%s%s",LaunchDir,"s9xTYL.ini");
	res=sceIoGetstat("s9xTYL.ini",&stat);
	if (res<0) res=sceIoGetstat(str,&stat);
	if (res>=0) {

		tfile=&(stat.st_mtime);
		time_t cur_time;
		struct tm *tsys;
		sceKernelLibcTime(&cur_time);
		cur_time+=os9x_timezone*60+os9x_daylsavings*3600;;
		tsys=localtime(&cur_time);
		show_msg=0;
	}
	if (show_msg) show_message();

#ifdef ME_SOUND
	//if (show_msg) msgBoxLines("WARNING EXPERIMENTAL BETA VERSION\n\nSound is emulated by MEDIA ENGINE\n\nSLEEP MODE IS NOT SUPPORTED",2*60);
#endif

}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_getfile() {
#ifdef __DEBUG_SNES_
	sprintf(rom_filename,__DEBUG__ROM__);
	strcpy(LastPath,"ms0:/PSP/GAME/snes9xTYL/");
#else
	strcpy(rom_filename,lastRom);

	bypass_rom_settings=getFilePath(rom_filename,os9x_notfirstlaunch)-1;
	if (bypass_rom_settings<0) return 0;

	strcpy(lastRom,os9x_shortfilename(rom_filename));
	strcpy(romPath,LastPath);
#endif
	char *file_ext=strrchr((const char *)rom_filename,'/');
	if (!file_ext) file_ext=rom_filename;
  strcpy(shortrom_filename,file_ext+1);

	if (strcasecmp(os9x_filename_ext(rom_filename),"spc")==0) {
		return 2;
	}
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int init_snes_rom() {
	///////////////////
	//Settings
	///////////////////
	memset( &Settings, 0, sizeof( Settings ) );
	memset( &Cheat, 0, sizeof( struct SCheatData ) );
	// ROM Options
	Settings.SDD1Pack = true;
  Settings.ForceLoROM = false;
  Settings.ForceInterleaved = false;
  Settings.ForceNotInterleaved = false;
  Settings.ForceInterleaved = false;
  Settings.ForceInterleaved2 = false;
  Settings.ForcePAL = false;
  Settings.ForceNTSC = false;
  Settings.ForceHeader = false;
  Settings.ForceNoHeader = false;
  // Sound options
  Settings.SoundSync = 0;
  Settings.InterpolatedSound = true;
  Settings.SoundEnvelopeHeightReading = true;
  Settings.DisableSoundEcho = false;
  Settings.DisableMasterVolume = false;
  Settings.Mute = FALSE;
  Settings.SoundSkipMethod = 0;
  Settings.SoundPlaybackRate = os9x_sndfreq;
  Settings.SixteenBitSound = true;
  Settings.Stereo = true;
  Settings.AltSampleDecode = 0;//os9x_sampledecoder;
  Settings.ReverseStereo = FALSE;
  Settings.SoundBufferSize = 1024;//4;
  Settings.SoundMixInterval = 0;//20;
	Settings.DisableSampleCaching=TRUE;
	Settings.FixFrequency = true;
	// Tracing options
  Settings.TraceDMA = false;
  Settings.TraceHDMA = false;
  Settings.TraceVRAM = false;
  Settings.TraceUnknownRegisters = false;
  Settings.TraceDSP = false;
  // Joystick options
  Settings.SwapJoypads = false;
  Settings.JoystickEnabled = false;
	// ROM timing options (see also H_Max above)
  Settings.PAL = false;
  Settings.FrameTimePAL = 20;
  Settings.FrameTimeNTSC = 17;
  // CPU options
  Settings.CyclesPercentage = os9x_CyclesPercentage;
  Settings.Shutdown = true;
  Settings.ShutdownMaster = true;
  Settings.NextAPUEnabled = Settings.APUEnabled = (os9x_apuenabled==1)||(os9x_apuenabled==2);
#ifdef DEBUGGER
  Settings.DisableIRQ = os9x_DisableIRQ;
#endif
  Settings.Paused = false;
  Settings.H_Max = SNES_CYCLES_PER_SCANLINE;
  Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;
  if (os9x_fskipvalue==AUTO_FSKIP) {
  	Settings.SkipFrames=AUTO_FRAMERATE;
  	os9x_autofskip_SkipFrames=0;
  } else Settings.SkipFrames=os9x_fskipvalue;
  // ROM image and peripheral options
  Settings.ForceSuperFX = false;
  Settings.ForceNoSuperFX = false;
  Settings.MultiPlayer5 = true;
  Settings.Mouse = true;
  Settings.SuperScope = true;
  Settings.MultiPlayer5Master = true;
  Settings.SuperScopeMaster = true;
  Settings.MouseMaster = true;
  Settings.SuperFX = false;
  // SNES graphics options
  Settings.BGLayering = false;
  Settings.DisableGraphicWindows = false;
  Settings.ForceTransparency = false;
  Settings.ForceNoTransparency = false;
  Settings.DisableHDMA = os9x_DisableHDMA;
  Settings.Mode7Interpolate = false;
  Settings.DisplayFrameRate = false;

  Settings.SixteenBit = 1;
  Settings.Transparency = 1;
  Settings.SupportHiRes = false;

  Settings.AutoSaveDelay = 1;
  Settings.ApplyCheats = true;

  os9x_TurboMode = 0;
  Settings.TurboSkipFrames = 20;
  Settings.AutoMaxSkipFrames = 10;

  Settings.ForcedPause = 0;
  Settings.StopEmulation = TRUE;
  Settings.Paused = FALSE;
  Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;

  ///////////////////
  ///////////////////
  if (  !Memory.Init() ) {
  	psp_msg(ERR_INIT_SNES, MSG_DEFAULT);
		return -1;
	}
	S9xInitSound( Settings.SoundPlaybackRate, Settings.Stereo, Settings.SoundBufferSize );
	S9xSetSoundMute( TRUE );

	uint32 saved_flags = CPUPack.CPU.Flags;

	//msgBoxLines("Loading ROM...",0);
	pgCopyScreen();

	if ( !Memory.LoadROM( rom_filename ) ){
		psp_msg(ERR_LOADING_ROM, MSG_DEFAULT);
		return -1;
	} else {
		Memory.LoadSRAM( (char*)S9xGetSaveFilename( ".SRM" ) );

		if (!bypass_rom_settings) {
			if (int ret=load_rom_settings(Memory.ROMCRC32)) {
				if (ret==-3) {
					psp_msg(SETTINGS_NOTCOMPLETE, MSG_DEFAULT);
				}
				else {
					psp_msg(SETTINGS_NOTFOUND, MSG_DEFAULT);
					if (load_rom_settings(0)) {
						if (!os9x_lowbat) save_rom_settings(0,"default");
					}
				}
			}
		} else {
			psp_msg(SETTINGS_FORCING_DEFAULT, MSG_DEFAULT);
			if (load_rom_settings(0)) {
					if (!os9x_lowbat) save_rom_settings(0,"default");
			}
		}


		//net stuff, called here to have settings loaded and so server can broadcast them
		if (os9x_netplay) {
#ifdef USE_ADHOC
				if (os9x_adhoc_active) { //if adhoc active, close it first
					adhocTerm();
					os9x_adhoc_active=0;
				}
				if (psp_initadhocgame()) { //try to initiate a adhoc game
					psp_msg(ADHOC_INIT_ERR, MSG_DEFAULT);
					adhocTerm();  //unavailable, no netplay
					os9x_adhoc_active=0;
					os9x_netplay=0;
				} else {
					os9x_adhoc_active=1; //netplay ready
					os9x_padfirstcall=1;
					os9x_applyhacks=0;

					if (os9x_conId==1) { //server
						sceKernelDelayThread(4000000); //wait 4s
						net_send_settings();
					} else {
						net_receive_settings();

					}

				}
#endif
		}

		if ((os9x_applyhacks)&&(os9x_findhacks(Memory.ROMCRC32))) {
			msgBox(psp_msg_string(INIT_SPEEDHACK));
		}
	}

	CPUPack.CPU.Flags = saved_flags;

	S9xInitDisplay();
	if ( !S9xGraphicsInit() ){
		psp_msg(ERR_INIT_GFX, MSG_DEFAULT);
		return -1;
	}




	if (os9x_apuenabled==2){
		S9xSetSoundMute( false );
	}
#ifdef __DEBUG_SNES_
	os9x_load(DEBUG_SAVE_SLOT);
#endif

	os9x_paused=*os9x_paused_ptr=0;
	resync_var();
	InitSoundThread();
	if (os9x_apuenabled==2)	S9xSetSoundMute( false );
	else S9xSetSoundMute( true );

	in_emu=1;

	s_iFrame = 0;
	s_TotalFrame = 0;
	os9x_updatepadFrame = 0;
	os9x_snespad=0;
	os9x_oldsnespad=0;
	memset(os9x_netsnespad,0,sizeof(os9x_netsnespad));;memset(os9x_netcrc32,0,sizeof(os9x_netcrc32));
	os9x_netsynclost=0;
	os9x_oldframe=0;
	os9x_updatepadcpt=0;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void close_snes_rom(){
	os9x_paused=*os9x_paused_ptr=1;
	StopSoundThread();
#ifndef ME_SOUND
	scePowerSetClockFrequency(222,222,111);
#endif
	Settings.Paused = TRUE;

	S9xGraphicsDeinit();
	Memory.Deinit();

	S9xCloseSoundDevice();
}

//struct SAPUPACK
//{
//	struct SAPURegisters APURegisters;	// 8bytes
//	struct SIAPU IAPU;					// 20bytes
//	struct SAPU APU;					// 228bytes
//};
#if 0
void me_apu_debug_me_StartSound (){
	me_sound_t *p=(me_sound_t *)((int)(&me_sound_data)|0x40000000);

	sceKernelDcacheWritebackInvalidateAll();
	CallME(me_data, (int)(&me_Dummy),0, -1, 0, 0, 0); // all we want is the ME to inv its dcache

	//p->buffer[0]=(uint8 *)( ((int)SoundBuffer[0])|0x40000000 );
	//p->buffer[1]=(uint8 *)( ((int)SoundBuffer[1])|0x40000000 );
	//p->buffer[2]=(uint8 *)( ((int)SoundBuffer[2])|0x40000000 );
	//p->buffer[3]=(uint8 *)( ((int)SoundBuffer[3])|0x40000000 );
	//if(p->buffer_idx<0 || p->buffer_idx>2)
	//	p->buffer_idx=2;
	//p->sample_count=((stSoundStatus.buffer_size))>>1;
	//p->freqratio=snd_freqratio;
	//p->os9x_apuenabled_ptr=(int*)( ((int)&os9x_apuenabled)|0x40000000);
	//p->os9x_paused_ptr=(int*)( ((int)&os9x_paused)|0x40000000);
	p->exit=0;
	BeginME(me_data, (int)(&me_MixSound), (int)p,-1,0,-1, 0);
}
int me_apu_debug_S9xProcessSound (SceSize ,void *) {
	int i;
	uint8 *apu_ram_save;
	//u32 sample_count=(stSoundStatus.buffer_size)>>1;
	//memset((char*)SoundBuffer[0],0,sample_count*2);
	//memset((char*)SoundBuffer[1],0,sample_count*2);
	//for (i = 0; i < 256; i++) S9xAPUCycles [i] = (int)S9xAPUCycleLengths [i] * (int)(IAPU.OneCycle) *os9x_apu_ratio / 256;
	//me_apu_debug_me_StartSound();
	me_sound_t *p=(me_sound_t *)((int)(&me_sound_data)|0x40000000);
	p->mixsample_flag=0;
	me_apu_debug_me_StartSound();
	do {
		if (!Settings.ThreadSound) break;
		i=p->buffer_idx;
		p->mixsample_flag=1;
		if ((os9x_apuenabled==2)&&(!os9x_paused)) {
			sceAudioOutputPannedBlocking( (stSoundStatus.sound_fd), MAXVOLUME, MAXVOLUME, (char*)((int)SoundBuffer[i]|0x00000000));
			//sprintf(me_debug_str,"mixsample_flag:%d\n",p->mixsample_flag);
		} else {
			sprintf(me_debug_str,"a:%dp:%d\n",os9x_apuenabled,os9x_paused);
			sceKernelDelayThread(200*1000);//200ms wait
		}
	} while (Settings.ThreadSound);
	p->exit=1;
	WaitME(me_data);
  return (0);
}
void me_apu_debug(int flag)
{
	if(flag==0)
	{
//		os9x_paused=*os9x_paused_ptr=1;
		StopSoundThread();
//		Settings.Paused = TRUE;

			psp_msg(APU_DEBUGGING, MSG_DEFAULT);

//		pgFillAllvram(0);
//		Settings.Paused = false;
		//os9x_paused=0;
//		os9x_paused=*os9x_paused_ptr=0;
		//reinit blitter
//		blit_reinit();
		//invalidate all gfx caches
//		ZeroMemory (IPPU.TileCached [TILE_2BIT], MAX_2BIT_TILES<<1);
//		ZeroMemory (IPPU.TileCached [TILE_4BIT], MAX_4BIT_TILES<<1);
//		ZeroMemory (IPPU.TileCached [TILE_8BIT], MAX_8BIT_TILES<<1);
//		tile_askforreset(-1);
//		resync_var();
//		if (os9x_apuenabled==2)	S9xSetSoundMute( false );
//		else S9xSetSoundMute( true );
			apu_init_after_load|=4;
		
			//InitSoundThread();

		if (g_sndthread!=-1) {ErrorExit( "Thread Exist" );return;}
		g_sndthread = sceKernelCreateThread( "sound thread", (SceKernelThreadEntry)me_apu_debug_S9xProcessSound, 0x8, 256*1024, 0, 0 );
		if ( g_sndthread < 0 ){ErrorExit( "Thread failed" );return;}
//		Settings.SoundPlaybackRate = os9x_sndfreq;
//		snd_freqratio = (u32)(Settings.SoundPlaybackRate)*(1<<16) / 44100;
//		snd_freqerr=0;
//		samples_error=0;
////		current_SoundBuffer=0;
//		S9xSetPlaybackRate( Settings.SoundPlaybackRate  );
		Settings.ThreadSound = true;
		sceKernelStartThread( g_sndthread, 0, 0 );
		sceKernelDelayThread(100*1000);
		while (apu_init_after_load) {
			sceKernelDelayThread(100*1000);
		}

	}
}
#endif

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void open_snes_rom() {
	if (init_snes_rom()) {
		close_snes_rom();
		return;
	}
	os9x_getnewfile=0;
	os9x_notfirstlaunch=1;
	pgFillAllvram(0);pgScreenFrame(2,0);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int user_main(SceSize args, void* argp) {
#ifdef GFX_TILE_ME
	ME_GFX->count=0;
#endif
	low_level_init();

	initvar_withdefault();
	load_settings();

	//OSK
	if (os9x_osk) danzeff_load16(LaunchDir);

	load_background();

	load_icons();

	welcome_message();

	filer_init(psp_msg_string(FILER_TITLE), romPath);

	sprintf(os9x_viewfile_path,"%sFAQS/",LaunchDir);

	//sound stuff for menu
	for (int i=0;i<6;i++) {
		snd_beep1_handle[i]=sceAudioChReserve( -1, ((size_snd_beep2-44)/8)&(~63), 0 );
		//snd_beep2_handle[i]=sceAudioChReserve( -1, ((size_snd_beep2-44)/4)&(~63), 0 );
	}

	if (os9x_autostart) {
		strcpy(rom_filename, romPath);
		strncat(rom_filename, lastRom, 255 - strlen(romPath));
		open_snes_rom();
	}

	while ( g_bLoop ) {

		if (os9x_getnewfile) {
			pgFillAllvram(0);
			pgScreenFrame(2,0);

			if (in_emu==1) {//currently emulating a game => save SRAM before loading new one
				if (!os9x_lowbat) {
					Memory.SaveSRAM( (char*)S9xGetSaveFilename(".SRM") );
					//S9xSaveCheatFile( (char*)S9xGetSaveFilename( ".cht" ) );
				}
				before_pause();
			}

			//initUSBdrivers();

			//1 is snes rom, 2 is spc file
			switch (os9x_getfile()) {
				case 0:
					if (in_emu==1) {
						after_pause();
					}
					os9x_getnewfile=0;
					pgFillAllvram(0);pgScreenFrame(2,0);
					break;
				case 1:
					if (in_emu==1) { //a snes rom was being emulated, close stuff
						close_snes_rom();
						in_emu=0;
					}
					save_settings();
					open_snes_rom();
				break;
				case 2:
					if (in_emu==1) { //a snes rom was being emulated, close stuff
						close_snes_rom();
						in_emu=0;
					}
					pgFillAllvram(0);pgScreenFrame(2,0);

					in_emu=2;
					//play spc, blocking
					psp_msg(BGMUSIC_PLAYING, MSG_DEFAULT);
					OSPC_Play(rom_filename,0,MAXVOLUME);
					blit_reinit();
					set_cpu_clock();

					//ask for a new file since we finished playing the current one
					os9x_getnewfile=1;
					os9x_notfirstlaunch=0;
					pgFillAllvram(0);pgScreenFrame(2,0);
					in_emu=0;
				break;
			}

#ifndef FW3X
			endUSBdrivers();
#endif
		}

		if ((in_emu==1) && ( !Settings.Paused )){
			S9xMainLoop();
		static int printed=false;
		extern uint32 g_nCount;
		if(g_nCount>200 && printed==false)
		{
			printed=true;
			debug_dump("PPU2100Dump");
		}
/*				char szBuf[16];
				sprintf(szBuf,"EXE:%d",IAPU_APUExecuting);
				pgPrintBG(0,3,0xffff,szBuf);	*/		
		}
		//if(strlen(me_debug_str)!=0){
		//	FileLog(me_debug_str);me_debug_str[0]=0;
		//}
		if (g_bSleep){
#ifdef ME_SOUND
			os9x_specialaction|=OS9X_MENUACCESS;
			S9xProcessEvents(false);
#else
			while(g_bSleep) pgWaitV();
			pgWaitVn(60*3);//give some times to wake up, 3seconds
			resync_var();
			InitSoundThread();
			if (os9x_apuenabled==2)	S9xSetSoundMute( false );
			else S9xSetSoundMute( true );
			*os9x_paused_ptr=0;
#endif
		}
#ifdef HOME_HOOK
    if( readHomeButton() > 0 )
    {
			os9x_specialaction|=OS9X_MENUACCESS;
			S9xProcessEvents(false);
    }
#endif
	}

	if (!os9x_lowbat) {
		save_settings();
		if (in_emu==1) {
			Memory.SaveSRAM( (char*)S9xGetSaveFilename(".SRM") );
			//S9xSaveCheatFile( (char*)S9xGetSaveFilename( ".cht" ) );
			save_rom_settings(Memory.ROMCRC32,Memory.ROMName);
			close_snes_rom();
			in_emu=0;
		}
	}

	low_level_deinit();

	if (bg_img) image_free(bg_img);
	for (int i=0;i<8;i++) if (icons[i]) {free(icons[i]);icons[i]=NULL;}

	return 0;
}


}
/////////////////////////////////////////////////////////////////////
// カウンタ実態

uint32 g_nCount;
clock_t g_ulStart;

void MyCounter_Init(void)
{
	g_nCount = 0;
	g_ulStart = sceKernelLibcClock();
	debug_count=0;
	//for(int i=0;i<100;i++)debug_counts[i]=0;
}
void MyCounter_drawCount()
{
	//if(!os9x_showcounter)
	//	return;
	if(!os9x_showfps)
		return;
	
	if (g_ulStart != 0xFFFFFFFF) {
		clock_t dwTime = sceKernelLibcClock();
		if (20000000 < dwTime - g_ulStart) {
			g_ulStart = 0xFFFFFFFF;
		}
		else {
			g_nCount++;
		}
	}
	char szBuf[16];
	sprintf(szBuf,"%d",g_nCount);
	pgPrintBG(0,0,0xffff,szBuf);
return;
	//pgPrintBG(0,1,0xffff,debug_str);	
	//sprintf(szBuf,"%d",debug_count);
	//pgPrintBG(0,2,0xffff,szBuf);	
	////pgPrintBG(0,2,0xffff,debug_str);	
	//pgPrintBG(0,3,0xffff,me_debug_str);


	//sprintf(szBuf,"%d:%d:%d",debug_counts[0],debug_counts[1],debug_counts[2]);
	//pgPrintBG(0,4,0xffff,szBuf);
	//sprintf(szBuf,"%d:%d:%d",debug_counts[3],debug_counts[4],debug_counts[5]);
	//pgPrintBG(0,5,0xffff,szBuf);
	//sprintf(szBuf,"%d:%d:%d",debug_counts[6],debug_counts[7],debug_counts[8]);
	//pgPrintBG(0,6,0xffff,szBuf);
	//sprintf(szBuf,"%d:%d:%d",debug_counts[9],debug_counts[10],debug_counts[11]);
	//pgPrintBG(0,7,0xffff,szBuf);
	//sprintf(szBuf,"%d:%d:%d",debug_counts[12],debug_counts[13],debug_counts[14]);
	//pgPrintBG(0,8,0xffff,szBuf);
	//sprintf(szBuf,"%d:%d:%d",debug_counts[15],debug_counts[16],debug_counts[17]);
	//pgPrintBG(0,9,0xffff,szBuf);
	//sprintf(szBuf,"%d:%d:%d",debug_counts[18],debug_counts[19],debug_counts[20]);
	//pgPrintBG(0,10,0xffff,szBuf);
	//sprintf(szBuf,"%d:%d:%d",debug_counts[21],debug_counts[22],debug_counts[23]);
	//pgPrintBG(0,11,0xffff,szBuf);
	//sprintf(szBuf,"%d:%d:%d",debug_counts[24],debug_counts[25],debug_counts[26]);
	//pgPrintBG(0,12,0xffff,szBuf);
	//sprintf(szBuf,"%d:%d:%d",debug_counts[30],debug_counts[31],debug_counts[32]);//IPPU.ClipFixMaxCount;
	//pgPrintBG(0,14,0x8fff,szBuf);
	//sprintf(szBuf,"BG:%d",PPU.BGMode);
	//pgPrintBG(0,15,0xffff,szBuf);
	//sprintf(szBuf,"SC%d:MC%d",debug_counts[SUBSCREEN_COLOUR_WINDOW],debug_counts[MAINSCREEN_COLOUR_WINDOW]);//subscreen colour window handling
	//pgPrintBG(0,16,0xffff,szBuf);

	//sprintf(szBuf,"FIX:%d,%d,%d,%d,%d",debug_counts[FIXCOLORCOUNT_ON_NO_COLOUR_WINDOW],debug_counts[FIXCOLORCOUNT_ON_COLOUR_WINDOW],
	//	debug_counts[44],debug_counts[45],debug_counts[46]);
	//pgPrintBG(0,18,0xffff,szBuf);
	//sprintf(szBuf,"COL:%d,%d,%d",debug_counts[MAINCOLORCOUNT_NO_CLIP],debug_counts[MAINCOLORCOUNT_CLIP],debug_counts[PALETTE_CAHNGE_COUNT]);
	//pgPrintBG(0,19,0xffff,szBuf);
	//sprintf(szBuf,"TRANS:%d,%d",debug_counts[TRANCE_COUNT],debug_counts[NO_TRANCE_COUNT]);
	//pgPrintBG(0,20,0xffff,szBuf);
	
	//sprintf(szBuf,"STRC:%d,%d",Settings.SRTC,Memory.HiROM);
	//pgPrintBG(0,18,0xffff,szBuf);
	  
	//
	//
	for(int i=0;i<100;i++)debug_counts[i]=0;
	//	
	//sprintf(szBuf,"%d",APURegistersUncached.PC);
	//pgPrintBG(0,4,0xffff,szBuf);	
	//sprintf(szBuf,"M%d",apu_glob_cycles_Main>>5);
	//pgPrintBG(0,5,0xffff,szBuf);	
	//sprintf(szBuf,"A%d",apu_event1_cpt1>>5);
	//pgPrintBG(0,6,0xffff,szBuf);	
	
}
void test(int a,int b){}
