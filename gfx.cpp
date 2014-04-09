#define PSP_GU_RENDER_MIN_UPDATED_LINES 4
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
 


#define _TRANSP_SUPPORT_ 

#define OLD_COLOUR_BLENDING

//#include <tapwave.h>

#include "snes9x.h"

#include "memmap.h"
#include "ppu.h"
#include "cpuexec.h"
#include "display.h"
#include "gfx.h"
#include "apu.h"
#include "cheats.h"
#include "tile.h"


extern unsigned char gammatab[10][32];
extern uint32 os9x_gammavalue;

extern int os9x_BG0,os9x_BG1,os9x_BG2,os9x_BG3,os9x_OBJ,os9x_easy;
extern int os9x_renderingpass,os9x_render;

/*extern "C" void asmClearScreenFast16(uint8 *scr,uint32 starty,uint32 endy,uint32 back);
extern "C" void asmClearScreenFast8(uint8 *scr,uint32 starty,uint32 endy,uint32 back);
extern "C" void asmClearBufferFast32_16(uint8 *buf,uint32 val,uint32 size);*/
inline void asmClearBufferFast32_16(uint8 *buf,uint32 val,uint32 size)
{
	for (;size;size--)
		*buf++=val;
}
inline void asmClearScreenFast16(uint8 *scr,uint32 starty,uint32 endy,uint32 back)
{
	int sizey=endy-starty+1;
	uint32 *p=(uint32*)(scr+starty*256*2);
	for (int i=(sizey*256*2>>2);i;i--)
		*p++=back;
}
inline void asmClearScreenFast8(uint8 *scr,uint32 starty,uint32 endy,uint32 back)
{
	int sizey=endy-starty+1;
	uint32 *p=(uint32*)(scr+starty*256);
	for (int i=(sizey*256>>2);i;i--)
		*p++=back;

}


extern int os9x_ShowSub;
extern int os9x_fastmode;
extern int os9x_fastsprite;



void (*ptrDrawTileNewSprite)(uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr);
void (*ptrDrawClippedTileNewSprite)(uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount, uint32 index_spr);

void (*ptrDrawTileNew)(uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount);
void (*ptrDrawClippedTileNew)(uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount);

void (*ptrDrawHiResTileNew) (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount);
void (*ptrDrawHiResClippedTileNew) (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount);

//#define asmPPU

#define M7 19
#define M8 19

void ComputeClipWindows ();
/*static void S9xDisplayFrameRate ();
static void S9xDisplayString (const char *string);*/

extern uint8 BitShifts[8][4];
extern uint8 TileShifts[8][4];
extern uint8 PaletteShifts[8][4];
extern uint8 PaletteMasks[8][4];
extern uint8 Depths[8][4];
extern uint8 BGSizes [2];

//extern struct SBG BG;

extern uint8  Mode7Depths [2];

#define BLACK BUILD_PIXEL(0,0,0)

#define FIXCLIP(n)\
if (GPUPack.GFX.r212c & (1 << (n))) \
    GPUPack.GFX.pCurrentClip = &IPPU.Clip [0]; \
else \
    GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];


#define DISPLAY(n)\
    ( !(PPU.BG_Forced & n) && \
    		(   (GPUPack.GFX.r212c & n) || ( ( (GPUPack.GFX.r212d & n) && subadd) )  ) \
    )

#define ON_MAIN(N) \
(GPUPack.GFX.r212c & (1 << (N)) && \
 !(PPU.BG_Forced & (1 << (N))))

#define SUB_OR_ADD(N) \
(GPUPack.GFX.r2131 & (1 << (N)))

#define ON_SUB(N) \
((GPUPack.GFX.r2130 & 0x30) != 0x30 && \
 (GPUPack.GFX.r2130 & 2) && \
 (GPUPack.GFX.r212d & (1 << N)) && \
 !(PPU.BG_Forced & (1 << (N))))

#define ADD_OR_SUB_ON_ANYTHING \
(GPUPack.GFX.r2131 & 0x3f)

#define ANYTHING_ON_SUB \
((GPUPack.GFX.r2130 & 0x30) != 0x30 && \
 (GPUPack.GFX.r2130 & 2) && \
 (GPUPack.GFX.r212d & 0x1f))



void os9x_SetTileRender(int32 transp)
{
		if (!transp)
		{
//			if (os9x_softrendering)
			{
				if (os9x_fastsprite)
				{
					ptrDrawTileNewSprite=softDrawTile16NewFastSprite;
					ptrDrawClippedTileNewSprite=softDrawClippedTile16NewFastSprite;
				}
				else
				{
					ptrDrawTileNewSprite=softDrawTile16NewSprite;
					ptrDrawClippedTileNewSprite=softDrawClippedTile16NewSprite;
				}
				ptrDrawTileNew=softDrawTile16New;
				ptrDrawClippedTileNew=softDrawClippedTile16New;
				ptrDrawHiResTileNew=softDrawHiResTile16New;
				ptrDrawHiResClippedTileNew=softDrawHiResClippedTile16New;
			}
/*			else
			{
				if (os9x_fastsprite)
				{
					ptrDrawTileNewSprite=DrawTile16NewFastSprite;
					ptrDrawClippedTileNewSprite=DrawClippedTile16NewFastSprite;
				}
				else
				{
					ptrDrawTileNewSprite=DrawTile16NewSprite;
					ptrDrawClippedTileNewSprite=DrawClippedTile16NewSprite;
				}
				ptrDrawTileNew=DrawTile16New;
				ptrDrawClippedTileNew=DrawClippedTile16New;
				ptrDrawHiResTileNew=DrawHiResTile16New;
				ptrDrawHiResClippedTileNew=DrawHiResClippedTile16New;
			}*/
		}
		else
		{
//			if (os9x_softrendering)
			{
				if (GPUPack.GFX.r2131 & 0x80)
				{
				 	if (!(GPUPack.GFX.r2131 & 0x40))
			    	{
				    	ptrDrawTileNewSprite=softDrawTile16SUBNewSprite;
						ptrDrawClippedTileNewSprite=softDrawClippedTile16SUBNewSprite;
						ptrDrawTileNew=softDrawTile16SUBNew;
						ptrDrawClippedTileNew=softDrawClippedTile16SUBNew;
						ptrDrawHiResTileNew=softDrawHiResTile16SUBNew;
						ptrDrawHiResClippedTileNew=softDrawHiResClippedTile16SUBNew;
			    	}
			    	else
			    	{
			    		ptrDrawTileNewSprite=softDrawTile16SUB1_2NewSprite;
						ptrDrawClippedTileNewSprite=softDrawClippedTile16SUB1_2NewSprite;
						ptrDrawTileNew=softDrawTile16SUB1_2New;
						ptrDrawClippedTileNew=softDrawClippedTile16SUB1_2New;
						ptrDrawHiResTileNew=softDrawHiResTile16SUB1_2New;
						ptrDrawHiResClippedTileNew=softDrawHiResClippedTile16SUB1_2New;
			    	}
			    }
			    else
			    {
			    	if (!(GPUPack.GFX.r2131 & 0x40))
		    		{
			    		ptrDrawTileNewSprite=softDrawTile16ADDNewSprite;
						ptrDrawClippedTileNewSprite=softDrawClippedTile16ADDNewSprite;
						ptrDrawTileNew=softDrawTile16ADDNew;
						ptrDrawClippedTileNew=softDrawClippedTile16ADDNew;
						ptrDrawHiResTileNew=softDrawHiResTile16ADDNew;
						ptrDrawHiResClippedTileNew=softDrawHiResClippedTile16ADDNew;
			    	}
			    	else
			    	{
		    			ptrDrawTileNewSprite=softDrawTile16ADD1_2NewSprite;
						ptrDrawClippedTileNewSprite=softDrawClippedTile16ADD1_2NewSprite;
						ptrDrawTileNew=softDrawTile16ADD1_2New;
						ptrDrawClippedTileNew=softDrawClippedTile16ADD1_2New;
						ptrDrawHiResTileNew=softDrawHiResTile16ADD1_2New;
						ptrDrawHiResClippedTileNew=softDrawHiResClippedTile16ADD1_2New;
		    		}
			    }

			}	
/*			else
			{
			}*/
		}
}

