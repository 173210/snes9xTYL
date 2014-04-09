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
#ifndef _PPU_H_
#define _PPU_H_

#define FIRST_VISIBLE_LINE 1

extern uint8 GetBank;
extern uint16 SignExtend [2];
//extern uint32 os9x_ColorsChanged;
extern int os9x_softrendering;
extern void tile_askforreset(s32 address);

#define TILE_2BIT 0
#define TILE_4BIT 1
#define TILE_8BIT 2

#define MAX_2BIT_TILES 4096
#define MAX_4BIT_TILES 2048
#define MAX_8BIT_TILES 1024

#define PPU_H_BEAM_IRQ_SOURCE	(1 << 0)
#define PPU_V_BEAM_IRQ_SOURCE	(1 << 1)
#define GSU_IRQ_SOURCE		(1 << 2)
#define SA1_IRQ_SOURCE		(1 << 7)
#define SA1_DMA_IRQ_SOURCE	(1 << 5)

struct ClipData {
    uint32  Count [6];
    uint32  Left [6][6];
    uint32  Right [6][6];
};
struct ClipDataFix {
    uint32  GroupCount [6];
    uint32  Count [6][256];
    uint32  Left [6][6][256];
    uint32  Right [6][6][256];
	uint32  Start [6][256];
	uint32  End [6][256];
};
struct InternalPPU {
    bool8  ColorsChanged;
    uint8  HDMA;
    bool8  HDMAStarted;
    uint8  MaxBrightness;
    bool8  LatchedBlanking;
    bool8  OBJChanged;
    bool8  RenderThisFrame;
    bool8  DirectColourMapsNeedRebuild;
    uint32 FrameCount;
    uint32 RenderedFramesCount;
    uint32 DisplayedRenderedFrameCount;
    uint32 SkippedFrames;
    uint32 FrameSkip;
    uint8  *TileCache [3];
    uint8  *TileCache8 [3];
    uint8  *TileCached [3];
    bool8  FirstVRAMRead;
    bool8  LatchedInterlace;
    bool8  DoubleWidthPixels;
    int    RenderedScreenHeight;
    int    RenderedScreenWidth;
    uint32 Red [256];
    uint32 Green [256];
    uint32 Blue [256];
    uint8  *XB;
    uint16 ScreenColors [256];
    int	   PreviousLine;
    int	   CurrentLine;
    int	   Controller;
    uint32 Joypads[5];
    uint32 SuperScope;
    uint32 Mouse[2];
    int    PrevMouseX[2];
    int    PrevMouseY[2];
    struct ClipData Clip [2];
	struct ClipDataFix ClipFix [2];
	int    ClipFixMaxCount;
	int    PreviousClipLine;

	int    MainColorCount;
	int    FixColorCount;

	uint8  MainColorLines[256];
	uint16 MainColorLog[256];

	uint8  FixColorLines[256];
	int	   FixColorLog[256];
	//int    PreviousPaletteLine;
	//int    PaletteLineCount;
	//uint8  PaletteLines[256];
	//uint16 ScreenColorsLog[256][256];
};

struct SOBJ
{
    short  HPos;
    uint16 VPos;//
    uint16 Name;
    uint8  VFlip;
    uint8  HFlip;//
    uint8  Priority;
    uint8  Palette;
    uint8  Size;
	uint8  sp;//
};

struct SPPU {
    uint8  BGMode;
    uint8  BG3Priority;
    uint8  Brightness;
    bool8  CGFLIP;//

    struct {
	bool8 High;
	uint8 Increment;
	uint16 Address;
	uint16 Mask1;
	uint16 FullGraphicCount;
	uint16 Shift;
	uint16 sp;
    } VMA;//

    struct {
	uint16 SCBase;
	union {
		uint16  VOffset;
		uint8   VOffset_Byte[2];
	};
		union {
		uint16  HOffset;
		uint8   HOffset_Byte[2];
	};
	uint8 BGSize;
	uint8 sp;//
	uint16 NameBase;
	uint16 SCSize;//
    } BG [4];

