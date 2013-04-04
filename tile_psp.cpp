//#define DEBUG_TILE

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
#include "display.h"
#include "gfx.h"
#include "tile_psp.h"

//extern s32 realZ1,realZ2;
extern int render_timestamp;

//extern int hires_offset,hires_offsetV;
//extern int hires_offset2,hires_offsetV2;

tile_cache_t tile_cache_data[3][TILE_CACHE_SIZE];
tile_cache_t *tile_cache_ptr[3][TILE_HASH_SIZE];
//u32 tile_cached_line[3][TILE_HASH_SIZE];
tile_cache_t *tile_cache_first_free[3];
u32 tile_cached[3],tile_resetask;
u32 tile_cache_reset;
u8 *tile_texture[3];  
//multiple textures, one for each tile colour mode => 2bits/pix, 4bits, 8bits
//each one is 512x512 => 64x64 tiles = 4096 tiles/texture


extern struct Vertex *vertices[3];
extern struct Vertex *vertices_ptr[3];
extern struct Vertex *vertices_end[3];
extern struct VertexCol *_verticesCol;
extern struct VertexCol *_verticesCol_ptr;
extern struct VertexCol *_verticesCol_end;
extern int current_bitshift;



// ask for a reset at next rendering pass


void tile_askforreset(s32 addr){	
	if (tile_cache_reset) return;
	if (addr==-1)	{tile_cache_reset=1;	return;}
	tile_resetask++;
	if (tile_resetask==TILE_CACHE_RESET_ASK_MAX) {tile_cache_reset=1;	return;}
	
	
	//invalidate entries
	u32 key=(addr>>4)&TILE_HASH_MASK;
	tile_cache_t *q,*plast,*p=tile_cache_ptr[TILE_2BIT][key];
	if (p){
		
		//tile_cached[TILE_2BIT]-=tile_cached_line[TILE_2BIT][key];
		plast=q=p;		
		while (q) {
			tile_cached[TILE_2BIT]--;
			plast=q;
			q=q->next;
		}
		
		//tile_cached_line[TILE_2BIT][key]=0;
		plast->next=tile_cache_first_free[TILE_2BIT];		
		tile_cache_first_free[TILE_2BIT]=p;
		tile_cache_ptr[TILE_2BIT][key]=NULL;
	}
	key=(addr>>5)&TILE_HASH_MASK;
	p=tile_cache_ptr[TILE_4BIT][key];
	if (p){
		
		//tile_cached[TILE_4BIT]-=tile_cached_line[TILE_4BIT][key];
		plast=q=p;
		while (q) {
			tile_cached[TILE_4BIT]--;
			plast=q;
			q=q->next;
		}
		
		//tile_cached_line[TILE_4BIT][key]=0;
		plast->next=tile_cache_first_free[TILE_4BIT];
		tile_cache_first_free[TILE_4BIT]=p;
		tile_cache_ptr[TILE_4BIT][key]=NULL;
	}
	key=(addr>>6)&TILE_HASH_MASK;
	p=tile_cache_ptr[TILE_8BIT][key];
	if (p){
		//tile_cached[TILE_8BIT]-=tile_cached_line[TILE_8BIT][key];
		plast=q=p;
		while (q) {
			tile_cached[TILE_8BIT]--;
			plast=q;
			q=q->next;
		}
		
		//tile_cached_line[TILE_8BIT][key]=0;
		plast->next=tile_cache_first_free[TILE_8BIT];
		tile_cache_first_free[TILE_8BIT]=p;
		tile_cache_ptr[TILE_8BIT][key]=NULL;
	}	
}
// get tile with hashkey=key
// return texture index if found, 0 if not
INLINE tile_cache_t *tile_get_cache(int cache_num,int key){
	 tile_cache_t *p=tile_cache_ptr[cache_num][key&TILE_HASH_MASK];
	 for (;;){	 	
	 	if (!p) return NULL;
	 	if (p->key==key) {
	 		p->used=render_timestamp;
	 		return p;
	 	}
	 	p=p->next;
	 }	 
}
// insert a new tile in cache
INLINE tile_cache_t *tile_insert_cache(int cache_num,u32 key){	
	tile_cache_t *p=tile_cache_ptr[cache_num][key&TILE_HASH_MASK];
	tile_cache_t *q;
	//get first free cache entry
	q=tile_cache_first_free[cache_num];		
	if (!q) {//error cache full		
		//debug_log("**tile cache full!");
		return NULL; 
	}
	tile_cached[cache_num]++;
	//tile_cached_line[cache_num][key]++;
	//update first free
	tile_cache_first_free[cache_num]=tile_cache_first_free[cache_num]->next;	
	//update new cached tile data
	q->next=NULL;
	q->key=key;
	q->used=render_timestamp;				
	if (!p) {//insert first	
		tile_cache_ptr[cache_num][key&TILE_HASH_MASK]=q;		
	}else{//insert at end
		for (;;){
			if (p->next) p=p->next;
			else {p->next=q;break;}
		}
	}
	return q;	
}
// clean used cached tile & recount total
void tile_clean_cache(int cache_num,int delay){
	int i;
	tile_cache_t *p,*prev_p;
	int nosupp,tosupp;
		
/*char str[64];		
sprintf(str,"ts : %d",render_timestamp);
pgPrintAllBG(34,32,31<<5,str);*/
	//debug_log("cache clean");
	
	nosupp=0;
	tosupp=0;
	for (i=0;i<TILE_HASH_SIZE;i++){
		prev_p=NULL;		
		p=tile_cache_ptr[cache_num][i];
		while (p) {			
			if ((render_timestamp-p->used)>delay) {
				//delete entry				
				tosupp++;
				tile_cached[cache_num]--;
//				tile_cached_line[cache_num][i]--;
				if (!prev_p) {//free tile in first pos				
					tile_cache_ptr[cache_num][i]=p->next;				
					p->next=tile_cache_first_free[cache_num];
					tile_cache_first_free[cache_num]=p;
					p=tile_cache_ptr[cache_num][i];					
				}else{//free tile at n-th pos
					prev_p->next=p->next;
					p->next=tile_cache_first_free[cache_num];
					tile_cache_first_free[cache_num]=p;
					p=prev_p->next;					
				}
			} else {
				prev_p=p;
				p=p->next;
				nosupp++;
			}
		}
	}
/*sprintf(str,"ts : %d, %d %d %d",render_timestamp,nosupp,tosupp,tile_cached[cache_num]);
pgPrintAllBG(34,31,31<<5,str);	
	{  char str[64];			
		sprintf(str,"aft:%d",tile_cached[cache_num]);
		debug_log(str);	
	}*/
}

