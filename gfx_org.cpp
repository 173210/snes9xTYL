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

#include "memmap.h"
#include "ppu.h"
#include "cpuexec.h"
#include "display.h"
#include "gfx.h"
#include "apu.h"
#include "cheats.h"

#define M7 19
#define M8 19

void ComputeClipWindows ();

extern int os9x_BG0,os9x_BG1,os9x_BG2,os9x_BG3,os9x_OBJ,os9x_easy;

extern void S9xSetupOBJ(void);

extern uint8 BitShifts[8][4];
extern uint8 TileShifts[8][4];
extern uint8 PaletteShifts[8][4];
extern uint8 PaletteMasks[8][4];
extern uint8 Depths[8][4];
extern uint8 BGSizes [2];

extern NormalTileRenderer DrawTilePtr;
extern ClippedTileRenderer DrawClippedTilePtr;
extern NormalTileRenderer DrawHiResTilePtr;
extern ClippedTileRenderer DrawHiResClippedTilePtr;
extern LargePixelRenderer DrawLargePixelPtr;

//extern struct SBG BG;

extern struct SLineData LineData[240];
extern struct SLineMatrixData LineMatrixData [240];

extern uint8  Mode7Depths [2];

#define ON_MAIN(N) \
(GPUPack.GFX.r212c & (1 << (N)) && \
!(PPUPack.PPU.BG_Forced & (1 << (N))))

#define SUB_OR_ADD(N) \
(GPUPack.GFX.r2131 & (1 << (N)))

#define ON_SUB(N) \
((GPUPack.GFX.r2130 & 0x30) != 0x30 && \
(GPUPack.GFX.r2130 & 2) && \
(GPUPack.GFX.r212d & (1 << N)) && \
!(PPUPack.PPU.BG_Forced & (1 << (N))))

#define ANYTHING_ON_SUB \
((GPUPack.GFX.r2130 & 0x30) != 0x30 && \
(GPUPack.GFX.r2130 & 2) && \
(GPUPack.GFX.r212d & 0x1f))

#define ADD_OR_SUB_ON_ANYTHING \
(GPUPack.GFX.r2131 & 0x3f)

#define BLACK BUILD_PIXEL(0,0,0)

void orgDrawTileHi16 (uint32 Tile, uint32 Offset, uint32 StartLine,
	         uint32 LineCount);
void orgDrawClippedTileHi16 (uint32 Tile, uint32 Offset,
		        uint32 StartPixel, uint32 Width,
		        uint32 StartLine, uint32 LineCount);

void orgDrawTile16 (uint32 Tile, uint32 Offset, uint32 StartLine,
	         uint32 LineCount);
void orgDrawClippedTile16 (uint32 Tile, uint32 Offset,
		        uint32 StartPixel, uint32 Width,
		        uint32 StartLine, uint32 LineCount);
void orgDrawTile16x2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		   uint32 LineCount);
void orgDrawClippedTile16x2 (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Width,
			  uint32 StartLine, uint32 LineCount);
void orgDrawTile16x2x2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		     uint32 LineCount);
void orgDrawClippedTile16x2x2 (uint32 Tile, uint32 Offset,
			    uint32 StartPixel, uint32 Width,
			    uint32 StartLine, uint32 LineCount);
void orgDrawLargePixel16 (uint32 Tile, uint32 Offset,
		       uint32 StartPixel, uint32 Pixels,
		       uint32 StartLine, uint32 LineCount);

void orgDrawTile16Add (uint32 Tile, uint32 Offset, uint32 StartLine,
		    uint32 LineCount);

void orgDrawClippedTile16Add (uint32 Tile, uint32 Offset,
			   uint32 StartPixel, uint32 Width,
			   uint32 StartLine, uint32 LineCount);

void orgDrawTile16Add1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		       uint32 LineCount);

void orgDrawClippedTile16Add1_2 (uint32 Tile, uint32 Offset,
			      uint32 StartPixel, uint32 Width,
			      uint32 StartLine, uint32 LineCount);

void orgDrawTile16FixedAdd1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
			    uint32 LineCount);

void orgDrawClippedTile16FixedAdd1_2 (uint32 Tile, uint32 Offset,
				   uint32 StartPixel, uint32 Width,
				   uint32 StartLine, uint32 LineCount);

void orgDrawTile16Sub (uint32 Tile, uint32 Offset, uint32 StartLine,
		    uint32 LineCount);

void orgDrawClippedTile16Sub (uint32 Tile, uint32 Offset,
			   uint32 StartPixel, uint32 Width,
			   uint32 StartLine, uint32 LineCount);

void orgDrawTile16Sub1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
		       uint32 LineCount);

void orgDrawClippedTile16Sub1_2 (uint32 Tile, uint32 Offset,
			      uint32 StartPixel, uint32 Width,
			      uint32 StartLine, uint32 LineCount);

void orgDrawTile16FixedSub1_2 (uint32 Tile, uint32 Offset, uint32 StartLine,
			    uint32 LineCount);

void orgDrawClippedTile16FixedSub1_2 (uint32 Tile, uint32 Offset,
				   uint32 StartPixel, uint32 Width,
				   uint32 StartLine, uint32 LineCount);

void orgDrawLargePixel16Add (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Pixels,
			  uint32 StartLine, uint32 LineCount);

void orgDrawLargePixel16Add1_2 (uint32 Tile, uint32 Offset,
			     uint32 StartPixel, uint32 Pixels,
			     uint32 StartLine, uint32 LineCount);

void orgDrawLargePixel16Sub (uint32 Tile, uint32 Offset,
			  uint32 StartPixel, uint32 Pixels,
			  uint32 StartLine, uint32 LineCount);

void orgDrawLargePixel16Sub1_2 (uint32 Tile, uint32 Offset,
			     uint32 StartPixel, uint32 Pixels,
			     uint32 StartLine, uint32 LineCount);
/*
char mul2shl[]={
0,
1,1,
2,2,2,2,
3,3,3,3,3,3,3,3,
4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9
};
*/

inline void orgSelectTileRenderer (bool8 normal)
{
	DrawHiResTilePtr= orgDrawTileHi16;
	DrawHiResClippedTilePtr = orgDrawClippedTileHi16;


    if (normal)
    {

	DrawTilePtr = orgDrawTile16;
	DrawClippedTilePtr = orgDrawClippedTile16;
	DrawLargePixelPtr = orgDrawLargePixel16;
    }
    else
    {

	if (GPUPack.GFX.r2131 & 0x80)
	{
	    if (GPUPack.GFX.r2131 & 0x40)
	    {
		if (GPUPack.GFX.r2130 & 2)
		{

		    DrawTilePtr = orgDrawTile16Sub1_2;
		    DrawClippedTilePtr = orgDrawClippedTile16Sub1_2;
		}
		else
		{

		    // Fixed colour substraction
		    DrawTilePtr = orgDrawTile16FixedSub1_2;
		    DrawClippedTilePtr = orgDrawClippedTile16FixedSub1_2;
		}
		DrawLargePixelPtr = orgDrawLargePixel16Sub1_2;
	    }
	    else
	    {

		DrawTilePtr = orgDrawTile16Sub;
		DrawClippedTilePtr = orgDrawClippedTile16Sub;
		DrawLargePixelPtr = orgDrawLargePixel16Sub;
	    }
	}
	else
	{
	    if (GPUPack.GFX.r2131 & 0x40)
	    {
		if (GPUPack.GFX.r2130 & 2)
		{

		    DrawTilePtr = orgDrawTile16Add1_2;
		    DrawClippedTilePtr = orgDrawClippedTile16Add1_2;
		}
		else
		{

		    // Fixed colour addition
		    DrawTilePtr = orgDrawTile16FixedAdd1_2;
		    DrawClippedTilePtr = orgDrawClippedTile16FixedAdd1_2;
		}
		DrawLargePixelPtr = orgDrawLargePixel16Add1_2;
	    }
	    else
	    {

		DrawTilePtr = orgDrawTile16Add;
		DrawClippedTilePtr = orgDrawClippedTile16Add;
		DrawLargePixelPtr = orgDrawLargePixel16Add;
	    }
	}
    }
}

void orgDrawOBJS (bool8 OnMain = FALSE, uint8 D = 0)
{
    uint32 O;
    uint32 BaseTile, Tile;

    CHECK_SOUND();

    GPUPack.BG.BitShift = 4;
    GPUPack.BG.TileShift = 5;
    GPUPack.BG.TileAddress = PPUPack.PPU.OBJNameBase;
    GPUPack.BG.StartPalette = 128;
    GPUPack.BG.PaletteShift = 4;
    GPUPack.BG.PaletteMask = 7;
    GPUPack.BG.Buffer = IPPU.TileCache [TILE_4BIT];
    GPUPack.BG.Buffered = IPPU.TileCached [TILE_4BIT];
    GPUPack.BG.NameSelect = PPUPack.PPU.OBJNameSelect;
    GPUPack.BG.DirectColourMode = FALSE;

 



	DrawTilePtr = orgDrawTile16;
	DrawClippedTilePtr = orgDrawClippedTile16;

    GPUPack.GFX.Z1 = D + 2;

    int I = 0;
    for (int S = GPUPack.GFX.OBJList [I++]; S >= 0; S = GPUPack.GFX.OBJList [I++])
    {
	int VPos = GPUPack.GFX.VPositions [S];
	int Size = GPUPack.GFX.Sizes[S];
	int TileInc = 1;
	int Offset;

	if (VPos + Size <= (int) GPUPack.GFX.StartY || VPos > (int) GPUPack.GFX.EndY)
	    continue;

	if (OnMain && SUB_OR_ADD(4))
	{
	    orgSelectTileRenderer (!GPUPack.GFX.Pseudo && PPUPack.PPU.OBJ [S].Palette < 4);
	}

	BaseTile = PPUPack.PPU.OBJ[S].Name | (PPUPack.PPU.OBJ[S].Palette << 10);

	if (PPUPack.PPU.OBJ[S].HFlip)
	{
	    BaseTile += ((Size >> 3) - 1) | H_FLIP;
	    TileInc = -1;
	}
	if (PPUPack.PPU.OBJ[S].VFlip)
	    BaseTile |= V_FLIP;

	int clipcount = GPUPack.GFX.pCurrentClip->Count [4];
	if (!clipcount)
	    clipcount = 1;

	GPUPack.GFX.Z2 = (PPUPack.PPU.OBJ[S].Priority + 1) * 4 + D;

	for (int clip = 0; clip < clipcount; clip++)
	{
	    int Left;
	    int Right;
	    if (!GPUPack.GFX.pCurrentClip->Count [4])
	    {
		Left = 0;
		Right = 256;
	    }
	    else
	    {
		Left = GPUPack.GFX.pCurrentClip->Left [clip][4];
		Right = GPUPack.GFX.pCurrentClip->Right [clip][4];
	    }

	    if (Right <= Left || PPUPack.PPU.OBJ[S].HPos + Size <= Left ||
		PPUPack.PPU.OBJ[S].HPos >= Right)
		continue;

	    for (int Y = 0; Y < Size; Y += 8)
	    {
		if (VPos + Y + 7 >= (int) GPUPack.GFX.StartY && VPos + Y <= (int) GPUPack.GFX.EndY)
		{
		    int StartLine;
		    int TileLine;
		    int LineCount;
		    int Last;

		    if ((StartLine = VPos + Y) < (int) GPUPack.GFX.StartY)
		    {
			StartLine = GPUPack.GFX.StartY - StartLine;
			LineCount = 8 - StartLine;
		    }
		    else
		    {
			StartLine = 0;
			LineCount = 8;
		    }
		    if ((Last = VPos + Y + 7 - GPUPack.GFX.EndY) > 0)
			if ((LineCount -= Last) <= 0)
			    break;

		    TileLine = StartLine << 3;
		    O = (VPos + Y + StartLine) * GPUPack.GFX.PPL;
		    if (!PPUPack.PPU.OBJ[S].VFlip)
			Tile = BaseTile + (Y << 1);
		    else
			Tile = BaseTile + ((Size - Y - 8) << 1);

		    int Middle = Size >> 3;
		    if (PPUPack.PPU.OBJ[S].HPos < Left)
		    {
			Tile += ((Left - PPUPack.PPU.OBJ[S].HPos) >> 3) * TileInc;
			Middle -= (Left - PPUPack.PPU.OBJ[S].HPos) >> 3;
			O += Left * 1;
			if ((Offset = (Left - PPUPack.PPU.OBJ[S].HPos) & 7))
			{
			    O -= Offset * 1;
			    int W = 8 - Offset;
			    int Width = Right - Left;
			    if (W > Width)
				W = Width;
			    (*DrawClippedTilePtr) (Tile, O, Offset, W,
						   TileLine, LineCount);

			    if (W >= Width)
				continue;
			    Tile += TileInc;
			    Middle--;
			    O += 8 * 1;
			}
		    }
		    else
			O += PPUPack.PPU.OBJ[S].HPos * 1;

		    if (PPUPack.PPU.OBJ[S].HPos + Size >= Right)
		    {
			Middle -= ((PPUPack.PPU.OBJ[S].HPos + Size + 7) -
				   Right) >> 3;
			Offset = (Right - (PPUPack.PPU.OBJ[S].HPos + Size)) & 7;
		    }
		    else
			Offset = 0;

		    for (int X = 0; X < Middle; X++, O += 8 * 1,
			 Tile += TileInc)
		    {
			(*DrawTilePtr) (Tile, O, TileLine, LineCount);
		    }
		    if (Offset)
		    {
			(*DrawClippedTilePtr) (Tile, O, 0, Offset,
					       TileLine, LineCount);
		    }
		}
	    }
	}
    }
}