bool8 S9xGraphicsInit ()
{
    register uint32 PixelOdd = 1;
    register uint32 PixelEven = 2;

#ifdef GFX_MULTI_FORMAT
    if (GPUPack.GFX.BuildPixel == NULL)
	S9xSetRenderPixelFormat (RGB565);
#endif

    for (uint8 bitshift = 0; bitshift < 4; bitshift++)
    {
	for (register int i = 0; i < 16; i++)
	{
	    register uint32 h = 0;
	    register uint32 l = 0;

#if defined(LSB_FIRST)
	    if (i & 8)
		h |= PixelOdd;
	    if (i & 4)
		h |= PixelOdd << 8;
	    if (i & 2)
		h |= PixelOdd << 16;
	    if (i & 1)
		h |= PixelOdd << 24;
	    if (i & 8)
		l |= PixelOdd;
	    if (i & 4)
		l |= PixelOdd << 8;
	    if (i & 2)
		l |= PixelOdd << 16;
	    if (i & 1)
		l |= PixelOdd << 24;
#else
	    if (i & 8)
		h |= (PixelOdd << 24);
	    if (i & 4)
		h |= (PixelOdd << 16);
	    if (i & 2)
		h |= (PixelOdd << 8);
	    if (i & 1)
		h |= PixelOdd;
	    if (i & 8)
		l |= (PixelOdd << 24);
	    if (i & 4)
		l |= (PixelOdd << 16);
	    if (i & 2)
		l |= (PixelOdd << 8);
	    if (i & 1)
		l |= PixelOdd;
#endif

	    odd_high[bitshift][i] = h;
	    odd_low[bitshift][i] = l;
	    h = l = 0;

#if defined(LSB_FIRST)
	    if (i & 8)
		h |= PixelEven;
	    if (i & 4)
		h |= PixelEven << 8;
	    if (i & 2)
		h |= PixelEven << 16;
	    if (i & 1)
		h |= PixelEven << 24;
	    if (i & 8)
		l |= PixelEven;
	    if (i & 4)
		l |= PixelEven << 8;
	    if (i & 2)
		l |= PixelEven << 16;
	    if (i & 1)
		l |= PixelEven << 24;
#else
	    if (i & 8)
		h |= (PixelEven << 24);
	    if (i & 4)
		h |= (PixelEven << 16);
	    if (i & 2)
		h |= (PixelEven << 8);
	    if (i & 1)
		h |= PixelEven;
	    if (i & 8)
		l |= (PixelEven << 24);
	    if (i & 4)
		l |= (PixelEven << 16);
	    if (i & 2)
		l |= (PixelEven << 8);
	    if (i & 1)
		l |= PixelEven;
#endif

	    even_high[bitshift][i] = h;
	    even_low[bitshift][i] = l;
	}
	PixelEven <<= 2;
	PixelOdd <<= 2;
    }

    GPUPack.GFX.RealPitch = GPUPack.GFX.Pitch2 = GPUPack.GFX.Pitch;
    GPUPack.GFX.ZPitch = GPUPack.GFX.Pitch;
//    if (Settings.SixteenBit)
	GPUPack.GFX.ZPitch >>= 1;
    GPUPack.GFX.Delta = (GPUPack.GFX.SubScreen - GPUPack.GFX.Screen) >> 1;
    GPUPack.GFX.DepthDelta = GPUPack.GFX.SubZBuffer - GPUPack.GFX.ZBuffer;
    //GPUPack.GFX.InfoStringTimeout = 0;
    //GPUPack.GFX.InfoString = NULL;

    PPU.BG_Forced = 0;
    IPPU.OBJChanged = TRUE;
//    if (Settings.Transparency)
//	Settings.SixteenBit = TRUE;

    IPPU.DirectColourMapsNeedRebuild = TRUE;
    GPUPack.GFX.PPL = GPUPack.GFX.Pitch >> 1;
	GPUPack.GFX.PPLx2 = GPUPack.GFX.Pitch;
    
    S9xFixColourBrightness ();

#ifdef _TRANSP_SUPPORT_
	//if (!(GPUPack.GFX.X2 = (uint16 *) malloc (sizeof (uint16) * 0x10000)))
	//    return (FALSE);

	if ((!(GPUPack.GFX.ZERO_OR_X2 = (uint16 *) malloc (sizeof (uint16) * 0x10000)))
   	 ||!(GPUPack.GFX.ZERO = (uint16 *) malloc (sizeof (uint16) * 0x10000)))
	{
	    if (GPUPack.GFX.ZERO_OR_X2)
	    {
		free ((char *) GPUPack.GFX.ZERO_OR_X2);
		GPUPack.GFX.ZERO_OR_X2 = NULL;
	    }
	    //if (GPUPack.GFX.X2)
	    //{
		//free ((char *) GPUPack.GFX.X2);
		//GPUPack.GFX.X2 = NULL;
	    //}
	    return (FALSE);
	}
	uint32 r, g, b;

	// Build a lookup table that multiplies a packed RGB value by 2 with
	// saturation.
	for (r = 0; r <= MAX_RED; r++)
	{
	    uint32 r2 = r << 1;
	    if (r2 > MAX_RED)
		r2 = MAX_RED;
	    for (g = 0; g <= MAX_GREEN; g++)
	    {
		uint32 g2 = g << 1;
		if (g2 > MAX_GREEN)
		    g2 = MAX_GREEN;
		for (b = 0; b <= MAX_BLUE; b++)
		{
		    uint32 b2 = b << 1;
		    if (b2 > MAX_BLUE)
			b2 = MAX_BLUE;
		    GPUPack.GFX.X2 [BUILD_PIXEL2 (r, g, b)] = BUILD_PIXEL2 (r2, g2, b2);
		    GPUPack.GFX.X2 [BUILD_PIXEL2 (r, g, b) & ~ALPHA_BITS_MASK] = BUILD_PIXEL2 (r2, g2, b2);
		}
	    }
	}
	ZeroMemory (GPUPack.GFX.ZERO, 0x10000 * sizeof (uint16));
	ZeroMemory (GPUPack.GFX.ZERO_OR_X2, 0x10000 * sizeof (uint16));
	// Build a lookup table that if the top bit of the color value is zero
	// then the value is zero, otherwise multiply the value by 2. Used by
	// the color subtraction code.

#if defined(OLD_COLOUR_BLENDING)
	for (r = 0; r <= MAX_RED; r++)
	{
	    uint32 r2 = r;
	    if ((r2 & 0x10) == 0)
		r2 = 0;
	    else
		r2 = (r2 << 1) & MAX_RED;

	    for (g = 0; g <= MAX_GREEN; g++)
	    {
		uint32 g2 = g;
		if ((g2 & GREEN_HI_BIT) == 0)
		    g2 = 0;
		else
		    g2 = (g2 << 1) & MAX_GREEN;

		for (b = 0; b <= MAX_BLUE; b++)
		{
		    uint32 b2 = b;
		    if ((b2 & 0x10) == 0)
			b2 = 0;
		    else
			b2 = (b2 << 1) & MAX_BLUE;

		    GPUPack.GFX.ZERO_OR_X2 [BUILD_PIXEL2 (r, g, b)] = BUILD_PIXEL2 (r2, g2, b2);
		    GPUPack.GFX.ZERO_OR_X2 [BUILD_PIXEL2 (r, g, b) & ~ALPHA_BITS_MASK] = BUILD_PIXEL2 (r2, g2, b2);
		}
	    }
	}
#else
        for (r = 0; r <= MAX_RED; r++)
        {
            uint32 r2 = r;
            if ((r2 & 0x10) == 0)
                r2 = 0;
            else
                r2 = (r2 << 1) & MAX_RED;

            if (r2 == 0)
                r2 = 1;
            for (g = 0; g <= MAX_GREEN; g++)
            {
                uint32 g2 = g;
                if ((g2 & GREEN_HI_BIT) == 0)
                    g2 = 0;
                else
                    g2 = (g2 << 1) & MAX_GREEN;

                if (g2 == 0)
                    g2 = 1;
                for (b = 0; b <= MAX_BLUE; b++)
                {
                    uint32 b2 = b;
                    if ((b2 & 0x10) == 0)
                        b2 = 0;
                    else
                        b2 = (b2 << 1) & MAX_BLUE;

                    if (b2 == 0)
                        b2 = 1;
                    GPUPack.GFX.ZERO_OR_X2 [BUILD_PIXEL2 (r, g, b)] = BUILD_PIXEL2 (r2, g2, b2);
                    GPUPack.GFX.ZERO_OR_X2 [BUILD_PIXEL2 (r, g, b) & ~ALPHA_BITS_MASK] = BUILD_PIXEL2 (r2, g2, b2);
                }
            }
        }
#endif

	// Build a lookup table that if the top bit of the color value is zero
	// then the value is zero, otherwise its just the value.
	for (r = 0; r <= MAX_RED; r++)
	{
	    uint32 r2 = r;
	    if ((r2 & 0x10) == 0)
		r2 = 0;
	    else
		r2 &= ~0x10;

	    for (g = 0; g <= MAX_GREEN; g++)
	    {
		uint32 g2 = g;
		if ((g2 & GREEN_HI_BIT) == 0)
		    g2 = 0;
		else
		    g2 &= ~GREEN_HI_BIT;
		for (b = 0; b <= MAX_BLUE; b++)
		{
		    uint32 b2 = b;
		    if ((b2 & 0x10) == 0)
			b2 = 0;
		    else
			b2 &= ~0x10;

		    GPUPack.GFX.ZERO [BUILD_PIXEL2 (r, g, b)] = BUILD_PIXEL2 (r2, g2, b2);
		    GPUPack.GFX.ZERO [BUILD_PIXEL2 (r, g, b) & ~ALPHA_BITS_MASK] = BUILD_PIXEL2 (r2, g2, b2);
		}
	    }
	}    
//	GPUPack.GFX.ZERO = NULL;
#else
    {
	GPUPack.GFX.X2 = NULL;
	GPUPack.GFX.ZERO_OR_X2 = NULL;
	GPUPack.GFX.ZERO = NULL;
    }
#endif
    return (TRUE);
}

void S9xGraphicsDeinit (void)
{
    // Free any memory allocated in S9xGraphicsInit
    //if (GPUPack.GFX.X2)
    //{
	//free (GPUPack.GFX.X2);
	//GPUPack.GFX.X2 = NULL;
    //}
    if (GPUPack.GFX.ZERO_OR_X2)
    {
	free (GPUPack.GFX.ZERO_OR_X2);
	GPUPack.GFX.ZERO_OR_X2 = NULL;
    }
    if (GPUPack.GFX.ZERO)
    {
	free (GPUPack.GFX.ZERO);
	GPUPack.GFX.ZERO = NULL;
    }
}


void S9xBuildDirectColourMaps ()
{
	uint8 *cgamma=(uint8*)gammatab[os9x_gammavalue];
    for (uint32 p = 0; p < 8; p++)
    {
	for (uint32 c = 0; c < 256; c++)
	{
// XXX: Brightness
	    DirectColourMaps [p][c] = BUILD_PIXEL (cgamma[((c & 7) << 2) | ((p & 1) << 1)],
						   cgamma[((c & 0x38) >> 1) | (p & 2)],
						   cgamma[((c & 0xc0) >> 3) | (p & 4)]);
	}
    }
    IPPU.DirectColourMapsNeedRebuild = FALSE;
}

void S9xStartScreenRefresh ()
{
//    if (GPUPack.GFX.InfoStringTimeout > 0 && --GPUPack.GFX.InfoStringTimeout == 0)
//	GPUPack.GFX.InfoString = NULL;

    if (IPPU.RenderThisFrame)
    {
	if (!S9xInitUpdate ())
	{
	    IPPU.RenderThisFrame = FALSE;
	    return;
	}
	IPPU.RenderedFramesCount++;
	IPPU.PreviousLine = IPPU.CurrentLine = 0;
	IPPU.MaxBrightness = PPU.Brightness;
	IPPU.LatchedBlanking = PPU.ForcedBlanking;
	IPPU.LatchedInterlace = (ROM_GLOBAL[0x2133] & 1);
//	IPPU.Interlace = (ROM_GLOBAL[0x2133] & 1);
	if (Settings.SupportHiRes && (PPU.BGMode == 5 || PPU.BGMode == 6 ||
				      IPPU.LatchedInterlace/*IPPU.Interlace*/))
	{
	    if (PPU.BGMode == 5 || PPU.BGMode == 6)
	    {
		IPPU.RenderedScreenWidth = 512;
		IPPU.DoubleWidthPixels = TRUE;
	    }
	    if (/*IPPU.Interlace*/IPPU.LatchedInterlace)
	    {
		IPPU.RenderedScreenHeight = PPU.ScreenHeight << 1;
		GPUPack.GFX.Pitch2 = GPUPack.GFX.RealPitch;
		GPUPack.GFX.Pitch = GPUPack.GFX.RealPitch * 2;
		//if (Settings.SixteenBit)
		    GPUPack.GFX.PPL = GPUPack.GFX.PPLx2 = GPUPack.GFX.RealPitch;
		//else
		//    GPUPack.GFX.PPL = GPUPack.GFX.PPLx2 = GPUPack.GFX.RealPitch << 1;
	    }
	    else
	    {
		IPPU.RenderedScreenHeight = PPU.ScreenHeight;
		GPUPack.GFX.Pitch2 = GPUPack.GFX.Pitch = GPUPack.GFX.RealPitch;
		//if (Settings.SixteenBit)
		    GPUPack.GFX.PPL = GPUPack.GFX.Pitch >> 1;
		//else
		//    GPUPack.GFX.PPL = GPUPack.GFX.Pitch;
		GPUPack.GFX.PPLx2 = GPUPack.GFX.PPL << 1;
	    }
#if defined(USE_GLIDE) || defined(USE_OPENGL)
	    GPUPack.GFX.ZPitch = GPUPack.GFX.RealPitch;
	    if (Settings.SixteenBit)
		GPUPack.GFX.ZPitch >>= 1;
#endif
	}
	else
	{
	    IPPU.RenderedScreenWidth = 256;
	    IPPU.RenderedScreenHeight = PPU.ScreenHeight;
	    IPPU.DoubleWidthPixels = FALSE;
	    {
		GPUPack.GFX.Pitch2 = GPUPack.GFX.Pitch = GPUPack.GFX.RealPitch;
		GPUPack.GFX.PPL = GPUPack.GFX.PPLx2 >> 1;
		GPUPack.GFX.ZPitch = GPUPack.GFX.RealPitch;
		//if (Settings.SixteenBit)
		    GPUPack.GFX.ZPitch >>= 1;
	    }                        
	}
	PPU.RecomputeClipWindows = TRUE;
	GPUPack.GFX.DepthDelta = GPUPack.GFX.SubZBuffer - GPUPack.GFX.ZBuffer;
	GPUPack.GFX.Delta = (GPUPack.GFX.SubScreen - GPUPack.GFX.Screen) >> 1;
    }
    if (++IPPU.FrameCount % Memory.ROMFramesPerSecond == 0)
    {
	IPPU.DisplayedRenderedFrameCount = IPPU.RenderedFramesCount;
	IPPU.RenderedFramesCount = 0;
	IPPU.FrameCount = 0;
    }

}

