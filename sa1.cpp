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
#include "snes9x.h"
#include "ppu.h"
#include "cpuexec.h"

#include "sa1.h"

static void S9xSA1CharConv2 ();
static void S9xSA1DMA ();
static void S9xSA1ReadVariableLengthData (bool8 inc, bool8 no_shift);

void S9xSA1Init ()
{
    SA1Pack_SA1.NMIActive = FALSE;
    SA1Pack_SA1.IRQActive = FALSE;
    SA1Pack_SA1.WaitingForInterrupt = FALSE;
    SA1Pack_SA1.Waiting = FALSE;
    SA1Pack_SA1.Flags = 0;
    SA1Pack_SA1.Executing = FALSE;
    memset (&FillRAM [0x2200], 0, 0x200);
    FillRAM [0x2200] = 0x20;
    FillRAM [0x2220] = 0x00;
    FillRAM [0x2221] = 0x01;
    FillRAM [0x2222] = 0x02;
    FillRAM [0x2223] = 0x03;
    FillRAM [0x2228] = 0xff;
    SA1Pack_SA1.op1 = 0;
    SA1Pack_SA1.op2 = 0;
    SA1Pack_SA1.arithmetic_op = 0;
    SA1Pack_SA1.sum = 0;
    SA1Pack_SA1.overflow = FALSE;
}

void S9xSA1Reset ()
{
    SA1Pack_SA1Registers.PB = 0;
    SA1Pack_SA1Registers.PC = FillRAM [0x2203] |
		      (FillRAM [0x2204] << 8);
    SA1Pack_SA1Registers.D.W = 0;
    SA1Pack_SA1Registers.DB = 0;
    SA1Pack_SA1Registers.SH = 1;
    SA1Pack_SA1Registers.SL = 0xFF;
    SA1Pack_SA1Registers.XH = 0;
    SA1Pack_SA1Registers.YH = 0;
    SA1Pack_SA1Registers.P.W = 0;

    SA1Pack_SA1.ShiftedPB = 0;
    SA1Pack_SA1.ShiftedDB = 0;
    SA1SetFlags (MemoryFlag | IndexFlag | IRQ | Emulation);
    SA1ClearFlags (Decimal);

    SA1Pack_SA1.WaitingForInterrupt = FALSE;
    SA1Pack_SA1.PC = NULL;
    SA1Pack_SA1.PCBase = NULL;
    S9xSA1SetPCBase (SA1Pack_SA1Registers.PC);
    SA1Pack_SA1.S9xOpcodes = S9xSA1OpcodesM1X1;

    S9xSA1UnpackStatus();
    S9xSA1FixCycles ();
    SA1Pack_SA1.Executing = TRUE;
    SA1Pack_SA1.BWRAM = SRAM;
    FillRAM [0x2225] = 0;
}

void S9xSA1SetBWRAMMemMap (uint8 val)
{
    int c;

    if (val & 0x80)
    {
	for (c = 0; c < 0x400; c += 16)
	{
	    SA1Pack_SA1.Map [c + 6] = SA1Pack_SA1.Map [c + 0x806] = (uint8 *) CMemory::MAP_BWRAM_BITMAP2;
	    SA1Pack_SA1.Map [c + 7] = SA1Pack_SA1.Map [c + 0x807] = (uint8 *) CMemory::MAP_BWRAM_BITMAP2;
	    SA1Pack_SA1.WriteMap [c + 6] = SA1Pack_SA1.WriteMap [c + 0x806] = (uint8 *) CMemory::MAP_BWRAM_BITMAP2;
	    SA1Pack_SA1.WriteMap [c + 7] = SA1Pack_SA1.WriteMap [c + 0x807] = (uint8 *) CMemory::MAP_BWRAM_BITMAP2;
	}
	SA1Pack_SA1.BWRAM = SRAM + (val & 0x7f) * 0x2000 / 4;
    }
    else
    {
	for (c = 0; c < 0x400; c += 16)
	{
	    SA1Pack_SA1.Map [c + 6] = SA1Pack_SA1.Map [c + 0x806] = (uint8 *) CMemory::MAP_BWRAM;
	    SA1Pack_SA1.Map [c + 7] = SA1Pack_SA1.Map [c + 0x807] = (uint8 *) CMemory::MAP_BWRAM;
	    SA1Pack_SA1.WriteMap [c + 6] = SA1Pack_SA1.WriteMap [c + 0x806] = (uint8 *) CMemory::MAP_BWRAM;
	    SA1Pack_SA1.WriteMap [c + 7] = SA1Pack_SA1.WriteMap [c + 0x807] = (uint8 *) CMemory::MAP_BWRAM;
	}
	SA1Pack_SA1.BWRAM = SRAM + (val & 7) * 0x2000;
    }
}

