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
 
#if 0

//#ifndef asmPPU
#include "snes9x.h"

#include "memmap.h"
#include "ppu.h"
#include "cpuexec.h"
#include "display.h"
#include "gfx.h"
#include "apu.h"


//#define asmPPU16
//#define asmPPU8
//#define asmPPU16T

#define TILE_INLINE inline

typedef union
{
    struct { uint8 b0,b1,b2,b3; } B;
    uint32 W;
} yo_uint32;

uint8 *SubScreenTranspBuffer;
uint8 tmpCache[64];



#ifdef asmPPU16
extern "C" void asmDrawTile16(uint32 cache,uint32 solidbuf,uint32 OffsetGP32,uint32 Flip);
extern "C" void asmDrawTileClipped16(uint32 cache,uint32 solidbuf,uint32 OffsetGP32,uint32 Flip);
#endif

#define yoRGB_REMOVE_LOW_BITS_MASK ((30<<11)|(30<<6)|(30<<1))
#define yoRGB_LOW_BITS_MASK ((1<<11)|(1<<6)|(1<<1)|1)
#define yoRGB_HI_BITS_MASKx2 ( ((16<<11)|(16<<6)|(16<<1))<<1 )


#define COLOR_ADD(C1, C2) \
GFX.X2 [((((C1) & yoRGB_REMOVE_LOW_BITS_MASK) + \
	  ((C2) & yoRGB_REMOVE_LOW_BITS_MASK)) >> 1) + \
	((C1) & (C2) & yoRGB_LOW_BITS_MASK)]


#define COLOR_ADD1_2(C1, C2) \
(((((C1) & yoRGB_REMOVE_LOW_BITS_MASK) + \
          ((C2) & yoRGB_REMOVE_LOW_BITS_MASK)) >> 1) + \
         ((C1) & (C2) & yoRGB_LOW_BITS_MASK) | ALPHA_BITS_MASK)
         
#define COLOR_SUB(C1, C2) \
GFX.ZERO_OR_X2 [(((C1) | yoRGB_HI_BITS_MASKx2) - \
		 ((C2) & yoRGB_REMOVE_LOW_BITS_MASK)) >> 1]
		 
#define COLOR_SUB1_2(C1, C2) \
GFX.ZERO [(((C1) | yoRGB_HI_BITS_MASKx2) - \
	   ((C2) & yoRGB_REMOVE_LOW_BITS_MASK)) >> 1]



