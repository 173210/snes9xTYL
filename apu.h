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

#ifndef _apu_h_
#define _apu_h_

#include "spc700.h"

//extern int old_cpu_cycles;
#define APU_EVENT_SIZE (65536)
#define APU_EVENT_MASK (APU_EVENT_SIZE-1)
typedef struct {

	int apu_cycles_left;
#ifdef ME_SOUND 
	
#endif
	int apu_glob_cycles;
	int apu_event1_cpt1;
	int apu_event2_cpt1;
	int apu_event1_cpt2;
	int apu_event2_cpt2;
	int apu_init_after_load;
	int apu_can_execute;
	int apu_ram_write_cpt1;
	int apu_ram_write_cpt2;
	volatile int apu_event1[APU_EVENT_SIZE];//ng
	int apu_event2[APU_EVENT_SIZE];
	unsigned short apu_ram_write_log[APU_EVENT_SIZE];//ok
#ifdef ME_SOUND 
	int32 APU_Cycles;							// me:rw main:rw <-danger? TBD
    uint8 APU_OutPorts[4];						// me:w main:r
#endif
	bool8 APUExecuting;					// me:rw main:w(DMA)
}SAPUEVENTS;
typedef struct {
	int32 APU_Cycles;							// me:rw main:rw <-danger? TBD
    uint8 APU_OutPorts[4];						// me:w main:r
	bool8 APUExecuting;					// me:rw main:w(DMA)
}SAPUEVENTS2;
extern SAPUEVENTS stAPUEvents;
#ifndef ME_SOUND
//#define old_cpu_cycles stAPUEvents.old_cpu_cycles
#define apu_cycles_left stAPUEvents.apu_cycles_left
#define apu_glob_cycles stAPUEvents.apu_glob_cycles
#define apu_event1 stAPUEvents.apu_event1
#define apu_event2 stAPUEvents.apu_event2
#define apu_event1_cpt1 stAPUEvents.apu_event1_cpt1
#define apu_event2_cpt1 stAPUEvents.apu_event2_cpt1
#define apu_event1_cpt2 stAPUEvents.apu_event1_cpt2
#define apu_event2_cpt2 stAPUEvents.apu_event2_cpt2
#define apu_init_after_load stAPUEvents.apu_init_after_load
#define apu_can_execute stAPUEvents.apu_can_execute
#define apu_ram_write_cpt1 stAPUEvents.apu_ram_write_cpt1
#define apu_ram_write_cpt2 stAPUEvents.apu_ram_write_cpt2
#define apu_ram_write_log stAPUEvents.apu_ram_write_log
#else
//extern int old_cpu_cycles;
/*
typedef struct {
	int apu_glob_cycles;						// me:r main:rw
	int	apu_event1_cpt1;						// me:rw main:r
	int	apu_event1_cpt2;						// me:r main:rw
//	int	apu_event2_cpt1;						// me:rw main:r
//	int	apu_event2_cpt2;						// me:r main:rw
	int32 APU_Cycles;							// me:rw main:rw <-danger? TBD
	bool8 IAPU_APUExecuting;					// me:rw main:w(DMA)
	int	apu_ram_write_cpt1;						// me:rw main:r
	int	apu_ram_write_cpt2;						// me:r main:rw
    uint8 APU_OutPorts[4];						// me:w main:r
	uint32 dwDeadlockTestMain;					// main:rw
	uint32 dwDeadlockTestMe;					// me:w main:r
	uint32 adwParam[4];							// me:w main:r
//	uint32 dwDummy[1];							// me: main:
	int apu_event1[0xFFFF];						// me:r main:w
//	int apu_event2[0xFFFF];						// me:r main:w
	unsigned short apu_ram_write_log[0xFFFF];	// me:r main:w 
}SAPUEVENTS;
extern SAPUEVENTS stAPUEvents;
*/

