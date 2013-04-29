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
#ifndef _SPC700_H_
#define _SPC700_H_

#define Carry       1
#define Zero        2
#define Interrupt   4
#define HalfCarry   8
#define BreakFlag  16
#define DirectPageFlag 32
#define Overflow   64
#define Negative  128

#define APUClearCarry() (((IAPU._Carry)) = 0)
#define APUSetCarry() (((IAPU._Carry)) = 1)
#define APUSetInterrupt() ((APURegisters.P) |= Interrupt)
#define APUClearInterrupt() ((APURegisters.P) &= ~Interrupt)
#define APUSetHalfCarry() ((APURegisters.P) |= HalfCarry)
#define APUClearHalfCarry() ((APURegisters.P) &= ~HalfCarry)
#define APUSetBreak() ((APURegisters.P) |= BreakFlag)
#define APUClearBreak() ((APURegisters.P) &= ~BreakFlag)
#define APUSetDirectPage() ((APURegisters.P) |= DirectPageFlag)
#define APUClearDirectPage() ((APURegisters.P) &= ~DirectPageFlag)
#define APUSetOverflow() (((IAPU._Overflow)) = 1)
#define APUClearOverflow() (((IAPU._Overflow)) = 0)

#define APUCheckZero() (((IAPU._Zero)) == 0)
#define APUCheckCarry() (((IAPU._Carry)))
#define APUCheckInterrupt() ((APURegisters.P) & Interrupt)
#define APUCheckHalfCarry() ((APURegisters.P) & HalfCarry)
#define APUCheckBreak() ((APURegisters.P) & BreakFlag)
#define APUCheckDirectPage() ((APURegisters.P) & DirectPageFlag)
#define APUCheckOverflow() (((IAPU._Overflow)))
#define APUCheckNegative() (((IAPU._Zero)) & 0x80)

#define APUClearFlags(f) ((APURegisters.P) &= ~(f))
#define APUSetFlags(f)   ((APURegisters.P) |=  (f))
#define APUCheckFlag(f)  ((APURegisters.P) &   (f))

typedef union
{
#ifdef LSB_FIRST
    struct { uint8 A, Y; } B;
#else
    struct { uint8 Y, A; } B;
#endif
    uint16 W;
} YAndA;

struct SAPURegisters{
    uint8  P;
    uint8  X;
    uint8  S;
	uint8 sp;
    YAndA YA;
    uint16  PC;
};

// Needed by ILLUSION OF GAIA
//#define ONE_APU_CYCLE 14
#define ONE_APU_CYCLE 21

// Needed by all games written by the software company called Human
//#define ONE_APU_CYCLE_HUMAN 17
#define ONE_APU_CYCLE_HUMAN 21

// 1.953us := 1.024065.54MHz