TILE_INLINE uint8 ConvertTile16New (uint8 *pCache,uint32 TileAddr,uint16 *ScreenColors)
{
    register uint8 *tp = &Memory.VRAM[TileAddr];
    uint8 *p = tmpCache;
    uint32 non_zero,tile_opaque,tile_mono;
    uint8 line;
    yo_uint32 p1;
    yo_uint32 p2;
    register uint8 pix;
	    
    non_zero=0;	
    switch (BG.BitShift)
    {
    case 8:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    p1.W = 0;
	    p2.W = 0;	    	    
	    if ((pix = *(tp + 0)))
	    {
		p1.W |= odd_high[0][pix >> 4];
		p2.W |= odd_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 1)))
	    {
		p1.W |= even_high[0][pix >> 4];
		p2.W |= even_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 16)))
	    {
		p1.W |= odd_high[1][pix >> 4];
		p2.W |= odd_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 17)))
	    {
		p1.W |= even_high[1][pix >> 4];
		p2.W |= even_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 32)))
	    {
		p1.W |= odd_high[2][pix >> 4];
		p2.W |= odd_low[2][pix & 0xf];
	    }
	    if ((pix = *(tp + 33)))
	    {
		p1.W |= even_high[2][pix >> 4];
		p2.W |= even_low[2][pix & 0xf];
	    }
	    if ((pix = *(tp + 48)))
	    {
		p1.W |= odd_high[3][pix >> 4];
		p2.W |= odd_low[3][pix & 0xf];
	    }
	    if ((pix = *(tp + 49)))
	    {
		p1.W |= even_high[3][pix >> 4];
		p2.W |= even_low[3][pix & 0xf];
	    }	    	    
	    
	    
	    p[0]  = p1.B.b0;
	    p[1*8]  = p1.B.b1;
	    p[2*8] = p1.B.b2;
	    p[3*8] = p1.B.b3;
	    p[4*8] = p2.B.b0;
	    p[5*8] = p2.B.b1;
	    p[6*8] = p2.B.b2;
	    p[7*8] = p2.B.b3;
	    
	    p+=1;
	    non_zero |= p1.W | p2.W;	    	    
	}
	break;

    case 4:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    p1.W = 0;
	    p2.W = 0;	    
	    if ((pix = *(tp + 0)))
	    {
		p1.W |= odd_high[0][pix >> 4];
		p2.W |= odd_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 1)))
	    {
		p1.W |= even_high[0][pix >> 4];
		p2.W |= even_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 16)))
	    {
		p1.W |= odd_high[1][pix >> 4];
		p2.W |= odd_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 17)))
	    {
		p1.W |= even_high[1][pix >> 4];
		p2.W |= even_low[1][pix & 0xf];
	    }	    
	    p[0]  = p1.B.b0;
	    p[1*8]  = p1.B.b1;
	    p[2*8] = p1.B.b2;
	    p[3*8] = p1.B.b3;
	    p[4*8] = p2.B.b0;
	    p[5*8] = p2.B.b1;
	    p[6*8] = p2.B.b2;
	    p[7*8] = p2.B.b3;
	    
	    p+=1;
	    non_zero |= p1.W | p2.W;	    	    
	}
	break;

    case 2:    	
	for (line = 8; line != 0; line--, tp += 2)
	{
	    p1.W = 0;
	    p2.W = 0; 	    
	    if ((pix = *(tp + 0)))
	    {
		p1.W |= odd_high[0][pix >> 4];
		p2.W |= odd_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 1)))
	    {
		p1.W |= even_high[0][pix >> 4];
		p2.W |= even_low[0][pix & 0xf];
	    }	    	   
	    
		p[0]  = p1.B.b0;
	    p[1*8]  = p1.B.b1;
	    p[2*8] = p1.B.b2;
	    p[3*8] = p1.B.b3;
	    p[4*8] = p2.B.b0;
	    p[5*8] = p2.B.b1;
	    p[6*8] = p2.B.b2;
	    p[7*8] = p2.B.b3;
	    
	    p+=1;
	    non_zero |= p1.W | p2.W;	    	    	}
	break;
	}		
	if (non_zero) 
	{	
		tile_opaque=1;
		p=tmpCache;
	    uint8 *q = pCache;
	    uint32 i;
		for (line = 8;line != 0; line--,q++,p+=8)
		{
			if (!p[0]) i=0x80;
			else i=0;
			if (!p[1]) i|=0x40;
			if (!p[2]) i|=0x20;
			if (!p[3]) i|=0x10;
			if (!p[4]) i|=0x08;
			if (!p[5]) i|=0x04;
			if (!p[6]) i|=0x02;
			if (!p[7]) i|=0x01;					
			if (i) tile_opaque=0;
			*q=i;
		}
	    uint16 *r = (uint16*)(pCache)+4;
		for (line = 0;line <64;line++)
			r[line]=ScreenColors[tmpCache[line]];
			

		if (tile_opaque) return 2; //Tile is cached and opaque
		return 3; //Tile is cached and transp
	}		
	else return 1; //Tile is totally transparent		
}


/********************************/
TILE_INLINE void NORMAL16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;    
   	    
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) *Screen = *Pixels; \
   	    	Screen+=256; Pixels++;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

TILE_INLINE void NORMAL16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;    
   	    #define FN \
   	    	*Screen = *Pixels; \
   	    	Screen+=256; Pixels++;   	    	   	    
   	    FN
   	    FN
   	    FN
   	    FN
   	    FN
   	    FN
   	    FN
   	    FN   	       	    	
   	    #undef FN
   	    
}

TILE_INLINE void FLIPPED16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;
   	    
   	    Screen += 7*256;
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) *Screen = *Pixels; \
   	    	Screen-=256; Pixels++;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

TILE_INLINE void FLIPPED16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;
   	    
   	    Screen += 7*256;

    	#define FN \
   	    	*Screen = *Pixels; \
   	    	Screen-=256; Pixels++;   	    	   	    
   	    FN
   	    FN
   	    FN
   	    FN
   	    FN
   	    FN
   	    FN
   	    FN   	       	    	
   	    #undef FN
}



