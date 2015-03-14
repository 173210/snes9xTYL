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
#include "tile_psp.h"




#define M7 19
#define M8 19

void ComputeClipWindows ();
extern int os9x_BG0,os9x_BG1,os9x_BG2,os9x_BG3,os9x_OBJ,os9x_easy,os9x_fastsprite;
extern int os9x_render;

int add_sub_mode;
int last_palette;
int current_bitshift;
int render_timestamp;

//extern u16 __attribute__((aligned(16))) clut256[256*3];
extern u16* clut256;
u16* clut;
u32 fixedcol16;
u32 fixedcol;
int rendering_beware_fix16;

// for OBJ & BGS
struct Vertex *vertices[3];
struct Vertex *vertices_ptr[3];
struct Vertex *vertices_end[3];
// for Blending
struct Vertex *_vertices;
struct Vertex *_vertices_ptr;

struct VertexCol *_verticesCol;
struct VertexCol *_verticesCol_ptr;
struct VertexCol *_verticesCol_end;

s32 realZ1,realZ2;

extern void S9xSetupOBJ(void);

extern uint8 BitShifts[8][4];
extern uint8 TileShifts[8][4];
extern uint8 PaletteShifts[8][4];
extern uint8 PaletteMasks[8][4];
extern uint8 Depths[8][4];
extern uint8 BGSizes [2];

extern NormalTileRendererPSP DrawTilePtr;
extern ClippedTileRendererPSP DrawClippedTilePtr;
extern NormalTileRendererPSP DrawHiResTilePtr;
extern ClippedTileRendererPSP DrawHiResClippedTilePtr;
extern LargePixelRendererPSP DrawLargePixelPtr;

extern struct SBG BG;

extern struct SLineData LineData[240];
extern struct SLineMatrixData LineMatrixData [240];

extern uint8  Mode7Depths [2];

#define ON_MAIN(N) \
(GFX.r212c & (1 << (N)) && \
 !(PPU.BG_Forced & (1 << (N))))

#define SUB_OR_ADD(N) \
(GFX.r2131 & (1 << (N)))

#define ON_SUB(N) \
((GFX.r2130 & 0x30) != 0x30 && \
 (GFX.r2130 & 2) && \
 (GFX.r212d & (1 << N)) && \
 !(PPU.BG_Forced & (1 << (N))))

#define ANYTHING_ON_SUB \
((GFX.r2130 & 0x30) != 0x30 && \
 (GFX.r2130 & 2) && \
 (GFX.r212d & 0x1f))

#define ADD_OR_SUB_ON_ANYTHING \
(GFX.r2131 & 0x3f)

#define BLACK BUILD_PIXEL(0,0,0)


void pspDrawTile16 (uint32 Tile, s32 x,s32 y, uint32 StartLine,
	         uint32 LineCount);
void pspDrawClippedTile16 (uint32 Tile, s32 x,s32 y,
		        uint32 StartPixel, uint32 Width,
		        uint32 StartLine, uint32 LineCount);
void pspDrawHiResTile16 (uint32 Tile, s32 x,s32 y, uint32 StartLine,
	         uint32 LineCount);
void pspDrawHiResClippedTile16 (uint32 Tile, s32 x,s32 y,
		        uint32 StartPixel, uint32 Width,
		        uint32 StartLine, uint32 LineCount);		        
		        
void pspDrawLargePixel16 (uint32 Tile, s32 x,s32 y,
		       uint32 StartPixel, uint32 Pixels,
		       uint32 StartLine, uint32 LineCount);



inline void pspSelectAddSubMode()  {		
	if (GFX.r2131 & 0x80) {
		if (GFX.r2131 & 0x40) {
			if (GFX.r2130 & 2) {
				add_sub_mode=1;
				//debug_log("tile sub 1_2");
			} else {
				add_sub_mode=2;
				// Fixed colour substraction
				//debug_log("Fixed colour substraction");
			}			
		} else {
			add_sub_mode=3;
			//debug_log("tile sub");
		}
	} else {
		if (GFX.r2131 & 0x40) {
			if (GFX.r2130 & 2) {
				add_sub_mode=4;		    
				//debug_log("tile add1_2");		    
			} else {
				add_sub_mode=5;
				// Fixed colour addition
				//debug_log("Fixed colour addition");
			}
	  } else {
	  	add_sub_mode=6;
			//debug_log("tile add");
		}
	}	
}

void initRendering(){
	sceGuStart(GU_DIRECT,list);
	//rendering options
				
	sceGuEnable(GU_DEPTH_TEST);
	sceGuDepthFunc(GU_GREATER);
		
	sceGuEnable(GU_ALPHA_TEST);			
	sceGuAlphaFunc(GU_EQUAL,0,0x1);

	sceGuEnable(GU_SCISSOR_TEST);
	sceGuEnable(GU_TEXTURE_2D);	
	sceGuTexFilter(GU_NEAREST,GU_NEAREST);
	
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
	sceGuTexScale(1.0f/512.0f,1.0f/512.0f);
	sceGuTexOffset(0,0);
	sceGuClutMode(GU_PSM_5551,0,255,0);	
	
	last_palette=0;  //start in non directcolour mode, coz first things to be rendered are s
	clut = (u16*)NO_CPU_CACHE(&clut256[0]);
	memcpy(clut,&IPPU.ScreenColors[0],256*2);		
	if (rendering_beware_fix16)
	for (int i=0;i<256;i++) {
		if (clut[i]==fixedcol16) clut[i]^=1<<10; //swap blue 1st bit
	}
	
	clut[0]|=0x8000;
  sceGuClutLoad(256/16,clut);
  
}

void endRendering(){
	sceGuFinish();
	sceGuSync(0,0);
}

void waitRendering() {
	
	/*sceGuFinish();
	sceGuSync(0,0);
	sceGuStart(GU_DIRECT,list);
	*/
}

void pspDrawFASTOBJS (bool8 OnMain, uint8 D, uint8 drawmode)
{

  //uint32 O;
  s32 Xs,Ys;
  int i;
  uint32 BaseTile, Tile;
    
  BG.BitShift = 4;  
  BG.TileShift = 5;
  BG.TileAddress = PPU.OBJNameBase;
  BG.StartPalette = 128;
  BG.PaletteShift = 4;
  BG.PaletteMask = 7;  
  current_bitshift=TILE_4BIT;
  BG.Buffer = tile_texture[TILE_4BIT];//IPPU.TileCache8 [TILE_4BIT];
  BG.Buffered = IPPU.TileCached [TILE_4BIT];
  BG.NameSelect = PPU.OBJNameSelect;
  BG.DirectColourMode = false;
        
/********************************************************/	
/****************PSP STUFF*******************************/    
/********************************************************/			
		
	int num_vert=0;//8192;//0;
	int I=0;
	 for (int S = GFX.OBJList [I++]; S >= 0; S = GFX.OBJList [I++]) {
	 	int VPos = GFX.VPositions [S];
			int Size = GFX.Sizes[S];
			int TileInc = 1;
			int Offset;
			if (VPos + Size <= (int) GFX.StartY || VPos > (int) GFX.EndY)  continue;
			
			num_vert+=(Size/8)*(Size/8);
	}
  
  
  if (!(vertices[current_bitshift]=(struct Vertex*)sceGuGetMemory(num_vert*2* sizeof(struct Vertex)))){
  	debug_log("not enough vertices!");
  	return;
  }
	vertices_ptr[current_bitshift]=vertices[current_bitshift];
	vertices_end[current_bitshift]=vertices_ptr[current_bitshift]+num_vert*2;
			
/********************************************************/
/********************************************************/				
    GFX.Z1 = (D + 2);
    realZ1=(int)(GFX.Z1);
		I=0;
    for (int S = GFX.OBJList [I++]; S >= 0; S = GFX.OBJList [I++]) {
			int VPos = GFX.VPositions [S];
			int Size = GFX.Sizes[S];
			int TileInc = 1;
			int Offset;

			if (VPos + Size <= (int) GFX.StartY || VPos > (int) GFX.EndY)
		    continue;

			//drawing sub/added sprites => palette has to be 4-7
			if (OnMain){
				if ((drawmode==1) && (PPU.OBJ [S].Palette < 4) ) continue;
				//drawing not sub/added sprites => palette has to be 4-7 or no add/sub in effect for OBJ
				if ((drawmode==2) && (PPU.OBJ [S].Palette >= 4) && SUB_OR_ADD(4)) continue;
			}
			
			BaseTile = PPU.OBJ[S].Name | (PPU.OBJ[S].Palette << 10);

			if (PPU.OBJ[S].HFlip)	{
	    	BaseTile += ((Size >> 3) - 1) | H_FLIP;
		    TileInc = -1;
			}
			if (PPU.OBJ[S].VFlip)  BaseTile |= V_FLIP;

			int clipcount = GFX.pCurrentClip->Count [4];
			if (!clipcount) clipcount = 1;
			
			GFX.Z2 = ((PPU.OBJ[S].Priority + 1) * 4 + D);
			realZ2=(int)(GFX.Z2);//+I-1;
	
			for (int clip = 0; clip < clipcount; clip++){
		    int Left; 
		    int Right;
		    if (!GFX.pCurrentClip->Count [4]){
					Left = 0;
					Right = 256;
		    }else{
					Left = GFX.pCurrentClip->Left [clip][4];
					Right = GFX.pCurrentClip->Right [clip][4];
	    	}

		    if (Right <= Left || PPU.OBJ[S].HPos + Size <= Left || PPU.OBJ[S].HPos >= Right) continue;
		    
	

		    for (int Y = 0; Y < Size; Y += 8) {
					if (VPos + Y + 7 >= (int) GFX.StartY && VPos + Y <= (int) GFX.EndY){
				    int StartLine;
				    int TileLine;
				    int LineCount;
				    int Last;
			    			    
				    if ((StartLine = VPos + Y) < (int) GFX.StartY) {
							StartLine = GFX.StartY - StartLine;
							LineCount = 8 - StartLine;
				    }else{
							StartLine = 0;
							LineCount = 8;
				    }
				    if ((Last = VPos + Y + 7 - GFX.EndY) > 0)
							if ((LineCount -= Last) <= 0) break;

							TileLine = StartLine;// << 3;
							Ys = (VPos + Y + StartLine);
				    	if (!PPU.OBJ[S].VFlip) Tile = BaseTile + (Y << 1);
				    	else Tile = BaseTile + ((Size - Y - 8) << 1);

				    	int Middle = Size >> 3;
				    	if (PPU.OBJ[S].HPos < Left) {
								Tile += ((Left - PPU.OBJ[S].HPos) >> 3) * TileInc;
								Middle -= (Left - PPU.OBJ[S].HPos) >> 3;
								Xs = Left ;
								if ((Offset = (Left - PPU.OBJ[S].HPos) & 7)) {
						    	Xs -= Offset ;
						    	int W = 8 - Offset;
						    	int Width = Right - Left;
						    	if (W > Width) W = Width;
					    		
					    		pspDrawClippedTile16 (Tile, Xs,Ys, Offset, W,TileLine, LineCount);

			    				if (W >= Width) continue;
						    	Tile += TileInc;
						    	Middle--;
						    	Xs += 8 ;
								}	
		    			}	else Xs = PPU.OBJ[S].HPos ;

				    	if (PPU.OBJ[S].HPos + Size >= Right) {
								Middle -= ((PPU.OBJ[S].HPos + Size + 7) - Right) >> 3;
								Offset = (Right - (PPU.OBJ[S].HPos + Size)) & 7;
				    	}else Offset = 0;

				    	for (int X = 0; X < Middle; X++, Xs += 8 , Tile += TileInc){
								pspDrawTile16 (Tile, Xs,Ys, TileLine, LineCount);
				    	}
				    	if (Offset){
								
								pspDrawClippedTile16 (Tile, Xs,Ys, 0, Offset,TileLine, LineCount);
				    	}
					}
		    }
			}
    }
  
  
	
	if (vertices_ptr[current_bitshift]-vertices[current_bitshift]) {  			  	
  	
		sceGuTexImage(0,512,512,512,tile_texture[current_bitshift]);  	
  	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,(vertices_ptr[current_bitshift]-vertices[current_bitshift]),0,vertices[current_bitshift]);
	}			
}