void S9xFixSA1AfterSnapshotLoad ()
{
    SA1Pack_SA1.ShiftedPB = (uint32) SA1Pack_SA1Registers.PB << 16;
    SA1Pack_SA1.ShiftedDB = (uint32) SA1Pack_SA1Registers.DB << 16;

    S9xSA1SetPCBase (SA1Pack_SA1.ShiftedPB + SA1Pack_SA1Registers.PC);
    S9xSA1UnpackStatus ();
    S9xSA1FixCycles ();
    SA1Pack_SA1.VirtualBitmapFormat = (FillRAM [0x223f] & 0x80) ? 2 : 4;
    BWRAM = SRAM + (FillRAM [0x2224] & 7) * 0x2000;
    S9xSA1SetBWRAMMemMap (FillRAM [0x2225]);

    SA1Pack_SA1.Waiting = (FillRAM [0x2200] & 0x60) != 0;
    SA1Pack_SA1.Executing = !SA1Pack_SA1.Waiting;
}

uint8 S9xSA1GetByte (uint32 address)
{
    uint8 *GetAddress = SA1Pack_SA1.Map [(address >> MEMMAP_SHIFT) & MEMMAP_MASK];
    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
	return (*(GetAddress + (address & 0xffff)));

    switch ((int) GetAddress)
    {
    case CMemory::MAP_PPU:
	return (S9xGetSA1 (address & 0xffff));
    case CMemory::MAP_LOROM_SRAM:
    case CMemory::MAP_SA1RAM:
	return (*(SRAM + (address & 0xffff)));
    case CMemory::MAP_BWRAM:
	return (*(SA1Pack_SA1.BWRAM + ((address & 0x7fff) - 0x6000)));
    case CMemory::MAP_BWRAM_BITMAP:
	address -= 0x600000;
	if (SA1Pack_SA1.VirtualBitmapFormat == 2)
	    return ((SRAM [(address >> 2) & 0xffff] >> ((address & 3) << 1)) & 3);
	else
	    return ((SRAM [(address >> 1) & 0xffff] >> ((address & 1) << 2)) & 15);
    case CMemory::MAP_BWRAM_BITMAP2:
	address = (address & 0xffff) - 0x6000;
	if (SA1Pack_SA1.VirtualBitmapFormat == 2)
	    return ((SA1Pack_SA1.BWRAM [(address >> 2) & 0xffff] >> ((address & 3) << 1)) & 3);
	else
	    return ((SA1Pack_SA1.BWRAM [(address >> 1) & 0xffff] >> ((address & 1) << 2)) & 15);

    case CMemory::MAP_DEBUG:
    default:
#ifdef DEBUGGER
//	printf ("R(B) %06x\n", address);
#endif

	return (0);
    }
}

uint16 S9xSA1GetWord (uint32 address)
{
	//return (S9xSA1GetByte (address) | (S9xSA1GetByte (address + 1) << 8));
    uint8 *GetAddress = SA1Pack_SA1.Map [(address >> MEMMAP_SHIFT) & MEMMAP_MASK];
    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
		return (*(GetAddress + (address+1 & 0xffff)))<<8 | (*(GetAddress + (address & 0xffff)));
	else
		return (S9xSA1GetByte (address) | (S9xSA1GetByte (address + 1) << 8));
/*    uint8 *GetAddress = SA1Pack_SA1.Map [(address >> MEMMAP_SHIFT) & MEMMAP_MASK];
    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
	return (*(GetAddress + (address+1 & 0xffff)))<<8 | (*(GetAddress + (address & 0xffff)));

    switch ((int) GetAddress)
    {
    case CMemory::MAP_PPU:
	return (S9xGetSA1 (address+1 & 0xffff)) <<8 | (S9xGetSA1 (address & 0xffff));
    case CMemory::MAP_LOROM_SRAM:
    case CMemory::MAP_SA1RAM:
	return (*(SRAM + (address+1 & 0xffff))) << 8 | (*(SRAM + (address & 0xffff)));
    case CMemory::MAP_BWRAM:
	return (*(SA1Pack_SA1.BWRAM + ((address+1 & 0x7fff) - 0x6000))) << 8 | (*(SA1Pack_SA1.BWRAM + ((address & 0x7fff) - 0x6000)));
    case CMemory::MAP_BWRAM_BITMAP:
	address -= 0x600000;
	if (SA1Pack_SA1.VirtualBitmapFormat == 2)
	    return ((SRAM [(address+1 >> 2) & 0xffff] >> ((address+1 & 3) << 1)) & 3) << 8 | ((SRAM [(address >> 2) & 0xffff] >> ((address & 3) << 1)) & 3);
	else
	    return ((SRAM [(address+1 >> 1) & 0xffff] >> ((address+1 & 1) << 2)) & 15) << 8 | ((SRAM [(address >> 1) & 0xffff] >> ((address & 1) << 2)) & 15);
    case CMemory::MAP_BWRAM_BITMAP2:
	address = (address & 0xffff) - 0x6000;
	if (SA1Pack_SA1.VirtualBitmapFormat == 2)
	    return ((SA1Pack_SA1.BWRAM [(address+1 >> 2) & 0xffff] >> ((address+1 & 3) << 1)) & 3)<<8 | ((SA1Pack_SA1.BWRAM [(address >> 2) & 0xffff] >> ((address & 3) << 1)) & 3);
	else
	    return ((SA1Pack_SA1.BWRAM [(address+1 >> 1) & 0xffff] >> ((address+1 & 1) << 2)) & 15)<<8 | ((SA1Pack_SA1.BWRAM [(address >> 1) & 0xffff] >> ((address & 1) << 2)) & 15);

    case CMemory::MAP_DEBUG:
    default:
#ifdef DEBUGGER
//	printf ("R(B) %06x\n", address);
#endif
	return (0);
    }
*/
}