#define APU_SETAPURAMruka() {\
  int apu_ram_write_cpt1_,apu_ram_write_cpt2_; \
  apu_ram_write_cpt1_=apu_ram_write_cpt1;\
  apu_ram_write_cpt2_=apu_ram_write_cpt2;\
  if (apu_ram_write_cpt1_<apu_ram_write_cpt2_) { \
    int cpt;\
    for (cpt=apu_ram_write_cpt1_;cpt<apu_ram_write_cpt2_;cpt++) {\
        unsigned short dwValue = apu_ram_write_log[cpt & 0xFFFF];\
        IAPU.RAM[((dwValue>>8) & 3) | 0xF4] = dwValue & 0xFF;\
    }\
    apu_ram_write_cpt1=apu_ram_write_cpt2_;\
  }\
}
//me
#define APU_EXECUTE3() { \
int apu_target_cycles_=apu_glob_cycles;\
int ac=Uncache_APU_Cycles;\
if ((ac)<=apu_target_cycles_) {\
  int apu_event1_cpt1_;\
  int apu_event1_cpt2_;\
  int apu_ram_write_cpt1_,apu_ram_write_cpt2_; \
  APUPack.APU.Cycles=ac; \
  apu_event1_cpt1_=APUPack.apu_event1_cpt1_me;\
  apu_event1_cpt2_=apu_event1_cpt2;\
  apu_ram_write_cpt1_=APUPack.apu_ram_write_cpt1_me;\
  apu_ram_write_cpt2_=apu_ram_write_cpt2;\
	if (apu_ram_write_cpt1_<apu_ram_write_cpt2_) {\
  		int cpt;\
  		for (cpt=apu_ram_write_cpt1_;cpt<apu_ram_write_cpt2_;cpt++){\
			unsigned short dwValue = apu_ram_write_log[cpt & 0xFFFF];\
			(IAPU.RAM)[(dwValue>>8)|0xF4]=dwValue &0xFF;\
		}\
		APUPack.apu_ram_write_cpt1_me=apu_ram_write_cpt2_;\
	}\
	while (APUPack.APU.Cycles<=apu_target_cycles_) {\
		(APUPack.APU.Cycles)+=S9xAPUCycles [*(IAPU.PC)];\
		(*S9xApuOpcodes[*(IAPU.PC)]) (); \
		while (apu_event1_cpt1_<apu_event1_cpt2_) {\
		    int32 EventVal = apu_event1[apu_event1_cpt1_&APU_EVENT_MASK];\
			int32 V_Counter = EventVal & 0x80000000;\
			EventVal &= 0x7FFFFFFF;\
			if (APUPack.APU.Cycles>=EventVal) {\
				apu_event1_cpt1_++;\
				if ((APUPack.APU.TimerEnabled) [2]) {\
					(APUPack.APU.Timer) [2] += 4;\
					while (APUPack.APU.Timer[2] >= APUPack.APU.TimerTarget[2]) {\
		  			IAPU.RAM[0xff] = (IAPU.RAM[0xff] + 1) & 0xf;\
		  			APUPack.APU.Timer[2] -= APUPack.APU.TimerTarget[2];\
					}\
				}\
				if (V_Counter) {\
					if (APUPack.APU.TimerEnabled[0]) {\
		  			APUPack.APU.Timer[0]++;\
		  			if (APUPack.APU.Timer[0] >= APUPack.APU.TimerTarget[0]) {\
							IAPU.RAM[0xfd] = (IAPU.RAM[0xfd] + 1) & 0xf;\
							APUPack.APU.Timer[0] = 0;\
					  }\
					}\
					if (APUPack.APU.TimerEnabled[1]) {\
		  			APUPack.APU.Timer[1]++;\
		  			if (APUPack.APU.Timer[1] >= APUPack.APU.TimerTarget[1]) {\
							IAPU.RAM[0xfe] = (IAPU.RAM[0xfe] + 1) & 0xf;\
							APUPack.APU.Timer[1] = 0;\
			  		}\
					}\
				}\
			} else break;\
		}\
}\
*((int*)(Uncache_APU_OutPorts))=*((int*)(APUPack.APU.OutPorts)); \
(apu_event1_cpt1)=APUPack.apu_event1_cpt1_me=apu_event1_cpt1_;\
Uncache_APU_Cycles=APUPack.APU.Cycles; \
} \
}