void pspDrawOBJS (bool8 OnMain, uint8 D, uint8 drawmode) {
  //uint32 O;
  s32 Xs,Ys;
  int speZ;
  uint32 BaseTile, Tile;
    
  BG.BitShift = 4;
  current_bitshift=TILE_4BIT;
  BG.TileShift = 5;
  BG.TileAddress = PPU.OBJNameBase;
  BG.StartPalette = 128;
  BG.PaletteShift = 4;
  BG.PaletteMask = 7;
  BG.Buffer = tile_texture[TILE_4BIT];//IPPU.TileCache8 [TILE_4BIT];
  BG.Buffered = IPPU.TileCached [TILE_4BIT];
  BG.NameSelect = PPU.OBJNameSelect;
  BG.DirectColourMode = false;
        
/********************************************************/	
/****************PSP STUFF*******************************/    
/********************************************************/		
			
	int num_vert=0;//8192;//0;
	int I=0;
	 for (int S = GFX.OBJList [I++]; S >= 0; S = GFX.OBJList [I++]) {
	 	int VPos = GFX.VPositions [S];
			int Size = GFX.Sizes[S];
			if (VPos + Size <= (int) GFX.StartY || VPos > (int) GFX.EndY)
		    continue;
			//drawing sub/added sprites => palette has to be 4-7			
			num_vert+=(Size/8)*(Size/8);
	}
	
	
  if (!(vertices[current_bitshift]=(struct Vertex*)sceGuGetMemory(num_vert*2* sizeof(struct Vertex)))){
  	debug_log("not enough vertices!");
  	return;
  }
	vertices_ptr[current_bitshift]=vertices[current_bitshift];
	vertices_end[current_bitshift]=vertices_ptr[current_bitshift]+num_vert*2;
							  				  				
/********************************************************/
/********************************************************/				
    GFX.Z1 = (D + 2);
    realZ1=(int)(GFX.Z1);
		I=0;
    for (int S = GFX.OBJList [I++]; S >= 0; S = GFX.OBJList [I++]) {
		int VPos = GFX.VPositions [S];
		int Size = GFX.Sizes[S];
		int TileInc = 1;
		int Offset;

		if (VPos + Size <= (int) GFX.StartY || VPos > (int) GFX.EndY)
	    continue;

		//drawing sub/added sprites => palette has to be 4-7
		speZ=0;
		if (OnMain){
			if ((drawmode==1) && (PPU.OBJ [S].Palette < 4) ) speZ=1;
			//drawing not sub/added sprites => palette has to be 4-7 or no add/sub in effect for OBJ
			else if ((drawmode==2) && (PPU.OBJ [S].Palette >= 4) && SUB_OR_ADD(4)) speZ=1;									
		} 
			
			
		BaseTile = PPU.OBJ[S].Name | (PPU.OBJ[S].Palette << 10);

		if (PPU.OBJ[S].HFlip)	{
    		BaseTile += ((Size >> 3) - 1) | H_FLIP;
			TileInc = -1;
		}
		if (PPU.OBJ[S].VFlip)  BaseTile |= V_FLIP;

		int clipcount = GFX.pCurrentClip->Count [4];
		if (!clipcount) clipcount = 1;
		
		GFX.Z2 = ((PPU.OBJ[S].Priority + 1) * 4 + D);
		if (!speZ) realZ2=(int)(GFX.Z2);//+I-1;
		else realZ2=0;
									
	
		for (int clip = 0; clip < clipcount; clip++){
			int Left; 
			int Right;
			if (!GFX.pCurrentClip->Count [4]){
					Left = 0;
					Right = 256;
			}else{
					Left = GFX.pCurrentClip->Left [clip][4];
					Right = GFX.pCurrentClip->Right [clip][4];
    		}

			if (Right <= Left || PPU.OBJ[S].HPos + Size <= Left || PPU.OBJ[S].HPos >= Right) continue;
	    
	

			for (int Y = 0; Y < Size; Y += 8) {
				if (VPos + Y + 7 >= (int) GFX.StartY && VPos + Y <= (int) GFX.EndY) {
					int StartLine;
					int TileLine;
					int LineCount;
					int Last;
			    			    
					if ((StartLine = VPos + Y) < (int) GFX.StartY) {
							StartLine = GFX.StartY - StartLine;
							LineCount = 8 - StartLine;
					} else {
							StartLine = 0;
							LineCount = 8;
					}
					if ((Last = VPos + Y + 7 - GFX.EndY) > 0)
							if ((LineCount -= Last) <= 0) break;

						TileLine = StartLine;// << 3;
						Ys = (VPos + Y + StartLine);
					if (!PPU.OBJ[S].VFlip) Tile = BaseTile + (Y << 1);
					else Tile = BaseTile + ((Size - Y - 8) << 1);

					int Middle = Size >> 3;
					if (PPU.OBJ[S].HPos < Left) {
						Tile += ((Left - PPU.OBJ[S].HPos) >> 3) * TileInc;
						Middle -= (Left - PPU.OBJ[S].HPos) >> 3;
						Xs = Left ;
						if ((Offset = (Left - PPU.OBJ[S].HPos) & 7)) {
				   			Xs -= Offset ;
				   			int W = 8 - Offset;
				   			int Width = Right - Left;
				   			if (W > Width) W = Width;
					   		
			   				pspDrawClippedTile16 (Tile, Xs,Ys, Offset, W,TileLine, LineCount);
					   		
	    					if (W >= Width) continue;
				   			Tile += TileInc;
				   			Middle--;
				   			Xs += 8 ;
						}	
	    			}	else Xs = PPU.OBJ[S].HPos ;

					if (PPU.OBJ[S].HPos + Size >= Right) {
						Middle -= ((PPU.OBJ[S].HPos + Size + 7) - Right) >> 3;
						Offset = (Right - (PPU.OBJ[S].HPos + Size)) & 7;
					}else Offset = 0;

					for (int X = 0; X < Middle; X++, Xs += 8 , Tile += TileInc){
						pspDrawTile16 (Tile, Xs,Ys, TileLine, LineCount);
					}
					if (Offset){
						pspDrawClippedTile16 (Tile, Xs,Ys, 0, Offset,TileLine, LineCount);
					}
				}
		    }
		}
    }
  /*{
  	int i=0;
  	char str[32];
		tile_cache_t *p=tile_cache_first_free[TILE_4BIT];
		while (p) {i++;p=p->next;}
		sprintf(str,"cached %d free %d/%d",tile_cached[TILE_4BIT],i,i+tile_cached[TILE_4BIT]);
		info(32,12,str);	    			
	}*/
    
	if (vertices_ptr[current_bitshift]-vertices[current_bitshift]) {  	  	  	
		
		sceGuEnable(GU_STENCIL_TEST);//stencil		
		sceGuClearStencil(0);
		sceGuClear(GU_STENCIL_BUFFER_BIT);
		sceGuStencilOp( GU_KEEP, GU_INCR, GU_INCR);
		sceGuStencilFunc(GU_EQUAL,0,0xFFFFFFFF);
		sceGuTexImage(0,512,512,512,tile_texture[current_bitshift]);  	
  		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,(vertices_ptr[current_bitshift]-vertices[current_bitshift]),0,vertices[current_bitshift]);  	  	  	
	}	
	sceGuDisable(GU_STENCIL_TEST);//stencil
}