TILE_INLINE void NORMAL16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;    
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { *Screen = *Pixels; *ZB=index_spr; } \
   	    	Screen+=256; Pixels++; ZB+=256;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

TILE_INLINE void NORMAL16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;    
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {*Screen=*Pixels; *ZB=index_spr;} \
	   	    Screen+=256; Pixels++; ZB+=256;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
}

TILE_INLINE void FLIPPED16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset + 7*256;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + 7*256;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { *Screen = *Pixels; *ZB=index_spr; }\
   	    	Screen-=256; Pixels++; ZB-=256;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

TILE_INLINE void FLIPPED16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset + 7*256;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + 7*256;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {*Screen=*Pixels; *ZB=index_spr;} \
   		    Screen-=256; Pixels++; ZB-=256;
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		#undef FN
}




TILE_INLINE void NORMAL_ADD_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen--; SubScreen--; Pixels++;   	    	   	    
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

TILE_INLINE void NORMAL_ADD_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen--; SubScreen--; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN   	    
	   	#undef FN
}

TILE_INLINE void FLIPPED_ADD_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{   	    
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    Screen -= 7;
   	    SubScreen -= 7;
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen++; SubScreen++; Pixels++;   	    	   	    	
   	    	
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

TILE_INLINE void FLIPPED_ADD_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    Screen -= 7;
   	    SubScreen -= 7;
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen++; SubScreen++; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	#undef FN
}



TILE_INLINE void NORMAL_ADD_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
	   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
   	    		*ZB=index_spr; \
   	    	} \
   	    	Screen--; SubScreen--; Pixels++; ZB--;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

TILE_INLINE void NORMAL_ADD_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
	   	    Screen--; SubScreen--; Pixels++; ZB--;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
}

TILE_INLINE void FLIPPED_ADD_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset - 7;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset - 7;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset - 7;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
   	    		if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   		*ZB=index_spr; }\
   	    	Screen++; SubScreen++; Pixels++; ZB++;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

TILE_INLINE void FLIPPED_ADD_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset - 7;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset - 7;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset - 7;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
   		    Screen++; SubScreen++; Pixels++; ZB++;
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		#undef FN
}


TILE_INLINE void NORMAL_ADD1_2_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen--; SubScreen--; Pixels++;   	    	   	    
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

TILE_INLINE void NORMAL_ADD1_2_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen--; SubScreen--; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN   	    
	   	#undef FN
}

TILE_INLINE void FLIPPED_ADD1_2_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{   	    
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    Screen -= 7;
   	    SubScreen -= 7;
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen++; SubScreen++; Pixels++;   	    	   	    	
   	    	
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

TILE_INLINE void FLIPPED_ADD1_2_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    Screen -= 7;
   	    SubScreen -= 7;
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen++; SubScreen++; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	#undef FN
}



TILE_INLINE void NORMAL_ADD1_2_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
	   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
   	    		*ZB=index_spr; \
   	    	} \
   	    	Screen--; SubScreen--; Pixels++; ZB--;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

TILE_INLINE void NORMAL_ADD1_2_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
	   	    Screen--; SubScreen--; Pixels++; ZB--;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
}

TILE_INLINE void FLIPPED_ADD1_2_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset - 7;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset - 7;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset - 7;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
   	    		if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   		*ZB=index_spr; }\
   	    	Screen++; SubScreen++; Pixels++; ZB++;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

TILE_INLINE void FLIPPED_ADD1_2_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset - 7;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset - 7;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset - 7;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
   		    Screen++; SubScreen++; Pixels++; ZB++;
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		#undef FN
}

TILE_INLINE void NORMAL_SUB_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen--; SubScreen--; Pixels++;   	    	   	    
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

TILE_INLINE void NORMAL_SUB_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen--; SubScreen--; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN   	    
	   	#undef FN
}

TILE_INLINE void FLIPPED_SUB_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{   	    
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    Screen -= 7;
   	    SubScreen -= 7;
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen++; SubScreen++; Pixels++;   	    	   	    	
   	    	
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

TILE_INLINE void FLIPPED_SUB_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    Screen -= 7;
   	    SubScreen -= 7;
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen++; SubScreen++; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	#undef FN
}



TILE_INLINE void NORMAL_SUB_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
	   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
   	    		*ZB=index_spr; \
   	    	} \
   	    	Screen--; SubScreen--; Pixels++; ZB--;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