void S9xSA1SetByte (uint8 byte, uint32 address)
{
    uint8 *Setaddress = SA1Pack_SA1.WriteMap [(address >> MEMMAP_SHIFT) & MEMMAP_MASK];

    if (Setaddress >= (uint8 *) CMemory::MAP_LAST)
    {
	*(Setaddress + (address & 0xffff)) = byte;
	return;
    }

    switch ((int) Setaddress)
    {
    case CMemory::MAP_PPU:
	S9xSetSA1 (byte, address & 0xffff);
	return;
    case CMemory::MAP_SA1RAM:
    case CMemory::MAP_LOROM_SRAM:
	*(SRAM + (address & 0xffff)) = byte;
	return;
    case CMemory::MAP_BWRAM:
	*(SA1Pack_SA1.BWRAM + ((address & 0x7fff) - 0x6000)) = byte;
	return;
    case CMemory::MAP_BWRAM_BITMAP:
	address -= 0x600000;
	if (SA1Pack_SA1.VirtualBitmapFormat == 2)
	{
	    uint8 *ptr = &SRAM [(address >> 2) & 0xffff];
	    *ptr &= ~(3 << ((address & 3) << 1));
	    *ptr |= (byte & 3) << ((address & 3) << 1);
	}
	else
	{
	    uint8 *ptr = &SRAM [(address >> 1) & 0xffff];
	    *ptr &= ~(15 << ((address & 1) << 2));
	    *ptr |= (byte & 15) << ((address & 1) << 2);
	}
	break;
    case CMemory::MAP_BWRAM_BITMAP2:
	address = (address & 0xffff) - 0x6000;
	if (SA1Pack_SA1.VirtualBitmapFormat == 2)
	{
	    uint8 *ptr = &SA1Pack_SA1.BWRAM [(address >> 2) & 0xffff];
	    *ptr &= ~(3 << ((address & 3) << 1));
	    *ptr |= (byte & 3) << ((address & 3) << 1);
	}
	else
	{
	    uint8 *ptr = &SA1Pack_SA1.BWRAM [(address >> 1) & 0xffff];
	    *ptr &= ~(15 << ((address & 1) << 2));
	    *ptr |= (byte & 15) << ((address & 1) << 2);
	}
    default:
	return;
    }
}

void S9xSA1SetWord (uint16 Word, uint32 address)
{
    //S9xSA1SetByte ((uint8) Word, address);
    //S9xSA1SetByte ((uint8) (Word >> 8), address + 1);

	uint8 *Setaddress = SA1Pack_SA1.WriteMap [(address >> MEMMAP_SHIFT) & MEMMAP_MASK];

    if (Setaddress >= (uint8 *) CMemory::MAP_LAST)
    {
	*(Setaddress + (address & 0xffff)) = Word;
	*(Setaddress + (address+1 & 0xffff)) = Word>>8;
	return;
	}
	else
	{
		S9xSA1SetByte ((uint8) Word, address);
		S9xSA1SetByte ((uint8) (Word >> 8), address + 1);
		return;
	}
/*	uint8 *Setaddress = SA1Pack_SA1.WriteMap [(address >> MEMMAP_SHIFT) & MEMMAP_MASK];

    if (Setaddress >= (uint8 *) CMemory::MAP_LAST)
    {
	*(Setaddress + (address & 0xffff)) = Word;
	*(Setaddress + (address+1 & 0xffff)) = Word>>8;
	return;
    }

    switch ((int) Setaddress)
    {
    case CMemory::MAP_PPU:
	S9xSetSA1 (Word, address & 0xffff);
	S9xSetSA1 (Word>>8, address+1 & 0xffff);
	return;
    case CMemory::MAP_SA1RAM:
    case CMemory::MAP_LOROM_SRAM:
	*(SRAM + (address & 0xffff)) = Word;
	*(SRAM + (address+1 & 0xffff)) = Word>>8;
	return;
    case CMemory::MAP_BWRAM:
	*(SA1Pack_SA1.BWRAM + ((address & 0x7fff) - 0x6000)) = Word;
	*(SA1Pack_SA1.BWRAM + ((address+1 & 0x7fff) - 0x6000)) = Word>>8;
	return;
    case CMemory::MAP_BWRAM_BITMAP:
	address -= 0x600000;
	if (SA1Pack_SA1.VirtualBitmapFormat == 2)
	{
	    uint8 *ptr = &SRAM [(address >> 2) & 0xffff];
	    *ptr &= ~(3 << ((address & 3) << 1));
	    *ptr |= (Word & 3) << ((address & 3) << 1);
	    ptr = &SRAM [(address+1 >> 2) & 0xffff];
	    *ptr &= ~(3 << ((address+1 & 3) << 1));
	    *ptr |= (Word>>8 & 3) << ((address+1 & 3) << 1);
	}
	else
	{
	    uint8 *ptr = &SRAM [(address >> 1) & 0xffff];
	    *ptr &= ~(15 << ((address & 1) << 2));
	    *ptr |= (Word & 15) << ((address & 1) << 2);
	    ptr = &SRAM [(address+1 >> 1) & 0xffff];
	    *ptr &= ~(15 << ((address+1 & 1) << 2));
	    *ptr |= (Word>>8 & 15) << ((address+1 & 1) << 2);
	}
	break;
    case CMemory::MAP_BWRAM_BITMAP2:
	address = (address & 0xffff) - 0x6000;
	if (SA1Pack_SA1.VirtualBitmapFormat == 2)
	{
	    uint8 *ptr = &SA1Pack_SA1.BWRAM [(address >> 2) & 0xffff];
	    *ptr &= ~(3 << ((address & 3) << 1));
	    *ptr |= (Word & 3) << ((address & 3) << 1);
	    ptr = &SA1Pack_SA1.BWRAM [(address+1 >> 2) & 0xffff];
	    *ptr &= ~(3 << ((address+1 & 3) << 1));
	    *ptr |= (Word>>8 & 3) << ((address+1 & 3) << 1);
	}
	else
	{
	    uint8 *ptr = &SA1Pack_SA1.BWRAM [(address >> 1) & 0xffff];
	    *ptr &= ~(15 << ((address & 1) << 2));
	    *ptr |= (Word & 15) << ((address & 1) << 2);
	    ptr = &SA1Pack_SA1.BWRAM [(address+1 >> 1) & 0xffff];
	    *ptr &= ~(15 << ((address+1 & 1) << 2));
	    *ptr |= (Word>>8 & 15) << ((address+1 & 1) << 2);
	}
    default:
	return;
    }*/
}