void orgDrawBackgroundMosaic (uint32 BGMode, uint32 bg, uint8 Z1, uint8 Z2)
{
    CHECK_SOUND();

    uint32 Tile;
    uint16 *SC0;
    uint16 *SC1;
    uint16 *SC2;
    uint16 *SC3;
    uint8 depths [2] = {Z1, Z2};

    if (BGMode == 0)
	GPUPack.BG.StartPalette = bg << 5;
    else
	GPUPack.BG.StartPalette = 0;

    SC0 = (uint16 *) &VRAM[PPUPack.PPU.BG[bg].SCBase << 1];

    if (PPUPack.PPU.BG[bg].SCSize & 1)
	SC1 = SC0 + 1024;
    else
	SC1 = SC0;

	if((SC1-(unsigned short*)VRAM)>0x10000)
		SC1-=0x10000;


    if (PPUPack.PPU.BG[bg].SCSize & 2)
	SC2 = SC1 + 1024;
    else
	SC2 = SC0;

	if((SC2-(unsigned short*)VRAM)>0x10000)
		SC2-=0x10000;


    if (PPUPack.PPU.BG[bg].SCSize & 1)
	SC3 = SC2 + 1024;
    else
	SC3 = SC2;

	if((SC3-(unsigned short*)VRAM)>0x10000)
		SC3-=0x10000;

    uint32 Lines;
    uint32 OffsetMask;
    uint32 OffsetShift;

    if (GPUPack.BG.TileSize == 16)
    {
	OffsetMask = 0x3ff;
	OffsetShift = 4;
    }
    else
    {
	OffsetMask = 0x1ff;
	OffsetShift = 3;
    }

    for (uint32 Y = GPUPack.GFX.StartY; Y <= GPUPack.GFX.EndY; Y += Lines)
    {
	uint32 VOffset = LineData [Y].BG[bg].VOffset;
	uint32 HOffset = LineData [Y].BG[bg].HOffset;
	uint32 MosaicOffset = Y % PPUPack.PPU.Mosaic;

	for (Lines = 1; Lines < PPUPack.PPU.Mosaic - MosaicOffset; Lines++)
	    if ((VOffset != LineData [Y + Lines].BG[bg].VOffset) ||
		(HOffset != LineData [Y + Lines].BG[bg].HOffset))
		break;

	uint32 MosaicLine = VOffset + Y - MosaicOffset;

	if (Y + Lines > GPUPack.GFX.EndY)
	    Lines = GPUPack.GFX.EndY + 1 - Y;
	uint32 VirtAlign = (MosaicLine & 7) << 3;

	uint16 *b1;
	uint16 *b2;

	uint32 ScreenLine = MosaicLine >> OffsetShift;
	uint32 Rem16 = MosaicLine & 15;

	if (ScreenLine & 0x20)
	    b1 = SC2, b2 = SC3;
	else
	    b1 = SC0, b2 = SC1;

	b1 += (ScreenLine & 0x1f) << 5;
	b2 += (ScreenLine & 0x1f) << 5;
	uint16 *t;
	uint32 Left = 0;
	uint32 Right = 256;

	uint32 ClipCount = GPUPack.GFX.pCurrentClip->Count [bg];
	uint32 HPos = HOffset;
	uint32 PixWidth = PPUPack.PPU.Mosaic;

	if (!ClipCount)
	    ClipCount = 1;

	for (uint32 clip = 0; clip < ClipCount; clip++)
	{
	    if (GPUPack.GFX.pCurrentClip->Count [bg])
	    {
		Left = GPUPack.GFX.pCurrentClip->Left [clip][bg];
		Right = GPUPack.GFX.pCurrentClip->Right [clip][bg];
		uint32 r = Left % PPUPack.PPU.Mosaic;
		HPos = HOffset + Left;
		PixWidth = PPUPack.PPU.Mosaic - r;
	    }
	    uint32 s = Y * GPUPack.GFX.PPL + Left * 1;
	    for (uint32 x = Left; x < Right; x += PixWidth,
		 s += PixWidth * 1,
		 HPos += PixWidth, PixWidth = PPUPack.PPU.Mosaic)
	    {
		uint32 Quot = (HPos & OffsetMask) >> 3;

		if (x + PixWidth >= Right)
		    PixWidth = Right - x;

		if (GPUPack.BG.TileSize == 8)
		{
		    if (Quot > 31)
			t = b2 + (Quot & 0x1f);
		    else
			t = b1 + Quot;
		}
		else
		{
		    if (Quot > 63)
			t = b2 + ((Quot >> 1) & 0x1f);
		    else
			t = b1 + (Quot >> 1);
		}

		Tile = READ_2BYTES (t);
		GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];

		// orgDraw tile...
		if (GPUPack.BG.TileSize != 8)
		{
		    if (Tile & H_FLIP)
		    {
			// Horizontal flip, but what about vertical flip ?
			if (Tile & V_FLIP)
			{
			    // Both horzontal & vertical flip
			    if (Rem16 < 8)
			    {
				(*DrawLargePixelPtr) (Tile + 17 - (Quot & 1), s,
						      HPos & 7, PixWidth,
						      VirtAlign, Lines);
			    }
			    else
			    {
				(*DrawLargePixelPtr) (Tile + 1 - (Quot & 1), s,
						      HPos & 7, PixWidth,
						      VirtAlign, Lines);
			    }
			}
			else
			{
			    // Horizontal flip only
			    if (Rem16 > 7)
			    {
				(*DrawLargePixelPtr) (Tile + 17 - (Quot & 1), s,
						      HPos & 7, PixWidth,
						      VirtAlign, Lines);
			    }
			    else
			    {
				(*DrawLargePixelPtr) (Tile + 1 - (Quot & 1), s,
						      HPos & 7, PixWidth,
						      VirtAlign, Lines);
			    }
			}
		    }
		    else
		    {
			// No horizontal flip, but is there a vertical flip ?
			if (Tile & V_FLIP)
			{
			    // Vertical flip only
			    if (Rem16 < 8)
			    {
				(*DrawLargePixelPtr) (Tile + 16 + (Quot & 1), s,
						      HPos & 7, PixWidth,
						      VirtAlign, Lines);
			    }
			    else
			    {
				(*DrawLargePixelPtr) (Tile + (Quot & 1), s,
						      HPos & 7, PixWidth,
						      VirtAlign, Lines);
			    }
			}
			else
			{
			    // Normal unflipped
			    if (Rem16 > 7)
			    {
				(*DrawLargePixelPtr) (Tile + 16 + (Quot & 1), s,
						      HPos & 7, PixWidth,
						      VirtAlign, Lines);
			    }
			    else
			    {
				(*DrawLargePixelPtr) (Tile + (Quot & 1), s,
						      HPos & 7, PixWidth,
						      VirtAlign, Lines);
			    }
			}
		    }
		}
		else
		    (*DrawLargePixelPtr) (Tile, s, HPos & 7, PixWidth,
					  VirtAlign, Lines);
	    }
	}
    }
}