// reset tile cache
extern "C" {
void tile_reset_cache(){
	int i,num;
	for (num=0;num<3;num++) {
		for (i=0;i<TILE_CACHE_SIZE-1;i++){				
			tile_cache_data[num][i].next=&tile_cache_data[num][i+1];
		}		
		tile_cache_data[num][i].next=NULL;
		tile_cache_first_free[num]=&tile_cache_data[num][0];
				
		tile_cached[num]=0;
		
		for (i=0;i<TILE_HASH_SIZE;i++) tile_cache_ptr[num][i]=NULL;		
	}
	//memset(tile_cache_ptr,0,3*4*TILE_HASH_SIZE);
	//memset(tile_cached_line,0,3*4*TILE_HASH_SIZE);
	
	tile_resetask=0;
	tile_cache_reset=0;
	debug_log("resetcache");
}

void tile_init_cache(){
	//init cache	
	int i,num;
	for (num=0;num<3;num++) {
		for (i=0;i<TILE_CACHE_SIZE-1;i++){		
			tile_cache_data[num][i].text_idx=i;
			tile_cache_data[num][i].next=&tile_cache_data[num][i+1];
		}	
		tile_cache_data[num][i].text_idx=i;
		tile_cache_data[num][i].next=NULL;
		tile_cache_first_free[num]=&tile_cache_data[num][0];
		
		tile_cached[num]=0;
		for (i=0;i<TILE_HASH_SIZE;i++) tile_cache_ptr[num][i]=NULL;		
	}
	//memset(tile_cache_ptr,0,3*4*TILE_HASH_SIZE);
	//memset(tile_cached_line,0,3*4*TILE_HASH_SIZE);
	tile_resetask=0;
	tile_cache_reset=0;
}
}

typedef union { struct { uint8 b0, b1, b2, b3; } B; uint32 W;} yo_uint32;
uint8 pspConvertTile (uint8 *pCache, uint32 TileAddr,uint32 col)
{
    register uint8 *tp = &VRAM[TileAddr];
    uint32 *p = (uint32 *) pCache;
    uint32 non_zero = 0;
    uint8 line;
    yo_uint32 colTmp;
    
    col=(col<<24)|(col<<16)|(col<<8)|col;

    switch (GPUPack.BG.BitShift)
    {
    case 8:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    uint32 p1 = 0;
	    uint32 p2 = 0;
	    register uint8 pix;

	    if ((pix = *(tp + 0)))
	    {
		p1 |= odd_high[0][pix >> 4];
		p2 |= odd_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 1)))
	    {
		p1 |= even_high[0][pix >> 4];
		p2 |= even_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 16)))
	    {
		p1 |= odd_high[1][pix >> 4];
		p2 |= odd_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 17)))
	    {
		p1 |= even_high[1][pix >> 4];
		p2 |= even_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 32)))
	    {
		p1 |= odd_high[2][pix >> 4];
		p2 |= odd_low[2][pix & 0xf];
	    }
	    if ((pix = *(tp + 33)))
	    {
		p1 |= even_high[2][pix >> 4];
		p2 |= even_low[2][pix & 0xf];
	    }
	    if ((pix = *(tp + 48)))
	    {
		p1 |= odd_high[3][pix >> 4];
		p2 |= odd_low[3][pix & 0xf];
	    }
	    if ((pix = *(tp + 49)))
	    {
		p1 |= even_high[3][pix >> 4];
		p2 |= even_low[3][pix & 0xf];
	    }
	    p[0] = p1;//|col;
	    p[1] = p2;//|col;
	    p+=512/4;
	    non_zero |= p1 | p2;
	}
	break;

    case 4:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    yo_uint32 p1;p1.W = 0;
	    yo_uint32 p2;p2.W = 0;
	    register uint8 pix;
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
	    non_zero |= p1.W | p2.W;
	    if (non_zero) {
		colTmp.W = col;
		if (!p1.B.b0) colTmp.B.b0 = 0;
		if (!p1.B.b1) colTmp.B.b1 = 0;
		if (!p1.B.b2) colTmp.B.b2 = 0;
		if (!p1.B.b3) colTmp.B.b3 = 0;
		p[0] = p1.W | colTmp.W;
		colTmp.W = col;
		if (!p2.B.b0) colTmp.B.b0 = 0;
		if (!p2.B.b1) colTmp.B.b1 = 0;
		if (!p2.B.b2) colTmp.B.b2 = 0;
		if (!p2.B.b3) colTmp.B.b3 = 0;
	    	p[1] = p2.W|colTmp.W;
	    } else {
	    	p[0] = p[1] = 0;
	    }
	    p+=512/4;
	    
	}
	break;

    case 2:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    yo_uint32 p1;p1.W = 0;
	    yo_uint32 p2;p2.W = 0;
	    register uint8 pix;
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
	    non_zero |= p1.W | p2.W;
	    if (non_zero) {
		colTmp.W = col;
		if (!p1.B.b0) colTmp.B.b0 = 0;
		if (!p1.B.b1) colTmp.B.b1 = 0;
		if (!p1.B.b2) colTmp.B.b2 = 0;
		if (!p1.B.b3) colTmp.B.b3 = 0;
		p[0] = p1.W | colTmp.W;
		colTmp.W = col;
		if (!p2.B.b0) colTmp.B.b0 = 0;
		if (!p2.B.b1) colTmp.B.b1 = 0;
		if (!p2.B.b2) colTmp.B.b2 = 0;
		if (!p2.B.b3) colTmp.B.b3 = 0;
		p[1] = p2.W|colTmp.W;
	    } else {
	    	p[0] = p[1] = 0;
	    }
	    p+=512/4;
	    non_zero |= p1.W | p2.W;
	}
	break;
    }
    return (non_zero ? TRUE : BLANK_TILE);
}