void S9xSA1SetPCBase (uint32 address)
{
    uint8 *GetAddress = SA1Pack_SA1.Map [(address >> MEMMAP_SHIFT) & MEMMAP_MASK];
    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
    {
	SA1Pack_SA1.PCBase = GetAddress;
	SA1Pack_SA1.PC = GetAddress + (address & 0xffff);
	return;
    }

    switch ((int) GetAddress)
    {
    case CMemory::MAP_PPU:
	SA1Pack_SA1.PCBase = FillRAM - 0x2000;
	SA1Pack_SA1.PC = SA1Pack_SA1.PCBase + (address & 0xffff);
	return;
	
    case CMemory::MAP_CPU:
	SA1Pack_SA1.PCBase = FillRAM - 0x4000;
	SA1Pack_SA1.PC = SA1Pack_SA1.PCBase + (address & 0xffff);
	return;
	
    case CMemory::MAP_DSP:
	SA1Pack_SA1.PCBase = FillRAM - 0x6000;
	SA1Pack_SA1.PC = SA1Pack_SA1.PCBase + (address & 0xffff);
	return;
	
    case CMemory::MAP_SA1RAM:
    case CMemory::MAP_LOROM_SRAM:
	SA1Pack_SA1.PCBase = SRAM;
	SA1Pack_SA1.PC = SA1Pack_SA1.PCBase + (address & 0xffff);
	return;

    case CMemory::MAP_BWRAM:
	SA1Pack_SA1.PCBase = SA1Pack_SA1.BWRAM - 0x6000;
	SA1Pack_SA1.PC = SA1Pack_SA1.PCBase + (address & 0xffff);
	return;
    case CMemory::MAP_HIROM_SRAM:
	SA1Pack_SA1.PCBase = SRAM - 0x6000;
	SA1Pack_SA1.PC = SA1Pack_SA1.PCBase + (address & 0xffff);
	return;

    case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
	printf ("SBP %06x\n", address);
#endif
	
    default:
    case CMemory::MAP_NONE:
	SA1Pack_SA1.PCBase = RAM;
	SA1Pack_SA1.PC = RAM + (address & 0xffff);
	return;
    }
}

void S9xSA1ExecuteDuringSleep ()
{
#if 0
    if (SA1Pack_SA1.Executing)
    {
	while (CPUPack.CPU.Cycles < CPUPack.CPU.NextEvent)
	{
	    S9xSA1MainLoop ();
	    CPUPack.CPU.Cycles += TWO_CYCLES * 2;
	}
    }
#endif
}

void S9xSetSA1MemMap (uint32 which1, uint8 map)
{
    int c;
    int start = which1 * 0x100 + 0xc00;
    int start2 = which1 * 0x200;

    if (which1 >= 2)
	start2 += 0x400;

    for (c = 0; c < 0x100; c += 16)
    {
	uint8 *block = &ROM [(map & 7) * 0x100000 + (c << 12)];
	int i;

	for (i = c; i < c + 16; i++)
	    Map [start + i] = SA1Pack_SA1.Map [start + i] = block;
    }
    
    for (c = 0; c < 0x200; c += 16)
    {
	uint8 *block = &ROM [(map & 7) * 0x100000 + (c << 11) - 0x8000];
	int i;

	for (i = c + 8; i < c + 16; i++)
	    Map [start2 + i] = SA1Pack_SA1.Map [start2 + i] = block;
    }
}