void pspDrawBackgroundMosaic (uint32 BGMode, uint32 bg, uint8 Z1, uint8 Z2) {    
	uint32 Tile;
  uint16 *SC0;
  uint16 *SC1;
  uint16 *SC2;
  uint16 *SC3;
  uint8 depths [2] = {Z1, Z2};
  
  if (last_palette!=BG.DirectColourMode){  	 
			//DirectColourMode has changed, so we have to reload palette
			//waitRendering();
			last_palette=BG.DirectColourMode;
			clut = (u16*)NO_CPU_CACHE(&clut256[0]);
			memcpy(clut,GFX.ScreenColors,256*2);			
			if (rendering_beware_fix16)
			for (int i=0;i<256;i++) {
				if (clut[i]==fixedcol16) clut[i]^=1<<10; //swap blue 1st bit
			}
			clut[0]|=0x8000;
  		sceGuClutLoad(256/16,clut);			
	}
  //bg is 32x30 tile but needs more in case of clipping or scroll changing every line
	int num_vert=((GFX.EndY-GFX.StartY+1)+2)*(256/PPU.Mosaic+6);
  if (!(_verticesCol=(struct VertexCol*)sceGuGetMemory(num_vert*2* sizeof(struct VertexCol)))){
  	debug_log("not enough vertices!");
  	return;
  }
	_verticesCol_ptr=_verticesCol;
	_verticesCol_end=_verticesCol_ptr+num_vert*2;
/***********************************************************/
/***********************************************************/
    
  if (BGMode == 0) BG.StartPalette = bg << 5;
  else BG.StartPalette = 0;
    
  SC0 = (uint16 *) &VRAM[PPU.BG[bg].SCBase << 1];

  if (PPU.BG[bg].SCSize & 1) SC1 = SC0 + 1024;
  else SC1 = SC0;

	if((SC1-(unsigned short*)VRAM)>0x10000) SC1-=0x10000;

  if (PPU.BG[bg].SCSize & 2) SC2 = SC1 + 1024;
  else SC2 = SC0;

	if((SC2-(unsigned short*)VRAM)>0x10000) SC2-=0x10000;


  if (PPU.BG[bg].SCSize & 1) SC3 = SC2 + 1024;
  else SC3 = SC2;

	if((SC3-(unsigned short*)VRAM)>0x10000) SC3-=0x10000;

  uint32 Lines;
  uint32 OffsetMask;
  uint32 OffsetShift;

  if (BG.TileSize == 16) {
		OffsetMask = 0x3ff;
		OffsetShift = 4;
  } else {
		OffsetMask = 0x1ff;
		OffsetShift = 3;
  }

  for (uint32 Y = GFX.StartY; Y <= GFX.EndY; Y += Lines) {
		uint32 VOffset = LineData [Y].BG[bg].VOffset;
		uint32 HOffset = LineData [Y].BG[bg].HOffset;
		uint32 MosaicOffset = Y % PPU.Mosaic;

		for (Lines = 1; Lines < PPU.Mosaic - MosaicOffset; Lines++)
	    if ((VOffset != LineData [Y + Lines].BG[bg].VOffset) || (HOffset != LineData [Y + Lines].BG[bg].HOffset)) break;
	
		uint32 MosaicLine = VOffset + Y - MosaicOffset;

		if (Y + Lines > GFX.EndY) Lines = GFX.EndY + 1 - Y;
		uint32 VirtAlign = (MosaicLine & 7);// << 3;
	
		uint16 *b1;
		uint16 *b2;

		uint32 ScreenLine = MosaicLine >> OffsetShift;
		uint32 Rem16 = MosaicLine & 15;

		if (ScreenLine & 0x20) b1 = SC2, b2 = SC3;
		else b1 = SC0, b2 = SC1;

		b1 += (ScreenLine & 0x1f) << 5;
		b2 += (ScreenLine & 0x1f) << 5;
		uint16 *t;
		uint32 Left = 0;
		uint32 Right = 256;

		uint32 ClipCount = GFX.pCurrentClip->Count [bg];
		uint32 HPos = HOffset;
		uint32 PixWidth = PPU.Mosaic;

		if (!ClipCount) ClipCount = 1;

		for (uint32 clip = 0; clip < ClipCount; clip++) {
	    if (GFX.pCurrentClip->Count [bg]) {
				Left = GFX.pCurrentClip->Left [clip][bg];
				Right = GFX.pCurrentClip->Right [clip][bg];
				uint32 r = Left % PPU.Mosaic;
				HPos = HOffset + Left;
				PixWidth = PPU.Mosaic - r;
	    }
	    //uint32 s = Y * GFX.PPL + Left * GFX.PixSize;
	    s32 Xt=Left;
	    s32 Yt=Y;
	    for (uint32 x = Left; x < Right; x += PixWidth, /*s += PixWidth * GFX.PixSize*/Xt+=PixWidth,HPos += PixWidth, PixWidth = PPU.Mosaic) {
				uint32 Quot = (HPos & OffsetMask) >> 3;

				if (x + PixWidth >= Right) PixWidth = Right - x;

				if (BG.TileSize == 8) {
		    	if (Quot > 31) t = b2 + (Quot & 0x1f);
		    	else t = b1 + Quot;
				} else {
		    	if (Quot > 63) t = b2 + ((Quot >> 1) & 0x1f);
		    	else t = b1 + (Quot >> 1);
				}

				Tile = READ_2BYTES (t);
				GFX.Z1 = GFX.Z2 = depths [(Tile & 0x2000) >> 13];
				realZ1=(int)GFX.Z1;realZ2=(int)GFX.Z2;

				// pspDraw tile...
				if (BG.TileSize != 8)	{
		    	if (Tile & H_FLIP) {
						// Horizontal flip, but what about vertical flip ?
						if (Tile & V_FLIP) {
			    		// Both horzontal & vertical flip
			    		if (Rem16 < 8) {
								pspDrawLargePixel16 (Tile + 17 - (Quot & 1), Xt,Yt,HPos & 7, PixWidth,VirtAlign, Lines);
			    		} else {
								pspDrawLargePixel16 (Tile + 1 - (Quot & 1), Xt,Yt,HPos & 7, PixWidth,VirtAlign, Lines);
					    }
						} else {
			    		// Horizontal flip only
			    		if (Rem16 > 7) {
								pspDrawLargePixel16 (Tile + 17 - (Quot & 1), Xt,Yt,HPos & 7, PixWidth,VirtAlign, Lines);
			    		} else {
								pspDrawLargePixel16 (Tile + 1 - (Quot & 1), Xt,Yt,HPos & 7, PixWidth,VirtAlign, Lines);
			    		}
						}
		    	} else {
						// No horizontal flip, but is there a vertical flip ?
						if (Tile & V_FLIP) {
				    	// Vertical flip only
				    	if (Rem16 < 8) {
								pspDrawLargePixel16 (Tile + 16 + (Quot & 1), Xt,Yt,HPos & 7, PixWidth,VirtAlign, Lines);
				    	} else {
								pspDrawLargePixel16 (Tile + (Quot & 1), Xt,Yt,HPos & 7, PixWidth,VirtAlign, Lines);
				    	}
						} else {
				    	// Normal unflipped
				    	if (Rem16 > 7) {
								pspDrawLargePixel16 (Tile + 16 + (Quot & 1), Xt,Yt,HPos & 7, PixWidth,VirtAlign, Lines);
				    	} else {
								pspDrawLargePixel16 (Tile + (Quot & 1), Xt,Yt,HPos & 7, PixWidth,VirtAlign, Lines);
				    	}
						}
		    	}
				} else 	pspDrawLargePixel16 (Tile, Xt,Yt, HPos & 7, PixWidth,VirtAlign, Lines);
	    }
		}
  }
  
  //render
	if (_verticesCol_ptr-_verticesCol) {  				
  	
		sceGuTexImage(0,512,512,512,tile_texture[current_bitshift]);		
  	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT/*GU_COLOR_5551*/|GU_VERTEX_16BIT|GU_TRANSFORM_2D,(_verticesCol_ptr-_verticesCol),0,_verticesCol);  	
  	
	}
}


