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

/*
 * Aww, gee, do I have to include that whole header? :) Okay, the IAPU and
 * APURegisters structs as well as all of the execution macros and cases
 * are based on code from Snes9x. The SPC ROM code and cycle count table
 * were copied from SNEeSe, but the cycle counts originate from a Nintendo
 * manual, I think. The UpdateTimer macro and latched cycle counter system
 * is based off the SNEeSe source code as well. The rest, I pretty much
 * wrote from scratch. The whole process, including converting dsp.c and
 * main.c to C++, took about 8 hours. Enjoy.
 *
 * Oh, thought it'd be interesting to note that this source code performs
 * about 80% as fast as the original on my system, but if compiled with
 * /O3 using Intel's C++ compiler, it performs on par with the original
 * SNEeSe assembly CPU core. Whew.
 *
 * -Chris Moeller, May 9, 2003
 */

//#include "TapWave.h"

#include "snes9x.h"

#include "psp/openspc++/spc.h"

//***************** added in PocketSPC (used to be in spc.h but was defined as const and declared a value, #included in multiple source files
int TS_CYC = 1024000/44100;//32000;

static const uint8 SPC_ROM_CODE[] = {
	0xCD, 0xEF, 0xBD, 0xE8, 0x00, 0xC6, 0x1D, 0xD0,
	0xFC, 0x8F, 0xAA, 0xF4, 0x8F, 0xBB, 0xF5, 0x78,
	0xCC, 0xF4, 0xD0, 0xFB, 0x2F, 0x19, 0xEB, 0xF4,
	0xD0, 0xFC, 0x7E, 0xF4, 0xD0, 0x0B, 0xE4, 0xF5,
	0xCB, 0xF4, 0xD7, 0x00, 0xFC, 0xD0, 0xF3, 0xAB,
	0x01, 0x10, 0xEF, 0x7E, 0xF4, 0x10, 0xEB, 0xBA,
	0xF6, 0xDA, 0x00, 0xBA, 0xF4, 0xC4, 0xF4, 0xDD,
	0x5D, 0xD0, 0xDB, 0x1F, 0x00, 0x00, 0xC0, 0xFF
};

static const int8 SPCCycleTable[] = {
	2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 4, 6, 8,
	2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 6, 5, 2, 2, 4, 6,
	2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 4, 5, 4,
	2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 6, 5, 2, 2, 3, 8,
	2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 4, 6, 6,
	2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 4, 5, 2, 2, 4, 3,
	2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 4, 5, 5,
	2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 3, 6,
	2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 2, 4, 5,
	2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 12, 5,
	3, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 2, 4, 4,
	2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 3, 4,
	3, 8, 4, 5, 4, 5, 4, 7, 2, 5, 6, 4, 5, 2, 4, 9,
	2, 8, 4, 5, 5, 6, 6, 7, 4, 5, 4, 5, 2, 2, 6, 3,
	2, 8, 4, 5, 3, 4, 3, 6, 2, 4, 5, 3, 4, 3, 4, 3,
	2, 8, 4, 5, 4, 5, 5, 6, 3, 4, 5, 4, 2, 2, 4, 3
};

#define Carry       1
#define Zero        2
#define Interrupt   4
#define HalfCarry   8
#define BreakFlag  16
#define DirectPageFlag 32
#define Overflow   64
#define Negative  128

#define APUClearCarry() (IAPU._Carry = 0)
#define APUSetCarry() (IAPU._Carry = 1)
#define APUSetInterrupt() (APURegisters.P |= Interrupt)
#define APUClearInterrupt() (APURegisters.P &= ~Interrupt)
#define APUSetHalfCarry() (APURegisters.P |= HalfCarry)
#define APUClearHalfCarry() (APURegisters.P &= ~HalfCarry)
#define APUSetBreak() (APURegisters.P |= BreakFlag)
#define APUClearBreak() (APURegisters.P &= ~BreakFlag)
#define APUSetDirectPage() (APURegisters.P |= DirectPageFlag)
#define APUClearDirectPage() (APURegisters.P &= ~DirectPageFlag)
#define APUSetOverflow() (IAPU._Overflow = 1)
#define APUClearOverflow() (IAPU._Overflow = 0)

#define APUCheckZero() (IAPU._Zero == 0)
#define APUCheckCarry() (IAPU._Carry)
#define APUCheckInterrupt() (APURegisters.P & Interrupt)
#define APUCheckHalfCarry() (APURegisters.P & HalfCarry)
#define APUCheckBreak() (APURegisters.P & BreakFlag)
#define APUCheckDirectPage() (APURegisters.P & DirectPageFlag)
#define APUCheckOverflow() (IAPU._Overflow)
#define APUCheckNegative() (IAPU._Zero & 0x80)

#define APUClearFlags(f) (APURegisters.P &= ~(f))
#define APUSetFlags(f)   (APURegisters.P |=  (f))
#define APUCheckFlag(f)  (APURegisters.P &   (f))

#ifdef WORDS_BIGENDIAN

#define PushW(w)\
    *(SPC_RAM + 0xFF + APURegisters.S) = w;\
    *(SPC_RAM + 0x100 + APURegisters.S) = (w >> 8);\
    APURegisters.S -= 2;
#define PopW(w)\
    APURegisters.S += 2; \
    (w) = *(SPC_RAM + 0xFF + APURegisters.S) + (*(SPC_RAM + 0x100 + APURegisters.S) << 8);

#else

#define PushW(w)\
    *(SPC_RAM + 0xff + APURegisters.S) = w;\
    *(SPC_RAM + 0x100 + APURegisters.S) = (w >> 8);\
    APURegisters.S -= 2;
#define PopW(w)\
    APURegisters.S += 2; \
    (w) = *(SPC_RAM + 0xff + APURegisters.S) + (*(SPC_RAM + 0x100 + APURegisters.S) << 8);

/*#define PushW(w)\
    *(uint16 *) (SPC_RAM + 0xFF + APURegisters.S) = w;\
    APURegisters.S -= 2;
#define PopW(w)\
    APURegisters.S += 2;\
    w = *(uint16 *) (SPC_RAM + 0xFF + APURegisters.S);
*/
#endif

#define Push(b)\
	*(SPC_RAM + 0x100 + APURegisters.S) = b;\
	APURegisters.S--;
#define Pop(b)\
	APURegisters.S++;\
	b = *(SPC_RAM + 0x100 + APURegisters.S);