uint8 S9xGetSA1 (uint32 address)
{
//	printf ("R: %04x\n", address);
    switch (address)
    {
    case 0x2300:
	return ((uint8) ((FillRAM [0x2209] & 0x5f) | 
		 (CPUPack.CPU.IRQActive & (SA1_IRQ_SOURCE | SA1_DMA_IRQ_SOURCE))));
    case 0x2301:
	return ((FillRAM [0x2200] & 0xf) |
		(FillRAM [0x2301] & 0xf0));
    case 0x2306:
	return ((uint8)  SA1Pack_SA1.sum);
    case 0x2307:
	return ((uint8) (SA1Pack_SA1.sum >>  8));
    case 0x2308:
	return ((uint8) (SA1Pack_SA1.sum >> 16));
    case 0x2309:
	return ((uint8) (SA1Pack_SA1.sum >> 24));
    case 0x230a:
	return ((uint8) (SA1Pack_SA1.sum >> 32));
    case 0x230c:
	return (FillRAM [0x230c]);
    case 0x230d:
    {
	uint8 byte = FillRAM [0x230d];

	if (FillRAM [0x2258] & 0x80)
	{
	    S9xSA1ReadVariableLengthData (TRUE, FALSE);
	}
	return (byte);
    }
    default:	
#ifndef __GP32__    
//	printf ("R: %04x\n", address);
#endif	
	break;
    }
    return (FillRAM [address]);
}