uint8 pspConvertTileHires (uint8 *pCache, uint32 TileAddr,uint32 col)
{
    register uint8 *tp = &VRAM[TileAddr];
    uint32 *p = (uint32 *) pCache;
    uint32 non_zero = 0;
    uint8 line;
    yo_uint32 colTmp;
    
    col=(col<<24)|(col<<16)|(col<<8)|col;

    switch (GPUPack.BG.BitShift)
    {
    case 8:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    uint32 p1 = 0;
	    uint32 p2 = 0;
	    register uint8 pix;

	    if ((pix = *(tp + 0)))
	    {
		p1 |= odd_high[0][pix >> 4];
		p2 |= odd_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 1)))
	    {
		p1 |= even_high[0][pix >> 4];
		p2 |= even_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 16)))
	    {
		p1 |= odd_high[1][pix >> 4];
		p2 |= odd_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 17)))
	    {
		p1 |= even_high[1][pix >> 4];
		p2 |= even_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 32)))
	    {
		p1 |= odd_high[2][pix >> 4];
		p2 |= odd_low[2][pix & 0xf];
	    }
	    if ((pix = *(tp + 33)))
	    {
		p1 |= even_high[2][pix >> 4];
		p2 |= even_low[2][pix & 0xf];
	    }
	    if ((pix = *(tp + 48)))
	    {
		p1 |= odd_high[3][pix >> 4];
		p2 |= odd_low[3][pix & 0xf];
	    }
	    if ((pix = *(tp + 49)))
	    {
		p1 |= even_high[3][pix >> 4];
		p2 |= even_low[3][pix & 0xf];
	    }
	    p[0] = p1;//|col;
	    p[1] = p2;//|col;
	    p+=512/4;
	    non_zero |= p1 | p2;
	}
	break;

    case 4:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    yo_uint32 p1;p1.W = 0;
	    yo_uint32 p2;p2.W = 0;
	    register uint8 pix;
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
	    non_zero |= p1.W | p2.W;
	    if (non_zero) {
			//hirez FIX + Ruka changes
			if(p1.B.b2)p1.W=p1.W&0x00FFFFFF | p1.B.b2<<8;
			if(p1.B.b0)p1.W=p1.W&0xFFFF00FF | p1.B.b0<<8;
			if(p2.B.b2)p2.W=p2.W&0x00FFFFFF | p2.B.b2<<8;
			if(p2.B.b3)p2.W=p2.W&0xFFFF00FF | p2.B.b0<<8;
			//orign + Ruka changes
		colTmp.W = col;
		if (!p1.B.b0) colTmp.B.b0 = 0;
		if (!p1.B.b1) colTmp.B.b1 = 0;
		if (!p1.B.b2) colTmp.B.b2 = 0;
		if (!p1.B.b3) colTmp.B.b3 = 0;
		p[0] = p1.W | colTmp.W;
		colTmp.W = col;
		if (!p2.B.b0) colTmp.B.b0 = 0;
		if (!p2.B.b1) colTmp.B.b1 = 0;
		if (!p2.B.b2) colTmp.B.b2 = 0;
		if (!p2.B.b3) colTmp.B.b3 = 0;
	    	p[1] = p2.W|colTmp.W;
	    } else {
	    	p[0] = p[1] = 0;
	    }
	    p+=512/4;
	    
	}
	break;

    case 2:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    yo_uint32 p1;p1.W = 0;
	    yo_uint32 p2;p2.W = 0;
	    register uint8 pix;
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
	    non_zero |= p1.W | p2.W;
	    if (non_zero) {
			//hirez FIX + Ruka changes
			if(p1.B.b2)p1.W=p1.W&0x00FFFFFF | p1.B.b2<<8;
			if(p1.B.b0)p1.W=p1.W&0xFFFF00FF | p1.B.b0<<8;
			if(p2.B.b2)p2.W=p2.W&0x00FFFFFF | p2.B.b2<<8;
			if(p2.B.b3)p2.W=p2.W&0xFFFF00FF | p2.B.b0<<8;
			//orign + Ruka changes
		colTmp.W = col;
		if (!p1.B.b0) colTmp.B.b0 = 0;
		if (!p1.B.b1) colTmp.B.b1 = 0;
		if (!p1.B.b2) colTmp.B.b2 = 0;
		if (!p1.B.b3) colTmp.B.b3 = 0;
		p[0] = p1.W | colTmp.W;
		colTmp.W = col;
		if (!p2.B.b0) colTmp.B.b0 = 0;
		if (!p2.B.b1) colTmp.B.b1 = 0;
		if (!p2.B.b2) colTmp.B.b2 = 0;
		if (!p2.B.b3) colTmp.B.b3 = 0;
		p[1] = p2.W|colTmp.W;
	    } else {
	    	p[0] = p[1] = 0;
	    }
	    p+=512/4;
	    non_zero |= p1.W | p2.W;
	}
	break;
    }
    return (non_zero ? TRUE : BLANK_TILE);
}