    uint16 CGDATA [256]; 
    struct SOBJ OBJ [128];
    uint8  FirstSprite;
    uint8  LastSprite;
    uint8  OAMPriorityRotation;
    uint8  OAMFlip;//

    uint16 OAMAddr;    
    uint16 OAMTileAddress;

    uint16 IRQVBeamPos;
	uint16 IRQHBeamPos;

    uint16 VBeamPosLatched;
	uint16 HBeamPosLatched;

    uint8  HBeamFlip;
    uint8  VBeamFlip;
    uint8  HVBeamCounterLatched;
	uint8 sp;

	union {
		short  MatrixA;
		uint8  MatrixA_Byte[2];
	};
	union {
		short  MatrixB;
		uint8  MatrixB_Byte[2];
	};
	union {
		short  MatrixC;
		uint8  MatrixC_Byte[2];
	};
	union {
		short  MatrixD;
		uint8  MatrixD_Byte[2];
	};
	union {
		short  CentreX;
		uint8  CentreX_Byte[2];
	};
	union {
		short  CentreY;
		uint8  CentreY_Byte[2];
	};

    uint8  Joypad1ButtonReadPos;
    uint8  Joypad2ButtonReadPos;
    uint8  CGADD;
    uint8  FixedColourRed;

    uint8  FixedColourGreen;
    uint8  FixedColourBlue;
    uint16 SavedOAMAddr;

    uint32 WRAM;
    uint16 ScreenHeight;
    uint8  BG_Forced;
    bool8  ForcedBlanking;

    bool8  OBJThroughMain;
    bool8  OBJThroughSub;
    uint8  OBJSizeSelect;
    bool8  OBJAddition;
    
    uint8  OAMData [512 + 32];
	uint16 OBJNameBase;
    uint8  OAMReadFlip;
    bool8  VTimerEnabled;

	bool8  HTimerEnabled;
    uint8  Mosaic;

	short  HTimerPosition;
    bool8  BGMosaic [4];
    bool8  Mode7HFlip;

	bool8  Mode7VFlip;
    uint8  Mode7Repeat;
    uint8  Window1Left;
    uint8  Window1Right;

    uint8  Window2Left;
    uint8  Window2Right;
    uint8  ClipCounts [6];

    uint8  ClipWindowOverlapLogic [6];
    uint8  ClipWindow1Enable [6];
    uint8  ClipWindow2Enable [6];
    bool8  ClipWindow1Inside [6];
    bool8  ClipWindow2Inside [6];
    bool8  RecomputeClipWindows;
    uint8  CGFLIPRead;

    uint16 OBJNameSelect;
    bool8  Need16x8Mulitply;
    uint8  Joypad3ButtonReadPos;

    uint8  MouseSpeed[2];
	uint16 sp2;
};

#define CLIP_OR 0
#define CLIP_AND 1
#define CLIP_XOR 2
#define CLIP_XNOR 3

#ifdef LSB_FIRST
 #define LOWBYTE 0
 #define HIGHBYTE 1
#else
 1#define LOWBYTE 1
 #define HIGHBYTE 0
#endif

struct SDMA {
    bool8  TransferDirection;
    bool8  AAddressFixed;
    bool8  AAddressDecrement;
    uint8  TransferMode;

    uint16 AAddress;
    uint16 Address;

    uint8  ABank;
    uint8  BAddress;

    // General DMA only:
    uint16 TransferBytes;

    // H-DMA only:
    uint16 IndirectAddress;
    bool8  HDMAIndirectAddressing;
    uint8  IndirectBank;

    uint8  Repeat;
    uint8  LineCount;
    uint8  FirstLine;
	uint8 sp;
};

START_EXTERN_C
void S9xUpdateScreen ();
void S9xResetPPU ();
void S9xFixColourBrightness ();
void S9xUpdateJoypads ();
void S9xProcessMouse(int which1);
void S9xSuperFXExec ();