void orgDrawBackgroundOffset (uint32 BGMode, uint32 bg, uint8 Z1, uint8 Z2)
{
    CHECK_SOUND();

    uint32 Tile;
    uint16 *SC0;
    uint16 *SC1;
    uint16 *SC2;
    uint16 *SC3;
    uint16 *BPS0;
    uint16 *BPS1;
    uint16 *BPS2;
    uint16 *BPS3;
    uint32 Width;
    int VOffsetOffset = BGMode == 4 ? 0 : 32;
    uint8 depths [2] = {Z1, Z2};

    GPUPack.BG.StartPalette = 0;

    BPS0 = (uint16 *) &VRAM[PPUPack.PPU.BG[2].SCBase << 1];

    if (PPUPack.PPU.BG[2].SCSize & 1)
	BPS1 = BPS0 + 1024;
    else
	BPS1 = BPS0;

    if (PPUPack.PPU.BG[2].SCSize & 2)
	BPS2 = BPS1 + 1024;
    else
	BPS2 = BPS0;

    if (PPUPack.PPU.BG[2].SCSize & 1)
	BPS3 = BPS2 + 1024;
    else
	BPS3 = BPS2;

    SC0 = (uint16 *) &VRAM[PPUPack.PPU.BG[bg].SCBase << 1];

    if (PPUPack.PPU.BG[bg].SCSize & 1)
	SC1 = SC0 + 1024;
    else
	SC1 = SC0;

	if((SC1-(unsigned short*)VRAM)>0x10000)
		SC1-=0x10000;


    if (PPUPack.PPU.BG[bg].SCSize & 2)
	SC2 = SC1 + 1024;
    else
	SC2 = SC0;

	if((SC2-(unsigned short*)VRAM)>0x10000)
		SC2-=0x10000;


    if (PPUPack.PPU.BG[bg].SCSize & 1)
	SC3 = SC2 + 1024;
    else
	SC3 = SC2;

	if((SC3-(unsigned short*)VRAM)>0x10000)
		SC3-=0x10000;


    static const int Lines = 1;
    int OffsetMask;
    int OffsetShift;
    int OffsetEnableMask = 1 << (bg + 13);

    if (GPUPack.BG.TileSize == 16)
    {
	OffsetMask = 0x3ff;
	OffsetShift = 4;
    }
    else
    {
	OffsetMask = 0x1ff;
	OffsetShift = 3;
    }

    for (uint32 Y = GPUPack.GFX.StartY; Y <= GPUPack.GFX.EndY; Y++)
    {
		uint32 VOff = LineData [Y].BG[2].VOffset - 1;
//		uint32 VOff = LineData [Y].BG[2].VOffset;
	uint32 HOff = LineData [Y].BG[2].HOffset;

	int VirtAlign;
	int ScreenLine = VOff >> 3;
	int t1;
	int t2;
	uint16 *s0;
	uint16 *s1;
	uint16 *s2;

	if (ScreenLine & 0x20)
	    s1 = BPS2, s2 = BPS3;
	else
	    s1 = BPS0, s2 = BPS1;

	s1 += (ScreenLine & 0x1f) << 5;
	s2 += (ScreenLine & 0x1f) << 5;

		if(BGMode != 4)
		{
			if((ScreenLine & 0x1f) == 0x1f)
			{
				if(ScreenLine & 0x20)
					VOffsetOffset = BPS0 - BPS2 - 0x1f*32;
				else
					VOffsetOffset = BPS2 - BPS0 - 0x1f*32;
			}
			else
			{
				VOffsetOffset = 32;
			}
		}

	int clipcount = GPUPack.GFX.pCurrentClip->Count [bg];
	if (!clipcount)
	    clipcount = 1;

	for (int clip = 0; clip < clipcount; clip++)
	{
	    uint32 Left;
	    uint32 Right;

	    if (!GPUPack.GFX.pCurrentClip->Count [bg])
	    {
		Left = 0;
		Right = 256;
	    }
	    else
	    {
		Left = GPUPack.GFX.pCurrentClip->Left [clip][bg];
		Right = GPUPack.GFX.pCurrentClip->Right [clip][bg];

		if (Right <= Left)
		    continue;
	    }

	    uint32 VOffset;
	    uint32 HOffset;
			//added:
			uint32 LineHOffset=LineData [Y].BG[bg].HOffset;

	    uint32 Offset;
	    uint32 HPos;
	    uint32 Quot;
	    uint32 Count;
	    uint16 *t;
	    uint32 Quot2;
	    uint32 VCellOffset;
	    uint32 HCellOffset;
	    uint16 *b1;
	    uint16 *b2;
	    uint32 TotalCount = 0;
	    uint32 MaxCount = 8;

	    uint32 s = Left * 1 + Y * GPUPack.GFX.PPL;
	    bool8 left_hand_edge = (Left == 0);
	    Width = Right - Left;

	    if (Left & 7)
		MaxCount = 8 - (Left & 7);

	    while (Left < Right)
	    {
		if (left_hand_edge)
		{
		    // The SNES offset-per-tile background mode has a
		    // hardware limitation that the offsets cannot be set
		    // for the tile at the left-hand edge of the screen.
		    VOffset = LineData [Y].BG[bg].VOffset;

					//MKendora; use temp var to reduce memory accesses
					//HOffset = LineData [Y].BG[bg].HOffset;

					HOffset = LineHOffset;
					//End MK

		    left_hand_edge = FALSE;
		}
		else

		{
		    // All subsequent offset tile data is shifted left by one,
		    // hence the - 1 below.

		    Quot2 = ((HOff + Left - 1) & OffsetMask) >> 3;

		    if (Quot2 > 31)
			s0 = s2 + (Quot2 & 0x1f);
		    else
			s0 = s1 + Quot2;

		    HCellOffset = READ_2BYTES (s0);

		    if (BGMode == 4)
		    {
			VOffset = LineData [Y].BG[bg].VOffset;

						//MKendora another mem access hack
						//HOffset = LineData [Y].BG[bg].HOffset;
						HOffset=LineHOffset;
						//end MK

			if ((HCellOffset & OffsetEnableMask))
			{
			    if (HCellOffset & 0x8000)
				VOffset = HCellOffset + 1;
			    else
				HOffset = HCellOffset;
			}
		    }
		    else
		    {
			VCellOffset = READ_2BYTES (s0 + VOffsetOffset);
			if ((VCellOffset & OffsetEnableMask))
			    VOffset = VCellOffset + 1;
			else
			    VOffset = LineData [Y].BG[bg].VOffset;

						//MKendora Strike Gunner fix
			if ((HCellOffset & OffsetEnableMask))
						{
							//HOffset= HCellOffset;

							HOffset = (HCellOffset & ~7)|(LineHOffset&7);
							//HOffset |= LineData [Y].BG[bg].HOffset&7;
						}
			else
							HOffset=LineHOffset;
							//HOffset = LineData [Y].BG[bg].HOffset -
							//Settings.StrikeGunnerOffsetHack;
						//HOffset &= (~7);
						//end MK
		    }
		}
		VirtAlign = ((Y + VOffset) & 7) << 3;
		ScreenLine = (VOffset + Y) >> OffsetShift;

		if (((VOffset + Y) & 15) > 7)
		{
		    t1 = 16;
		    t2 = 0;
		}
		else
		{
		    t1 = 0;
		    t2 = 16;
		}

		if (ScreenLine & 0x20)
		    b1 = SC2, b2 = SC3;
		else
		    b1 = SC0, b2 = SC1;

		b1 += (ScreenLine & 0x1f) << 5;
		b2 += (ScreenLine & 0x1f) << 5;

		HPos = (HOffset + Left) & OffsetMask;

		Quot = HPos >> 3;

		if (GPUPack.BG.TileSize == 8)
		{
		    if (Quot > 31)
			t = b2 + (Quot & 0x1f);
		    else
			t = b1 + Quot;
		}
		else
		{
		    if (Quot > 63)
			t = b2 + ((Quot >> 1) & 0x1f);
		    else
			t = b1 + (Quot >> 1);
		}

		if (MaxCount + TotalCount > Width)
		    MaxCount = Width - TotalCount;

		Offset = HPos & 7;

				//Count =1;
		Count = 8 - Offset;
		if (Count > MaxCount)
		    Count = MaxCount;

		s -= Offset * 1;
		Tile = READ_2BYTES(t);
		GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];

		if (GPUPack.BG.TileSize == 8)
		    (*DrawClippedTilePtr) (Tile, s, Offset, Count, VirtAlign, Lines);
		else
		{
		    if (!(Tile & (V_FLIP | H_FLIP)))
		    {
			// Normal, unflipped
			(*DrawClippedTilePtr) (Tile + t1 + (Quot & 1),
					       s, Offset, Count, VirtAlign, Lines);
		    }
		    else
		    if (Tile & H_FLIP)
		    {
			if (Tile & V_FLIP)
			{
			    // H & V flip
			    (*DrawClippedTilePtr) (Tile + t2 + 1 - (Quot & 1),
						   s, Offset, Count, VirtAlign, Lines);
			}
			else
			{
			    // H flip only
			    (*DrawClippedTilePtr) (Tile + t1 + 1 - (Quot & 1),
						   s, Offset, Count, VirtAlign, Lines);
			}
		    }
		    else
		    {
			// V flip only
			(*DrawClippedTilePtr) (Tile + t2 + (Quot & 1),
					       s, Offset, Count, VirtAlign, Lines);
		    }
		}

		Left += Count;
		TotalCount += Count;
		s += (Offset + Count) * 1;
		MaxCount = 8;
	    }
	}
    }
}