void pspDrawTile16 (uint32 Tile, s32 x,s32 y, uint32 StartLine,uint32 LineCount,short realZ2) {		
	uint32 u,v,col,TileKey,TileNumber,TileAddr;		
	tile_cache_t *cache_ptr;
	uint8 *pCache;
  
	TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
	if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect; 
	TileAddr &= 0xffff; 
	TileNumber = (TileAddr >> GPUPack.BG.TileShift);
    
	if (GPUPack.BG.DirectColourMode) col=0;     
	else col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    
	TileKey=TileNumber|(col<<14)|(current_bitshift<<12); //TileNumber is TileAddr(16) >> TileShift(min 4) => max 12bits

	cache_ptr=tile_get_cache(current_bitshift,TileKey);
	if (!cache_ptr) {//tile not cached
  		if (tile_cached[current_bitshift]==(TILE_CACHE_SIZE-1)) {  	
  			tile_clean_cache(current_bitshift,0);
  			if (tile_cached[current_bitshift]==(TILE_CACHE_SIZE-1)) {debug_log("cache full!!!");return;}  			
  		}

  		cache_ptr=tile_insert_cache(current_bitshift,TileKey);
		  	  	
  		u=(cache_ptr->text_idx&63)<<3;
  		v=(cache_ptr->text_idx>>6)<<3;
   		pCache = tile_texture[current_bitshift]+(v<<9)+u;
   		if ((cache_ptr->tile_type=pspConvertTile ((u8*)NO_CPU_CACHE(pCache), TileAddr,col)==BLANK_TILE)) return;
   	
  }else{
    if (cache_ptr->tile_type==BLANK_TILE) return;
    u=(cache_ptr->text_idx&63)<<3;
		v=(cache_ptr->text_idx>>6)<<3;
  }
  
  if (vertices_ptr[current_bitshift]==vertices_end[current_bitshift]){
  	debug_log("!!ran out of vertices !!");
  	return;
  }

	
	vertices_ptr[current_bitshift][0].x = x; vertices_ptr[current_bitshift][0].y = y/*-StartLine*/; vertices_ptr[current_bitshift][0].z = realZ2;		
	vertices_ptr[current_bitshift][1].x = x+8; vertices_ptr[current_bitshift][1].y = y+LineCount/*-StartLine+8*/; vertices_ptr[current_bitshift][1].z = realZ2;				
	if (!(Tile & (V_FLIP | H_FLIP))) {//NO FLIP
		vertices_ptr[current_bitshift][0].u = u; vertices_ptr[current_bitshift][0].v = v+StartLine;		
		vertices_ptr[current_bitshift][1].u = u+8; vertices_ptr[current_bitshift][1].v = v+StartLine+LineCount;		
	}else if (!(Tile & V_FLIP)) {//HFLIP
		vertices_ptr[current_bitshift][1].u = u; vertices_ptr[current_bitshift][0].v = v+StartLine;		
		vertices_ptr[current_bitshift][0].u = u+8; vertices_ptr[current_bitshift][1].v = v+StartLine+LineCount;			
	}else if (Tile & H_FLIP) {//HFLIP & VFLIP						
		vertices_ptr[current_bitshift][1].u = u; vertices_ptr[current_bitshift][1].v = v+8-StartLine-LineCount;
		vertices_ptr[current_bitshift][0].u = u+8; vertices_ptr[current_bitshift][0].v = v+8-StartLine;
	}else {//VFLIP
		vertices_ptr[current_bitshift][0].u = u; vertices_ptr[current_bitshift][1].v = v+8-StartLine-LineCount;
		vertices_ptr[current_bitshift][1].u = u+8; vertices_ptr[current_bitshift][0].v = v+8-StartLine;
	}
	vertices_ptr[current_bitshift]+=2;				

}