void S9xSetPPU (uint8 Byte, uint16 Address);
uint8 S9xGetPPU (uint16 Address);
void S9xSetCPU (uint8 Byte, uint16 Address);
uint8 S9xGetCPU (uint16 Address);

//#ifndef __GP32__
void S9xInitC4 ();
void S9xSetC4 (uint8 Byte, uint16 Address);
uint8 S9xGetC4 (uint16 Address);
void S9xSetC4RAM (uint8 Byte, uint16 Address);
uint8 S9xGetC4RAM (uint16 Address);
//#endif
struct SPPUPACK{
	struct SPPU PPU;
	struct SDMA DMA [8];
	struct InternalPPU IPPU;
};
extern struct SPPUPACK PPUPack;
#define PPU PPUPack.PPU
#define DMA PPUPack.DMA
#define IPPU PPUPack.IPPU
//extern struct SPPU PPU;
//extern struct SDMA DMA [8];
//extern struct InternalPPU IPPU;
//yoyo
extern uint32 os9x_ColorsChanged;
END_EXTERN_C

void ResetClipWindowsFix();
void ComputeClipWindowsFix ();

#include "gfx.h"
#include "memmap.h"

STATIC inline uint8 REGISTER_4212()
{
    GetBank = 0;
    if (CPUPack.CPU.V_Counter >= PPU.ScreenHeight + FIRST_VISIBLE_LINE &&
	CPUPack.CPU.V_Counter < PPU.ScreenHeight + FIRST_VISIBLE_LINE + 3)
	GetBank = 1;

    GetBank |= CPUPack.CPU.Cycles >= Settings.HBlankStart ? 0x40 : 0;
    if (CPUPack.CPU.V_Counter >= PPU.ScreenHeight + FIRST_VISIBLE_LINE)
	GetBank |= 0x80; /* XXX: 0x80 or 0xc0 ? */

    return (GetBank);
}

STATIC inline void FLUSH_REDRAW ()
{
    if (IPPU.PreviousLine != IPPU.CurrentLine) {    	    	
			S9xUpdateScreen ();						
		}
}

STATIC inline void REGISTER_2104 (uint8 byte)
{
    if (PPU.OAMAddr >= 0x110)
	return;
	
    int addr = (PPU.OAMAddr << 1) + (PPU.OAMFlip & 1);
    
    if (byte != PPU.OAMData [addr])
    {
    	INFO_FLUSH_REDRAW("2104");
	FLUSH_REDRAW ();
	PPU.OAMData [addr] = byte;
	IPPU.OBJChanged = TRUE;
	if (addr & 0x200)
	{
	    // X position high bit, and sprite size (x4)
	    struct SOBJ *pObj = &PPU.OBJ [(addr & 0x1f) * 4];

	    pObj->HPos = (pObj->HPos & 0xFF) | SignExtend[(byte >> 0) & 1];
	    pObj++->Size = byte & 2;
	    pObj->HPos = (pObj->HPos & 0xFF) | SignExtend[(byte >> 2) & 1];
	    pObj++->Size = byte & 8;
	    pObj->HPos = (pObj->HPos & 0xFF) | SignExtend[(byte >> 4) & 1];
	    pObj++->Size = byte & 32;
	    pObj->HPos = (pObj->HPos & 0xFF) | SignExtend[(byte >> 6) & 1];
	    pObj->Size = byte & 128;
	}
	else
	{
	    if (addr & 1)
	    {
		if (addr & 2)
		{
		    addr = PPU.OAMAddr >> 1;
		    // Tile within group, priority, h and v flip.
		    PPU.OBJ[addr].Name &= 0xFF;
		    PPU.OBJ[addr].Name |= ((uint16) (byte & 1)) << 8;
		    PPU.OBJ[addr].Palette = (byte >> 1) & 7;
		    PPU.OBJ[addr].Priority = (byte >> 4) & 3;
		    PPU.OBJ[addr].HFlip = (byte >> 6) & 1;
		    PPU.OBJ[addr].VFlip = (byte >> 7) & 1;
		}
		else
		{
		    // Sprite Y position
		    PPU.OBJ[PPU.OAMAddr >> 1].VPos = byte;
		}
	    }
	    else
	    {
		if (addr & 2)
		{
		    // Tile group
		    
		    PPU.OBJ[addr = PPU.OAMAddr >> 1].Name &= 0x100;
		    PPU.OBJ[addr].Name |= byte;
		}
		else
		{
		    // X position (low)
		    PPU.OBJ[addr = PPU.OAMAddr >> 1].HPos &= 0xFF00;
		    PPU.OBJ[addr].HPos |= byte;
		}
	    }
	}
    }
    PPU.OAMFlip ^= 1;
    if (!(PPU.OAMFlip & 1))
	PPU.OAMAddr++;

    ROM_GLOBAL [0x2104] = byte;
}