#if 0
#define UpdateTimer(a) \
if (SPC_RAM[0xF1] & (1 << (a))) \
{ \
	int count, remainder; \
	count = cycles - T##a##_cycle_latch; \
	if (a != 2) \
	{ \
		remainder = count & ~255; \
		count >>= 8; \
	} \
	else \
	{ \
		remainder = count & ~31; \
		count >>= 5; \
	} \
	T##a##_cycle_latch += remainder; \
	count += T##a##_position; \
	T##a##_position = count; \
	if (count >= T##a##_target) \
	{ \
		T##a##_counter = (T##a##_counter + count / T##a##_target) & 15; \
		T##a##_position = count % T##a##_target; \
	} \
}
#else
#define UpdateTimer(a) \
if (SPC_RAM[0xF1] & (1 << (a))) \
{ \
	int count, remainder; \
	count = cycles - T##a##_cycle_latch; \
	if (a != 2) \
	{ \
		remainder = count & ~127; \
		count >>= 7; \
	} \
	else \
	{ \
		remainder = count & ~15; \
		count >>= 4; \
	} \
	T##a##_cycle_latch += remainder; \
	count += T##a##_position; \
	T##a##_position = count; \
	if (count >= T##a##_target) \
	{ \
		T##a##_counter = (T##a##_counter + count / T##a##_target) & 15; \
		T##a##_position = count % T##a##_target; \
	} \
}
#endif

#define IndexedXIndirect()\
    IAPU.Address = ReadWordZ(OP1 + APURegisters.X)
#define Absolute()\
    IAPU.Address = OP1 + (OP2 << 8);

#define AbsoluteX()\
    IAPU.Address = OP1 + (OP2 << 8) + APURegisters.X;

#define AbsoluteY()\
    IAPU.Address = OP1 + (OP2 << 8) + APURegisters.YA.B.Y;

#define MemBit()\
    IAPU.Address = OP1 + (OP2 << 8);\
    IAPU.Bit = (uint8) (IAPU.Address >> 13);\
    IAPU.Address &= 0x1fff;

#define IndirectIndexedY()\
    IAPU.Address = ReadWordZ(OP1) + \
		  APURegisters.YA.B.Y;

__inline void
spc::UnpackStatus ()
{
	IAPU._Zero = ((APURegisters.P & Zero) == 0) | (APURegisters.P & Negative);
	IAPU._Carry = (APURegisters.P & Carry);
	IAPU._Overflow = (APURegisters.P & Overflow) >> 6;
}

__inline void
spc::PackStatus ()
{
	APURegisters.P &= ~(Zero | Negative | Carry | Overflow);
	APURegisters.P |= IAPU._Carry | ((IAPU._Zero == 0) << 1) |
		(IAPU._Zero & 0x80) | (IAPU._Overflow << 6);
}

uint8
spc::ReadByte (uint16 address)
{
	if (address >= 0xFFC0)
		return IAPU.ExtendedPage[address];
	switch (address)
	{
	case 0xF0:
	case 0xF8:
	case 0xF9:
	case 0xFA:
	case 0xFB:
	case 0xFC:
		return 0xFF;
		break;
/*
		case 0xF1:
*/
	default:
		return SPC_RAM[address];
		break;
	case 0xF2:
		return DSPindex;
		break;
	case 0xF3:
		return DSPregs[DSPindex];
		break;
	case 0xF4:
		return PORTR[0];
		break;
	case 0xF5:
		return PORTR[1];
		break;
	case 0xF6:
		return PORTR[2];
		break;
	case 0xF7:
		return PORTR[3];
		break;
	case 0xFD:
		{
			UpdateTimer (0);
			uint8 count = T0_counter;
			T0_counter = 0;
			return count;
		}
		break;
	case 0xFE:
		{
			UpdateTimer (1);
			uint8 count = T1_counter;
			T1_counter = 0;
			return count;
		}
		break;
	case 0xFF:
		{
			UpdateTimer (2);
			uint8 count = T2_counter;
			T2_counter = 0;
			return count;
		}
		break;
	}
}

void
spc::WriteByte (uint16 address, uint8 data)
{
	switch (address)
	{
	case 0xF0:
	case 0xF8:
	case 0xF9:
	case 0xFD:
	case 0xFE:
	case 0xFF:
		break;
	case 0xF1:
		{
			if (data & 0x80)
				IAPU.ExtendedPage = (unsigned char *) &SPC_ROM_CODE - 0xFFC0;
			else
				IAPU.ExtendedPage = SPC_RAM;
			if (data & 0x10)
			{
				SPC_RAM[0xF4] = 0;
				SPC_RAM[0xF5] = 0;
			}
			if (data & 0x20)
			{
				SPC_RAM[0xF6] = 0;
				SPC_RAM[0xF7] = 0;
			}
			if (!(SPC_RAM[0xF1] & 4) && (data & 4))
			{
				T2_counter = SPC_RAM[0xFF] = 0;
				T2_position = 0;
				T2_cycle_latch = cycles;
			}
			if (!(SPC_RAM[0xF1] & 2) && (data & 2))
			{
				T1_counter = SPC_RAM[0xFE] = 0;
				T1_position = 0;
				T1_cycle_latch = cycles;
			}
			if (!(SPC_RAM[0xF1] & 1) && (data & 1))
			{
				T0_counter = SPC_RAM[0xFD] = 0;
				T0_position = 0;
				T0_cycle_latch = cycles;
			}
			SPC_RAM[0xF1] = data;
		}
		break;
	case 0xF2:
		DSPindex = data;
		break;
	case 0xF3:
		DSPregs[DSPindex] = data;
		break;
	case 0xF4:
		PORTW[0] = data;
		break;
	case 0xF5:
		PORTW[1] = data;
		break;
	case 0xF6:
		PORTW[2] = data;
	case 0xF7:
		PORTW[3] = data;
		break;
	case 0xFA:
		if (SPC_RAM[0xFA] != data)
		{
			UpdateTimer (0);
			if (!data)
				T0_target = 0x100;
			else
				T0_target = data;
			SPC_RAM[0xFA] = data;
		}
		break;
	case 0xFB:
		if (SPC_RAM[0xFB] != data)
		{
			UpdateTimer (1);
			if (!data)
				T1_target = 0x100;
			else
				T1_target = data;
			SPC_RAM[0xFB] = data;
		}
		break;
	case 0xFC:
		if (SPC_RAM[0xFC] != data)
		{
			UpdateTimer (2);
			if (!data)
				T2_target = 0x100;
			else
				T2_target = data;
			SPC_RAM[0xFC] = data;
		}
		break;
	default:
		SPC_RAM[address] = data;
		break;
	}
}

uint16
spc::ReadWord (uint16 address)
{
	return ReadByte (address) | (ReadByte (address + 1) << 8);
}

void
spc::WriteWord (uint16 address, uint16 data)
{
	WriteByte (address, data & 0xFF);
	WriteByte (address + 1, data >> 8);
}

#define ReadByteZ(a) ReadByte(((a)&255) + IAPU._DirectPage)
#define ReadWordZ(a) ReadWord(((a)&255) + IAPU._DirectPage)
#define WriteByteZ(a,d) WriteByte(((a)&255) + IAPU._DirectPage, d)
#define WriteWordZ(a,d) WriteWord(((a)&255) + IAPU._DirectPage, d)

void
spc::reset ()
{
	IAPU.ExtendedPage = (unsigned char *) &SPC_ROM_CODE - 0xFFC0;

	cycles = 0;

	APURegisters.P = 0;
	UnpackStatus ();
	if (APUCheckDirectPage ())
		IAPU._DirectPage = 0x100;
	else
		IAPU._DirectPage = 0;
	APURegisters.YA.W = 0;
	APURegisters.X = 0;
	APURegisters.S = 0xEF;
	APURegisters.PC = ReadWord (0xFFFE);

	SPC_RAM[0xF1] = 0x80;

	T0_counter = 0;
	T1_counter = 0;
	T2_counter = 0;
	T0_target = 256;
	T1_target = 256;
	T2_target = 256;
	T0_position = 0;
	T1_position = 0;
	T2_position = 0;
	T0_cycle_latch = 0;
	T1_cycle_latch = 0;
	T2_cycle_latch = 0;

	PORTW[0] = 0;
	PORTW[1] = 0;
	PORTW[2] = 0;
	PORTW[3] = 0;

	PORTR[0] = 0;
	PORTR[1] = 0;
	PORTR[2] = 0;
	PORTR[3] = 0;

	DSPindex = 0;
}

void
spc::setstate (int pPC, int pA, int pX, int pY, int pP, int pSP)
{
	if (SPC_RAM[0xF1] & 0x80)
		IAPU.ExtendedPage = (unsigned char *) &SPC_ROM_CODE - 0xFFC0;
	else
		IAPU.ExtendedPage = SPC_RAM;
	T0_target = SPC_RAM[0xFA];
	if (!T0_target)
		T0_target = 0x100;
	T1_target = SPC_RAM[0xFB];
	if (!T1_target)
		T1_target = 0x100;
	T2_target = SPC_RAM[0xFC];
	if (!T2_target)
		T2_target = 0x100;
	T0_counter = SPC_RAM[0xFD] & 0xF;
	T1_counter = SPC_RAM[0xFE] & 0xF;
	T2_counter = SPC_RAM[0xFF] & 0xF;

	DSPindex = SPC_RAM[0xF2];

	PORTR[0] = SPC_RAM[0xF4];
	PORTR[1] = SPC_RAM[0xF5];
	PORTR[2] = SPC_RAM[0xF6];
	PORTR[3] = SPC_RAM[0xF7];

	APURegisters.PC = pPC;
	APURegisters.YA.B.A = pA;
	APURegisters.YA.B.Y = pY;
	APURegisters.X = pX;
	APURegisters.S = pSP;

	APURegisters.P = pP;
	UnpackStatus ();
}

void
spc::run (int cyc)
{
	cycles -= cyc;
	T0_cycle_latch -= cyc;
	T1_cycle_latch -= cyc;
	T2_cycle_latch -= cyc;

	while (cycles < 0)
		exec ();
}

void
spc::writeport (int port, char data)
{
	if (port >= 0 && port < 4)
		PORTR[port] = data;
}

int
spc::readport (int port)
{
	if (port >= 0 && port < 4)
		return PORTW[port];
	else
		return 0xFF;
}

void
spc::exec ()
{
	uint32 address = APURegisters.PC;
	uint32 opcode = ReadByte (address);
	uint8 Work8;
	uint16 Work16;
	uint32 Work32;
	int8 Int8;
	int16 Int16;
	int32 Int32;
	uint8 W1;
	uint32 op1, op2;

	cycles += SPCCycleTable[opcode];

/*
		char err[32];
		wsprintf(err, "%2.2x %2.2x %2.2x", opcode, ReadByte(address+1), ReadByte(address+2));
		OutputDebugString(err);
*/

	switch (opcode)
	{

	case 0x00:
		address++;
		break;

#define TCALL(n)\
{\
    PushW (address + 1); \
    APURegisters.PC = (SPC_RAM [0xFFC0 + ((15 - n) << 1)] + \
					  (SPC_RAM [0xFFC1 + ((15 - n) << 1)] << 8)); \
	address = APURegisters.PC; \
}

	case 0x01:
		TCALL (0);
		break;
	case 0x11:
		TCALL (1);
		break;
	case 0x21:
		TCALL (2);
		break;
	case 0x31:
		TCALL (3);
		break;
	case 0x41:
		TCALL (4);
		break;
	case 0x51:
		TCALL (5);
		break;
	case 0x61:
		TCALL (6);
		break;
	case 0x71:
		TCALL (7);
		break;
	case 0x81:
		TCALL (8);
		break;
	case 0x91:
		TCALL (9);
		break;
	case 0xA1:
		TCALL (10);
		break;
	case 0xB1:
		TCALL (11);
		break;
	case 0xC1:
		TCALL (12);
		break;
	case 0xD1:
		TCALL (13);
		break;
	case 0xE1:
		TCALL (14);
		break;
	case 0xF1:
		TCALL (15);
		break;

#define OP1 ReadByte(address + 1)
#define OP2 ReadByte(address + 2)

	case 0x3F:
		Absolute ();
		PushW (address + 3);
		address = IAPU.Address;
		break;
	case 0x4F:
		Work8 = OP1;
		PushW (address + 2);
		address = 0xff00 + Work8;
		break;

#define SET(b) \
	op1 = OP1; \
	WriteByteZ(op1, (uint8) (ReadByteZ (op1) | (1 << (b)))); \
	address += 2;

	case 0x02:
		SET (0);
		break;
	case 0x22:
		SET (1);
		break;
	case 0x42:
		SET (2);
		break;
	case 0x62:
		SET (3);
		break;
	case 0x82:
		SET (4);
		break;
	case 0xA2:
		SET (5);
		break;
	case 0xC2:
		SET (6);
		break;
	case 0xE2:
		SET (7);
		break;

#define CLR(b) \
	op1 = OP1; \
	WriteByteZ(op1, (uint8) (ReadByteZ (op1) & ~(1 << (b)))); \
	address += 2;

	case 0x12:
		CLR (0);
		break;
	case 0x32:
		CLR (1);
		break;
	case 0x52:
		CLR (2);
		break;
	case 0x72:
		CLR (3);
		break;
	case 0x92:
		CLR (4);
		break;
	case 0xB2:
		CLR (5);
		break;
	case 0xD2:
		CLR (6);
		break;
	case 0xF2:
		CLR (7);
		break;

#define Relative()\
    Int8 = OP1;\
    Int16 = (int) (address + 2) + Int8;

#define Relative2()\
    Int8 = OP2;\
    Int16 = (int) (address + 3) + Int8;

#define BBS(b) \
{ \
	Work8 = OP1; \
	if (ReadByteZ(Work8) & (1 << (b))) \
	{ \
		Relative2 (); \
		address = (uint16) Int16; \
		cycles += 2; \
	} \
	else \
		address += 3; \
}

	case 0x03:
		BBS (0);
		break;
	case 0x23:
		BBS (1);
		break;
	case 0x43:
		BBS (2);
		break;
	case 0x63:
		BBS (3);
		break;
	case 0x83:
		BBS (4);
		break;
	case 0xA3:
		BBS (5);
		break;
	case 0xC3:
		BBS (6);
		break;
	case 0xE3:
		BBS (7);
		break;

#define BBC(b) \
{ \
	Work8 = OP1; \
	if (!(ReadByteZ(Work8) & (1 << (b)))) \
	{ \
		Relative2 (); \
		address = (uint16) Int16; \
		cycles += 2; \
	} \
	else \
		address += 3; \
}

	case 0x13:
		BBC (0);
		break;
	case 0x33:
		BBC (1);
		break;
	case 0x53:
		BBC (2);
		break;
	case 0x73:
		BBC (3);
		break;
	case 0x93:
		BBC (4);
		break;
	case 0xB3:
		BBC (5);
		break;
	case 0xD3:
		BBC (6);
		break;
	case 0xF3:
		BBC (7);
		break;

#define APUSetZN8(b)\
    IAPU._Zero = (b);

#define APUSetZN16(w)\
    IAPU._Zero = ((w) != 0) | ((w) >> 8);

	case 0x04:
		APURegisters.YA.B.A |= ReadByteZ (OP1);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0x05:
		Absolute ();
		APURegisters.YA.B.A |= ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 3;
		break;
	case 0x06:
		APURegisters.YA.B.A |= ReadByteZ (APURegisters.X);
		APUSetZN8 (APURegisters.YA.B.A);
		address++;
		break;
	case 0x07:
		IndexedXIndirect ();
		APURegisters.YA.B.A |= ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0x08:
		APURegisters.YA.B.A |= OP1;
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0x09:
		Work8 = ReadByteZ (OP1);
		op2 = OP2;
		Work8 |= ReadByteZ (op2);
		WriteByteZ (op2, Work8);
		APUSetZN8 (Work8);
		address += 3;
		break;
	case 0x14:
		APURegisters.YA.B.A |= ReadByteZ (OP1 + APURegisters.X);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0x15:
		AbsoluteX ();
		APURegisters.YA.B.A |= ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 3;
		break;
	case 0x16:
		AbsoluteY ();
		APURegisters.YA.B.A |= ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 3;
		break;
	case 0x17:
		IndirectIndexedY ();
		APURegisters.YA.B.A |= ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0x18:
		op2 = OP2;
		Work8 = OP1;
		Work8 |= ReadByteZ (op2);
		WriteByteZ (op2, Work8);
		APUSetZN8 (Work8);
		address += 3;
		break;
	case 0x19:
		Work8 = ReadByteZ (APURegisters.X) | ReadByteZ (APURegisters.YA.B.Y);
		APUSetZN8 (Work8);
		WriteByteZ (APURegisters.X, Work8);
		address++;
		break;

	case 0x0A:
		MemBit ();
		if (!APUCheckCarry ())
		{
			if (ReadByte (IAPU.Address) & (1 << IAPU.Bit))
				APUSetCarry ();
		}
		address += 3;
		break;
	case 0x2A:
		MemBit ();
		if (!APUCheckCarry ())
		{
			if (!(ReadByte (IAPU.Address) & (1 << IAPU.Bit)))
				APUSetCarry ();
		}
		address += 3;
		break;
	case 0x4A:
		MemBit ();
		if (APUCheckCarry ())
		{
			if (!(ReadByte (IAPU.Address) & (1 << IAPU.Bit)))
				APUClearCarry ();
		}
		address += 3;
		break;
	case 0x6A:
		MemBit ();
		if (APUCheckCarry ())
		{
			if ((ReadByte (IAPU.Address) & (1 << IAPU.Bit)))
				APUClearCarry ();
		}
		address += 3;
		break;
	case 0x8A:
		MemBit ();
		if (APUCheckCarry ())
		{
			if (ReadByte (IAPU.Address) & (1 << IAPU.Bit))
				APUClearCarry ();
		}
		else
		{
			if (ReadByte (IAPU.Address) & (1 << IAPU.Bit))
				APUSetCarry ();
		}
		address += 3;
		break;
	case 0xAA:
		MemBit ();
		if (ReadByte (IAPU.Address) & (1 << IAPU.Bit))
			APUSetCarry ();
		else
			APUClearCarry ();
		address += 3;
		break;
	case 0xCA:
		MemBit ();
		if (APUCheckCarry ())
		{
			WriteByte (IAPU.Address,
					   ReadByte (IAPU.Address) | (1 << IAPU.Bit));
		}
		else
		{
			WriteByte (IAPU.Address,
					   ReadByte (IAPU.Address) & ~(1 << IAPU.Bit));
		}
		address += 3;
		break;
	case 0xEA:
		MemBit ();
		WriteByte (IAPU.Address, ReadByte (IAPU.Address) ^ (1 << IAPU.Bit));
		address += 3;
		break;

#define ASL(b)\
    IAPU._Carry = ((b) & 0x80) != 0; \
    (b) <<= 1;\
    APUSetZN8 (b);

	case 0x0B:
		op1 = OP1;
		Work8 = ReadByteZ (op1);
		ASL (Work8);
		WriteByteZ (op1, Work8);
		address += 2;
		break;
	case 0x0C:
		Absolute ();
		Work8 = ReadByte (IAPU.Address);
		ASL (Work8);
		WriteByte (IAPU.Address, Work8);
		address += 3;
		break;
	case 0x1B:
		op1 = OP1;
		Work8 = ReadByteZ (op1 + APURegisters.X);
		ASL (Work8);
		WriteByteZ (op1 + APURegisters.X, Work8);
		address += 2;
		break;
	case 0x1C:
		ASL (APURegisters.YA.B.A);
		address++;
		break;

	case 0x0D:
		PackStatus ();
		Push (APURegisters.P);
		address++;
		break;
	case 0x2D:
		Push (APURegisters.YA.B.A);
		address++;
		break;
	case 0x4D:
		Push (APURegisters.X);
		address++;
		break;
	case 0x6D:
		Push (APURegisters.YA.B.Y);
		address++;
		break;
	case 0x8E:
		Pop (APURegisters.P);
		UnpackStatus ();
		if (APUCheckDirectPage ())
			IAPU._DirectPage = 0x100;
		else
			IAPU._DirectPage = 0;
		address++;
		break;
	case 0xAE:
		Pop (APURegisters.YA.B.A);
		address++;
		break;
	case 0xCE:
		Pop (APURegisters.X);
		address++;
		break;
	case 0xEE:
		Pop (APURegisters.YA.B.Y);
		address++;
		break;

	case 0x0E:
		Absolute ();
		Work8 = ReadByte (IAPU.Address);
		WriteByte (IAPU.Address, Work8 | APURegisters.YA.B.A);
		Work8 &= APURegisters.YA.B.A;
		APUSetZN8 (Work8);
		address += 3;
		break;
	case 0x4E:
		Absolute ();
		Work8 = ReadByte (IAPU.Address);
		WriteByte (IAPU.Address, Work8 & ~APURegisters.YA.B.A);
		Work8 &= APURegisters.YA.B.A;
		APUSetZN8 (Work8);
		address += 3;
		break;
	case 0x0F:
		PushW (address + 1);
		PackStatus ();
		Push (APURegisters.P);
		APUSetBreak ();
		APUClearInterrupt ();
		address = SPC_RAM[0xFFE0] + (SPC_RAM[0xFFE1] << 8);
		break;
	case 0xEF:
		break;
	case 0xFF:
		break;
	case 0x10:
		if (!APUCheckNegative ())
		{
			Relative ();
			address = (uint16) Int16;
			cycles += 2;
		}
		else
			address += 2;
		break;
	case 0x30:
		if (APUCheckNegative ())
		{
			Relative ();
			address = (uint16) Int16;
			cycles += 2;
		}
		else
			address += 2;
		break;
	case 0x90:
		if (!APUCheckCarry ())
		{
			Relative ();
			address = (uint16) Int16;
			cycles += 2;
		}
		else
			address += 2;
		break;
	case 0xB0:
		if (APUCheckCarry ())
		{
			Relative ();
			address = (uint16) Int16;
			cycles += 2;
		}
		else
			address += 2;
		break;
	case 0xD0:
		if (!APUCheckZero ())
		{
			Relative ();
			address = (uint16) Int16;
			cycles += 2;
		}
		else
			address += 2;
		break;
	case 0xF0:
		if (APUCheckZero ())
		{
			Relative ();
			address = (uint16) Int16;
			cycles += 2;
		}
		else
			address += 2;
		break;
	case 0x50:
		if (!APUCheckOverflow ())
		{
			Relative ();
			address = (uint16) Int16;
			cycles += 2;
		}
		else
			address += 2;
		break;
	case 0x70:
		if (APUCheckOverflow ())
		{
			Relative ();
			address = (uint16) Int16;
			cycles += 2;
		}
		else
			address += 2;
		break;
	case 0x2F:
		Relative ();
		address = (uint16) Int16;
		break;

	case 0x80:
		APUSetCarry ();
		address++;
		break;
	case 0xED:
		IAPU._Carry ^= 1;
		address++;
		break;

	case 0x40:
		APUSetDirectPage ();
		IAPU._DirectPage = 0x100;
		address++;
		break;

	case 0x1A:
		op1 = OP1;
		Work16 = ReadWordZ (op1);
		Work16--;
		WriteWordZ (op1, Work16);
		APUSetZN16 (Work16);
		address += 2;
		break;
	case 0x5A:
		Work16 = ReadWordZ (OP1);
		Int32 = (int) APURegisters.YA.W - (int) Work16;
		IAPU._Carry = Int32 >= 0;
		APUSetZN16 ((uint16) Int32);
		address += 2;
		break;
	case 0x3A:
		op1 = OP1;
		Work16 = ReadWordZ (op1);
		Work16++;
		WriteWordZ (op1, Work16);
		APUSetZN16 (Work16);
		address += 2;
		break;
	case 0x7A:
		Work16 = ReadWordZ (OP1);
		Work32 = (uint32) APURegisters.YA.W + Work16;
		IAPU._Carry = Work32 >= 0x10000;
		if (~(APURegisters.YA.W ^ Work16) & (Work16 ^ (uint16) Work32) &
			0x8000)
			APUSetOverflow ();
		else
			APUClearOverflow ();
		APUClearHalfCarry ();
		if ((APURegisters.YA.W ^ Work16 ^ (uint16) Work32) & 0x10)
			APUSetHalfCarry ();
		APURegisters.YA.W = (uint16) Work32;
		APUSetZN16 (APURegisters.YA.W);
		address += 2;
		break;
	case 0x9A:
		Work16 = ReadWordZ (OP1);
		Int32 = (long) APURegisters.YA.W - (long) Work16;
		APUClearHalfCarry ();
		IAPU._Carry = Int32 >= 0;
		if (((APURegisters.YA.W ^ Work16) & 0x8000) &&
			((APURegisters.YA.W ^ (uint16) Int32) & 0x8000))
			APUSetOverflow ();
		else
			APUClearOverflow ();
		if (((APURegisters.YA.W ^ Work16) & 0x0080) &&
			((APURegisters.YA.W ^ (uint16) Int32) & 0x0080))
			APUSetHalfCarry ();
		APUSetHalfCarry ();
		if ((APURegisters.YA.W ^ Work16 ^ (uint16) Int32) & 0x10)
			APUClearHalfCarry ();
		APURegisters.YA.W = (uint16) Int32;
		APUSetZN16 (APURegisters.YA.W);
		address += 2;
		break;
	case 0xBA:
		APURegisters.YA.W = ReadWordZ (OP1);
		APUSetZN16 (APURegisters.YA.W);
		address += 2;
		break;
	case 0xDA:
		WriteWordZ (OP1, APURegisters.YA.W);
		address += 2;
		break;

#define CMP(a,b)\
Int16 = (short) (a) - (short) (b);\
IAPU._Carry = Int16 >= 0;\
APUSetZN8 ((uint8) Int16);

	case 0x64:
		Work8 = ReadByteZ (OP1);
		CMP (APURegisters.YA.B.A, Work8);
		address += 2;
		break;
	case 0x65:
		Absolute ();
		Work8 = ReadByte (IAPU.Address);
		CMP (APURegisters.YA.B.A, Work8);
		address += 3;
		break;
	case 0x66:
		Work8 = ReadByteZ (APURegisters.X);
		CMP (APURegisters.YA.B.A, Work8);
		address++;
		break;
	case 0x67:
		IndexedXIndirect ();
		Work8 = ReadByte (IAPU.Address);
		CMP (APURegisters.YA.B.A, Work8);
		address += 2;
		break;
	case 0x68:
		Work8 = OP1;
		CMP (APURegisters.YA.B.A, Work8);
		address += 2;
		break;
	case 0x69:
		W1 = ReadByteZ (OP1);
		Work8 = ReadByteZ (OP2);
		CMP (Work8, W1);
		address += 3;
		break;
	case 0x74:
		Work8 = ReadByteZ (OP1 + APURegisters.X);
		CMP (APURegisters.YA.B.A, Work8);
		address += 2;
		break;
	case 0x75:
		AbsoluteX ();
		Work8 = ReadByte (IAPU.Address);
		CMP (APURegisters.YA.B.A, Work8);
		address += 3;
		break;
	case 0x76:
		AbsoluteY ();
		Work8 = ReadByte (IAPU.Address);
		CMP (APURegisters.YA.B.A, Work8);
		address += 3;
		break;
	case 0x77:
		IndirectIndexedY ();
		Work8 = ReadByte (IAPU.Address);
		CMP (APURegisters.YA.B.A, Work8);
		address += 2;
		break;
	case 0x78:
		Work8 = OP1;
		W1 = ReadByteZ (OP2);
		CMP (W1, Work8);
		address += 3;
		break;
	case 0x79:
		W1 = ReadByteZ (APURegisters.X);
		Work8 = ReadByteZ (APURegisters.YA.B.Y);
		CMP (W1, Work8);
		address++;
		break;
	case 0x1E:
		Absolute ();
		Work8 = ReadByte (IAPU.Address);
		CMP (APURegisters.X, Work8);
		address += 3;
		break;
	case 0x3E:
		Work8 = ReadByteZ (OP1);
		CMP (APURegisters.X, Work8);
		address += 2;
		break;
	case 0xC8:
		CMP (APURegisters.X, OP1);
		address += 2;
		break;
	case 0x5E:
		Absolute ();
		Work8 = ReadByte (IAPU.Address);
		CMP (APURegisters.YA.B.Y, Work8);
		address += 3;
		break;
	case 0x7E:
		Work8 = ReadByteZ (OP1);
		CMP (APURegisters.YA.B.Y, Work8);
		address += 2;
		break;
	case 0xAD:
		Work8 = OP1;
		CMP (APURegisters.YA.B.Y, Work8);
		address += 2;
		break;
	case 0x1F:
		Absolute ();
		address = ReadWord (IAPU.Address + APURegisters.X);
		break;
	case 0x5F:
		Absolute ();
		address = IAPU.Address;
		break;
	case 0x20:
		APUClearDirectPage ();
		IAPU._DirectPage = 0;
		address++;
		break;
	case 0x60:
		APUClearCarry ();
		address++;
		break;
	case 0xE0:
		APUClearHalfCarry ();
		APUClearOverflow ();
		address++;
		break;
	case 0x24:
		APURegisters.YA.B.A &= ReadByteZ (OP1);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0x25:
		Absolute ();
		APURegisters.YA.B.A &= ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 3;
		break;
	case 0x26:
		APURegisters.YA.B.A &= ReadByteZ (APURegisters.X);
		APUSetZN8 (APURegisters.YA.B.A);
		address++;
		break;
	case 0x27:
		IndexedXIndirect ();
		APURegisters.YA.B.A &= ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0x28:
		APURegisters.YA.B.A &= OP1;
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0x29:
		Work8 = ReadByteZ (OP1);
		Work8 &= ReadByteZ (op2 = OP2);
		WriteByteZ (op2, Work8);
		APUSetZN8 (Work8);
		address += 3;
		break;
	case 0x34:
		APURegisters.YA.B.A &= ReadByteZ (OP1 + APURegisters.X);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0x35:
		AbsoluteX ();
		APURegisters.YA.B.A &= ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 3;
		break;
	case 0x36:
		AbsoluteY ();
		APURegisters.YA.B.A &= ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 3;
		break;
	case 0x37:
		IndirectIndexedY ();
		APURegisters.YA.B.A &= ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0x38:
		Work8 = OP1;
		Work8 &= ReadByteZ (op2 = OP2);
		WriteByteZ (op2, Work8);
		APUSetZN8 (Work8);
		address += 3;
		break;
	case 0x39:
		Work8 = ReadByteZ (APURegisters.X) & ReadByteZ (APURegisters.YA.B.Y);
		APUSetZN8 (Work8);
		WriteByteZ (APURegisters.X, Work8);
		address++;
		break;

#define ROL(b)\
    Work16 = ((b) << 1) | APUCheckCarry (); \
    IAPU._Carry = Work16 >= 0x100; \
    (b) = (uint8) Work16; \
    APUSetZN8 (b);

	case 0x2B:
		Work8 = ReadByteZ (op1 = OP1);
		ROL (Work8);
		WriteByteZ (op1, Work8);
		address += 2;
		break;
	case 0x2C:
		Absolute ();
		Work8 = ReadByte (IAPU.Address);
		ROL (Work8);
		WriteByte (IAPU.Address, Work8);
		address += 3;
		break;
	case 0x3B:
		Work8 = ReadByteZ ((op1 = OP1) + APURegisters.X);
		ROL (Work8);
		WriteByteZ (op1 + APURegisters.X, Work8);
		address += 2;
		break;
	case 0x3C:
		ROL (APURegisters.YA.B.A);
		address++;
		break;
	case 0x2E:
		Work8 = OP1;
		if (ReadByteZ (Work8) != APURegisters.YA.B.A)
		{
			Relative2 ();
			address = (uint16) Int16;
			cycles += 2;
		}
		else
			address += 3;
		break;
	case 0xDE:
		Work8 = OP1 + APURegisters.X;
		if (ReadByteZ (Work8) != APURegisters.YA.B.A)
		{
			Relative2 ();
			address = (uint16) Int16;
			cycles += 2;
		}
		else
			address += 3;
		break;
	case 0x3D:
		APURegisters.X++;
		APUSetZN8 (APURegisters.X);
		address++;
		break;
	case 0xFC:
		APURegisters.YA.B.Y++;
		APUSetZN8 (APURegisters.YA.B.Y);
		address++;
		break;
	case 0x1D:
		APURegisters.X--;
		APUSetZN8 (APURegisters.X);
		address++;
		break;
	case 0xDC:
		APURegisters.YA.B.Y--;
		APUSetZN8 (APURegisters.YA.B.Y);
		address++;
		break;
	case 0xAB:
		Work8 = ReadByteZ (op1 = OP1) + 1;
		WriteByteZ (op1, Work8);
		APUSetZN8 (Work8);
		address += 2;
		break;
	case 0xAC:
		Absolute ();
		Work8 = ReadByte (IAPU.Address) + 1;
		WriteByte (IAPU.Address, Work8);
		APUSetZN8 (Work8);
		address += 3;
		break;
	case 0xBB:
		Work8 = ReadByteZ ((op1 = OP1) + APURegisters.X) + 1;
		WriteByteZ (op1 + APURegisters.X, Work8);
		APUSetZN8 (Work8);
		address += 2;
		break;
	case 0xBC:
		APURegisters.YA.B.A++;
		APUSetZN8 (APURegisters.YA.B.A);
		address++;
		break;
	case 0x8B:
		Work8 = ReadByteZ (op1 = OP1) - 1;
		WriteByteZ (op1, Work8);
		APUSetZN8 (Work8);
		address += 2;
		break;
	case 0x8C:
		Absolute ();
		Work8 = ReadByte (IAPU.Address) - 1;
		WriteByte (IAPU.Address, Work8);
		APUSetZN8 (Work8);
		address += 3;
		break;
	case 0x9B:
		Work8 = ReadByteZ ((op1 = OP1) + APURegisters.X) - 1;
		WriteByteZ (op1 + APURegisters.X, Work8);
		APUSetZN8 (Work8);
		address += 2;
		break;
	case 0x9C:
		APURegisters.YA.B.A--;
		APUSetZN8 (APURegisters.YA.B.A);
		address++;
		break;
	case 0x44:
		APURegisters.YA.B.A ^= ReadByteZ (OP1);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0x45:
		Absolute ();
		APURegisters.YA.B.A ^= ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 3;
		break;
	case 0x46:
		APURegisters.YA.B.A ^= ReadByteZ (APURegisters.X);
		APUSetZN8 (APURegisters.YA.B.A);
		address++;
		break;
	case 0x47:
		IndexedXIndirect ();
		APURegisters.YA.B.A ^= ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0x48:
		APURegisters.YA.B.A ^= OP1;
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0x49:
		Work8 = ReadByteZ (OP1);
		Work8 ^= ReadByteZ (op2 = OP2);
		WriteByteZ (op2, Work8);
		APUSetZN8 (Work8);
		address += 3;
		break;
	case 0x54:
		APURegisters.YA.B.A ^= ReadByteZ (OP1 + APURegisters.X);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0x55:
		AbsoluteX ();
		APURegisters.YA.B.A ^= ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 3;
		break;
	case 0x56:
		AbsoluteY ();
		APURegisters.YA.B.A ^= ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 3;
		break;
	case 0x57:
		IndirectIndexedY ();
		APURegisters.YA.B.A ^= ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0x58:
		Work8 = OP1;
		Work8 ^= ReadByteZ (op2 = OP2);
		WriteByteZ (op2, Work8);
		APUSetZN8 (Work8);
		address += 3;
		break;
	case 0x59:
		Work8 = ReadByteZ (APURegisters.X) ^ ReadByteZ (APURegisters.YA.B.Y);
		APUSetZN8 (Work8);
		WriteByteZ (APURegisters.X, Work8);
		address++;
		break;

#define LSR(b)\
    IAPU._Carry = (b) & 1;\
    (b) >>= 1;\
    APUSetZN8 (b);

	case 0x4B:
		Work8 = ReadByteZ (op1 = OP1);
		LSR (Work8);
		WriteByteZ (op1, Work8);
		address += 2;
		break;
	case 0x4C:
		Absolute ();
		Work8 = ReadByte (IAPU.Address);
		LSR (Work8);
		WriteByte (IAPU.Address, Work8);
		address += 3;
		break;
	case 0x5B:
		Work8 = ReadByteZ ((op1 = OP1) + APURegisters.X);
		LSR (Work8);
		WriteByteZ (op1 + APURegisters.X, Work8);
		address += 2;
		break;
	case 0x5C:
		LSR (APURegisters.YA.B.A);
		address++;
		break;
	case 0x7D:
		APURegisters.YA.B.A = APURegisters.X;
		APUSetZN8 (APURegisters.YA.B.A);
		address++;
		break;
	case 0xDD:
		APURegisters.YA.B.A = APURegisters.YA.B.Y;
		APUSetZN8 (APURegisters.YA.B.A);
		address++;
		break;
	case 0x5D:
		APURegisters.X = APURegisters.YA.B.A;
		APUSetZN8 (APURegisters.X);
		address++;
		break;
	case 0xFD:
		APURegisters.YA.B.Y = APURegisters.YA.B.A;
		APUSetZN8 (APURegisters.YA.B.Y);
		address++;
		break;
	case 0x9D:
		APURegisters.X = APURegisters.S;
		APUSetZN8 (APURegisters.X);
		address++;
		break;
	case 0xBD:
		APURegisters.S = APURegisters.X;
		address++;
		break;

#define ROR(b)\
    Work16 = (b) | ((uint16) APUCheckCarry () << 8); \
    IAPU._Carry = (uint8) Work16 & 1; \
    Work16 >>= 1; \
    (b) = (uint8) Work16; \
    APUSetZN8 (b);

	case 0x6B:
		Work8 = ReadByteZ (op1 = OP1);
		ROR (Work8);
		WriteByteZ (op1, Work8);
		address += 2;
		break;
	case 0x6C:
		Absolute ();
		Work8 = ReadByte (IAPU.Address);
		ROR (Work8);
		WriteByte (IAPU.Address, Work8);
		address += 3;
		break;
	case 0x7B:
		Work8 = ReadByteZ ((op1 = OP1) + APURegisters.X);
		ROR (Work8);
		WriteByteZ (op1 + APURegisters.X, Work8);
		address += 2;
		break;
	case 0x7C:
		ROR (APURegisters.YA.B.A);
		address++;
		break;
	case 0x6E:
		Work8 = OP1;
		W1 = ReadByteZ (Work8) - 1;
		WriteByteZ (Work8, W1);
		if (W1 != 0)
		{
			Relative2 ();
			address = (uint16) Int16;
			cycles += 2;
		}
		else
			address += 3;
		break;
	case 0xFE:
		APURegisters.YA.B.Y--;
		if (APURegisters.YA.B.Y != 0)
		{
			Relative ();
			address = (uint16) Int16;
			cycles += 2;
		}
		else
			address += 2;
		break;
	case 0x6F:
		PopW (APURegisters.PC);
		address = APURegisters.PC;
		break;
	case 0x7F:
		Pop (APURegisters.P);
		UnpackStatus ();
		PopW (APURegisters.PC);
		address = APURegisters.PC;
		break;

#define ADC(a,b)\
Work16 = (a) + (b) + APUCheckCarry();\
IAPU._Carry = Work16 >= 0x100; \
if (~((a) ^ (b)) & ((b) ^ (uint8) Work16) & 0x80)\
    APUSetOverflow ();\
else \
    APUClearOverflow (); \
APUClearHalfCarry ();\
if(((a) ^ (b) ^ (uint8) Work16) & 0x10)\
    APUSetHalfCarry ();\
(a) = (uint8) Work16;\
APUSetZN8 ((uint8) Work16);

	case 0x84:
		Work8 = ReadByteZ (OP1);
		ADC (APURegisters.YA.B.A, Work8);
		address += 2;
		break;
	case 0x85:
		Absolute ();
		Work8 = ReadByte (IAPU.Address);
		ADC (APURegisters.YA.B.A, Work8);
		address += 3;
		break;
	case 0x86:
		Work8 = ReadByteZ (APURegisters.X);
		ADC (APURegisters.YA.B.A, Work8);
		address++;
		break;
	case 0x87:
		IndexedXIndirect ();
		Work8 = ReadByte (IAPU.Address);
		ADC (APURegisters.YA.B.A, Work8);
		address += 2;
		break;
	case 0x88:
		Work8 = OP1;
		ADC (APURegisters.YA.B.A, Work8);
		address += 2;
		break;
	case 0x89:
		Work8 = ReadByteZ (OP1);
		W1 = ReadByteZ (op2 = OP2);
		ADC (W1, Work8);
		WriteByteZ (op2, W1);
		address += 3;
		break;
	case 0x94:
		Work8 = ReadByteZ (OP1 + APURegisters.X);
		ADC (APURegisters.YA.B.A, Work8);
		address += 2;
		break;
	case 0x95:
		AbsoluteX ();
		Work8 = ReadByte (IAPU.Address);
		ADC (APURegisters.YA.B.A, Work8);
		address += 3;
		break;
	case 0x96:
		AbsoluteY ();
		Work8 = ReadByte (IAPU.Address);
		ADC (APURegisters.YA.B.A, Work8);
		address += 3;
		break;
	case 0x97:
		IndirectIndexedY ();
		Work8 = ReadByte (IAPU.Address);
		ADC (APURegisters.YA.B.A, Work8);
		address += 2;
		break;
	case 0x98:
		Work8 = OP1;
		W1 = ReadByteZ (op2 = OP2);
		ADC (W1, Work8);
		WriteByteZ (op2, W1);
		address += 3;
		break;
	case 0x99:
		W1 = ReadByteZ (APURegisters.X);
		Work8 = ReadByteZ (APURegisters.YA.B.Y);
		ADC (W1, Work8);
		WriteByteZ (APURegisters.X, W1);
		address++;
		break;
	case 0x8D:
		APURegisters.YA.B.Y = OP1;
		APUSetZN8 (APURegisters.YA.B.Y);
		address += 2;
		break;
	case 0x8F:
		Work8 = OP1;
		WriteByteZ (OP2, Work8);
		address += 3;
		break;
	case 0x9E:
		if (APURegisters.X == 0)
		{
		  overflow:
			APUSetOverflow ();
			APURegisters.YA.W = 0xffff;
		}
		else
		{
			Work16 = APURegisters.YA.W / ((uint16) APURegisters.X);
			if (Work16 >= 0x100)
				goto overflow;
			else
				APUClearOverflow ();
			APURegisters.YA.B.Y =
				(uint8) (APURegisters.YA.W % ((uint16) APURegisters.X));
			APURegisters.YA.B.A = (uint8) Work16;
		}
		APUSetZN8 (APURegisters.YA.B.A);
		address++;
		break;
	case 0x9F:
		APURegisters.YA.B.A =
			(APURegisters.YA.B.A >> 4) | (APURegisters.YA.B.A << 4);
		APUSetZN8 (APURegisters.YA.B.A);
		address++;
		break;

#define SBC(a,b)\
Int16 = (short) (a) - (short) (b) + (short) (APUCheckCarry ()) - 1;\
IAPU._Carry = Int16 >= 0;\
if ((((a) ^ (b)) & 0x80) && (((a) ^ (uint8) Int16) & 0x80))\
    APUSetOverflow ();\
else \
    APUClearOverflow (); \
APUSetHalfCarry ();\
if(((a) ^ (b) ^ (uint8) Int16) & 0x10)\
    APUClearHalfCarry ();\
(a) = (uint8) Int16;\
APUSetZN8 ((uint8) Int16);

	case 0xA4:
		Work8 = ReadByteZ (OP1);
		SBC (APURegisters.YA.B.A, Work8);
		address += 2;
		break;
	case 0xA5:
		Absolute ();
		Work8 = ReadByte (IAPU.Address);
		SBC (APURegisters.YA.B.A, Work8);
		address += 3;
		break;
	case 0xA6:
		Work8 = ReadByteZ (APURegisters.X);
		SBC (APURegisters.YA.B.A, Work8);
		address++;
		break;
	case 0xA7:
		IndexedXIndirect ();
		Work8 = ReadByte (IAPU.Address);
		SBC (APURegisters.YA.B.A, Work8);
		address += 2;
		break;
	case 0xA8:
		Work8 = OP1;
		SBC (APURegisters.YA.B.A, Work8);
		address += 2;
		break;
	case 0xA9:
		Work8 = ReadByteZ (OP1);
		W1 = ReadByteZ (op2 = OP2);
		SBC (W1, Work8);
		WriteByteZ (op2, W1);
		address += 3;
		break;
	case 0xB4:
		Work8 = ReadByteZ (OP1 + APURegisters.X);
		SBC (APURegisters.YA.B.A, Work8);
		address += 2;
		break;
	case 0xB5:
		AbsoluteX ();
		Work8 = ReadByte (IAPU.Address);
		SBC (APURegisters.YA.B.A, Work8);
		address += 3;
		break;
	case 0xB6:
		AbsoluteY ();
		Work8 = ReadByte (IAPU.Address);
		SBC (APURegisters.YA.B.A, Work8);
		address += 3;
		break;
	case 0xB7:
		IndirectIndexedY ();
		Work8 = ReadByte (IAPU.Address);
		SBC (APURegisters.YA.B.A, Work8);
		address += 2;
		break;
	case 0xB8:
		Work8 = OP1;
		W1 = ReadByteZ (op2 = OP2);
		SBC (W1, Work8);
		WriteByteZ (op2, W1);
		address += 3;
		break;
	case 0xB9:
		W1 = ReadByteZ (APURegisters.X);
		Work8 = ReadByteZ (APURegisters.YA.B.Y);
		SBC (W1, Work8);
		WriteByteZ (APURegisters.X, W1);
		address++;
		break;
	case 0xAF:
		WriteByteZ (APURegisters.X++, APURegisters.YA.B.A);
		address++;
		break;
	case 0xBE:
		if ((APURegisters.YA.B.A & 0x0f) > 9 || !APUCheckHalfCarry ())
		{
			APURegisters.YA.B.A -= 6;
		}
		if (APURegisters.YA.B.A > 0x9f || !IAPU._Carry)
		{
			APURegisters.YA.B.A -= 0x60;
			APUClearCarry ();
		}
		else
		{
			APUSetCarry ();
		}
		APUSetZN8 (APURegisters.YA.B.A);
		address++;
		break;
	case 0xBF:
		APURegisters.YA.B.A = ReadByteZ (APURegisters.X++);
		APUSetZN8 (APURegisters.YA.B.A);
		address++;
		break;
	case 0xC0:
		APUClearInterrupt ();
		address++;
		break;
	case 0xA0:
		APUSetInterrupt ();
		address++;
		break;
	case 0xC4:
		WriteByteZ (OP1, APURegisters.YA.B.A);
		address += 2;
		break;
	case 0xC5:
		Absolute ();
		WriteByte (IAPU.Address, APURegisters.YA.B.A);
		address += 3;
		break;
	case 0xC6:
		WriteByteZ (APURegisters.X, APURegisters.YA.B.A);
		address++;
		break;
	case 0xC7:
		IndexedXIndirect ();
		WriteByte (IAPU.Address, APURegisters.YA.B.A);
		address += 2;
		break;
	case 0xC9:
		Absolute ();
		WriteByte (IAPU.Address, APURegisters.X);
		address += 3;
		break;
	case 0xCB:
		WriteByteZ (OP1, APURegisters.YA.B.Y);
		address += 2;
		break;
	case 0xCC:
		Absolute ();
		WriteByte (IAPU.Address, APURegisters.YA.B.Y);
		address += 3;
		break;
	case 0xCD:
		APURegisters.X = OP1;
		APUSetZN8 (APURegisters.X);
		address += 2;
		break;
	case 0xCF:
		APURegisters.YA.W =
			(uint16) APURegisters.YA.B.A * (uint16) APURegisters.YA.B.Y;
		APUSetZN16 (APURegisters.YA.W);
		address++;
		break;
	case 0xD4:
		WriteByteZ (OP1 + APURegisters.X, APURegisters.YA.B.A);
		address += 2;
		break;
	case 0xD5:
		AbsoluteX ();
		WriteByte (IAPU.Address, APURegisters.YA.B.A);
		address += 3;
		break;
	case 0xD6:
		AbsoluteY ();
		WriteByte (IAPU.Address, APURegisters.YA.B.A);
		address += 3;
		break;
	case 0xD7:
		IndirectIndexedY ();
		WriteByte (IAPU.Address, APURegisters.YA.B.A);
		address += 2;
		break;
	case 0xD8:
		WriteByteZ (OP1, APURegisters.X);
		address += 2;
		break;
	case 0xD9:
		WriteByteZ (OP1 + APURegisters.YA.B.Y, APURegisters.X);
		address += 2;
		break;
	case 0xDB:
		WriteByteZ (OP1 + APURegisters.X, APURegisters.YA.B.Y);
		address += 2;
		break;
	case 0xDF:
		if ((APURegisters.YA.B.A & 0x0f) > 9 || APUCheckHalfCarry ())
		{
			if (APURegisters.YA.B.A > 0xf0)
				APUSetCarry ();
			APURegisters.YA.B.A += 6;
			//APUSetHalfCarry (); Intel procs do this, but this is a Sony proc...
		}
		//else { APUClearHalfCarry (); } ditto as above
		if (APURegisters.YA.B.A > 0x9f || IAPU._Carry)
		{
			APURegisters.YA.B.A += 0x60;
			APUSetCarry ();
		}
		else
		{
			APUClearCarry ();
		}
		APUSetZN8 (APURegisters.YA.B.A);
		address++;
		break;
	case 0xE4:
		APURegisters.YA.B.A = ReadByteZ (OP1);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0xE5:
		Absolute ();
		APURegisters.YA.B.A = ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 3;
		break;
	case 0xE6:
		APURegisters.YA.B.A = ReadByteZ (APURegisters.X);
		APUSetZN8 (APURegisters.YA.B.A);
		address++;
		break;
	case 0xE7:
		IndexedXIndirect ();
		APURegisters.YA.B.A = ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0xE8:
		APURegisters.YA.B.A = OP1;
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0xE9:
		Absolute ();
		APURegisters.X = ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.X);
		address += 3;
		break;
	case 0xEB:
		APURegisters.YA.B.Y = ReadByteZ (OP1);
		APUSetZN8 (APURegisters.YA.B.Y);
		address += 2;
		break;
	case 0xEC:
		Absolute ();
		APURegisters.YA.B.Y = ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.Y);
		address += 3;
		break;
	case 0xF4:
		APURegisters.YA.B.A = ReadByteZ (OP1 + APURegisters.X);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0xF5:
		AbsoluteX ();
		APURegisters.YA.B.A = ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 3;
		break;
	case 0xF6:
		AbsoluteY ();
		APURegisters.YA.B.A = ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 3;
		break;
	case 0xF7:
		IndirectIndexedY ();
		APURegisters.YA.B.A = ReadByte (IAPU.Address);
		APUSetZN8 (APURegisters.YA.B.A);
		address += 2;
		break;
	case 0xF8:
		APURegisters.X = ReadByteZ (OP1);
		APUSetZN8 (APURegisters.X);
		address += 2;
		break;
	case 0xF9:
		APURegisters.X = ReadByteZ (OP1 + APURegisters.YA.B.Y);
		APUSetZN8 (APURegisters.X);
		address += 2;
		break;
	case 0xFA:
		WriteByteZ (OP2, ReadByteZ (OP1));
		address += 3;
		break;
	case 0xFB:
		APURegisters.YA.B.Y = ReadByteZ (OP1 + APURegisters.X);
		APUSetZN8 (APURegisters.YA.B.Y);
		address += 2;
		break;
	}
	APURegisters.PC = address;
}

spc::spc(void * pSPC_RAM, void * pDSPregs)
{
	SPC_RAM = (unsigned char *) pSPC_RAM;
	DSPregs = (unsigned char *) pDSPregs;

	cycles = 0;
	memset(&IAPU, 0, sizeof(IAPU));
	memset(&APURegisters, 0, sizeof(APURegisters));

	reset();
}