void pspDrawBackgroundMode5 (uint32 /* BGMODE */, uint32 bg, uint8 Z1, uint8 Z2) {
	int i;

  GFX.Pitch = GFX.RealPitch;
  GFX.PPL = GFX.PPLx2 >> 1;
  uint8 depths [2] = {Z1, Z2};

  uint32 Tile;
  uint16 *SC0;
  uint16 *SC1;
  uint16 *SC2;
  uint16 *SC3;
  uint32 Width;
  
  if (last_palette!=BG.DirectColourMode){  	 
			//DirectColourMode has changed, so we have to reload palette
			//waitRendering();
			last_palette=BG.DirectColourMode;
			clut = (u16*)NO_CPU_CACHE(&clut256[0]);
			memcpy(clut,GFX.ScreenColors,256*2);			
			if (rendering_beware_fix16)
			for (int i=0;i<256;i++) {
				if (clut[i]==fixedcol16) clut[i]^=1<<10; //swap blue 1st bit
			}
			clut[0]|=0x8000;
  		sceGuClutLoad(256/16,clut);			
	}
  
  //bg is 32x30 tile but needs more in case of clipping or scroll changing every line
	int num_vert=((GFX.EndY-GFX.StartY+1)+2)*(256/8+6);			
  if (!(vertices[current_bitshift]=(struct Vertex*)sceGuGetMemory(num_vert*2* sizeof(struct Vertex)))){
  	debug_log("not enough vertices!");
  	return;
  }
	vertices_ptr[current_bitshift]=vertices[current_bitshift];
	vertices_end[current_bitshift]=vertices_ptr[current_bitshift]+num_vert*2;
		
/********************************************************/
/********************************************************/
    
  BG.StartPalette = 0;
                   
	
    

	SC0 = (uint16 *) &VRAM[PPU.BG[bg].SCBase << 1];
  if ((PPU.BG[bg].SCSize & 1)) SC1 = SC0 + 1024;
  else SC1 = SC0;

	if((SC1-(unsigned short*)VRAM)>0x10000) SC1=(uint16*)&VRAM[(((uint8*)SC1)-VRAM)%0x10000];

	if ((PPU.BG[bg].SCSize & 2)) SC2 = SC1 + 1024;
	else SC2 = SC0;

	if((SC2-(unsigned short*)VRAM)>0x10000) SC2=(uint16*)&VRAM[(((uint8*)SC2)-VRAM)%0x10000];

	if ((PPU.BG[bg].SCSize & 1)) SC3 = SC2 + 1024;
	else SC3 = SC2;
    
	if((SC3-(unsigned short*)VRAM)>0x10000) SC3=(uint16*)&VRAM[(((uint8*)SC3)-VRAM)%0x10000];


	int Lines;
	int VOffsetMask;
  int VOffsetShift;

  if (BG.TileSize == 16) {
		VOffsetMask = 0x3ff;
		VOffsetShift = 4;
  } else {
		VOffsetMask = 0x1ff;
		VOffsetShift = 3;
	}
  int endy = GFX.EndY;

  for (int Y = GFX.StartY; Y <= endy; Y += Lines) {
		int y = Y;
		uint32 VOffset = LineData [y].BG[bg].VOffset;
		uint32 HOffset = LineData [y].BG[bg].HOffset;
		int VirtAlign = (Y + VOffset) & 7;
	
		for (Lines = 1; Lines < 8 - VirtAlign; Lines++)
	    if ((VOffset != LineData [y + Lines].BG[bg].VOffset) || (HOffset != LineData [y + Lines].BG[bg].HOffset)) break;

		HOffset <<= 1;
		if (Y + Lines > endy) Lines = endy + 1 - Y;
	//	VirtAlign <<= 3;
	
		int ScreenLine = (VOffset + Y) >> VOffsetShift;
		int t1;
		int t2;
		if (((VOffset + Y) & 15) > 7) {
			t1 = 16;
			t2 = 0;
		} else {
			t1 = 0;
		  t2 = 16;
		}
		uint16 *b1;
		uint16 *b2;
	
		if (ScreenLine & 0x20) b1 = SC2, b2 = SC3;
		else b1 = SC0, b2 = SC1;
	
		b1 += (ScreenLine & 0x1f) << 5;
		b2 += (ScreenLine & 0x1f) << 5;
	
		int clipcount = GFX.pCurrentClip->Count [bg];
		if (!clipcount) clipcount = 1;
		for (int clip = 0; clip < clipcount; clip++) {
	    int Left;
	    int Right;

	    if (!GFX.pCurrentClip->Count [bg]) {
				Left = 0;
				Right = 512;
	    } else {
				Left = GFX.pCurrentClip->Left [clip][bg]* 2;
				Right = GFX.pCurrentClip->Right [clip][bg] * 2;

				if (Right <= Left) continue;
	    }

	    //uint32 s = (Left>>1) * GFX.PixSize + Y * 256;//GFX.PPL;
	    s32 Xt=Left>>1;
	    s32 Yt=Y;
	    uint32 HPos = (HOffset + Left * 1) & 0x3ff;

	    uint32 Quot = HPos >> 3;
	    uint32 Count = 0;
	    
	    uint16 *t;
	    if (Quot > 63) t = b2 + ((Quot >> 1) & 0x1f);
	    else t = b1 + (Quot >> 1);

	    Width = Right - Left;
	    // Left hand edge clipped tile
	    if (HPos & 7) {
				int Offset = (HPos & 7);
				Count = 8 - Offset;
				if (Count > Width) Count = Width;
				//s -= Offset>>1;
				Xt-=Offset>>1;
				Tile = READ_2BYTES (t);
				GFX.Z1 = GFX.Z2 = depths [(Tile & 0x2000) >> 13];
				realZ1=(int)GFX.Z1;realZ2=(int)GFX.Z2;
	
				if (BG.TileSize == 8) {
		    	if (!(Tile & H_FLIP)) {
						// Normal, unflipped
						pspDrawHiResClippedTile16 (Tile + (Quot & 1), Xt,Yt, Offset, Count, VirtAlign, Lines);
		    	} else {
						// H flip
						pspDrawHiResClippedTile16 (Tile + 1 - (Quot & 1),Xt,Yt, Offset, Count, VirtAlign, Lines);
		    	}
				} else {
		    	if (!(Tile & (V_FLIP | H_FLIP))) {
						// Normal, unflipped
						pspDrawHiResClippedTile16 (Tile + t1 + (Quot & 1), Xt,Yt, Offset, Count, VirtAlign, Lines);
		    	} else
		    	if (Tile & H_FLIP) {
						if (Tile & V_FLIP) {
			    		// H & V flip
			    		pspDrawHiResClippedTile16 (Tile + t2 + 1 - (Quot & 1), Xt,Yt, Offset, Count, VirtAlign, Lines);
						} else {
			    		// H flip only
			    		pspDrawHiResClippedTile16 (Tile + t1 + 1 - (Quot & 1), Xt,Yt, Offset, Count, VirtAlign, Lines);
						}
		    	} else {
						// V flip only
						pspDrawHiResClippedTile16 (Tile + t2 + (Quot & 1), Xt,Yt, Offset, Count, VirtAlign, Lines);
		    	}
				}

				t += Quot & 1;
				if (Quot == 63) t = b2;
				else if (Quot == 127) t = b1;
				Quot++;
				//s += 4;
				Xt+=4;
	    }

	    // Middle, unclipped tiles
	    Count = Width - Count;
	    int Middle = Count >> 3;
	    Count &= 7;
	    for (int C = Middle; C > 0; Xt+=4/*s += 4*/, Quot++, C--) {
				Tile = READ_2BYTES(t);
				GFX.Z1 = GFX.Z2 = depths [(Tile & 0x2000) >> 13];
				realZ1=(int)GFX.Z1;realZ2=(int)GFX.Z2;
				if (BG.TileSize == 8) {
		    	if (!(Tile & H_FLIP)) {
						// Normal, unflipped
						pspDrawHiResTile16 (Tile + (Quot & 1),Xt,Yt, VirtAlign, Lines);
		    	} else {
						// H flip
						pspDrawHiResTile16 (Tile + 1 - (Quot & 1), Xt,Yt, VirtAlign, Lines);
		    	}
				} else {
		    	if (!(Tile & (V_FLIP | H_FLIP))) {
						// Normal, unflipped
						pspDrawHiResTile16 (Tile + t1 + (Quot & 1),Xt,Yt, VirtAlign, Lines);
		    	} else if (Tile & H_FLIP) {
						if (Tile & V_FLIP) {
				    	// H & V flip
				    	pspDrawHiResTile16 (Tile + t2 + 1 - (Quot & 1), Xt,Yt, VirtAlign, Lines);
						} else {			 
				    	// H flip only
			  	  	pspDrawHiResTile16 (Tile + t1 + 1 - (Quot & 1), Xt,Yt, VirtAlign, Lines);
						}
			    } else {
						// V flip only
						pspDrawHiResTile16 (Tile + t2 + (Quot & 1), Xt,Yt, VirtAlign, Lines);
			    }
				}

				t += Quot & 1;
				if (Quot == 63) t = b2;
				else if (Quot == 127) t = b1;
			}

	    // Right-hand edge clipped tiles
	    if (Count) {
				Tile = READ_2BYTES(t);
				GFX.Z1 = GFX.Z2 = depths [(Tile & 0x2000) >> 13];
				realZ1=(int)GFX.Z1;realZ2=(int)GFX.Z2;
				if (BG.TileSize == 8) {
		    	if (!(Tile & H_FLIP)) {
						// Normal, unflipped
						pspDrawHiResClippedTile16 (Tile + (Quot & 1), Xt,Yt, 0, Count, VirtAlign, Lines);
		    	} else {
						// H flip
						pspDrawHiResClippedTile16 (Tile + 1 - (Quot & 1), Xt,Yt, 0, Count, VirtAlign, Lines);
		    	}
				} else {
		    	if (!(Tile & (V_FLIP | H_FLIP))) {
						// Normal, unflipped
						pspDrawHiResClippedTile16 (Tile + t1 + (Quot & 1), Xt,Yt, 0, Count, VirtAlign, Lines);
		    	} else if (Tile & H_FLIP) {
						if (Tile & V_FLIP) {
			    		// H & V flip
			    		pspDrawHiResClippedTile16 (Tile + t2 + 1 - (Quot & 1), Xt,Yt, 0, Count, VirtAlign, Lines);
						} else {
			    		// H flip only
			    		pspDrawHiResClippedTile16 (Tile + t1 + 1 - (Quot & 1), Xt,Yt, 0, Count, VirtAlign, Lines);
						}
		    	} else {
						// V flip only
						pspDrawHiResClippedTile16 (Tile + t2 + (Quot & 1), Xt,Yt, 0, Count, VirtAlign, Lines);
		    	}
				}
	    }
		}
  }
  GFX.Pitch = GFX.RealPitch;
  GFX.PPL = GFX.PPLx2 >> 1;
  
  //render
	if (vertices_ptr[current_bitshift]-vertices[current_bitshift]) {  	
  	
		sceGuTexImage(0,512,512,512,tile_texture[current_bitshift]);
  	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,(vertices_ptr[current_bitshift]-vertices[current_bitshift]),0,vertices[current_bitshift]);
	}
}
 

