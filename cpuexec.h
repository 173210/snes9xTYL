/*
 * Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 *
 * (c) Copyright 1996 - 2001 Gary Henderson (gary.henderson@ntlworld.com) and
 *                           Jerremy Koot (jkoot@snes9x.com)
 *
 * Super FX C emulator code 
 * (c) Copyright 1997 - 1999 Ivar (ivar@snes9x.com) and
 *                           Gary Henderson.
 * Super FX assembler emulator code (c) Copyright 1998 zsKnight and _Demo_.
 *
 * DSP1 emulator code (c) Copyright 1998 Ivar, _Demo_ and Gary Henderson.
 * C4 asm and some C emulation code (c) Copyright 2000 zsKnight and _Demo_.
 * C4 C code (c) Copyright 2001 Gary Henderson (gary.henderson@ntlworld.com).
 *
 * DOS port code contains the works of other authors. See headers in
 * individual files.
 *
 * Snes9x homepage: http://www.snes9x.com
 *
 * Permission to use, copy, modify and distribute Snes9x in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Snes9x is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for Snes9x or software derived from Snes9x.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so everyone can benefit from the modifications
 * in future versions.
 *
 * Super NES and Super Nintendo Entertainment System are trademarks of
 * Nintendo Co., Limited and its subsidiary companies.
 */
#ifndef _CPUEXEC_H_
#define _CPUEXEC_H_
#include "ppu.h"
#include "memmap.h"
#include "65c816.h"

//extern int cpu_glob_cycles;

extern void (*S9x_Current_HBlank_Event)();
#define  S9xDoHBlankProcessing() (*S9x_Current_HBlank_Event)();
			
			

void S9xDoHBlankProcessing_HBLANK_START_EVENT();
void S9xDoHBlankProcessing_HBLANK_END_EVENT();
void S9xDoHBlankProcessing_HTIMER_BEFORE_EVENT();
void S9xDoHBlankProcessing_HTIMER_AFTER_EVENT();

#define DO_HBLANK_CHECK() \
    if (CPUPack.CPU.Cycles >= CPUPack.CPU.NextEvent) {\
			S9xDoHBlankProcessing ();\
		}
	
	
struct SOpcodes {
#ifdef __WIN32__
	void (__cdecl *S9xOpcode)( void);
#else
	void (*S9xOpcode)( void);
#endif
};
/* move to snes9x.h
struct SICPU
{
    uint8  *Speed;
    struct SOpcodes *S9xOpcodes;
    uint8  _Carry;
    uint8  _Zero;
    uint8  _Negative;
    uint8  _Overflow;
    bool8  CPUExecuting;
    uint32 ShiftedPB;
    uint32 ShiftedDB;
//    uint32 Frame;
//    uint32 Scanline;
#ifdef DEBUGGER
    uint32 FrameAdvanceCount;
#endif
};
*/

START_EXTERN_C
void S9xMainLoop (void);
void S9xReset (void);
//void S9xDoHBlankProcessing ();
void S9xClearIRQ (uint32);
void S9xSetIRQ (uint32);

extern struct SOpcodes S9xOpcodesM1X1 [256];
extern struct SOpcodes S9xOpcodesM1X0 [256];
extern struct SOpcodes S9xOpcodesM0X1 [256];
extern struct SOpcodes S9xOpcodesM0X0 [256];

#ifndef VAR_CYCLES
extern uint8 S9xE1M1X1 [256];
extern uint8 S9xE0M1X0 [256];
extern uint8 S9xE0M1X1 [256];
extern uint8 S9xE0M0X0 [256];
extern uint8 S9xE0M0X1 [256];
#endif

extern struct SCPUPACK CPUPack;
END_EXTERN_C