extern volatile int *apu_cycles_left_p,*apu_glob_cycles_p;
extern volatile int *apu_event1,*apu_event2,*apu_event1_cpt1_p,*apu_event2_cpt1_p,*apu_event1_cpt2_p,*apu_event2_cpt2_p;
extern volatile unsigned short *apu_ram_write_log;
extern volatile int *apu_init_after_load_,*apu_can_execute_p,*apu_ram_write_cpt1_p,*apu_ram_write_cpt2_p;
extern int apu_ram_write_cpt2_main;
extern int apu_event1_cpt2_main;
#if 1
#define apu_cycles_left (*apu_cycles_left_p)
#define apu_glob_cycles (*apu_glob_cycles_p)
#define apu_event1_cpt1 (*apu_event1_cpt1_p)
#define apu_event2_cpt1 (*apu_event2_cpt1_p)
#define apu_event1_cpt2 (*apu_event1_cpt2_p)
#define apu_event2_cpt2 (*apu_event2_cpt2_p)
#define apu_init_after_load (*apu_init_after_load_)
#define apu_can_execute (*apu_can_execute_p)
#define apu_ram_write_cpt1 (*apu_ram_write_cpt1_p)
#define apu_ram_write_cpt2 (*apu_ram_write_cpt2_p)
#elif 0
#define apu_cycles_left (((SAPUEVENTS*)UNCACHE_PTR(&stAPUEvents))->apu_cycles_left)
#define apu_glob_cycles (((SAPUEVENTS*)UNCACHE_PTR(&stAPUEvents))->apu_glob_cycles)
#define apu_event1_cpt1 (((SAPUEVENTS*)UNCACHE_PTR(&stAPUEvents))->apu_event1_cpt1)
#define apu_event2_cpt1 (((SAPUEVENTS*)UNCACHE_PTR(&stAPUEvents))->apu_event2_cpt1)
#define apu_event1_cpt2 (((SAPUEVENTS*)UNCACHE_PTR(&stAPUEvents))->apu_event1_cpt2)
#define apu_event2_cpt2 (((SAPUEVENTS*)UNCACHE_PTR(&stAPUEvents))->apu_event2_cpt2)
#define apu_init_after_load (((SAPUEVENTS*)UNCACHE_PTR(&stAPUEvents))->apu_init_after_load)
#define apu_can_execute (((SAPUEVENTS*)UNCACHE_PTR(&stAPUEvents))->apu_can_execute)
#define apu_ram_write_cpt1 (((SAPUEVENTS*)UNCACHE_PTR(&stAPUEvents))->apu_ram_write_cpt1)
#define apu_ram_write_cpt2 (((SAPUEVENTS*)UNCACHE_PTR(&stAPUEvents))->apu_ram_write_cpt2)
#else
#define apu_cycles_left (stAPUEvents_p->apu_cycles_left)
#define apu_glob_cycles (stAPUEvents_p->apu_glob_cycles)
#define apu_event1_cpt1 (stAPUEvents_p->apu_event1_cpt1)
#define apu_event2_cpt1 (stAPUEvents_p->apu_event2_cpt1)
#define apu_event1_cpt2 (stAPUEvents_p->apu_event1_cpt2)
#define apu_event2_cpt2 (stAPUEvents_p->apu_event2_cpt2)
#define apu_init_after_load (stAPUEvents_p->apu_init_after_load)
#define apu_can_execute (stAPUEvents_p->apu_can_execute)
#define apu_ram_write_cpt1 (stAPUEvents_p->apu_ram_write_cpt1)
#define apu_ram_write_cpt2 (stAPUEvents_p->apu_ram_write_cpt2)
#endif

#endif