void pspDrawBackground (uint32 BGMode, uint32 bg, uint8 Z1, uint8 Z2) {		

  BG.TileSize = BGSizes [PPU.BG[bg].BGSize];
  BG.BitShift = BitShifts[BGMode][bg];  
  BG.TileShift = TileShifts[BGMode][bg];
  BG.TileAddress = PPU.BG[bg].NameBase << 1;
  BG.NameSelect = 0;
  current_bitshift=Depths [BGMode][bg];  
  BG.Buffer = tile_texture[current_bitshift];//IPPU.TileCache8 [Depths [BGMode][bg]];
  BG.Buffered = IPPU.TileCached [Depths [BGMode][bg]];
  BG.PaletteShift = PaletteShifts[BGMode][bg];
  BG.PaletteMask = PaletteMasks[BGMode][bg];
  BG.DirectColourMode = (BGMode == 3 || BGMode == 4) && bg == 0 && (GFX.r2130 & 1);
  
  if (BG.DirectColourMode) { 
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps (); 
    GFX.ScreenColors = DirectColourMaps[0];           	    	
  } 
  else GFX.ScreenColors = &IPPU.ScreenColors[0];

/********************************************************/  
/********************************************************/
  
/********************************************************/
/********************************************************/

  if (PPU.BGMosaic [bg] && PPU.Mosaic > 1){
#ifndef RELEASE  	
{char str[32];
sprintf(str,"BGMode : %d mosaic",BGMode);
info(32,10,str);	    	
}  	
#endif
  	
		pspDrawBackgroundMosaic (BGMode, bg, Z1, Z2);
		//debug_log("mosaic mode not supported");
		return;
  }
  switch (BGMode) {
    case 2: if (Settings.WrestlemaniaArcade) break;
    case 4: // Used by Puzzle Bobble
    	// Not used in psp hardware mode, since it's not really fit 
      //pspDrawBackgroundOffset (BGMode, bg, Z1, Z2);
			return;
    case 5:
    case 6: // XXX: is also offset per tile.				
#ifndef RELEASE    
{char str[32];
sprintf(str,"BGMode : %d mode5",BGMode);
info(32,10,str);	    	
}  	    
#endif
	    pspDrawBackgroundMode5 (BGMode, bg, Z1, Z2);
	    return;				
  }
#ifndef RELEASE  
{char str[32];
sprintf(str,"BGMode : %d",BGMode);
info(32,10,str);	    	
}  	  
#endif
  uint32 Tile;
  uint16 *SC0;
  uint16 *SC1;
  uint16 *SC2;
  uint16 *SC3;
  uint32 Width;
  uint8 depths [2] = {Z1, Z2};
  
  
  if (last_palette!=BG.DirectColourMode){  	 
			//DirectColourMode has changed, so we have to reload palette
			//waitRendering();
			last_palette=BG.DirectColourMode;
			clut = (u16*)NO_CPU_CACHE(&clut256[0]);
			memcpy(clut,GFX.ScreenColors,256*2);			
			if (rendering_beware_fix16)
			for (int i=0;i<256;i++) {
				if (clut[i]==fixedcol16) clut[i]^=1<<10; //swap blue 1st bit
			}
			clut[0]|=0x8000;
  		sceGuClutLoad(256/16,clut);			
	}
  //bg is 32x30 tile but needs more in case of clipping or scroll changing every line
	int num_vert=((GFX.EndY-GFX.StartY+1)+2)*(256/8+6);			
  if (!(vertices[current_bitshift]=(struct Vertex*)sceGuGetMemory(num_vert*2* sizeof(struct Vertex)))){
  	debug_log("not enough vertices!");
  	return;
  }
	vertices_ptr[current_bitshift]=vertices[current_bitshift];
	vertices_end[current_bitshift]=vertices_ptr[current_bitshift]+num_vert*2;
	
    
  if (BGMode == 0) BG.StartPalette = bg << 5;
  else BG.StartPalette = 0;
	                				
	SC0 = (uint16 *) &VRAM[PPU.BG[bg].SCBase << 1];

  if (PPU.BG[bg].SCSize & 1) SC1 = SC0 + 1024;
  else SC1 = SC0;
  
  if(SC1>=(unsigned short*)(VRAM+0x10000))		SC1=(uint16*)&VRAM[((uint8*)SC1-&VRAM[0])%0x10000];

  if (PPU.BG[bg].SCSize & 2) SC2 = SC1 + 1024;
  else SC2 = SC0;

  if (PPU.BG[bg].SCSize & 1) SC3 = SC2 + 1024;
	else SC3 = SC2;
	
	if((SC3-(unsigned short*)VRAM)>0x10000)		SC3-=0x10000;
    
  int Lines;
  int OffsetMask;
  int OffsetShift;
  bool BGTileSizeis8=BG.TileSize == 8;
  if (BG.TileSize == 16){
		OffsetMask = 0x3ff;
		OffsetShift = 4;
	}else{
		OffsetMask = 0x1ff;
		OffsetShift = 3;
  }

  for (uint32 Y = GFX.StartY; Y <= GFX.EndY; Y += Lines) {
		uint32 VOffset = LineData [Y].BG[bg].VOffset;
		uint32 HOffset = LineData [Y].BG[bg].HOffset;
		int VirtAlign = (Y + VOffset) & 7;
	
		for (Lines = 1; Lines < 8 - VirtAlign; Lines++)
	    if ((VOffset != LineData [Y + Lines].BG[bg].VOffset) ||	(HOffset != LineData [Y + Lines].BG[bg].HOffset)) break;

		if (Y + Lines > GFX.EndY) Lines = GFX.EndY + 1 - Y;

		uint32 ScreenLine = (VOffset + Y) >> OffsetShift;
		uint32 t1;
		uint32 t2;
		if (((VOffset + Y) & 15) > 7){
	    t1 = 16;
	    t2 = 0;
		}else{
	    t1 = 0;
	    t2 = 16;
		}
		uint16 *b1;
		uint16 *b2;

		if (ScreenLine & 0x20) b1 = SC2, b2 = SC3;
		else b1 = SC0, b2 = SC1;

		b1 += (ScreenLine & 0x1f) << 5;
		b2 += (ScreenLine & 0x1f) << 5;

		int clipcount = GFX.pCurrentClip->Count [bg];
		if (!clipcount) clipcount = 1;
		for (int clip = 0; clip < clipcount; clip++) {
	    uint32 Left;
	    uint32 Right;


	    if (!GFX.pCurrentClip->Count [bg]) {
				Left = 0;
				Right = 256;
	    } else {
				Left = GFX.pCurrentClip->Left [clip][bg];
				Right = GFX.pCurrentClip->Right [clip][bg];

				if (Right <= Left) continue;
	    }	    
#ifndef RELEASE	    
{char st[16];
	sprintf(st,"L%d-R%d",Left,Right);
info(32,1,st);
}	    	    
#endif	    	    
	    //uint32 sGP32 = Left  + Y*256;
	    s32 Xt=Left;
	    s32 Yt=Y;
	    uint32 HPos = (HOffset + Left) & OffsetMask;

	    uint32 Quot = HPos >> 3;
	    uint32 Count = 0;
	    
	    uint16 *t;
		
	    if (BGTileSizeis8) {
				if (Quot > 31) t = b2 + (Quot & 0x1f);
				else t = b1 + Quot;
	    } else {
				if (Quot > 63) t = b2 + ((Quot >> 1) & 0x1f);
				else t = b1 + (Quot >> 1);
	    }

	    Width = Right - Left;
	    // Left hand edge clipped tile
	    if (HPos & 7) {
				uint32 Offset = (HPos & 7);
				Count = 8 - Offset;
				if (Count > Width) Count = Width;
				
				Xt-=Offset;//sGP32 -= Offset ;
				Tile = READ_2BYTES(t);
				GFX.Z1 = GFX.Z2 = depths [(Tile & 0x2000) >> 13];
				realZ1=(int)GFX.Z1;realZ2=(int)GFX.Z2;
						
				if (BGTileSizeis8) pspDrawClippedTile16 (Tile, Xt,Yt, Offset, Count, VirtAlign, Lines);
				else {
					if (!(Tile & (V_FLIP | H_FLIP))) {
						// Normal, unflipped
						pspDrawClippedTile16(Tile + t1 + (Quot & 1),Xt,Yt, Offset, Count, VirtAlign, Lines);
			    } else if (Tile & H_FLIP) {
						if (Tile & V_FLIP) {
				    	// H & V flip
				    	pspDrawClippedTile16 (Tile + t2 + 1 - (Quot & 1),Xt,Yt, Offset, Count, VirtAlign, Lines);
						} else {
				    	// H flip only
				    	pspDrawClippedTile16 (Tile + t1 + 1 - (Quot & 1),Xt,Yt, Offset, Count, VirtAlign, Lines);
						}
			    } else {
						// V flip only
						pspDrawClippedTile16 (Tile + t2 + (Quot & 1), Xt,Yt,Offset, Count, VirtAlign, Lines);
					}
				}

				if (BG.TileSize == 8) {
		    	t++;
		    	if (Quot == 31) t = b2;
		    	else if (Quot == 63) t = b1;
				} else {
		    	t += Quot & 1;
		    	if (Quot == 63)t = b2;
		    	else if (Quot == 127)
					t = b1;
				}
				Quot++;
				//sGP32 += 8 ;
				Xt+=8;
	  	}
	    // Middle, unclipped tiles
	    Count = Width - Count;
	    int Middle = Count >> 3;//count/8
//char st[16];
//sprintf(st,"count=%d;middle=%d",Count,Middle);count=256;middle=32
//pgPrintBG(0,1,(31<<10)|(31<<5)|31,st);
	    Count &= 7;
	    for (int C = Middle; C > 0;/*sGP32 += 8*/Xt+=8 ,  Quot++, C--) {
				Tile = READ_2BYTES(t);
				GFX.Z1 = GFX.Z2 = depths [(Tile & 0x2000) >> 13];
				realZ1=(int)GFX.Z1;realZ2=(int)GFX.Z2;
				
				if (!BGTileSizeis8) {
		    	if (Tile & H_FLIP) {
						// Horizontal flip, but what about vertical flip ?
						if (Tile & V_FLIP) {
			    		// Both horzontal & vertical flip
			    		pspDrawTile16 (Tile + t2 + 1 - (Quot & 1), Xt,Yt,VirtAlign, Lines);
						} else {
			    		// Horizontal flip only
			    		pspDrawTile16 (Tile + t1 + 1 - (Quot & 1), Xt,Yt,VirtAlign, Lines);
						}
		    	} else {
						// No horizontal flip, but is there a vertical flip ?
						if (Tile & V_FLIP) {
			    		// Vertical flip only
			    		pspDrawTile16 (Tile + t2 + (Quot & 1), Xt,Yt,VirtAlign, Lines);
						} else {
			    		// Normal unflipped
			    		pspDrawTile16 (Tile + t1 + (Quot & 1), Xt,Yt,VirtAlign, Lines);
						}
		    	}
				} else {
		    	pspDrawTile16 (Tile, Xt,Yt, VirtAlign, Lines);		
				}
		
				if (BGTileSizeis8) {
		    		t++;
		    		if (Quot == 31) t = b2;
		    		else if (Quot == 63)
						t = b1;
				} else {
		    		t += Quot & 1;
		    		if (Quot == 63) t = b2;
		    		else if (Quot == 127)
						t = b1;
				}
	    }
	    // Right-hand edge clipped tiles
	    if (Count) {
	    	Tile = READ_2BYTES(t);
	    	GFX.Z1 = GFX.Z2 = depths [(Tile & 0x2000) >> 13];
	    	realZ1=(int)GFX.Z1;realZ2=(int)GFX.Z2;
				
				if (BGTileSizeis8) pspDrawClippedTile16 (Tile, Xt,Yt, 0, Count, VirtAlign, Lines);
				else {
		    	if (!(Tile & (V_FLIP | H_FLIP))) {
						// Normal, unflipped
						pspDrawClippedTile16 (Tile + t1 + (Quot & 1), Xt,Yt, 0, Count, VirtAlign, Lines);
		    	}else if (Tile & H_FLIP) {
						if (Tile & V_FLIP) {
			   			// H & V flip
			   			pspDrawClippedTile16 (Tile + t2 + 1 - (Quot & 1),Xt,Yt, 0, Count, VirtAlign, Lines);
						} else {
			   			// H flip only
			   			pspDrawClippedTile16 (Tile + t1 + 1 - (Quot & 1),Xt,Yt, 0, Count, VirtAlign, Lines);
						}
		    	} else {
						// V flip only
						pspDrawClippedTile16 (Tile + t2 + (Quot & 1),Xt,Yt, 0, Count, VirtAlign, Lines);
		    	}
				}
	    }
		}
  }
	//render
	if (vertices_ptr[current_bitshift]-vertices[current_bitshift]) {  	
  	
  	sceGuTexImage(0,512,512,512,tile_texture[current_bitshift]);  	
  	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,(vertices_ptr[current_bitshift]-vertices[current_bitshift]),0,vertices[current_bitshift]);
	}
}


	
	