STATIC inline void REGISTER_2118 (uint8 Byte)
{
    uint32 address;
    if (PPU.VMA.FullGraphicCount)
    {
	uint32 rem = PPU.VMA.Address & PPU.VMA.Mask1;
	address = (((PPU.VMA.Address & ~PPU.VMA.Mask1) +
			 (rem >> PPU.VMA.Shift) +
			 ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3)) << 1) & 0xffff;
	VRAM [address] = Byte;
    }
    else
    {
	VRAM[address = (PPU.VMA.Address << 1) & 0xFFFF] = Byte;
    }

		if (os9x_softrendering>=2) tile_askforreset(address);
		if (os9x_softrendering!=2) {
    	IPPU.TileCached [TILE_2BIT][(address >> 4)<<1] = FALSE;
    	IPPU.TileCached [TILE_4BIT][(address >> 5)<<1] = FALSE;
    	IPPU.TileCached [TILE_8BIT][(address >> 6)<<1] = FALSE;
    }
    

    if (!PPU.VMA.High)
    {
#ifdef DEBUGGER
	if (Settings.TraceVRAM && !CPUPack.CPU.InDMA)
	{
	    printf ("VRAM write byte: $%04X (%d,%d)\n", PPU.VMA.Address,
		    ROM_GLOBAL[0x2115] & 3,
		    (ROM_GLOBAL [0x2115] & 0x0c) >> 2);
	}
#endif	
	PPU.VMA.Address += PPU.VMA.Increment;
    }
//    ROM_GLOBAL [0x2118] = Byte;
}

STATIC inline void REGISTER_2118_tile (uint8 Byte)
{
    uint32 address;
    uint32 rem = PPU.VMA.Address & PPU.VMA.Mask1;
    address = (((PPU.VMA.Address & ~PPU.VMA.Mask1) +
		 (rem >> PPU.VMA.Shift) +
		 ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3)) << 1) & 0xffff;
    VRAM [address] = Byte;

		if (os9x_softrendering>=2) tile_askforreset(address);
		if (os9x_softrendering!=2) {
    	IPPU.TileCached [TILE_2BIT][(address >> 4)<<1] = FALSE;
    	IPPU.TileCached [TILE_4BIT][(address >> 5)<<1] = FALSE;
    	IPPU.TileCached [TILE_8BIT][(address >> 6)<<1] = FALSE;
    }
   
    if (!PPU.VMA.High)
	PPU.VMA.Address += PPU.VMA.Increment;
//    ROM_GLOBAL [0x2118] = Byte;
}

STATIC inline void REGISTER_2118_linear (uint8 Byte)
{
    uint32 address;
    VRAM[address = (PPU.VMA.Address << 1) & 0xFFFF] = Byte;

		if (os9x_softrendering>=2) tile_askforreset(address);
		if (os9x_softrendering!=2) {
    	IPPU.TileCached [TILE_2BIT][(address >> 4)<<1] = FALSE;
    	IPPU.TileCached [TILE_4BIT][(address >> 5)<<1] = FALSE;
    	IPPU.TileCached [TILE_8BIT][(address >> 6)<<1] = FALSE;
    }
   
    if (!PPU.VMA.High)
	PPU.VMA.Address += PPU.VMA.Increment;
//    ROM_GLOBAL [0x2118] = Byte;
}