void S9xSetSA1 (uint8 byte, uint32 address)
{
//printf ("W: %02x -> %04x\n", byte, address);
    switch (address)
    {
    case 0x2200:
	SA1Pack_SA1.Waiting = (byte & 0x60) != 0;
//	SA1Pack_SA1.Executing = !SA1Pack_SA1.Waiting && SA1Pack_SA1.S9xOpcodes;

	if (!(byte & 0x20) && (FillRAM [0x2200] & 0x20))
	{
	    S9xSA1Reset ();
	}
	if (byte & 0x80)
	{
	    FillRAM [0x2301] |= 0x80;
	    if (FillRAM [0x220a] & 0x80)
	    {
		SA1Pack_SA1.Flags |= IRQ_PENDING_FLAG;
		SA1Pack_SA1.IRQActive |= SNES_IRQ_SOURCE;
		SA1Pack_SA1.Executing = !SA1Pack_SA1.Waiting && SA1Pack_SA1.S9xOpcodes;
	    }
	}
	if (byte & 0x10)
	{
	    FillRAM [0x2301] |= 0x10;
#ifdef DEBUGGER
		printf ("###SA1 NMI\n");
#endif
	    if (FillRAM [0x220a] & 0x10)
	    {
	    }
	}
	break;

    case 0x2201:
	if (((byte ^ FillRAM [0x2201]) & 0x80) &&
	    (FillRAM [0x2300] & byte & 0x80))
	{
	    S9xSetIRQ (SA1_IRQ_SOURCE);
	}
	if (((byte ^ FillRAM [0x2201]) & 0x20) &&
	    (FillRAM [0x2300] & byte & 0x20))
	{
	    S9xSetIRQ (SA1_DMA_IRQ_SOURCE);
	}
	break;
    case 0x2202:
	if (byte & 0x80)
	{
	    FillRAM [0x2300] &= ~0x80;
	    S9xClearIRQ (SA1_IRQ_SOURCE);
	}
	if (byte & 0x20)
	{
	    FillRAM [0x2300] &= ~0x20;
	    S9xClearIRQ (SA1_DMA_IRQ_SOURCE);
	}
	break;
    case 0x2203:
//	printf ("SA1 reset vector: %04x\n", byte | (FillRAM [0x2204] << 8));
	break;
    case 0x2204:
//	printf ("SA1 reset vector: %04x\n", (byte << 8) | FillRAM [0x2203]);
	break;

    case 0x2205:
//	printf ("SA1 NMI vector: %04x\n", byte | (FillRAM [0x2206] << 8));
	break;
    case 0x2206:
//	printf ("SA1 NMI vector: %04x\n", (byte << 8) | FillRAM [0x2205]);
	break;

    case 0x2207:
//	printf ("SA1 IRQ vector: %04x\n", byte | (FillRAM [0x2208] << 8));
	break;
    case 0x2208:
//	printf ("SA1 IRQ vector: %04x\n", (byte << 8) | FillRAM [0x2207]);
	break;

    case 0x2209:
	FillRAM [0x2209] = byte;
	if (byte & 0x80)
	    FillRAM [0x2300] |= 0x80;

	if (byte & FillRAM [0x2201] & 0x80)
	{
	    S9xSetIRQ (SA1_IRQ_SOURCE);
	}
	break;
    case 0x220a:
	if (((byte ^ FillRAM [0x220a]) & 0x80) &&
	    (FillRAM [0x2301] & byte & 0x80))
	{
	    SA1Pack_SA1.Flags |= IRQ_PENDING_FLAG;
	    SA1Pack_SA1.IRQActive |= SNES_IRQ_SOURCE;
//	    SA1Pack_SA1.Executing = !SA1Pack_SA1.Waiting;
	}
	if (((byte ^ FillRAM [0x220a]) & 0x40) &&
	    (FillRAM [0x2301] & byte & 0x40))
	{
	    SA1Pack_SA1.Flags |= IRQ_PENDING_FLAG;
	    SA1Pack_SA1.IRQActive |= TIMER_IRQ_SOURCE;
//	    SA1Pack_SA1.Executing = !SA1Pack_SA1.Waiting;
	}
	if (((byte ^ FillRAM [0x220a]) & 0x20) &&
	    (FillRAM [0x2301] & byte & 0x20))
	{
	    SA1Pack_SA1.Flags |= IRQ_PENDING_FLAG;
	    SA1Pack_SA1.IRQActive |= DMA_IRQ_SOURCE;
//	    SA1Pack_SA1.Executing = !SA1Pack_SA1.Waiting;
	}
	if (((byte ^ FillRAM [0x220a]) & 0x10) &&
	    (FillRAM [0x2301] & byte & 0x10))
	{
#ifdef DEBUGGER
	    printf ("###SA1 NMI\n");
#endif
	}
	break;
    case 0x220b:
	if (byte & 0x80)
	{
	    SA1Pack_SA1.IRQActive &= ~SNES_IRQ_SOURCE;
	    FillRAM [0x2301] &= ~0x80;
	}
	if (byte & 0x40)
	{
	    SA1Pack_SA1.IRQActive &= ~TIMER_IRQ_SOURCE;
	    FillRAM [0x2301] &= ~0x40;
	}
	if (byte & 0x20)
	{
	    SA1Pack_SA1.IRQActive &= ~DMA_IRQ_SOURCE;
	    FillRAM [0x2301] &= ~0x20;
	}
	if (byte & 0x10)
	{
	    // Clear NMI
	    FillRAM [0x2301] &= ~0x10;
	}
	if (!SA1Pack_SA1.IRQActive)
	    SA1Pack_SA1.Flags &= ~IRQ_PENDING_FLAG;
	break;
    case 0x220c:
//	printf ("SNES NMI vector: %04x\n", byte | (FillRAM [0x220d] << 8));
	break;
    case 0x220d:
//	printf ("SNES NMI vector: %04x\n", (byte << 8) | FillRAM [0x220c]);
	break;

    case 0x220e:
//	printf ("SNES IRQ vector: %04x\n", byte | (FillRAM [0x220f] << 8));
	break;
    case 0x220f:
//	printf ("SNES IRQ vector: %04x\n", (byte << 8) | FillRAM [0x220e]);
	break;

    case 0x2210:
#if 0
	printf ("Timer %s\n", (byte & 0x80) ? "linear" : "HV");
	printf ("Timer H-IRQ %s\n", (byte & 1) ? "enabled" : "disabled");
	printf ("Timer V-IRQ %s\n", (byte & 2) ? "enabled" : "disabled");
#endif
	break;
    case 0x2211:
//	printf ("Timer reset\n");
	break;
    case 0x2212:
#ifndef __GP32__        
//	printf ("H-Timer %04x\n", byte | (FillRAM [0x2213] << 8));
#endif	
	break;
    case 0x2213:
#ifndef __GP32__            
//	printf ("H-Timer %04x\n", (byte << 8) | FillRAM [0x2212]);
#endif		
	break;
    case 0x2214:
#ifndef __GP32__            
//	printf ("V-Timer %04x\n", byte | (FillRAM [0x2215] << 8));
#endif		
	break;
    case 0x2215:
#ifndef __GP32__            
//	printf ("V-Timer %04x\n", (byte << 8) | FillRAM [0x2214]);
#endif		
	break;
    case 0x2220:
    case 0x2221:
    case 0x2222:
    case 0x2223:
	S9xSetSA1MemMap (address - 0x2220, byte);
//	printf ("MMC: %02x\n", byte);
	break;
    case 0x2224:
//	printf ("BWRAM image SNES %02x -> 0x6000\n", byte);
	BWRAM = SRAM + (byte & 7) * 0x2000;
	break;
    case 0x2225:
//	printf ("BWRAM image SA1 %02x -> 0x6000 (%02x)\n", byte, FillRAM [address]);
	if (byte != FillRAM [address])
	    S9xSA1SetBWRAMMemMap (byte);
	break;
    case 0x2226:
//	printf ("BW-RAM SNES write %s\n", (byte & 0x80) ? "enabled" : "disabled");
	break;
    case 0x2227:
//	printf ("BW-RAM SA1 write %s\n", (byte & 0x80) ? "enabled" : "disabled");
	break;

    case 0x2228:
//	printf ("BW-RAM write protect area %02x\n", byte);
	break;
    case 0x2229:
//	printf ("I-RAM SNES write protect area %02x\n", byte);
	break;
    case 0x222a:
//	printf ("I-RAM SA1 write protect area %02x\n", byte);
	break;
    case 0x2230:
#if 0
	printf ("SA1 DMA %s\n", (byte & 0x80) ? "enabled" : "disabled");
	printf ("DMA priority %s\n", (byte & 0x40) ? "DMA" : "SA1");
	printf ("DMA %s\n", (byte & 0x20) ? "char conv" : "normal");
	printf ("DMA type %s\n", (byte & 0x10) ? "BW-RAM -> I-RAM" : "SA1 -> I-RAM");
	printf ("DMA distination %s\n", (byte & 4) ? "BW-RAM" : "I-RAM");
	printf ("DMA source %s\n", DMAsource [byte & 3]);
#endif
	break;
    case 0x2231:
	if (byte & 0x80)
	    SA1Pack_SA1.in_char_dma = FALSE;
#if 0
	printf ("CHDEND %s\n", (byte & 0x80) ? "complete" : "incomplete");
	printf ("DMA colour mode %d\n", byte & 3);
	printf ("virtual VRAM width %d\n", (byte >> 2) & 7);
#endif
	break;
    case 0x2232:
    case 0x2233:
    case 0x2234:
	FillRAM [address] = byte;
#if 0
	printf ("DMA source start %06x\n", 
		FillRAM [0x2232] | (FillRAM [0x2233] << 8) |
		(FillRAM [0x2234] << 16));
#endif
	break;
    case 0x2235:
	FillRAM [address] = byte;
	break;
    case 0x2236:
	FillRAM [address] = byte;
	if ((FillRAM [0x2230] & 0xa4) == 0x80)
	{
	    // Normal DMA to I-RAM
	    S9xSA1DMA ();
	}
	else
	if ((FillRAM [0x2230] & 0xb0) == 0xb0)
	{
	    FillRAM [0x2300] |= 0x20;
	    if (FillRAM [0x2201] & 0x20)
		S9xSetIRQ (SA1_DMA_IRQ_SOURCE);
	    SA1Pack_SA1.in_char_dma = TRUE;
	}
	break;
    case 0x2237:
	FillRAM [address] = byte;
	if ((FillRAM [0x2230] & 0xa4) == 0x84)
	{
	    // Normal DMA to BW-RAM
	    S9xSA1DMA ();
	}
#if 0
	printf ("DMA dest address %06x\n", 
		FillRAM [0x2235] | (FillRAM [0x2236] << 8) |
		(FillRAM [0x2237] << 16));
#endif
	break;
    case 0x2238:
    case 0x2239:
	FillRAM [address] = byte;
#if 0
	printf ("DMA length %04x\n", 
		FillRAM [0x2238] | (FillRAM [0x2239] << 8));
#endif
	break;
    case 0x223f:
	SA1Pack_SA1.VirtualBitmapFormat = (byte & 0x80) ? 2 : 4;
	//printf ("virtual VRAM depth %d\n", (byte & 0x80) ? 2 : 4);
	break;

    case 0x2240:    case 0x2241:    case 0x2242:    case 0x2243:
    case 0x2244:    case 0x2245:    case 0x2246:    case 0x2247:
    case 0x2248:    case 0x2249:    case 0x224a:    case 0x224b:
    case 0x224c:    case 0x224d:    case 0x224e:
#if 0
	if (!(SA1Pack_SA1.Flags & TRACE_FLAG))
	{
	    TraceSA1 ();
	    Trace ();
	}
#endif
	FillRAM [address] = byte;
	break;

    case 0x224f:
	FillRAM [address] = byte;
	if ((FillRAM [0x2230] & 0xb0) == 0xa0)
	{
	    // Char conversion 2 DMA enabled
	    memmove (&ROM [/*CMemory::*/Memory.MAX_ROM_SIZE - 0x10000] + SA1Pack_SA1.in_char_dma * 16,
		     &FillRAM [0x2240], 16);
	    SA1Pack_SA1.in_char_dma = (SA1Pack_SA1.in_char_dma + 1) & 7;
	    if ((SA1Pack_SA1.in_char_dma & 3) == 0)
	    {
		S9xSA1CharConv2 ();
	    }
	}
	break;
    case 0x2250:
	if (byte & 2)
	    SA1Pack_SA1.sum = 0;
	SA1Pack_SA1.arithmetic_op = byte & 3;
	break;
    
    case 0x2251:
	SA1Pack_SA1.op1 = (SA1Pack_SA1.op1 & 0xff00) | byte;
	break;
    case 0x2252:
	SA1Pack_SA1.op1 = (SA1Pack_SA1.op1 & 0xff) | (byte << 8);
	break;
    case 0x2253:
	SA1Pack_SA1.op2 = (SA1Pack_SA1.op2 & 0xff00) | byte;
	break;
    case 0x2254:
	SA1Pack_SA1.op2 = (SA1Pack_SA1.op2 & 0xff) | (byte << 8);
	switch (SA1Pack_SA1.arithmetic_op)
	{
        case 0:	// multiply
	    SA1Pack_SA1.sum = SA1Pack_SA1.op1 * SA1Pack_SA1.op2;
	    break;
	case 1: // divide
	    if (SA1Pack_SA1.op2 == 0)
		SA1Pack_SA1.sum = SA1Pack_SA1.op1 << 16;
	    else
	    {
		SA1Pack_SA1.sum = (SA1Pack_SA1.op1 / (int) ((uint16) SA1Pack_SA1.op2)) |
			  ((SA1Pack_SA1.op1 % (int) ((uint16) SA1Pack_SA1.op2)) << 16);
	    }
	    break;
	case 2:
	default: // cumulative sum
	    SA1Pack_SA1.sum += SA1Pack_SA1.op1 * SA1Pack_SA1.op2;
	    if (SA1Pack_SA1.sum & ((int64) 0xffffff << 32))
		SA1Pack_SA1.overflow = TRUE;
	    break;
	}
	break;
    case 0x2258:    // Variable bit-field length/auto inc/start.
	FillRAM [0x2258] = byte;
	S9xSA1ReadVariableLengthData (TRUE, FALSE);
	return;
    case 0x2259:
    case 0x225a:
    case 0x225b:    // Variable bit-field start address
	FillRAM [address] = byte;
	// XXX: ???
	SA1Pack_SA1.variable_bit_pos = 0;
	S9xSA1ReadVariableLengthData (FALSE, TRUE);
	return;
    default:
//	printf ("W: %02x->%04x\n", byte, address);
	break;
    }
    if (address >= 0x2200 && address <= 0x22ff)
	FillRAM [address] = byte;
}