//drawmode is : 0 => everything (for sub & main when no transp)
//     					1 => only what's added/subbed (for main)
//							2 => only what's not added/subbed (for main)
void pspRenderScreen (uint8 *Screen, bool8 sub, uint8 D, uint8 drawmode)
{	
#define TO_DRAW(a) \
	( (drawmode==0) || \
	    		 ((drawmode==1)&&SUB_OR_ADD(a)) ||\
	    		 ((drawmode==2)&&(!SUB_OR_ADD(a))) \
	    	 )	
	    	 
#define TO_DRAW_OBJ(a) \
	( (drawmode==0) || \
	    		 ((drawmode==1)&&SUB_OR_ADD(a)) ||\
	    		 (drawmode==2) \
	    	 )		    	 
	    	 
	bool8 BG0;
  bool8 BG1;
  bool8 BG2;
  bool8 BG3;
  bool8 OB;

  GFX.S = Screen;
    
  if (!sub) {
		GFX.pCurrentClip = &IPPU.Clip [0];
		BG0 = ON_MAIN (0);
		BG1 = ON_MAIN (1);
		BG2 = ON_MAIN (2);
		BG3 = ON_MAIN (3);
		OB  = ON_MAIN (4);
  } else {
		GFX.pCurrentClip = &IPPU.Clip [1];
		BG0 = ON_SUB (0);
		BG1 = ON_SUB (1);
		BG2 = ON_SUB (2);
		BG3 = ON_SUB (3);
		OB  = ON_SUB (4);
  }
  
  if (PPU.BGMode <= 1) {
		if (OB&&os9x_OBJ) {
			if (TO_DRAW_OBJ(4)) {
	    	if (os9x_fastsprite) pspDrawFASTOBJS (!sub, D,drawmode);
	    	else pspDrawOBJS (!sub, D,drawmode);
	  	}
		}
		if (BG0&&os9x_BG0) {	    
	    if (TO_DRAW(0)) {	    	
	    	pspDrawBackground (PPU.BGMode, 0, D + 10, D + 14);
	    }
		}
		if (BG1&&os9x_BG1) {			    
			if (TO_DRAW(1)) {				
				pspDrawBackground (PPU.BGMode, 1, D + 9, D + 13);
			}
		}
		if (BG2&&os9x_BG2) {	    
	    if (TO_DRAW(2)) {	    	
	    	pspDrawBackground (PPU.BGMode, 2, D + 3,  (FillRAM [0x2105] & 8) == 0 ? D + 6 : D + 17);
	    }
		}
		if (BG3 && PPU.BGMode == 0 && os9x_BG3) {	    
	    if (TO_DRAW(3)) {	    	
	    	pspDrawBackground (PPU.BGMode, 3, D + 2, D + 5);
	    }
		}
	} else if (PPU.BGMode != 7) {
		if (OB&&os9x_OBJ) {
			if (os9x_fastsprite) pspDrawFASTOBJS (!sub, D,drawmode);
	    	else pspDrawOBJS (!sub, D,drawmode);
		}
		if (BG0&&os9x_BG0) {	    
	    if (TO_DRAW(0)) {	    	
	    	pspDrawBackground (PPU.BGMode, 0, D + 5, D + 13);
	    }
		}
		if (PPU.BGMode != 6 && BG1 &&os9x_BG1) {	    
	    if (TO_DRAW(1)) {	    	
	    	pspDrawBackground (PPU.BGMode, 1, D + 2, D + 9);
	    }
		}
	} else {
    	return;
	/*if (OB &&os9x_OBJ)
	{
	    
	    //pspDrawOBJS (!sub, D);
	}
	if (BG0 || ((FillRAM [0x2133] & 0x40) && BG1) &&os9x_BG0)
	{
	    int bg;

	    if (FillRAM [0x2133] & 0x40)
	    {
		GFX.Mode7Mask = 0x7f;
		GFX.Mode7PriorityMask = 0x80;
		Mode7Depths [0] = 5 + D;
		Mode7Depths [1] = 9 + D;
		bg = 1;
	    }
	    else
	    {
		GFX.Mode7Mask = 0xff;
		GFX.Mode7PriorityMask = 0;
		Mode7Depths [0] = 5 + D;
		Mode7Depths [1] = 5 + D;
		bg = 0;
	    }
	    if (sub || !SUB_OR_ADD(0))
	    {

		    pspDrawBGMode7Background16 (Screen, bg);
	    }
	    else
	    {
		if (GFX.r2131 & 0x80)
		{
		    if (GFX.r2131 & 0x40)
		    {
			    pspDrawBGMode7Background16Sub1_2 (Screen, bg);
		    }
		    else
		    {
			    pspDrawBGMode7Background16Sub (Screen, bg);
		    }
		}
		else
		{
		    if (GFX.r2131 & 0x40)
		    {
			    pspDrawBGMode7Background16Add1_2 (Screen, bg);
		    }
		    else
		    {

			    pspDrawBGMode7Background16Add (Screen, bg);
		    }
		}
	    }
	}*/
    }
#undef TO_DRAW    
}



