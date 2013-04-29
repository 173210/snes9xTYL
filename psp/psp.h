#ifndef __PSP_H__
#define __PSP_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern int os9x_apu_ratio;

#define MESSAGE_DAYS 1  //show boot message every ... days


#define RELEASE

#ifdef RELEASE
#define info(a,b,c) 
#else
#define PROFILE
#define info(a,b,c)  pgPrintBG(a,b,(31<<10)|(31<<5)|31,c);
#endif

#define NET_UPDATE_INTERVAL 2/*2*/  //relative to call to syncspeed
#define OFFLINE_UPDATE_INTERVAL 0
#define NET_DELAY 3   //min is 1.  stands for delay between reading an input & using it
#define NET_PKT_LEN 32
#define NET_SYNCLOST_TS 64

#define RECV_DELAY 200  //in microseconds => 0.2ms
#define RECV_MAX_RETRY 10000   // => 2s
#define NET_MAGIC1 0x12
#define NET_MAGIC2 0x34
#define NET_MAGIC3 0x56

#define INFO_FLUSH_REDRAW(a) //debug_log(a)

#define PPU_IGNORE_FIXEDCOLCHANGES (1<<0)
#define PPU_IGNORE_WINDOW					 (1<<1)
#define PPU_IGNORE_ADDSUB					 (1<<2)
#define PPU_IGNORE_PALWRITE				     (1<<3)
#define GFX_FASTMODE7						 (1<<4)  //not used anymore, here for compatibility
#define APU_FIX								 (1<<5)  //not used anymore, here for compatibility
#define HIRES_FIX							 (1<<6)
#define PPU_SIMPLE_PALWRITE				     (1<<6)
#define OLD_PSP_ACCEL                        (1<<7)
extern int os9x_hack;

#ifdef ME_SOUND
#define UNCACHE_PTR(a) ( ((int)(a))|0x40000000 )
#define NORMAL_PTR(a) ( ((int)(a))&0xBFFFFFFF )
#else
#define UNCACHE_PTR(a) (a)
#define NORMAL_PTR(a) (a)
#endif
#define UNCACHE_PTR2(a) ( ((int)(a))|0x40000000 )
#define NORMAL_PTR2(a) ( ((int)(a))&0xBFFFFFFF )

#define VERSION_MAJOR 0
#define VERSION_MINOR 4

#define VERSION_MAJOR_COMP 0
#define VERSION_MINOR_COMP 4

#define PSP_UP				0
#define PSP_DOWN			1
#define PSP_LEFT			2
#define PSP_RIGHT			3
#define PSP_TRIANGLE	4
#define PSP_CIRCLE		5
#define PSP_CROSS			6
#define PSP_SQUARE		7
#define PSP_START			8
#define PSP_SELECT		9
#define PSP_TL				10
#define PSP_TR				11
#define PSP_AUP				12
#define PSP_ADOWN			13
#define PSP_ALEFT			14
#define PSP_ARIGHT		15

#define PSP_TL_TR			16
#define PSP_TL_START	17
#define PSP_TR_START	18
#define PSP_TL_SELECT	19
#define PSP_TR_SELECT	20
#define PSP_SELECT_START	21

#define PSP_BUTTONS_TOTAL	22

//#define PSP_NOTE			...

#define OS9X_FRAMESKIP_UP		0x00010000
#define OS9X_FRAMESKIP_DOWN	0x00020000
#define OS9X_MENUACCESS			0x00040000
#define OS9X_TURBO					0x00080000
#define OS9X_GFXENGINE			0x00100000
#define OS9X_SAVE_STATE			0x00200000
#define OS9X_LOAD_STATE			0x00400000


#define MAXVOLUME	0x8000

#include <pspuser.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <psppower.h>
#include <pspgu.h>
#include <psputility.h>

#include <pspusb.h>
#include <pspusbstor.h>

#include <pspiofilemgr.h>
#include <pspiofilemgr_fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define bool int
#define false 0
#define true 1


#include "pg.h"
#include "psp_ui.h"
#include "psplib.h"
#include "profiler.h"
#include "psp/ram.h"
#include "psp/me/mem64.h"

#include "psp/psp_msg.h"

void os9x_beep1();
void os9x_beep2();

extern unsigned int __attribute__((aligned(64))) list[262144*4];

extern volatile int *mixsample_flag,*emulapu_flag;

#define VRAM_ADDR	(0x04000000)

#define SCREEN_WIDTH	480
#define SCREEN_HEIGHT	272

#define	PIXELSIZE	1				//in short
#define	LINESIZE	512				//in short
#define	FRAMESIZE	0x44000			//in byte





#define SJIS_SQUARE "Å†"
#define SJIS_CROSS "Å~"
#define SJIS_CIRCLE "Åõ"
#define SJIS_TRIANGLE "Å¢"
#define SJIS_RIGHT "Å®"
#define SJIS_LEFT "Å©"
#define SJIS_UP "Å™"
#define SJIS_DOWN "Å´"
#define SJIS_STAR "Åö"


#define printf pspDebugScreenPrintf


#define ALL_PAD_BUTTONS (PSP_CTRL_UP|PSP_CTRL_DOWN|PSP_CTRL_LEFT|PSP_CTRL_RIGHT|PSP_CTRL_CIRCLE|PSP_CTRL_SQUARE|PSP_CTRL_CROSS|PSP_CTRL_TRIANGLE|PSP_CTRL_START|PSP_CTRL_SELECT|PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER/*|PSP_CTRL_NOTE*/)

void debug_log( const char* message );

void psp_showProgressBar(int,int);

void StopSoundThread();
void InitSoundThread();
void FileLog(char* msg);

typedef struct {
	unsigned char *buffer[4];
	int buffer_ready;	
	int buffer_idx;
	int sample_count;
	int freqratio;
	int freqerr;
	int *os9x_apuenabled_ptr;
	int *os9x_paused_ptr;
	int exit;
	
	// Me Mode7 Stuff.
	int mode7LineFlag[512]; // Support up to 512 lines.
    int enableMode7;
    int MECanPaint;
    int MEStillPaint;
	int Mode7Type;
	// Current block info.
	unsigned char * screen;
	int bg;
	int prio;
	int pitch;
	int StartY;
	int EndY;
} me_s9xpsp_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PSP_H__ */