TILE_INLINE void NORMAL_SUB_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
	   	    Screen--; SubScreen--; Pixels++; ZB--;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
}

TILE_INLINE void FLIPPED_SUB_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset - 7;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset - 7;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset - 7;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
   	    		if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   		*ZB=index_spr; }\
   	    	Screen++; SubScreen++; Pixels++; ZB++;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

TILE_INLINE void FLIPPED_SUB_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset - 7;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset - 7;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset - 7;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
   		    Screen++; SubScreen++; Pixels++; ZB++;
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		#undef FN
}


TILE_INLINE void NORMAL_SUB1_2_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen--; SubScreen--; Pixels++;   	    	   	    
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

TILE_INLINE void NORMAL_SUB1_2_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen--; SubScreen--; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN   	    
	   	#undef FN
}

TILE_INLINE void FLIPPED_SUB1_2_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{   	    
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    Screen -= 7;
   	    SubScreen -= 7;
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen++; SubScreen++; Pixels++;   	    	   	    	
   	    	
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

TILE_INLINE void FLIPPED_SUB1_2_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    Screen -= 7;
   	    SubScreen -= 7;
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen++; SubScreen++; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	#undef FN
}



TILE_INLINE void NORMAL_SUB1_2_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
	   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
   	    		*ZB=index_spr; \
   	    	} \
   	    	Screen--; SubScreen--; Pixels++; ZB--;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

TILE_INLINE void NORMAL_SUB1_2_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
	   	    Screen--; SubScreen--; Pixels++; ZB--;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
}

TILE_INLINE void FLIPPED_SUB1_2_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset - 7;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset - 7;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset - 7;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
   	    		if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   		*ZB=index_spr; }\
   	    	Screen++; SubScreen++; Pixels++; ZB++;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

TILE_INLINE void FLIPPED_SUB1_2_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset - 7;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset - 7;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset - 7;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
   		    Screen++; SubScreen++; Pixels++; ZB++;
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		#undef FN
}




void DrawTile16New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),0);			
#else
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),3);	   		
#else	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED16_O (Offset, (uint16*)bp);
#endif
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),1);
#else	   		
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED16_O (Offset, (uint16*)bp);
#endif
		}
	}
	else
	{
		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)),0);
#else			
		    headerbp = pCache;
			bp = pCache+8;					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+7*2),3);
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)),1);
#else			
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
		}
	}
}

void DrawClippedTile16New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU16
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU16
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else					  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU16
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);
#else			
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU16
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);
#else			
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED16_O (Offset, (uint16*)bp);
#endif		    	
		}
	}
	else
	{

		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU16
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU16
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else			
	    	
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
#ifdef asmPPU16
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);			
#else
	   		
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU16
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);			
#else		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
		}
	}
}

void DrawTile16NewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{
		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawClippedTile16NewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL16_SPR_O (Offset, (uint16*)bp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{

		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawHiResTile16New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, Offset++)
			   	NORMAL16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset--)
		    	NORMAL16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset--)
		    	FLIPPED16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset++)
		    	FLIPPED16_O (Offset, (uint16*)bp);

		}
	}
	else
	{
		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
			   	NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

void DrawHiResClippedTile16New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    

    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP		  
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset++)
			   	NORMAL16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP		  
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset--)
		    	NORMAL16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset--)
		    	FLIPPED16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset++)
		    	FLIPPED16_O (Offset, (uint16*)bp);

		}
	}
	else
	{

		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
			   	NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

/****************** ADD ***********************/
void DrawTile16ADDNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);

    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),0);			
#else
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_ADD_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_ADD_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),3);	   		
#else	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);
#endif
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),1);
#else	   		
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);
#endif
		}
	}
	else
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    		    
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)),0);
#else			
		    headerbp = pCache;
			bp = pCache+8;					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    	    	
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+7*2),3);
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)),1);
#else			
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
		}
	}
}

void DrawClippedTile16ADDNew (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_ADD_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else					  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_ADD_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);
#else			
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);
#else			
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);
#endif		    	
		}
	}
	else
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else			
	    	
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);			
#else
	   		
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);			
#else		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
		}
	}
}