void pspS9xUpdateScreen (){
  int i;  
  
  render_timestamp++;

  /*GFX.S = GFX.Screen;
  GFX.r2131 = FillRAM [0x2131];
  GFX.r212c = FillRAM [0x212c];
  GFX.r212d = FillRAM [0x212d];
  GFX.r2130 = FillRAM [0x2130];
  GFX.Pseudo = (FillRAM [0x2133] & 8) != 0 &&
    (GFX.r212c & 15) != (GFX.r212d & 15) &&
    (GFX.r2131 & 0x3f) == 0;

  if (IPPU.OBJChanged)
    S9xSetupOBJ ();

  if (PPU.RecomputeClipWindows)
    {
      ComputeClipWindows ();
      PPU.RecomputeClipWindows = FALSE;
    }
*/

  GFX.StartY = IPPU.PreviousLine;
  if ((GFX.EndY = IPPU.CurrentLine - 1) >= PPU.ScreenHeight)
    GFX.EndY = PPU.ScreenHeight - 1;

  uint32 starty = GFX.StartY;
  uint32 endy = GFX.EndY;
    
  if (tile_cache_reset) tile_reset_cache();
  else tile_resetask=0;    
                
  if (GFX.Pseudo)	{
  	GFX.r2131 = 0x5f;
	  GFX.r212d = (FillRAM [0x212c] ^ FillRAM [0x212d]) & 15;
	  GFX.r212c &= ~GFX.r212d;
	  GFX.r2130 |= 2;
	}

  if (!os9x_easy&&!PPU.ForcedBlanking && ADD_OR_SUB_ON_ANYTHING && (GFX.r2130 & 0x30) != 0x30 &&
	  	!((GFX.r2130 & 0x30) == 0x10 && IPPU.Clip[1].Count[5] == 0)){	  		
	  fixedcol=((int)(IPPU.XB [PPU.FixedColourRed])<<3)|((int)(IPPU.XB [PPU.FixedColourGreen])<<11)|((int)(IPPU.XB [PPU.FixedColourBlue])<<19);	  	
	  fixedcol16=((int)(IPPU.XB [PPU.FixedColourRed])<<0)|((int)(IPPU.XB [PPU.FixedColourGreen])<<5)|((int)(IPPU.XB [PPU.FixedColourBlue])<<10);	  	
info(32,0,"transp");
		//get current add_sub_mode 
	  pspSelectAddSubMode ();
	  
	  int _2passblending,_fixaddsub;
	  if ((add_sub_mode==1)||(add_sub_mode==4)) _2passblending=1;
	  else _2passblending=0;	  	  
	  if ((add_sub_mode==2)||(add_sub_mode==5)) _fixaddsub=1;
	  else _fixaddsub=0;	  	  
	  
		if (_2passblending||_fixaddsub) rendering_beware_fix16=1;
		//start a new rendering pass
		initRendering();
				
		//set subscreen as drawing buffer
		sceGuDrawBufferList(GU_PSM_5551,(void*)(512*272*2*2+256*240*2+0*256*256*2),256);		
		sceGuTexMode(GU_PSM_T8,0,0,0); //8bit texture
		
		//clear the ZBuffer
		sceGuScissor(0,starty,256,endy/*-starty*/+1);
		sceGuClearDepth(0);
		sceGuClear(GU_DEPTH_BUFFER_BIT);
																						  		
		//subscreen colour window handling
	  if ( IPPU.Clip [1].Count [5]) {
	  	// Colour window enabled on subscreen	  	

info(32,1,"color win sub   ");
			// first we clear it with 0
			// if 2 pass blending is need, 0 will be used to detect fixed colour
			// so the "normal black", 0, is replaced by 1 (dark blue)
			if (rendering_beware_fix16) {
				if (fixedcol==0) sceGuClearColor(1<<19);
				else sceGuClearColor(0);
			} else sceGuClearColor(0);
			
	   	sceGuClear(GU_COLOR_BUFFER_BIT);
	   		   	
	   	// then with FixedColour where it will be rendered
	   	sceGuClearColor(fixedcol);
  		for (uint32 c = 0; c < IPPU.Clip [1].Count [5]; c++) {
  			//char st[64];sprintf(st,"l %d r %d",IPPU.Clip [1].Left [c][5],IPPU.Clip [1].Right [c][5]);
  			//info(32,15+c,st);
  			if (IPPU.Clip [1].Right [c][5] > IPPU.Clip [1].Left [c][5]){
			 		//current rendering area
	    		sceGuScissor(IPPU.Clip [1].Left [c][5],starty,(IPPU.Clip [1].Right [c][5]/*-IPPU.Clip [1].Left [c][5]*/)+1,endy/*-starty*/+1);
     			sceGuClear(GU_COLOR_BUFFER_BIT);
     		}
     	}
     	sceGuScissor(0,starty,256,endy/*-starty*/+1);
    } else {
    	// No colour window enabled on subscreen
info(32,1,"no color win sub");
    	// we clear it with FixedColour    				
    	sceGuClearColor(fixedcol);
	   	sceGuClear(GU_COLOR_BUFFER_BIT);
	  }
	  	  	  			   
	  if (ANYTHING_ON_SUB)  {
info(32,2,"rendering subscreen ");								
	      pspRenderScreen (GFX.SubScreen, true, SUB_SCREEN_DEPTH,0);
	  } else {
info(32,2,"nothing on subscreen");
		}							
		
	  //now main screen	  	  
	  sceGuDrawBufferList(GU_PSM_5551,(void*)(512*272*2*2+256*240*2+1*256*256*2),256);	  
	  	  
	  //handle main screen colour window
	  
		if (IPPU.Clip [0].Count[5]) {
			// Colour window enabled on mainscreen
			int col=(((IPPU.ScreenColors [0])&31)<<3)|(((IPPU.ScreenColors [0]>>5)&31)<<11)|(((IPPU.ScreenColors [0]>>10)&31)<<19);			
info(32,3,"color win main   ");
			// first we clear it with 0 everywhere
			if (rendering_beware_fix16) {
				if (fixedcol==0) sceGuClearColor(1<<19);
				else sceGuClearColor(0);
			} else sceGuClearColor(0);
			
	   	sceGuClear(GU_COLOR_BUFFER_BIT);	   	
	   	// and back color inside rendered area
	   		   	
	   	if (rendering_beware_fix16) {
				if (fixedcol==IPPU.ScreenColors [0]) col^=1<<19;
			} 
			sceGuClearColor(col);
  		for (uint32 c = 0; c < IPPU.Clip [0].Count [5]; c++) {
  			if (IPPU.Clip [0].Right [c][5] > IPPU.Clip [0].Left [c][5]){
		  		//current rendering area
	     		sceGuScissor(IPPU.Clip [0].Left [c][5],starty,(IPPU.Clip [0].Right [c][5]/*-IPPU.Clip [0].Left [c][5]*/)+1,endy/*-starty*/+1);
     			sceGuClear(GU_COLOR_BUFFER_BIT);
     		}
     	}
     	sceGuScissor(0,starty,256,endy/*-starty*/+1);    
	   		   		   	
	   	// perhaps we should only do that when subscreen & mainscreen colour window aren't overlapping
	   	// then we copy subscreen where there are pixels
	   	if (ANYTHING_ON_SUB){
	   		sceGuTexMode(GU_PSM_5551,0,0,0); //16bit texture
	  		sceGuDisable(GU_ALPHA_TEST);	  			  		
	  		sceGuDepthFunc(GU_LESS);
				sceGuDepthMask(GU_TRUE);				
				sceGuTexImage(0,256,256,256,(u8*)(0x44000000+512*272*2*2+256*240*2+0*256*256*2));				
				_vertices = (struct Vertex*)sceGuGetMemory(4*2 * sizeof(struct Vertex));
    		_vertices_ptr=_vertices;
    		for (i=0;i<256;i+=64){
    			_vertices_ptr->u = i; _vertices_ptr->v = starty;
					_vertices_ptr->x = i; _vertices_ptr->y = starty;_vertices_ptr->z = 0;
					_vertices_ptr++;
					_vertices_ptr->u = i+64; _vertices_ptr->v = endy+1;
					_vertices_ptr->x = i+64; _vertices_ptr->y = endy+1;_vertices_ptr->z = 0;
					_vertices_ptr++;
				}    		
				sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2*4,0,_vertices);												
	 			sceGuEnable(GU_ALPHA_TEST);	 				 			
	 			sceGuDepthFunc(GU_GREATER);
				sceGuDepthMask(GU_FALSE);
				sceGuTexMode(GU_PSM_T8,0,0,0); //8bit texture
	   	}
	   	
	   	// we set ZBuffer to 0
	   	// so further blending won't update others pixels (outside colour window)	   		   	
			sceGuClearDepth(0);			
			sceGuClear(GU_DEPTH_BUFFER_BIT);
			// and 1 to updatable area
			sceGuClearDepth(1);			
  		for (uint32 c = 0; c < IPPU.Clip [0].Count [5]; c++) {
  			if (IPPU.Clip [0].Right [c][5] > IPPU.Clip [0].Left [c][5]){
		  		//current rendering area
	     		sceGuScissor(IPPU.Clip [0].Left [c][5],starty,(IPPU.Clip [0].Right [c][5]/*-IPPU.Clip [0].Left [c][5]*/)+1,endy/*-starty*/+1);
     			sceGuClear(GU_DEPTH_BUFFER_BIT);
     		}
     	}
     	// and back to whole  rendering area
     	sceGuScissor(0,starty,256,endy/*-starty*/+1);
    } else {
    	// Colour window disabled on mainscreen
info(32,3,"no color win main");
			// we clear it with ScreenColors[0]
	   	sceGuClearColor((((IPPU.ScreenColors [0])&31)<<3)|(((IPPU.ScreenColors [0]>>5)&31)<<11)|(((IPPU.ScreenColors [0]>>10)&31)<<19));	   	
	   	//clear Z buffer, TODO : check if Zbuffer clear is needed each time (ADD_OR_SUB(5) only since no colour window ?)
	  	sceGuClearDepth(1);//MAIN_SCREEN_DEPTH);//0??? seiken3 dialog
	   	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	  }
						
		// now first pass main screen (with BG/OBJ to be combined with subscreen/fixed colour
		//invalidate last palette
				
	  pspRenderScreen (GFX.Screen, false, SUB_SCREEN_DEPTH,1);
	  	  	  	  
	  // now draw on final buffer 	  	 	
	  // if rendering mode == 0, then draw directly on final screen
	  
	  sceGuDrawBufferList(GU_PSM_5551,(void*)(512*272*2*2+256*240*2+2*256*256*2),256);	  
	  
	  	  	  	  	  	  	  		
		// first, simple copy of first pass main screen
		sceGuTexMode(GU_PSM_5551,0,0,0); //16bit texture
		sceGuTexImage(0,256,256,256,(u16*)(0x44000000+512*272*2*2+256*240*2+1*256*256*2));				
		sceGuDisable(GU_DEPTH_TEST);
		sceGuDisable(GU_ALPHA_TEST);
	  
	  int zmin;
		if ((!IPPU.Clip [0].Count[5])&&SUB_OR_ADD(5)){
			//no colour window on mainscreen
			//and sub_or_add back colour as well
			//so no need to filter subscreen, the whole will be blended
	  	//sceGuDepthFunc(GU_ALWAYS);	  	  	
	  	zmin=0;
	  } else {
			// colour window on mainscreen or not sub_or_add back colour
			// we have to filter rendering using ZBuffer
			// but no update has to be done in ZBuffer in order to not corrupt the second pass of
			// main screen rendering	
			if (SUB_OR_ADD(5)) zmin=0; //depth of back is 1
			else zmin=1;			
	  }
	  
	  
		_vertices = (struct Vertex*)sceGuGetMemory(4*2 * sizeof(struct Vertex));
    _vertices_ptr=_vertices;
    for (i=0;i<256;i+=64){
    	_vertices_ptr->u = i; _vertices_ptr->v = starty;
		_vertices_ptr->x = i; _vertices_ptr->y = starty;_vertices_ptr->z = zmin;
		_vertices_ptr++;
		_vertices_ptr->u = (i+64); _vertices_ptr->v = endy+1;
		_vertices_ptr->x = (i+64); _vertices_ptr->y = endy+1;_vertices_ptr->z = zmin;
		_vertices_ptr++;
		}		    
		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2*4,0,_vertices);