void orgDrawBackgroundMode5 (uint32 /* BGMODE */, uint32 bg, uint8 Z1, uint8 
Z2)
{
    CHECK_SOUND();

    GPUPack.GFX.Pitch = GPUPack.GFX.RealPitch;
    GPUPack.GFX.PPL = GPUPack.GFX.PPLx2 >> 1;
    uint8 depths [2] = {Z1, Z2};

    uint32 Tile;
    uint16 *SC0;
    uint16 *SC1;
    uint16 *SC2;
    uint16 *SC3;
    uint32 Width;

    GPUPack.BG.StartPalette = 0;

    SC0 = (uint16 *) &VRAM[PPUPack.PPU.BG[bg].SCBase << 1];

    if ((PPUPack.PPU.BG[bg].SCSize & 1))
	SC1 = SC0 + 1024;
    else
	SC1 = SC0;

	if((SC1-(unsigned short*)VRAM)>0x10000)
		SC1=(uint16*)&VRAM[(((uint8*)SC1)-VRAM)%0x10000];

    if ((PPUPack.PPU.BG[bg].SCSize & 2))
	SC2 = SC1 + 1024;
    else
	SC2 = SC0;

		if((SC2-(unsigned short*)VRAM)>0x10000)
		SC2=(uint16*)&VRAM[(((uint8*)SC2)-VRAM)%0x10000];


    if ((PPUPack.PPU.BG[bg].SCSize & 1))
	SC3 = SC2 + 1024;
    else
	SC3 = SC2;

	if((SC3-(unsigned short*)VRAM)>0x10000)
		SC3=(uint16*)&VRAM[(((uint8*)SC3)-VRAM)%0x10000];


    int Lines;
    int endy = GPUPack.GFX.EndY;

    for (int Y = GPUPack.GFX.StartY; Y <= endy; Y += Lines)
    {
	int y = Y;
	uint32 VOffset = LineData [y].BG[bg].VOffset;
	uint32 HOffset = LineData [y].BG[bg].HOffset;
	int VirtAlign = (Y + VOffset) & 7;

	for (Lines = 1; Lines < 8 - VirtAlign; Lines++)
	    if ((VOffset != LineData [y + Lines].BG[bg].VOffset) ||
		(HOffset != LineData [y + Lines].BG[bg].HOffset))
		break;

	HOffset <<= 1;
	if (Y + Lines > endy)
	    Lines = endy + 1 - Y;
//	VirtAlign <<= 3;

	int ScreenLine = (VOffset + Y) >> (GPUPack.BG.TileSize == 16 ? 4 : 3);
	int t1;
	int t2;
	if (((VOffset + Y) & 15) > 7)
	{
	    t1 = 16;
	    t2 = 0;
	}
	else
	{
	    t1 = 0;
	    t2 = 16;
	}
	uint16 *b1;
	uint16 *b2;

	if (ScreenLine & 0x20)
	    b1 = SC2, b2 = SC3;
	else
	    b1 = SC0, b2 = SC1;

	b1 += (ScreenLine & 0x1f) << 5;
	b2 += (ScreenLine & 0x1f) << 5;

	int clipcount = GPUPack.GFX.pCurrentClip->Count [bg];
	if (!clipcount)
	    clipcount = 1;
	for (int clip = 0; clip < clipcount; clip++)
	{
	    int Left;
	    int Right;

	    if (!GPUPack.GFX.pCurrentClip->Count [bg])
	    {
		Left = 0;
		Right = 512;
	    }
	    else
	    {
		Left = GPUPack.GFX.pCurrentClip->Left [clip][bg]* 2;
		Right = GPUPack.GFX.pCurrentClip->Right [clip][bg] * 2;

		if (Right <= Left)
		    continue;
	    }

	    uint32 s = (Left>>1) /** 1*/ + Y * 256;//GPUPack.GFX.PPL;
	    uint32 HPos = (HOffset + Left /** 1*/) & 0x3ff;

	    uint32 Quot = HPos >> 3;
	    uint32 Count = 0;

	    uint16 *t;
	    if (Quot > 63)
		t = b2 + ((Quot >> 1) & 0x1f);
	    else
		t = b1 + (Quot >> 1);

	    Width = Right - Left;
	    // Left hand edge clipped tile
	    if (HPos & 7)
	    {
		int Offset = (HPos & 7);
		Count = 8 - Offset;
		if (Count > Width)
		    Count = Width;
		s -= Offset>>1;
		Tile = READ_2BYTES (t);
		GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];

		if (GPUPack.BG.TileSize == 8)
		{
		    if (!(Tile & H_FLIP))
		    {
			// Normal, unflipped
			(*DrawHiResClippedTilePtr) (Tile + (Quot & 1),
						    s, Offset, Count, VirtAlign, Lines);
		    }
		    else
		    {
			// H flip
			(*DrawHiResClippedTilePtr) (Tile + 1 - (Quot & 1),
						    s, Offset, Count, VirtAlign, Lines);
		    }
		}
		else
		{
		    if (!(Tile & (V_FLIP | H_FLIP)))
		    {
			// Normal, unflipped
			(*DrawHiResClippedTilePtr) (Tile + t1 + (Quot & 1),
						    s, Offset, Count, VirtAlign, Lines);
		    }
		    else
		    if (Tile & H_FLIP)
		    {
			if (Tile & V_FLIP)
			{
			    // H & V flip
			    (*DrawHiResClippedTilePtr) (Tile + t2 + 1 - (Quot & 1),
							s, Offset, Count, VirtAlign, Lines);
			}
			else
			{
			    // H flip only
			    (*DrawHiResClippedTilePtr) (Tile + t1 + 1 - (Quot & 1),
							s, Offset, Count, VirtAlign, Lines);
			}
		    }
		    else
		    {
			// V flip only
			(*DrawHiResClippedTilePtr) (Tile + t2 + (Quot & 1),
						    s, Offset, Count, VirtAlign, Lines);
		    }
		}

		t += Quot & 1;
		if (Quot == 63)
		    t = b2;
		else if (Quot == 127)
		    t = b1;
		Quot++;
		s += /*8*/4;
	    }

	    // Middle, unclipped tiles
	    Count = Width - Count;
	    int Middle = Count >> 3;
	    Count &= 7;
	    for (int C = Middle; C > 0; s += /*8*/4, Quot++, C--)
	    {
		Tile = READ_2BYTES(t);
		GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];
		if (GPUPack.BG.TileSize == 8)
		{
		    if (!(Tile & H_FLIP))
		    {
			// Normal, unflipped
			(*DrawHiResTilePtr) (Tile + (Quot & 1),
					     s, VirtAlign, Lines);
		    }
		    else
		    {
			// H flip
			(*DrawHiResTilePtr) (Tile + 1 - (Quot & 1),
					    s, VirtAlign, Lines);
		    }
		}
		else
		{
		    if (!(Tile & (V_FLIP | H_FLIP)))
		    {
			// Normal, unflipped
			(*DrawHiResTilePtr) (Tile + t1 + (Quot & 1),
					     s, VirtAlign, Lines);
		    }
		    else
		    if (Tile & H_FLIP)
		    {
			if (Tile & V_FLIP)
			{
			    // H & V flip
			    (*DrawHiResTilePtr) (Tile + t2 + 1 - (Quot & 1),
						 s, VirtAlign, Lines);
			}
			else
			{
			    // H flip only
			    (*DrawHiResTilePtr) (Tile + t1 + 1 - (Quot & 1),
						 s, VirtAlign, Lines);
			}
		    }
		    else
		    {
			// V flip only
			(*DrawHiResTilePtr) (Tile + t2 + (Quot & 1),
					     s, VirtAlign, Lines);
		    }
		}

		t += Quot & 1;
		if (Quot == 63)
		    t = b2;
		else
		if (Quot == 127)
		    t = b1;
	    }

	    // Right-hand edge clipped tiles
	    if (Count)
	    {
		Tile = READ_2BYTES(t);
		GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];
		if (GPUPack.BG.TileSize == 8)
		{
		    if (!(Tile & H_FLIP))
		    {
			// Normal, unflipped
			(*DrawHiResClippedTilePtr) (Tile + (Quot & 1),
						    s, 0, Count, VirtAlign, Lines);
		    }
		    else
		    {
			// H flip
			(*DrawHiResClippedTilePtr) (Tile + 1 - (Quot & 1),
						    s, 0, Count, VirtAlign, Lines);
		    }
		}
		else
		{
		    if (!(Tile & (V_FLIP | H_FLIP)))
		    {
			// Normal, unflipped
			(*DrawHiResClippedTilePtr) (Tile + t1 + (Quot & 1),
						    s, 0, Count, VirtAlign, Lines);
		    }
		    else
		    if (Tile & H_FLIP)
		    {
			if (Tile & V_FLIP)
			{
			    // H & V flip
			    (*DrawHiResClippedTilePtr) (Tile + t2 + 1 - (Quot & 1),
							s, 0, Count, VirtAlign, Lines);
			}
			else
			{
			    // H flip only
			    (*DrawHiResClippedTilePtr) (Tile + t1 + 1 - (Quot & 1),
							s, 0, Count, VirtAlign, Lines);
			}
		    }
		    else
		    {
			// V flip only
			(*DrawHiResClippedTilePtr) (Tile + t2 + (Quot & 1),
						    s, 0, Count, VirtAlign, Lines);
		    }
		}
	    }
	}
    }
    GPUPack.GFX.Pitch = GPUPack.GFX.RealPitch;
    GPUPack.GFX.PPL = GPUPack.GFX.PPLx2 >> 1;
}

