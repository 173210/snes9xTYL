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
#ifndef _CPUMACRO_H_
#define _CPUMACRO_H_

STATIC inline void SetZN16 (uint16 Work)
{
    CPUPack.ICPU._Zero = Work != 0;
    CPUPack.ICPU._Negative = (uint8) (Work >> 8);
}

STATIC inline void SetZN8 (uint8 Work)
{
    CPUPack.ICPU._Zero = Work;
    CPUPack.ICPU._Negative = Work;
}

STATIC inline void ADC8 (long OpAddress)
{
    uint8 Work8 = S9xGetByte (OpAddress);
    
    if (CheckDecimal ())
    {
	uint8 A1 = (CPUPack.Registers.A.W) & 0xF;
	uint8 A2 = (CPUPack.Registers.A.W >> 4) & 0xF;
	uint8 W1 = Work8 & 0xF;
	uint8 W2 = (Work8 >> 4) & 0xF;

	A1 += W1 + CheckCarry();
	if (A1 > 9)
	{
	    A1 -= 10;
	    A2++;
	}

	A2 += W2;
	if (A2 > 9)
	{
	    A2 -= 10;
	    SetCarry ();
	}
	else
	{
	    ClearCarry ();
	}

	uint8 Ans8 = (A2 << 4) | A1;
	if (~(CPUPack.Registers.AL ^ Work8) &
	    (Work8 ^ Ans8) & 0x80)
	    SetOverflow();
	else
	    ClearOverflow();
	CPUPack.Registers.AL = Ans8;
	SetZN8 (CPUPack.Registers.AL);
    }
    else
    {
	uint16 Ans16 = CPUPack.Registers.AL + Work8 + CheckCarry();

	CPUPack.ICPU._Carry = Ans16 >= 0x100;

	if (~(CPUPack.Registers.AL ^ Work8) & 
	     (Work8 ^ (uint8) Ans16) & 0x80)
	    SetOverflow();
	else
	    ClearOverflow();
	CPUPack.Registers.AL = (uint8) Ans16;
	SetZN8 (CPUPack.Registers.AL);

    }
}

STATIC inline void ADC16 (long OpAddress)
{
    uint16 Work16 = S9xGetWord (OpAddress);

    if (CheckDecimal ())
    {
	uint8 A1 = (CPUPack.Registers.A.W) & 0xF;
	uint8 A2 = (CPUPack.Registers.A.W >> 4) & 0xF;
	uint8 A3 = (CPUPack.Registers.A.W >> 8) & 0xF;
	uint8 A4 = (CPUPack.Registers.A.W >> 12) & 0xF;
	uint8 W1 = Work16 & 0xF;
	uint8 W2 = (Work16 >> 4) & 0xF;
	uint8 W3 = (Work16 >> 8) & 0xF;
	uint8 W4 = (Work16 >> 12) & 0xF;

	A1 += W1 + CheckCarry ();
	if (A1 > 9)
	{
	    A1 -= 10;
	    A2++;
	}

	A2 += W2;
	if (A2 > 9)
	{
	    A2 -= 10;
	    A3++;
	}

	A3 += W3;
	if (A3 > 9)
	{
	    A3 -= 10;
	    A4++;
	}

	A4 += W4;
	if (A4 > 9)
	{
	    A4 -= 10;
	    SetCarry ();
	}
	else
	{
	    ClearCarry ();
	}

	uint16 Ans16 = (A4 << 12) | (A3 << 8) | (A2 << 4) | (A1);
	if (~(CPUPack.Registers.A.W ^ Work16) &
	    (Work16 ^ Ans16) & 0x8000)
	    SetOverflow();
	else
	    ClearOverflow();
	CPUPack.Registers.A.W = Ans16;
	SetZN16 (CPUPack.Registers.A.W);
    }
    else
    {
	uint32 Ans32 = CPUPack.Registers.A.W + Work16 + CheckCarry();

	CPUPack.ICPU._Carry = Ans32 >= 0x10000;

	if (~(CPUPack.Registers.A.W ^ Work16) &
	    (Work16 ^ (uint16) Ans32) & 0x8000)
	    SetOverflow();
	else
	    ClearOverflow();
	CPUPack.Registers.A.W = (uint16) Ans32;
	SetZN16 (CPUPack.Registers.A.W);
    }
}

