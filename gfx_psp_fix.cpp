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

extern int add_sub_mode;
extern int last_palette;
extern int current_bitshift;
extern int render_timestamp;

//extern u16 __attribute__((aligned(16))) clut256[256*3];
extern u16* clut256;
extern u16* clut;
extern u32 fixedcol16;
extern u32 fixedcol;
extern int rendering_beware_fix16;

// for OBJ & BGS
extern struct Vertex *vertices[3];
extern struct Vertex *vertices_ptr[3];
extern struct Vertex *vertices_end[3];
// for Blending
extern struct Vertex *_vertices;
extern struct Vertex *_vertices_ptr;

extern struct VertexCol *_verticesCol;
extern struct VertexCol *_verticesCol_ptr;
extern struct VertexCol *_verticesCol_end;

//s32 realZ1,realZ2;

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

ClipDataFix* pCurrentClipFix;

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

#define ANYTHING_ON_SUB \
((GPUPack.GFX.r2130 & 0x30) != 0x30 && \
 (GPUPack.GFX.r2130 & 2) && \
 (GPUPack.GFX.r212d & 0x1f))

#define ADD_OR_SUB_ON_ANYTHING \
(GPUPack.GFX.r2131 & 0x3f)

#define BLACK BUILD_PIXEL(0,0,0)


void pspDrawTile16 (uint32 Tile, s32 x,s32 y, uint32 StartLine,
	         uint32 LineCount,short realZ2);
void pspDrawClippedTile16 (uint32 Tile, s32 x,s32 y,
		        uint32 StartPixel, uint32 Width,
		        uint32 StartLine, uint32 LineCount,short realZ2);
void pspDrawHiResTile16 (uint32 Tile, s32 x,s32 y, uint32 StartLine,
	         uint32 LineCount,short realZ2);
void pspDrawHiResClippedTile16 (uint32 Tile, s32 x,s32 y,
		        uint32 StartPixel, uint32 Width,
		        uint32 StartLine, uint32 LineCount,short realZ2);		        
		        
void pspDrawLargePixel16 (uint32 Tile, s32 x,s32 y,
		       uint32 StartPixel, uint32 Pixels,
		       uint32 StartLine, uint32 LineCount,short realZ2);



