#include "snes9x.h"

#include "memmap.h"
#include "ppu.h"
#include "cpuexec.h"
#include "display.h"
#include "gfx.h"
#include "apu.h"
#include "cheats.h"
#include "tile.h"

#define M7 19

#define RENDER_BACKGROUND_MODE7NEW_PHASE2(TYPE,FUNC,PITCH) \
    \
    uint8 *VRAM1 = VRAM + 1; \
    if (GFX.r2130 & 1) \
    { \
	if (IPPU.DirectColourMapsNeedRebuild) \
	    S9xBuildDirectColourMaps (); \
	GFX.ScreenColors = DirectColourMaps [0]; \
    } \
    else \
	GFX.ScreenColors = IPPU.ScreenColors; \
\
    int aa, cc; \
    int dir; \
    int startx, endx; \
    uint32 Left = 0; \
    uint32 Right = 256; \
    uint32 ClipCount = GFX.pCurrentClip->Count [bg]; \
\
    if (!ClipCount) \
	ClipCount = 1; \
\
    Screen += GFX.StartY*PITCH; \
/*    uint8 *Depth = GFX.DB + GFX.StartY * GFX.PPL;*/ \
    struct SLineMatrixData *l = &LineMatrixData [GFX.StartY]; \
\
    for (uint32 Line = GFX.StartY; Line <= GFX.EndY; Line++, Screen +=PITCH/*, Depth += GFX.PPL*/, l++) \
    { \
	int yy; \
\
	int32 HOffset = ((int32) LineData [Line].BG[0].HOffset << M7) >> M7; \
	int32 VOffset = ((int32) LineData [Line].BG[0].VOffset << M7) >> M7; \
\
	int32 CentreX = ((int32) l->CentreX << M7) >> M7; \
	int32 CentreY = ((int32) l->CentreY << M7) >> M7; \
\
	if (PPU.Mode7VFlip) \
	    yy = 261 - (int) Line; \
	else \
	    yy = Line; \
\
	if (PPU.Mode7Repeat == 0) \
	    yy += (VOffset - CentreY) % 1023; \
	else \
	    yy += VOffset - CentreY; \
	int BB = l->MatrixB * yy + (CentreX << 8); \
	int DD = l->MatrixD * yy + (CentreY << 8); \
\
	for (uint32 clip = 0; clip < ClipCount; clip++) \
	{ \
	    if (GFX.pCurrentClip->Count [bg]) \
	    { \
		Left = GFX.pCurrentClip->Left [clip][bg]; \
		Right = GFX.pCurrentClip->Right [clip][bg]; \
		if (Right <= Left) \
		    continue; \
	    } \
	    TYPE *p = (TYPE *) Screen + Left; \
	    if (PPU.Mode7HFlip) { \
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
	    if (PPU.Mode7Repeat == 0) \
		xx = startx + (HOffset - CentreX) % 1023; \
	    else \
		xx = startx + HOffset - CentreX; \
	    int AA = l->MatrixA * xx; \
	    int CC = l->MatrixC * xx; \
  		AA +=BB;\
  		CC +=DD;\
		int length = (endx-startx); \
		if (length<0) { length = -length; }\
\
		register uint32 Y;\
		register uint32 X;\
		register uint8* TileData;\
		register uint32 b;\
	  if (!PPU.Mode7Repeat) { \
			if (cc==0) {\
				register uint8* AdrY;\
				register int AdrY2;\
				Y = CC>>4; AdrY = &VRAM[((Y & 0x3f80) << 1)]; AdrY2 = (Y & 0x70);\
				while ((length>>3)!=0) {\
				  	X = AA >> 7;\
			  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
					b = *(TileData + AdrY2 + (X & 14)); \
					*p++ = (FUNC); \
					AA+= aa;\
					\
				  	X = AA >> 7;\
			  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
					b = *(TileData + AdrY2 + (X & 14)); \
					*p++ = (FUNC); \
					AA+= aa;\
					\
				  	X = AA >> 7;\
			  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
					b = *(TileData + AdrY2 + (X & 14)); \
					*p++ = (FUNC); \
					AA+= aa;\
					\
				  	X = AA >> 7;\
			  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
					b = *(TileData + AdrY2 + (X & 14)); \
					*p++ = (FUNC); \
					AA+= aa;\
					\
				  	X = AA >> 7;\
			  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
					b = *(TileData + AdrY2 + (X & 14)); \
					*p++ = (FUNC); \
					AA+= aa;\
					\
				  	X = AA >> 7;\
			  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
					b = *(TileData + AdrY2 + (X & 14)); \
					*p++ = (FUNC); \
					AA+= aa;\
					\
				  	X = AA >> 7;\
			  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
					b = *(TileData + AdrY2 + (X & 14)); \
					*p++ = (FUNC); \
					AA+= aa;\
					\
				  	X = AA >> 7;\
			  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
					b = *(TileData + AdrY2 + (X & 14)); \
					*p++ = (FUNC); \
					AA+= aa;\
					\
					length -=8;\
				}\
				\
				while (length!=0) {\
				  	X = (AA >> 8);\
			  		TileData = VRAM1 + (AdrY[(X >> 2) & ~1] << 7); \
					b = *(TileData + AdrY2 + ((X & 7) << 1)); \
				  	*p++  = (FUNC);\
                    AA += aa;\
					length--;\
				}\
			} else {\
				while (length!=0) {\
				  	X = AA >> 7;\
				  	Y = CC >> 4;\
			  		TileData = VRAM1 + (VRAM[((Y & 0x3f80) << 1) + ((X >> 3) & 0xfe)] << 7); \
					b = *(TileData + (Y & 0x70) + (X & 14)); \
					*p++ = (FUNC); \
					AA += aa; CC += cc;\
					length--;\
				}\
			}\
	  } else { \
	  	if (Settings.Dezaemon && PPU.Mode7Repeat == 2) {\
			for (int x = startx; x != endx; x += dir, AA += aa, CC += cc, p++) \
			{ \
			    int X = (AA >> 8) & 0x7ff; \
			    int Y = (CC >> 8) & 0x7ff; \
	\
			    if (((X | Y) & ~0x3ff) == 0) \
			    { \
					uint8 *TileData = VRAM1 + (VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
					uint32 b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
				    *p = (FUNC); \
			    } \
			    else \
			    { \
					if (PPU.Mode7Repeat == 3) \
					{ \
					    X = (x + HOffset) & 7; \
					    Y = (yy + CentreY) & 7; \
					    uint32 b = *(VRAM1 + ((Y & 7) << 4) + ((X & 7) << 1)); \
						*p = (FUNC); \
					} \
			    } \
			} \
	    } else {\
			for (int x = startx; x != endx; x += dir, AA += aa, CC += cc, p++) \
			{ \
			    int X = (AA >> 8); \
			    int Y = (CC >> 8); \
	\
			    if (((X | Y) & ~0x3ff) == 0) \
			    { \
					uint8 *TileData = VRAM1 + (VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
					uint32 b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
				    *p = (FUNC); \
			    } \
			    else \
			    { \
					if (PPU.Mode7Repeat == 3) \
					{ \
					    X = (x + HOffset) & 7; \
					    Y = (yy + CentreY) & 7; \
					    uint32 b = *(VRAM1 + ((Y & 7) << 4) + ((X & 7) << 1)); \
						*p = (FUNC); \
					} \
			    } \
			} \
	    }\
		} \
	} \
}


#define RENDER_BACKGROUND_MODE7NEW(TYPE,FUNC,PITCH) \
    \
    uint8 *VRAM1 = VRAM + 1; \
    if (GFX.r2130 & 1) \
    { \
	if (IPPU.DirectColourMapsNeedRebuild) \
	    S9xBuildDirectColourMaps (); \
	GFX.ScreenColors = DirectColourMaps [0]; \
    } \
    else \
	GFX.ScreenColors = IPPU.ScreenColors; \
\
    int aa, cc; \
    int dir; \
    int startx, endx; \
    uint32 Left = 0; \
    uint32 Right = 256; \
    uint32 ClipCount = GFX.pCurrentClip->Count [bg]; \
\
    if (!ClipCount) \
	ClipCount = 1; \
\
    Screen += GFX.StartY*PITCH; \
/*    uint8 *Depth = GFX.DB + GFX.StartY * GFX.PPL;*/ \
    struct SLineMatrixData *l = &LineMatrixData [GFX.StartY]; \
\
    for (uint32 Line = GFX.StartY; Line <= GFX.EndY; Line++, Screen +=PITCH/*, Depth += GFX.PPL*/, l++) \
    { \
	int yy; \
\
	int32 HOffset = ((int32) LineData [Line].BG[0].HOffset << M7) >> M7; \
	int32 VOffset = ((int32) LineData [Line].BG[0].VOffset << M7) >> M7; \
\
	int32 CentreX = ((int32) l->CentreX << M7) >> M7; \
	int32 CentreY = ((int32) l->CentreY << M7) >> M7; \
\
	if (PPU.Mode7VFlip) \
	    yy = 261 - (int) Line; \
	else \
	    yy = Line; \
\
	if (PPU.Mode7Repeat == 0) \
	    yy += (VOffset - CentreY) % 1023; \
	else \
	    yy += VOffset - CentreY; \
	int BB = l->MatrixB * yy + (CentreX << 8); \
	int DD = l->MatrixD * yy + (CentreY << 8); \
\
	for (uint32 clip = 0; clip < ClipCount; clip++) \
	{ \
	    if (GFX.pCurrentClip->Count [bg]) \
	    { \
		Left = GFX.pCurrentClip->Left [clip][bg]; \
		Right = GFX.pCurrentClip->Right [clip][bg]; \
		if (Right <= Left) \
		    continue; \
	    } \
	    TYPE *p = (TYPE *) Screen + Left; \
	    if (PPU.Mode7HFlip) { \
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
	    if (PPU.Mode7Repeat == 0) \
		xx = startx + (HOffset - CentreX) % 1023; \
	    else \
		xx = startx + HOffset - CentreX; \
	    int AA = l->MatrixA * xx; \
	    int CC = l->MatrixC * xx; \
			AA += BB;/* Change Laxer*/\
			CC += DD;/* Change Laxer*/\
			int length = (endx-startx); \
			if (length<0) { length = -length; }\
\
	  if (!PPU.Mode7Repeat) { \
			  	register int X = (AA >> 8) & 0x3ff; \
			  	register int Y = (CC >> 8) & 0x3ff; \
			  	uint8 *TileData; \
			  	uint32 b;\
			  	int32 nAA = AA + (length*aa); \
			  	int32 nCC = CC + (length*cc); \
\
				if (((nAA>>18) | (nCC>>18) | (AA >> 18) | (CC >> 18))==0) {\
TestNoClipping:\
					if (cc == 0) {\
						register uint8* AdrY;\
						register int AdrY2;\
						Y = (CC>>8) & 0x3ff; AdrY = &VRAM[((Y & ~7) << 5)]; AdrY2 = ((Y & 7) << 4);\
						\
						while ((length>>3)!=0) {\
						  	X = (AA >> 7);\
					  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
							b = *(TileData + AdrY2 + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa;\
	\
						  	X = (AA >> 7);\
					  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
							b = *(TileData + AdrY2 + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa;\
	\
						  	X = (AA >> 7);\
					  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
							b = *(TileData + AdrY2 + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa;\
	\
						  	X = (AA >> 7);\
					  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
							b = *(TileData + AdrY2 + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa;\
	\
						  	X = (AA >> 7);\
					  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
							b = *(TileData + AdrY2 + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa;\
	\
						  	X = (AA >> 7);\
					  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
							b = *(TileData + AdrY2 + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa;\
	\
						  	X = (AA >> 7);\
					  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
							b = *(TileData + AdrY2 + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa;\
	\
						  	X = (AA >> 7);\
					  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
							b = *(TileData + AdrY2 + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa;\
	\
						  	length -= 8;\
						}\
	\
						while (length!=0) {\
						  	X = (AA >> 8);\
					  		TileData = VRAM1 + (AdrY[(X >> 2) & ~1] << 7); \
							b = *(TileData + AdrY2 + ((X & 7) << 1)); \
						  	{ *p++  = (FUNC); }\
	                        AA += aa;\
							length--;\
						}\
					}\
					else\
					{\
						while ((length>>3)!=0) {\
						  	X = (AA >> 7); Y = (CC >> 4); \
						  	TileData = VRAM1 + (VRAM[((Y & 0x3fe0) << 1) + ((X >> 3) & 0xfe)] << 7); \
							b = *(TileData + (Y & 0x70) + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa; CC += cc; \
	\
						  	X = (AA >> 7); Y = (CC >> 4); \
						  	TileData = VRAM1 + (VRAM[((Y & 0x3fe0) << 1) + ((X >> 3) & 0xfe)] << 7); \
							b = *(TileData + (Y & 0x70) + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa; CC += cc;  \
	\
						  	X = (AA >> 7); Y = (CC >> 4); \
						  	TileData = VRAM1 + (VRAM[((Y & 0x3fe0) << 1) + ((X >> 3) & 0xfe)] << 7); \
							b = *(TileData + (Y & 0x70) + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa; CC += cc;  \
	\
						  	X = (AA >> 7); Y = (CC >> 4); \
						  	TileData = VRAM1 + (VRAM[((Y & 0x3fe0) << 1) + ((X >> 3) & 0xfe)] << 7); \
							b = *(TileData + (Y & 0x70) + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa; CC += cc;  \
	\
						  	X = (AA >> 7); Y = (CC >> 4); \
						  	TileData = VRAM1 + (VRAM[((Y & 0x3fe0) << 1) + ((X >> 3) & 0xfe)] << 7); \
							b = *(TileData + (Y & 0x70) + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa; CC += cc;  \
	\
						  	X = (AA >> 7); Y = (CC >> 4); \
						  	TileData = VRAM1 + (VRAM[((Y & 0x3fe0) << 1) + ((X >> 3) & 0xfe)] << 7); \
							b = *(TileData + (Y & 0x70) + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa; CC += cc;  \
	\
						  	X = (AA >> 7); Y = (CC >> 4); \
						  	TileData = VRAM1 + (VRAM[((Y & 0x3fe0) << 1) + ((X >> 3) & 0xfe)] << 7); \
							b = *(TileData + (Y & 0x70) + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa; CC += cc;  \
	\
						  	X = (AA >> 7); Y = (CC >> 4); \
						  	TileData = VRAM1 + (VRAM[((Y & 0x3fe0) << 1) + ((X >> 3) & 0xfe)] << 7); \
							b = *(TileData + (Y & 0x70) + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa; CC += cc; \
	\
						  	length -= 8;\
						}\
	\
						while (length!=0) {\
						  	X = (AA >> 8); Y = (CC >> 8);\
						  	TileData = VRAM1 + (VRAM[((Y & ~1) << 5) + ((X >> 2) & ~1)] << 7);\
							b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1));\
						  	{ *p++  = (FUNC); }\
	                        AA += aa; CC += cc;\
							length--;\
						}\
					}\
				} else {/*Clipped*/\
					\
					register int X;\
					if (cc == 0) {\
						register uint8* AdrY;\
						register int AdrY2;\
					  	uint8 *TileData;\
					  	uint32 b;\
						Y = (CC>>8) & 0x3ff; AdrY = &VRAM[((Y & ~7) << 5)]; AdrY2 = ((Y & 7) << 4);\
						while ((length>>3)!=0) {\
						  	X = (AA >> 7);\
					  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
							b = *(TileData + AdrY2 + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa;\
	\
						  	X = (AA >> 7);\
					  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
							b = *(TileData + AdrY2 + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa;\
	\
						  	X = (AA >> 7);\
					  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
							b = *(TileData + AdrY2 + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa;\
	\
						  	X = (AA >> 7);\
					  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
							b = *(TileData + AdrY2 + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa;\
	\
						  	X = (AA >> 7);\
					  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
							b = *(TileData + AdrY2 + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa;\
	\
						  	X = (AA >> 7);\
					  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
							b = *(TileData + AdrY2 + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa;\
	\
						  	X = (AA >> 7);\
					  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
							b = *(TileData + AdrY2 + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa;\
	\
						  	X = (AA >> 7);\
					  		TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7); \
							b = *(TileData + AdrY2 + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa;\
	\
					        length -=8;\
						}\
						\
						\
						while (length!=0) {\
							X = (AA >> 7) & 0x7fe;\
							TileData = VRAM1 + (AdrY[(X >> 3) & 0xfe] << 7);\
							b = *(TileData + AdrY2 + (X & 0xe));\
							{ *p++ = (FUNC); }\
							AA += aa;\
							\
							length--;\
						}\
						\
					} else {/* Free Rotation */\
					  	uint8 *TileData; \
\
						while ((length>>3)!=0) {\
						  	X = (AA >> 7); Y = (CC >> 4); \
						  	TileData = VRAM1 + (VRAM[((Y & 0x3fe0) << 1) + ((X >> 3) & 0xfe)] << 7); \
							b = *(TileData + (Y & 0x70) + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa; CC += cc; \
	\
						  	X = (AA >> 7); Y = (CC >> 4); \
						  	TileData = VRAM1 + (VRAM[((Y & 0x3fe0) << 1) + ((X >> 3) & 0xfe)] << 7); \
							b = *(TileData + (Y & 0x70) + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa; CC += cc;  \
	\
						  	X = (AA >> 7); Y = (CC >> 4); \
						  	TileData = VRAM1 + (VRAM[((Y & 0x3fe0) << 1) + ((X >> 3) & 0xfe)] << 7); \
							b = *(TileData + (Y & 0x70) + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa; CC += cc;  \
	\
						  	X = (AA >> 7); Y = (CC >> 4); \
						  	TileData = VRAM1 + (VRAM[((Y & 0x3fe0) << 1) + ((X >> 3) & 0xfe)] << 7); \
							b = *(TileData + (Y & 0x70) + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa; CC += cc;  \
	\
						  	X = (AA >> 7); Y = (CC >> 4); \
						  	TileData = VRAM1 + (VRAM[((Y & 0x3fe0) << 1) + ((X >> 3) & 0xfe)] << 7); \
							b = *(TileData + (Y & 0x70) + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa; CC += cc;  \
	\
						  	X = (AA >> 7); Y = (CC >> 4); \
						  	TileData = VRAM1 + (VRAM[((Y & 0x3fe0) << 1) + ((X >> 3) & 0xfe)] << 7); \
							b = *(TileData + (Y & 0x70) + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa; CC += cc;  \
	\
						  	X = (AA >> 7); Y = (CC >> 4); \
						  	TileData = VRAM1 + (VRAM[((Y & 0x3fe0) << 1) + ((X >> 3) & 0xfe)] << 7); \
							b = *(TileData + (Y & 0x70) + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa; CC += cc;  \
	\
						  	X = (AA >> 7); Y = (CC >> 4); \
						  	TileData = VRAM1 + (VRAM[((Y & 0x3fe0) << 1) + ((X >> 3) & 0xfe)] << 7); \
							b = *(TileData + (Y & 0x70) + (X & 14)); \
						  	{ *p++  = (FUNC); } \
	                        AA += aa; CC += cc; \
	\
						  	length -= 8;\
						}\
	\
						while (length!=0) { \
						  	X = (AA >> 8) & 0x3ff; Y = (CC >> 8) & 0x3ff; \
						  	TileData = VRAM1 + (VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
							b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
						  	{ *p++ = (FUNC); } \
	                        AA += aa; CC += cc;\
							length--;\
						}\
					} /* End Free Rotation */\
				} /*  End clipped Normal mode */ \
	  } else { \
	  	if (Settings.Dezaemon && PPU.Mode7Repeat == 2) {\
			for (int x = startx; x != endx; x += dir, AA += aa, CC += cc, p++) \
			{ \
			    int X = (AA >> 8) & 0x7ff; \
			    int Y = (CC >> 8) & 0x7ff; \
	\
			    if (((X | Y) & ~0x3ff) == 0) \
			    { \
					uint8 *TileData = VRAM1 + (VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
					uint32 b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
				    *p = (FUNC); \
			    } \
			    else \
			    { \
					if (PPU.Mode7Repeat == 3) \
					{ \
					    X = (x + HOffset) & 7; \
					    Y = (yy + CentreY) & 7; \
					    uint32 b = *(VRAM1 + ((Y & 7) << 4) + ((X & 7) << 1)); \
						*p = (FUNC); \
					} \
			    } \
			} \
	    } else {\
			for (int x = startx; x != endx; x += dir, AA += aa, CC += cc, p++) \
			{ \
			    int X = (AA >> 8); \
			    int Y = (CC >> 8); \
	\
			    if (((X | Y) & ~0x3ff) == 0) \
			    { \
					uint8 *TileData = VRAM1 + (VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
					uint32 b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
				    *p = (FUNC); \
			    } \
			    else \
			    { \
					if (PPU.Mode7Repeat == 3) \
					{ \
					    X = (x + HOffset) & 7; \
					    Y = (yy + CentreY) & 7; \
					    uint32 b = *(VRAM1 + ((Y & 7) << 4) + ((X & 7) << 1)); \
						*p = (FUNC); \
					} \
			    } \
			} \
	    }\
	  } \
	} \
}


void DrawBGMode7Background16New (uint16 *Screen, int bg)
{
    // /*GFX.ScreenColors [b & GFX.Mode7Mask]*/
    RENDER_BACKGROUND_MODE7NEW_PHASE2 (uint16, (GFX.ScreenColors [b]),256)
}

void DrawBGMode7Background16PrioNew (uint16 *Screen, int bg, int prio)
{
	if ((GFX.Mode7PriorityMask >> 7)!=prio) { return; } else {
    	DrawBGMode7Background16New (Screen, bg);
 	}
}