void pspDrawClippedTile16 (uint32 Tile, s32 x,s32 y,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount,short realZ2) {		
#ifdef DEBUG_TILE
return;	
#endif
	uint32 u,v,col,TileKey,TileNumber,TileAddr;		
	tile_cache_t *cache_ptr;
  uint8 *pCache;
  
  TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
  if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect; 
  TileAddr &= 0xffff; 
  TileNumber = (TileAddr >> GPUPack.BG.TileShift);
    
  if (GPUPack.BG.DirectColourMode) col=0;     
  else col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    
  TileKey=TileNumber|(col<<14)|(current_bitshift<<12); //TileNumber is TileAddr(16) >> TileShift(min 4) => max 12bits
  cache_ptr=tile_get_cache(current_bitshift,TileKey);
  if (!cache_ptr) {//tile not cached
  	if (tile_cached[current_bitshift]==(TILE_CACHE_SIZE-1)) {
  		tile_clean_cache(current_bitshift,0);  			  		
  		if (tile_cached[current_bitshift]==(TILE_CACHE_SIZE-1)) {debug_log("cache full!!!");return;}  			
  	}
  	cache_ptr=tile_insert_cache(current_bitshift,TileKey);  	
  	
  	u=(cache_ptr->text_idx&63)<<3;
  	v=(cache_ptr->text_idx>>6)<<3;  	
   	pCache = tile_texture[current_bitshift]+(v<<9)+u;
   	if ((cache_ptr->tile_type=pspConvertTile ((u8*)NO_CPU_CACHE(pCache), TileAddr,col)==BLANK_TILE)) return;   	
  }else{
    if (cache_ptr->tile_type==BLANK_TILE) return;
    u=(cache_ptr->text_idx&63)<<3;
		v=(cache_ptr->text_idx>>6)<<3;  	
  }
  
  if (vertices_ptr[current_bitshift]==vertices_end[current_bitshift]){
  	debug_log("!!ran out of vertices !!");
  	return;
  }
        		
	vertices_ptr[current_bitshift][0].x = x+StartPixel; vertices_ptr[current_bitshift][0].y = y; vertices_ptr[current_bitshift][0].z = realZ2;		
	vertices_ptr[current_bitshift][1].x = x+StartPixel+Width; vertices_ptr[current_bitshift][1].y = y+LineCount; vertices_ptr[current_bitshift][1].z = realZ2;		
				
	if (!(Tile & (V_FLIP | H_FLIP))) {//NO FLIP
		vertices_ptr[current_bitshift][0].u = u+StartPixel; vertices_ptr[current_bitshift][0].v = v+StartLine;
		vertices_ptr[current_bitshift][1].u = u+StartPixel+Width; vertices_ptr[current_bitshift][1].v = v+StartLine+LineCount;
	}else if (!(Tile & V_FLIP)) {//HFLIP
		vertices_ptr[current_bitshift][1].u = u+8-StartPixel-Width; vertices_ptr[current_bitshift][0].v = v+StartLine;
		vertices_ptr[current_bitshift][0].u = u+8-StartPixel; vertices_ptr[current_bitshift][1].v = v+StartLine+LineCount;
	}
	else if (Tile & H_FLIP) {//HFLIP & VFLIP
		vertices_ptr[current_bitshift][1].u = u+8-StartPixel-Width; vertices_ptr[current_bitshift][1].v = v+8-StartLine-LineCount;
		vertices_ptr[current_bitshift][0].u = u+8-StartPixel; vertices_ptr[current_bitshift][0].v = v+8-StartLine;
	}else {//VFLIP
		vertices_ptr[current_bitshift][0].u = u+StartPixel; vertices_ptr[current_bitshift][1].v = v+8-StartLine-LineCount;
		vertices_ptr[current_bitshift][1].u = u+StartPixel+Width; vertices_ptr[current_bitshift][0].v = v+8-StartLine;
	}				
	vertices_ptr[current_bitshift]+=2;
}


void pspDrawHiResTile16 (uint32 Tile, s32 x,s32 y, uint32 StartLine,uint32 LineCount,short realZ2) {
#ifdef DEBUG_TILE
return;	
#endif
	uint32 u,v,col,TileKey,TileNumber,TileAddr;		
	tile_cache_t *cache_ptr;
  uint8 *pCache;
  
  TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
  if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect; 
  TileAddr &= 0xffff; 
  TileNumber = (TileAddr >> GPUPack.BG.TileShift);
    
  if (GPUPack.BG.DirectColourMode) col=0;     
  else col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    
  TileKey=TileNumber|(col<<14)|(current_bitshift<<12); //TileNumber is TileAddr(16) >> TileShift(min 4) => max 12bits

  cache_ptr=tile_get_cache(current_bitshift,TileKey);
  if (!cache_ptr) {//tile not cached
  	if (tile_cached[current_bitshift]==(TILE_CACHE_SIZE-1)) {  	
  		tile_clean_cache(current_bitshift,0);
  		if (tile_cached[current_bitshift]==(TILE_CACHE_SIZE-1)) {debug_log("cache full!!!");return;}  			
  	}

  	cache_ptr=tile_insert_cache(current_bitshift,TileKey);  	
  	  	
  	u=(cache_ptr->text_idx&63)<<3;
  	v=(cache_ptr->text_idx>>6)<<3;
   	pCache = tile_texture[current_bitshift]+(v<<9)+u;
   	if ((cache_ptr->tile_type=pspConvertTileHires ((u8*)NO_CPU_CACHE(pCache), TileAddr,col)==BLANK_TILE)) return;
   	
  }else{
    if (cache_ptr->tile_type==BLANK_TILE) return;
    u=(cache_ptr->text_idx&63)<<3;
		v=(cache_ptr->text_idx>>6)<<3;
  }
  
  if (vertices_ptr[current_bitshift]==vertices_end[current_bitshift]){
  	debug_log("!!ran out of vertices !!");
  	return;
  }

	vertices_ptr[current_bitshift][0].x = x; vertices_ptr[current_bitshift][0].y = y; vertices_ptr[current_bitshift][0].z = realZ2;		
	vertices_ptr[current_bitshift][1].x = x+4; vertices_ptr[current_bitshift][1].y = y+LineCount; vertices_ptr[current_bitshift][1].z = realZ2;				
	if (!(Tile & (V_FLIP | H_FLIP))) {//NO FLIP
		vertices_ptr[current_bitshift][0].u = u; vertices_ptr[current_bitshift][0].v = v+StartLine;
		vertices_ptr[current_bitshift][1].u = u+8; vertices_ptr[current_bitshift][1].v = v+StartLine+LineCount;
	}else if (!(Tile & V_FLIP)) {//HFLIP
		vertices_ptr[current_bitshift][1].u = u; vertices_ptr[current_bitshift][0].v = v+StartLine;
		vertices_ptr[current_bitshift][0].u = u+8; vertices_ptr[current_bitshift][1].v = v+StartLine+LineCount;
	}else if (Tile & H_FLIP) {//HFLIP & VFLIP						
		vertices_ptr[current_bitshift][1].u = u; vertices_ptr[current_bitshift][1].v = v+8-StartLine-LineCount;
		vertices_ptr[current_bitshift][0].u = u+8; vertices_ptr[current_bitshift][0].v = v+8-StartLine;
	}else {//VFLIP
		vertices_ptr[current_bitshift][0].u = u; vertices_ptr[current_bitshift][1].v = v+8-StartLine-LineCount;
		vertices_ptr[current_bitshift][1].u = u+8; vertices_ptr[current_bitshift][0].v = v+8-StartLine;
	}
	vertices_ptr[current_bitshift]+=2;				

}