void DrawTile16ADDNewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_ADD_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_ADD_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_ADD_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_ADD_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL_ADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_ADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_ADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_ADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawClippedTile16ADDNewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_ADD_16_SPR_O (Offset, (uint16*)bp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_ADD_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_ADD_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_ADD_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL_ADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_ADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_ADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_ADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawHiResTile16ADDNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, Offset++)
			   	NORMAL_ADD_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset--)
		    	NORMAL_ADD_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset--)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset++)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
			   	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

void DrawHiResClippedTile16ADDNew (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    

    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP		  
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset++)
			   	NORMAL_ADD_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP		  
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset--)
		    	NORMAL_ADD_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset--)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset++)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
			   	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

/****************** ADD1_2 ***********************/
void DrawTile16ADD1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),0);			
#else
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),3);	   		
#else	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);
#endif
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),1);
#else	   		
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);
#endif
		}
	}
	else
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    		    
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)),0);
#else			
		    headerbp = pCache;
			bp = pCache+8;					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    	    	
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+7*2),3);
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)),1);
#else			
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
		}
	}
}

void DrawClippedTile16ADD1_2New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else					  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);
#else			
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);
#else			
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);
#endif		    	
		}
	}
	else
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else			
	    	
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);			
#else
	   		
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);			
#else		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
		}
	}
}

void DrawTile16ADD1_2NewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_ADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_ADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_ADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_ADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL_ADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_ADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_ADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_ADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawClippedTile16ADD1_2NewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_ADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_ADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_ADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_ADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL_ADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_ADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_ADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_ADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawHiResTile16ADD1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, Offset++)
			   	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset--)
		    	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset--)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset++)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
			   	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

void DrawHiResClippedTile16ADD1_2New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    

    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP		  
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset++)
			   	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP		  
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset--)
		    	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset--)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset++)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
			   	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}


/****************** SUB ***********************/
void DrawTile16SUBNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),0);			
#else
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_SUB_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_SUB_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),3);	   		
#else	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);
#endif
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),1);
#else	   		
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);
#endif
		}
	}
	else
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    		    
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)),0);
#else			
		    headerbp = pCache;
			bp = pCache+8;					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    	    	
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+7*2),3);
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)),1);
#else			
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
		}
	}
}

void DrawClippedTile16SUBNew (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_SUB_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else					  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_SUB_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);
#else			
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);
#else			
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);
#endif		    	
		}
	}
	else
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else			
	    	
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);			
#else
	   		
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);			
#else		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
		}
	}
}

void DrawTile16SUBNewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_SUB_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_SUB_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_SUB_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_SUB_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL_SUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_SUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_SUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_SUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawClippedTile16SUBNewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_SUB_16_SPR_O (Offset, (uint16*)bp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_SUB_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_SUB_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_SUB_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL_SUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_SUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_SUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_SUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawHiResTile16SUBNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, Offset++)
			   	NORMAL_SUB_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset--)
		    	NORMAL_SUB_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset--)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset++)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
			   	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

void DrawHiResClippedTile16SUBNew (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    

    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP		  
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset++)
			   	NORMAL_SUB_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP		  
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset--)
		    	NORMAL_SUB_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset--)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset++)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
			   	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

/****************** SUB1_2 ***********************/
void DrawTile16SUB1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),0);			
#else
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),3);	   		
#else	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);
#endif
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),1);
#else	   		
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);
#endif
		}
	}
	else
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    		    
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)),0);
#else			
		    headerbp = pCache;
			bp = pCache+8;					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    	    	
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+7*2),3);
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)),1);
#else			
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
		}
	}
}

void DrawClippedTile16SUB1_2New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else					  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);
#else			
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);
#else			
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);
#endif		    	
		}
	}
	else
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else			
	    	
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);			
#else
	   		
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);			
#else		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
		}
	}
}

void DrawTile16SUB1_2NewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_SUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_SUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_SUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_SUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL_SUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_SUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_SUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_SUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawClippedTile16SUB1_2NewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_SUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_SUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_SUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_SUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL_SUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_SUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_SUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_SUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawHiResTile16SUB1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, Offset++)
			   	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset--)
		    	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset--)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset++)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
			   	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

void DrawHiResClippedTile16SUB1_2New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    

    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP		  
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset++)
			   	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP		  
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset--)
		    	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset--)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset++)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{

		Offset += StartLine; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
			   	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

#endif
