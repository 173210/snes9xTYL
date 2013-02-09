/*
 * Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 *
 * (c) Copyright 1996 - 2001 Gary Henderson (gary.henderson@ntlworld.com) and
 *                           Jerremy Koot (jkoot@snes9x.com)
 *
 * Super FX C emulator code 
 * (c) Copyright 1997 - 1999 Ivar (ivar@snes9x.com) and
 *                           Gary Henderson.
 * Super FX     assembler emulator code (c) Copyright 1998 zsKnight and _Demo_.
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

#include "snes9x.h"

#include "memmap.h"
#include "cpuops.h"
#include "ppu.h"
#include "cpuexec.h"
#include "debug.h"
#include "snapshot.h"
#include "gfx.h"
#include "missing.h"
#include "apu.h"
#include "dma.h"
#include "fxemu.h"
#include "sa1.h"




#if defined(__DEBUG_SNES_ASM__)||defined(__testopL__)
char *S9xMnemonics[256] =
  { "BRK", "ORA", "COP", "ORA", "TSB", "ORA", "ASL", "ORA",
  "PHP", "ORA", "ASL", "PHD", "TSB", "ORA", "ASL", "ORA",
  "BPL", "ORA", "ORA", "ORA", "TRB", "ORA", "ASL", "ORA",
  "CLC", "ORA", "INC", "TCS", "TRB", "ORA", "ASL", "ORA",
  "JSR", "AND", "JSL", "AND", "BIT", "AND", "ROL", "AND",
  "PLP", "AND", "ROL", "PLD", "BIT", "AND", "ROL", "AND",
  "BMI", "AND", "AND", "AND", "BIT", "AND", "ROL", "AND",
  "SEC", "AND", "DEC", "TSC", "BIT", "AND", "ROL", "AND",
  "RTI", "EOR", "DB ", "EOR", "MVP", "EOR", "LSR", "EOR",
  "PHA", "EOR", "LSR", "PHK", "JMP", "EOR", "LSR", "EOR",
  "BVC", "EOR", "EOR", "EOR", "MVN", "EOR", "LSR", "EOR",
  "CLI", "EOR", "PHY", "TCD", "JMP", "EOR", "LSR", "EOR",
  "RTS", "ADC", "PER", "ADC", "STZ", "ADC", "ROR", "ADC",
  "PLA", "ADC", "ROR", "RTL", "JMP", "ADC", "ROR", "ADC",
  "BVS", "ADC", "ADC", "ADC", "STZ", "ADC", "ROR", "ADC",
  "SEI", "ADC", "PLY", "TDC", "JMP", "ADC", "ROR", "ADC",
  "BRA", "STA", "BRL", "STA", "STY", "STA", "STX", "STA",
  "DEY", "BIT", "TXA", "PHB", "STY", "STA", "STX", "STA",
  "BCC", "STA", "STA", "STA", "STY", "STA", "STX", "STA",
  "TYA", "STA", "TXS", "TXY", "STZ", "STA", "STZ", "STA",
  "LDY", "LDA", "LDX", "LDA", "LDY", "LDA", "LDX", "LDA",
  "TAY", "LDA", "TAX", "PLB", "LDY", "LDA", "LDX", "LDA",
  "BCS", "LDA", "LDA", "LDA", "LDY", "LDA", "LDX", "LDA",
  "CLV", "LDA", "TSX", "TYX", "LDY", "LDA", "LDX", "LDA",
  "CPY", "CMP", "REP", "CMP", "CPY", "CMP", "DEC", "CMP",
  "INY", "CMP", "DEX", "WAI", "CPY", "CMP", "DEC", "CMP",
  "BNE", "CMP", "CMP", "CMP", "PEI", "CMP", "DEC", "CMP",
  "CLD", "CMP", "PHX", "STP", "JML", "CMP", "DEC", "CMP",
  "CPX", "SBC", "SEP", "SBC", "CPX", "SBC", "INC", "SBC",
  "INX", "SBC", "NOP", "XBA", "CPX", "SBC", "INC", "SBC",
  "BEQ", "SBC", "SBC", "SBC", "PEA", "SBC", "INC", "SBC",
  "SED", "SBC", "PLX", "XCE", "JSR", "SBC", "INC", "SBC"
};
#endif


extern uint32 os9x_asmcpu;
void
S9xMainLoop (void)
{
  for (;;)
    {
      APU_EXECUTE ();
      if (CPU.Flags)
	{
	  if (CPU.Flags & NMI_FLAG)
	    {
	      if (--CPU.NMICycleCount == 0)
		{
		  CPU.Flags &= ~NMI_FLAG;
		  if (CPU.WaitingForInterrupt)
		    {
		      CPU.WaitingForInterrupt = FALSE;
		      CPU.PC++;
		    }
		  S9xOpcode_NMI ();
		}
	    }

	  if (CPU.Flags & IRQ_PENDING_FLAG)
	    {
	      if (CPU.IRQCycleCount == 0)
		{
		  if (CPU.WaitingForInterrupt)
		    {
		      CPU.WaitingForInterrupt = FALSE;
		      CPU.PC++;
		    }
		  if (CPU.IRQActive && !Settings.DisableIRQ)
		    {
		      if (!CheckFlag (IRQ))
			S9xOpcode_IRQ ();
		    }
		  else
		    CPU.Flags &= ~IRQ_PENDING_FLAG;
		}
	      else
		CPU.IRQCycleCount--;
	    }
	  if (CPU.Flags & SCAN_KEYS_FLAG)
	    break;
	}

#ifdef CPU_SHUTDOWN
      CPU.PCAtOpcodeStart = CPU.PC;
#endif
      CPU.Cycles += CPU.MemSpeed;

      (*ICPU.S9xOpcodes[*CPU.PC++].S9xOpcode) ();


      S9xUpdateAPUTimer ();


      if (SA1.Executing)
	S9xSA1MainLoop ();
      DO_HBLANK_CHECK ();

    }


  Registers.PC = CPU.PC - CPU.PCBase;
  S9xPackStatus ();
  (APURegistersUncached.PC) = (IAPUuncached.PC) - (IAPUuncached.RAM);
  S9xAPUPackStatus ();

  if (CPU.Flags & SCAN_KEYS_FLAG)
    {
      S9xSyncSpeed ();
      CPU.Flags &= ~SCAN_KEYS_FLAG;
    }

  if (CPU.BRKTriggered && Settings.SuperFX && !CPU.TriedInterleavedMode2)
    {
      CPU.TriedInterleavedMode2 = TRUE;
      CPU.BRKTriggered = FALSE;
      S9xDeinterleaveMode2 ();
    }

}

void
S9xSetIRQ (uint32 source)
{
  CPU.IRQActive |= source;
  CPU.Flags |= IRQ_PENDING_FLAG;
  CPU.IRQCycleCount = 3;
  if (CPU.WaitingForInterrupt)
    {
      // Force IRQ to trigger immediately after WAI - 
      // Final Fantasy Mystic Quest crashes without this.
      CPU.IRQCycleCount = 0;
      CPU.WaitingForInterrupt = FALSE;
      CPU.PC++;
    }
}

void
S9xClearIRQ (uint32 source)
{
  CLEAR_IRQ_SOURCE (source);
}

void
S9xDoHBlankProcessing ()
{
	START_PROFILE_FUNC (S9xDoHBlankProcessing);
	
#ifdef CPU_SHUTDOWN
  CPU.WaitCounter++;
#endif
  switch (CPU.WhichEvent)
    {
    case HBLANK_START_EVENT:
      if (IPPU.HDMA && CPU.V_Counter <= PPU.ScreenHeight)
	IPPU.HDMA = S9xDoHDMA (IPPU.HDMA);

      break;

    case HBLANK_END_EVENT:
      S9xSuperFXExec ();


      CPU.Cycles -= Settings.H_Max;
      (IAPUuncached.NextAPUTimerPos) -= (Settings.H_Max * 10000L);
      if ( (IAPU_APUExecuting))
	(Uncache_APU_Cycles) -= Settings.H_Max;
      else
	(Uncache_APU_Cycles) = 0;

      CPU.NextEvent = -1;
      ICPU.Scanline++;

      if (++CPU.V_Counter >
	  (Settings.PAL ? SNES_MAX_PAL_VCOUNTER : SNES_MAX_NTSC_VCOUNTER))
	{
	  PPU.OAMAddr = PPU.SavedOAMAddr;
	  PPU.OAMFlip = 0;
	  CPU.V_Counter = 0;
	  CPU.NMIActive = FALSE;
	  ICPU.Frame++;
	  PPU.HVBeamCounterLatched = 0;
	  CPU.Flags |= SCAN_KEYS_FLAG;
	  S9xStartHDMA ();
	}

      if (PPU.VTimerEnabled && !PPU.HTimerEnabled &&
	  CPU.V_Counter == PPU.IRQVBeamPos)
	{
	  S9xSetIRQ (PPU_V_BEAM_IRQ_SOURCE);
	}

      if (CPU.V_Counter == PPU.ScreenHeight + FIRST_VISIBLE_LINE)
	{
	  // Start of V-blank
	  S9xEndScreenRefresh ();
	  PPU.FirstSprite = 0;
	  IPPU.HDMA = 0;
	  // Bits 7 and 6 of $4212 are computed when read in S9xGetPPU.
	  missing.dma_this_frame = 0;
	  IPPU.MaxBrightness = PPU.Brightness;
	  PPU.ForcedBlanking = (FillRAM[0x2100] >> 7) & 1;

	  FillRAM[0x4210] = 0x80;
	  if (FillRAM[0x4200] & 0x80)
	    {
	      CPU.NMIActive = TRUE;
	      CPU.Flags |= NMI_FLAG;
	      CPU.NMICycleCount = CPU.NMITriggerPoint;
	    }


	}

      if (CPU.V_Counter == PPU.ScreenHeight + 3)
	S9xUpdateJoypads ();

      if (CPU.V_Counter == FIRST_VISIBLE_LINE)
	{
	  FillRAM[0x4210] = 0;
	  CPU.Flags &= ~NMI_FLAG;
	  S9xStartScreenRefresh ();
	}
      if (CPU.V_Counter >= FIRST_VISIBLE_LINE &&
	  CPU.V_Counter < PPU.ScreenHeight + FIRST_VISIBLE_LINE)
	{
	  RenderLine (CPU.V_Counter - FIRST_VISIBLE_LINE);
	}

      break;
    case HTIMER_BEFORE_EVENT:
    case HTIMER_AFTER_EVENT:
      if (PPU.HTimerEnabled &&
	  (!PPU.VTimerEnabled || CPU.V_Counter == PPU.IRQVBeamPos))
	{
	  S9xSetIRQ (PPU_H_BEAM_IRQ_SOURCE);
	}
      break;
    }
  S9xReschedule ();
  
  FINISH_PROFILE_FUNC (S9xDoHBlankProcessing);
}
