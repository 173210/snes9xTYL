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
#ifndef _TILE_PSP_H_
#define _TILE_PSP_H_

#define NO_CPU_CACHE(a) (((u32)a)|0x40000000)

#define TILE_PREAMBLE 

#define TILE_CACHE_RESET_ASK_MAX 0 //0 for never
#define TILE_CACHE_SIZE 4096
//#define TILE_CACHE_TEXTURES_SIZE (TILE_CACHE_SIZE/(64*64))

//exactly 12 bit, as max for TileNumber = Address(16bits) >> Shift(4,5 or 6)
//so it's faster to invalidate an entry in cache
//independantly of current colour palette & BG.BitShift
#define TILE_HASH_SIZE 4096   
										
#define TILE_HASH_MASK (TILE_HASH_SIZE-1)
#define TILE_CACHE_MAX_DELAY 1

/***********************************/
typedef struct tile_cache_s{
	u32 key;s32 used;
	struct tile_cache_s *next;
	u16 text_idx;
	u16 tile_type;
}tile_cache_t;

struct Vertex
{
	unsigned short u, v;	
	short x, y, z;
};

struct VertexCol
{	
	//unsigned short color;
	unsigned short u, v;	
	short x, y, z;
};

/***********************************/
extern u8 *tile_texture[3]; //3 textures, for 3 bits mode (2,4,8 / pixels)
extern u32 tile_cache_reset,tile_cached[3],tile_resetask;
extern tile_cache_t *tile_cache_first_free[3];
/***********************************/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" void tile_init_cache();
extern "C" void tile_reset_cache();
#else
void tile_init_cache();
void tile_reset_cache();
#endif

void tile_askforreset(s32 addr);
void tile_clean_cache(int cache_num,int delay);
/***********************************/

#undef GFX_TILE_ME
#ifdef GFX_TILE_ME
typedef struct 
{
	int TileAddress;
	int TileShift;
	int NameSelect;
	int DirectColourMode;
	int PaletteMask;
	int PaletteShift;
	int StartPalette;

	struct Vertex *vertices_ptr[3];

	int count;
	int current_bitshift;
	bool8 owner_order_end;
	bool8 me_job_end;
	char sp[2];
	struct {
			uint32 Tile; s32 x;s32 y;
		        uint32 StartPixel; uint32 Width;
		        uint32 StartLine; uint32 LineCount;int realZ2;
				int type;
	} ArgMent[1000];
}SME_GFX;
extern SME_GFX* ME_GFX;
  //GPUPack.BG.TileAddress  GPUPack.BG.TileShift;
  //GPUPack.BG.NameSelect; 
  //GPUPack.BG.TileShift;
  //GPUPack.BG.DirectColourMode;     
  //GPUPack.BG.PaletteMask  GPUPack.BG.PaletteShift GPUPack.BG.StartPalette;
#endif

#endif