STATIC inline void S9xUnpackStatus()
{
    CPUPack.ICPU._Zero = (CPUPack.Registers.PL & Zero) == 0;
    CPUPack.ICPU._Negative = (CPUPack.Registers.PL & Negative);
    CPUPack.ICPU._Carry = (CPUPack.Registers.PL & Carry);
    CPUPack.ICPU._Overflow = (CPUPack.Registers.PL & Overflow) >> 6;
}

STATIC inline void S9xPackStatus()
{
    CPUPack.Registers.PL &= ~(Zero | Negative | Carry | Overflow);
    CPUPack.Registers.PL |= CPUPack.ICPU._Carry | ((CPUPack.ICPU._Zero == 0) << 1) |
		    (CPUPack.ICPU._Negative & 0x80) | (CPUPack.ICPU._Overflow << 6);
}

STATIC inline void CLEAR_IRQ_SOURCE (uint32 M)
{
    CPUPack.CPU.IRQActive &= ~M;
    if (!CPUPack.CPU.IRQActive)
	CPUPack.CPU.Flags &= ~IRQ_PENDING_FLAG;
}
	
STATIC inline void S9xFixCycles ()
{
    if (CheckEmulation ())
    {
#ifndef VAR_CYCLES
	CPUPack.ICPU.Speed = S9xE1M1X1;
#endif
	CPUPack.ICPU.S9xOpcodes = S9xOpcodesM1X1;
    }
    else
    if (CheckMemory ())
    {
	if (CheckIndex ())
	{
#ifndef VAR_CYCLES
	    CPUPack.ICPU.Speed = S9xE0M1X1;
#endif
	    CPUPack.ICPU.S9xOpcodes = S9xOpcodesM1X1;
	}
	else
	{
#ifndef VAR_CYCLES
	    CPUPack.ICPU.Speed = S9xE0M1X0;
#endif
	    CPUPack.ICPU.S9xOpcodes = S9xOpcodesM1X0;
	}
    }
    else
    {
	if (CheckIndex ())
	{
#ifndef VAR_CYCLES
	    CPUPack.ICPU.Speed = S9xE0M0X1;
#endif
	    CPUPack.ICPU.S9xOpcodes = S9xOpcodesM0X1;
	}
	else
	{
#ifndef VAR_CYCLES
	    CPUPack.ICPU.Speed = S9xE0M0X0;
#endif
	    CPUPack.ICPU.S9xOpcodes = S9xOpcodesM0X0;
	}
    }
}

#define S9xReschedule() { \
	uint8 which; \
  long max; \
  if (CPUPack.CPU.WhichEvent == HBLANK_START_EVENT || CPUPack.CPU.WhichEvent == HTIMER_AFTER_EVENT) { \
		which = HBLANK_END_EVENT; \
		max = Settings.H_Max; \
		S9x_Current_HBlank_Event=S9xDoHBlankProcessing_HBLANK_END_EVENT; \
  } else { \
		which = HBLANK_START_EVENT; \
		max = Settings.HBlankStart; \
		S9x_Current_HBlank_Event=S9xDoHBlankProcessing_HBLANK_START_EVENT; \
  } \
 \
  if (PPU.HTimerEnabled && (long) PPU.HTimerPosition < max &&	(long) PPU.HTimerPosition > CPUPack.CPU.NextEvent && \
		(!PPU.VTimerEnabled || (PPU.VTimerEnabled && CPUPack.CPU.V_Counter == PPU.IRQVBeamPos))) { \
		which = (long) PPU.HTimerPosition < Settings.HBlankStart ? HTIMER_BEFORE_EVENT : HTIMER_AFTER_EVENT; \
		S9x_Current_HBlank_Event=(long) PPU.HTimerPosition < Settings.HBlankStart ?S9xDoHBlankProcessing_HTIMER_BEFORE_EVENT:S9xDoHBlankProcessing_HTIMER_AFTER_EVENT; \
		max = PPU.HTimerPosition; \
  } \
  CPUPack.CPU.NextEvent = max; \
  CPUPack.CPU.WhichEvent = which; \
}

#endif