STATIC inline void AND16 (long OpAddress)
{
    CPUPack.Registers.A.W &= S9xGetWord (OpAddress);
    SetZN16 (CPUPack.Registers.A.W);
}

STATIC inline void AND8 (long OpAddress)
{
    CPUPack.Registers.AL &= S9xGetByte (OpAddress);
    SetZN8 (CPUPack.Registers.AL);
}

STATIC inline void A_ASL16 ()
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    CPUPack.ICPU._Carry = (CPUPack.Registers.AH & 0x80) != 0;
    CPUPack.Registers.A.W <<= 1;
    SetZN16 (CPUPack.Registers.A.W);
}

STATIC inline void A_ASL8 ()
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    CPUPack.ICPU._Carry = (CPUPack.Registers.AL & 0x80) != 0;
    CPUPack.Registers.AL <<= 1;
    SetZN8 (CPUPack.Registers.AL);
}

STATIC inline void ASL16 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetWord (OpAddress);
    CPUPack.ICPU._Carry = (Work16 & 0x8000) != 0;
    Work16 <<= 1;
    S9xSetWord (Work16, OpAddress);
    SetZN16 (Work16);
}

STATIC inline void ASL8 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    uint8 Work8 = S9xGetByte (OpAddress);
    CPUPack.ICPU._Carry = (Work8 & 0x80) != 0;
    Work8 <<= 1;
    S9xSetByte (Work8, OpAddress);
    SetZN8 (Work8);
}

STATIC inline void BIT16 (long OpAddress)
{
    uint16 Work16 = S9xGetWord (OpAddress);
    CPUPack.ICPU._Overflow = (Work16 & 0x4000) != 0;
    CPUPack.ICPU._Negative = (uint8) (Work16 >> 8);
    CPUPack.ICPU._Zero = (Work16 & CPUPack.Registers.A.W) != 0;
}

STATIC inline void BIT8 (long OpAddress)
{
    uint8 Work8 = S9xGetByte (OpAddress);
    CPUPack.ICPU._Overflow = (Work8 & 0x40) != 0;
    CPUPack.ICPU._Negative = Work8;
    CPUPack.ICPU._Zero = Work8 & CPUPack.Registers.AL;
}

STATIC inline void CMP16 (long OpAddress)
{
    long s9xInt32 = (long) CPUPack.Registers.A.W -
	    (long) S9xGetWord (OpAddress);
    CPUPack.ICPU._Carry = s9xInt32 >= 0;
    SetZN16 ((uint16) s9xInt32);
}

STATIC inline void CMP8 (long OpAddress)
{
    short s9xInt16 = (short) CPUPack.Registers.AL -
	    (short) S9xGetByte (OpAddress);
    CPUPack.ICPU._Carry = s9xInt16 >= 0;
    SetZN8 ((uint8) s9xInt16);
}

STATIC inline void CMX16 (long OpAddress)
{
    long s9xInt32 = (long) CPUPack.Registers.X.W -
	    (long) S9xGetWord (OpAddress);
    CPUPack.ICPU._Carry = s9xInt32 >= 0;
    SetZN16 ((uint16) s9xInt32);
}

STATIC inline void CMX8 (long OpAddress)
{
    short s9xInt16 = (short) CPUPack.Registers.XL -
	    (short) S9xGetByte (OpAddress);
    CPUPack.ICPU._Carry = s9xInt16 >= 0;
    SetZN8 ((uint8) s9xInt16);
}

STATIC inline void CMY16 (long OpAddress)
{
    long s9xInt32 = (long) CPUPack.Registers.Y.W -
	    (long) S9xGetWord (OpAddress);
    CPUPack.ICPU._Carry = s9xInt32 >= 0;
    SetZN16 ((uint16) s9xInt32);
}

STATIC inline void CMY8 (long OpAddress)
{
    short s9xInt16 = (short) CPUPack.Registers.YL -
	    (short) S9xGetByte (OpAddress);
    CPUPack.ICPU._Carry = s9xInt16 >= 0;
    SetZN8 ((uint8) s9xInt16);
}

STATIC inline void A_DEC16 ()
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPUPack.CPU.WaitAddress = NULL;
#endif

    CPUPack.Registers.A.W--;
    SetZN16 (CPUPack.Registers.A.W);
}

STATIC inline void A_DEC8 ()
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPUPack.CPU.WaitAddress = NULL;
#endif

    CPUPack.Registers.AL--;
    SetZN8 (CPUPack.Registers.AL);
}