#if 0
#define APU_EXECUTE3() { \
int apu_target_cycles_=apu_glob_cycles;\
if ((Uncache_APU_Cycles)<=apu_target_cycles_) {\
  int apu_event1_cpt1_;\
  int apu_event1_cpt2_;\
  int apu_event2_cpt1_;\
  int apu_event2_cpt2_;\
  int apu_ram_write_cpt1_,apu_ram_write_cpt2_; \
  APUPack.APU.Cycles=Uncache_APU_Cycles; \
  apu_event1_cpt1_=apu_event1_cpt1;\
  apu_event2_cpt1_=apu_event2_cpt1;\
  apu_event1_cpt2_=apu_event1_cpt2;\
  apu_event2_cpt2_=apu_event2_cpt2;\
  apu_ram_write_cpt1_=apu_ram_write_cpt1;\
  apu_ram_write_cpt2_=apu_ram_write_cpt2;\
	if (apu_ram_write_cpt1_<apu_ram_write_cpt2_) { \
  		int cpt;\
  		for (cpt=apu_ram_write_cpt1_;cpt<apu_ram_write_cpt2_;cpt++)(IAPU.RAM)[(apu_ram_write_log[cpt&0xFFFF]>>8)|0xF4]=apu_ram_write_log[cpt&0xFFFF]&0xFF;	\
		(apu_ram_write_cpt1)=apu_ram_write_cpt2_;\
	}\
	while (APUPack.APU.Cycles<=apu_target_cycles_) {\
		(APUPack.APU.Cycles)+=S9xAPUCycles [*(IAPU.PC)];\
		(*S9xApuOpcodes[*(IAPU.PC)]) (); \
		while (apu_event1_cpt1_<apu_event1_cpt2_) {\
			if (APUPack.APU.Cycles>=apu_event1[apu_event1_cpt1_&APU_EVENT_MASK]) {\
				apu_event1_cpt1_++;\
				if (((APUPack.APU.TimerEnabled)) [2]) {\
					((APUPack.APU.Timer)) [2] += 4;\
					while (((APUPack.APU.Timer)) [2] >= ((APUPack.APU.TimerTarget)) [2]) {\
		  			((IAPU.RAM)) [0xff] = (((IAPU.RAM)) [0xff] + 1) & 0xf;\
		  			((APUPack.APU.Timer)) [2] -= ((APUPack.APU.TimerTarget)) [2];\
					}\
				}\
			} else break;\
		}\
		while (apu_event2_cpt1_<apu_event2_cpt2_) {\
			if (APUPack.APU.Cycles>=apu_event2[apu_event2_cpt1_&APU_EVENT_MASK]) {\
				apu_event2_cpt1_++;\
				if (((APUPack.APU.TimerEnabled)) [0]) {\
		  		((APUPack.APU.Timer)) [0]++;\
		  		if (((APUPack.APU.Timer)) [0] >= ((APUPack.APU.TimerTarget)) [0]) {\
						((IAPU.RAM)) [0xfd] = (((IAPU.RAM)) [0xfd] + 1) & 0xf;\
						((APUPack.APU.Timer)) [0] = 0;\
				  }\
				}\
				if (((APUPack.APU.TimerEnabled)) [1]) {\
		  		((APUPack.APU.Timer)) [1]++;\
		  		if (((APUPack.APU.Timer)) [1] >= ((APUPack.APU.TimerTarget)) [1]) {\
						((IAPU.RAM)) [0xfe] = (((IAPU.RAM)) [0xfe] + 1) & 0xf;\
						((APUPack.APU.Timer)) [1] = 0;\
			  	}\
				}\
			} else break;\
		}\
	}\
*((int*)(Uncache_APU_OutPorts))=*((int*)(APUPack.APU.OutPorts)); \
(apu_event1_cpt1)=apu_event1_cpt1_;\
(apu_event2_cpt1)=apu_event2_cpt1_;\
Uncache_APU_Cycles=APUPack.APU.Cycles; \
} \
}
#endif
#ifdef ME_SOUND
#define APU_EXECUTE2() FLUSH_APU();if (Uncache_APU_Cycles<=(apu_glob_cycles)) {\
int dummy=0;\
for (;;) {\
	if (Uncache_APU_Cycles>(apu_glob_cycles)) break; \
	dummy=rand()+dummy;\
}\
apu_glob_cycles=apu_glob_cycles_Main-=cpu_glob_cycles;\
Uncache_APU_Cycles-=cpu_glob_cycles;\
cpu_glob_cycles=0;\
	{\
	int i,j;\
		if ((i=apu_event1_cpt1)<(j=apu_event1_cpt2)) { \
			for (;i<j;i++) apu_event1[i&APU_EVENT_MASK]=0;\
		}\
	}\
}
#if 0
#define APU_EXECUTE2() FLUSH_APU();if (Uncache_APU_Cycles<=(apu_glob_cycles)) {\
int dummy=0;\
for (;;) {\
	if (Uncache_APU_Cycles>(apu_glob_cycles)) break; \
	dummy=rand()+dummy;\
}\
apu_glob_cycles_Main-=cpu_glob_cycles;\
Uncache_APU_Cycles-=cpu_glob_cycles;\
cpu_glob_cycles=0;\
if (apu_event1_cpt1<apu_event1_cpt2) { \
	int i,j;\
	j=apu_event1_cpt2;\
	for (i=apu_event1_cpt1;i<j;i++) apu_event1[i&APU_EVENT_MASK]=0;\
}\
if (apu_event2_cpt1<apu_event2_cpt2) { \
	int i,j;\
	j=apu_event2_cpt2;\
	for (i=apu_event2_cpt1;i<j;i++) apu_event2[i&APU_EVENT_MASK]=0;\
}\
}
#endif
#else
#define APU_EXECUTE2() { \
if ((Uncache_APU_Cycles)<=apu_glob_cycles) {\
	int apu_target_cycles_=apu_glob_cycles;\
  int apu_event1_cpt1_,apu_event2_cpt1_;\
  int apu_event1_cpt2_,apu_event2_cpt2_;\
  int apu_ram_write_cpt1_,apu_ram_write_cpt2_; \
  APUPack.APU.Cycles=Uncache_APU_Cycles; \
  apu_event1_cpt1_=apu_event1_cpt1;\
  apu_event2_cpt1_=apu_event2_cpt1;\
  apu_event1_cpt2_=apu_event1_cpt2;\
  apu_event2_cpt2_=apu_event2_cpt2;\
  apu_ram_write_cpt1_=apu_ram_write_cpt1;\
  apu_ram_write_cpt2_=apu_ram_write_cpt2;\
  if (apu_ram_write_cpt1_<apu_ram_write_cpt2_) { \
  	int cpt;\
  	for (cpt=apu_ram_write_cpt1_;cpt<apu_ram_write_cpt2_;cpt++) (IAPU.RAM)[(apu_ram_write_log[cpt&APU_EVENT_MASK]>>8)|0xF4]=apu_ram_write_log[cpt&APU_EVENT_MASK]&0xFF;	\
		(apu_ram_write_cpt1)=apu_ram_write_cpt2_;\
	}  \
	while (APUPack.APU.Cycles<=apu_target_cycles_) {\
		(APUPack.APU.Cycles)+=S9xAPUCycles [*(IAPU.PC)];\
		(*S9xApuOpcodes[*(IAPU.PC)]) (); \
		while (apu_event1_cpt1_<apu_event1_cpt2_) {\
			int32 EventVal = apu_event1[apu_event1_cpt1_&APU_EVENT_MASK];\
			int32 V_Counter = EventVal & 0x80000000;\
			EventVal &= 0x7FFFFFFF;\
			if (APUPack.APU.Cycles>=EventVal) {\
				apu_event1_cpt1_++;\
				if (((APUPack.APU.TimerEnabled)) [2]) {\
					((APUPack.APU.Timer)) [2] += 4;\
					while (((APUPack.APU.Timer)) [2] >= ((APUPack.APU.TimerTarget)) [2]) {\
		  			((IAPU.RAM)) [0xff] = (((IAPU.RAM)) [0xff] + 1) & 0xf;\
		  			((APUPack.APU.Timer)) [2] -= ((APUPack.APU.TimerTarget)) [2];\
					}\
				}\
				if (V_Counter) {\
					if (APUPack.APU.TimerEnabled[0]) {\
		  			APUPack.APU.Timer[0]++;\
		  			if (APUPack.APU.Timer[0] >= APUPack.APU.TimerTarget[0]) {\
							IAPU.RAM[0xfd] = (IAPU.RAM[0xfd] + 1) & 0xf;\
							APUPack.APU.Timer[0] = 0;\
					  }\
					}\
					if (APUPack.APU.TimerEnabled[1]) {\
		  			APUPack.APU.Timer[1]++;\
		  			if (APUPack.APU.Timer[1] >= APUPack.APU.TimerTarget[1]) {\
							IAPU.RAM[0xfe] = (IAPU.RAM[0xfe] + 1) & 0xf;\
							APUPack.APU.Timer[1] = 0;\
			  		}\
					}\
				}\
			} else break;\
		}\
}\
*((int*)(APUPack.APU.OutPorts))=*((int*)(APUPack.APU.OutPorts)); \
(apu_event1_cpt1)=apu_event1_cpt1_;\
 APUPack.APU.Cycles=APUPack.APU.Cycles; \
apu_glob_cycles-=cpu_glob_cycles;\
Uncache_APU_Cycles-=cpu_glob_cycles;\
cpu_glob_cycles=0;\
if (apu_event1_cpt1<apu_event1_cpt2) { \
	int i,j;\
	j=apu_event1_cpt2;\
	for (i=apu_event1_cpt1;i<j;i++) apu_event1[i&APU_EVENT_MASK]=0;\
}\
} \
}
#endif

#endif