#ifdef ME_SOUND
extern SAPUEVENTS  *  const stAPUEvents_p;
//extern volatile int32 *  const Uncache_APU_Cycles_p;
//extern uint8 * const Uncache_APU_OutPorts_p;
//extern volatile bool8 *  const Uncache_IAPU_APUExecuting_p;
//1　単体
//#define Uncache_APU_Cycles (*Uncache_APU_Cycles_p)
//#define Uncache_APU_OutPorts Uncache_APU_OutPorts_p
//#define IAPU_APUExecuting (*Uncache_IAPU_APUExecuting_p)
//2　構造体
//#define Uncache_APU_Cycles (((SAPUEVENTS*)UNCACHE_PTR(&stAPUEvents))->APU_Cycles)
//#define Uncache_APU_OutPorts (((SAPUEVENTS*)UNCACHE_PTR(&stAPUEvents))->APU_OutPorts)
//#define IAPU_APUExecuting (((SAPUEVENTS*)UNCACHE_PTR(&stAPUEvents))->APUExecuting)
//2.5　構造体
#define Uncache_APU_Cycles (stAPUEvents_p->APU_Cycles)
#define Uncache_APU_OutPorts (stAPUEvents_p->APU_OutPorts)
#define IAPU_APUExecuting (stAPUEvents_p->APUExecuting)
//3 スクラッチパッドできない
//#define Uncache_APU_Cycles (*(int32*)UNCACHE_PTR(0x10400))
//#define Uncache_APU_OutPorts ((uint8*)UNCACHE_PTR(0x10404))
//#define IAPU_APUExecuting (*(int32*)UNCACHE_PTR(0x10408))
//毎回で無く、一定期間ごとにIAPU_APUExecuting更新する
#if 1
#define FAST_IAPU_APUEXECUTING_CHECK
#endif
//#define IAPU_APUExecuting IAPUuncached.APUExecuting //不具合の可能性あり
#else
#define Uncache_APU_Cycles APUPack.APU.Cycles
#define Uncache_APU_OutPorts APUPack.APU.OutPorts
#define IAPU_APUExecuting IAPUuncached.APUExecuting
#define IAPU_APUExecuting_Main IAPU_APUExecuting
#endif

#ifdef ME_SOUND
#ifndef FAST_IAPU_APUEXECUTING_CHECK
#define IAPU_APUExecuting_Main IAPU_APUExecuting
#define UPDATE_APU_COUNTER() {\
		if (IAPU_APUExecuting_Main) {\
			if (CPUPack.CPU.Cycles-old_cpu_cycles<0) msgBoxLines("1",60);\
			else cpu_glob_cycles += CPUPack.CPU.Cycles-old_cpu_cycles;\
			old_cpu_cycles=CPUPack.CPU.Cycles;\
			apu_glob_cycles=cpu_glob_cycles;\
			if (cpu_glob_cycles>=0x00700000) {\
				APU_EXECUTE2 ();\
			}\
		}\
}
#define RESET_APU_COUNTER()
#define FLUSH_APU()
#define apu_glob_cycles_Main apu_glob_cycles
#else
struct SIAPU_APUExecuting
{
	uint8 _APUExecuting_Main_Counter;	
	bool8 _IAPU_APUExecuting_Main;
	uint8 sp[2];
	int _apu_glob_cycles_Main;
};
//extern bool8 IAPU_APUExecuting_Main;
//extern uint8 APUExecuting_Main_Counter;
//extern int apu_glob_cycles_Main;
extern struct SIAPU_APUExecuting _IAPU_APUExecuting;
#define IAPU_APUExecuting_Main CPUPack._IAPU_APUExecuting_Main
#define APUExecuting_Main_Counter CPUPack._APUExecuting_Main_Counter
#define apu_glob_cycles_Main CPUPack._apu_glob_cycles_Main

#define RESET_APU_COUNTER() {APUExecuting_Main_Counter=0;}
#define FLUSH_APU() {apu_glob_cycles=apu_glob_cycles_Main; APUExecuting_Main_Counter=0;}
#define UPDATE_APU_COUNTER() {\
	if (IAPU_APUExecuting_Main==false || APUExecuting_Main_Counter==0){\
		IAPU_APUExecuting_Main = IAPU_APUExecuting;\
	}\
	if (IAPU_APUExecuting_Main){\
		cpu_glob_cycles += CPUPack.CPU.Cycles-old_cpu_cycles;\
		old_cpu_cycles=CPUPack.CPU.Cycles;\
		apu_glob_cycles_Main=cpu_glob_cycles;\
		if (cpu_glob_cycles>=0x00700000) {\
			APU_EXECUTE2();\
		}\
	}\
	if (APUExecuting_Main_Counter==0){\
		FLUSH_APU();\
	}\
	APUExecuting_Main_Counter=0;\
}
#undef UPDATE_APU_COUNTER
#define UPDATE_APU_COUNTER() {\
	if (IAPU_APUExecuting_Main){\
		cpu_glob_cycles += CPUPack.CPU.Cycles-old_cpu_cycles;\
		old_cpu_cycles=CPUPack.CPU.Cycles;\
		apu_glob_cycles_Main=cpu_glob_cycles;\
		if (cpu_glob_cycles>=0x00700000) {\
			APU_EXECUTE2();\
		}\
	}\
	if (APUExecuting_Main_Counter==0){\
		FLUSH_APU();\
	}\
	APUExecuting_Main_Counter++;\
}
#endif
#else
//no Me_sound
#define UPDATE_APU_COUNTER() {\
		if (IAPU_APUExecuting_Main) {\
			if (CPUPack.CPU.Cycles-old_cpu_cycles<0) msgBoxLines("1",60);\
			else cpu_glob_cycles += CPUPack.CPU.Cycles-old_cpu_cycles;\
			old_cpu_cycles=CPUPack.CPU.Cycles;\
			apu_glob_cycles=cpu_glob_cycles;\
			if (cpu_glob_cycles>=0x00700000) {\
				APU_EXECUTE2 ();\
			}\
		}\
}
#define RESET_APU_COUNTER()
#define FLUSH_APU()
#define apu_glob_cycles_Main apu_glob_cycles
#endif