STATIC inline void DEC16 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPUPack.CPU.WaitAddress = NULL;
#endif

    uint16 Work16 = S9xGetWord (OpAddress) - 1;
    S9xSetWord (Work16, OpAddress);
    SetZN16 (Work16);
}

STATIC inline void DEC8 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPUPack.CPU.WaitAddress = NULL;
#endif

    uint8 Work8 = S9xGetByte (OpAddress) - 1;
    S9xSetByte (Work8, OpAddress);
    SetZN8 (Work8);
}

STATIC inline void EOR16 (long OpAddress)
{
    CPUPack.Registers.A.W ^= S9xGetWord (OpAddress);
    SetZN16 (CPUPack.Registers.A.W);
}

STATIC inline void EOR8 (long OpAddress)
{
    CPUPack.Registers.AL ^= S9xGetByte (OpAddress);
    SetZN8 (CPUPack.Registers.AL);
}

STATIC inline void A_INC16 ()
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPUPack.CPU.WaitAddress = NULL;
#endif

    CPUPack.Registers.A.W++;
    SetZN16 (CPUPack.Registers.A.W);
}

STATIC inline void A_INC8 ()
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPUPack.CPU.WaitAddress = NULL;
#endif

    CPUPack.Registers.AL++;
    SetZN8 (CPUPack.Registers.AL);
}

STATIC inline void INC16 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPUPack.CPU.WaitAddress = NULL;
#endif

    uint16 Work16 = S9xGetWord (OpAddress) + 1;
    S9xSetWord (Work16, OpAddress);
    SetZN16 (Work16);
}

STATIC inline void INC8 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPUPack.CPU.WaitAddress = NULL;
#endif

    uint8 Work8 = S9xGetByte (OpAddress) + 1;
    S9xSetByte (Work8, OpAddress);
    SetZN8 (Work8);
}

STATIC inline void LDA16 (long OpAddress)
{
    CPUPack.Registers.A.W = S9xGetWord (OpAddress);
    SetZN16 (CPUPack.Registers.A.W);
}

STATIC inline void LDA8 (long OpAddress)
{
    CPUPack.Registers.AL = S9xGetByte (OpAddress);
    SetZN8 (CPUPack.Registers.AL);
}

STATIC inline void LDX16 (long OpAddress)
{
    CPUPack.Registers.X.W = S9xGetWord (OpAddress);
    SetZN16 (CPUPack.Registers.X.W);
}

STATIC inline void LDX8 (long OpAddress)
{
    CPUPack.Registers.XL = S9xGetByte (OpAddress);
    SetZN8 (CPUPack.Registers.XL);
}

STATIC inline void LDY16 (long OpAddress)
{
    CPUPack.Registers.Y.W = S9xGetWord (OpAddress);
    SetZN16 (CPUPack.Registers.Y.W);
}

STATIC inline void LDY8 (long OpAddress)
{
    CPUPack.Registers.YL = S9xGetByte (OpAddress);
    SetZN8 (CPUPack.Registers.YL);
}

STATIC inline void A_LSR16 ()
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    CPUPack.ICPU._Carry = CPUPack.Registers.AL & 1;
    CPUPack.Registers.A.W >>= 1;
    SetZN16 (CPUPack.Registers.A.W);
}

STATIC inline void A_LSR8 ()
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    CPUPack.ICPU._Carry = CPUPack.Registers.AL & 1;
    CPUPack.Registers.AL >>= 1;
    SetZN8 (CPUPack.Registers.AL);
}

STATIC inline void LSR16 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetWord (OpAddress);
    CPUPack.ICPU._Carry = Work16 & 1;
    Work16 >>= 1;
    S9xSetWord (Work16, OpAddress);
    SetZN16 (Work16);
}

STATIC inline void LSR8 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    uint8 Work8 = S9xGetByte (OpAddress);
    CPUPack.ICPU._Carry = Work8 & 1;
    Work8 >>= 1;
    S9xSetByte (Work8, OpAddress);
    SetZN8 (Work8);
}

STATIC inline void ORA16 (long OpAddress)
{
    CPUPack.Registers.A.W |= S9xGetWord (OpAddress);
    SetZN16 (CPUPack.Registers.A.W);
}