inline void pspSelectAddSubMode()  {		
	if (GPUPack.GFX.r2131 & 0x80) {
		if (GPUPack.GFX.r2131 & 0x40) {
			if (GPUPack.GFX.r2130 & 2) {
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
		if (GPUPack.GFX.r2131 & 0x40) {
			if (GPUPack.GFX.r2130 & 2) {
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



#define pspDrawTile16_order pspDrawTile16
#define pspDrawClippedTile16_order pspDrawClippedTile16

void initRenderingFix(){
	sceGuStart(GU_DIRECT,list);
	//rendering options
				
	sceGuEnable(GU_DEPTH_TEST);
	sceGuDepthFunc(GU_GREATER);
		
	sceGuEnable(GU_ALPHA_TEST);			
	sceGuAlphaFunc(GU_EQUAL,0,0x1);

	sceGuEnable(GU_TEXTURE_2D);	
	sceGuTexFilter(GU_NEAREST,GU_NEAREST);
	
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
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
void pspDrawOBJS (bool8 OnMain, uint8 D, uint8 drawmode);
void endRenderingFix(){
	sceGuFinish();
	sceGuSync(0,0);
}

void waitRenderingFix() {
	
	/*sceGuFinish();
	sceGuSync(0,0);
	sceGuStart(GU_DIRECT,list);
	*/
}

void pspDrawFASTOBJSFix (bool8 OnMain, uint8 D, uint8 drawmode)
{

  //uint32 O;
  s32 Xs,Ys;
  int i;
  uint32 BaseTile, Tile;
    
  GPUPack.BG.BitShift = 4;  
  GPUPack.BG.TileShift = 5;
  GPUPack.BG.TileAddress = PPU.OBJNameBase;
  GPUPack.BG.StartPalette = 128;
  GPUPack.BG.PaletteShift = 4;
  GPUPack.BG.PaletteMask = 7;  
  current_bitshift=TILE_4BIT;
  GPUPack.BG.Buffer = tile_texture[TILE_4BIT];//IPPU.TileCache8 [TILE_4BIT];
  GPUPack.BG.Buffered = IPPU.TileCached [TILE_4BIT];
  GPUPack.BG.NameSelect = PPU.OBJNameSelect;
  GPUPack.BG.DirectColourMode = false;
        
/********************************************************/	
/****************PSP STUFF*******************************/    
/********************************************************/			
		
	int num_vert=0;//8192;//0;
	int I=0;
	 for (int S = GPUPack.GFX.OBJList [I++]; S >= 0; S = GPUPack.GFX.OBJList [I++]) {
	 	int VPos = GPUPack.GFX.VPositions [S];
			int Size = GPUPack.GFX.Sizes[S];
			int TileInc = 1;
			int Offset;
			if (VPos + Size <= (int) GPUPack.GFX.StartY || VPos > (int) GPUPack.GFX.EndY)  continue;
			
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
    GPUPack.GFX.Z1 = (D + 2);
    //realZ1=(int)(GPUPack.GFX.Z1);
		I=0;
    for (int S = GPUPack.GFX.OBJList [I++]; S >= 0; S = GPUPack.GFX.OBJList [I++]) {
			int VPos = GPUPack.GFX.VPositions [S];
			int Size = GPUPack.GFX.Sizes[S];
			int TileInc = 1;
			int Offset;

			if (VPos + Size <= (int) GPUPack.GFX.StartY || VPos > (int) GPUPack.GFX.EndY)
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

			int clipcount = GPUPack.GFX.pCurrentClip->Count [4];
			if (!clipcount) clipcount = 1;
			
			GPUPack.GFX.Z2 = ((PPU.OBJ[S].Priority + 1) * 4 + D);
			short realZ2=(short)(GPUPack.GFX.Z2);//+I-1;
	
			for (int clip = 0; clip < clipcount; clip++){
		    int Left; 
		    int Right;
		    if (!GPUPack.GFX.pCurrentClip->Count [4]){
					Left = 0;
					Right = 256;
		    }else{
					Left = GPUPack.GFX.pCurrentClip->Left [clip][4];
					Right = GPUPack.GFX.pCurrentClip->Right [clip][4];
	    	}

		    if (Right <= Left || PPU.OBJ[S].HPos + Size <= Left || PPU.OBJ[S].HPos >= Right) continue;
		    
	

		    for (int Y = 0; Y < Size; Y += 8) {
					if (VPos + Y + 7 >= (int) GPUPack.GFX.StartY && VPos + Y <= (int) GPUPack.GFX.EndY){
				    int StartLine;
				    int TileLine;
				    int LineCount;
				    int Last;
			    			    
				    if ((StartLine = VPos + Y) < (int) GPUPack.GFX.StartY) {
							StartLine = GPUPack.GFX.StartY - StartLine;
							LineCount = 8 - StartLine;
				    }else{
							StartLine = 0;
							LineCount = 8;
				    }
				    if ((Last = VPos + Y + 7 - GPUPack.GFX.EndY) > 0)
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
					    		
					    		pspDrawClippedTile16 (Tile, Xs,Ys, Offset, W,TileLine, LineCount,realZ2);

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
								pspDrawTile16 (Tile, Xs,Ys, TileLine, LineCount,realZ2);
				    	}
				    	if (Offset){
								
								pspDrawClippedTile16 (Tile, Xs,Ys, 0, Offset,TileLine, LineCount,realZ2);
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


void pspDrawOBJSFix (bool8 OnMain, uint8 D, uint8 drawmode) {
  //uint32 O;
  s32 Xs,Ys;
  int speZ;
  uint32 BaseTile, Tile;
    
  GPUPack.BG.BitShift = 4;
  current_bitshift=TILE_4BIT;
  GPUPack.BG.TileShift = 5;
  GPUPack.BG.TileAddress = PPU.OBJNameBase;
  GPUPack.BG.StartPalette = 128;
  GPUPack.BG.PaletteShift = 4;
  GPUPack.BG.PaletteMask = 7;
  GPUPack.BG.Buffer = tile_texture[TILE_4BIT];//IPPU.TileCache8 [TILE_4BIT];
  GPUPack.BG.Buffered = IPPU.TileCached [TILE_4BIT];
  GPUPack.BG.NameSelect = PPU.OBJNameSelect;
  GPUPack.BG.DirectColourMode = false;
        
/********************************************************/	
/****************PSP STUFF*******************************/    
/********************************************************/		
			
	int num_vert=0;//8192;//0;
	int I=0;
	 for (int S = GPUPack.GFX.OBJList [I++]; S >= 0; S = GPUPack.GFX.OBJList [I++]) {
	 	int VPos = GPUPack.GFX.VPositions [S];
			int Size = GPUPack.GFX.Sizes[S];
			if (VPos + Size <= (int) GPUPack.GFX.StartY || VPos > (int) GPUPack.GFX.EndY)
		    continue;
			//drawing sub/added sprites => palette has to be 4-7			
			num_vert+=(Size/8)*(Size/8);
	}
	num_vert += 33*pCurrentClipFix->GroupCount [4];
	
  if (!(vertices[current_bitshift]=(struct Vertex*)sceGuGetMemory(num_vert*2* sizeof(struct Vertex)))){
  	debug_log("not enough vertices!");
  	return;
  }
	vertices_ptr[current_bitshift]=vertices[current_bitshift];
	vertices_end[current_bitshift]=vertices_ptr[current_bitshift]+num_vert*2;
							  				  				
/********************************************************/
/********************************************************/				
    GPUPack.GFX.Z1 = (D + 2);
    //realZ1=(int)(GPUPack.GFX.Z1);
		I=0;
    for (int S = GPUPack.GFX.OBJList [I++]; S >= 0; S = GPUPack.GFX.OBJList [I++]) {
		int VPos = GPUPack.GFX.VPositions [S];
		int Size = GPUPack.GFX.Sizes[S];
		int TileInc = 1;
		int Offset;

		for (uint32 g = 0; g < pCurrentClipFix->GroupCount [4]; g++) 
		{
			if (VPos + Size <= (int) pCurrentClipFix->Start[4][g] || VPos > (int) pCurrentClipFix->End[4][g])
				continue;

			//drawing sub/added sprites => palette has to be 4-7
			speZ=0;
			if (OnMain)
			{
				if ((drawmode==1) && (PPU.OBJ [S].Palette < 4) ) speZ=1;
				//drawing not sub/added sprites => palette has to be 4-7 or no add/sub in effect for OBJ
				else if ((drawmode==2) && (PPU.OBJ [S].Palette >= 4) && SUB_OR_ADD(4)) speZ=1;									
			} 
			
			
			BaseTile = PPU.OBJ[S].Name | (PPU.OBJ[S].Palette << 10);

			if (PPU.OBJ[S].HFlip)
			{
    			BaseTile += ((Size >> 3) - 1) | H_FLIP;
				TileInc = -1;
			}
			if (PPU.OBJ[S].VFlip)  BaseTile |= V_FLIP;
		
			int clipcount = pCurrentClipFix->Count [4][g];
			if (!clipcount) clipcount = 1;
		
			GPUPack.GFX.Z2 = ((PPU.OBJ[S].Priority + 1) * 4 + D);
			short realZ2;
			if (!speZ) realZ2=(short)(GPUPack.GFX.Z2);//+I-1;
			else realZ2=0;
									
	
			for (int clip = 0; clip < clipcount; clip++)
			{
				int Left; 
				int Right;
				if(pCurrentClipFix->Count [4][g]==0)
				{
					Left = 0;
					Right = 256;
				}else
				{
					Left = pCurrentClipFix->Left [clip][4][g];
					Right = pCurrentClipFix->Right [clip][4][g];
    			}

				if (Right <= Left || PPU.OBJ[S].HPos + Size <= Left || PPU.OBJ[S].HPos >= Right) continue;
	    
				for (int Y = 0; Y < Size; Y += 8) 
				{
					if (VPos + Y + 7 >= (int) pCurrentClipFix->Start[4][g] && VPos + Y <= (int) pCurrentClipFix->End[4][g]) 
					{
						int StartLine;
						int TileLine;
						int LineCount;
						int Last;
				    			    
						if ((StartLine = VPos + Y) < (int) pCurrentClipFix->Start[4][g])
						{
							StartLine = pCurrentClipFix->Start[4][g] - StartLine;
							LineCount = 8 - StartLine;
						} else 
						{
							StartLine = 0;
							LineCount = 8;
						}
						if ((Last = VPos + Y + 7 - pCurrentClipFix->End[4][g]) > 0)
							if ((LineCount -= Last) <= 0) break;

						TileLine = StartLine;// << 3;
						Ys = (VPos + Y + StartLine);
						if (!PPU.OBJ[S].VFlip) Tile = BaseTile + (Y << 1);
						else Tile = BaseTile + ((Size - Y - 8) << 1);

						int Middle = Size >> 3;
						if (PPU.OBJ[S].HPos < Left)
						{
							Tile += ((Left - PPU.OBJ[S].HPos) >> 3) * TileInc;
							Middle -= (Left - PPU.OBJ[S].HPos) >> 3;
							Xs = Left ;
							if ((Offset = (Left - PPU.OBJ[S].HPos) & 7)) 
							{
				   				Xs -= Offset ;
				   				int W = 8 - Offset;
				   				int Width = Right - Left;
				   				if (W > Width) W = Width;
						   		
			   					pspDrawClippedTile16 (Tile, Xs,Ys, Offset, W,TileLine, LineCount,realZ2);
						   		
	    						if (W >= Width) continue;
				   				Tile += TileInc;
				   				Middle--;
				   				Xs += 8 ;
							}	
	    				}
						else Xs = PPU.OBJ[S].HPos ;

						if (PPU.OBJ[S].HPos + Size >= Right) 
						{
							Middle -= ((PPU.OBJ[S].HPos + Size + 7) - Right) >> 3;
							Offset = (Right - (PPU.OBJ[S].HPos + Size)) & 7;
						}
						else Offset = 0;

						for (int X = 0; X < Middle; X++, Xs += 8 , Tile += TileInc)
						{
							pspDrawTile16 (Tile, Xs,Ys, TileLine, LineCount,realZ2);
						}
						if (Offset)
						{
							pspDrawClippedTile16 (Tile, Xs,Ys, 0, Offset,TileLine, LineCount,realZ2);
						}
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
    
	if (vertices_ptr[current_bitshift]-vertices[current_bitshift])
	{  	  	  			
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


void pspDrawBackgroundMosaicFix (uint32 BGMode, uint32 bg, uint8 Z1, uint8 Z2) {    
	uint32 Tile;
  uint16 *SC0;
  uint16 *SC1;
  uint16 *SC2;
  uint16 *SC3;
  uint8 depths [2] = {Z1, Z2};
  
  if (last_palette!=GPUPack.BG.DirectColourMode){  	 
			//DirectColourMode has changed, so we have to reload palette
			waitRenderingFix();
			last_palette=GPUPack.BG.DirectColourMode;
			clut = (u16*)NO_CPU_CACHE(&clut256[0]);
			memcpy(clut,GPUPack.GFX.ScreenColors,256*2);			
			if (rendering_beware_fix16)
			for (int i=0;i<256;i++) {
				if (clut[i]==fixedcol16) clut[i]^=1<<10; //swap blue 1st bit
			}
			clut[0]|=0x8000;
  		sceGuClutLoad(256/16,clut);			
	}
  //bg is 32x30 tile but needs more in case of clipping or scroll changing every line
	int num_vert=((GPUPack.GFX.EndY-GPUPack.GFX.StartY+1)+2+pCurrentClipFix->GroupCount [bg])*(256/8+6);		
  if (!(_verticesCol=(struct VertexCol*)sceGuGetMemory(num_vert*2* sizeof(struct VertexCol)))){
  	debug_log("not enough vertices!");
  	return;
  }
	_verticesCol_ptr=_verticesCol;
	_verticesCol_end=_verticesCol_ptr+num_vert*2;
/***********************************************************/
/***********************************************************/
    
  if (BGMode == 0) GPUPack.BG.StartPalette = bg << 5;
  else GPUPack.BG.StartPalette = 0;
    
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

  if (GPUPack.BG.TileSize == 16) {
		OffsetMask = 0x3ff;
		OffsetShift = 4;
  } else {
		OffsetMask = 0x1ff;
		OffsetShift = 3;
  }

	uint32 Y = GPUPack.GFX.StartY;
	for (uint32 g = 0; g < pCurrentClipFix->GroupCount [bg]; g++) 
	{		
		Y=pCurrentClipFix->Start [bg][g];

 		for (; Y <= GPUPack.GFX.EndY && Y <= pCurrentClipFix->End [bg][g]; Y += Lines)
		{
			uint32 VOffset = LineData [Y].BG[bg].VOffset;
			uint32 HOffset = LineData [Y].BG[bg].HOffset;
			uint32 MosaicOffset = Y % PPU.Mosaic;

			for (Lines = 1; Lines < PPU.Mosaic - MosaicOffset; Lines++)
			if ((VOffset != LineData [Y + Lines].BG[bg].VOffset) || (HOffset != LineData [Y + Lines].BG[bg].HOffset)) break;
		
			uint32 MosaicLine = VOffset + Y - MosaicOffset;

			if (Y + Lines > GPUPack.GFX.EndY) Lines = GPUPack.GFX.EndY + 1 - Y;
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

			uint32 ClipCount = pCurrentClipFix->Count [bg][g];
			uint32 HPos = HOffset;
			uint32 PixWidth = PPU.Mosaic;

			if (!ClipCount) ClipCount = 1;

			for (uint32 clip = 0; clip < ClipCount; clip++)
			{
				if (pCurrentClipFix->Count [bg][g]) 
				{
					Left = pCurrentClipFix->Left [clip][bg][g];
					Right = pCurrentClipFix->Right [clip][bg][g];
					uint32 r = Left % PPU.Mosaic;
					HPos = HOffset + Left;
					PixWidth = PPU.Mosaic - r;
				}
				//uint32 s = Y * GPUPack.GFX.PPL + Left * GPUPack.GFX.PixSize;
				s32 Xt=Left;
				s32 Yt=Y;
				for (uint32 x = Left; x < Right; x += PixWidth, /*s += PixWidth * GPUPack.GFX.PixSize*/Xt+=PixWidth,HPos += PixWidth, PixWidth = PPU.Mosaic) 
				{
					uint32 Quot = (HPos & OffsetMask) >> 3;

					if (x + PixWidth >= Right) PixWidth = Right - x;

					if (GPUPack.BG.TileSize == 8) {
		    		if (Quot > 31) t = b2 + (Quot & 0x1f);
		    		else t = b1 + Quot;
					} else {
		    		if (Quot > 63) t = b2 + ((Quot >> 1) & 0x1f);
		    		else t = b1 + (Quot >> 1);
					}

					Tile = READ_2BYTES (t);
					GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];
					//realZ1=(int)GPUPack.GFX.Z1;realZ2=(int)GPUPack.GFX.Z2;
					short realZ2=(short)GPUPack.GFX.Z2;

					// pspDraw tile...
					if (GPUPack.BG.TileSize != 8)	
					{
		    			if (Tile & H_FLIP) 
						{
							// Horizontal flip, but what about vertical flip ?
							if (Tile & V_FLIP) 
							{
			    				// Both horzontal & vertical flip
			    				if (Rem16 < 8) {
									pspDrawLargePixel16 (Tile + 17 - (Quot & 1), Xt,Yt,HPos & 7, PixWidth,VirtAlign, Lines,realZ2);
			    				} else {
									pspDrawLargePixel16 (Tile + 1 - (Quot & 1), Xt,Yt,HPos & 7, PixWidth,VirtAlign, Lines,realZ2);
								}
							} else {
			    				// Horizontal flip only
			    				if (Rem16 > 7) {
									pspDrawLargePixel16 (Tile + 17 - (Quot & 1), Xt,Yt,HPos & 7, PixWidth,VirtAlign, Lines,realZ2);
			    				} else {
									pspDrawLargePixel16 (Tile + 1 - (Quot & 1), Xt,Yt,HPos & 7, PixWidth,VirtAlign, Lines,realZ2);
			    				}
							}
		    			} else
						{
							// No horizontal flip, but is there a vertical flip ?
							if (Tile & V_FLIP) 
							{
				    			// Vertical flip only
				    			if (Rem16 < 8) {
									pspDrawLargePixel16 (Tile + 16 + (Quot & 1), Xt,Yt,HPos & 7, PixWidth,VirtAlign, Lines,realZ2);
				    			} else {
									pspDrawLargePixel16 (Tile + (Quot & 1), Xt,Yt,HPos & 7, PixWidth,VirtAlign, Lines,realZ2);
				    			}
							} else 
							{
				    			// Normal unflipped
				    			if (Rem16 > 7) {
									pspDrawLargePixel16 (Tile + 16 + (Quot & 1), Xt,Yt,HPos & 7, PixWidth,VirtAlign, Lines,realZ2);
				    			} else {
									pspDrawLargePixel16 (Tile + (Quot & 1), Xt,Yt,HPos & 7, PixWidth,VirtAlign, Lines,realZ2);
				    			}
							}
		    			}
					} else 	pspDrawLargePixel16 (Tile, Xt,Yt, HPos & 7, PixWidth,VirtAlign, Lines,realZ2);
				}
			}
		}
	}
  
	//render
	if (_verticesCol_ptr-_verticesCol) {  				
  	
		sceGuTexImage(0,512,512,512,tile_texture[current_bitshift]);		
  	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT/*GU_COLOR_5551*/|GU_VERTEX_16BIT|GU_TRANSFORM_2D,(_verticesCol_ptr-_verticesCol),0,_verticesCol);  	
  	
	}
}


void pspDrawBackgroundMode5Fix (uint32 /* BGMODE */, uint32 bg, uint8 Z1, uint8 Z2) {
	int i;

  GPUPack.GFX.Pitch = GPUPack.GFX.RealPitch;
  GPUPack.GFX.PPL = GPUPack.GFX.PPLx2 >> 1;
  uint8 depths [2] = {Z1, Z2};

  uint32 Tile;
  uint16 *SC0;
  uint16 *SC1;
  uint16 *SC2;
  uint16 *SC3;
  uint32 Width;
  
  if (last_palette!=GPUPack.BG.DirectColourMode){  	 
			//DirectColourMode has changed, so we have to reload palette
			waitRenderingFix();
			last_palette=GPUPack.BG.DirectColourMode;
			clut = (u16*)NO_CPU_CACHE(&clut256[0]);
			memcpy(clut,GPUPack.GFX.ScreenColors,256*2);			
			if (rendering_beware_fix16)
			for (int i=0;i<256;i++) {
				if (clut[i]==fixedcol16) clut[i]^=1<<10; //swap blue 1st bit
			}
			clut[0]|=0x8000;
  		sceGuClutLoad(256/16,clut);			
	}
  
  //bg is 32x30 tile but needs more in case of clipping or scroll changing every line
	int num_vert=((GPUPack.GFX.EndY-GPUPack.GFX.StartY+1)+2+pCurrentClipFix->GroupCount [bg])*(256/8+6);			
  if (!(vertices[current_bitshift]=(struct Vertex*)sceGuGetMemory(num_vert*2* sizeof(struct Vertex)))){
  	debug_log("not enough vertices!");
  	return;
  }
	vertices_ptr[current_bitshift]=vertices[current_bitshift];
	vertices_end[current_bitshift]=vertices_ptr[current_bitshift]+num_vert*2;
		
/********************************************************/
/********************************************************/
    
  GPUPack.BG.StartPalette = 0;
                   
	
    

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

  if (GPUPack.BG.TileSize == 16) {
		VOffsetMask = 0x3ff;
		VOffsetShift = 4;
  } else {
		VOffsetMask = 0x1ff;
		VOffsetShift = 3;
	}
  int endy = GPUPack.GFX.EndY;

	uint32 Y = GPUPack.GFX.StartY;
	for (uint32 g = 0; g < pCurrentClipFix->GroupCount [bg]; g++) 
	{		
		Y=pCurrentClipFix->Start [bg][g];

 		for (; Y <= GPUPack.GFX.EndY && Y <= pCurrentClipFix->End [bg][g]; Y += Lines)
		{
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
	
			int clipcount = pCurrentClipFix->Count [bg][g];
			if (!clipcount) clipcount = 1;
			for (int clip = 0; clip < clipcount; clip++) 
			{
				int Left;
				int Right;

				if (pCurrentClipFix->Count [bg][g]==0)
				{
					Left = 0;
					Right = 512;
				} else 
				{
					Left = pCurrentClipFix->Left [clip][bg][g]* 2;
					Right = pCurrentClipFix->Right [clip][bg][g] * 2;

					if (Right <= Left) continue;
				}

				//uint32 s = (Left>>1) * GPUPack.GFX.PixSize + Y * 256;//GPUPack.GFX.PPL;
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
				if (HPos & 7) 
				{
					int Offset = (HPos & 7);
					Count = 8 - Offset;
					if (Count > Width) Count = Width;
					//s -= Offset>>1;
					Xt-=Offset>>1;
					Tile = READ_2BYTES (t);
					GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];
					//realZ1=(int)GPUPack.GFX.Z1;realZ2=(int)GPUPack.GFX.Z2;
					short realZ2=(short)GPUPack.GFX.Z2;
					if (GPUPack.BG.TileSize == 8)
					{
		    			if (!(Tile & H_FLIP))
						{
							// Normal, unflipped
							pspDrawHiResClippedTile16 (Tile + (Quot & 1), Xt,Yt, Offset, Count, VirtAlign, Lines,realZ2);
		    			} else 
						{
							// H flip
							pspDrawHiResClippedTile16 (Tile + 1 - (Quot & 1),Xt,Yt, Offset, Count, VirtAlign, Lines,realZ2);
		    			}
					} else
					{
		    			if (!(Tile & (V_FLIP | H_FLIP))) 
						{
							// Normal, unflipped
							pspDrawHiResClippedTile16 (Tile + t1 + (Quot & 1), Xt,Yt, Offset, Count, VirtAlign, Lines,realZ2);
		    			} else if (Tile & H_FLIP) 
						{
							if (Tile & V_FLIP) 
							{
			    				// H & V flip
			    				pspDrawHiResClippedTile16 (Tile + t2 + 1 - (Quot & 1), Xt,Yt, Offset, Count, VirtAlign, Lines,realZ2);
							} else 
							{
			    				// H flip only
			    				pspDrawHiResClippedTile16 (Tile + t1 + 1 - (Quot & 1), Xt,Yt, Offset, Count, VirtAlign, Lines,realZ2);
							}
		    			} else 
						{
							// V flip only
							pspDrawHiResClippedTile16 (Tile + t2 + (Quot & 1), Xt,Yt, Offset, Count, VirtAlign, Lines,realZ2);
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
				for (int C = Middle; C > 0; Xt+=4/*s += 4*/, Quot++, C--)
				{
					Tile = READ_2BYTES(t);
					GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];
					//realZ1=(int)GPUPack.GFX.Z1;realZ2=(int)GPUPack.GFX.Z2;
					short realZ2=(short)GPUPack.GFX.Z2;
					if (GPUPack.BG.TileSize == 8) 
					{
		    			if (!(Tile & H_FLIP))
						{
							// Normal, unflipped
							pspDrawHiResTile16 (Tile + (Quot & 1),Xt,Yt, VirtAlign, Lines,realZ2);
		    			} else 
						{
							// H flip
							pspDrawHiResTile16 (Tile + 1 - (Quot & 1), Xt,Yt, VirtAlign, Lines,realZ2);
		    			}
					} else 
					{
		    			if (!(Tile & (V_FLIP | H_FLIP))) 
						{
							// Normal, unflipped
							pspDrawHiResTile16 (Tile + t1 + (Quot & 1),Xt,Yt, VirtAlign, Lines,realZ2);
		    			} else if (Tile & H_FLIP)
						{
							if (Tile & V_FLIP)
							{
				    			// H & V flip
				    			pspDrawHiResTile16 (Tile + t2 + 1 - (Quot & 1), Xt,Yt, VirtAlign, Lines,realZ2);
							} else
							{			 
				    			// H flip only
			  	  				pspDrawHiResTile16 (Tile + t1 + 1 - (Quot & 1), Xt,Yt, VirtAlign, Lines,realZ2);
							}
						} else 
						{
							// V flip only
							pspDrawHiResTile16 (Tile + t2 + (Quot & 1), Xt,Yt, VirtAlign, Lines,realZ2);
						}
					}

					t += Quot & 1;
					if (Quot == 63) t = b2;
					else if (Quot == 127) t = b1;
				}

				// Right-hand edge clipped tiles
				if (Count)
				{
					Tile = READ_2BYTES(t);
					GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];
					//realZ1=(int)GPUPack.GFX.Z1;realZ2=(int)GPUPack.GFX.Z2;
					short realZ2=(short)GPUPack.GFX.Z2;
					if (GPUPack.BG.TileSize == 8)
					{
		    			if (!(Tile & H_FLIP))
						{
							// Normal, unflipped
							pspDrawHiResClippedTile16 (Tile + (Quot & 1), Xt,Yt, 0, Count, VirtAlign, Lines,realZ2);
		    			} else 
						{
							// H flip
							pspDrawHiResClippedTile16 (Tile + 1 - (Quot & 1), Xt,Yt, 0, Count, VirtAlign, Lines,realZ2);
		    			}
					} else 
					{
		    			if (!(Tile & (V_FLIP | H_FLIP)))
						{
							// Normal, unflipped
							pspDrawHiResClippedTile16 (Tile + t1 + (Quot & 1), Xt,Yt, 0, Count, VirtAlign, Lines,realZ2);
		    			} else if (Tile & H_FLIP) 
						{
							if (Tile & V_FLIP)
							{
			    				// H & V flip
			    				pspDrawHiResClippedTile16 (Tile + t2 + 1 - (Quot & 1), Xt,Yt, 0, Count, VirtAlign, Lines,realZ2);
							} else
							{
			    				// H flip only
			    				pspDrawHiResClippedTile16 (Tile + t1 + 1 - (Quot & 1), Xt,Yt, 0, Count, VirtAlign, Lines,realZ2);
							}
		    			} else 
						{
							// V flip only
							pspDrawHiResClippedTile16 (Tile + t2 + (Quot & 1), Xt,Yt, 0, Count, VirtAlign, Lines,realZ2);
		    			}
					}
				}
			}
		}
	}
	GPUPack.GFX.Pitch = GPUPack.GFX.RealPitch;
	GPUPack.GFX.PPL = GPUPack.GFX.PPLx2 >> 1;
  
	//render
	if (vertices_ptr[current_bitshift]-vertices[current_bitshift]) {  	
  	
		sceGuTexImage(0,512,512,512,tile_texture[current_bitshift]);
  		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,(vertices_ptr[current_bitshift]-vertices[current_bitshift]),0,vertices[current_bitshift]);
	}
}
 
//#define DEBUG_DRAWBACKGROUND
#if 0
#else
void pspDrawBackground (uint32 BGMode, uint32 bg, uint8 Z1, uint8 Z2);
void pspDrawBackgroundOffsetFix (uint32 BGMode, uint32 bg, uint8 Z1, uint8 Z2);
void pspDrawBackgroundFix (uint32 BGMode, uint32 bg, uint8 Z1, uint8 Z2) {
  GPUPack.BG.TileSize = BGSizes [PPU.BG[bg].BGSize];
  GPUPack.BG.BitShift = BitShifts[BGMode][bg];  
  GPUPack.BG.TileShift = TileShifts[BGMode][bg];
  GPUPack.BG.TileAddress = PPU.BG[bg].NameBase << 1;
  GPUPack.BG.NameSelect = 0;
  current_bitshift=Depths [BGMode][bg];  
  GPUPack.BG.Buffer = tile_texture[current_bitshift];//IPPU.TileCache8 [Depths [BGMode][bg]];
  GPUPack.BG.Buffered = IPPU.TileCached [Depths [BGMode][bg]];
  GPUPack.BG.PaletteShift = PaletteShifts[BGMode][bg];
  GPUPack.BG.PaletteMask = PaletteMasks[BGMode][bg];
  GPUPack.BG.DirectColourMode = (BGMode == 3 || BGMode == 4) && bg == 0 && (GPUPack.GFX.r2130 & 1);
  
  if (GPUPack.BG.DirectColourMode) { 
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps (); 
    GPUPack.GFX.ScreenColors = DirectColourMaps[0];           	    	
  } 
  else GPUPack.GFX.ScreenColors = &IPPU.ScreenColors[0];

/********************************************************/  
/********************************************************/
  
/********************************************************/
/********************************************************/

  if (PPU.BGMosaic [bg] && PPU.Mosaic > 1){
#ifdef DEBUG_DRAWBACKGROUND  	
{char str[32];
sprintf(str,"BGMode : %d mosaic",BGMode);
pgPrintBG(0,10,0xffff,str);	    	
}  	
#endif
  	
		pspDrawBackgroundMosaicFix (BGMode, bg, Z1, Z2);
		//debug_log("mosaic mode not supported");
		return;
  }
  switch (BGMode) {
    case 2: if (Settings.WrestlemaniaArcade) break;
    case 4: // Used by Puzzle Bobble
    	// Not used in psp hardware mode, since it's not really fit 
		//void pspDrawBackgroundOffset (uint32 BGMode, uint32 bg, uint8 Z1, uint8 Z2);
      pspDrawBackgroundOffsetFix (BGMode, bg, Z1, Z2);
			return;
    case 5:
    case 6: // XXX: is also offset per tile.				
#ifdef DEBUG_DRAWBACKGROUND    
{char str[32];
sprintf(str,"BGMode : %d mode5",BGMode);
pgPrintBG(0,10,0xffff,str);	    	
}  	    
#endif
	    pspDrawBackgroundMode5Fix (BGMode, bg, Z1, Z2);
	    return;				
  }
#ifdef DEBUG_DRAWBACKGROUND  
{char str[32];
sprintf(str,"BGMode : %d",BGMode);
pgPrintBG(0,10,0xffff,str);	    	
}  	  
#endif
  uint32 Tile;
  uint16 *SC0;
  uint16 *SC1;
  uint16 *SC2;
  uint16 *SC3;
  uint32 Width;
  uint8 depths [2] = {Z1, Z2};
  
  
  if (last_palette!=GPUPack.BG.DirectColourMode){  	 
			//DirectColourMode has changed, so we have to reload palette
			waitRenderingFix();
			last_palette=GPUPack.BG.DirectColourMode;
			clut = (u16*)NO_CPU_CACHE(&clut256[0]);
			memcpy(clut,GPUPack.GFX.ScreenColors,256*2);			
			if (rendering_beware_fix16)
			for (int i=0;i<256;i++) {
				if (clut[i]==fixedcol16) clut[i]^=1<<10; //swap blue 1st bit
			}
			clut[0]|=0x8000;
  		sceGuClutLoad(256/16,clut);			
	}
  //bg is 32x30 tile but needs more in case of clipping or scroll changing every line
	//int num_vert=((GPUPack.GFX.EndY-GPUPack.GFX.StartY+1)+2)*(256/8+6);		
	int num_vert=((GPUPack.GFX.EndY-GPUPack.GFX.StartY+1)+2+pCurrentClipFix->GroupCount [bg])*(256/8+6);		
  if (!(vertices[current_bitshift]=(struct Vertex*)sceGuGetMemory(num_vert*2* sizeof(struct Vertex)))){
  	debug_log("not enough vertices!");
  	return;
  }
	vertices_ptr[current_bitshift]=vertices[current_bitshift];
	vertices_end[current_bitshift]=vertices_ptr[current_bitshift]+num_vert*2;
	
    

  if (BGMode == 0) GPUPack.BG.StartPalette = bg << 5;
  else GPUPack.BG.StartPalette = 0;
	                				
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
	bool BGTileSizeis8=GPUPack.BG.TileSize == 8;
	if (GPUPack.BG.TileSize == 16){
		OffsetMask = 0x3ff;
		OffsetShift = 4;
	}else{
		OffsetMask = 0x1ff;
		OffsetShift = 3;
	}
	uint32 Y = GPUPack.GFX.StartY;
	for (uint32 g = 0; g < pCurrentClipFix->GroupCount [bg]; g++) 
	{		
		Y=pCurrentClipFix->Start [bg][g];

 		for (; Y <= GPUPack.GFX.EndY && Y <= pCurrentClipFix->End [bg][g]; Y += Lines)
		{
			uint32 VOffset = LineData [Y].BG[bg].VOffset;
			uint32 HOffset = LineData [Y].BG[bg].HOffset;
			int VirtAlign = (Y + VOffset) & 7;
		
			for (Lines = 1; Lines < 8 - VirtAlign; Lines++)
				if ((VOffset != LineData [Y + Lines].BG[bg].VOffset) ||	(HOffset != LineData [Y + Lines].BG[bg].HOffset)) break;

			if (Y + Lines > pCurrentClipFix->End [bg][g]) Lines = pCurrentClipFix->End [bg][g] + 1 - Y;

			uint32 ScreenLine = (VOffset + Y) >> OffsetShift;
			uint32 t1;
			uint32 t2;
			if (((VOffset + Y) & 15) > 7)
			{
				t1 = 16;
				t2 = 0;
			}else
			{
				t1 = 0;
				t2 = 16;
			}
			uint16 *b1;
			uint16 *b2;

			if (ScreenLine & 0x20) b1 = SC2, b2 = SC3;
			else b1 = SC0, b2 = SC1;

			b1 += (ScreenLine & 0x1f) << 5;
			b2 += (ScreenLine & 0x1f) << 5;

			int clipcount = pCurrentClipFix->Count [bg][g];
			if (!clipcount) clipcount = 1;
			//int clip = 0;
			//for (int clip = 0; clip < clipcount; clip++)
			for (uint32 clip = 0; clip < clipcount; clip++)
			{
				uint32 Left;
				uint32 Right;
				if (pCurrentClipFix->Count [bg][g]==0)
				{
					Left = 0;
					Right = 256;
				} else
				{
					Left = pCurrentClipFix->Left [clip][bg][g];
					Right = pCurrentClipFix->Right [clip][bg][g];

					if (Right <= Left) continue;
				}	    
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
				if (HPos & 7)
				{
					uint32 Offset = (HPos & 7);
					Count = 8 - Offset;
					if (Count > Width) Count = Width;
					
					Xt-=Offset;//sGP32 -= Offset ;
					Tile = READ_2BYTES(t);
					GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];
					//realZ1=(int)GPUPack.GFX.Z1;realZ2=(int)GPUPack.GFX.Z2;
					short realZ2=(short)GPUPack.GFX.Z2;		
					if (BGTileSizeis8) pspDrawClippedTile16_order (Tile, Xt,Yt, Offset, Count, VirtAlign, Lines,realZ2);
					else 
					{
						if (!(Tile & (V_FLIP | H_FLIP)))
						{
							// Normal, unflipped
							pspDrawClippedTile16_order(Tile + t1 + (Quot & 1),Xt,Yt, Offset, Count, VirtAlign, Lines,realZ2);
						} else if (Tile & H_FLIP) 
						{
							if (Tile & V_FLIP) 
							{
				    			// H & V flip
				    			pspDrawClippedTile16_order (Tile + t2 + 1 - (Quot & 1),Xt,Yt, Offset, Count, VirtAlign, Lines,realZ2);
							} else 
							{
				    			// H flip only
				    			pspDrawClippedTile16_order (Tile + t1 + 1 - (Quot & 1),Xt,Yt, Offset, Count, VirtAlign, Lines,realZ2);
							}
						} else 
						{
							// V flip only
							pspDrawClippedTile16_order (Tile + t2 + (Quot & 1), Xt,Yt,Offset, Count, VirtAlign, Lines,realZ2);
						}
					}

					if (GPUPack.BG.TileSize == 8) 
					{
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

				Count &= 7;
				for (int C = Middle; C > 0;/*sGP32 += 8*/Xt+=8 ,  Quot++, C--) {
					Tile = READ_2BYTES(t);
					GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];
					//realZ1=(int)GPUPack.GFX.Z1;realZ2=(int)GPUPack.GFX.Z2;
					short realZ2=(short)GPUPack.GFX.Z2;	
					if (!BGTileSizeis8) 
					{
		    			if (Tile & H_FLIP) 
						{
							// Horizontal flip, but what about vertical flip ?
							if (Tile & V_FLIP) 
							{
			    				// Both horzontal & vertical flip
			    				pspDrawTile16_order (Tile + t2 + 1 - (Quot & 1), Xt,Yt,VirtAlign, Lines,realZ2);
							} else 
							{
			    				// Horizontal flip only
			    				pspDrawTile16_order (Tile + t1 + 1 - (Quot & 1), Xt,Yt,VirtAlign, Lines,realZ2);
							}
		    			} else 
						{
							// No horizontal flip, but is there a vertical flip ?
							if (Tile & V_FLIP) 
							{
			    				// Vertical flip only
			    				pspDrawTile16_order (Tile + t2 + (Quot & 1), Xt,Yt,VirtAlign, Lines,realZ2);
							} else
							{
			    				// Normal unflipped
			    				pspDrawTile16_order (Tile + t1 + (Quot & 1), Xt,Yt,VirtAlign, Lines,realZ2);
							}
		    			}
					} 
					else 
					{
		    			pspDrawTile16_order (Tile, Xt,Yt, VirtAlign, Lines,realZ2);		
					}
		
					if (BGTileSizeis8)
					{
		    			t++;
		    			if (Quot == 31) t = b2;
		    			else if (Quot == 63)
							t = b1;
					} else 
					{
		    			t += Quot & 1;
		    			if (Quot == 63) t = b2;
		    			else if (Quot == 127)
							t = b1;
					}
				}
				// Right-hand edge clipped tiles
				if (Count) 
				{
	    			Tile = READ_2BYTES(t);
	    			GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];
	    			//realZ1=(int)GPUPack.GFX.Z1;realZ2=(int)GPUPack.GFX.Z2;
					short realZ2=(short)GPUPack.GFX.Z2;
					if (BGTileSizeis8) pspDrawClippedTile16_order (Tile, Xt,Yt, 0, Count, VirtAlign, Lines,realZ2);
					else 
					{
		    			if (!(Tile & (V_FLIP | H_FLIP)))
						{
							// Normal, unflipped
							pspDrawClippedTile16_order (Tile + t1 + (Quot & 1), Xt,Yt, 0, Count, VirtAlign, Lines,realZ2);
		    			}else if (Tile & H_FLIP)
						{
							if (Tile & V_FLIP)
							{
			   					// H & V flip
			   					pspDrawClippedTile16_order (Tile + t2 + 1 - (Quot & 1),Xt,Yt, 0, Count, VirtAlign, Lines,realZ2);
							} else
							{
			   					// H flip only
			   					pspDrawClippedTile16_order (Tile + t1 + 1 - (Quot & 1),Xt,Yt, 0, Count, VirtAlign, Lines,realZ2);
							}
		    			} else 
						{
							// V flip only
							pspDrawClippedTile16_order (Tile + t2 + (Quot & 1),Xt,Yt, 0, Count, VirtAlign, Lines,realZ2);
		    			}
					}
				}
			}
		}
	}
	//render
	if (vertices_ptr[current_bitshift]-vertices[current_bitshift]) {  
  	
  		sceGuTexImage(0,512,512,512,tile_texture[current_bitshift]);  	
  		//if(pCurrentClipFix->GroupCount [bg]==0)
			sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,(vertices_ptr[current_bitshift]-vertices[current_bitshift]),0,vertices[current_bitshift]);
	}

}

#endif	
	
//drawmode is : 0 => everything (for sub & main when no transp)
//     					1 => only what's added/subbed (for main)
//							2 => only what's not added/subbed (for main)
void pspRenderScreenFix (uint8 *Screen, bool8 sub, uint8 D, uint8 drawmode)
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

  GPUPack.GFX.S = Screen;

  if (!sub) {
		GPUPack.GFX.pCurrentClip = &IPPU.Clip [0];
		pCurrentClipFix = &IPPU.ClipFix [0];
		BG0 = ON_MAIN (0);
		BG1 = ON_MAIN (1);
		BG2 = ON_MAIN (2);
		BG3 = ON_MAIN (3);
		OB  = ON_MAIN (4);
  } else {
		GPUPack.GFX.pCurrentClip = &IPPU.Clip [1];
		pCurrentClipFix = &IPPU.ClipFix [1];
		BG0 = ON_SUB (0);
		BG1 = ON_SUB (1);
		BG2 = ON_SUB (2);
		BG3 = ON_SUB (3);
		OB  = ON_SUB (4);
  }

  if (PPU.BGMode <= 1) {
		if (OB&&os9x_OBJ) {
			if (TO_DRAW_OBJ(4)) {
	    	/*if (os9x_fastsprite) pspDrawFASTOBJSFix (!sub, D,drawmode);
	    	else*/ if(pCurrentClipFix->GroupCount [4]>0)
				pspDrawOBJSFix (!sub, D,drawmode);
			else
				pspDrawOBJS (!sub, D,drawmode);
	  	}
		}
		if (BG0&&os9x_BG0) {	    
	    if (TO_DRAW(0)) {
/*			if(IPPU.PaletteLineCount>0)
				pspDrawBackgroundFixPalette (PPU.BGMode, 0, D + 10, D + 14);
	    	else*/ if(pCurrentClipFix->GroupCount [0]>0)
				pspDrawBackgroundFix (PPU.BGMode, 0, D + 10, D + 14);
			else
				pspDrawBackground (PPU.BGMode, 0, D + 10, D + 14);
	    }
		}
		if (BG1&&os9x_BG1) {			    
			if (TO_DRAW(1)) {				
/*			if(IPPU.PaletteLineCount>0)
				pspDrawBackgroundFixPalette (PPU.BGMode, 1, D + 9, D + 13);
	    	else*/ if(pCurrentClipFix->GroupCount [1]>0)
				pspDrawBackgroundFix (PPU.BGMode, 1, D + 9, D + 13);
			else
				pspDrawBackground (PPU.BGMode, 1, D + 9, D + 13);
			}
		}
		if (BG2&&os9x_BG2) {	    
	    if (TO_DRAW(2)) {	    	
/*			if(IPPU.PaletteLineCount>0)
				pspDrawBackgroundFixPalette (PPU.BGMode, 2, D + 3,  (FillRAM [0x2105] & 8) == 0 ? D + 6 : D + 17);
	    	else*/ if(pCurrentClipFix->GroupCount [2]>0)
				pspDrawBackgroundFix (PPU.BGMode, 2, D + 3,  (FillRAM [0x2105] & 8) == 0 ? D + 6 : D + 17);
			else
				pspDrawBackground (PPU.BGMode, 2, D + 3,  (FillRAM [0x2105] & 8) == 0 ? D + 6 : D + 17);
	    }
		}
		if (BG3 && PPU.BGMode == 0 && os9x_BG3) {	    
	    if (TO_DRAW(3)) {	    	
/*			if(IPPU.PaletteLineCount>0)
				pspDrawBackgroundFixPalette (PPU.BGMode, 3, D + 2, D + 5);
	    	else*/ if(pCurrentClipFix->GroupCount [3]>0)
    			pspDrawBackgroundFix (PPU.BGMode, 3, D + 2, D + 5);
			else
				pspDrawBackground (PPU.BGMode, 3, D + 2, D + 5);
	    }
		}
	} else if (PPU.BGMode != 7) {
		if (OB&&os9x_OBJ) {
			/*if (os9x_fastsprite) pspDrawFASTOBJSFix (!sub, D,drawmode);
	    	else*/ if(pCurrentClipFix->GroupCount [4]>0)
				pspDrawOBJSFix (!sub, D,drawmode);
			else
				pspDrawOBJS (!sub, D,drawmode);
		}
		if (BG0&&os9x_BG0) {	    
	    if (TO_DRAW(0)) {	    	
	    	if(pCurrentClipFix->GroupCount [0]>0)
				pspDrawBackgroundFix (PPU.BGMode, 0, D + 5, D + 13);
			else
				pspDrawBackground (PPU.BGMode, 0, D + 5, D + 13);
	    }
		}
		if (PPU.BGMode != 6 && BG1 &&os9x_BG1) {	    
	    if (TO_DRAW(1)) {	    	
	    	if(pCurrentClipFix->GroupCount [1]>0)
				pspDrawBackgroundFix (PPU.BGMode, 1, D + 2, D + 9);
			else
				pspDrawBackground (PPU.BGMode, 1, D + 2, D + 9);
	    }
		}
	} else {
    	return;
	/*if (OB &&os9x_OBJ)
	{
	    
	    //pspDrawOBJSFix (!sub, D);
	}
	if (BG0 || ((FillRAM [0x2133] & 0x40) && BG1) &&os9x_BG0)
	{
	    int bg;

	    if (FillRAM [0x2133] & 0x40)
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

		    pspDrawBGMode7Background16 (Screen, bg);
	    }
	    else
	    {
		if (GPUPack.GFX.r2131 & 0x80)
		{
		    if (GPUPack.GFX.r2131 & 0x40)
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
		    if (GPUPack.GFX.r2131 & 0x40)
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


extern int debug_counts[];
void pspS9xUpdateScreenFix (){
	int i;  
  
	render_timestamp++;

  /*GPUPack.GFX.S = GPUPack.GFX.Screen;
  GPUPack.GFX.r2131 = FillRAM [0x2131];
  GPUPack.GFX.r212c = FillRAM [0x212c];
  GPUPack.GFX.r212d = FillRAM [0x212d];
  GPUPack.GFX.r2130 = FillRAM [0x2130];
  GPUPack.GFX.Pseudo = (FillRAM [0x2133] & 8) != 0 &&
    (GPUPack.GFX.r212c & 15) != (GPUPack.GFX.r212d & 15) &&
    (GPUPack.GFX.r2131 & 0x3f) == 0;

  if (IPPU.OBJChanged)
    S9xSetupOBJ ();

  if (PPU.RecomputeClipWindows)
    {
      ComputeClipWindows ();
      PPU.RecomputeClipWindows = FALSE;
    }
*/
	SET_DEBUG_COUNT(30,IPPU.ClipFixMaxCount);
	GPUPack.GFX.StartY = IPPU.PreviousLine;
	if ((GPUPack.GFX.EndY = IPPU.CurrentLine - 1) >= PPU.ScreenHeight)
		GPUPack.GFX.EndY = PPU.ScreenHeight - 1;

	uint32 starty = GPUPack.GFX.StartY;
	uint32 endy = GPUPack.GFX.EndY;
    
	if (tile_cache_reset) tile_reset_cache();
	else tile_resetask=0;    
                
	if (GPUPack.GFX.Pseudo)	{
		GPUPack.GFX.r2131 = 0x5f;
		GPUPack.GFX.r212d = (FillRAM [0x212c] ^ FillRAM [0x212d]) & 15;
		GPUPack.GFX.r212c &= ~GPUPack.GFX.r212d;
		GPUPack.GFX.r2130 |= 2;
	}
			//extern ClipDataFix debug_ClipDataFix;
			//extern int debug_start;
			//extern int debug_end;
			//extern int os9x_renderingpass;	
			//if(os9x_renderingpass==1)
			//{
			//	debug_start=starty | 1000;
			//	debug_end=endy;
			//	memcpy(&debug_ClipDataFix,&IPPU.ClipFix [1],sizeof(ClipDataFix));
			//}
  		
  if (!os9x_easy&&!PPU.ForcedBlanking && ADD_OR_SUB_ON_ANYTHING && (GPUPack.GFX.r2130 & 0x30) != 0x30 &&
	  	(!((GPUPack.GFX.r2130 & 0x30) == 0x10 && IPPU.ClipFix[1].GroupCount[5] == 0) || !((GPUPack.GFX.r2130 & 0x30) == 0x10 && IPPU.Clip[1].Count[5] == 0))){	  		
	  fixedcol=((int)(IPPU.XB [PPU.FixedColourRed])<<3)|((int)(IPPU.XB [PPU.FixedColourGreen])<<11)|((int)(IPPU.XB [PPU.FixedColourBlue])<<19);	  	
	  fixedcol16=((int)(IPPU.XB [PPU.FixedColourRed])<<0)|((int)(IPPU.XB [PPU.FixedColourGreen])<<5)|((int)(IPPU.XB [PPU.FixedColourBlue])<<10);	
		INC_DEBUG_COUNT(TRANCE_COUNT);

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
		initRenderingFix();
				
		//set subscreen as drawing buffer
		sceGuDrawBufferList(GU_PSM_5551,(void*)(512*272*2*2+256*240*2+0*256*256*2),256);		
		sceGuTexMode(GU_PSM_T8,0,0,0); //8bit texture
		
		//clear the ZBuffer
		sceGuScissor(0,starty,256,endy/*-starty*/+1);
		sceGuClearDepth(0);
		sceGuClear(GU_DEPTH_BUFFER_BIT);
																						  		
		//subscreen colour window handling
		if ( IPPU.ClipFix [1].GroupCount [5]) {
			SET_DEBUG_COUNT(SUBSCREEN_COLOUR_WINDOW,1);

	  		// Colour window enabled on subscreen
			// first we clear it with 0
			// if 2 pass blending is need, 0 will be used to detect fixed colour
			// so the "normal black", 0, is replaced by 1 (dark blue)
			if (rendering_beware_fix16) {
				if (fixedcol==0) sceGuClearColor(1<<19);
				else sceGuClearColor(0);
			} else sceGuClearColor(0);
	   		sceGuClear(GU_COLOR_BUFFER_BIT);
			//extern ClipDataFix debug_ClipDataFix;
			//if(IPPU.ClipFix [1].GroupCount [5]>10)memcpy(&debug_ClipDataFix,&IPPU.ClipFix [1],sizeof(ClipDataFix));
	   		   
			if(IPPU.FixColorCount)
			{
				//clipping && Fix color change
				SET_DEBUG_COUNT(FIXCOLORCOUNT_ON_COLOUR_WINDOW,IPPU.FixColorCount);
				int startc=starty;
				int endc=starty;
				for (uint32 g = 0; g < IPPU.ClipFix [1].GroupCount [5]; g++) {
					int startcl;
					int endcl;
					startcl = IPPU.ClipFix [1].Start [5][g];
					endcl = IPPU.ClipFix [1].End [5][g]+1;
  					int i=0;
					int startc=startcl;
					int endc=startcl;
					for(;i<IPPU.FixColorCount-1;i++)
					{
						if(IPPU.FixColorLines[i+1]<startcl)
							continue;
						if(IPPU.FixColorLines[i+1]>endcl)
							break;

						endc= IPPU.FixColorLines[i+1];
						sceGuScissor(0,startc,256,endc);
						sceGuClearColor(IPPU.FixColorLog [i]);
  						for (uint32 c = 0; c < IPPU.ClipFix [1].Count [5][g]; c++) {
  							if (IPPU.ClipFix [1].Right [c][5][g] > IPPU.ClipFix [1].Left [c][5][g]){
			 						//current rendering area
	    						sceGuScissor(IPPU.ClipFix [1].Left [c][5][g],startc,(IPPU.ClipFix [1].Right [c][5][g])+1,endc);
     							sceGuClear(GU_COLOR_BUFFER_BIT);
     						}
     					}
						startc=endc;
					}
					if(endc<endcl)
					{
						sceGuClearColor(IPPU.FixColorLog [i]);
  						for (uint32 c = 0; c < IPPU.ClipFix [1].Count [5][g]; c++) {
  							if (IPPU.ClipFix [1].Right [c][5][g] > IPPU.ClipFix [1].Left [c][5][g]){
			 						//current rendering area
	    						sceGuScissor(IPPU.ClipFix [1].Left [c][5][g],endc,(IPPU.ClipFix [1].Right [c][5][g])+1,endcl);
     							sceGuClear(GU_COLOR_BUFFER_BIT);
     						}
     					}
					}

				}
			}
			else
			{
	   			// then with FixedColour where it will be rendered
	   			sceGuClearColor(fixedcol);
				for (uint32 g = 0; g < IPPU.ClipFix [1].GroupCount [5]; g++) {
  					for (uint32 c = 0; c < IPPU.ClipFix [1].Count [5][g]; c++) {
  						if (IPPU.ClipFix [1].Right [c][5][g] > IPPU.ClipFix [1].Left [c][5][g]){
			 					//current rendering area
	    					sceGuScissor(IPPU.ClipFix [1].Left [c][5][g],IPPU.ClipFix [1].Start [5][g],(IPPU.ClipFix [1].Right [c][5][g])+1,IPPU.ClipFix [1].End [5][g]+1);
     						sceGuClear(GU_COLOR_BUFFER_BIT);
     					}
     				}
				}
			}
			sceGuScissor(0,starty,256,endy+1);

		} else {

    		// No colour window enabled on subscreen
			info(32,1,"no color win sub");
    		// we clear it with FixedColour

			//if(IPPU.FixColorCount>100)
			//{
			//extern int    debug_FixColorCount;
			//extern uint8  debug_FixColorLines[256];
			//extern uint16 debug_FixColorLog[256];
			//memcpy(debug_FixColorLines,IPPU.FixColorLines,256);
			//memcpy(debug_FixColorLog,IPPU.FixColorLog,256*2);
			//debug_FixColorCount=IPPU.FixColorCount;
			//}
			if(IPPU.FixColorCount)
			{
				SET_DEBUG_COUNT(FIXCOLORCOUNT_ON_NO_COLOUR_WINDOW,IPPU.FixColorCount);
				int startc=starty;
				int endc=starty;
				int i=0;
				for(;i<IPPU.FixColorCount-1;i++)
				{
					endc= IPPU.FixColorLines[i+1];
					sceGuScissor(0,startc,256,endc);
					sceGuClearColor(IPPU.FixColorLog[i]);
	   				sceGuClear(GU_COLOR_BUFFER_BIT);
					startc=endc;
				}
				if(endc-1<endy)
				{
					sceGuScissor(0,endc,256,endy+1);
					sceGuClearColor(IPPU.FixColorLog[i]);
	   				sceGuClear(GU_COLOR_BUFFER_BIT);
				}
				sceGuScissor(0,starty,256,endy+1);
			}
			else
			{
    			sceGuClearColor(fixedcol);
	   			sceGuClear(GU_COLOR_BUFFER_BIT);
			}
		}
	  	  	  			   
	  if (ANYTHING_ON_SUB)  {
info(32,2,"rendering subscreen ");								
	      pspRenderScreenFix (GPUPack.GFX.SubScreen, true, SUB_SCREEN_DEPTH,0);
	  } else {
info(32,2,"nothing on subscreen");
		}							
		
		//now main screen	  	  
		sceGuDrawBufferList(GU_PSM_5551,(void*)(512*272*2*2+256*240*2+1*256*256*2),256);	  
	  	  
		//handle main screen colour window
		if (IPPU.ClipFix [0].GroupCount[5])
		{
			SET_DEBUG_COUNT(MAINSCREEN_COLOUR_WINDOW,1);
			// Colour window enabled on mainscreen
			int col=(((IPPU.ScreenColors [0])&31)<<3)|(((IPPU.ScreenColors [0]>>5)&31)<<11)|(((IPPU.ScreenColors [0]>>10)&31)<<19);			
			// first we clear it with 0 everywhere
			if (rendering_beware_fix16)
			{
				if (fixedcol==0) sceGuClearColor(1<<19);
				else sceGuClearColor(0);
			} else sceGuClearColor(0);
			
	   		sceGuClear(GU_COLOR_BUFFER_BIT);	   	
	   		// and back color inside rendered area
	   		   	
	   		if (rendering_beware_fix16) {
				if (fixedcol==IPPU.ScreenColors [0]) col^=1<<19;
				//if (fixedcol16==IPPU.ScreenColors [0]) col^=1<<19;
			} 
			if(IPPU.MainColorCount)
			{
				//clipping && color change
				SET_DEBUG_COUNT(MAINCOLORCOUNT_CLIP,IPPU.MainColorCount);
				int startc=starty;
				int endc=starty;
				sceGuClearColor(col);
   				for (uint32 g = 0; g < IPPU.ClipFix [0].GroupCount [5]; g++) {
					int startcl;
					int endcl;
					startcl = IPPU.ClipFix [0].Start [5][g];
					endcl = IPPU.ClipFix [0].End [5][g]+1;
  					int i=0;
					int startc=startcl;
					int endc=startcl;
					for(;i<IPPU.MainColorCount-1;i++)
					{
						if(IPPU.MainColorLines[i+1]<startcl)
							continue;
						if(IPPU.MainColorLines[i+1]>endcl)
							break;

						endc= IPPU.MainColorLines[i+1];
						sceGuScissor(0,startc,256,endc);
						col=(((IPPU.MainColorLog [i])&31)<<3)|(((IPPU.MainColorLog [i]>>5)&31)<<11)|(((IPPU.MainColorLog [i]>>10)&31)<<19);
						if (rendering_beware_fix16) {
							if (fixedcol==IPPU.ScreenColors [0]) col^=1<<19;
							//if (IPPU.MainColorLog [i]==IPPU.ScreenColors [0]) col^=1<<19;
						} 
						sceGuClearColor(col);
						for (uint32 c = 0; c < IPPU.ClipFix [0].Count [5][g]; c++) {
  							if (IPPU.ClipFix [0].Right [c][5][g] > IPPU.ClipFix [0].Left [c][5][g]){
			 						//current rendering area
	    						sceGuScissor(IPPU.ClipFix [0].Left [c][5][g],startc,(IPPU.ClipFix [0].Right [c][5][g])+1,endc);
     							sceGuClear(GU_COLOR_BUFFER_BIT);
     						}
     					}
						startc=endc;
					}
					if(endc<endcl)
					{
						col=(((IPPU.MainColorLog [i])&31)<<3)|(((IPPU.MainColorLog [i]>>5)&31)<<11)|(((IPPU.MainColorLog [i]>>10)&31)<<19);
						if (rendering_beware_fix16) {
							if (fixedcol==IPPU.ScreenColors [0]) col^=1<<19;
						} 
						sceGuClearColor(col);
						for (uint32 c = 0; c < IPPU.ClipFix [0].Count [5][g]; c++) {
  							if (IPPU.ClipFix [0].Right [c][5][g] > IPPU.ClipFix [0].Left [c][5][g]){
			 						//current rendering area
	    						sceGuScissor(IPPU.ClipFix [0].Left [c][5][g],endc,(IPPU.ClipFix [0].Right [c][5][g])+1,endcl);
     							sceGuClear(GU_COLOR_BUFFER_BIT);
     						}
     					}
					}

				}
			}
			else
			{
				//clipping
				sceGuClearColor(col);
   				for (uint32 g = 0; g < IPPU.ClipFix [0].GroupCount [5]; g++) {
					int startcl;
					int endcl;
					startcl = IPPU.ClipFix [0].Start [5][g];
					endcl = IPPU.ClipFix [0].End [5][g]+1;
  					for (uint32 c = 0; c < IPPU.ClipFix [0].Count [5][g]; c++) {
  						if (IPPU.ClipFix [0].Right [c][5][g] > IPPU.ClipFix [0].Left [c][5][g]){
			 					//current rendering area
	    					sceGuScissor(IPPU.ClipFix [0].Left [c][5][g],startcl,(IPPU.ClipFix [0].Right [c][5][g])+1,endcl);
     						sceGuClear(GU_COLOR_BUFFER_BIT);
     					}
     				}
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
				if (_vertices)
				{
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
				}
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
   			for (uint32 g = 0; g < IPPU.ClipFix [0].GroupCount [5]; g++) {
  				for (uint32 c = 0; c < IPPU.ClipFix [0].Count [5][g]; c++) {
  					if (IPPU.ClipFix [0].Right [c][5][g] > IPPU.ClipFix [0].Left [c][5][g]){
		  				//current rendering area
	     				sceGuScissor(IPPU.ClipFix [0].Left [c][5][g],IPPU.ClipFix [0].Start [5][g],(IPPU.ClipFix [0].Right [c][5][g])+1,IPPU.ClipFix [0].End [5][g]+1);
     					sceGuClear(GU_DEPTH_BUFFER_BIT);
     				}
				}
			}
     		// and back to whole  rendering area
     		sceGuScissor(0,starty,256,endy/*-starty*/+1);

		} else {//"no color win main"
    		// Colour window disabled on mainscreen
			// we clear it with ScreenColors[0]
			//extern int    debug_MainColorCount;
			//extern uint8  debug_MainColorLines[256];
			//extern uint16 debug_MainColorLog[256];
			if(IPPU.MainColorCount)
			{
				SET_DEBUG_COUNT(MAINCOLORCOUNT_NO_CLIP,IPPU.MainColorCount);
				int startc=starty;
				int endc=starty;//endy+1;//IPPU.MainColorLines[1];
				int i=0;
				extern int os9x_renderingpass;
				//if(os9x_renderingpass==4)
				//{
				//	memcpy(debug_MainColorLines,IPPU.MainColorLines,256);
				//	memcpy(debug_MainColorLog,IPPU.MainColorLog,256*2);
				//	debug_MainColorCount=IPPU.MainColorCount;
				//}
				sceGuClearDepth(1);//MAIN_SCREEN_DEPTH);//0??? seiken3 dialog
				for(;i<IPPU.MainColorCount-1;i++)
				{
					endc= IPPU.MainColorLines[i+1];
					sceGuScissor(0,startc,256,endc);
					sceGuClearColor((((IPPU.MainColorLog [i])&31)<<3)|(((IPPU.MainColorLog [i]>>5)&31)<<11)|(((IPPU.MainColorLog [i]>>10)&31)<<19));
	   				sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
					startc=endc;
				}
				if(endc-1<endy)
				{
					sceGuScissor(0,endc,256,endy+1);
					sceGuClearColor((((IPPU.MainColorLog [i])&31)<<3)|(((IPPU.MainColorLog [i]>>5)&31)<<11)|(((IPPU.MainColorLog [i]>>10)&31)<<19));
	   				sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
				}
				sceGuScissor(0,starty,256,endy+1);
			}
			else
			{
				sceGuClearColor((((IPPU.ScreenColors [0])&31)<<3)|(((IPPU.ScreenColors [0]>>5)&31)<<11)|(((IPPU.ScreenColors [0]>>10)&31)<<19));	   	
	   			//clear Z buffer, TODO : check if Zbuffer clear is needed each time (ADD_OR_SUB(5) only since no colour window ?)
	  			sceGuClearDepth(1);//MAIN_SCREEN_DEPTH);//0??? seiken3 dialog
	   			sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
			}
		}
						
		// now first pass main screen (with BG/OBJ to be combined with subscreen/fixed colour
		//invalidate last palette
				
		pspRenderScreenFix (GPUPack.GFX.Screen, false, SUB_SCREEN_DEPTH,1);
	  	  	  	  
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
		if (_vertices)
		{
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
		}
		
		//load subscreen texture
		sceGuTexImage(0,256,256,256,(u16*)(0x44000000+512*272*2*2+256*240*2+0*256*256*2));
		
		sceGuEnable(GU_DEPTH_TEST);
		sceGuDepthMask(GU_TRUE);				
		// now we'll add subscreen/fixedcolour
		
		if ((!IPPU.ClipFix [0].Count[5])&&SUB_OR_ADD(5))
		{
			//no colour window on mainscreen
			//and sub_or_add back colour as well
			//so no need to filter subscreen, the whole will be blended
	  		//sceGuDepthFunc(GU_ALWAYS);
	  		sceGuDisable(GU_DEPTH_TEST);
		} else 
		{
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
			
			if (IPPU.ClipFix [1].GroupCount [5]) {
				//col win sub, so not apply blending everywhere
				int end=-1;
				int start;
				for (uint32 g = 0; g < IPPU.ClipFix [1].GroupCount [5]; g++) {
					if(IPPU.ClipFix [1].Start [5][g] == end-1)
						start=end;
					else
						start=IPPU.ClipFix [1].Start [5][g];
					end = IPPU.ClipFix [1].End [5][g]+1;
					for (uint32 c = 0; c < IPPU.ClipFix [1].Count [5][g]; c++) {
  						if (IPPU.ClipFix [1].Right [c][5][g] > IPPU.ClipFix [1].Left [c][5][g]){

			 				//current rendering area
							sceGuScissor(IPPU.ClipFix [1].Left [c][5][g],start,(IPPU.ClipFix [1].Right [c][5][g])+1,end);
     						sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2*4,0,_vertices);
						}
     				}
     			}
     			sceGuScissor(0,starty,256,endy/*-starty*/+1);
     		} else {
     			sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2*4,0,_vertices);												
     		}
		}
#if 0
	 	waitRenderingFix();
#endif	 	
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
		pspRenderScreenFix (GPUPack.GFX.Screen, false,SUB_SCREEN_DEPTH,2);						

		endRenderingFix();	  
	} else {
		INC_DEBUG_COUNT(NO_TRANCE_COUNT);
		info(32,0,"notransp");
	/*  		if(os9x_renderingpass==1)
			{
				debug_start=starty | 2000;
				debug_end=endy;
				memcpy(&debug_ClipDataFix,&IPPU.ClipFix [0],sizeof(ClipDataFix));
			}*/
	  // 16bit and transparency but currently no transparency effects in
	  // operation.
	  
	  
	  if (!PPU.ForcedBlanking) {	  		  	
	  	
		rendering_beware_fix16=0;	  	
		initRenderingFix();

	  	
	  	sceGuDrawBufferList(GU_PSM_5551,(void*)(0x40000000+512*272*2*2+256*240*2+2*256*256*2),256);	  		  	
	  	sceGuTexMode(GU_PSM_T8,0,0,0); //8bit texture
	  								  		  		  	
	  	//setup back colors
		int col=(((IPPU.ScreenColors [0])&31)<<3)|(((IPPU.ScreenColors [0]>>5)&31)<<11)|(((IPPU.ScreenColors [0]>>10)&31)<<19);		
	  	//sceGuClearColor((((IPPU.ScreenColors [0]>>0)&31)<<3)|(((IPPU.ScreenColors [0]>>5)&31)<<11)|(((IPPU.ScreenColors [0]>>10)&31)<<19));	  	
	  	/*if (IPPU.Clip [0].Count [5]) {
			sceGuClearColor(col);
	  		for (uint32 c = 0; c < IPPU.Clip [0].Count [5]; c++) {
	  			if (IPPU.Clip [0].Right [c][5] > IPPU.Clip [0].Left [c][5]){
		     		sceGuScissor(IPPU.Clip [0].Left [c][5],starty,(IPPU.Clip [0].Right [c][5])+1,endy+1);
	     			sceGuClear(GU_COLOR_BUFFER_BIT);	     			
	     		} 
	     	}
			sceGuScissor(0,starty,256,endy+1);
			sceGuClearDepth(0);
			sceGuClear(GU_DEPTH_BUFFER_BIT);*/
		if (IPPU.ClipFix [0].GroupCount [5]) {
			if(IPPU.MainColorCount)
			{
				//clipping && color change
				SET_DEBUG_COUNT(MAINCOLORCOUNT_CLIP,IPPU.MainColorCount);
				int startc=starty;
				int endc=starty;
				sceGuClearColor(col);
   				for (uint32 g = 0; g < IPPU.ClipFix [0].GroupCount [5]; g++) {
					int startcl;
					int endcl;
					startcl = IPPU.ClipFix [0].Start [5][g];
					endcl = IPPU.ClipFix [0].End [5][g]+1;
  					int i=0;
					int startc=startcl;
					int endc=startcl;
					for(;i<IPPU.MainColorCount-1;i++)
					{
						if(IPPU.MainColorLines[i+1]<startcl)
							continue;
						if(IPPU.MainColorLines[i+1]>endcl)
							break;

						endc= IPPU.MainColorLines[i+1];
						sceGuScissor(0,startc,256,endc);
						col=(((IPPU.MainColorLog [i])&31)<<3)|(((IPPU.MainColorLog [i]>>5)&31)<<11)|(((IPPU.MainColorLog [i]>>10)&31)<<19);
						if (rendering_beware_fix16) {
							if (fixedcol==IPPU.ScreenColors [0]) col^=1<<19;
							//if (IPPU.MainColorLog [i]==IPPU.ScreenColors [0]) col^=1<<19;
						} 
						sceGuClearColor(col);
						for (uint32 c = 0; c < IPPU.ClipFix [0].Count [5][g]; c++) {
  							if (IPPU.ClipFix [0].Right [c][5][g] > IPPU.ClipFix [0].Left [c][5][g]){
			 						//current rendering area
	    						sceGuScissor(IPPU.ClipFix [0].Left [c][5][g],startc,(IPPU.ClipFix [0].Right [c][5][g])+1,endc);
     							sceGuClear(GU_COLOR_BUFFER_BIT);
     						}
     					}
						startc=endc;
					}
					if(endc<endcl)
					{
						col=(((IPPU.MainColorLog [i])&31)<<3)|(((IPPU.MainColorLog [i]>>5)&31)<<11)|(((IPPU.MainColorLog [i]>>10)&31)<<19);
						if (rendering_beware_fix16) {
							if (fixedcol==IPPU.ScreenColors [0]) col^=1<<19;
						} 
						sceGuClearColor(col);
						for (uint32 c = 0; c < IPPU.ClipFix [0].Count [5][g]; c++) {
  							if (IPPU.ClipFix [0].Right [c][5][g] > IPPU.ClipFix [0].Left [c][5][g]){
			 						//current rendering area
	    						sceGuScissor(IPPU.ClipFix [0].Left [c][5][g],endc,(IPPU.ClipFix [0].Right [c][5][g])+1,endcl);
     							sceGuClear(GU_COLOR_BUFFER_BIT);
     						}
     					}
					}

				}
			}
			else
			{
				//clipping
				sceGuClearColor(col);
   				for (uint32 g = 0; g < IPPU.ClipFix [0].GroupCount [5]; g++) {
					int startcl;
					int endcl;
					startcl = IPPU.ClipFix [0].Start [5][g];
					endcl = IPPU.ClipFix [0].End [5][g]+1;
  					for (uint32 c = 0; c < IPPU.ClipFix [0].Count [5][g]; c++) {
  						if (IPPU.ClipFix [0].Right [c][5][g] > IPPU.ClipFix [0].Left [c][5][g]){
			 					//current rendering area
	    					sceGuScissor(IPPU.ClipFix [0].Left [c][5][g],startcl,(IPPU.ClipFix [0].Right [c][5][g])+1,endcl);
     						sceGuClear(GU_COLOR_BUFFER_BIT);
     					}
     				}
				}
			}
	     	sceGuScissor(0,starty,256,endy/*-starty*/+1);
	     	sceGuClearDepth(0);
	     	sceGuClear(GU_DEPTH_BUFFER_BIT);
	    } else {
	    	//sceGuScissor(0,starty,256,endy/*-starty*/+1);
	    	//sceGuClearDepth(0);
		   	//sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

			if(IPPU.MainColorCount)
			{
				SET_DEBUG_COUNT(MAINCOLORCOUNT_NO_CLIP,IPPU.MainColorCount);
				int startc=starty;
				int endc=starty;//endy+1;//IPPU.MainColorLines[1];
				int i=0;
					for(;i<IPPU.MainColorCount-1;i++)
				{
					endc= IPPU.MainColorLines[i+1];
					sceGuScissor(0,startc,256,endc);
					sceGuClearColor((((IPPU.MainColorLog [i])&31)<<3)|(((IPPU.MainColorLog [i]>>5)&31)<<11)|(((IPPU.MainColorLog [i]>>10)&31)<<19));
	   				sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
					startc=endc;
				}
				if(endc<endy+1)
				{
					sceGuScissor(0,endc,256,endy+1);
					sceGuClearColor((((IPPU.MainColorLog [i])&31)<<3)|(((IPPU.MainColorLog [i]>>5)&31)<<11)|(((IPPU.MainColorLog [i]>>10)&31)<<19));
	   				sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
				}
				sceGuScissor(0,starty,256,endy+1);
				sceGuClearDepth(0);//MAIN_SCREEN_DEPTH);//0??? seiken3 dialog
			}
			else
			{
				sceGuScissor(0,starty,256,endy/*-starty*/+1);
				sceGuClearColor(col);	   	
	   			//clear Z buffer, TODO : check if Zbuffer clear is needed each time (ADD_OR_SUB(5) only since no colour window ?)
	  			sceGuClearDepth(0);//MAIN_SCREEN_DEPTH);//0??? seiken3 dialog
	   			sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
			}
		  }
		  
		  if (os9x_easy) {		  			  			  			  			  	
		  	pspRenderScreenFix (GPUPack.GFX.Screen, true, SUB_SCREEN_DEPTH,0);		  	
		  	pspRenderScreenFix (GPUPack.GFX.Screen, false, MAIN_SCREEN_DEPTH,0);
		  } else	{		  	
		  	pspRenderScreenFix (GPUPack.GFX.Screen, false, SUB_SCREEN_DEPTH,0);
		  }		  		  	  			  
		  endRenderingFix();
		}
	}	
	

  IPPU.PreviousLine = IPPU.CurrentLine;
}
void pspDrawBackgroundOffsetFix (uint32 BGMode, uint32 bg, uint8 Z1, uint8 Z2)
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


  if (last_palette!=GPUPack.BG.DirectColourMode){  	 
			//DirectColourMode has changed, so we have to reload palette
			last_palette=GPUPack.BG.DirectColourMode;
			clut = (u16*)NO_CPU_CACHE(&clut256[0]);
			memcpy(clut,GPUPack.GFX.ScreenColors,256*2);			
			if (rendering_beware_fix16)
			for (int i=0;i<256;i++) {
				if (clut[i]==fixedcol16) clut[i]^=1<<10; //swap blue 1st bit
			}
			clut[0]|=0x8000;
  		sceGuClutLoad(256/16,clut);			
	}
  
  //bg is 32x30 tile but needs more in case of clipping or scroll changing every line
	int num_vert=((GPUPack.GFX.EndY-GPUPack.GFX.StartY+1)+2+pCurrentClipFix->GroupCount [bg])*(256/8+6);			
  if (!(vertices[current_bitshift]=(struct Vertex*)sceGuGetMemory(num_vert*2* sizeof(struct Vertex)))){
  	debug_log("not enough vertices!");
  	return;
  }
	vertices_ptr[current_bitshift]=vertices[current_bitshift];
	vertices_end[current_bitshift]=vertices_ptr[current_bitshift]+num_vert*2;
		
/********************************************************/
/********************************************************/


    int VOffsetOffset = BGMode == 4 ? 0 : 32;
    uint8 depths [2] = {Z1, Z2};

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

	if((SC1-(unsigned short*)VRAM)>0x10000)
		SC1-=0x10000;


    if (PPU.BG[bg].SCSize & 2)
	SC2 = SC1 + 1024;
    else
	SC2 = SC0;

	if((SC2-(unsigned short*)VRAM)>0x10000)
		SC2-=0x10000;


    if (PPU.BG[bg].SCSize & 1)
	SC3 = SC2 + 1024;
    else
	SC3 = SC2;

	if((SC3-(unsigned short*)VRAM)>0x10000)
		SC3-=0x10000;


    /*static const */int Lines = 1;
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

	uint32 Y = GPUPack.GFX.StartY;
	for (uint32 g = 0; g < pCurrentClipFix->GroupCount [bg]; g++) 
	{		
	Y=pCurrentClipFix->Start [bg][g];

 	for (; Y <= GPUPack.GFX.EndY && Y <= pCurrentClipFix->End [bg][g]; Y += Lines)
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

	    int Xt = Left;// * 1 + Y * GPUPack.GFX.PPL;
		int Yt =Y;
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
		VirtAlign = ((Y + VOffset) & 7)/* << 3*/;
		ScreenLine = (VOffset + Y) >> OffsetShift;

		//for (Lines = 1; Lines < 8 - VirtAlign; Lines++)
		//	if ((LineData [Y].BG[bg].VOffset != LineData [Y + Lines].BG[bg].VOffset) ||	(LineData [Y].BG[bg].HOffset != LineData [Y + Lines].BG[bg].HOffset)) break;

		//if (Y + Lines > GPUPack.GFX.EndY) Lines = GPUPack.GFX.EndY + 1 - Y;

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

		Xt -= Offset;
		Tile = READ_2BYTES(t);
		GPUPack.GFX.Z1 = GPUPack.GFX.Z2 = depths [(Tile & 0x2000) >> 13];
		short realZ2=(short)GPUPack.GFX.Z2;
		if (GPUPack.BG.TileSize == 8)
			pspDrawClippedTile16_order (Tile, Xt,Yt, Offset, Count, VirtAlign, Lines,realZ2);
		    //(*DrawClippedTilePtr)    (Tile,     s, Offset, Count, VirtAlign, Lines);
		else
		{
			if (!(Tile & (V_FLIP | H_FLIP)))
			{
					// Normal, unflipped
					pspDrawClippedTile16_order(Tile + t1 + (Quot & 1),Xt,Yt, Offset, Count, VirtAlign, Lines,realZ2);
					//(*DrawClippedTilePtr)   (Tile + t1 + (Quot & 1), s,    Offset, Count, VirtAlign, Lines);
		    } else if (Tile & H_FLIP) 
			{
				if (Tile & V_FLIP) 
				{
			    	// H & V flip
			    	pspDrawClippedTile16_order (Tile + t2 + 1 - (Quot & 1),Xt,Yt, Offset, Count, VirtAlign, Lines,realZ2);
					//(*DrawClippedTilePtr)    (Tile + t2 + 1 - (Quot & 1),    s, Offset, Count, VirtAlign, Lines);
				} else
				{
			    	// H flip only
			    	pspDrawClippedTile16_order (Tile + t1 + 1 - (Quot & 1),Xt,Yt, Offset, Count, VirtAlign, Lines,realZ2);
					//(*DrawClippedTilePtr)    (Tile + t1 + 1 - (Quot & 1),	   s, Offset, Count, VirtAlign, Lines);
				}
		    } else 
			{
				// V flip only
				pspDrawClippedTile16_order (Tile + t2 + (Quot & 1), Xt,Yt,Offset, Count, VirtAlign, Lines,realZ2);
			}
		}

		Left += Count;
		TotalCount += Count;
		Xt += (Offset + Count) * 1;
		//Xt+=8;
		MaxCount = 8;
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