struct SIAPU
{
    uint8  *PC;
    uint8  *RAM;
    uint8  *DirectPage;

	bool8  APUExecuting;
	uint8  sp[3];

    uint32 Address;
    uint8  *WaitAddress1;
    uint8  *WaitAddress2;
    uint32 WaitCounter;
//    uint8  *ShadowRAM;
//    uint8  *CachedSamples;
    uint8  Bit;
    uint8  _Carry;
    uint8  _Zero;
    uint8  _Overflow;

    uint32 TimerErrorCounter;
    int32  NextAPUTimerPos;
    int32  APUTimerCounter;
    uint32 Scanline;
    int32  OneCycle;
    int32  TwoCycles;
};

struct SAPU
{
    int32  Cycles;

    bool8  ShowROM;
    uint8  Flags;
    uint8  KeyedChannels;
    uint8  dummy_for_align;

    uint8  OutPorts [4];

    uint8  DSP [0x80];
    uint8  ExtraRAM [64];

    uint16 Timer [3];
    uint16 TimerTarget [3];

    bool8  TimerEnabled [3];
    bool8  TimerValueWritten [3];
	uint8 sp[2];
};

struct SAPUPACK
{
	struct SAPURegisters APURegisters;	// 8bytes
	struct SIAPU IAPU;					// 60bytes
	struct SAPU APU;					// 228bytes
	int apu_event1_cpt1_me;
	int apu_ram_write_cpt1_me;
};

EXTERN_C  struct SAPUPACK APUPack;
//EXTERN_C  struct SAPU APU;
//EXTERN_C  struct SIAPU IAPU;
//EXTERN_C  struct SAPURegisters APURegisters;
//EXTERN_C  struct SAPU APUuncached;
EXTERN_C  struct SIAPU IAPUuncached;
//#define APURegistersUncached APURegisters
//#define IAPUuncached IAPU

#ifdef ME_SOUND
//extern SIAPU* Uncache_IAPU_p;
//#define IAPU (*Uncache_IAPU_p)
#define IAPU APUPack.IAPU

extern SIAPU* const Uncache_Uncache_IAPU_p;
//#define IAPUuncached (*Uncache_Uncache_IAPU_p)
#define IAPUuncached IAPU

EXTERN_C SAPU * const Uncache_Uncache_APU_p;
//#define APUuncached (*Uncache_Uncache_APU_p)
#define APUuncached APUPack.APU

EXTERN_C  SAPURegisters*   const Uncache_APURegisters_p;
//#define APURegistersUncached (*Uncache_APURegisters_p)
//EXTERN_C  struct SAPURegisters APURegistersUncached;
#define APURegistersUncached APURegisters
#else
#define IAPU APUPack.IAPU
#define APUuncached APUPack.APU
EXTERN_C  struct SAPURegisters APURegistersUncached;
#endif
#define APURegisters APUPack.APURegisters