extern void DrawBGMode7Background16New (uint16 *Screen, int bg);
extern void DrawBGMode7Background16PrioNew (uint16 *Screen, int bg, uint32 prio);



/*void RenderLine (uint8 C)
{
    if (IPPU.RenderThisFrame)
    {
	LineData[C].BG[0].VOffset = PPU.BG[0].VOffset + 1;
	LineData[C].BG[0].HOffset = PPU.BG[0].HOffset;
	LineData[C].BG[1].VOffset = PPU.BG[1].VOffset + 1;
	LineData[C].BG[1].HOffset = PPU.BG[1].HOffset;

	if (PPU.BGMode == 7)
	{
	    struct SLineMatrixData *p = &LineMatrixData [C];
	    p->MatrixA = PPU.MatrixA;
	    p->MatrixB = PPU.MatrixB;
	    p->MatrixC = PPU.MatrixC;
	    p->MatrixD = PPU.MatrixD;
	    p->CentreX = PPU.CentreX;
	    p->CentreY = PPU.CentreY;
	}
	else
	{
	    if (Settings.StarfoxHack && PPU.BG[2].VOffset == 0 &&
		PPU.BG[2].HOffset == 0xe000)
	    {
		LineData[C].BG[2].VOffset = 0xe1;
		LineData[C].BG[2].HOffset = 0;
	    }
	    else
	    {
		LineData[C].BG[2].VOffset = PPU.BG[2].VOffset + 1;
		LineData[C].BG[2].HOffset = PPU.BG[2].HOffset;
		LineData[C].BG[3].VOffset = PPU.BG[3].VOffset + 1;
		LineData[C].BG[3].HOffset = PPU.BG[3].HOffset;
	    }
	}
	IPPU.CurrentLine = C + 1;
    }
}*/

void S9xEndScreenRefresh ()
{
    IPPU.HDMAStarted = FALSE;
    if (IPPU.RenderThisFrame)
    {
  //  	INFO_FLUSH_REDRAW("endrefresh");
		FLUSH_REDRAW ();
		if (IPPU.ColorsChanged)
		{
			uint32 saved = PPU.CGDATA[0];
			IPPU.ColorsChanged = FALSE;
			S9xSetPalette ();
			PPU.CGDATA[0] = saved;
		}
        GPUPack.GFX.Pitch = GPUPack.GFX.Pitch2 = GPUPack.GFX.RealPitch;
        GPUPack.GFX.PPL = GPUPack.GFX.PPLx2 >> 1;
        

/*	if (Settings.DisplayFrameRate)
	    S9xDisplayFrameRate ();
	if (GPUPack.GFX.InfoString)
	    S9xDisplayString (GPUPack.GFX.InfoString);*/

		S9xDeinitUpdate (IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight,
			 TRUE);
    }
	else
		ResetClipWindowsFix();

    S9xApplyCheats ();
#ifdef DEBUGGER
    if (CPUPack.CPU.Flags & FRAME_ADVANCE_FLAG)
    {
	if (CPUPack.ICPU.FrameAdvanceCount)
	{
	    CPUPack.ICPU.FrameAdvanceCount--;
	    IPPU.RenderThisFrame = TRUE;
	    IPPU.FrameSkip = 0;
	}
	else
	{
	    CPUPack.CPU.Flags &= ~FRAME_ADVANCE_FLAG;
	    CPUPack.CPU.Flags |= DEBUG_MODE_FLAG;
	}
    }
#endif
    if (CPUPack.CPU.SRAMModified) {
			if (!CPUPack.CPU.AutoSaveTimer) {
	    	if (!(CPUPack.CPU.AutoSaveTimer = Settings.AutoSaveDelay * Memory.ROMFramesPerSecond)) CPUPack.CPU.SRAMModified = FALSE;
			} else {
	    	if (!--CPUPack.CPU.AutoSaveTimer) {
					S9xAutoSaveSRAM ();
					CPUPack.CPU.SRAMModified = FALSE;
	    	}
			}
    }
}

//void S9xSetInfoString (const char *string)
//{
 //   GPUPack.GFX.InfoString = string;
 //   GPUPack.GFX.InfoStringTimeout = 120;
//}



void S9xSetupOBJ ()
{
    int SmallSize;
    int LargeSize;

    switch (PPU.OBJSizeSelect)
    {
    case 0:
	SmallSize = 8;
	LargeSize = 16;
	break;
    case 1:
	SmallSize = 8;
	LargeSize = 32;
	break;
    case 2:
	SmallSize = 8;
	LargeSize = 64;
	break;
    case 3:
	SmallSize = 16;
	LargeSize = 32;
	break;
    case 4:
	SmallSize = 16;
	LargeSize = 64;
	break;
    case 5:
    default:
	SmallSize = 32;
	LargeSize = 64;
	break;
    }

    int C = 0;

	if (os9x_fastsprite)    
	{
	    int FirstSprite = PPU.FirstSprite & 0x7f;	
	    /*yo*/
	    int LSPR = (FirstSprite-1)&0x7F;
	    int S = LSPR;
	    /**/
	    do
	    {
			int Size;
			if (PPU.OBJ [S].Size)
			    Size = LargeSize;
			else
			    Size = SmallSize;

			long VPos = PPU.OBJ [S].VPos;
	
			if (VPos >= PPU.ScreenHeight)
			    VPos -= 256;
			if (PPU.OBJ [S].HPos < 256 && PPU.OBJ [S].HPos > -Size &&
	    		VPos < PPU.ScreenHeight && VPos > -Size)
			{
			    GPUPack.GFX.OBJList [C++] = S;
			    GPUPack.GFX.Sizes[S] = Size;
		    	GPUPack.GFX.VPositions[S] = VPos;

			}
			S = (S - 1) & 0x7f;
	    } while (S != LSPR);
	}
	else
	{
		int FirstSprite = PPU.FirstSprite & 0x7f;
	    int S = FirstSprite;    	    
	    do
	    {
			int Size;
			if (PPU.OBJ [S].Size)
			    Size = LargeSize;
			else
			    Size = SmallSize;

			long VPos = PPU.OBJ [S].VPos;
	
			if (VPos >= PPU.ScreenHeight)
			    VPos -= 256;
			if (PPU.OBJ [S].HPos < 256 && PPU.OBJ [S].HPos > -Size &&
	    		VPos < PPU.ScreenHeight && VPos > -Size)
			{
			    GPUPack.GFX.OBJList [C++] = S;
			    GPUPack.GFX.Sizes[S] = Size;
		    	GPUPack.GFX.VPositions[S] = VPos;

			}
			S = (S + 1) & 0x7f;			
	    } while (S != FirstSprite);
	}
    // Terminate the list
    GPUPack.GFX.OBJList [C] = -1;
    IPPU.OBJChanged = FALSE;
}


void DrawOBJSNew (int32 prio,int32 OnMain=FALSE)
{
    uint32 OGP32;
    uint32 BaseTile, Tile;

    CHECK_SOUND();

    GPUPack.BG.BitShift = 4;
    GPUPack.BG.TileShift = 5;
    GPUPack.BG.TileAddress = PPU.OBJNameBase;
    GPUPack.BG.StartPalette = 128;
    GPUPack.BG.PaletteShift = 4;
    GPUPack.BG.PaletteMask = 7;
//    if (os9x_softrendering)
    {
	    GPUPack.BG.Buffer = IPPU.TileCache [TILE_4BIT];
	    GPUPack.BG.BufferRam = IPPU.TileCache8 [TILE_4BIT];	    
    }
/*    else
    {
	    BG.Buffer = os9x_tileCachesPtr[TILE_4BIT];//IPPU.TileCache [TILE_4BIT];
	    BG.BufferRam = IPPU.TileCache [TILE_4BIT];
		os9x_curTileCache = os9x_tileCaches[TILE_4BIT];
	}*/

    GPUPack.BG.Buffered = IPPU.TileCached [TILE_4BIT];
    GPUPack.BG.NameSelect = PPU.OBJNameSelect;
    GPUPack.BG.DirectColourMode = FALSE;

/*    if (GPUPack.GFX.OBJList[0]<0) return;
    int I = 0;
    while (GPUPack.GFX.OBJList[++I]>=0);
    for (int S = GPUPack.GFX.OBJList [--I]; I >= 0; S = GPUPack.GFX.OBJList [--I])*/
    int I = 0;
    for (int S = GPUPack.GFX.OBJList [I++]; S >= 0; S = GPUPack.GFX.OBJList [I++])
    {
		int VPos = GPUPack.GFX.VPositions [S];
		int Size = GPUPack.GFX.Sizes[S];
		int TileInc = 1;
		int Offset;


		if ((PPU.OBJ[S].Priority!=prio)&&(prio!=-1))
			continue;
		if (VPos + Size <= (int) GPUPack.GFX.StartY || VPos > (int) GPUPack.GFX.EndY)
		    continue;
		    
		if (OnMain &&  SUB_OR_ADD(4))
		{
			os9x_SetTileRender( PPU.OBJ [S].Palette >= 4);
		}

		BaseTile = PPU.OBJ[S].Name | (PPU.OBJ[S].Palette << 10);

		if (PPU.OBJ[S].HFlip)
		{
	    	BaseTile += ((Size >> 3) - 1) | H_FLIP;
		    TileInc = -1;
		}
		if (PPU.OBJ[S].VFlip)
		    BaseTile |= V_FLIP;

		int clipcount = GPUPack.GFX.pCurrentClip->Count [4];
		if (!clipcount)
		    clipcount = 1;
	
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

		    if (Right <= Left || PPU.OBJ[S].HPos + Size <= Left ||
			PPU.OBJ[S].HPos >= Right)
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

					TileLine = StartLine;// << 3;
					OGP32 = (VPos + Y + StartLine)*256;
				    if (!PPU.OBJ[S].VFlip)
						Tile = BaseTile + (Y << 1);
				    else
						Tile = BaseTile + ((Size - Y - 8) << 1);

				    int Middle = Size >> 3;
				    if (PPU.OBJ[S].HPos < Left)
				    {
						Tile += ((Left - PPU.OBJ[S].HPos) >> 3) * TileInc;
						Middle -= (Left - PPU.OBJ[S].HPos) >> 3;
						OGP32 += Left ;
						if ((Offset = (Left - PPU.OBJ[S].HPos) & 7))
						{
						    OGP32 -= Offset ;
						    int W = 8 - Offset;
						    int Width = Right - Left;
						    if (W > Width)
							W = Width;
					    	ptrDrawClippedTileNewSprite(Tile, OGP32, Offset, W,TileLine, LineCount,I);

			    
						    if (W >= Width)
								continue;
						    Tile += TileInc;
						    Middle--;
						    OGP32 += 8 ;
						}	
		    		}
				    else
						OGP32 += PPU.OBJ[S].HPos ;

				    if (PPU.OBJ[S].HPos + Size >= Right)
				    {
						Middle -= ((PPU.OBJ[S].HPos + Size + 7) - Right) >> 3;
						Offset = (Right - (PPU.OBJ[S].HPos + Size)) & 7;
				    }
				    else
						Offset = 0;

				    for (int X = 0; X < Middle; X++, OGP32 += 8 , Tile += TileInc)
				    {
						ptrDrawTileNewSprite (Tile, OGP32, TileLine, LineCount,I);
				    }
				    if (Offset)
				    {
						ptrDrawClippedTileNewSprite (Tile, OGP32, 0, Offset,TileLine, LineCount,I);
				    }
				}
		    }
		}
    }
}