static void S9xSA1CharConv2 ()
{
    uint32 dest = FillRAM [0x2235] | (FillRAM [0x2236] << 8);
    uint32 offset = (SA1Pack_SA1.in_char_dma & 7) ? 0 : 1;
    int depth = (FillRAM [0x2231] & 3) == 0 ? 8 :
		(FillRAM [0x2231] & 3) == 1 ? 4 : 2;
    int bytes_per_char = 8 * depth;
    uint8 *p = &FillRAM [0x3000] + dest + offset * bytes_per_char;
    uint8 *q = &ROM [/*CMemory::*/Memory.MAX_ROM_SIZE - 0x10000] + offset * 64;

    switch (depth)
    {
    case 2:
	break;
    case 4:
	break;
    case 8:
	for (int l = 0; l < 8; l++, q += 8)
	{
	    for (int b = 0; b < 8; b++)
	    {
		uint8 r = *(q + b);
		*(p +  0) = (*(p +  0) << 1) | ((r >> 0) & 1);
		*(p +  1) = (*(p +  1) << 1) | ((r >> 1) & 1);
		*(p + 16) = (*(p + 16) << 1) | ((r >> 2) & 1);
		*(p + 17) = (*(p + 17) << 1) | ((r >> 3) & 1);
		*(p + 32) = (*(p + 32) << 1) | ((r >> 4) & 1);
		*(p + 33) = (*(p + 33) << 1) | ((r >> 5) & 1);
		*(p + 48) = (*(p + 48) << 1) | ((r >> 6) & 1);
		*(p + 49) = (*(p + 49) << 1) | ((r >> 7) & 1);
	    }
	    p += 2;
	}
	break;
    }
}