void pspDrawHiResClippedTile16 (uint32 Tile, s32 x,s32 y,uint32 StartPixel, uint32 Width,uint32 StartLine, uint32 LineCount,short realZ2) {
#ifdef DEBUG_TILE
return;	
#endif
	uint32 u,v,col,TileKey,TileNumber,TileAddr;		
	tile_cache_t *cache_ptr;
  uint8 *pCache;
  
  TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
  if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect; 
  TileAddr &= 0xffff; 
  TileNumber = (TileAddr >> GPUPack.BG.TileShift);
    
  if (GPUPack.BG.DirectColourMode) col=0;     
  else col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    
  TileKey=TileNumber|(col<<14)|(current_bitshift<<12); //TileNumber is TileAddr(16) >> TileShift(min 4) => max 12bits
  cache_ptr=tile_get_cache(current_bitshift,TileKey);
  if (!cache_ptr) {//tile not cached
  	if (tile_cached[current_bitshift]==(TILE_CACHE_SIZE-1)) {
  		tile_clean_cache(current_bitshift,0);  			  		
  		if (tile_cached[current_bitshift]==(TILE_CACHE_SIZE-1)) {debug_log("cache full!!!");return;}  			
  	}
  	cache_ptr=tile_insert_cache(current_bitshift,TileKey);  	
  	
  	u=(cache_ptr->text_idx&63)<<3;
  	v=(cache_ptr->text_idx>>6)<<3;  	
   	pCache = tile_texture[current_bitshift]+(v<<9)+u;
   	if ((cache_ptr->tile_type=pspConvertTileHires ((u8*)NO_CPU_CACHE(pCache), TileAddr,col)==BLANK_TILE)) return;   	
  }else{
    if (cache_ptr->tile_type==BLANK_TILE) return;
    u=(cache_ptr->text_idx&63)<<3;
		v=(cache_ptr->text_idx>>6)<<3;  	
  }
  
  if (vertices_ptr[current_bitshift]==vertices_end[current_bitshift]){
  	debug_log("!!ran out of vertices !!");
  	return;
  }
        		
	vertices_ptr[current_bitshift][0].x = x+(StartPixel>>1); vertices_ptr[current_bitshift][0].y = y; vertices_ptr[current_bitshift][0].z = realZ2;		
	vertices_ptr[current_bitshift][1].x = x+((StartPixel+Width)>>1); vertices_ptr[current_bitshift][1].y = y+LineCount; vertices_ptr[current_bitshift][1].z = realZ2;
				
	if (!(Tile & (V_FLIP | H_FLIP))) {//NO FLIP
		vertices_ptr[current_bitshift][0].u = u+StartPixel; vertices_ptr[current_bitshift][0].v = v+StartLine;
		vertices_ptr[current_bitshift][1].u = u+StartPixel+Width; vertices_ptr[current_bitshift][1].v = v+StartLine+LineCount;
	}else if (!(Tile & V_FLIP)) {//HFLIP
		vertices_ptr[current_bitshift][1].u = u+8-StartPixel-Width; vertices_ptr[current_bitshift][0].v = v+StartLine;
		vertices_ptr[current_bitshift][0].u = u+8-StartPixel; vertices_ptr[current_bitshift][1].v = v+StartLine+LineCount;
	}
	else if (Tile & H_FLIP) {//HFLIP & VFLIP
		vertices_ptr[current_bitshift][1].u = u+8-StartPixel-Width; vertices_ptr[current_bitshift][1].v = v+8-StartLine-LineCount;
		vertices_ptr[current_bitshift][0].u = u+8-StartPixel; vertices_ptr[current_bitshift][0].v = v+8-StartLine;
	}else {//VFLIP
		vertices_ptr[current_bitshift][0].u = u+StartPixel; vertices_ptr[current_bitshift][1].v = v+8-StartLine-LineCount;
		vertices_ptr[current_bitshift][1].u = u+StartPixel+Width; vertices_ptr[current_bitshift][0].v = v+8-StartLine;
	}				
	vertices_ptr[current_bitshift]+=2;
}