STATIC inline void REGISTER_2119 (uint8 Byte)
{
    uint32 address;
    if (PPU.VMA.FullGraphicCount)
    {
	uint32 rem = PPU.VMA.Address & PPU.VMA.Mask1;
	address = ((((PPU.VMA.Address & ~PPU.VMA.Mask1) +
		    (rem >> PPU.VMA.Shift) +
		    ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3)) << 1) + 1) & 0xFFFF;
	VRAM [address] = Byte;
    }
    else
    {
	VRAM[address = ((PPU.VMA.Address << 1) + 1) & 0xFFFF] = Byte;
    }

		if (os9x_softrendering>=2) tile_askforreset(address);
		if (os9x_softrendering!=2) {
    	IPPU.TileCached [TILE_2BIT][(address >> 4)<<1] = FALSE;
    	IPPU.TileCached [TILE_4BIT][(address >> 5)<<1] = FALSE;
    	IPPU.TileCached [TILE_8BIT][(address >> 6)<<1] = FALSE;
    }
   
    if (PPU.VMA.High)
    {
#ifdef DEBUGGER
	if (Settings.TraceVRAM && !CPUPack.CPU.InDMA)
	{
	    printf ("VRAM write word: $%04X (%d,%d)\n", PPU.VMA.Address,
		    ROM_GLOBAL[0x2115] & 3,
		    (ROM_GLOBAL [0x2115] & 0x0c) >> 2);
	}
#endif	
	PPU.VMA.Address += PPU.VMA.Increment;
    }
//    ROM_GLOBAL [0x2119] = Byte;
}

STATIC inline void REGISTER_2119_tile (uint8 Byte)
{
    uint32 rem = PPU.VMA.Address & PPU.VMA.Mask1;
    uint32 address = ((((PPU.VMA.Address & ~PPU.VMA.Mask1) +
		    (rem >> PPU.VMA.Shift) +
		    ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3)) << 1) + 1) & 0xFFFF;
    VRAM [address] = Byte;

		if (os9x_softrendering>=2) tile_askforreset(address);
		if (os9x_softrendering!=2) {
    	IPPU.TileCached [TILE_2BIT][(address >> 4)<<1] = FALSE;
    	IPPU.TileCached [TILE_4BIT][(address >> 5)<<1] = FALSE;
    	IPPU.TileCached [TILE_8BIT][(address >> 6)<<1] = FALSE;
    }
 
    if (PPU.VMA.High)
	PPU.VMA.Address += PPU.VMA.Increment;
//    ROM_GLOBAL [0x2119] = Byte;
}

STATIC inline void REGISTER_2119_linear (uint8 Byte)
{
    uint32 address;
    VRAM[address = ((PPU.VMA.Address << 1) + 1) & 0xFFFF] = Byte;

		if (os9x_softrendering>=2) tile_askforreset(address);
		if (os9x_softrendering!=2) {
    	IPPU.TileCached [TILE_2BIT][(address >> 4)<<1] = FALSE;
    	IPPU.TileCached [TILE_4BIT][(address >> 5)<<1] = FALSE;
    	IPPU.TileCached [TILE_8BIT][(address >> 6)<<1] = FALSE;
    }
    
    if (PPU.VMA.High)
	PPU.VMA.Address += PPU.VMA.Increment;
//    ROM_GLOBAL [0x2119] = Byte;
}
STATIC inline void FixColorsLog_BeforeUpdate()
{
	if(IPPU.CurrentLine >= PPU.ScreenHeight) return;
		
	if(os9x_softrendering>=2 && !(os9x_hack&OLD_PSP_ACCEL) && (PPU.BGMode!=7))	
	{
		if(IPPU.FixColorCount==0)
		{
			int fixedcol=((int)(IPPU.XB [PPU.FixedColourRed])<<3)|((int)(IPPU.XB [PPU.FixedColourGreen])<<11)|((int)(IPPU.XB [PPU.FixedColourBlue])<<19);
			IPPU.FixColorLog[0]=fixedcol;
			if(IPPU.PreviousLine>IPPU.CurrentLine)
				IPPU.FixColorLines[0]=0;
			else
				IPPU.FixColorLines[0]=IPPU.PreviousLine;
			IPPU.FixColorCount=1;
		}
	}

}