static void S9xSA1DMA ()
{
    uint32 src =  FillRAM [0x2232] |
	         (FillRAM [0x2233] << 8) |
		 (FillRAM [0x2234] << 16);
    uint32 dst =  FillRAM [0x2235] |
	         (FillRAM [0x2236] << 8) |
		 (FillRAM [0x2237] << 16);
    uint32 len =  FillRAM [0x2238] |
		 (FillRAM [0x2239] << 8);

    uint8 *s;
    uint8 *d;

    switch (FillRAM [0x2230] & 3)
    {
    case 0: // ROM
	s = SA1Pack_SA1.Map [(src >> MEMMAP_SHIFT) & MEMMAP_MASK];
	if (s >= (uint8 *) CMemory::MAP_LAST)
	    s += (src & 0xffff);
	else
	    s = ROM + (src & 0xffff);
	break;
    case 1: // BW-RAM
	src &= Memory.SRAMMask;
	len &= Memory.SRAMMask;
	s = SRAM + src;
	break;
    default:
    case 2:
	src &= 0x3ff;
	len &= 0x3ff;
	s = &FillRAM [0x3000] + src;
	break;
    }

    if (FillRAM [0x2230] & 4)
    {
	dst &= Memory.SRAMMask;
	len &= Memory.SRAMMask;
	d = SRAM + dst;
    }
    else
    {
	dst &= 0x3ff;
	len &= 0x3ff;
	d = &FillRAM [0x3000] + dst;
    }
    memmove (d, s, len);
    FillRAM [0x2301] |= 0x20;
    
    if (FillRAM [0x220a] & 0x20)
    {
	SA1Pack_SA1.Flags |= IRQ_PENDING_FLAG;
	SA1Pack_SA1.IRQActive |= DMA_IRQ_SOURCE;
//	SA1Pack_SA1.Executing = !SA1Pack_SA1.Waiting;
    }
}

void S9xSA1ReadVariableLengthData (bool8 inc, bool8 no_shift)
{
    uint32 addr =  FillRAM [0x2259] |
		  (FillRAM [0x225a] << 8) |
		  (FillRAM [0x225b] << 16);
    uint8 shift = FillRAM [0x2258] & 15;

    if (no_shift)
	shift = 0;
    else
    if (shift == 0)
	shift = 16;

    uint8 s = shift + SA1Pack_SA1.variable_bit_pos;

    if (s >= 16)
    {
	addr += (s >> 4) << 1;
	s &= 15;
    }
    uint32 data = S9xSA1GetWord (addr) |
		  (S9xSA1GetWord (addr + 2) << 16);

    data >>= s;
    FillRAM [0x230c] = (uint8) data;
    FillRAM [0x230d] = (uint8) (data >> 8);
    if (inc)
    {
	SA1Pack_SA1.variable_bit_pos = (SA1Pack_SA1.variable_bit_pos + shift) & 15;
	FillRAM [0x2259] = (uint8) addr;
	FillRAM [0x225a] = (uint8) (addr >> 8);
	FillRAM [0x225b] = (uint8) (addr >> 16);
    }
}