void pspDrawLargePixel16 (uint32 Tile, s32 x,s32 y,uint32 StartPixel, uint32 Pixels,uint32 StartLine, uint32 LineCount,short realZ2) {
#ifdef DEBUG_TILE
return;	
#endif
	uint32 u,v,col,TileKey,TileNumber,TileAddr;		
	tile_cache_t *cache_ptr;
  uint8 *pCache;
  
  TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
  if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect; 
  TileAddr &= 0xffff; 
  TileNumber = (TileAddr >> GPUPack.BG.TileShift);
    
  if (GPUPack.BG.DirectColourMode) col=0;     
  else col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    
  TileKey=TileNumber|(col<<14)|(current_bitshift<<12); //TileNumber is TileAddr(16) >> TileShift(min 4) => max 12bits
  cache_ptr=tile_get_cache(current_bitshift,TileKey);
  if (!cache_ptr) {//tile not cached
  	if (tile_cached[current_bitshift]==(TILE_CACHE_SIZE-1)) {
  		tile_clean_cache(current_bitshift,0);  			  		
  		if (tile_cached[current_bitshift]==(TILE_CACHE_SIZE-1)) {debug_log("cache full!!!");return;}  			
  	}
  	cache_ptr=tile_insert_cache(current_bitshift,TileKey);  	
  	
  	u=(cache_ptr->text_idx&63)<<3;
  	v=(cache_ptr->text_idx>>6)<<3;  	
   	pCache = tile_texture[current_bitshift]+(v<<9)+u;
   	if ((cache_ptr->tile_type=pspConvertTile ((u8*)NO_CPU_CACHE(pCache), TileAddr,col)==BLANK_TILE)) return;   	
  }else{
    if (cache_ptr->tile_type==BLANK_TILE) return;
    u=(cache_ptr->text_idx&63)<<3;
		v=(cache_ptr->text_idx>>6)<<3;  	
  }
  
  if (_verticesCol_ptr==_verticesCol_end){
  	debug_log("!!ran out of vertices !!");
  	return;
  }
        		
	_verticesCol_ptr[0].x = x; _verticesCol_ptr[0].y = y; _verticesCol_ptr[0].z = realZ2;		
	_verticesCol_ptr[1].x = x+Pixels; _verticesCol_ptr[1].y = y+LineCount; _verticesCol_ptr[1].z = realZ2;
				
	if (!(Tile & (V_FLIP | H_FLIP))) {//NO FLIP
		u+=StartPixel;
		v+=StartLine;		
	}else if (!(Tile & V_FLIP)) {//HFLIP
		u+=7-StartPixel;
		v+=StartLine;
	}else if (Tile & H_FLIP) {//HFLIP & VFLIP
		u+=7-StartPixel;
		v+=7-StartLine;
	}else {//VFLIP
		u+=StartPixel;
		v+=7-StartLine;
	}	
	_verticesCol_ptr[0].u=u;
	_verticesCol_ptr[1].u=u+1;
	_verticesCol_ptr[0].v=v;
	_verticesCol_ptr[1].v=v+1;
	
	//_verticesCol_ptr[0].color=_verticesCol_ptr[1].color=GPUPack.GFX.ScreenColors[tile_texture[current_bitshift][(v<<9)|u]];
	_verticesCol_ptr+=2;
}

#ifdef GFX_TILE_ME
void pspDrawClippedTile16_me (uint32 Tile, s32 x,s32 y, uint32 StartPixel, uint32 Width, uint32 StartLine, uint32 LineCount,short realZ2) {		
#ifdef DEBUG_TILE
return;	
#endif
	
	uint32 u,v,col,TileKey,TileNumber,TileAddr;		
	tile_cache_t *cache_ptr;
  uint8 *pCache;

  int current_bitshift = ME_GFX->current_bitshift;
  Vertex* *vertices_ptr = ME_GFX->vertices_ptr;
  
  TileAddr = ME_GFX->TileAddress + ((Tile & 0x3ff) << ME_GFX->TileShift);
  if (Tile & 0x100) TileAddr += ME_GFX->NameSelect; 
  TileAddr &= 0xffff; 
  TileNumber = (TileAddr >> ME_GFX->TileShift);
    
  if (ME_GFX->DirectColourMode) col=0;     
  else col = (((Tile >> 10) & ME_GFX->PaletteMask) << ME_GFX->PaletteShift) + ME_GFX->StartPalette;
    
  TileKey=TileNumber|(col<<14)|(current_bitshift<<12); //TileNumber is TileAddr(16) >> TileShift(min 4) => max 12bits
  cache_ptr=tile_get_cache(current_bitshift,TileKey);
  if (!cache_ptr) {//tile not cached
  	if (tile_cached[current_bitshift]==(TILE_CACHE_SIZE-1)) {
  		tile_clean_cache(current_bitshift,0);  			  		
  		if (tile_cached[current_bitshift]==(TILE_CACHE_SIZE-1)) {debug_log("cache full!!!");return;}  			
  	}
  	cache_ptr=tile_insert_cache(current_bitshift,TileKey);  	
  	
  	u=(cache_ptr->text_idx&63)<<3;
  	v=(cache_ptr->text_idx>>6)<<3;  	
   	pCache = tile_texture[current_bitshift]+(v<<9)+u;
   	if ((cache_ptr->tile_type=pspConvertTile ((u8*)NO_CPU_CACHE(pCache), TileAddr,col)==BLANK_TILE)) return;   	
  }else{
    if (cache_ptr->tile_type==BLANK_TILE) return;
    u=(cache_ptr->text_idx&63)<<3;
		v=(cache_ptr->text_idx>>6)<<3;  	
  }
  
  if (vertices_ptr[current_bitshift]==vertices_end[current_bitshift]){
  	debug_log("!!ran out of vertices !!");
  	return;
  }
        		
	vertices_ptr[current_bitshift][0].x = x+StartPixel; vertices_ptr[current_bitshift][0].y = y; vertices_ptr[current_bitshift][0].z = realZ2;		
	vertices_ptr[current_bitshift][1].x = x+StartPixel+Width; vertices_ptr[current_bitshift][1].y = y+LineCount; vertices_ptr[current_bitshift][1].z = realZ2;		
				
	if (!(Tile & (V_FLIP | H_FLIP))) {//NO FLIP
		vertices_ptr[current_bitshift][0].u = u+StartPixel; vertices_ptr[current_bitshift][0].v = v+StartLine;
		vertices_ptr[current_bitshift][1].u = u+StartPixel+Width; vertices_ptr[current_bitshift][1].v = v+StartLine+LineCount;
	}else if (!(Tile & V_FLIP)) {//HFLIP
		vertices_ptr[current_bitshift][1].u = u+8-StartPixel-Width; vertices_ptr[current_bitshift][0].v = v+StartLine;
		vertices_ptr[current_bitshift][0].u = u+8-StartPixel; vertices_ptr[current_bitshift][1].v = v+StartLine+LineCount;
	}
	else if (Tile & H_FLIP) {//HFLIP & VFLIP
		vertices_ptr[current_bitshift][1].u = u+8-StartPixel-Width; vertices_ptr[current_bitshift][1].v = v+8-StartLine-LineCount;
		vertices_ptr[current_bitshift][0].u = u+8-StartPixel; vertices_ptr[current_bitshift][0].v = v+8-StartLine;
	}else {//VFLIP
		vertices_ptr[current_bitshift][0].u = u+StartPixel; vertices_ptr[current_bitshift][1].v = v+8-StartLine-LineCount;
		vertices_ptr[current_bitshift][1].u = u+StartPixel+Width; vertices_ptr[current_bitshift][0].v = v+8-StartLine;
	}				
	vertices_ptr[current_bitshift]+=2;
}