STATIC inline void ORA8 (long OpAddress)
{
    CPUPack.Registers.AL |= S9xGetByte (OpAddress);
    SetZN8 (CPUPack.Registers.AL);
}

STATIC inline void A_ROL16 ()
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    uint32 Work32 = (CPUPack.Registers.A.W << 1) | CheckCarry();
    CPUPack.ICPU._Carry = Work32 >= 0x10000;
    CPUPack.Registers.A.W = (uint16) Work32;
    SetZN16 ((uint16) Work32);
}

STATIC inline void A_ROL8 ()
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = CPUPack.Registers.AL;
    Work16 <<= 1;
    Work16 |= CheckCarry();
    CPUPack.ICPU._Carry = Work16 >= 0x100;
    CPUPack.Registers.AL = (uint8) Work16;
    SetZN8 ((uint8) Work16);
}

STATIC inline void ROL16 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    uint32 Work32 = S9xGetWord (OpAddress);
    Work32 <<= 1;
    Work32 |= CheckCarry();
    CPUPack.ICPU._Carry = Work32 >= 0x10000;
    S9xSetWord ((uint16) Work32, OpAddress);
    SetZN16 ((uint16) Work32);
}

STATIC inline void ROL8 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetByte (OpAddress);
    Work16 <<= 1;
    Work16 |= CheckCarry ();
    CPUPack.ICPU._Carry = Work16 >= 0x100;
    S9xSetByte ((uint8) Work16, OpAddress);
    SetZN8 ((uint8) Work16);
}

STATIC inline void A_ROR16 ()
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    uint32 Work32 = CPUPack.Registers.A.W;
    Work32 |= (int) CheckCarry() << 16;
    CPUPack.ICPU._Carry = (uint8) (Work32 & 1);
    Work32 >>= 1;
    CPUPack.Registers.A.W = (uint16) Work32;
    SetZN16 ((uint16) Work32);
}

STATIC inline void A_ROR8 ()
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = CPUPack.Registers.AL | ((uint16) CheckCarry() << 8);
    CPUPack.ICPU._Carry = (uint8) Work16 & 1;
    Work16 >>= 1;
    CPUPack.Registers.AL = (uint8) Work16;
    SetZN8 ((uint8) Work16);
}

STATIC inline void ROR16 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    uint32 Work32 = S9xGetWord (OpAddress);
    Work32 |= (int) CheckCarry() << 16;
    CPUPack.ICPU._Carry = (uint8) (Work32 & 1);
    Work32 >>= 1;
    S9xSetWord ((uint16) Work32, OpAddress);
    SetZN16 ((uint16) Work32);
}

STATIC inline void ROR8 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetByte (OpAddress);
    Work16 |= (int) CheckCarry () << 8;
    CPUPack.ICPU._Carry = (uint8) (Work16 & 1);
    Work16 >>= 1;
    S9xSetByte ((uint8) Work16, OpAddress);
    SetZN8 ((uint8) Work16);
}

STATIC inline void SBC16 (long OpAddress)
{
    uint16 Work16 = S9xGetWord (OpAddress);

    if (CheckDecimal ())
    {
	uint8 A1 = (CPUPack.Registers.A.W) & 0xF;
	uint8 A2 = (CPUPack.Registers.A.W >> 4) & 0xF;
	uint8 A3 = (CPUPack.Registers.A.W >> 8) & 0xF;
	uint8 A4 = (CPUPack.Registers.A.W >> 12) & 0xF;
	uint8 W1 = Work16 & 0xF;
	uint8 W2 = (Work16 >> 4) & 0xF;
	uint8 W3 = (Work16 >> 8) & 0xF;
	uint8 W4 = (Work16 >> 12) & 0xF;

	A1 -= W1 + !CheckCarry ();
	A2 -= W2;
	A3 -= W3;
	A4 -= W4;
	if (A1 > 9)
	{
	    A1 += 10;
	    A2--;
	}
	if (A2 > 9)
	{
	    A2 += 10;
	    A3--;
	}
	if (A3 > 9)
	{
	    A3 += 10;
	    A4--;
	}
	if (A4 > 9)
	{
	    A4 += 10;
	    ClearCarry ();
	}
	else
	{
	    SetCarry ();
	}

	uint16 Ans16 = (A4 << 12) | (A3 << 8) | (A2 << 4) | (A1);
	if ((CPUPack.Registers.A.W ^ Work16) &
	    (CPUPack.Registers.A.W ^ Ans16) & 0x8000)
	    SetOverflow();
	else
	    ClearOverflow();
	CPUPack.Registers.A.W = Ans16;
	SetZN16 (CPUPack.Registers.A.W);
    }
    else
    {

	long s9xInt32 = (long) CPUPack.Registers.A.W - (long) Work16 + (long) CheckCarry() - 1;

	CPUPack.ICPU._Carry = s9xInt32 >= 0;

	if ((CPUPack.Registers.A.W ^ Work16) &
	    (CPUPack.Registers.A.W ^ (uint16) s9xInt32) & 0x8000)
	    SetOverflow();
	else
	    ClearOverflow ();
	CPUPack.Registers.A.W = (uint16) s9xInt32;
	SetZN16 (CPUPack.Registers.A.W);
    }
}