static inline void S9xAPUUnpackStatus()
{
    ((IAPU._Zero)) = (((APURegisters.P) & Zero) == 0) | ((APURegisters.P) & Negative);
    ((IAPU._Carry)) = ((APURegisters.P) & Carry);
    ((IAPU._Overflow)) = ((APURegisters.P) & Overflow) >> 6;
}

STATIC inline void S9xAPUPackStatus()
{
    (APURegisters.P) &= ~(Zero | Negative | Carry | Overflow);
    (APURegisters.P) |= ((IAPU._Carry)) | ((((IAPU._Zero)) == 0) << 1) |
		      (((IAPU._Zero)) & 0x80) | (((IAPU._Overflow)) << 6);
}

static inline void S9xAPUUnpackStatusUncached()
{
    ((IAPUuncached._Zero)) = (((APURegistersUncached.P) & Zero) == 0) | ((APURegistersUncached.P) & Negative);
    ((IAPUuncached._Carry)) = ((APURegistersUncached.P) & Carry);
    ((IAPUuncached._Overflow)) = ((APURegistersUncached.P) & Overflow) >> 6;
}

STATIC inline void S9xAPUPackStatusUncached()
{
    (APURegistersUncached.P) &= ~(Zero | Negative | Carry | Overflow);
    (APURegistersUncached.P) |= ((IAPUuncached._Carry)) | ((((IAPUuncached._Zero)) == 0) << 1) |
		      (((IAPUuncached._Zero)) & 0x80) | (((IAPUuncached._Overflow)) << 6);
}

START_EXTERN_C
void S9xResetAPU (void);
bool8 S9xInitAPU ();
void S9xDeinitAPU ();
void S9xDecacheSamples ();
int S9xTraceAPU ();
int S9xAPUOPrint (char *buffer, uint16 Address);
void S9xSetAPUControl (uint8 byte);
void S9xSetAPUDSP (uint8 byte);
uint8 S9xGetAPUDSP ();
void S9xSetAPUTimer (uint16 Address, uint8 byte);

bool8 S9xInitSound (int quality, bool8 stereo, int buffer_size);
void S9xFreeSound ();
void S9xAllocSound ();
void S9xOpenCloseSoundTracingFile (bool8);
void S9xPrintAPUState ();
extern uint16 S9xAPUCycles [256];	// Scaled cycle lengths
extern uint8 S9xAPUCycleLengths [256];	// Raw data.
extern void (*S9xApuOpcodes [256]) (void);
void S9xSuspendSoundProcess(void);
void S9xResumeSoundProcess(void);
END_EXTERN_C


#define APU_VOL_LEFT 0x00
#define APU_VOL_RIGHT 0x01
#define APU_P_LOW 0x02
#define APU_P_HIGH 0x03
#define APU_SRCN 0x04
#define APU_ADSR1 0x05
#define APU_ADSR2 0x06
#define APU_GAIN 0x07
#define APU_ENVX 0x08
#define APU_OUTX 0x09

#define APU_MVOL_LEFT 0x0c
#define APU_MVOL_RIGHT 0x1c
#define APU_EVOL_LEFT 0x2c
#define APU_EVOL_RIGHT 0x3c
#define APU_KON 0x4c
#define APU_KOFF 0x5c
#define APU_FLG 0x6c
#define APU_ENDX 0x7c

#define APU_EFB 0x0d
#define APU_PMON 0x2d
#define APU_NON 0x3d
#define APU_EON 0x4d
#define APU_DIR 0x5d
#define APU_ESA 0x6d
#define APU_EDL 0x7d

#define APU_C0 0x0f
#define APU_C1 0x1f
#define APU_C2 0x2f
#define APU_C3 0x3f
#define APU_C4 0x4f
#define APU_C5 0x5f
#define APU_C6 0x6f
#define APU_C7 0x7f

#define APU_SOFT_RESET 0x80
#define APU_MUTE 0x40
#define APU_ECHO_DISABLED 0x20

#define FREQUENCY_MASK 0x3fff
START_EXTERN_C
extern uint8 APUI00a,APUI00b,APUI00c;
extern uint8 APUI01a,APUI01b,APUI01c;
extern uint8 APUI02a,APUI02b,APUI02c;
extern uint8 APUI03a,APUI03b,APUI03c;
END_EXTERN_C
#endif