void DrawOBJSFastNew (int32 prio)
{
    uint32 OGP32;
    uint32 BaseTile, Tile;

    CHECK_SOUND();

    GPUPack.BG.BitShift = 4;
    GPUPack.BG.TileShift = 5;
    GPUPack.BG.TileAddress = PPU.OBJNameBase;
    GPUPack.BG.StartPalette = 128;
    GPUPack.BG.PaletteShift = 4;
    GPUPack.BG.PaletteMask = 7;
//    if (os9x_softrendering)
    {
	    GPUPack.BG.Buffer = IPPU.TileCache [TILE_4BIT];
	    GPUPack.BG.BufferRam = IPPU.TileCache8 [TILE_4BIT];
    }
/*    else
    {
	    BG.Buffer = os9x_tileCachesPtr[TILE_4BIT];//IPPU.TileCache [TILE_4BIT];
	    BG.BufferRam = IPPU.TileCache [TILE_4BIT];
		os9x_curTileCache = os9x_tileCaches[TILE_4BIT];
	}*/

    GPUPack.BG.Buffered = IPPU.TileCached [TILE_4BIT];
    GPUPack.BG.NameSelect = PPU.OBJNameSelect;
    GPUPack.BG.DirectColourMode = FALSE;

/*   if (GPUPack.GFX.OBJList[0]<0) return;
    int I = 0;
    while (GPUPack.GFX.OBJList[++I]>=0);
    for (int S = GPUPack.GFX.OBJList [--I]; I >= 0; S = GPUPack.GFX.OBJList [--I])*/
    int I = 0;
    for (int S = GPUPack.GFX.OBJList [I++]; S >= 0; S = GPUPack.GFX.OBJList [I++])
    {
		int VPos = GPUPack.GFX.VPositions [S];
		int Size = GPUPack.GFX.Sizes[S];
		int TileInc = 1;
		int Offset;


		if ((PPU.OBJ[S].Priority!=prio)&&(prio!=-1))
			continue;
		if (VPos + Size <= (int) GPUPack.GFX.StartY || VPos > (int) GPUPack.GFX.EndY)
		    continue;

		BaseTile = PPU.OBJ[S].Name | (PPU.OBJ[S].Palette << 10);

		if (PPU.OBJ[S].HFlip)
		{
	    	BaseTile += ((Size >> 3) - 1) | H_FLIP;
		    TileInc = -1;
		}
		if (PPU.OBJ[S].VFlip)
		    BaseTile |= V_FLIP;

		int clipcount = GPUPack.GFX.pCurrentClip->Count [4];
		if (!clipcount)
		    clipcount = 1;

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

		    if (Right <= Left || PPU.OBJ[S].HPos + Size <= Left ||
			PPU.OBJ[S].HPos >= Right)
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

					TileLine = StartLine;// << 3;
					OGP32 = (VPos + Y + StartLine)*256;
				    if (!PPU.OBJ[S].VFlip)
						Tile = BaseTile + (Y << 1);
				    else
						Tile = BaseTile + ((Size - Y - 8) << 1);

				    int Middle = Size >> 3;
				    if (PPU.OBJ[S].HPos < Left)
				    {
						Tile += ((Left - PPU.OBJ[S].HPos) >> 3) * TileInc;
						Middle -= (Left - PPU.OBJ[S].HPos) >> 3;
						OGP32 += Left ;
						if ((Offset = (Left - PPU.OBJ[S].HPos) & 7))
						{
						    OGP32 -= Offset ;
						    int W = 8 - Offset;
						    int Width = Right - Left;
						    if (W > Width)
							W = Width;
					    	ptrDrawClippedTileNew(Tile, OGP32, Offset, W,TileLine, LineCount);

			    
						    if (W >= Width)
								continue;
						    Tile += TileInc;
						    Middle--;
						    OGP32 += 8 ;
						}	
		    		}
				    else
						OGP32 += PPU.OBJ[S].HPos;

				    if (PPU.OBJ[S].HPos + Size >= Right)
				    {
						Middle -= ((PPU.OBJ[S].HPos + Size + 7) - Right) >> 3;
						Offset = (Right - (PPU.OBJ[S].HPos + Size)) & 7;
				    }
				    else
						Offset = 0;

				    for (int X = 0; X < Middle; X++, OGP32 += 8 , Tile += TileInc)
				    {
						ptrDrawTileNew (Tile, OGP32, TileLine, LineCount);
				    }
				    if (Offset)
				    {
						ptrDrawClippedTileNew (Tile, OGP32, 0, Offset,TileLine, LineCount);
				    }
				}
		    }
		}
    }
}

void DrawBackgroundOffsetNew (uint32 BGMode, uint32 bg, uint32 prio)
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
    
    GPUPack.BG.StartPalette = 0;

    BPS0 = (uint16 *) &VRAM[PPU.BG[2].SCBase << 1];

    if (PPU.BG[2].SCSize & 1)
	BPS1 = BPS0 + 1024;
    else
	BPS1 = BPS0;

    if (PPU.BG[2].SCSize & 2)
	BPS2 = BPS1 + 1024;
    else
	BPS2 = BPS0;

    if (PPU.BG[2].SCSize & 1)
	BPS3 = BPS2 + 1024;
    else
	BPS3 = BPS2;
    
    SC0 = (uint16 *) &VRAM[PPU.BG[bg].SCBase << 1];

    if (PPU.BG[bg].SCSize & 1)
	SC1 = SC0 + 1024;
    else
	SC1 = SC0;

    if (PPU.BG[bg].SCSize & 2)
	SC2 = SC1 + 1024;
    else
	SC2 = SC0;
    if (PPU.BG[bg].SCSize & 1)
	SC3 = SC2 + 1024;
    else
	SC3 = SC2;

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
	uint32 VOff = LineData [Y].BG[2].VOffset;
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

	    uint32 sGP32 = Left  + Y*256;
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
		    HOffset = LineData [Y].BG[bg].HOffset;
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
			HOffset = LineData [Y].BG[bg].HOffset;
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

			if ((HCellOffset & OffsetEnableMask))
			    HOffset = HCellOffset;
			else
			    HOffset = LineData [Y].BG[bg].HOffset - 
				      Settings.StrikeGunnerOffsetHack;
		    }
		}
		VirtAlign = ((Y + VOffset) & 7);
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

		Count = 8 - Offset;
		if (Count > MaxCount)
		    Count = MaxCount;

		sGP32 -= Offset;
		Tile = READ_2BYTES(t);
//		GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];

		if (((Tile&0x2000)>>13)==prio)
		{
			if (GPUPack.BG.TileSize == 8)
			     ptrDrawClippedTileNew (Tile, sGP32, Offset, Count, VirtAlign, Lines);
			else
			{
			    if (!(Tile & (V_FLIP | H_FLIP)))
			    {
					// Normal, unflipped
					ptrDrawClippedTileNew (Tile + t1 + (Quot & 1),sGP32, Offset, Count, VirtAlign, Lines);
			    }
			    else
			    if (Tile & H_FLIP)
			    {
					if (Tile & V_FLIP)
					{
				    // H & V flip
					    ptrDrawClippedTileNew (Tile + t2 + 1 - (Quot & 1),sGP32, Offset, Count, VirtAlign, Lines);
					}
					else
					{
					    // H flip only
					    ptrDrawClippedTileNew (Tile + t1 + 1 - (Quot & 1), sGP32, Offset, Count, VirtAlign, Lines);
					}
			    }
			    else
			    {
					// V flip only
						ptrDrawClippedTileNew (Tile + t2 + (Quot & 1),sGP32, Offset, Count, VirtAlign, Lines);
			    }
			}
		}

		Left += Count;
		TotalCount += Count;
		sGP32 += (Offset + Count) ;
		MaxCount = 8;
		}
	}
    }
}