void orgDrawBackground (uint32 BGMode, uint32 bg, uint8 Z1, uint8 Z2)
{

    GPUPack.BG.TileSize = BGSizes [PPUPack.PPU.BG[bg].BGSize];
    GPUPack.BG.BitShift = BitShifts[BGMode][bg];
    GPUPack.BG.TileShift = TileShifts[BGMode][bg];
    GPUPack.BG.TileAddress = PPUPack.PPU.BG[bg].NameBase << 1;
    GPUPack.BG.NameSelect = 0;
    GPUPack.BG.Buffer = IPPU.TileCache [Depths [BGMode][bg]];
    GPUPack.BG.Buffered = IPPU.TileCached [Depths [BGMode][bg]];
    GPUPack.BG.PaletteShift = PaletteShifts[BGMode][bg];
    GPUPack.BG.PaletteMask = PaletteMasks[BGMode][bg];
    GPUPack.BG.DirectColourMode = (BGMode == 3 || BGMode == 4) && bg == 0 &&
		          (GPUPack.GFX.r2130 & 1);

    if (PPUPack.PPU.BGMosaic [bg] && PPUPack.PPU.Mosaic > 1)
    {
	orgDrawBackgroundMosaic (BGMode, bg, Z1, Z2);
	return;

    }
    switch (BGMode)
    {
    case 2:
	if (Settings.WrestlemaniaArcade)
	    break;
    case 4: // Used by Puzzle Bobble
       orgDrawBackgroundOffset (BGMode, bg, Z1, Z2);
	return;

    case 5:
    case 6: // XXX: is also offset per tile.
//	if (Settings.SupportHiRes)
	{
	    orgDrawBackgroundMode5 (BGMode, bg, Z1, Z2);
	    return;
	}
	break;
    }
    CHECK_SOUND();

    uint32 Tile;
    uint16 *SC0;
    uint16 *SC1;
    uint16 *SC2;
    uint16 *SC3;
    uint32 Width;
    uint8 depths [2] = {Z1, Z2};

    if (BGMode == 0)
	GPUPack.BG.StartPalette = bg << 5;
    else
	GPUPack.BG.StartPalette = 0;

    SC0 = (uint16 *) &VRAM[PPUPack.PPU.BG[bg].SCBase << 1];

    if (PPUPack.PPU.BG[bg].SCSize & 1)	SC1 = SC0 + 1024;
    else	SC1 = SC0;

	if(SC1>=(unsigned 
short*)(VRAM+0x10000))		SC1=(uint16*)&VRAM[((uint8*)SC1-&VRAM[0])%0x10000];

    if (PPUPack.PPU.BG[bg].SCSize & 2)	SC2 = SC1 + 1024;
    else	SC2 = SC0;

		if((SC2-(unsigned short*)VRAM)>0x10000)		SC2-=0x10000;


    if (PPUPack.PPU.BG[bg].SCSize & 1)	SC3 = SC2 + 1024;
    else	SC3 = SC2;

	if((SC3-(unsigned short*)VRAM)>0x10000)		SC3-=0x10000;


    int Lines;
    int OffsetMask;
    int OffsetShift;

    if (GPUPack.BG.TileSize == 16)
    {
	OffsetMask = 0x3ff;
	OffsetShift = 4;
    }
    else
    {
	OffsetMask = 0x1ff;
	OffsetShift = 3;
    }

    for (uint32 Y = GPUPack.GFX.StartY; Y <= GPUPack.GFX.EndY; Y += Lines)
    {
	uint32 VOffset = LineData [Y].BG[bg].VOffset;
	uint32 HOffset = LineData [Y].BG[bg].HOffset;
	int VirtAlign = (Y + VOffset) & 7;

	for (Lines = 1; Lines < 8 - VirtAlign; Lines++)
	    if ((VOffset != LineData [Y + Lines].BG[bg].VOffset) ||
		(HOffset != LineData [Y + Lines].BG[bg].HOffset))
		break;

	if (Y + Lines > GPUPack.GFX.EndY)
	    Lines = GPUPack.GFX.EndY + 1 - Y;

	VirtAlign <<= 3;

	uint32 ScreenLine = (VOffset + Y) >> OffsetShift;
	uint32 t1;
	uint32 t2;
	if (((VOffset + Y) & 15) > 7)
	{
	    t1 = 16;
	    t2 = 0;
	}
	else
	{
	    t1 = 0;
	    t2 = 16;
	}
	uint16 *b1;
	uint16 *b2;

	if (ScreenLine & 0x20)
	    b1 = SC2, b2 = SC3;
	else
	    b1 = SC0, b2 = SC1;

	b1 += (ScreenLine & 0x1f) << 5;
	b2 += (ScreenLine & 0x1f) << 5;

	int clipcount = GPUPack.GFX.pCurrentClip->Count [bg];
	if (!clipcount)
	    clipcount = 1;
	for (int clip = 0; clip < clipcount; clip++)
	{
	    uint32 Left;
	    uint32 Right;

	    if (!GPUPack.GFX.pCurrentClip->Count [bg])
	    {
		Left = 0;
		Right = 256;
	    }
	    else
	    {
		Left = GPUPack.GFX.pCurrentClip->Left [clip][bg];
		Right = GPUPack.GFX.pCurrentClip->Right [clip][bg];

		if (Right <= Left)
		    continue;
	    }
	    

	    uint32 s = Left * 1 + Y * GPUPack.GFX.PPL;
	    uint32 HPos = (HOffset + Left) & OffsetMask;

	    uint32 Quot = HPos >> 3;
	    uint32 Count = 0;

	    uint16 *t;
	    if (GPUPack.BG.TileSize == 8)
	    {
		if (Quot > 31)
		    t = b2 + (Quot & 0x1f);
		else
		    t = b1 + Quot;
	    }
	    else
	    {
		if (Quot > 63)
		    t = b2 + ((Quot >> 1) & 0x1f);
		else
		    t = b1 + (Quot >> 1);
	    }

	    Width = Right - Left;
	    // Left hand edge clipped tile
	    if (HPos & 7)
	    {
		uint32 Offset = (HPos & 7);
		Count = 8 - Offset;
		if (Count > Width)
		    Count = Width;
		s -= Offset * 1;
		Tile = READ_2BYTES(t);
		GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];

		if (GPUPack.BG.TileSize == 8)
		{
		    (*DrawClippedTilePtr) (Tile, s, Offset, Count, VirtAlign,
					   Lines);
		}
		else
		{
		    if (!(Tile & (V_FLIP | H_FLIP)))
		    {
			// Normal, unflipped
			(*DrawClippedTilePtr) (Tile + t1 + (Quot & 1),
					       s, Offset, Count, VirtAlign, Lines);
		    }
		    else
		    if (Tile & H_FLIP)
		    {
			if (Tile & V_FLIP)
			{
			    // H & V flip
			    (*DrawClippedTilePtr) (Tile + t2 + 1 - (Quot & 1),
						   s, Offset, Count, VirtAlign, Lines);
			}
			else
			{
			    // H flip only
			    (*DrawClippedTilePtr) (Tile + t1 + 1 - (Quot & 1),
						   s, Offset, Count, VirtAlign, Lines);
			}
		    }
		    else
		    {
			// V flip only
			(*DrawClippedTilePtr) (Tile + t2 + (Quot & 1), s,
					       Offset, Count, VirtAlign, Lines);
		    }
		}

		if (GPUPack.BG.TileSize == 8)
		{
		    t++;
		    if (Quot == 31)
			t = b2;
		    else if (Quot == 63)
			t = b1;
		}
		else
		{
		    t += Quot & 1;
		    if (Quot == 63)
			t = b2;
		    else if (Quot == 127)
			t = b1;
		}
		Quot++;
		s += 8 * 1;
	    }

	    // Middle, unclipped tiles
	    Count = Width - Count;
	    int Middle = Count >> 3;
	    Count &= 7;
	    for (int C = Middle; C > 0; s += 8 * 1, Quot++, C--)
	    {
		Tile = READ_2BYTES(t);
		GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];

		if (GPUPack.BG.TileSize != 8)
		{
		    if (Tile & H_FLIP)
		    {
			// Horizontal flip, but what about vertical flip ?
			if (Tile & V_FLIP)
			{
			    // Both horzontal & vertical flip
			    (*DrawTilePtr) (Tile + t2 + 1 - (Quot & 1), s,
					    VirtAlign, Lines);
			}
			else
			{
			    // Horizontal flip only
			    (*DrawTilePtr) (Tile + t1 + 1 - (Quot & 1), s,
					    VirtAlign, Lines);
			}
		    }
		    else
		    {
			// No horizontal flip, but is there a vertical flip ?
			if (Tile & V_FLIP)
			{
			    // Vertical flip only
			    (*DrawTilePtr) (Tile + t2 + (Quot & 1), s,
					    VirtAlign, Lines);
			}
			else
			{
			    // Normal unflipped
			    (*DrawTilePtr) (Tile + t1 + (Quot & 1), s,
					    VirtAlign, Lines);
			}
		    }
		}
		else
		{
		    (*DrawTilePtr) (Tile, s, VirtAlign, Lines);
		}

		if (GPUPack.BG.TileSize == 8)
		{
		    t++;
		    if (Quot == 31)
			t = b2;
		    else
		    if (Quot == 63)
			t = b1;
		}
		else
		{
		    t += Quot & 1;
		    if (Quot == 63)
			t = b2;
		    else
		    if (Quot == 127)
			t = b1;
		}
	    }
	    // Right-hand edge clipped tiles
	    if (Count)
	    {
		Tile = READ_2BYTES(t);
		GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];

		if (GPUPack.BG.TileSize == 8)
		    (*DrawClippedTilePtr) (Tile, s, 0, Count, VirtAlign,
					   Lines);
		else
		{
		    if (!(Tile & (V_FLIP | H_FLIP)))
		    {
			// Normal, unflipped
			(*DrawClippedTilePtr) (Tile + t1 + (Quot & 1), s, 0,
					       Count, VirtAlign, Lines);
		    }
		    else
		    if (Tile & H_FLIP)
		    {
			if (Tile & V_FLIP)
			{
			    // H & V flip
			    (*DrawClippedTilePtr) (Tile + t2 + 1 - (Quot & 1),
						   s, 0, Count, VirtAlign,
						   Lines);
			}
			else
			{
			    // H flip only
			    (*DrawClippedTilePtr) (Tile + t1 + 1 - (Quot & 1),
						   s, 0, Count, VirtAlign,
						   Lines);
			}
		    }
		    else
		    {
			// V flip only
			(*DrawClippedTilePtr) (Tile + t2 + (Quot & 1),
					       s, 0, Count, VirtAlign,
					       Lines);
		    }
		}
	    }
	}
    }
}

#define RENDER_BACKGROUND_MODE7(TYPE,FUNC) \
    CHECK_SOUND(); \
\
    uint8 *VRAM1 = VRAM + 1; \
    if (GPUPack.GFX.r2130 & 1) \
    { \
	if (IPPU.DirectColourMapsNeedRebuild) \
	    S9xBuildDirectColourMaps (); \
	GPUPack.GFX.ScreenColors = DirectColourMaps [0]; \
    } \
    else \
	GPUPack.GFX.ScreenColors = IPPU.ScreenColors; \
\
    int aa, cc; \
    int dir; \
    int startx, endx; \
    uint32 Left = 0; \
    uint32 Right = 256; \
    uint32 ClipCount = GPUPack.GFX.pCurrentClip->Count [bg]; \
\
    if (!ClipCount) \
	ClipCount = 1; \
\
    Screen += GPUPack.GFX.StartY * GPUPack.GFX.Pitch; \
    uint8 *Depth = GPUPack.GFX.DB + GPUPack.GFX.StartY * GPUPack.GFX.PPL; \
    struct SLineMatrixData *l = &LineMatrixData [GPUPack.GFX.StartY]; \