STATIC inline void SBC8 (long OpAddress)
{
    uint8 Work8 = S9xGetByte (OpAddress);
    if (CheckDecimal ())
    {
	uint8 A1 = (CPUPack.Registers.A.W) & 0xF;
	uint8 A2 = (CPUPack.Registers.A.W >> 4) & 0xF;
	uint8 W1 = Work8 & 0xF;
	uint8 W2 = (Work8 >> 4) & 0xF;

	A1 -= W1 + !CheckCarry ();
	A2 -= W2;
	if (A1 > 9)
	{
	    A1 += 10;
	    A2--;
	}
	if (A2 > 9)
	{
	    A2 += 10;
	    ClearCarry ();
	}
	else
	{
	    SetCarry ();
	}

	uint8 Ans8 = (A2 << 4) | A1;
	if ((CPUPack.Registers.AL ^ Work8) &
	    (CPUPack.Registers.AL ^ Ans8) & 0x80)
	    SetOverflow ();
	else
	    ClearOverflow ();
	CPUPack.Registers.AL = Ans8;
	SetZN8 (CPUPack.Registers.AL);
    }
    else
    {
	short s9xInt16 = (short) CPUPack.Registers.AL - (short) Work8 + (short) CheckCarry() - 1;

	CPUPack.ICPU._Carry = s9xInt16 >= 0;
	if ((CPUPack.Registers.AL ^ Work8) &
	    (CPUPack.Registers.AL ^ (uint8) s9xInt16) & 0x80)
	    SetOverflow ();
	else
	    ClearOverflow ();
	CPUPack.Registers.AL = (uint8) s9xInt16;
	SetZN8 (CPUPack.Registers.AL);
    }
}

STATIC inline void STA16 (long OpAddress)
{
    S9xSetWord (CPUPack.Registers.A.W, OpAddress);
}

STATIC inline void STA8 (long OpAddress)
{
    S9xSetByte (CPUPack.Registers.AL, OpAddress);
}

STATIC inline void STX16 (long OpAddress)
{
    S9xSetWord (CPUPack.Registers.X.W, OpAddress);
}

STATIC inline void STX8 (long OpAddress)
{
    S9xSetByte (CPUPack.Registers.XL, OpAddress);
}

STATIC inline void STY16 (long OpAddress)
{
    S9xSetWord (CPUPack.Registers.Y.W, OpAddress);
}

STATIC inline void STY8 (long OpAddress)
{
    S9xSetByte (CPUPack.Registers.YL, OpAddress);
}

STATIC inline void STZ16 (long OpAddress)
{
    S9xSetWord (0, OpAddress);
}

STATIC inline void STZ8 (long OpAddress)
{
    S9xSetByte (0, OpAddress);
}

STATIC inline void TSB16 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetWord (OpAddress);
    CPUPack.ICPU._Zero = (Work16 & CPUPack.Registers.A.W) != 0;
    Work16 |= CPUPack.Registers.A.W;
    S9xSetWord (Work16, OpAddress);
}

STATIC inline void TSB8 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    uint8 Work8 = S9xGetByte (OpAddress);
    CPUPack.ICPU._Zero = Work8 & CPUPack.Registers.AL;
    Work8 |= CPUPack.Registers.AL;
    S9xSetByte (Work8, OpAddress);
}

STATIC inline void TRB16 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    uint16 Work16 = S9xGetWord (OpAddress);
    CPUPack.ICPU._Zero = (Work16 & CPUPack.Registers.A.W) != 0;
    Work16 &= ~CPUPack.Registers.A.W;
    S9xSetWord (Work16, OpAddress);
}