void DrawBackgroundMode5New (uint32 /* BGMODE */, uint32 bg, uint32 prio)
{
	CHECK_SOUND();

    GPUPack.GFX.Pitch = GPUPack.GFX.RealPitch;
    GPUPack.GFX.PPL = GPUPack.GFX.PPLx2 >> 1;

    uint32 Tile;
    uint16 *SC0;
    uint16 *SC1;
    uint16 *SC2;
    uint16 *SC3;
    uint32 Width;
    
    GPUPack.BG.StartPalette = 0;

    SC0 = (uint16 *) &VRAM[PPU.BG[bg].SCBase << 1];

    if ((PPU.BG[bg].SCSize & 1))
	SC1 = SC0 + 1024;
    else
	SC1 = SC0;

    if ((PPU.BG[bg].SCSize & 2))
	SC2 = SC1 + 1024;
    else
	SC2 = SC0;

    if ((PPU.BG[bg].SCSize & 1))
	SC3 = SC2 + 1024;
    else
	SC3 = SC2;
    
    int Lines;
//    int endy = IPPU.LatchedInterlace ? GPUPack.GFX.EndY << 1 : GPUPack.GFX.EndY;
    int endy = GPUPack.GFX.EndY;

//    for (int Y = IPPU.LatchedInterlace ? GPUPack.GFX.StartY << 1 : GPUPack.GFX.StartY; Y <= endy; Y += Lines)
    for (int Y = GPUPack.GFX.StartY; Y <= endy; Y += Lines)
    {
//	int y = IPPU.LatchedInterlace ? (Y >> 1) : Y;
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
			Left = GPUPack.GFX.pCurrentClip->Left [clip][bg] * 2;
			Right = GPUPack.GFX.pCurrentClip->Right [clip][bg] * 2;

			if (Right <= Left)
			    continue;
	    }

	    uint32 sGP32 = (Left>>1) + Y*256;
	    uint32 HPos = (HOffset + Left ) & 0x3ff;

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
			sGP32 -= (Offset>>1); 
			Tile = READ_2BYTES (t);
	//		GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];

			if (((Tile & 0x2000) >> 13)==prio)
			{
				if (GPUPack.BG.TileSize == 8)
				{
				    if (!(Tile & H_FLIP))
				    {
						// Normal, unflipped
						ptrDrawHiResClippedTileNew (Tile + (Quot & 1),sGP32, Offset, Count, VirtAlign, Lines);
				    }
				    else
				    {
						// H flip
						ptrDrawHiResClippedTileNew (Tile + 1 - (Quot & 1),sGP32, Offset, Count, VirtAlign, Lines);
				    }
				}
				else
				{
				    if (!(Tile & (V_FLIP | H_FLIP)))
				    {
						// Normal, unflipped
						ptrDrawHiResClippedTileNew (Tile + t1 + (Quot & 1),sGP32, Offset, Count, VirtAlign, Lines);
				    }
				    else
				    if (Tile & H_FLIP)
				    {
						if (Tile & V_FLIP)
						{
						    // H & V flip
						    ptrDrawHiResClippedTileNew (Tile + t2 + 1 - (Quot & 1),sGP32, Offset, Count, VirtAlign, Lines);
						}
						else
						{
						    // H flip only
						    ptrDrawHiResClippedTileNew (Tile + t1 + 1 - (Quot & 1),sGP32, Offset, Count, VirtAlign, Lines);
						}
				    }
				    else
				    {
						// V flip only
						ptrDrawHiResClippedTileNew (Tile + t2 + (Quot & 1),sGP32, Offset, Count, VirtAlign, Lines);
				    }
				}
			}

			t += Quot & 1;
			if (Quot == 63)
			    t = b2;
			else if (Quot == 127)
			    t = b1;
			Quot++;
			sGP32 += (8>>1);
	    }

	    // Middle, unclipped tiles
	    Count = Width - Count;
	    int Middle = Count >> 3;
	    Count &= 7;
	    for (int C = Middle; C > 0; sGP32 += (8>>1), Quot++, C--)
	    {
			Tile = READ_2BYTES(t);
			//GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];
			if (((Tile & 0x2000) >> 13)==prio)
			{
				if (GPUPack.BG.TileSize == 8)
				{
				    if (!(Tile & H_FLIP))
				    {
					// Normal, unflipped
						ptrDrawHiResTileNew (Tile + (Quot & 1),sGP32, VirtAlign, Lines);
				    }
				    else
				    {
						// H flip
						ptrDrawHiResTileNew (Tile + 1 - (Quot & 1),sGP32, VirtAlign, Lines);
				    }
				}
				else
				{
				    if (!(Tile & (V_FLIP | H_FLIP)))
				    {
					// Normal, unflipped
						ptrDrawHiResTileNew (Tile + t1 + (Quot & 1),sGP32, VirtAlign, Lines);
				    }
				    else
				    if (Tile & H_FLIP)
				    {
					if (Tile & V_FLIP)
					{
					    // H & V flip
					    ptrDrawHiResTileNew (Tile + t2 + 1 - (Quot & 1),sGP32, VirtAlign, Lines);
					}
					else
					{
					    // H flip only
					    ptrDrawHiResTileNew (Tile + t1 + 1 - (Quot & 1), sGP32, VirtAlign, Lines);
					}
				    }
				    else
				    {
					// V flip only
						ptrDrawHiResTileNew (Tile + t2 + (Quot & 1),sGP32, VirtAlign, Lines);
				    }
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
//			GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];
			if (((Tile & 0x2000) >> 13)==prio)
			{
				if (GPUPack.BG.TileSize == 8)
				{
				    if (!(Tile & H_FLIP))
				    {
					// Normal, unflipped
						ptrDrawHiResClippedTileNew (Tile + (Quot & 1),sGP32, 0, Count, VirtAlign, Lines);
				    }
				    else
				    {
					// H flip
						ptrDrawHiResClippedTileNew (Tile + 1 - (Quot & 1),sGP32, 0, Count, VirtAlign, Lines);
				    }
				}
				else
				{
				    if (!(Tile & (V_FLIP | H_FLIP)))
				    {
					// Normal, unflipped
						ptrDrawHiResClippedTileNew (Tile + t1 + (Quot & 1),sGP32, 0, Count, VirtAlign, Lines);
				    }
				    else
				    if (Tile & H_FLIP)
				    {
						if (Tile & V_FLIP)
						{
						    // H & V flip
						    ptrDrawHiResClippedTileNew (Tile + t2 + 1 - (Quot & 1),sGP32, 0, Count, VirtAlign, Lines);
						}
						else
						{
						    // H flip only
						    ptrDrawHiResClippedTileNew (Tile + t1 + 1 - (Quot & 1),sGP32, 0, Count, VirtAlign, Lines);
						}
				    }
				    else
				    {
					// V flip only
							ptrDrawHiResClippedTileNew (Tile + t2 + (Quot & 1),sGP32, 0, Count, VirtAlign, Lines);
				    }
				}
			}
	    }
	}
    }
    /*GPUPack.GFX.Pitch = IPPU.LatchedInterlace ? GPUPack.GFX.RealPitch * 2 : GPUPack.GFX.RealPitch;
    GPUPack.GFX.PPL = IPPU.LatchedInterlace ? GPUPack.GFX.PPLx2 : (GPUPack.GFX.PPLx2 >> 1);*/
}