\
    for (uint32 Line = GPUPack.GFX.StartY; Line <= GPUPack.GFX.EndY; Line++, Screen += GPUPack.GFX.Pitch, Depth += GPUPack.GFX.PPL, l++) \
    { \
	int yy; \
\
	int32 HOffset = ((int32) LineData [Line].BG[0].HOffset << M7) >> M7; \
	int32 VOffset = ((int32) LineData [Line].BG[0].VOffset << M7) >> M7; \
\
	int32 CentreX = ((int32) l->CentreX << M7) >> M7; \
	int32 CentreY = ((int32) l->CentreY << M7) >> M7; \
\
	if (PPUPack.PPU.Mode7VFlip) \
	    yy = 261 - (int) Line; \
	else \
	    yy = Line; \
\
	if (PPUPack.PPU.Mode7Repeat == 0) \
	    yy += (VOffset - CentreY) % 1023; \
	else \
	    yy += VOffset - CentreY; \
	int BB = l->MatrixB * yy + (CentreX << 8); \
	int DD = l->MatrixD * yy + (CentreY << 8); \
\
	for (uint32 clip = 0; clip < ClipCount; clip++) \
	{ \
	    if (GPUPack.GFX.pCurrentClip->Count [bg]) \
	    { \
		Left = GPUPack.GFX.pCurrentClip->Left [clip][bg]; \
		Right = GPUPack.GFX.pCurrentClip->Right [clip][bg]; \
		if (Right <= Left) \
		    continue; \
	    } \
	    TYPE *p = (TYPE *) Screen + Left; \
	    uint8 *d = Depth + Left; \
\
	    if (PPUPack.PPU.Mode7HFlip) \
	    { \
		startx = Right - 1; \
		endx = Left - 1; \
		dir = -1; \
		aa = -l->MatrixA; \
		cc = -l->MatrixC; \
	    } \
	    else \
	    { \
		startx = Left; \
		endx = Right; \
		dir = 1; \
		aa = l->MatrixA; \
		cc = l->MatrixC; \
	    } \
	    int xx; \
	    if (PPUPack.PPU.Mode7Repeat == 0) \
		xx = startx + (HOffset - CentreX) % 1023; \
	    else \
		xx = startx + HOffset - CentreX; \
	    int AA = l->MatrixA * xx; \
	    int CC = l->MatrixC * xx; \
		AA += BB;\
		CC += DD;\
		\
	    if (!PPUPack.PPU.Mode7Repeat) \
	    { \
	    	if (cc == 0) {\
				register uint8* AdrY;\
				register int AdrY2;\
				int Y = CC>>4; AdrY = &VRAM[((Y & 0x3f80) << 1)]; AdrY2 = (Y & 0x70);\
\
				for (int x = startx; x != endx; x += dir, AA += aa, p++, d++) \
				{ \
				    int X = (AA >> 7); \
				    uint8 *TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
				    uint32 b = *(TileData + AdrY2 + (X & 14)); \
				    GPUPack.GFX.Z1 = Mode7Depths [(b & GPUPack.GFX.Mode7PriorityMask) >> 7]; \
				    if (GPUPack.GFX.Z1 > *d && (b & GPUPack.GFX.Mode7Mask) ) \
				    { \
						*p = (FUNC); \
						*d = GPUPack.GFX.Z1; \
				    } \
				} \
      		} else {\
				for (int x = startx; x != endx; x += dir, AA += aa, CC += cc, p++, d++) \
				{ \
				  	int X = AA >> 7;\
				  	int Y = CC >> 4;\
			  		uint8* TileData = VRAM1 + (VRAM[((Y & 0x3f80) << 1) + ((X >> 3) & 0xfe)] << 7); \
					uint32 b = *(TileData + (Y & 0x70) + (X & 14)); \
				    GPUPack.GFX.Z1 = Mode7Depths [(b & GPUPack.GFX.Mode7PriorityMask) >> 7]; \
				    if (GPUPack.GFX.Z1 > *d && (b & GPUPack.GFX.Mode7Mask) ) \
				    { \
						*p = (FUNC); \
						*d = GPUPack.GFX.Z1; \
				    } \
				} \
        	}\
	    } \
	    else \
	    { \
			if (Settings.Dezaemon && PPUPack.PPU.Mode7Repeat == 2) \
				for (int x = startx; x != endx; x += dir, AA += aa, CC += cc, p++, d++) \
				{ \
				    int X = (AA >> 8) & 0x7ff; \
				    int Y = (CC >> 8) & 0x7ff; \
		\
				    if (((X | Y) & ~0x3ff) == 0) \
				    { \
					uint8 *TileData = VRAM1 + (VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
					uint32 b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
					GPUPack.GFX.Z1 = Mode7Depths [(b & GPUPack.GFX.Mode7PriorityMask) >> 7]; \
					if (GPUPack.GFX.Z1 > *d && (b & GPUPack.GFX.Mode7Mask) ) \
					{ \
					    *p = (FUNC); \
					    *d = GPUPack.GFX.Z1; \
					} \
				    } \
				    else \
				    { \
						if (PPUPack.PPU.Mode7Repeat == 3) \
						{ \
						    X = (x + HOffset) & 7; \
						    Y = (yy + CentreY) & 7; \
						    uint32 b = *(VRAM1 + ((Y & 7) << 4) + ((X & 7) << 1)); \
						    GPUPack.GFX.Z1 = Mode7Depths [(b & GPUPack.GFX.Mode7PriorityMask) >> 7]; \
						    if (GPUPack.GFX.Z1 > *d && (b & GPUPack.GFX.Mode7Mask) ) \
						    { \
							*p = (FUNC); \
							*d = GPUPack.GFX.Z1; \
						    } \
						} \
				    } \
				} \
			else\
				for (int x = startx; x != endx; x += dir, AA += aa, CC += cc, p++, d++) \
				{ \
				    int X = (AA >> 8); \
				    int Y = (CC >> 8); \
		\
				    if (((X | Y) & ~0x3ff) == 0) \
				    { \
					uint8 *TileData = VRAM1 + (VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
					uint32 b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
					GPUPack.GFX.Z1 = Mode7Depths [(b & GPUPack.GFX.Mode7PriorityMask) >> 7]; \
					if (GPUPack.GFX.Z1 > *d && (b & GPUPack.GFX.Mode7Mask) ) \
					{ \
					    *p = (FUNC); \
					    *d = GPUPack.GFX.Z1; \
					} \
				    } \
				    else \
				    { \
						if (PPUPack.PPU.Mode7Repeat == 3) \
						{ \
						    X = (x + HOffset) & 7; \
						    Y = (yy + CentreY) & 7; \
						    uint32 b = *(VRAM1 + ((Y & 7) << 4) + ((X & 7) << 1)); \
						    GPUPack.GFX.Z1 = Mode7Depths [(b & GPUPack.GFX.Mode7PriorityMask) >> 7]; \
						    if (GPUPack.GFX.Z1 > *d && (b & GPUPack.GFX.Mode7Mask) ) \
						    { \
							*p = (FUNC); \
							*d = GPUPack.GFX.Z1; \
						    } \
						} \
				    } \
				} \
	    } \
	} \
    }

void orgDrawBGMode7Background16 (uint8 *Screen, int bg)
{
    RENDER_BACKGROUND_MODE7 (uint16, GPUPack.GFX.ScreenColors [b & GPUPack.GFX.Mode7Mask]);
}

void orgDrawBGMode7Background16Add (uint8 *Screen, int bg)
{
    RENDER_BACKGROUND_MODE7 (uint16, *(d + GPUPack.GFX.DepthDelta) ?
					(*(d + GPUPack.GFX.DepthDelta) != 1 ?
					    COLOR_ADD (GPUPack.GFX.ScreenColors [b & GPUPack.GFX.Mode7Mask],
						       p [GPUPack.GFX.Delta]) :
					    COLOR_ADD (GPUPack.GFX.ScreenColors [b & GPUPack.GFX.Mode7Mask],
						       GPUPack.GFX.FixedColour)) :
					 GPUPack.GFX.ScreenColors [b & GPUPack.GFX.Mode7Mask]);
}

void orgDrawBGMode7Background16Add1_2 (uint8 *Screen, int bg)
{
    RENDER_BACKGROUND_MODE7 (uint16, *(d + GPUPack.GFX.DepthDelta) ?
					(*(d + GPUPack.GFX.DepthDelta) != 1 ?
					    COLOR_ADD1_2 (GPUPack.GFX.ScreenColors [b & GPUPack.GFX.Mode7Mask],
						       p [GPUPack.GFX.Delta]) :
					    COLOR_ADD (GPUPack.GFX.ScreenColors [b & GPUPack.GFX.Mode7Mask],
						       GPUPack.GFX.FixedColour)) :
					 GPUPack.GFX.ScreenColors [b & GPUPack.GFX.Mode7Mask]);
}

void orgDrawBGMode7Background16Sub (uint8 *Screen, int bg)
{
    RENDER_BACKGROUND_MODE7 (uint16, *(d + GPUPack.GFX.DepthDelta) ?
					(*(d + GPUPack.GFX.DepthDelta) != 1 ?
					    COLOR_SUB (GPUPack.GFX.ScreenColors [b & GPUPack.GFX.Mode7Mask],
						       p [GPUPack.GFX.Delta]) :
					    COLOR_SUB (GPUPack.GFX.ScreenColors [b & GPUPack.GFX.Mode7Mask],
						       GPUPack.GFX.FixedColour)) :
					 GPUPack.GFX.ScreenColors [b & GPUPack.GFX.Mode7Mask]);
}

void orgDrawBGMode7Background16Sub1_2 (uint8 *Screen, int bg)
{
    RENDER_BACKGROUND_MODE7 (uint16, *(d + GPUPack.GFX.DepthDelta) ?
					(*(d + GPUPack.GFX.DepthDelta) != 1 ?
					    COLOR_SUB1_2 (GPUPack.GFX.ScreenColors [b & GPUPack.GFX.Mode7Mask],
						       p [GPUPack.GFX.Delta]) :
					    COLOR_SUB (GPUPack.GFX.ScreenColors [b & GPUPack.GFX.Mode7Mask],
						       GPUPack.GFX.FixedColour)) :
					 GPUPack.GFX.ScreenColors [b & GPUPack.GFX.Mode7Mask]);
}


#define _BUILD_SETUP(F) \
GPUPack.GFX.BuildPixel = BuildPixel##F; \
GPUPack.GFX.BuildPixel2 = BuildPixel2##F; \
GPUPack.GFX.DecomposePixel = DecomposePixel##F; \
RED_LOW_BIT_MASK = RED_LOW_BIT_MASK_##F; \
GREEN_LOW_BIT_MASK = GREEN_LOW_BIT_MASK_##F; \
BLUE_LOW_BIT_MASK = BLUE_LOW_BIT_MASK_##F; \
RED_HI_BIT_MASK = RED_HI_BIT_MASK_##F; \
GREEN_HI_BIT_MASK = GREEN_HI_BIT_MASK_##F; \
BLUE_HI_BIT_MASK = BLUE_HI_BIT_MASK_##F; \
MAX_RED = MAX_RED_##F; \
MAX_GREEN = MAX_GREEN_##F; \
MAX_BLUE = MAX_BLUE_##F; \
GREEN_HI_BIT = ((MAX_GREEN_##F + 1) >> 1); \
SPARE_RGB_BIT_MASK = SPARE_RGB_BIT_MASK_##F; \
RGB_LOW_BITS_MASK = (RED_LOW_BIT_MASK_##F | \
		     GREEN_LOW_BIT_MASK_##F | \
		     BLUE_LOW_BIT_MASK_##F); \
RGB_HI_BITS_MASK = (RED_HI_BIT_MASK_##F | \
		    GREEN_HI_BIT_MASK_##F | \
		    BLUE_HI_BIT_MASK_##F); \
RGB_HI_BITS_MASKx2 = ((RED_HI_BIT_MASK_##F | \
		       GREEN_HI_BIT_MASK_##F | \
		       BLUE_HI_BIT_MASK_##F) << 1); \
RGB_REMOVE_LOW_BITS_MASK = ~RGB_LOW_BITS_MASK; \
FIRST_COLOR_MASK = FIRST_COLOR_MASK_##F; \
SECOND_COLOR_MASK = SECOND_COLOR_MASK_##F; \
THIRD_COLOR_MASK = THIRD_COLOR_MASK_##F; \
ALPHA_BITS_MASK = ALPHA_BITS_MASK_##F; \
FIRST_THIRD_COLOR_MASK = FIRST_COLOR_MASK | THIRD_COLOR_MASK; \
TWO_LOW_BITS_MASK = RGB_LOW_BITS_MASK | (RGB_LOW_BITS_MASK << 1); \
HIGH_BITS_SHIFTED_TWO_MASK = (( (FIRST_COLOR_MASK | SECOND_COLOR_MASK | THIRD_COLOR_MASK) & \
                                ~TWO_LOW_BITS_MASK ) >> 2);

void RenderScreen (uint8 *Screen, bool8 sub, bool8 force_no_add, uint8 D)
{
    bool8 BG0;
    bool8 BG1;
    bool8 BG2;
    bool8 BG3;
    bool8 OB;

    GPUPack.GFX.S = Screen;

    if (!sub)
    {
	GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];
	BG0 = ON_MAIN (0) &&  os9x_BG0;
	BG1 = ON_MAIN (1) &&  os9x_BG1;
	BG2 = ON_MAIN (2) &&  os9x_BG2;
	BG3 = ON_MAIN (3) &&  os9x_BG3;
	OB  = ON_MAIN (4) &&  os9x_OBJ;
    }
    else
    {
	GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];
	BG0 = ON_SUB (0) &&  os9x_BG0;
	BG1 = ON_SUB (1) &&  os9x_BG1;
	BG2 = ON_SUB (2) &&  os9x_BG2;
	BG3 = ON_SUB (3) &&  os9x_BG3;
	OB  = ON_SUB (4) &&  os9x_OBJ;
    }

    sub |= force_no_add;

    if (PPUPack.PPU.BGMode <= 1)
    {

	if (OB)
	{
	    orgSelectTileRenderer (sub || !SUB_OR_ADD(4));
	    orgDrawOBJS (!sub, D);
	}
	if (BG0)
	{

	    orgSelectTileRenderer (sub || !SUB_OR_ADD(0));
	    orgDrawBackground (PPUPack.PPU.BGMode, 0, D + 10, D + 14);
	}
	if (BG1)
	{
	    orgSelectTileRenderer (sub || !SUB_OR_ADD(1));
	    orgDrawBackground (PPUPack.PPU.BGMode, 1, D + 9, D + 13);
	}
	if (BG2)
	{
	    orgSelectTileRenderer (sub || !SUB_OR_ADD(2));
	    orgDrawBackground (PPUPack.PPU.BGMode, 2, D + 3, (ROM_GLOBAL [0x2105] & 8) == 0 ? 
D + 6 : D + 17);
	}
	if (BG3 && PPUPack.PPU.BGMode == 0)
	{
	    orgSelectTileRenderer (sub || !SUB_OR_ADD(3));
	    orgDrawBackground (PPUPack.PPU.BGMode, 3, D + 2, D + 5);
	}
    }
    else if (PPUPack.PPU.BGMode != 7)
    {

	if (OB)
	{
	    orgSelectTileRenderer (sub || !SUB_OR_ADD(4));
	    orgDrawOBJS (!sub, D);
	}
	if (BG0)
	{

	    orgSelectTileRenderer (sub || !SUB_OR_ADD(0));
	    orgDrawBackground (PPUPack.PPU.BGMode, 0, D + 5, D + 13);
	}
	if (PPUPack.PPU.BGMode != 6 && BG1)
	{
	    orgSelectTileRenderer (sub || !SUB_OR_ADD(1));
	    orgDrawBackground (PPUPack.PPU.BGMode, 1, D + 2, D + 9);
	}
    }
    else
    {

	if (OB)
	{
	    orgSelectTileRenderer (sub || !SUB_OR_ADD(4));
	    orgDrawOBJS (!sub, D);
	}
	if (BG0 || ((ROM_GLOBAL [0x2133] & 0x40) && BG1))
	{
	    int bg;

	    if (ROM_GLOBAL [0x2133] & 0x40)
	    {
		GPUPack.GFX.Mode7Mask = 0x7f;
		GPUPack.GFX.Mode7PriorityMask = 0x80;
		Mode7Depths [0] = 5 + D;
		Mode7Depths [1] = 9 + D;
		bg = 1;
	    }
	    else
	    {
		GPUPack.GFX.Mode7Mask = 0xff;
		GPUPack.GFX.Mode7PriorityMask = 0;
		Mode7Depths [0] = 5 + D;
		Mode7Depths [1] = 5 + D;
		bg = 0;
	    }
	    if (sub || !SUB_OR_ADD(0))
	    {

		    orgDrawBGMode7Background16 (Screen, bg);
	    }
	    else
	    {
		if (GPUPack.GFX.r2131 & 0x80)
		{
		    if (GPUPack.GFX.r2131 & 0x40)
		    {
			    orgDrawBGMode7Background16Sub1_2 (Screen, bg);
		    }
		    else
		    {
			    orgDrawBGMode7Background16Sub (Screen, bg);
		    }
		}
		else
		{
		    if (GPUPack.GFX.r2131 & 0x40)
		    {
			    orgDrawBGMode7Background16Add1_2 (Screen, bg);
		    }
		    else
		    {

			    orgDrawBGMode7Background16Add (Screen, bg);
		    }
		}
	    }
	}
    }
}


void orgS9xUpdateScreen ()
{
  int32 x2 = 1;

  /*GPUPack.GFX.S = GPUPack.GFX.Screen;
  GPUPack.GFX.r2131 = ROM_GLOBAL [0x2131];
  GPUPack.GFX.r212c = ROM_GLOBAL [0x212c];
  GPUPack.GFX.r212d = ROM_GLOBAL [0x212d];
  GPUPack.GFX.r2130 = ROM_GLOBAL [0x2130];
  GPUPack.GFX.Pseudo = (ROM_GLOBAL [0x2133] & 8) != 0 &&
    (GPUPack.GFX.r212c & 15) != (GPUPack.GFX.r212d & 15) &&
    (GPUPack.GFX.r2131 & 0x3f) == 0;

  if (IPPU.OBJChanged)
    S9xSetupOBJ ();

  if (PPUPack.PPU.RecomputeClipWindows)
    {
      ComputeClipWindows ();
      PPUPack.PPU.RecomputeClipWindows = FALSE;
    }

  GPUPack.GFX.StartY = IPPU.PreviousLine;
  if ((GPUPack.GFX.EndY = IPPU.CurrentLine - 1) >= PPUPack.PPU.ScreenHeight)
    GPUPack.GFX.EndY = PPUPack.PPU.ScreenHeight - 1;
*/
	uint32 starty = GPUPack.GFX.StartY;
	uint32 endy = GPUPack.GFX.EndY;

	uint32 black = BLACK | (BLACK << 16);



	if (GPUPack.GFX.Pseudo)
	{
		GPUPack.GFX.r2131 = 0x5f;
		GPUPack.GFX.r212d = (ROM_GLOBAL [0x212c] ^
		ROM_GLOBAL [0x212d]) & 15;
		GPUPack.GFX.r212c &= ~GPUPack.GFX.r212d;
		GPUPack.GFX.r2130 |= 2;
	}

    if (!os9x_easy && !PPUPack.PPU.ForcedBlanking && ADD_OR_SUB_ON_ANYTHING &&
		(GPUPack.GFX.r2130 & 0x30) != 0x30 &&
		!((GPUPack.GFX.r2130 & 0x30) == 0x10 && IPPU.Clip[1].Count[5] == 0))
	{

		struct ClipData *pClip;

		GPUPack.GFX.FixedColour = BUILD_PIXEL (IPPU.XB [PPUPack.PPU.FixedColourRed],
					 IPPU.XB [PPUPack.PPU.FixedColourGreen],
					 IPPU.XB [PPUPack.PPU.FixedColourBlue]);

		// Clear the z-buffer, marking areas 'covered' by the fixed
		// colour as depth 1.
		pClip = &IPPU.Clip [1];

		// Clear the z-buffer
		if (pClip->Count [5])
	    {
	      // Colour window enabled.
			uint8* pSubZ=GPUPack.GFX.SubZBuffer + starty * GPUPack.GFX.ZPitch;
			uint8* pZ=GPUPack.GFX.ZBuffer + starty * GPUPack.GFX.ZPitch;
			uint8* pSub=GPUPack.GFX.SubScreen + starty * GPUPack.GFX.Pitch2;
			for (uint32 y = starty; y <= endy; y++,pSubZ+=GPUPack.GFX.ZPitch,pZ+=GPUPack.GFX.ZPitch,pSub+=GPUPack.GFX.Pitch2) 
			{
				ZeroMemory (pSubZ,IPPU.RenderedScreenWidth);
				ZeroMemory (pZ,IPPU.RenderedScreenWidth);
				if (IPPU.Clip [0].Count [5])
                {
					uint32 *p = (uint32 *) (pSub);
					uint32 *q = (uint32 *) ((uint16 *) p + IPPU.RenderedScreenWidth);
					while (p < q) *p++ = black;
                }
				for (uint32 c = 0; c < pClip->Count [5]; c++)
				{
					if (pClip->Right [c][5] > pClip->Left [c][5])
					{
						memset (pSubZ + pClip->Left [c][5] * x2,1, 
							(pClip->Right [c][5] - pClip->Left [c][5]) * x2);
						if (IPPU.Clip [0].Count [5])
						{
							// Blast, have to clear the sub-screen to the fixed-colour
							// because there is a colour window in effect clipping
							// the main screen that will allow the sub-screen
							// 'underneath' to show through.

							uint16 *p = (uint16 *) (pSub);
							uint16 *q = p + pClip->Right [c][5] * x2;
							p += pClip->Left [c][5] * x2;

							while (p < q) *p++ = (uint16) GPUPack.GFX.FixedColour;
						}
					}
				}
			}

	    }
		else
		{
			uint8* pSubZ=GPUPack.GFX.SubZBuffer + starty * GPUPack.GFX.ZPitch;
			uint8* pZ=GPUPack.GFX.ZBuffer + starty * GPUPack.GFX.ZPitch;
			uint8* pSub=GPUPack.GFX.SubScreen + starty * GPUPack.GFX.Pitch2;
			uint32 b = GPUPack.GFX.FixedColour | (GPUPack.GFX.FixedColour << 16);
			for (uint32 y = starty; y <= endy; y++,pSubZ+=GPUPack.GFX.ZPitch,pZ+=GPUPack.GFX.ZPitch,pSub+=GPUPack.GFX.Pitch2)
			{
				ZeroMemory (pZ,IPPU.RenderedScreenWidth);
				memset (pSubZ, 1,IPPU.RenderedScreenWidth);

				if (IPPU.Clip [0].Count [5])
				{
					// Blast, have to clear the sub-screen to the fixed-colour
					// because there is a colour window in effect clipping
					// the main screen that will allow the sub-screen
					// 'underneath' to show through.
					uint32 *p = (uint32 *) (pSub);
					uint32 *q = (uint32 *) ((uint16 *) p + IPPU.RenderedScreenWidth);

					while (p < q)	*p++ = b;
				}
			}
		}
		if (ANYTHING_ON_SUB)
	    {
			GPUPack.GFX.DB = GPUPack.GFX.SubZBuffer;
			RenderScreen (GPUPack.GFX.SubScreen, TRUE, TRUE, SUB_SCREEN_DEPTH);
	    }

		if (IPPU.Clip [0].Count [5])
	    {

			uint8* pSubZ=GPUPack.GFX.SubZBuffer + starty * GPUPack.GFX.ZPitch;
			uint8* pS=GPUPack.GFX.Screen + starty * GPUPack.GFX.Pitch2;
			for (uint32 y = starty; y <= endy; y++,pSubZ+=GPUPack.GFX.ZPitch,pS+=GPUPack.GFX.Pitch2)
			{
				register uint16 *p = (uint16 *) (pS);
				register uint8 *d = pSubZ;
				register uint8 *e = d + IPPU.RenderedScreenWidth;

				while (d < e)
				{
					if (*d > 1)
						*p = *(p + GPUPack.GFX.Delta);
					else
						*p = BLACK;
					d++;
					p++;
				}
			}
	    }



		GPUPack.GFX.DB = GPUPack.GFX.ZBuffer;
		RenderScreen (GPUPack.GFX.Screen, FALSE, FALSE, MAIN_SCREEN_DEPTH);





		if (SUB_OR_ADD(5))
	    {
			uint32 back = IPPU.ScreenColors [0];
			uint32 Left = 0;
			uint32 Right = 256;
			uint32 Count;

			pClip = &IPPU.Clip [0];
			for (uint32 y = starty; y <= endy; y++)
			{
				if (!(Count = pClip->Count [5]))
				{
					Left = 0;
					Right = 256 * x2;
					Count = 1;
				}

				for (uint32 b = 0; b < Count; b++)
				{
					if (pClip->Count [5])
					{
						Left = pClip->Left [b][5] * x2;
						Right = pClip->Right [b][5] * x2;
						if (Right <= Left)
							continue;
					}

		      if (GPUPack.GFX.r2131 & 0x80)
			{
			  if (GPUPack.GFX.r2131 & 0x40)
			    {
			      // Subtract, halving the result.
			      register uint16 *p = (uint16 *) (GPUPack.GFX.Screen + y * GPUPack.GFX.Pitch2) + Left;
			      register uint8 *d = GPUPack.GFX.ZBuffer + y * GPUPack.GFX.ZPitch;
			      register uint8 *s = GPUPack.GFX.SubZBuffer + y * GPUPack.GFX.ZPitch + Left;
			      register uint8 *e = d + Right;
			      uint16 back_fixed = COLOR_SUB (back, GPUPack.GFX.FixedColour);

			      d += Left;
			      while (d < e)
				{
				  if (*d == 0)
				    {
				      if (*s)
					{
					  if (*s != 1)
					    *p = COLOR_SUB1_2 (back, *(p + GPUPack.GFX.Delta));
					  else
					    *p = back_fixed;
					}
				      else
					*p = (uint16) back;
				    }
				  d++;
				  p++;
				  s++;
				}
			    }
			  else
			    {
			      // Subtract
			      register uint16 *p = (uint16 *) (GPUPack.GFX.Screen + y * GPUPack.GFX.Pitch2) + 
Left;
			      register uint8 *s = GPUPack.GFX.SubZBuffer + y * GPUPack.GFX.ZPitch + Left;
			      register uint8 *d = GPUPack.GFX.ZBuffer + y * GPUPack.GFX.ZPitch;
			      register uint8 *e = d + Right;
			      uint16 back_fixed = COLOR_SUB (back, GPUPack.GFX.FixedColour);

			      d += Left;
			      while (d < e)
				{
				  if (*d == 0)
				    {
				      if (*s)
					{
					  if (*s != 1)
					    *p = COLOR_SUB (back, *(p + GPUPack.GFX.Delta));
					  else
					    *p = back_fixed;
					}
				      else
					*p = (uint16) back;
				    }
				  d++;
				  p++;
				  s++;
				}
			    }
			}
		      else
			if (GPUPack.GFX.r2131 & 0x40)
			  {
			    register uint16 *p = (uint16 *) (GPUPack.GFX.Screen + y * GPUPack.GFX.Pitch2) + Left;
			    register uint8 *d = GPUPack.GFX.ZBuffer + y * GPUPack.GFX.ZPitch;
			    register uint8 *s = GPUPack.GFX.SubZBuffer + y * GPUPack.GFX.ZPitch + Left;
			    register uint8 *e = d + Right;
			    uint16 back_fixed = COLOR_ADD (back, GPUPack.GFX.FixedColour);
			    d += Left;
			    while (d < e)
			      {
				if (*d == 0)
				  {
				    if (*s)
				      {
					if (*s != 1)
					  *p = COLOR_ADD1_2 (back, *(p + GPUPack.GFX.Delta));
					else
					  *p = back_fixed;
				      }
				    else
				      *p = (uint16) back;
				  }
				d++;
				p++;
				s++;
			      }
			  }
			else
			  if (back != 0)
			    {
			      register uint16 *p = (uint16 *) (GPUPack.GFX.Screen + y * GPUPack.GFX.Pitch2) + 
Left;
			      register uint8 *d = GPUPack.GFX.ZBuffer + y * GPUPack.GFX.ZPitch;
			      register uint8 *s = GPUPack.GFX.SubZBuffer + y * GPUPack.GFX.ZPitch + Left;
			      register uint8 *e = d + Right;
			      uint16 back_fixed = COLOR_ADD (back, GPUPack.GFX.FixedColour);
			      d += Left;
			      while (d < e)
				{
				  if (*d == 0)
				    {
				      if (*s)
					{
					  if (*s != 1)
					    *p = COLOR_ADD (back, *(p + GPUPack.GFX.Delta));
					  else
					    *p = back_fixed;
					}
				      else
					*p = (uint16) back;
				    }
				  d++;
				  p++;
				  s++;
				}
			    }
			  else
			    {
			      if (!pClip->Count [5])
				{
				  // The backdrop has not been cleared yet - so
				  // copy the sub-screen to the main screen
				  // or fill it with the back-drop colour if the
				  // sub-screen is clear.
				  register uint16 *p = (uint16 *) (GPUPack.GFX.Screen + y * GPUPack.GFX.Pitch2) + Left;
				  register uint8 *d = GPUPack.GFX.ZBuffer + y * GPUPack.GFX.ZPitch;
				  register uint8 *s = GPUPack.GFX.SubZBuffer + y * GPUPack.GFX.ZPitch + Left;
				  register uint8 *e = d + Right;
				  d += Left;
				  while (d < e)
				    {
				      if (*d == 0)
					{
					  if (*s)
					    {
					      if (*s != 1)
						*p = *(p + GPUPack.GFX.Delta);
					      else
						*p = GPUPack.GFX.FixedColour;
					    }
					  else
					    *p = (uint16) back;
					}
				      d++;
				      p++;
				      s++;
				    }
				}
			    }
		    }
		}
	    }
	  else
	    {
	      // Subscreen not being added to back
	      uint32 back = IPPU.ScreenColors [0] | (IPPU.ScreenColors [0] << 16);
	      pClip = &IPPU.Clip [0];

	      if (pClip->Count [5])
		{
		  for (uint32 y = starty; y <= endy; y++)
		    {
		      for (uint32 b = 0; b < pClip->Count [5]; b++)
			{
			  uint32 Left = pClip->Left [b][5] * x2;
			  uint32 Right = pClip->Right [b][5] * x2;
			  uint16 *p = (uint16 *) (GPUPack.GFX.Screen + y * GPUPack.GFX.Pitch2) + Left;
			  uint8 *d = GPUPack.GFX.ZBuffer + y * GPUPack.GFX.ZPitch;
			  uint8 *e = d + Right;
			  d += Left;

			  while (d < e)
			    {
			      if (*d == 0)
				*p = (int16) back;
			      d++;
			      p++;
			    }
			}
		    }
		}
	      else
		{
		  for (uint32 y = starty; y <= endy; y++)
		    {
		      uint16 *p = (uint16 *) (GPUPack.GFX.Screen + y * GPUPack.GFX.Pitch2);
		      uint8 *d = GPUPack.GFX.ZBuffer + y * GPUPack.GFX.ZPitch;
		      uint8 *e = d + 256 * x2;

		      while (d < e)
			{
			  if (*d == 0)
			    *p = (int16) back;
			  d++;
			  p++;
			}
		    }
		}
	    }
	}
      else
	{
	  // 16bit and transparency but currently no transparency effects in
	  // operation.

	  uint32 back = IPPU.ScreenColors [0] |
	    (IPPU.ScreenColors [0] << 16);

	  if (PPUPack.PPU.ForcedBlanking)
	    back = black;

	  if (IPPU.Clip [0].Count[5])
	    {
	      for (uint32 y = starty; y <= endy; y++)
		{
		  uint32 *p = (uint32 *) (GPUPack.GFX.Screen + y * GPUPack.GFX.Pitch2);
		  uint32 *q = (uint32 *) ((uint16 *) p + IPPU.RenderedScreenWidth);

		  while (p < q)
		    *p++ = black;

		  for (uint32 c = 0; c < IPPU.Clip [0].Count [5]; c++)
		    {
		      if (IPPU.Clip [0].Right [c][5] > IPPU.Clip [0].Left [c][5])
			{
			  uint16 *p = (uint16 *) (GPUPack.GFX.Screen + y * GPUPack.GFX.Pitch2);
			  uint16 *q = p + IPPU.Clip [0].Right [c][5] * x2;
			  p += IPPU.Clip [0].Left [c][5] * x2;

			  while (p < q)
			    *p++ = (uint16) back;
			}
		    }
		}
	    }
	  else
	    {
	      for (uint32 y = starty; y <= endy; y++)
		{
		  uint32 *p = (uint32 *) (GPUPack.GFX.Screen + y * GPUPack.GFX.Pitch2);
		  uint32 *q = (uint32 *) ((uint16 *) p + IPPU.RenderedScreenWidth);
		  while (p < q)
		    *p++ = back;
		}
	    }
	  if (!PPUPack.PPU.ForcedBlanking)
	    {
	      for (uint32 y = starty; y <= endy; y++)
		{
		  ZeroMemory (GPUPack.GFX.ZBuffer + y * GPUPack.GFX.ZPitch,
			      IPPU.RenderedScreenWidth);
		}
	      GPUPack.GFX.DB = GPUPack.GFX.ZBuffer;
	      RenderScreen (GPUPack.GFX.Screen, FALSE, TRUE, SUB_SCREEN_DEPTH);
	    }
	}


  IPPU.PreviousLine = IPPU.CurrentLine;
}

#ifdef GFX_MULTI_FORMAT

#define _BUILD_PIXEL(F) \
uint32 BuildPixel##F(uint32 R, uint32 G, uint32 B) \
{ \
    return (BUILD_PIXEL_##F(R,G,B)); \
}\
uint32 BuildPixel2##F(uint32 R, uint32 G, uint32 B) \
{ \
    return (BUILD_PIXEL2_##F(R,G,B)); \
} \
void DecomposePixel##F(uint32 pixel, uint32 &R, uint32 &G, uint32 &B) \
{ \
    DECOMPOSE_PIXEL_##F(pixel,R,G,B); \
}

_BUILD_PIXEL(RGB565)
_BUILD_PIXEL(RGB555)
_BUILD_PIXEL(BGR565)
_BUILD_PIXEL(BGR555)
_BUILD_PIXEL(GBR565)
_BUILD_PIXEL(GBR555)
_BUILD_PIXEL(RGB5551)

bool8 S9xSetRenderPixelFormat (int format)
{
    extern uint32 current_graphic_format;

    current_graphic_format = format;

    switch (format)
    {
    case RGB565:
	_BUILD_SETUP(RGB565)
	return (TRUE);
    case RGB555:
	_BUILD_SETUP(RGB555)
	return (TRUE);
    case BGR565:
	_BUILD_SETUP(BGR565)
	return (TRUE);
    case BGR555:
	_BUILD_SETUP(BGR555)
	return (TRUE);
    case GBR565:
	_BUILD_SETUP(GBR565)
	return (TRUE);
    case GBR555:
	_BUILD_SETUP(GBR555)
	return (TRUE);
    case RGB5551:
        _BUILD_SETUP(RGB5551)
        return (TRUE);
    default:
	break;
    }
    return (FALSE);
}
#endif