STATIC inline void TRB8 (long OpAddress)
{
#ifdef VAR_CYCLES
    CPUPack.CPU.Cycles += ONE_CYCLE;
#endif
    uint8 Work8 = S9xGetByte (OpAddress);
    CPUPack.ICPU._Zero = Work8 & CPUPack.Registers.AL;
    Work8 &= ~CPUPack.Registers.AL;
    S9xSetByte (Work8, OpAddress);
}





/*

STATIC inline void ADC8D0 (long OpAddress)
{
    uint8 Work8 = S9xGetByte (OpAddress);
    
    {
	uint16 Ans16 = CPUPack.Registers.AL + Work8 + CheckCarry();

	CPUPack.ICPU._Carry = Ans16 >= 0x100;

	if (~(CPUPack.Registers.AL ^ Work8) & 
	     (Work8 ^ (uint8) Ans16) & 0x80)
	    SetOverflow();
	else
	    ClearOverflow();
	CPUPack.Registers.AL = (uint8) Ans16;
	SetZN8 (CPUPack.Registers.AL);

    }
}

STATIC inline void ADC8D1 (long OpAddress)
{
    uint8 Work8 = S9xGetByte (OpAddress);
    
    {
	uint8 A1 = (CPUPack.Registers.A.W) & 0xF;
	uint8 A2 = (CPUPack.Registers.A.W >> 4) & 0xF;
	uint8 W1 = Work8 & 0xF;
	uint8 W2 = (Work8 >> 4) & 0xF;

	A1 += W1 + CheckCarry();
	if (A1 > 9)
	{
	    A1 -= 10;
	    A2++;
	}

	A2 += W2;
	if (A2 > 9)
	{
	    A2 -= 10;
	    SetCarry ();
	}
	else
	{
	    ClearCarry ();
	}

	uint8 Ans8 = (A2 << 4) | A1;
	if (~(CPUPack.Registers.AL ^ Work8) &
	    (Work8 ^ Ans8) & 0x80)
	    SetOverflow();
	else
	    ClearOverflow();
	CPUPack.Registers.AL = Ans8;
	SetZN8 (CPUPack.Registers.AL);
    }
}

STATIC inline void ADC16D0 (long OpAddress)
{
    uint16 Work16 = S9xGetWord (OpAddress);

     {
	uint32 Ans32 = CPUPack.Registers.A.W + Work16 + CheckCarry();

	CPUPack.ICPU._Carry = Ans32 >= 0x10000;

	if (~(CPUPack.Registers.A.W ^ Work16) &
	    (Work16 ^ (uint16) Ans32) & 0x8000)
	    SetOverflow();
	else
	    ClearOverflow();
	CPUPack.Registers.A.W = (uint16) Ans32;
	SetZN16 (CPUPack.Registers.A.W);
    }
}


















STATIC inline void ADC16D1 (long OpAddress)
{
    uint16 Work16 = S9xGetWord (OpAddress);

    {
	uint8 A1 = (CPUPack.Registers.A.W) & 0xF;
	uint8 A2 = (CPUPack.Registers.A.W >> 4) & 0xF;
	uint8 A3 = (CPUPack.Registers.A.W >> 8) & 0xF;
	uint8 A4 = (CPUPack.Registers.A.W >> 12) & 0xF;
	uint8 W1 = Work16 & 0xF;
	uint8 W2 = (Work16 >> 4) & 0xF;
	uint8 W3 = (Work16 >> 8) & 0xF;
	uint8 W4 = (Work16 >> 12) & 0xF;

	A1 += W1 + CheckCarry ();
	if (A1 > 9)
	{
	    A1 -= 10;
	    A2++;
	}

	A2 += W2;
	if (A2 > 9)
	{
	    A2 -= 10;
	    A3++;
	}

	A3 += W3;
	if (A3 > 9)
	{
	    A3 -= 10;
	    A4++;
	}

	A4 += W4;
	if (A4 > 9)
	{
	    A4 -= 10;
	    SetCarry ();
	}
	else
	{
	    ClearCarry ();
	}

	uint16 Ans16 = (A4 << 12) | (A3 << 8) | (A2 << 4) | (A1);
	if (~(CPUPack.Registers.A.W ^ Work16) &
	    (Work16 ^ Ans16) & 0x8000)
	    SetOverflow();
	else
	    ClearOverflow();
	CPUPack.Registers.A.W = Ans16;
	SetZN16 (CPUPack.Registers.A.W);
    }

}

STATIC inline void SBC16D0 (long OpAddress)
{
    uint16 Work16 = S9xGetWord (OpAddress);

    {

	long s9xInt32 = (long) CPUPack.Registers.A.W - (long) Work16 + (long) CheckCarry() - 1;

	CPUPack.ICPU._Carry = s9xInt32 >= 0;

	if ((CPUPack.Registers.A.W ^ Work16) &
	    (CPUPack.Registers.A.W ^ (uint16) s9xInt32) & 0x8000)
	    SetOverflow();
	else
	    ClearOverflow ();
	CPUPack.Registers.A.W = (uint16) s9xInt32;
	SetZN16 (CPUPack.Registers.A.W);
    }
}

STATIC inline void SBC8D0 (long OpAddress)
{
    uint8 Work8 = S9xGetByte (OpAddress);
    {
	short s9xInt16 = (short) CPUPack.Registers.AL - (short) Work8 + (short) CheckCarry() - 1;

	CPUPack.ICPU._Carry = s9xInt16 >= 0;
	if ((CPUPack.Registers.AL ^ Work8) &
	    (CPUPack.Registers.AL ^ (uint8) s9xInt16) & 0x80)
	    SetOverflow ();
	else
	    ClearOverflow ();
	CPUPack.Registers.AL = (uint8) s9xInt16;
	SetZN8 (CPUPack.Registers.AL);
    }
}

STATIC inline void SBC16D1 (long OpAddress)
{
    uint16 Work16 = S9xGetWord (OpAddress);

    {
	uint8 A1 = (CPUPack.Registers.A.W) & 0xF;
	uint8 A2 = (CPUPack.Registers.A.W >> 4) & 0xF;
	uint8 A3 = (CPUPack.Registers.A.W >> 8) & 0xF;
	uint8 A4 = (CPUPack.Registers.A.W >> 12) & 0xF;
	uint8 W1 = Work16 & 0xF;
	uint8 W2 = (Work16 >> 4) & 0xF;
	uint8 W3 = (Work16 >> 8) & 0xF;
	uint8 W4 = (Work16 >> 12) & 0xF;

	A1 -= W1 + !CheckCarry ();
	A2 -= W2;
	A3 -= W3;
	A4 -= W4;
	if (A1 > 9)
	{
	    A1 += 10;
	    A2--;
	}
	if (A2 > 9)
	{
	    A2 += 10;
	    A3--;
	}
	if (A3 > 9)
	{
	    A3 += 10;
	    A4--;
	}
	if (A4 > 9)
	{
	    A4 += 10;
	    ClearCarry ();
	}
	else
	{
	    SetCarry ();
	}

	uint16 Ans16 = (A4 << 12) | (A3 << 8) | (A2 << 4) | (A1);
	if ((CPUPack.Registers.A.W ^ Work16) &
	    (CPUPack.Registers.A.W ^ Ans16) & 0x8000)
	    SetOverflow();
	else
	    ClearOverflow();
	CPUPack.Registers.A.W = Ans16;
	SetZN16 (CPUPack.Registers.A.W);
    }

}

STATIC inline void SBC8D1 (long OpAddress)
{
    uint8 Work8 = S9xGetByte (OpAddress);
    {
	uint8 A1 = (CPUPack.Registers.A.W) & 0xF;
	uint8 A2 = (CPUPack.Registers.A.W >> 4) & 0xF;
	uint8 W1 = Work8 & 0xF;
	uint8 W2 = (Work8 >> 4) & 0xF;

	A1 -= W1 + !CheckCarry ();
	A2 -= W2;
	if (A1 > 9)
	{
	    A1 += 10;
	    A2--;
	}
	if (A2 > 9)
	{
	    A2 += 10;
	    ClearCarry ();
	}
	else
	{
	    SetCarry ();
	}

	uint8 Ans8 = (A2 << 4) | A1;
	if ((CPUPack.Registers.AL ^ Work8) &
	    (CPUPack.Registers.AL ^ Ans8) & 0x80)
	    SetOverflow ();
	else
	    ClearOverflow ();
	CPUPack.Registers.AL = Ans8;
	SetZN8 (CPUPack.Registers.AL);
    }

}





*/










#endif
