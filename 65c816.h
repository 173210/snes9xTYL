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
#ifndef _65c816_h_
#define _65c816_h_

#define AL A.B.l
#define AH A.B.h
#define XL X.B.l
#define XH X.B.h
#define YL Y.B.l
#define YH Y.B.h
#define SL S.B.l
#define SH S.B.h
#define DL D.B.l
#define DH D.B.h
#define PL P.B.l
#define PH P.B.h

#define Carry       1
#define Zero        2
#define IRQ         4
#define Decimal     8
#define IndexFlag  16
#define MemoryFlag 32
#define Overflow   64
#define Negative  128
#define Emulation 256

#define ClearCarry() (CPUPack.ICPU._Carry = 0)
#define SetCarry() (CPUPack.ICPU._Carry = 1)
#define SetZero() (CPUPack.ICPU._Zero = 0)
#define ClearZero() (CPUPack.ICPU._Zero = 1)
#define SetIRQ() (CPUPack.Registers.PL |= IRQ)
#define ClearIRQ() (CPUPack.Registers.PL &= ~IRQ)
#define SetDecimal() (CPUPack.Registers.PL |= Decimal)
#define ClearDecimal() (CPUPack.Registers.PL &= ~Decimal)
#define SetIndex() (CPUPack.Registers.PL |= IndexFlag)
#define ClearIndex() (CPUPack.Registers.PL &= ~IndexFlag)
#define SetMemory() (CPUPack.Registers.PL |= MemoryFlag)
#define ClearMemory() (CPUPack.Registers.PL &= ~MemoryFlag)
#define SetOverflow() (CPUPack.ICPU._Overflow = 1)
#define ClearOverflow() (CPUPack.ICPU._Overflow = 0)
#define SetNegative() (CPUPack.ICPU._Negative = 0x80)
#define ClearNegative() (CPUPack.ICPU._Negative = 0)

#define CheckZero() (CPUPack.ICPU._Zero == 0)
#define CheckCarry() (CPUPack.ICPU._Carry)
#define CheckIRQ() (CPUPack.Registers.PL & IRQ)
#define CheckDecimal() (CPUPack.Registers.PL & Decimal)
#define CheckIndex() (CPUPack.Registers.PL & IndexFlag)
#define CheckMemory() (CPUPack.Registers.PL & MemoryFlag)
#define CheckOverflow() (CPUPack.ICPU._Overflow)
#define CheckNegative() (CPUPack.ICPU._Negative & 0x80)
#define CheckEmulation() (CPUPack.Registers.P.W & Emulation)

#define ClearFlags(f) (CPUPack.Registers.P.W &= ~(f))
#define SetFlags(f)   (CPUPack.Registers.P.W |=  (f))
#define CheckFlag(f)  (CPUPack.Registers.PL & (f))

typedef union
{
#ifdef LSB_FIRST
    struct { uint8 l,h; } B;
#else
    struct { uint8 h,l; } B;
#endif
    uint16 W/*,dummy*/;
} pair;

struct SRegisters{
    pair   P;
    uint8 /*uint32*/  PB;
    uint8 /*uint32*/  DB;

    pair   A;
    pair   D;

    pair   S;
    pair   X;

    pair   Y;
    uint16 /*uint32*/ PC;
};
#endif