STATIC inline void FixColorsLog_Update()
{
	if(IPPU.CurrentLine >= PPU.ScreenHeight) return;
	if(IPPU.FixColorCount>=255)return;
	if(os9x_softrendering>=2 && !(os9x_hack&OLD_PSP_ACCEL) && (PPU.BGMode!=7))	
	{
		{//fix color
			int fixedcol=((int)(IPPU.XB [PPU.FixedColourRed])<<3)|((int)(IPPU.XB [PPU.FixedColourGreen])<<11)|((int)(IPPU.XB [PPU.FixedColourBlue])<<19);
			if(IPPU.FixColorLines[IPPU.FixColorCount-1] == IPPU.CurrentLine)
			{
				IPPU.FixColorLog[IPPU.FixColorCount-1]=fixedcol;
			}
			else if(IPPU.FixColorLog[IPPU.FixColorCount-1]==fixedcol)
				return;
			else
			{
				IPPU.FixColorLines[IPPU.FixColorCount] = IPPU.CurrentLine;
				IPPU.FixColorLog[IPPU.FixColorCount]=fixedcol;
				IPPU.FixColorCount++;
				INC_DEBUG_COUNT(46);
			}
		}
	
	
	}
}
STATIC inline void ScreenColorsLog_BeforeUpdate()
{
	if(IPPU.CurrentLine >= PPU.ScreenHeight) return;
	if(os9x_softrendering>=2 && !((os9x_hack&OLD_PSP_ACCEL)&&(PPU.BGMode==2)&&(!Settings.WrestlemaniaArcade)) && !((os9x_hack&OLD_PSP_ACCEL)&&(PPU.BGMode==4)) && (PPU.BGMode!=7))	
	{
		if(PPU.CGADD==0 && IPPU.MainColorCount==0)
		{
			IPPU.MainColorLog[0]=IPPU.ScreenColors[0];
			if(IPPU.PreviousLine>IPPU.CurrentLine)
				IPPU.MainColorLines[0]=0;
			else
				IPPU.MainColorLines[0]=IPPU.PreviousLine;
			IPPU.MainColorCount=1;
		}
	}

}