#ifndef RELEASE
		if (SUB_OR_ADD(5)) {
info(32,5,"sub added to back    ");	  
    } else {
info(32,5,"sub not added to back");
	  }
#endif  
	  
		
		//load subscreen texture
		sceGuTexImage(0,256,256,256,(u16*)(0x44000000+512*272*2*2+256*240*2+0*256*256*2));
		
		sceGuEnable(GU_DEPTH_TEST);
		sceGuDepthMask(GU_TRUE);				
		// now we'll add subscreen/fixedcolour
		
		if ((!IPPU.Clip [0].Count[5])&&SUB_OR_ADD(5)){
			//no colour window on mainscreen
			//and sub_or_add back colour as well
			//so no need to filter subscreen, the whole will be blended
	  	//sceGuDepthFunc(GU_ALWAYS);
	  	sceGuDisable(GU_DEPTH_TEST);
		} else {
			// colour window on mainscreen or not sub_or_add back colour
			// we have to filter rendering using ZBuffer
			// but no update has to be done in ZBuffer in order to not corrupt the second pass of
			// main screen rendering									
	  	sceGuDepthFunc(GU_LESS);
	  }
										
		// setup blend mode
		
		   			
		if (_2passblending) {
			
			//char st[32];sprintf(st,"%08X",fixedcol);info(32,20,st);
			
			sceGuEnable(GU_BLEND);			
			if (add_sub_mode==1) sceGuBlendFunc(2,GU_FIX,GU_FIX,0xffffff,0xffffff);
			else sceGuBlendFunc(0,GU_FIX,GU_FIX,0xffffff,0xffffff);										
			
			sceGuEnable(GU_COLOR_TEST); //color test
			sceGuColorFunc(GU_EQUAL,fixedcol,(0x1F<<3)|(0x1F<<11)|(0x1F<<19));
			sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2*4,0,_vertices);
			sceGuColorFunc(GU_NOTEQUAL,fixedcol,(0x1F<<3)|(0x1F<<11)|(0x1F<<19));
			
		} else sceGuEnable(GU_BLEND);
		//detect blending type		
		switch (add_sub_mode){
			case 0:info(32,4,"oops!!");break;
			case 1://sub1_2
			info(32,4,"sub1_2");
				sceGuBlendFunc(2,GU_FIX,GU_FIX,0x7f7f7f,0x7f7f7f); break;
			case 2://fix sub
			info(32,4,"subfix");
				sceGuBlendFunc(2,GU_FIX,GU_FIX,0x7f7f7f,0x7f7f7f); break;
			case 3://sub
			info(32,4,"sub***");
				sceGuBlendFunc(2,GU_FIX,GU_FIX,0xffffff,0xffffff); break;
			case 4://add1_2
			info(32,4,"add1_2");
				sceGuBlendFunc(0,GU_FIX,GU_FIX,0x7f7f7f,0x7f7f7f); break;				
			case 5://fix add
			info(32,4,"addfix");
				sceGuBlendFunc(0,GU_FIX,GU_FIX,0x7f7f7f,0x7f7f7f); break;				
			case 6://add
			info(32,4,"add***");
				sceGuBlendFunc(0,GU_FIX,GU_FIX,0xffffff,0xffffff); break;			
		}
							   	
		if (_fixaddsub) {							 
			sceGuEnable(GU_COLOR_TEST); //color test
			sceGuColorFunc(GU_NOTEQUAL,0,0xFFFFFF);
			sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2*4,0,_vertices);		
			
		} else {
			
			if (IPPU.Clip [1].Count [5]) {
				//col win sub, so not apply blending everywhere
  				for (uint32 c = 0; c < IPPU.Clip [1].Count [5]; c++) {
  					if (IPPU.Clip [1].Right [c][5] > IPPU.Clip [1].Left [c][5]){
			 				//current rendering area
	    				sceGuScissor(IPPU.Clip [1].Left [c][5],starty,(IPPU.Clip [1].Right [c][5]/*-IPPU.Clip [1].Left [c][5]*/)+1,endy/*-starty*/+1);
     					sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2*4,0,_vertices);
     				}
     			}
     			sceGuScissor(0,starty,256,endy/*-starty*/+1);
     		} else {
     			sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2*4,0,_vertices);												
     		}
		}
 	
		sceGuDisable(GU_BLEND);
		if (_fixaddsub||_2passblending) {
	 		sceGuDisable(GU_COLOR_TEST);
	 	}
	 	

		sceGuTexMode(GU_PSM_T8,0,0,0);	//8bit texture
		sceGuEnable(GU_ALPHA_TEST);
		
		
		sceGuEnable(GU_DEPTH_TEST);
		sceGuDepthFunc(GU_GREATER);
		sceGuDepthMask(GU_FALSE);
  		// and now second pass for mainscreen (OBJs/BGs not combined with subscreen)
  		//invalidate last palette	  
  	
  		rendering_beware_fix16=0;
		pspRenderScreen (GFX.Screen, false,SUB_SCREEN_DEPTH,2);						

		endRendering();	  
	} else {
		info(32,0,"notransp");
	  		
	  // 16bit and transparency but currently no transparency effects in
	  // operation.
	  
	  
	  if (!PPU.ForcedBlanking) {	  		  	
	  	
		rendering_beware_fix16=0;	  	
		initRendering();

	  	
	  	sceGuDrawBufferList(GU_PSM_5551,(void*)(0x40000000+512*272*2*2+256*240*2+2*256*256*2),256);	  		  	
	  	sceGuTexMode(GU_PSM_T8,0,0,0); //8bit texture
	  								  		  		  	
	  	//setup back colors
	  	sceGuClearColor((((IPPU.ScreenColors [0]>>0)&31)<<3)|(((IPPU.ScreenColors [0]>>5)&31)<<11)|(((IPPU.ScreenColors [0]>>10)&31)<<19));	  	
	  	if (IPPU.Clip [0].Count[5]) {
	  		for (uint32 c = 0; c < IPPU.Clip [0].Count [5]; c++) {
	  			if (IPPU.Clip [0].Right [c][5] > IPPU.Clip [0].Left [c][5]){
		     		sceGuScissor(IPPU.Clip [0].Left [c][5],starty,(IPPU.Clip [0].Right [c][5]/*-IPPU.Clip [0].Left [c][5]*/)+1,endy/*-starty*/+1);
	     			sceGuClear(GU_COLOR_BUFFER_BIT);	     			
	     		}
	     	}
	     	sceGuScissor(0,starty,256,endy/*-starty*/+1);
	     	sceGuClearDepth(0);
	     	sceGuClear(GU_DEPTH_BUFFER_BIT);
	    } else {
	    	sceGuScissor(0,starty,256,endy/*-starty*/+1);
	    	sceGuClearDepth(0);
		   	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
		  }
		  
		  if (os9x_easy) {		  			  			  			  			  	
		  	pspRenderScreen (GFX.Screen, true, SUB_SCREEN_DEPTH,0);		  	
		  	pspRenderScreen (GFX.Screen, false, MAIN_SCREEN_DEPTH,0);
		  } else	{		  	
		  	pspRenderScreen (GFX.Screen, false, SUB_SCREEN_DEPTH,0);
		  }		  		  	  			  
		  endRendering();
		}
	}	
	

  IPPU.PreviousLine = IPPU.CurrentLine;
}
