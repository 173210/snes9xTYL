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
#ifndef _TILE_H_
#define _TILE_H_

/*void DrawHiResTile16New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount);
void DrawHiResClippedTile16New (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount);
void DrawTile16New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount);
void DrawClippedTile16New (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount);
void DrawTile16NewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr);
void DrawClippedTile16NewSprite (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount, uint32 index_spr);

void DrawTile16NewFastSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr);
void DrawClippedTile16NewFastSprite (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount, uint32 index_spr);
*/

/**************************************/

void softDrawHiResTile16New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount);
void softDrawHiResClippedTile16New (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount);
void softDrawTile16New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount);
void softDrawClippedTile16New (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount);
void softDrawTile16NewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr);
void softDrawClippedTile16NewSprite (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount, uint32 index_spr);
void softDrawTile16NewFastSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr);
void softDrawClippedTile16NewFastSprite (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount, uint32 index_spr);

void softDrawHiResTile16ADDNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount);
void softDrawHiResClippedTile16ADDNew (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount);
void softDrawTile16ADDNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount);
void softDrawClippedTile16ADDNew (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount);
void softDrawTile16ADDNewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr);
void softDrawClippedTile16ADDNewSprite (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount, uint32 index_spr);

void softDrawHiResTile16ADD1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount);
void softDrawHiResClippedTile16ADD1_2New (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount);
void softDrawTile16ADD1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount);
void softDrawClippedTile16ADD1_2New (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount);
void softDrawTile16ADD1_2NewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr);
void softDrawClippedTile16ADD1_2NewSprite (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount, uint32 index_spr);


void softDrawHiResTile16SUBNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount);
void softDrawHiResClippedTile16SUBNew (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount);
void softDrawTile16SUBNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount);
void softDrawClippedTile16SUBNew (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount);
void softDrawTile16SUBNewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr);
void softDrawClippedTile16SUBNewSprite (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount, uint32 index_spr);

void softDrawHiResTile16SUB1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount);
void softDrawHiResClippedTile16SUB1_2New (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount);
void softDrawTile16SUB1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount);
void softDrawClippedTile16SUB1_2New (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount);
void softDrawTile16SUB1_2NewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr);
void softDrawClippedTile16SUB1_2NewSprite (uint32 Tile, uint32 Offset,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount, uint32 index_spr);

			
//extern int8 *SubScreenTranspBuffer;

#endif