void DrawBackgroundNew (uint32 BGMode, uint32 bg, uint32 prio)
{
    GPUPack.BG.TileSize = BGSizes [PPU.BG[bg].BGSize];
    GPUPack.BG.BitShift = BitShifts[BGMode][bg];
    GPUPack.BG.TileShift = TileShifts[BGMode][bg];
    GPUPack.BG.TileAddress = PPU.BG[bg].NameBase << 1;
    GPUPack.BG.NameSelect = 0;

//    if (os9x_softrendering)
    {
	    GPUPack.BG.Buffer = IPPU.TileCache [Depths [BGMode][bg]];
	    GPUPack.BG.BufferRam = IPPU.TileCache8 [Depths [BGMode][bg]];
    }
/*    else
    {
		BG.Buffer = os9x_tileCachesPtr[Depths [BGMode][bg]];//IPPU.TileCache [Depths [BGMode][bg]];
		BG.BufferRam = IPPU.TileCache [Depths [BGMode][bg]];
		os9x_curTileCache = os9x_tileCaches[Depths [BGMode][bg]];
	}*/


    GPUPack.BG.Buffered = IPPU.TileCached [Depths [BGMode][bg]];
    GPUPack.BG.PaletteShift = PaletteShifts[BGMode][bg];
    GPUPack.BG.PaletteMask = PaletteMasks[BGMode][bg];
    GPUPack.BG.DirectColourMode = (BGMode == 3 || BGMode == 4) && bg == 0 &&
		          (GPUPack.GFX.r2130 & 1);

    if (PPU.BGMosaic [bg] && PPU.Mosaic > 1)
    {
		//DrawBackgroundMosaic (BGMode, bg, Z1, Z2);
		//return;
    }
    switch (BGMode)
    {
	    case 2:
			if (Settings.WrestlemaniaArcade)
		    break;
	    case 4: // Used by Puzzle Bobble
    	    DrawBackgroundOffsetNew (BGMode, bg, prio);
			return;
	    case 5:
    	case 6: // XXX: is also offset per tile.
	    	DrawBackgroundMode5New (BGMode, bg, prio);
			return;
	}





    CHECK_SOUND();

    uint32 Tile;
    uint16 *SC0;
    uint16 *SC1;
    uint16 *SC2;
    uint16 *SC3;
    uint32 Width;

    if (BGMode == 0)
	GPUPack.BG.StartPalette = bg << 5;
    else
	GPUPack.BG.StartPalette = 0;

    SC0 = (uint16 *) &VRAM[PPU.BG[bg].SCBase << 1];

    if (PPU.BG[bg].SCSize & 1)
	SC1 = SC0 + 1024;
    else
	SC1 = SC0;

    if (PPU.BG[bg].SCSize & 2)
	SC2 = SC1 + 1024;
    else
	SC2 = SC0;

    if (PPU.BG[bg].SCSize & 1)
	SC3 = SC2 + 1024;
    else
	SC3 = SC2;

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

//	VirtAlign <<= 4; //*16

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

	    uint32 sGP32 = Left  + Y*256;
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

		sGP32 -= Offset ;
		Tile = READ_2BYTES(t);

		if (((Tile & 0x2000) >> 13)==prio)
		{
			if (GPUPack.BG.TileSize == 8)
			{
				    ptrDrawClippedTileNew(Tile, sGP32, Offset, Count, VirtAlign,Lines);
			}
			else
			{
		 	   if (!(Tile & (V_FLIP | H_FLIP)))
			    {
				// Normal, unflipped
					ptrDrawClippedTileNew(Tile + t1 + (Quot & 1),sGP32, Offset, Count, VirtAlign, Lines);
			    }
			    else
			    if (Tile & H_FLIP)
			    {
					if (Tile & V_FLIP)
					{
				    // H & V flip
				    ptrDrawClippedTileNew (Tile + t2 + 1 - (Quot & 1),sGP32, Offset, Count, VirtAlign, Lines);
					}
					else
					{
				    // H flip only
				    ptrDrawClippedTileNew (Tile + t1 + 1 - (Quot & 1),sGP32, Offset, Count, VirtAlign, Lines);
					}
			    }
			    else
			    {
					// V flip only
					ptrDrawClippedTileNew (Tile + t2 + (Quot & 1), sGP32,Offset, Count, VirtAlign, Lines);
				}
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
		sGP32 += 8 ;
	    }

	    // Middle, unclipped tiles
	    Count = Width - Count;
	    int Middle = Count >> 3;
	    Count &= 7;
	    for (int C = Middle; C > 0;sGP32 += 8 ,  Quot++, C--)
	    {
		Tile = READ_2BYTES(t);
//		GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];

		if (((Tile & 0x2000) >> 13)==prio)
		{
		if (GPUPack.BG.TileSize != 8)
		{
		    if (Tile & H_FLIP)
		    {
			// Horizontal flip, but what about vertical flip ?
			if (Tile & V_FLIP)
			{
			    // Both horzontal & vertical flip
			    ptrDrawTileNew (Tile + t2 + 1 - (Quot & 1), sGP32,VirtAlign, Lines);
			}
			else
			{
			    // Horizontal flip only
			    ptrDrawTileNew (Tile + t1 + 1 - (Quot & 1), sGP32,VirtAlign, Lines);
			}
		    }
		    else
		    {
			// No horizontal flip, but is there a vertical flip ?
			if (Tile & V_FLIP)
			{
			    // Vertical flip only
			    ptrDrawTileNew (Tile + t2 + (Quot & 1), sGP32,VirtAlign, Lines);
			}
			else
			{
			    // Normal unflipped
			    ptrDrawTileNew (Tile + t1 + (Quot & 1), sGP32,VirtAlign, Lines);
			}
		    }
		}
		else
		{
		    ptrDrawTileNew (Tile, sGP32, VirtAlign, Lines);
		}
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
//		GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];
		if (((Tile & 0x2000) >> 13)==prio)
		{
		if (GPUPack.BG.TileSize == 8)
		    ptrDrawClippedTileNew (Tile, sGP32, 0, Count, VirtAlign,
					   Lines);
		else
		{
		    if (!(Tile & (V_FLIP | H_FLIP)))
		    {
			// Normal, unflipped
				ptrDrawClippedTileNew (Tile + t1 + (Quot & 1), sGP32, 0,
					       Count, VirtAlign, Lines);
		    }
		    else
		    if (Tile & H_FLIP)
		    {
			if (Tile & V_FLIP)
			{
			    // H & V flip
			    ptrDrawClippedTileNew (Tile + t2 + 1 - (Quot & 1),
						   sGP32, 0, Count, VirtAlign,
						   Lines);
			}
			else
			{
			    // H flip only
			    ptrDrawClippedTileNew (Tile + t1 + 1 - (Quot & 1),
						   sGP32, 0, Count, VirtAlign,
						   Lines);
			}
		    }
		    else
		    {
			// V flip only
				ptrDrawClippedTileNew (Tile + t2 + (Quot & 1),
					       sGP32, 0, Count, VirtAlign,
					       Lines);
		    }
		}
	    }
	    }

	}
    }
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



/*#include "font.h"

void DisplayChar (uint8 *Screen, uint8 c)
{
    int line = (((c & 0x7f) - 32) >> 4) * font_height;
    int offset = (((c & 0x7f) - 32) & 15) * font_width;
    if (Settings.SixteenBit)
    {
	int h, w;
	uint16 *s = (uint16 *) Screen;
	for (h = 0; h < font_height; h++, line++,
	     s += GPUPack.GFX.PPL - font_width)
	{
	    for (w = 0; w < font_width; w++, s++)
	    {
		uint8 p = font [line][offset + w];

		if (p == '#')
		    *s = 0xffff;
		else
		if (p == '.')
		    *s = BLACK;
	    }
	}
    }
    else
    {
	int h, w;
	uint8 *s = Screen;
	for (h = 0; h < font_height; h++, line++,
	     s += GPUPack.GFX.PPL - font_width)
	{
	    for (w = 0; w < font_width; w++, s++)
	    {
		uint8 p = font [line][offset + w];

		if (p == '#')
		    *s = 255;
		else
		if (p == '.')
		    *s = BLACK;
	    }
	}
    }
}

static void S9xDisplayFrameRate ()
{
    uint8 *Screen = GPUPack.GFX.Screen + 2 +
		    (IPPU.RenderedScreenHeight - font_height - 1) * GPUPack.GFX.Pitch2;
    char string [10];
    int len = 5;

    sprintf (string, "%02d/%02d", IPPU.DisplayedRenderedFrameCount,
	     (int) Memory.ROMFramesPerSecond);

    int i;
    for (i = 0; i < len; i++)
    {
	DisplayChar (Screen, string [i]);
	Screen += Settings.SixteenBit ? (font_width - 1) * sizeof (uint16) : 
		  (font_width - 1);
    }
}

static void S9xDisplayString (const char *string)
{
    uint8 *Screen = GPUPack.GFX.Screen + 2 +
		    (IPPU.RenderedScreenHeight - font_height * 5) * GPUPack.GFX.Pitch2;
    int len = strlen (string);
    int max_chars = IPPU.RenderedScreenWidth / (font_width - 1);
    int char_count = 0;
    int i;

    for (i = 0; i < len; i++, char_count++)
    {
	if (char_count >= max_chars || string [i] < 32)
	{
	    Screen -= Settings.SixteenBit ? 
			(font_width - 1) * sizeof (uint16) * max_chars :
			(font_width - 1) * max_chars;
	    Screen += font_height * GPUPack.GFX.Pitch;
	    if (Screen >= GPUPack.GFX.Screen + GPUPack.GFX.Pitch * IPPU.RenderedScreenHeight)
		break;
	    char_count -= max_chars;
	}
	if (string [i] < 32)
	    continue;
	DisplayChar (Screen, string [i]);
	Screen += Settings.SixteenBit ? (font_width - 1) * sizeof (uint16) : 
		  (font_width - 1);
    }
}
*/

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
_BUILD_PIXEL(BGR444)

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


/******************************************************************

YOOO

*******************************************************************/


void orgS9xUpdateScreen(void);
void pspS9xUpdateScreen(void);
void pspS9xUpdateScreenFix(void);
extern int32 os9x_specialskipframe;
extern int swap_buf;

void S9xUpdateScreen (){
	os9x_renderingpass++;
   
#ifdef PROFILING
	PROF_START(8);
#endif
#ifdef PROFILING
	PROF_START(9);
#endif

	GPUPack.GFX.S = GPUPack.GFX.Screen;
  GPUPack.GFX.r2131 = ROM_GLOBAL [0x2131];
  GPUPack.GFX.r212c = ROM_GLOBAL [0x212c];
  GPUPack.GFX.r212d = ROM_GLOBAL [0x212d];
  GPUPack.GFX.r2130 = ROM_GLOBAL [0x2130];
  GPUPack.GFX.Pseudo = (ROM_GLOBAL [0x2133] & 8) != 0 &&
		 (GPUPack.GFX.r212c & 15) != (GPUPack.GFX.r212d & 15) &&
		 (GPUPack.GFX.r2131 & 0x3f) == 0;

  if (IPPU.OBJChanged) S9xSetupOBJ ();

  if(os9x_softrendering<2)
  {
	  if (PPU.RecomputeClipWindows)
	  {
			ComputeClipWindows ();
			PPU.RecomputeClipWindows = FALSE;		
	  }	
  }
  else
  {
	  if (PPU.RecomputeClipWindows && IPPU.ClipFixMaxCount==0 && IPPU.MainColorCount==0 && IPPU.FixColorCount==0)
	  {
			ComputeClipWindows ();
			PPU.RecomputeClipWindows = FALSE;		
	  }	
	  else //if(IPPU.ClipFixMaxCount!=0)
		  ComputeClipWindowsFix ();

  }
  
  

  GPUPack.GFX.StartY = IPPU.PreviousLine;
  if ((GPUPack.GFX.EndY = IPPU.CurrentLine - 1) >= PPU.ScreenHeight)
	GPUPack.GFX.EndY = PPU.ScreenHeight - 1;
		
  uint32 starty = GPUPack.GFX.StartY;
  uint32 endy = GPUPack.GFX.EndY;      

#ifdef PROFILING
	PROF_END(9);
#endif
    
    
  if (starty>endy) return;  	  
  switch(os9x_softrendering)
  {
  case 1:
	  	// software rendering. so cache on!
		GPUPack.GFX.Screen = (uint8*)CACHE_ON(GPUPack.GFX.Screen);
		GPUPack.GFX.S = GPUPack.GFX.Screen;
		GPUPack.GFX.Delta = (GPUPack.GFX.SubScreen - GPUPack.GFX.Screen) >> 1;
		orgS9xUpdateScreen();
		// put back cache off and writeback Dcache
		GPUPack.GFX.Screen = (uint8*)CACHE_OFF(GPUPack.GFX.Screen);
		sceKernelDcacheWritebackInvalidateAll();
		return;
  case 2:
		if(IPPU.ClipFixMaxCount==0 && IPPU.MainColorCount==0 && IPPU.FixColorCount==0 )
			pspS9xUpdateScreen();
		else
		{
			pspS9xUpdateScreenFix();
		}
		ResetClipWindowsFix();
		return;
  case 3:
	  	if ((IPPU.ClipFixMaxCount>1 || IPPU.MainColorCount>0 || IPPU.FixColorCount>0 || (endy-starty+1>=PSP_GU_RENDER_MIN_UPDATED_LINES))&&
			!((os9x_hack&OLD_PSP_ACCEL)&&(PPU.BGMode==2)&&(!Settings.WrestlemaniaArcade)) &&  // BGOffset Mode
			!((os9x_hack&OLD_PSP_ACCEL)&&(PPU.BGMode==4))																	  // BGOffset Mode
			&&(PPU.BGMode!=7)
			)
		{							//mode 7
			if(IPPU.ClipFixMaxCount==0 && IPPU.MainColorCount==0 && IPPU.FixColorCount==0 )
				pspS9xUpdateScreen();
			else
			{
				pspS9xUpdateScreenFix();
			}
		}	else {	
			// software rendering. so cache on!
			GPUPack.GFX.Screen = (uint8*)CACHE_ON(GPUPack.GFX.Screen);
			GPUPack.GFX.S = GPUPack.GFX.Screen;
			GPUPack.GFX.Delta = (GPUPack.GFX.SubScreen - GPUPack.GFX.Screen) >> 1;
			orgS9xUpdateScreen();
			/*if (!os9x_render)	{
				dst=(u32*)(0x44000000+512*2*(272*swap_buf+starty ));
				for (int y=endy-starty+1;y;y--,dst+=256/2) __memcpy4a((long unsigned int*)dst,(long unsigned int*)src,256*2/4);
			} else*/ {
				for (int y=endy-starty+1;y;y--)
					__memcpy4a(
						(long unsigned int*)(0x44000000+512*272*2*2+256*240*2+2*256*256*2+starty*256*2),
						(long unsigned int*)(GPUPack.GFX.Screen+starty*GPUPack.GFX.Pitch2),
						256*2/4);
			}
			// put back cache off and writeback Dcache
			GPUPack.GFX.Screen = (uint8*)CACHE_OFF(GPUPack.GFX.Screen);
			//sceKernelDcacheWritebackInvalidateAll();
			sceKernelDcacheWritebackAll();
			//sceKernelDcacheWritebackInvalidateRange((void*)(0x04000000+2*512*272*2),256*240*2/*+3*256*256*2*/);
		}
		ResetClipWindowsFix();
		return;
	case 4:
	  	if ((IPPU.ClipFixMaxCount>1 || IPPU.MainColorCount>0 || IPPU.FixColorCount>0 || (endy-starty+1>=PSP_GU_RENDER_MIN_UPDATED_LINES))&&
			!((os9x_hack&OLD_PSP_ACCEL)&&(PPU.BGMode==2)&&(!Settings.WrestlemaniaArcade)) &&  // BGOffset Mode
			!((os9x_hack&OLD_PSP_ACCEL)&&(PPU.BGMode==4))																	  // BGOffset Mode
			&&(PPU.BGMode!=7))
		{							//mode 7
			if(IPPU.ClipFixMaxCount==0 && IPPU.MainColorCount==0 && IPPU.FixColorCount==0 )
				pspS9xUpdateScreen();
			else
			{
				pspS9xUpdateScreenFix();
			}	
			ResetClipWindowsFix();
			return;
		}else
		{						//go for fast software mode, so continue & update at the end
		}
  default:
	  break;
  }
    // software rendering. so cache on!
	GPUPack.GFX.Screen = (uint8*)CACHE_ON(GPUPack.GFX.Screen);
	GPUPack.GFX.S = GPUPack.GFX.Screen;
    
    os9x_SetTileRender(0);
    
    if (os9x_ColorsChanged)
    {
#ifdef PROFILING
	PROF_START(10);
#endif    
	    //memset(IPPU.TileCached[TILE_2BIT],0,(MAX_2BIT_TILES+MAX_4BIT_TILES+MAX_8BIT_TILES)<<1);
	    uint8 *p=IPPU.TileCached[TILE_2BIT]+1;
	    for (int i=(MAX_2BIT_TILES+MAX_4BIT_TILES+MAX_8BIT_TILES);i;i--,p+=2) *p=0xFF;
	    //asmClearBufferFast32_16(IPPU.TileCached[TILE_2BIT],0,(MAX_2BIT_TILES+MAX_4BIT_TILES+MAX_8BIT_TILES)<<1);
	    os9x_ColorsChanged=0;
#ifdef PROFILING
	PROF_END(10);
#endif
	    
	}
    
    uint32 back;
        
    //mario hack : fixed colour addition on backdrop colour
    if (!(ROM_GLOBAL [0x2131] & 0x80) &&(ROM_GLOBAL[0x2131] & 0x20) &&
		    (PPU.FixedColourRed || PPU.FixedColourGreen || PPU.FixedColourBlue))
	{
	    back = (IPPU.XB[PPU.FixedColourRed]) |
	    	   (IPPU.XB[PPU.FixedColourGreen] << 5) | 
	    	   (IPPU.XB[PPU.FixedColourBlue]<<10 );
	    back = (back << 16) | back;
	}
    else
    {
    	back = IPPU.ScreenColors [0] | (IPPU.ScreenColors [0] << 16);
    }
    
    if (PPU.ForcedBlanking) back = BLACK | (BLACK << 16);
    
#ifdef PROFILING
PROF_START(11);
#endif


	
	if (!os9x_fastsprite) asmClearScreenFast8(GPUPack.GFX.ZBuffer,starty,endy,0xFFFFFFFF);	
//	if (os9x_softrendering)
	{
		asmClearScreenFast16(GPUPack.GFX.Screen,starty,endy,back);	
	}
		
	if ((os9x_ShowSub==4)&&(ADD_OR_SUB_ON_ANYTHING)
		&& ( (GPUPack.GFX.r2130 & 0x30) != 0x30)
		&& ( !((GPUPack.GFX.r2130 & 0x30) == 0x10) && ( IPPU.Clip[1].Count[5] == 0))
		&& ANYTHING_ON_SUB /*hack*/ )
	{
		asmClearScreenFast16(GPUPack.GFX.SubScreen,starty,endy,/*back*/0);
	}
	
#ifdef PROFILING
PROF_END(11);
#endif
	
	if (!PPU.ForcedBlanking)
	{
	
#ifdef PROFILING
	PROF_START(12);
#endif
	
	
	    GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];


	int32 BG0 = ON_MAIN(0) &&  os9x_BG0;
	int32 BG1 = ON_MAIN(1) &&  os9x_BG1;
	int32 BG2 = ON_MAIN(2) &&  os9x_BG2;
	int32 BG3 = ON_MAIN(3) &&  os9x_BG3;
	int32 OB  = ON_MAIN(4) &&  os9x_OBJ;
	    
	int32 BG0_SUB,BG1_SUB,BG2_SUB,BG3_SUB,OB_SUB;
	if (ADD_OR_SUB_ON_ANYTHING)
	{
	 	BG0_SUB = ON_SUB(0) &&  os9x_BG0;
	   	BG1_SUB = ON_SUB(1) &&  os9x_BG1;
	    BG2_SUB = ON_SUB(2) &&  os9x_BG2;
	    BG3_SUB = ON_SUB(3) &&  os9x_BG3;
	    OB_SUB = ON_SUB(4) &&  os9x_OBJ;
	} else BG0_SUB=BG1_SUB=BG2_SUB=BG3_SUB=OB_SUB=0;
	
	if ((os9x_ShowSub==4)&&(ADD_OR_SUB_ON_ANYTHING)
			&& ( (GPUPack.GFX.r2130 & 0x30) != 0x30)
			&& ( !((GPUPack.GFX.r2130 & 0x30) == 0x10) && ( IPPU.Clip[1].Count[5] == 0))
			&& ANYTHING_ON_SUB /*hack*/)
	{
	
		if (PPU.BGMode <= 1)
	    {
			
	    	if (ROM_GLOBAL [0x2105] & 8)
	    	{
	    		
	    		GPUPack.GFX.S = GPUPack.GFX.SubScreen;
	    		if (BG3_SUB && PPU.BGMode == 0)	{GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 3,0);}
				if (BG2_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 2,0);}
				if (OB_SUB)	{GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (0);else DrawOBJSFastNew(0);}				
				if (BG3_SUB && PPU.BGMode == 0)	{GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 3, 1);}				
				if (OB_SUB)	{GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (1);else DrawOBJSFastNew(1);}
				if (BG1_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 1, 0);}				
				if (BG0_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 0, 0);}
				if (OB_SUB)	{GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (2);else DrawOBJSFastNew(2);}																		
				if (BG1_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 1, 1);}						
				if (BG0_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 0, 1);}				
				if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (3);else DrawOBJSFastNew(3);}				
				if (BG2_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 2, 1);}
				
				asmClearScreenFast8(GPUPack.GFX.ZBuffer,starty,endy,0xFFFFFFFF);	
				asmClearScreenFast8(GPUPack.GFX.SubZBuffer,starty,endy,0);	
				
	    		GPUPack.GFX.S = GPUPack.GFX.Screen;	    		

		    	if (BG3 && PPU.BGMode == 0)	{os9x_SetTileRender(SUB_OR_ADD(3));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 3,0);}
				if (BG2){os9x_SetTileRender(SUB_OR_ADD(2));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 2,0);}
				if (OB)	{os9x_SetTileRender(SUB_OR_ADD(4));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (0,TRUE);else DrawOBJSFastNew(0);}				
				if (BG3 && PPU.BGMode == 0)	{os9x_SetTileRender(SUB_OR_ADD(3));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 3, 1);}				
				if (OB)	{os9x_SetTileRender(SUB_OR_ADD(4));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (1,TRUE);else DrawOBJSFastNew(1);}
				if (BG1){os9x_SetTileRender(SUB_OR_ADD(1));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 1, 0);}				
				if (BG0){os9x_SetTileRender(SUB_OR_ADD(0));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 0, 0);}
				if (OB)	{os9x_SetTileRender(SUB_OR_ADD(4));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (2,TRUE);else DrawOBJSFastNew(2);}																		
				if (BG1){os9x_SetTileRender(SUB_OR_ADD(1));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 1, 1);}						
				if (BG0){os9x_SetTileRender(SUB_OR_ADD(0));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 0, 1);}				
				if (OB){os9x_SetTileRender(SUB_OR_ADD(4));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (3,TRUE);else DrawOBJSFastNew(3);}				
				if (BG2){os9x_SetTileRender(SUB_OR_ADD(2));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 2, 1);}
				

	    	}
	    	else
	    	{	
		    	GPUPack.GFX.S = GPUPack.GFX.SubScreen;
		    	if (BG3_SUB && PPU.BGMode == 0){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 3, 0);}
				if (BG2_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 2, 0);}
				if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (0);else DrawOBJSFastNew(0);}
				if (BG3_SUB && PPU.BGMode == 0){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 3, 1);}
				if (BG2_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 2, 1);}
				if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (1);else DrawOBJSFastNew(1);}
				if (BG1_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 1, 0);}
				if (BG0_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 0, 0);}
				if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (2);else DrawOBJSFastNew(2);}
				if (BG1_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 1, 1);}
				if (BG0_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 0, 1);}
				if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (3);else DrawOBJSFastNew(3);}
	    	    	    
	    	    asmClearScreenFast8(GPUPack.GFX.ZBuffer,starty,endy,0xFFFFFFFF);
	    	    asmClearScreenFast8(GPUPack.GFX.SubZBuffer,starty,endy,0);	
				GPUPack.GFX.S = GPUPack.GFX.Screen;

			    if (BG3 && PPU.BGMode == 0){os9x_SetTileRender(SUB_OR_ADD(3));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 3, 0);}
				if (BG2){os9x_SetTileRender(SUB_OR_ADD(2));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 2, 0);}
				if (OB){os9x_SetTileRender(SUB_OR_ADD(4));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (0,TRUE);else DrawOBJSFastNew(0);}
				if (BG3 && PPU.BGMode == 0){os9x_SetTileRender(SUB_OR_ADD(3));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 3, 1);}
				if (BG2){os9x_SetTileRender(SUB_OR_ADD(2));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 2, 1);}
				if (OB){os9x_SetTileRender(SUB_OR_ADD(4));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (1,TRUE);else DrawOBJSFastNew(1);}
				if (BG1){os9x_SetTileRender(SUB_OR_ADD(1));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 1, 0);}
				if (BG0){os9x_SetTileRender(SUB_OR_ADD(0));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 0, 0);}
				if (OB){os9x_SetTileRender(SUB_OR_ADD(4));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (2,TRUE);else DrawOBJSFastNew(2);}
				if (BG1){os9x_SetTileRender(SUB_OR_ADD(1));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 1, 1);}
				if (BG0){os9x_SetTileRender(SUB_OR_ADD(0));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 0, 1);}
				if (OB){os9x_SetTileRender(SUB_OR_ADD(4));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (3,TRUE);else DrawOBJSFastNew(3);}
				

			}			
	    }
	    else if (PPU.BGMode != 7)
	    {	
		    GPUPack.GFX.S = GPUPack.GFX.SubScreen;
		    if (BG1_SUB&&(PPU.BGMode!=6)){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 1,0);}
			if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (0);else DrawOBJSFastNew(0);}			
			if (BG0_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 0,0);}
			if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (1);else DrawOBJSFastNew(1);}
			if (BG1_SUB&&(PPU.BGMode!=6)){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 1,1);}
			if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (2);else DrawOBJSFastNew(2);}			
			if (BG0_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 0,1);}
			if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (3);else DrawOBJSFastNew(3);}
	    
	        asmClearScreenFast8(GPUPack.GFX.ZBuffer,starty,endy,0xFFFFFFFF);
	        asmClearScreenFast8(GPUPack.GFX.SubZBuffer,starty,endy,0);	
		    GPUPack.GFX.S = GPUPack.GFX.Screen;
		    

	        if (BG1&&(PPU.BGMode!=6)){os9x_SetTileRender(SUB_OR_ADD(1));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 1,0);}
			if (OB){os9x_SetTileRender(SUB_OR_ADD(4));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (0,TRUE);else DrawOBJSFastNew(0);}			
			if (BG0){os9x_SetTileRender(SUB_OR_ADD(0));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 0,0);}
			if (OB){os9x_SetTileRender(SUB_OR_ADD(4));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (1,TRUE);else DrawOBJSFastNew(1);}
			if (BG1&&(PPU.BGMode!=6)){os9x_SetTileRender(SUB_OR_ADD(1));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 1,1);}
			if (OB){os9x_SetTileRender(SUB_OR_ADD(4));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (2,TRUE);else DrawOBJSFastNew(2);}			
			if (BG0){os9x_SetTileRender(SUB_OR_ADD(0));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 0,1);}
			if (OB){os9x_SetTileRender(SUB_OR_ADD(4));GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (3,TRUE);else DrawOBJSFastNew(3);}
			

	    }
	    else	    
	    {
	    	//Mod 7
			if (OB)
			{
			    FIXCLIP(4);
		    	if (!os9x_fastsprite) DrawOBJSNew (0);
				    else DrawOBJSFastNew(0);

			}				
			if (BG0 || ((ROM_GLOBAL [0x2133] & 0x40) && BG1))
			{
			    //int bg;
			    FIXCLIP(0);
			    if (OB&&(ROM_GLOBAL [0x2133] & 0x40))
			    {
			    	
			    	info(32,11,"mode 7 ext");
					GPUPack.GFX.Mode7Mask = 0x7f;
					GPUPack.GFX.Mode7PriorityMask = 0x80;
					//bg = 1;
					
					//int pitch;
					/*if (os9x_softrendering)*/// pitch=256;
//					else pitch=320;

					DrawBGMode7Background16PrioNew ((uint16*)GPUPack.GFX.Screen, 1,0);

			    	FIXCLIP(4);	    	
					if (!os9x_fastsprite) DrawOBJSNew (1);
				    else DrawOBJSFastNew(1);
					
					DrawBGMode7Background16PrioNew ((uint16*)GPUPack.GFX.Screen, 1,1);
										
					FIXCLIP(4);	    	
					if (!os9x_fastsprite) DrawOBJSNew (2);
				    else DrawOBJSFastNew(2);
					if (!os9x_fastsprite) DrawOBJSNew (3);
				    else DrawOBJSFastNew(3);
			    }
			    
			    else
			    {
			    	info(32,11,"mode 7 norm");
					GPUPack.GFX.Mode7Mask = 0xff;
					GPUPack.GFX.Mode7PriorityMask = 0;
					//bg = 0;
					
					//int pitch;
					/*if (os9x_softrendering)*/// pitch=256;
					//else pitch=320;
					
					DrawBGMode7Background16New ((uint16*)GPUPack.GFX.Screen, 0);
				    	
				    if (OB)
					{
					    FIXCLIP(4);		
					    if (!os9x_fastsprite) 
					    {
					    	DrawOBJSNew (1);
					    	DrawOBJSNew (2);
					    	DrawOBJSNew (3);
					    }
					    else 
					    {
					    	DrawOBJSFastNew(1);
					    	DrawOBJSFastNew(2);
					    	DrawOBJSFastNew(3);
						}
					}				
			    }			    
			}
			else
			{
				if (OB)
				{
				    FIXCLIP(4);		
				    if (!os9x_fastsprite) 
					{
					   	DrawOBJSNew (1);
					   	DrawOBJSNew (2);
					   	DrawOBJSNew (3);
					}
					else 
					{
					   	DrawOBJSFastNew(1);
					   	DrawOBJSFastNew(2);
					   	DrawOBJSFastNew(3);
					}
				}				
			}
	    }
	
	
	}
	else
	{
		switch (os9x_ShowSub&3)
		{
			case 3:
				//Only one, SUB
				BG0=BG1=BG2=BG3=OB=0;
				break;
			case 2:
				//Only one, MAIN
				BG0_SUB=BG1_SUB=BG2_SUB=BG3_SUB=OB_SUB=0;
				break;
			case 1:
				//Both, equal priority
				BG0|=BG0_SUB;
				BG1|=BG1_SUB;
				BG2|=BG2_SUB;
				BG3|=BG3_SUB;
				OB|=OB_SUB;
				
				BG0_SUB=BG1_SUB=BG2_SUB=BG3_SUB=OB_SUB=0;
				break;
			case 0:		
				//Both, MAIN has priority
				BG0_SUB = BG0_SUB && (!BG0);
				BG1_SUB = BG1_SUB && (!BG1);
				BG2_SUB = BG2_SUB && (!BG2);
				BG3_SUB = BG3_SUB && (!BG3);
				OB_SUB = OB_SUB && (!OB);
				break;
		}	    
	    
	    if (PPU.BGMode <= 1)
	    {
			
	    	if (ROM_GLOBAL [0x2105] & 8)
	    	{
	    		if (BG3_SUB && PPU.BGMode == 0)	{GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 3,0);}
				if (BG2_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 2,0);}
				if (OB_SUB)	{GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (0);else DrawOBJSFastNew(0);}				
				if (BG3_SUB && PPU.BGMode == 0)	{GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 3, 1);}				
				if (OB_SUB)	{GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (1);else DrawOBJSFastNew(1);}
				if (BG1_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 1, 0);}				
				if (BG0_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 0, 0);}
				if (OB_SUB)	{GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (2);else DrawOBJSFastNew(2);}																		
				if (BG1_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 1, 1);}						
				if (BG0_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 0, 1);}				
				if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (3);else DrawOBJSFastNew(3);}				
				if (BG2_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 2, 1);}
	    	
	    	
		    	if (BG3 && PPU.BGMode == 0)	{GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 3,0);}
				if (BG2){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 2,0);}
				if (OB)	{GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (0);else DrawOBJSFastNew(0);}				
				if (BG3 && PPU.BGMode == 0)	{GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 3, 1);}				
				if (OB)	{GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (1);else DrawOBJSFastNew(1);}
				if (BG1){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 1, 0);}				
				if (BG0){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 0, 0);}
				if (OB)	{GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (2);else DrawOBJSFastNew(2);}																		
				if (BG1){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 1, 1);}						
				if (BG0){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 0, 1);}				
				if (OB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (3);else DrawOBJSFastNew(3);}				
				if (BG2){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 2, 1);}
	    	}
	    	else
	    	{	
		    	if (BG3_SUB && PPU.BGMode == 0){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 3, 0);}
				if (BG2_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 2, 0);}
				if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (0);else DrawOBJSFastNew(0);}
				if (BG3_SUB && PPU.BGMode == 0){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 3, 1);}
				if (BG2_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 2, 1);}
				if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (1);else DrawOBJSFastNew(1);}
				if (BG1_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 1, 0);}
				if (BG0_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 0, 0);}
				if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (2);else DrawOBJSFastNew(2);}
				if (BG1_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 1, 1);}
				if (BG0_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 0, 1);}
				if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (3);else DrawOBJSFastNew(3);}
	    	    	    
			    if (BG3 && PPU.BGMode == 0){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 3, 0);}
				if (BG2){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 2, 0);}
				if (OB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (0);else DrawOBJSFastNew(0);}
				if (BG3 && PPU.BGMode == 0){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 3, 1);}
				if (BG2){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 2, 1);}
				if (OB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (1);else DrawOBJSFastNew(1);}
				if (BG1){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 1, 0);}
				if (BG0){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 0, 0);}
				if (OB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (2);else DrawOBJSFastNew(2);}
				if (BG1){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 1, 1);}
				if (BG0){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 0, 1);}
				if (OB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (3);else DrawOBJSFastNew(3);}
			}			
	    }
	    else if (PPU.BGMode != 7)
	    {	
		    if (BG1_SUB&&(PPU.BGMode!=6)){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 1,0);}
			if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (0);else DrawOBJSFastNew(0);}			
			if (BG0_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 0,0);}
			if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (1);else DrawOBJSFastNew(1);}
			if (BG1_SUB&&(PPU.BGMode!=6)){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 1,1);}
			if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (2);else DrawOBJSFastNew(2);}			
			if (BG0_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];DrawBackgroundNew (PPU.BGMode, 0,1);}
			if (OB_SUB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];if (!os9x_fastsprite) DrawOBJSNew (3);else DrawOBJSFastNew(3);}
	    
	        if (BG1&&(PPU.BGMode!=6)){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 1,0);}
			if (OB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (0);else DrawOBJSFastNew(0);}			
			if (BG0){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 0,0);}
			if (OB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (1);else DrawOBJSFastNew(1);}
			if (BG1&&(PPU.BGMode!=6)){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 1,1);}
			if (OB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (2);else DrawOBJSFastNew(2);}			
			if (BG0){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];DrawBackgroundNew (PPU.BGMode, 0,1);}
			if (OB){GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];if (!os9x_fastsprite) DrawOBJSNew (3);else DrawOBJSFastNew(3);}
	    }
	    else	    
	    {
	    	//Mod 7
			if (OB)
			{
			    FIXCLIP(4);
		    	if (!os9x_fastsprite) DrawOBJSNew (0);
				    else DrawOBJSFastNew(0);

			}				
			if (BG0 || ((ROM_GLOBAL [0x2133] & 0x40) && BG1))
			{
			    //int bg;
			    FIXCLIP(0);
			    if (OB&&(ROM_GLOBAL [0x2133] & 0x40))
			    {
			    	info(32,11,"mode 7 ext");
					GPUPack.GFX.Mode7Mask = 0x7f;
					GPUPack.GFX.Mode7PriorityMask = 0x80;
					//bg = 1;
					
					//int pitch;
					/*if (os9x_softrendering)*/ //pitch=256;
					//else pitch=320;

					DrawBGMode7Background16PrioNew ((uint16*)GPUPack.GFX.Screen, 1,0);

			    	FIXCLIP(4);	    	
					if (!os9x_fastsprite) DrawOBJSNew (1);
				    else DrawOBJSFastNew(1);
					
					DrawBGMode7Background16PrioNew ((uint16*)GPUPack.GFX.Screen, 1,1);
										
					FIXCLIP(4);	    	
					if (!os9x_fastsprite) DrawOBJSNew (2);
				    else DrawOBJSFastNew(2);
					if (!os9x_fastsprite) DrawOBJSNew (3);
				    else DrawOBJSFastNew(3);
			    }
			    
			    else
			    {
			    	info(32,11,"mode 7 norm");
					GPUPack.GFX.Mode7Mask = 0xff;
					GPUPack.GFX.Mode7PriorityMask = 0;
					//bg = 0;
					
					//int pitch;
					/*if (os9x_softrendering)*/ //pitch=256;
					//else pitch=320;
					
					/*int x;
					for (x=0;x<32768;x++){
						VRAMmode7[x]=VRAM[x<<1];
						VRAMmode7[x|32768]=VRAM[(x<<1)|1];
					}*/
					
					
					DrawBGMode7Background16New ((uint16*)GPUPack.GFX.Screen, 0);
				  
				    	
				    if (OB)
					{
					    FIXCLIP(4);		
					    if (!os9x_fastsprite) 
					    {
					    	DrawOBJSNew (1);
					    	DrawOBJSNew (2);
					    	DrawOBJSNew (3);
					    }
					    else 
					    {
					    	DrawOBJSFastNew(1);
					    	DrawOBJSFastNew(2);
					    	DrawOBJSFastNew(3);
						}
					}				
			    }			    
			}
			else
			{
				if (OB)
				{
				    FIXCLIP(4);		
				    if (!os9x_fastsprite) 
					{
					   	DrawOBJSNew (1);
					   	DrawOBJSNew (2);
					   	DrawOBJSNew (3);
					}
					else 
					{
					   	DrawOBJSFastNew(1);
					   	DrawOBJSFastNew(2);
					   	DrawOBJSFastNew(3);
					}
				}				
			}
	    }
	}
