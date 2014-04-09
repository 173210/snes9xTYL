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
#ifndef _CPUADDR_H_
#define _CPUADDR_H_

EXTERN_C long OpAddress;

STATIC inline long Immediate8 ()
{
    long OpAddress = CPUPack.ICPU.ShiftedPB + CPUPack.CPU.PC - CPUPack.CPU.PCBase;
    CPUPack.CPU.PC++;
	return OpAddress;
}

STATIC inline long Immediate16 ()
{
    long OpAddress = CPUPack.ICPU.ShiftedPB + CPUPack.CPU.PC - CPUPack.CPU.PCBase;
    CPUPack.CPU.PC += 2;
	return OpAddress;
}

STATIC inline long Relative ()
{
    signed char s9xInt8 = *CPUPack.CPU.PC++;
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeed;
#endif    
	return ((int) (CPUPack.CPU.PC - CPUPack.CPU.PCBase) + s9xInt8) & 0xffff;;
}

STATIC inline long RelativeLong ()
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = *(uint16 *) CPUPack.CPU.PC;
#else
    long OpAddress = *CPUPack.CPU.PC + (*(CPUPack.CPU.PC + 1) << 8);
#endif
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeedx2 + ONE_CYCLE;
#endif
    CPUPack.CPU.PC += 2;
    OpAddress += (CPUPack.CPU.PC - CPUPack.CPU.PCBase);
    OpAddress &= 0xffff;
	return OpAddress;
}

STATIC inline long AbsoluteIndexedIndirect ()
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = (CPUPack.Registers.X.W + *(uint16 *) CPUPack.CPU.PC) & 0xffff;
#else
    long OpAddress = (CPUPack.Registers.X.W + *CPUPack.CPU.PC + (*(CPUPack.CPU.PC + 1) << 8)) & 0xffff;
#endif
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeedx2;
#endif
    CPUPack.CPU.PC += 2;
    OpAddress = S9xGetWord (CPUPack.ICPU.ShiftedPB + OpAddress);
	return OpAddress;
}

STATIC inline long AbsoluteIndirectLong ()
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = *(uint16 *) CPUPack.CPU.PC;
#else
    long OpAddress = *CPUPack.CPU.PC + (*(CPUPack.CPU.PC + 1) << 8);
#endif

#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeedx2;
#endif
    CPUPack.CPU.PC += 2;
    OpAddress = S9xGetWord (OpAddress) | (S9xGetByte (OpAddress + 2) << 16);
	return OpAddress;
}

STATIC inline long AbsoluteIndirect ()
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = *(uint16 *) CPUPack.CPU.PC;
#else
    long OpAddress = *CPUPack.CPU.PC + (*(CPUPack.CPU.PC + 1) << 8);
#endif

#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeedx2;
#endif
    CPUPack.CPU.PC += 2;
    OpAddress = S9xGetWord (OpAddress) + CPUPack.ICPU.ShiftedPB;
	return OpAddress;
}

STATIC inline long Absolute ()
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = *(uint16 *) CPUPack.CPU.PC + CPUPack.ICPU.ShiftedDB;
#else
    long OpAddress = *CPUPack.CPU.PC + (*(CPUPack.CPU.PC + 1) << 8) + CPUPack.ICPU.ShiftedDB;
#endif
    CPUPack.CPU.PC += 2;
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeedx2;
#endif
	return OpAddress;
}

STATIC inline long AbsoluteLong ()
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = (*(uint32 *) CPUPack.CPU.PC) & 0xffffff;
#else
    long OpAddress = *CPUPack.CPU.PC + (*(CPUPack.CPU.PC + 1) << 8) + (*(CPUPack.CPU.PC + 2) << 16);
#endif
    CPUPack.CPU.PC += 3;
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeedx2 + CPUPack.CPU.MemSpeed;
#endif
	return OpAddress;
}

STATIC inline long Direct( void)
{
    long OpAddress = (*CPUPack.CPU.PC++ + CPUPack.Registers.D.W) & 0xffff;
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeed;
#endif
//    if (CPUPack.Registers.DL != 0) CPUPack.CPU.Cycles += ONE_CYCLE;
	return OpAddress;
}

STATIC inline long DirectIndirectIndexed ()
{
    long OpAddress = (*CPUPack.CPU.PC++ + CPUPack.Registers.D.W) & 0xffff;
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeed;
#endif

    OpAddress = CPUPack.ICPU.ShiftedDB + S9xGetWord (OpAddress) + CPUPack.Registers.Y.W;

//    if (CPUPack.Registers.DL != 0) CPUPack.CPU.Cycles += ONE_CYCLE;
    // XXX: always add one if STA
    // XXX: else Add one cycle if crosses page boundary
	return OpAddress;
}

STATIC inline long DirectIndirectIndexedLong ()
{
    long OpAddress = (*CPUPack.CPU.PC++ + CPUPack.Registers.D.W) & 0xffff;
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeed;
#endif

    OpAddress = S9xGetWord (OpAddress) + (S9xGetByte (OpAddress + 2) << 16) +
		CPUPack.Registers.Y.W;
//    if (CPUPack.Registers.DL != 0) CPUPack.CPU.Cycles += ONE_CYCLE;
	return OpAddress;
}