void pspDrawTile16_me        (uint32 Tile, s32 x,s32 y,                                  uint32 StartLine, uint32 LineCount,short realZ2) {		
	uint32 u,v,col,TileKey,TileNumber,TileAddr;		
	tile_cache_t *cache_ptr;
	uint8 *pCache;
  
	int current_bitshift = ME_GFX->current_bitshift;
	Vertex* *vertices_ptr = ME_GFX->vertices_ptr;

	TileAddr = ME_GFX->TileAddress + ((Tile & 0x3ff) << ME_GFX->TileShift);
	if (Tile & 0x100) TileAddr += ME_GFX->NameSelect; 
	TileAddr &= 0xffff; 
	TileNumber = (TileAddr >> ME_GFX->TileShift);
    
	if (ME_GFX->DirectColourMode) col=0;     
	else col = (((Tile >> 10) & ME_GFX->PaletteMask) << ME_GFX->PaletteShift) + ME_GFX->StartPalette;
    
	TileKey=TileNumber|(col<<14)|(current_bitshift<<12); //TileNumber is TileAddr(16) >> TileShift(min 4) => max 12bits

	cache_ptr=tile_get_cache(current_bitshift,TileKey);
	if (!cache_ptr) {//tile not cached
  		if (tile_cached[current_bitshift]==(TILE_CACHE_SIZE-1)) {  	
  			tile_clean_cache(current_bitshift,0);
  			if (tile_cached[current_bitshift]==(TILE_CACHE_SIZE-1)) {debug_log("cache full!!!");return;}  			
  		}

  		cache_ptr=tile_insert_cache(current_bitshift,TileKey);
		  	  	
  		u=(cache_ptr->text_idx&63)<<3;
  		v=(cache_ptr->text_idx>>6)<<3;
   		pCache = tile_texture[current_bitshift]+(v<<9)+u;
   		if ((cache_ptr->tile_type=pspConvertTile ((u8*)NO_CPU_CACHE(pCache), TileAddr,col)==BLANK_TILE)) return;
   	
  }else{
    if (cache_ptr->tile_type==BLANK_TILE) return;
    u=(cache_ptr->text_idx&63)<<3;
		v=(cache_ptr->text_idx>>6)<<3;
  }
  
  if (vertices_ptr[current_bitshift]==vertices_end[current_bitshift]){
  	debug_log("!!ran out of vertices !!");
  	return;
  }

	
	vertices_ptr[current_bitshift][0].x = x; vertices_ptr[current_bitshift][0].y = y/*-StartLine*/; vertices_ptr[current_bitshift][0].z = realZ2;		
	vertices_ptr[current_bitshift][1].x = x+8; vertices_ptr[current_bitshift][1].y = y+LineCount/*-StartLine+8*/; vertices_ptr[current_bitshift][1].z = realZ2;				
	if (!(Tile & (V_FLIP | H_FLIP))) {//NO FLIP
		vertices_ptr[current_bitshift][0].u = u; vertices_ptr[current_bitshift][0].v = v+StartLine;		
		vertices_ptr[current_bitshift][1].u = u+8; vertices_ptr[current_bitshift][1].v = v+StartLine+LineCount;		
	}else if (!(Tile & V_FLIP)) {//HFLIP
		vertices_ptr[current_bitshift][1].u = u; vertices_ptr[current_bitshift][0].v = v+StartLine;		
		vertices_ptr[current_bitshift][0].u = u+8; vertices_ptr[current_bitshift][1].v = v+StartLine+LineCount;			
	}else if (Tile & H_FLIP) {//HFLIP & VFLIP						
		vertices_ptr[current_bitshift][1].u = u; vertices_ptr[current_bitshift][1].v = v+8-StartLine-LineCount;
		vertices_ptr[current_bitshift][0].u = u+8; vertices_ptr[current_bitshift][0].v = v+8-StartLine;
	}else {//VFLIP
		vertices_ptr[current_bitshift][0].u = u; vertices_ptr[current_bitshift][1].v = v+8-StartLine-LineCount;
		vertices_ptr[current_bitshift][1].u = u+8; vertices_ptr[current_bitshift][0].v = v+8-StartLine;
	}
	vertices_ptr[current_bitshift]+=2;				

}

#endif