STATIC inline void ScreenColorsLog_Update()
{
	if(IPPU.CurrentLine >= PPU.ScreenHeight) return;
	if(IPPU.MainColorCount>=255)return;
	if(os9x_softrendering>=2 && !((os9x_hack&OLD_PSP_ACCEL)&&(PPU.BGMode==2)&&(!Settings.WrestlemaniaArcade)) && !((os9x_hack&OLD_PSP_ACCEL)&&(PPU.BGMode==4)) && (PPU.BGMode!=7))
	{
		if(PPU.CGADD==0)
		{//main screen color
			if(IPPU.MainColorLines[IPPU.MainColorCount-1] == IPPU.CurrentLine)
			{
				IPPU.MainColorLog[IPPU.MainColorCount-1]=IPPU.ScreenColors[0];
			}
			else if(IPPU.MainColorLog[IPPU.MainColorCount-1]==IPPU.ScreenColors[0])
			{
				return;
			}
			else
			{
				IPPU.MainColorLines[IPPU.MainColorCount] = IPPU.CurrentLine;
				IPPU.MainColorLog[IPPU.MainColorCount]=IPPU.ScreenColors[0];
				IPPU.MainColorCount++;
			}
		}
	}
}
STATIC inline void REGISTER_2122(uint8 Byte)
{
    // CG-RAM (palette) write
    if (PPU.CGFLIP)
    {    	
	if ((Byte & 0x7f) != (PPU.CGDATA[PPU.CGADD] >> 8))
	{
		if (os9x_softrendering<2 || PPU.CGADD!=0 ||  ((os9x_hack&OLD_PSP_ACCEL)&&(PPU.BGMode==2)&&(!Settings.WrestlemaniaArcade)) || ((os9x_hack&OLD_PSP_ACCEL) && (PPU.BGMode==4)) || (PPU.BGMode==7))
		{
			INC_DEBUG_COUNT(PALETTE_CAHNGE_COUNT);
			if (os9x_hack&PPU_IGNORE_PALWRITE){}
			else if (os9x_hack&PPU_SIMPLE_PALWRITE)
			{
				if((IPPU.CurrentLine&7)==0 || (IPPU.CurrentLine-IPPU.PreviousLine)>7 )
				{
					FLUSH_REDRAW ();
				}
			}
			else
				FLUSH_REDRAW ();
		}
		ScreenColorsLog_BeforeUpdate();
	    PPU.CGDATA[PPU.CGADD] &= 0x00FF;
	    PPU.CGDATA[PPU.CGADD] |= (Byte & 0x7f) << 8;
	    IPPU.ColorsChanged = TRUE;
	    //if (Settings.SixteenBit)
	    {
		IPPU.Blue [PPU.CGADD] = IPPU.XB [(Byte >> 2) & 0x1f];
		IPPU.Green [PPU.CGADD] = IPPU.XB [(PPU.CGDATA[PPU.CGADD] >> 5) & 0x1f];
		IPPU.ScreenColors [PPU.CGADD] = (uint16) BUILD_PIXEL (IPPU.Red [PPU.CGADD],
							     IPPU.Green [PPU.CGADD],
							     IPPU.Blue [PPU.CGADD]);
		os9x_ColorsChanged=1;							     
	    }
		ScreenColorsLog_Update();

	}
	PPU.CGADD++;
    }
    else
    {
	if (Byte != (uint8) (PPU.CGDATA[PPU.CGADD] & 0xff))
	{
		if (os9x_softrendering<2 || PPU.CGADD!=0 ||  ((os9x_hack&OLD_PSP_ACCEL)&&(PPU.BGMode==2)&&(!Settings.WrestlemaniaArcade)) || ((os9x_hack&OLD_PSP_ACCEL) && (PPU.BGMode==4)) || (PPU.BGMode==7))
		{
			if (os9x_hack&PPU_IGNORE_PALWRITE){}
			else if (os9x_hack&PPU_SIMPLE_PALWRITE)
			{
				if((IPPU.CurrentLine&7)==0) /*|| (IPPU.CurrentLine-IPPU.PreviousLine)>7*/ 
				{
					FLUSH_REDRAW ();
				}
			}
			else
				FLUSH_REDRAW ();
		}
		ScreenColorsLog_BeforeUpdate();
	    PPU.CGDATA[PPU.CGADD] &= 0x7F00;
	    PPU.CGDATA[PPU.CGADD] |= Byte;
	    IPPU.ColorsChanged = TRUE;
	    //if (Settings.SixteenBit)
	    {
		IPPU.Red [PPU.CGADD] = IPPU.XB [Byte & 0x1f];
		IPPU.Green [PPU.CGADD] = IPPU.XB [(PPU.CGDATA[PPU.CGADD] >> 5) & 0x1f];
		IPPU.ScreenColors [PPU.CGADD] = (uint16) BUILD_PIXEL (IPPU.Red [PPU.CGADD],
							     IPPU.Green [PPU.CGADD],
							     IPPU.Blue [PPU.CGADD]);
		os9x_ColorsChanged=1;							 
	    }
		ScreenColorsLog_Update();
	}
    }
    PPU.CGFLIP ^= 1;
//    ROM_GLOBAL [0x2122] = Byte;
}

STATIC inline void REGISTER_2180(uint8 Byte)
{
    RAM[PPU.WRAM++] = Byte;
    PPU.WRAM &= 0x1FFFF;
    ROM_GLOBAL [0x2180] = Byte;
}
#endif