STATIC inline long DirectIndexedIndirect( void)
{
    long OpAddress = (*CPUPack.CPU.PC++ + CPUPack.Registers.D.W + CPUPack.Registers.X.W) & 0xffff;
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeed;
#endif

    OpAddress = S9xGetWord (OpAddress) + CPUPack.ICPU.ShiftedDB;

#ifdef VAR_CYCLES
//    if (CPUPack.Registers.DL != 0)
//	CPUPack.CPU.Cycles += TWO_CYCLES;
//    else
	CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
	return OpAddress;
}

STATIC inline long DirectIndexedX ()
{
    long OpAddress = (*CPUPack.CPU.PC++ + CPUPack.Registers.D.W + CPUPack.Registers.X.W) & 0xffff;
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeed;
#endif

#ifdef VAR_CYCLES
//    if (CPUPack.Registers.DL != 0)
//	CPUPack.CPU.Cycles += TWO_CYCLES;
//    else
	CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
	return OpAddress;
}

STATIC inline long DirectIndexedY ()
{
    long OpAddress = (*CPUPack.CPU.PC++ + CPUPack.Registers.D.W + CPUPack.Registers.Y.W) & 0xffff;
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeed;
#endif

#ifdef VAR_CYCLES
//    if (CPUPack.Registers.DL != 0)
//	CPUPack.CPU.Cycles += TWO_CYCLES;
//    else
	CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
	return OpAddress;
}

STATIC inline long AbsoluteIndexedX ()
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = CPUPack.ICPU.ShiftedDB + *(uint16 *) CPUPack.CPU.PC + CPUPack.Registers.X.W;
#else
    long OpAddress = CPUPack.ICPU.ShiftedDB + *CPUPack.CPU.PC + (*(CPUPack.CPU.PC + 1) << 8) +
		CPUPack.Registers.X.W;
#endif
    CPUPack.CPU.PC += 2;
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeedx2;
#endif
    // XXX: always add one cycle for ROL, LSR, etc
    // XXX: else is cross page boundary add one cycle
	return OpAddress;
}

STATIC inline long AbsoluteIndexedY ()
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = CPUPack.ICPU.ShiftedDB + *(uint16 *) CPUPack.CPU.PC + CPUPack.Registers.Y.W;
#else
    long OpAddress = CPUPack.ICPU.ShiftedDB + *CPUPack.CPU.PC + (*(CPUPack.CPU.PC + 1) << 8) +
		CPUPack.Registers.Y.W;
#endif    
    CPUPack.CPU.PC += 2;
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeedx2;
#endif
    // XXX: always add cycle for STA
    // XXX: else is cross page boundary add one cycle
	return OpAddress;
}

STATIC inline long AbsoluteLongIndexedX ()
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = (*(uint32 *) CPUPack.CPU.PC + CPUPack.Registers.X.W) & 0xffffff;
#else
    long OpAddress = (*CPUPack.CPU.PC + (*(CPUPack.CPU.PC + 1) << 8) + (*(CPUPack.CPU.PC + 2) << 16) + CPUPack.Registers.X.W) & 0xffffff;
#endif
    CPUPack.CPU.PC += 3;
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeedx2 + CPUPack.CPU.MemSpeed;
#endif
	return OpAddress;
}

STATIC inline long DirectIndirect ()
{
    long OpAddress = (*CPUPack.CPU.PC++ + CPUPack.Registers.D.W) & 0xffff;
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeed;
#endif
    OpAddress = S9xGetWord (OpAddress) + CPUPack.ICPU.ShiftedDB;

//    if (CPUPack.Registers.DL != 0) CPUPack.CPU.Cycles += ONE_CYCLE;
	return OpAddress;
}

STATIC inline long DirectIndirectLong ()
{
    long OpAddress = (*CPUPack.CPU.PC++ + CPUPack.Registers.D.W) & 0xffff;
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeed;
#endif
    OpAddress = S9xGetWord (OpAddress) +
		(S9xGetByte (OpAddress + 2) << 16);
//    if (CPUPack.Registers.DL != 0) CPUPack.CPU.Cycles += ONE_CYCLE;
	return OpAddress;
}

STATIC inline long StackRelative ()
{
    long OpAddress = (*CPUPack.CPU.PC++ + CPUPack.Registers.S.W) & 0xffff;
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeed;
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
	return OpAddress;
}

STATIC inline long StackRelativeIndirectIndexed ()
{
    long OpAddress = (*CPUPack.CPU.PC++ + CPUPack.Registers.S.W) & 0xffff;
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += CPUPack.CPU.MemSpeed;
    CPUPack.CPU.Cycles += TWO_CYCLES;
#endif
    OpAddress = (S9xGetWord (OpAddress) + CPUPack.ICPU.ShiftedDB +
		 CPUPack.Registers.Y.W) & 0xffffff;
	return OpAddress;
}
#endif