#ifdef PROFILING
	PROF_END(12);
#endif
	    
	    
	}
    
    IPPU.PreviousLine = IPPU.CurrentLine;
  if (os9x_softrendering==4){  
		u32 *src,*dst;
		src=(u32*)(GPUPack.GFX.Screen+starty*GPUPack.GFX.Pitch2);		
		/*if (!os9x_render)	{
				dst=(u32*)(0x44000000+512*2*(272*swap_buf+starty ));
				for (int y=endy-starty+1;y;y--,dst+=256/2) __memcpy4a((long unsigned int*)dst,(long unsigned int*)src,256*2/4);				
			} else*/ {
				dst=(u32*)(0x44000000+512*272*2*2+256*240*2+2*256*256*2+starty*256*2 );
				for (int y=endy-starty+1;y;y--) __memcpy4a((long unsigned int*)dst,(long unsigned int*)src,256*2/4);				
			}
	}
	// put back cache off and writeback Dcache
	GPUPack.GFX.Screen = (uint8*)CACHE_OFF(GPUPack.GFX.Screen);
	sceKernelDcacheWritebackAll();
	//sceKernelDcacheWritebackInvalidateAll();
	ResetClipWindowsFix();
			
#ifdef PROFILING
	PROF_END(8);
#endif    
}

