#include "snes9x.h"

#include "memmap.h"
#include "ppu.h"
#include "cpuexec.h"
#include "display.h"
#include "gfx.h"
#include "apu.h"

// psp optimized by ruka. original yoyo

typedef union
{
    struct { uint8 b0,b1,b2,b3; } B;
    uint32 W;
} yo_uint32;

uint8 __attribute__((aligned(64))) tmpCache[64];


__inline uint8 softConvertTile16New (uint8 *pCache,uint32 TileAddr,uint16 *ScreenColors)
{
    register uint8 *tp = &VRAM[TileAddr];
    uint8 *p = tmpCache;
    uint32 non_zero,tile_opaque,tile_mono;
    uint8 line;
    yo_uint32 p1;
    yo_uint32 p2;
    register uint8 pix;
	    
    non_zero=0;	
    switch (GPUPack.BG.BitShift)
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
	    p[8]  = p1.B.b1;
	    p[16] = p1.B.b2;
	    p[24] = p1.B.b3;
	    p[32] = p2.B.b0;
	    p[40] = p2.B.b1;
	    p[48] = p2.B.b2;
	    p[56] = p2.B.b3;
	    
	    p++;
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
	    p[8]  = p1.B.b1;
	    p[16] = p1.B.b2;
	    p[24] = p1.B.b3;
	    p[32] = p2.B.b0;
	    p[40] = p2.B.b1;
	    p[48] = p2.B.b2;
	    p[56] = p2.B.b3;
	    
	    p++;
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
	    p[8]  = p1.B.b1;
	    p[16] = p1.B.b2;
	    p[24] = p1.B.b3;
	    p[32] = p2.B.b0;
	    p[40] = p2.B.b1;
	    p[48] = p2.B.b2;
	    p[56] = p2.B.b3;
	    
	    p++;
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
__inline void NORMAL16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.S + Offset;    
   	    /*
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
*/
   	    if (!(solid&0x80)) Screen[   0]=Pixels[0];
   	    if (!(solid&0x40)) Screen[ 256]=Pixels[1];
   	    if (!(solid&0x20)) Screen[ 512]=Pixels[2];
   	    if (!(solid&0x10)) Screen[ 768]=Pixels[3];
   	    if (!(solid&0x08)) Screen[1024]=Pixels[4];
   	    if (!(solid&0x04)) Screen[1280]=Pixels[5];
   	    if (!(solid&0x02)) Screen[1536]=Pixels[6];
   	    if (!(solid&0x01)) Screen[1792]=Pixels[7];
}

__inline void NORMAL16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.S + Offset;    
   	    
/*   	    *Screen=*Pixels;
   	    Screen+=256; Pixels++;
   	    *Screen=*Pixels;
   	    Screen+=256; Pixels++;
   	    *Screen=*Pixels;
   	    Screen+=256; Pixels++;
   	    *Screen=*Pixels;
   	    Screen+=256; Pixels++;
   	    *Screen=*Pixels;
   	    Screen+=256; Pixels++;
   	    *Screen=*Pixels;
   	    Screen+=256; Pixels++;
   	    *Screen=*Pixels;
   	    Screen+=256; Pixels++;
   	    *Screen=*Pixels;
		*/
   	    Screen[   0]=Pixels[0];
   	    Screen[ 256]=Pixels[1];
   	    Screen[ 512]=Pixels[2];
   	    Screen[ 768]=Pixels[3];
   	    Screen[1024]=Pixels[4];
   	    Screen[1280]=Pixels[5];
   	    Screen[1536]=Pixels[6];
   	    Screen[1792]=Pixels[7];
		

}

__inline void FLIPPED16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
/*
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.S + Offset + 7*256;
   	    
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
*/
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.S + Offset;
   	    if (!(solid&0x80)) Screen[1792]=Pixels[0];
   	    if (!(solid&0x40)) Screen[1536]=Pixels[1];
   	    if (!(solid&0x20)) Screen[1280]=Pixels[2];
   	    if (!(solid&0x10)) Screen[1024]=Pixels[3];
   	    if (!(solid&0x08)) Screen[ 768]=Pixels[4];
   	    if (!(solid&0x04)) Screen[ 512]=Pixels[5];
   	    if (!(solid&0x02)) Screen[ 256]=Pixels[6];
   	    if (!(solid&0x01)) Screen[   0]=Pixels[7];
}

__inline void FLIPPED16_O (uint32 Offset,uint16 *Pixels)
{
/*   	    uint16 *Screen = (uint16 *) GPUPack.GFX.S + Offset + 7*256;
    	*Screen=*Pixels;
   	    Screen-=256; Pixels++;
   	    *Screen=*Pixels;
   	    Screen-=256; Pixels++;
   	    *Screen=*Pixels;
   	    Screen-=256; Pixels++;
   	    *Screen=*Pixels;
   	    Screen-=256; Pixels++;
   	    *Screen=*Pixels;
   	    Screen-=256; Pixels++;
   	    *Screen=*Pixels;
   	    Screen-=256; Pixels++;
   	    *Screen=*Pixels;
   	    Screen-=256; Pixels++;
   	    *Screen=*Pixels;
		*/

   	    uint16 *Screen = (uint16 *) GPUPack.GFX.S + Offset;
   	    Screen[1792]=Pixels[0];
   	    Screen[1536]=Pixels[1];
   	    Screen[1280]=Pixels[2];
   	    Screen[1024]=Pixels[3];
   	    Screen[ 768]=Pixels[4];
   	    Screen[ 512]=Pixels[5];
   	    Screen[ 256]=Pixels[6];
   	    Screen[   0]=Pixels[7];
		
}



__inline void NORMAL16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.S + Offset;    
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
/*   	    
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
		*/

   	    if ((!(solid&0x80))&&(ZB[   0]>index_spr)) { Screen[   0] = Pixels[0]; ZB[   0]=index_spr; }
   	    if ((!(solid&0x40))&&(ZB[ 256]>index_spr)) { Screen[ 256] = Pixels[1]; ZB[ 256]=index_spr; }
   	    if ((!(solid&0x20))&&(ZB[ 512]>index_spr)) { Screen[ 512] = Pixels[2]; ZB[ 512]=index_spr; }
   	    if ((!(solid&0x10))&&(ZB[ 768]>index_spr)) { Screen[ 768] = Pixels[3]; ZB[ 768]=index_spr; }
   	    if ((!(solid&0x08))&&(ZB[1024]>index_spr)) { Screen[1024] = Pixels[4]; ZB[1024]=index_spr; }
   	    if ((!(solid&0x04))&&(ZB[1280]>index_spr)) { Screen[1280] = Pixels[5]; ZB[1280]=index_spr; }
   	    if ((!(solid&0x02))&&(ZB[1536]>index_spr)) { Screen[1536] = Pixels[6]; ZB[1536]=index_spr; }
   	    if ((!(solid&0x01))&&(ZB[1792]>index_spr)) { Screen[1792] = Pixels[7]; ZB[1792]=index_spr; }

}

__inline void NORMAL16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.S + Offset;    
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
/*   	    
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
		*/

   	    if (ZB[   0]>index_spr) { Screen[   0] = Pixels[0]; ZB[   0]=index_spr; }
   	    if (ZB[ 256]>index_spr) { Screen[ 256] = Pixels[1]; ZB[ 256]=index_spr; }
   	    if (ZB[ 512]>index_spr) { Screen[ 512] = Pixels[2]; ZB[ 512]=index_spr; }
   	    if (ZB[ 768]>index_spr) { Screen[ 768] = Pixels[3]; ZB[ 768]=index_spr; }
   	    if (ZB[1024]>index_spr) { Screen[1024] = Pixels[4]; ZB[1024]=index_spr; }
   	    if (ZB[1280]>index_spr) { Screen[1280] = Pixels[5]; ZB[1280]=index_spr; }
   	    if (ZB[1536]>index_spr) { Screen[1536] = Pixels[6]; ZB[1536]=index_spr; }
   	    if (ZB[1792]>index_spr) { Screen[1792] = Pixels[7]; ZB[1792]=index_spr; }
}

__inline void FLIPPED16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
/*
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.S + Offset + 7*256;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset + 7*256;
   	    
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
		*/
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.S + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
   	    if ((!(solid&0x80))&&(ZB[1792]>index_spr)) { Screen[1792] = Pixels[0]; ZB[1792]=index_spr; }
   	    if ((!(solid&0x40))&&(ZB[1536]>index_spr)) { Screen[1536] = Pixels[1]; ZB[1536]=index_spr; }
   	    if ((!(solid&0x20))&&(ZB[1280]>index_spr)) { Screen[1280] = Pixels[2]; ZB[1280]=index_spr; }
   	    if ((!(solid&0x10))&&(ZB[1024]>index_spr)) { Screen[1024] = Pixels[3]; ZB[1024]=index_spr; }
   	    if ((!(solid&0x08))&&(ZB[ 768]>index_spr)) { Screen[ 768] = Pixels[4]; ZB[ 768]=index_spr; }
   	    if ((!(solid&0x04))&&(ZB[ 512]>index_spr)) { Screen[ 512] = Pixels[5]; ZB[ 512]=index_spr; }
   	    if ((!(solid&0x02))&&(ZB[ 256]>index_spr)) { Screen[ 256] = Pixels[6]; ZB[ 256]=index_spr; }
   	    if ((!(solid&0x01))&&(ZB[   0]>index_spr)) { Screen[   0] = Pixels[7]; ZB[   0]=index_spr; }

}

__inline void FLIPPED16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
/*   	    uint16 *Screen = (uint16 *) GPUPack.GFX.S + Offset + 7*256;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset + 7*256;
   	    
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
		*/

   	    uint16 *Screen = (uint16 *) GPUPack.GFX.S + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
   	    if (ZB[1792]>index_spr) { Screen[1792] = Pixels[0]; ZB[1792]=index_spr; }
   	    if (ZB[1536]>index_spr) { Screen[1536] = Pixels[1]; ZB[1536]=index_spr; }
   	    if (ZB[1280]>index_spr) { Screen[1280] = Pixels[2]; ZB[1280]=index_spr; }
   	    if (ZB[1024]>index_spr) { Screen[1024] = Pixels[3]; ZB[1024]=index_spr; }
   	    if (ZB[ 768]>index_spr) { Screen[ 768] = Pixels[4]; ZB[ 768]=index_spr; }
   	    if (ZB[ 512]>index_spr) { Screen[ 512] = Pixels[5]; ZB[ 512]=index_spr; }
   	    if (ZB[ 256]>index_spr) { Screen[ 256] = Pixels[6]; ZB[ 256]=index_spr; }
   	    if (ZB[   0]>index_spr) { Screen[   0] = Pixels[7]; ZB[   0]=index_spr; }

}

__inline void NORMAL_ADD_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;       	    
/*
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen+=256; SubScreen+=256; Pixels++;   	    	   	    
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
		*/
   	    if (!(solid&0x80)) { if (SubScreen[   0]) Screen[   0] = COLOR_ADD(Pixels[0], SubScreen[   0]); else Screen[   0] = Pixels[0];}
   	    if (!(solid&0x40)) { if (SubScreen[ 256]) Screen[ 256] = COLOR_ADD(Pixels[1], SubScreen[ 256]); else Screen[ 256] = Pixels[1];}
   	    if (!(solid&0x20)) { if (SubScreen[ 512]) Screen[ 512] = COLOR_ADD(Pixels[2], SubScreen[ 512]); else Screen[ 512] = Pixels[2];}
   	    if (!(solid&0x10)) { if (SubScreen[ 768]) Screen[ 768] = COLOR_ADD(Pixels[3], SubScreen[ 768]); else Screen[ 768] = Pixels[3];}
   	    if (!(solid&0x08)) { if (SubScreen[1024]) Screen[1024] = COLOR_ADD(Pixels[4], SubScreen[1024]); else Screen[1024] = Pixels[4];}
   	    if (!(solid&0x04)) { if (SubScreen[1280]) Screen[1280] = COLOR_ADD(Pixels[5], SubScreen[1280]); else Screen[1280] = Pixels[5];}
   	    if (!(solid&0x02)) { if (SubScreen[1536]) Screen[1536] = COLOR_ADD(Pixels[6], SubScreen[1536]); else Screen[1536] = Pixels[6];}
   	    if (!(solid&0x01)) { if (SubScreen[1792]) Screen[1792] = COLOR_ADD(Pixels[7], SubScreen[1792]); else Screen[1792] = Pixels[7];}
}

__inline void NORMAL_ADD_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;       	       	    
/*
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen+=256; SubScreen+=256; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN   	    
	   	#undef FN
		*/
   	    if (SubScreen[   0]) Screen[   0] = COLOR_ADD(Pixels[0], SubScreen[   0]); else Screen[   0] = Pixels[0];
   	    if (SubScreen[ 256]) Screen[ 256] = COLOR_ADD(Pixels[1], SubScreen[ 256]); else Screen[ 256] = Pixels[1];
   	    if (SubScreen[ 512]) Screen[ 512] = COLOR_ADD(Pixels[2], SubScreen[ 512]); else Screen[ 512] = Pixels[2];
   	    if (SubScreen[ 768]) Screen[ 768] = COLOR_ADD(Pixels[3], SubScreen[ 768]); else Screen[ 768] = Pixels[3];
   	    if (SubScreen[1024]) Screen[1024] = COLOR_ADD(Pixels[4], SubScreen[1024]); else Screen[1024] = Pixels[4];
   	    if (SubScreen[1280]) Screen[1280] = COLOR_ADD(Pixels[5], SubScreen[1280]); else Screen[1280] = Pixels[5];
   	    if (SubScreen[1536]) Screen[1536] = COLOR_ADD(Pixels[6], SubScreen[1536]); else Screen[1536] = Pixels[6];
   	    if (SubScreen[1792]) Screen[1792] = COLOR_ADD(Pixels[7], SubScreen[1792]); else Screen[1792] = Pixels[7];
}

__inline void FLIPPED_ADD_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{   	    
/*   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset + 7*256;    
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset + 7*256;       	    
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen-=256; SubScreen-=256; Pixels++;   	    	   	    	
   	    	
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
		*/

   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;       	    
   	    if (!(solid&0x80)) { if (SubScreen[1792]) Screen[1792] = COLOR_ADD(Pixels[0], SubScreen[1792]); else Screen[1792] = Pixels[0];}
   	    if (!(solid&0x40)) { if (SubScreen[1536]) Screen[1536] = COLOR_ADD(Pixels[1], SubScreen[1536]); else Screen[1536] = Pixels[1];}
   	    if (!(solid&0x20)) { if (SubScreen[1280]) Screen[1280] = COLOR_ADD(Pixels[2], SubScreen[1280]); else Screen[1280] = Pixels[2];}
   	    if (!(solid&0x10)) { if (SubScreen[1024]) Screen[1024] = COLOR_ADD(Pixels[3], SubScreen[1024]); else Screen[1024] = Pixels[3];}
   	    if (!(solid&0x08)) { if (SubScreen[ 768]) Screen[ 768] = COLOR_ADD(Pixels[4], SubScreen[ 768]); else Screen[ 768] = Pixels[4];}
   	    if (!(solid&0x04)) { if (SubScreen[ 512]) Screen[ 512] = COLOR_ADD(Pixels[5], SubScreen[ 512]); else Screen[ 512] = Pixels[5];}
   	    if (!(solid&0x02)) { if (SubScreen[ 256]) Screen[ 256] = COLOR_ADD(Pixels[6], SubScreen[ 256]); else Screen[ 256] = Pixels[6];}
   	    if (!(solid&0x01)) { if (SubScreen[   0]) Screen[   0] = COLOR_ADD(Pixels[7], SubScreen[   0]); else Screen[   0] = Pixels[7];}
}

__inline void FLIPPED_ADD_16_O (uint32 Offset,uint16 *Pixels)
{
/*
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset + 7*256;    
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset +7*256;       	       	    
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen-=256; SubScreen-=256; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	#undef FN
		*/
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    if (SubScreen[1792]) Screen[1792] = COLOR_ADD(Pixels[0], SubScreen[1792]); else Screen[1792] = Pixels[0];
   	    if (SubScreen[1536]) Screen[1536] = COLOR_ADD(Pixels[1], SubScreen[1536]); else Screen[1536] = Pixels[1];
   	    if (SubScreen[1280]) Screen[1280] = COLOR_ADD(Pixels[2], SubScreen[1280]); else Screen[1280] = Pixels[2];
   	    if (SubScreen[1024]) Screen[1024] = COLOR_ADD(Pixels[3], SubScreen[1024]); else Screen[1024] = Pixels[3];
   	    if (SubScreen[ 768]) Screen[ 768] = COLOR_ADD(Pixels[4], SubScreen[ 768]); else Screen[ 768] = Pixels[4];
   	    if (SubScreen[ 512]) Screen[ 512] = COLOR_ADD(Pixels[5], SubScreen[ 512]); else Screen[ 512] = Pixels[5];
   	    if (SubScreen[ 256]) Screen[ 256] = COLOR_ADD(Pixels[6], SubScreen[ 256]); else Screen[ 256] = Pixels[6];
   	    if (SubScreen[   0]) Screen[   0] = COLOR_ADD(Pixels[7], SubScreen[   0]); else Screen[   0] = Pixels[7];
}



__inline void NORMAL_ADD_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
/*   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
	   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
   	    		*ZB=index_spr; \
   	    	} \
   	    	Screen+=256; SubScreen+=256; Pixels++; ZB+=256;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
		*/

   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
   	    if (!(solid&0x80)&&(ZB[   0]>index_spr)) { if (SubScreen[   0]) Screen[   0] = COLOR_ADD(Pixels[0], SubScreen[   0]); else Screen[   0] = Pixels[0]; ZB[   0]=index_spr;}
   	    if (!(solid&0x40)&&(ZB[ 256]>index_spr)) { if (SubScreen[ 256]) Screen[ 256] = COLOR_ADD(Pixels[1], SubScreen[ 256]); else Screen[ 256] = Pixels[1]; ZB[ 256]=index_spr;}
   	    if (!(solid&0x20)&&(ZB[ 512]>index_spr)) { if (SubScreen[ 512]) Screen[ 512] = COLOR_ADD(Pixels[2], SubScreen[ 512]); else Screen[ 512] = Pixels[2]; ZB[ 512]=index_spr;}
   	    if (!(solid&0x10)&&(ZB[ 768]>index_spr)) { if (SubScreen[ 768]) Screen[ 768] = COLOR_ADD(Pixels[3], SubScreen[ 768]); else Screen[ 768] = Pixels[3]; ZB[ 768]=index_spr;}
   	    if (!(solid&0x08)&&(ZB[1024]>index_spr)) { if (SubScreen[1024]) Screen[1024] = COLOR_ADD(Pixels[4], SubScreen[1024]); else Screen[1024] = Pixels[4]; ZB[1024]=index_spr;}
   	    if (!(solid&0x04)&&(ZB[1280]>index_spr)) { if (SubScreen[1280]) Screen[1280] = COLOR_ADD(Pixels[5], SubScreen[1280]); else Screen[1280] = Pixels[5]; ZB[1280]=index_spr;}
   	    if (!(solid&0x02)&&(ZB[1536]>index_spr)) { if (SubScreen[1536]) Screen[1536] = COLOR_ADD(Pixels[6], SubScreen[1536]); else Screen[1536] = Pixels[6]; ZB[1536]=index_spr;}
   	    if (!(solid&0x01)&&(ZB[1792]>index_spr)) { if (SubScreen[1792]) Screen[1792] = COLOR_ADD(Pixels[7], SubScreen[1792]); else Screen[1792] = Pixels[7]; ZB[1792]=index_spr;}
}

__inline void NORMAL_ADD_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
   	    
/*
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
	   	    Screen+=256; SubScreen+=256; Pixels++; ZB+=256;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
		*/
   	    if (ZB[   0]>index_spr) { if (SubScreen[   0]) Screen[   0] = COLOR_ADD(Pixels[0], SubScreen[   0]); else Screen[   0] = Pixels[0]; ZB[   0]=index_spr;}
   	    if (ZB[ 256]>index_spr) { if (SubScreen[ 256]) Screen[ 256] = COLOR_ADD(Pixels[1], SubScreen[ 256]); else Screen[ 256] = Pixels[1]; ZB[ 256]=index_spr;}
   	    if (ZB[ 512]>index_spr) { if (SubScreen[ 512]) Screen[ 512] = COLOR_ADD(Pixels[2], SubScreen[ 512]); else Screen[ 512] = Pixels[2]; ZB[ 512]=index_spr;}
   	    if (ZB[ 768]>index_spr) { if (SubScreen[ 768]) Screen[ 768] = COLOR_ADD(Pixels[3], SubScreen[ 768]); else Screen[ 768] = Pixels[3]; ZB[ 768]=index_spr;}
   	    if (ZB[1024]>index_spr) { if (SubScreen[1024]) Screen[1024] = COLOR_ADD(Pixels[4], SubScreen[1024]); else Screen[1024] = Pixels[4]; ZB[1024]=index_spr;}
   	    if (ZB[1280]>index_spr) { if (SubScreen[1280]) Screen[1280] = COLOR_ADD(Pixels[5], SubScreen[1280]); else Screen[1280] = Pixels[5]; ZB[1280]=index_spr;}
   	    if (ZB[1536]>index_spr) { if (SubScreen[1536]) Screen[1536] = COLOR_ADD(Pixels[6], SubScreen[1536]); else Screen[1536] = Pixels[6]; ZB[1536]=index_spr;}
   	    if (ZB[1792]>index_spr) { if (SubScreen[1792]) Screen[1792] = COLOR_ADD(Pixels[7], SubScreen[1792]); else Screen[1792] = Pixels[7]; ZB[1792]=index_spr;}
}

__inline void FLIPPED_ADD_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
/*
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset + 7*256;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset + 7*256;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset + 7*256;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
   	    		if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   		*ZB=index_spr; }\
   	    	Screen-=256; SubScreen-=256; Pixels++; ZB-=256;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
		*/
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
   	    if (!(solid&0x80)&&(ZB[1792]>index_spr)) { if (SubScreen[1792]) Screen[1792] = COLOR_ADD(Pixels[0], SubScreen[1792]); else Screen[1792] = Pixels[0]; ZB[1792]=index_spr;}
   	    if (!(solid&0x40)&&(ZB[1536]>index_spr)) { if (SubScreen[1536]) Screen[1536] = COLOR_ADD(Pixels[1], SubScreen[1536]); else Screen[1536] = Pixels[1]; ZB[1536]=index_spr;}
   	    if (!(solid&0x20)&&(ZB[1280]>index_spr)) { if (SubScreen[1280]) Screen[1280] = COLOR_ADD(Pixels[2], SubScreen[1280]); else Screen[1280] = Pixels[2]; ZB[1280]=index_spr;}
   	    if (!(solid&0x10)&&(ZB[1024]>index_spr)) { if (SubScreen[1024]) Screen[1024] = COLOR_ADD(Pixels[3], SubScreen[1024]); else Screen[1024] = Pixels[3]; ZB[1024]=index_spr;}
   	    if (!(solid&0x08)&&(ZB[ 768]>index_spr)) { if (SubScreen[ 768]) Screen[ 768] = COLOR_ADD(Pixels[4], SubScreen[ 768]); else Screen[ 768] = Pixels[4]; ZB[ 768]=index_spr;}
   	    if (!(solid&0x04)&&(ZB[ 512]>index_spr)) { if (SubScreen[ 512]) Screen[ 512] = COLOR_ADD(Pixels[5], SubScreen[ 512]); else Screen[ 512] = Pixels[5]; ZB[ 512]=index_spr;}
   	    if (!(solid&0x02)&&(ZB[ 256]>index_spr)) { if (SubScreen[ 256]) Screen[ 256] = COLOR_ADD(Pixels[6], SubScreen[ 256]); else Screen[ 256] = Pixels[6]; ZB[ 256]=index_spr;}
   	    if (!(solid&0x01)&&(ZB[   0]>index_spr)) { if (SubScreen[   0]) Screen[   0] = COLOR_ADD(Pixels[7], SubScreen[   0]); else Screen[   0] = Pixels[7]; ZB[   0]=index_spr;}
}

__inline void FLIPPED_ADD_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
/*
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset + 7*256;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset + 7*256;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset + 7*256;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
   		    Screen-=256; SubScreen-=256; Pixels++; ZB-=256;
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		#undef FN
		*/
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
   	    if (ZB[1792]>index_spr) { if (SubScreen[1792]) Screen[1792] = COLOR_ADD(Pixels[0], SubScreen[1792]); else Screen[1792] = Pixels[0]; ZB[1792]=index_spr;}
   	    if (ZB[1536]>index_spr) { if (SubScreen[1536]) Screen[1536] = COLOR_ADD(Pixels[1], SubScreen[1536]); else Screen[1536] = Pixels[1]; ZB[1536]=index_spr;}
   	    if (ZB[1280]>index_spr) { if (SubScreen[1280]) Screen[1280] = COLOR_ADD(Pixels[2], SubScreen[1280]); else Screen[1280] = Pixels[2]; ZB[1280]=index_spr;}
   	    if (ZB[1024]>index_spr) { if (SubScreen[1024]) Screen[1024] = COLOR_ADD(Pixels[3], SubScreen[1024]); else Screen[1024] = Pixels[3]; ZB[1024]=index_spr;}
   	    if (ZB[ 768]>index_spr) { if (SubScreen[ 768]) Screen[ 768] = COLOR_ADD(Pixels[4], SubScreen[ 768]); else Screen[ 768] = Pixels[4]; ZB[ 768]=index_spr;}
   	    if (ZB[ 512]>index_spr) { if (SubScreen[ 512]) Screen[ 512] = COLOR_ADD(Pixels[5], SubScreen[ 512]); else Screen[ 512] = Pixels[5]; ZB[ 512]=index_spr;}
   	    if (ZB[ 256]>index_spr) { if (SubScreen[ 256]) Screen[ 256] = COLOR_ADD(Pixels[6], SubScreen[ 256]); else Screen[ 256] = Pixels[6]; ZB[ 256]=index_spr;}
   	    if (ZB[   0]>index_spr) { if (SubScreen[   0]) Screen[   0] = COLOR_ADD(Pixels[7], SubScreen[   0]); else Screen[   0] = Pixels[7]; ZB[   0]=index_spr;}
}


__inline void NORMAL_ADD1_2_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;       	    
/*
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen+=256; SubScreen+=256; Pixels++;   	    	   	    
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
		*/
   	    if (!(solid&0x80)) { if (SubScreen[   0]) Screen[   0] = COLOR_ADD(Pixels[0], SubScreen[   0]); else Screen[   0] = Pixels[0];}
   	    if (!(solid&0x40)) { if (SubScreen[ 256]) Screen[ 256] = COLOR_ADD(Pixels[1], SubScreen[ 256]); else Screen[ 256] = Pixels[1];}
   	    if (!(solid&0x20)) { if (SubScreen[ 512]) Screen[ 512] = COLOR_ADD(Pixels[2], SubScreen[ 512]); else Screen[ 512] = Pixels[2];}
   	    if (!(solid&0x10)) { if (SubScreen[ 768]) Screen[ 768] = COLOR_ADD(Pixels[3], SubScreen[ 768]); else Screen[ 768] = Pixels[3];}
   	    if (!(solid&0x08)) { if (SubScreen[1024]) Screen[1024] = COLOR_ADD(Pixels[4], SubScreen[1024]); else Screen[1024] = Pixels[4];}
   	    if (!(solid&0x04)) { if (SubScreen[1280]) Screen[1280] = COLOR_ADD(Pixels[5], SubScreen[1280]); else Screen[1280] = Pixels[5];}
   	    if (!(solid&0x02)) { if (SubScreen[1536]) Screen[1536] = COLOR_ADD(Pixels[6], SubScreen[1536]); else Screen[1536] = Pixels[6];}
   	    if (!(solid&0x01)) { if (SubScreen[1792]) Screen[1792] = COLOR_ADD(Pixels[7], SubScreen[1792]); else Screen[1792] = Pixels[7];}
}

__inline void NORMAL_ADD1_2_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;       	       	    
/*   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen+=256; SubScreen+=256; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN   	    
	   	#undef FN
		*/

   	    if (SubScreen[   0]) Screen[   0] = COLOR_ADD1_2(Pixels[0], SubScreen[   0]); else Screen[   0] = Pixels[0];
   	    if (SubScreen[ 256]) Screen[ 256] = COLOR_ADD1_2(Pixels[1], SubScreen[ 256]); else Screen[ 256] = Pixels[1];
   	    if (SubScreen[ 512]) Screen[ 512] = COLOR_ADD1_2(Pixels[2], SubScreen[ 512]); else Screen[ 512] = Pixels[2];
   	    if (SubScreen[ 768]) Screen[ 768] = COLOR_ADD1_2(Pixels[3], SubScreen[ 768]); else Screen[ 768] = Pixels[3];
   	    if (SubScreen[1024]) Screen[1024] = COLOR_ADD1_2(Pixels[4], SubScreen[1024]); else Screen[1024] = Pixels[4];
   	    if (SubScreen[1280]) Screen[1280] = COLOR_ADD1_2(Pixels[5], SubScreen[1280]); else Screen[1280] = Pixels[5];
   	    if (SubScreen[1536]) Screen[1536] = COLOR_ADD1_2(Pixels[6], SubScreen[1536]); else Screen[1536] = Pixels[6];
   	    if (SubScreen[1792]) Screen[1792] = COLOR_ADD1_2(Pixels[7], SubScreen[1792]); else Screen[1792] = Pixels[7];
}

__inline void FLIPPED_ADD1_2_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{   	    
/*   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset + 7*256;    
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset + 7*256;       	    
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen-=256; SubScreen-=256; Pixels++;   	    	   	    	
   	    	
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
		*/

   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    if (!(solid&0x80)) {if (SubScreen[1792]) Screen[1792] = COLOR_ADD1_2(Pixels[0], SubScreen[1792]); else Screen[1792] = Pixels[0];}
   	    if (!(solid&0x40)) {if (SubScreen[1536]) Screen[1536] = COLOR_ADD1_2(Pixels[1], SubScreen[1536]); else Screen[1536] = Pixels[1];}
   	    if (!(solid&0x20)) {if (SubScreen[1280]) Screen[1280] = COLOR_ADD1_2(Pixels[2], SubScreen[1280]); else Screen[1280] = Pixels[2];}
   	    if (!(solid&0x10)) {if (SubScreen[1024]) Screen[1024] = COLOR_ADD1_2(Pixels[3], SubScreen[1024]); else Screen[1024] = Pixels[3];}
   	    if (!(solid&0x08)) {if (SubScreen[ 768]) Screen[ 768] = COLOR_ADD1_2(Pixels[4], SubScreen[ 768]); else Screen[ 768] = Pixels[4];}
   	    if (!(solid&0x04)) {if (SubScreen[ 512]) Screen[ 512] = COLOR_ADD1_2(Pixels[5], SubScreen[ 512]); else Screen[ 512] = Pixels[5];}
   	    if (!(solid&0x02)) {if (SubScreen[ 256]) Screen[ 256] = COLOR_ADD1_2(Pixels[6], SubScreen[ 256]); else Screen[ 256] = Pixels[6];}
   	    if (!(solid&0x01)) {if (SubScreen[   0]) Screen[   0] = COLOR_ADD1_2(Pixels[7], SubScreen[   0]); else Screen[   0] = Pixels[7];}
}

__inline void FLIPPED_ADD1_2_16_O (uint32 Offset,uint16 *Pixels)
{
/*   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset +7*256;    
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset +7*256;       	       	    
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen-=256; SubScreen-=256; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	#undef FN
		*/

   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    if (SubScreen[1792]) Screen[1792] = COLOR_ADD1_2(Pixels[0], SubScreen[1792]); else Screen[1792] = Pixels[0];
   	    if (SubScreen[1536]) Screen[1536] = COLOR_ADD1_2(Pixels[1], SubScreen[1536]); else Screen[1536] = Pixels[1];
   	    if (SubScreen[1280]) Screen[1280] = COLOR_ADD1_2(Pixels[2], SubScreen[1280]); else Screen[1280] = Pixels[2];
   	    if (SubScreen[1024]) Screen[1024] = COLOR_ADD1_2(Pixels[3], SubScreen[1024]); else Screen[1024] = Pixels[3];
   	    if (SubScreen[ 768]) Screen[ 768] = COLOR_ADD1_2(Pixels[4], SubScreen[ 768]); else Screen[ 768] = Pixels[4];
   	    if (SubScreen[ 512]) Screen[ 512] = COLOR_ADD1_2(Pixels[5], SubScreen[ 512]); else Screen[ 512] = Pixels[5];
   	    if (SubScreen[ 256]) Screen[ 256] = COLOR_ADD1_2(Pixels[6], SubScreen[ 256]); else Screen[ 256] = Pixels[6];
   	    if (SubScreen[   0]) Screen[   0] = COLOR_ADD1_2(Pixels[7], SubScreen[   0]); else Screen[   0] = Pixels[7];
}



__inline void NORMAL_ADD1_2_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
/*   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
	   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
   	    		*ZB=index_spr; \
   	    	} \
   	    	Screen+=256; SubScreen+=256; Pixels++; ZB+=256;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
*/
   	    if (!(solid&0x80)&&(ZB[   0]>index_spr)) { if (SubScreen[   0]) Screen[   0] = COLOR_ADD1_2(Pixels[0], SubScreen[   0]); else Screen[   0] = Pixels[0]; ZB[   0]=index_spr;}
   	    if (!(solid&0x40)&&(ZB[ 256]>index_spr)) { if (SubScreen[ 256]) Screen[ 256] = COLOR_ADD1_2(Pixels[1], SubScreen[ 256]); else Screen[ 256] = Pixels[1]; ZB[ 256]=index_spr;}
   	    if (!(solid&0x20)&&(ZB[ 512]>index_spr)) { if (SubScreen[ 512]) Screen[ 512] = COLOR_ADD1_2(Pixels[2], SubScreen[ 512]); else Screen[ 512] = Pixels[2]; ZB[ 512]=index_spr;}
   	    if (!(solid&0x10)&&(ZB[ 768]>index_spr)) { if (SubScreen[ 768]) Screen[ 768] = COLOR_ADD1_2(Pixels[3], SubScreen[ 768]); else Screen[ 768] = Pixels[3]; ZB[ 768]=index_spr;}
   	    if (!(solid&0x08)&&(ZB[1024]>index_spr)) { if (SubScreen[1024]) Screen[1024] = COLOR_ADD1_2(Pixels[4], SubScreen[1024]); else Screen[1024] = Pixels[4]; ZB[1024]=index_spr;}
   	    if (!(solid&0x04)&&(ZB[1280]>index_spr)) { if (SubScreen[1280]) Screen[1280] = COLOR_ADD1_2(Pixels[5], SubScreen[1280]); else Screen[1280] = Pixels[5]; ZB[1280]=index_spr;}
   	    if (!(solid&0x02)&&(ZB[1536]>index_spr)) { if (SubScreen[1536]) Screen[1536] = COLOR_ADD1_2(Pixels[6], SubScreen[1536]); else Screen[1536] = Pixels[6]; ZB[1536]=index_spr;}
   	    if (!(solid&0x01)&&(ZB[1792]>index_spr)) { if (SubScreen[1792]) Screen[1792] = COLOR_ADD1_2(Pixels[7], SubScreen[1792]); else Screen[1792] = Pixels[7]; ZB[1792]=index_spr;}
}

__inline void NORMAL_ADD1_2_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
/*   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
	   	    Screen+=256; SubScreen+=256; Pixels++; ZB+=256;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
*/
   	    if (ZB[   0]>index_spr) { if (SubScreen[   0]) Screen[   0] = COLOR_ADD1_2(Pixels[0], SubScreen[   0]); else Screen[   0] = Pixels[0]; ZB[   0]=index_spr;}
   	    if (ZB[ 256]>index_spr) { if (SubScreen[ 256]) Screen[ 256] = COLOR_ADD1_2(Pixels[1], SubScreen[ 256]); else Screen[ 256] = Pixels[1]; ZB[ 256]=index_spr;}
   	    if (ZB[ 512]>index_spr) { if (SubScreen[ 512]) Screen[ 512] = COLOR_ADD1_2(Pixels[2], SubScreen[ 512]); else Screen[ 512] = Pixels[2]; ZB[ 512]=index_spr;}
   	    if (ZB[ 768]>index_spr) { if (SubScreen[ 768]) Screen[ 768] = COLOR_ADD1_2(Pixels[3], SubScreen[ 768]); else Screen[ 768] = Pixels[3]; ZB[ 768]=index_spr;}
   	    if (ZB[1024]>index_spr) { if (SubScreen[1024]) Screen[1024] = COLOR_ADD1_2(Pixels[4], SubScreen[1024]); else Screen[1024] = Pixels[4]; ZB[1024]=index_spr;}
   	    if (ZB[1280]>index_spr) { if (SubScreen[1280]) Screen[1280] = COLOR_ADD1_2(Pixels[5], SubScreen[1280]); else Screen[1280] = Pixels[5]; ZB[1280]=index_spr;}
   	    if (ZB[1536]>index_spr) { if (SubScreen[1536]) Screen[1536] = COLOR_ADD1_2(Pixels[6], SubScreen[1536]); else Screen[1536] = Pixels[6]; ZB[1536]=index_spr;}
   	    if (ZB[1792]>index_spr) { if (SubScreen[1792]) Screen[1792] = COLOR_ADD1_2(Pixels[7], SubScreen[1792]); else Screen[1792] = Pixels[7]; ZB[1792]=index_spr;}
}

__inline void FLIPPED_ADD1_2_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
/*   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset + 7*256;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset + 7*256;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset + 7*256;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
   	    		if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   		*ZB=index_spr; }\
   	    	Screen-=256; SubScreen-=256; Pixels++; ZB-=256;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
*/
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
   	    if (!(solid&0x80)&&(ZB[1792]>index_spr)) { if (SubScreen[1792]) Screen[1792] = COLOR_ADD1_2(Pixels[0], SubScreen[1792]); else Screen[1792] = Pixels[0]; ZB[1792]=index_spr;}
   	    if (!(solid&0x40)&&(ZB[1536]>index_spr)) { if (SubScreen[1536]) Screen[1536] = COLOR_ADD1_2(Pixels[1], SubScreen[1536]); else Screen[1536] = Pixels[1]; ZB[1536]=index_spr;}
   	    if (!(solid&0x20)&&(ZB[1280]>index_spr)) { if (SubScreen[1280]) Screen[1280] = COLOR_ADD1_2(Pixels[2], SubScreen[1280]); else Screen[1280] = Pixels[2]; ZB[1280]=index_spr;}
   	    if (!(solid&0x10)&&(ZB[1024]>index_spr)) { if (SubScreen[1024]) Screen[1024] = COLOR_ADD1_2(Pixels[3], SubScreen[1024]); else Screen[1024] = Pixels[3]; ZB[1024]=index_spr;}
   	    if (!(solid&0x08)&&(ZB[ 768]>index_spr)) { if (SubScreen[ 768]) Screen[ 768] = COLOR_ADD1_2(Pixels[4], SubScreen[ 768]); else Screen[ 768] = Pixels[4]; ZB[ 768]=index_spr;}
   	    if (!(solid&0x04)&&(ZB[ 512]>index_spr)) { if (SubScreen[ 512]) Screen[ 512] = COLOR_ADD1_2(Pixels[5], SubScreen[ 512]); else Screen[ 512] = Pixels[5]; ZB[ 512]=index_spr;}
   	    if (!(solid&0x02)&&(ZB[ 256]>index_spr)) { if (SubScreen[ 256]) Screen[ 256] = COLOR_ADD1_2(Pixels[6], SubScreen[ 256]); else Screen[ 256] = Pixels[6]; ZB[ 256]=index_spr;}
   	    if (!(solid&0x01)&&(ZB[   0]>index_spr)) { if (SubScreen[   0]) Screen[   0] = COLOR_ADD1_2(Pixels[7], SubScreen[   0]); else Screen[   0] = Pixels[7]; ZB[   0]=index_spr;}
}

__inline void FLIPPED_ADD1_2_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
/*   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset + 7*256;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset + 7*256;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset + 7*256;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
   		    Screen-=256; SubScreen-=256; Pixels++; ZB-=256;
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		#undef FN
*/
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
   	    if (ZB[1792]>index_spr) { if (SubScreen[1792]) Screen[1792] = COLOR_ADD1_2(Pixels[0], SubScreen[1792]); else Screen[1792] = Pixels[0]; ZB[1792]=index_spr;}
   	    if (ZB[1536]>index_spr) { if (SubScreen[1536]) Screen[1536] = COLOR_ADD1_2(Pixels[1], SubScreen[1536]); else Screen[1536] = Pixels[1]; ZB[1536]=index_spr;}
   	    if (ZB[1280]>index_spr) { if (SubScreen[1280]) Screen[1280] = COLOR_ADD1_2(Pixels[2], SubScreen[1280]); else Screen[1280] = Pixels[2]; ZB[1280]=index_spr;}
   	    if (ZB[1024]>index_spr) { if (SubScreen[1024]) Screen[1024] = COLOR_ADD1_2(Pixels[3], SubScreen[1024]); else Screen[1024] = Pixels[3]; ZB[1024]=index_spr;}
   	    if (ZB[ 768]>index_spr) { if (SubScreen[ 768]) Screen[ 768] = COLOR_ADD1_2(Pixels[4], SubScreen[ 768]); else Screen[ 768] = Pixels[4]; ZB[ 768]=index_spr;}
   	    if (ZB[ 512]>index_spr) { if (SubScreen[ 512]) Screen[ 512] = COLOR_ADD1_2(Pixels[5], SubScreen[ 512]); else Screen[ 512] = Pixels[5]; ZB[ 512]=index_spr;}
   	    if (ZB[ 256]>index_spr) { if (SubScreen[ 256]) Screen[ 256] = COLOR_ADD1_2(Pixels[6], SubScreen[ 256]); else Screen[ 256] = Pixels[6]; ZB[ 256]=index_spr;}
   	    if (ZB[   0]>index_spr) { if (SubScreen[   0]) Screen[   0] = COLOR_ADD1_2(Pixels[7], SubScreen[   0]); else Screen[   0] = Pixels[7]; ZB[   0]=index_spr;}
}

__inline void NORMAL_SUB_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;       	    
/*   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen+=256; SubScreen+=256; Pixels++;   	    	   	    
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
*/
   	    if (!(solid&0x80)) { if (SubScreen[   0]) Screen[   0] = COLOR_SUB(Pixels[0], SubScreen[   0]); else Screen[   0] = Pixels[0];}
   	    if (!(solid&0x40)) { if (SubScreen[ 256]) Screen[ 256] = COLOR_SUB(Pixels[1], SubScreen[ 256]); else Screen[ 256] = Pixels[1];}
   	    if (!(solid&0x20)) { if (SubScreen[ 512]) Screen[ 512] = COLOR_SUB(Pixels[2], SubScreen[ 512]); else Screen[ 512] = Pixels[2];}
   	    if (!(solid&0x10)) { if (SubScreen[ 768]) Screen[ 768] = COLOR_SUB(Pixels[3], SubScreen[ 768]); else Screen[ 768] = Pixels[3];}
   	    if (!(solid&0x08)) { if (SubScreen[1024]) Screen[1024] = COLOR_SUB(Pixels[4], SubScreen[1024]); else Screen[1024] = Pixels[4];}
   	    if (!(solid&0x04)) { if (SubScreen[1280]) Screen[1280] = COLOR_SUB(Pixels[5], SubScreen[1280]); else Screen[1280] = Pixels[5];}
   	    if (!(solid&0x02)) { if (SubScreen[1536]) Screen[1536] = COLOR_SUB(Pixels[6], SubScreen[1536]); else Screen[1536] = Pixels[6];}
   	    if (!(solid&0x01)) { if (SubScreen[1792]) Screen[1792] = COLOR_SUB(Pixels[7], SubScreen[1792]); else Screen[1792] = Pixels[7];}
}

__inline void NORMAL_SUB_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;       	       	    
/*   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen+=256; SubScreen+=256; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN   	    
	   	#undef FN
*/
   	    if (SubScreen[   0]) Screen[   0] = COLOR_SUB(Pixels[0], SubScreen[   0]); else Screen[   0] = Pixels[0];
   	    if (SubScreen[ 256]) Screen[ 256] = COLOR_SUB(Pixels[1], SubScreen[ 256]); else Screen[ 256] = Pixels[1];
   	    if (SubScreen[ 512]) Screen[ 512] = COLOR_SUB(Pixels[2], SubScreen[ 512]); else Screen[ 512] = Pixels[2];
   	    if (SubScreen[ 768]) Screen[ 768] = COLOR_SUB(Pixels[3], SubScreen[ 768]); else Screen[ 768] = Pixels[3];
   	    if (SubScreen[1024]) Screen[1024] = COLOR_SUB(Pixels[4], SubScreen[1024]); else Screen[1024] = Pixels[4];
   	    if (SubScreen[1280]) Screen[1280] = COLOR_SUB(Pixels[5], SubScreen[1280]); else Screen[1280] = Pixels[5];
   	    if (SubScreen[1536]) Screen[1536] = COLOR_SUB(Pixels[6], SubScreen[1536]); else Screen[1536] = Pixels[6];
   	    if (SubScreen[1792]) Screen[1792] = COLOR_SUB(Pixels[7], SubScreen[1792]); else Screen[1792] = Pixels[7];
}

__inline void FLIPPED_SUB_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{   	    
/*   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset +7*256;    
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset+7*256;       	    
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen-=256; SubScreen-=256; Pixels++;   	    	   	    	
   	    	
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
*/
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    if (!(solid&0x80)) {if (SubScreen[1792]) Screen[1792] = COLOR_SUB(Pixels[0], SubScreen[1792]); else Screen[1792] = Pixels[0];}
   	    if (!(solid&0x40)) {if (SubScreen[1536]) Screen[1536] = COLOR_SUB(Pixels[1], SubScreen[1536]); else Screen[1536] = Pixels[1];}
   	    if (!(solid&0x20)) {if (SubScreen[1280]) Screen[1280] = COLOR_SUB(Pixels[2], SubScreen[1280]); else Screen[1280] = Pixels[2];}
   	    if (!(solid&0x10)) {if (SubScreen[1024]) Screen[1024] = COLOR_SUB(Pixels[3], SubScreen[1024]); else Screen[1024] = Pixels[3];}
   	    if (!(solid&0x08)) {if (SubScreen[ 768]) Screen[ 768] = COLOR_SUB(Pixels[4], SubScreen[ 768]); else Screen[ 768] = Pixels[4];}
   	    if (!(solid&0x04)) {if (SubScreen[ 512]) Screen[ 512] = COLOR_SUB(Pixels[5], SubScreen[ 512]); else Screen[ 512] = Pixels[5];}
   	    if (!(solid&0x02)) {if (SubScreen[ 256]) Screen[ 256] = COLOR_SUB(Pixels[6], SubScreen[ 256]); else Screen[ 256] = Pixels[6];}
   	    if (!(solid&0x01)) {if (SubScreen[   0]) Screen[   0] = COLOR_SUB(Pixels[7], SubScreen[   0]); else Screen[   0] = Pixels[7];}
}

__inline void FLIPPED_SUB_16_O (uint32 Offset,uint16 *Pixels)
{
/*   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset+7*256;    
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset+7*256;       	       	    
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen-=256; SubScreen-=256; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	#undef FN
*/
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    if (SubScreen[1792]) Screen[1792] = COLOR_SUB(Pixels[0], SubScreen[1792]); else Screen[1792] = Pixels[0];
   	    if (SubScreen[1536]) Screen[1536] = COLOR_SUB(Pixels[1], SubScreen[1536]); else Screen[1536] = Pixels[1];
   	    if (SubScreen[1280]) Screen[1280] = COLOR_SUB(Pixels[2], SubScreen[1280]); else Screen[1280] = Pixels[2];
   	    if (SubScreen[1024]) Screen[1024] = COLOR_SUB(Pixels[3], SubScreen[1024]); else Screen[1024] = Pixels[3];
   	    if (SubScreen[ 768]) Screen[ 768] = COLOR_SUB(Pixels[4], SubScreen[ 768]); else Screen[ 768] = Pixels[4];
   	    if (SubScreen[ 512]) Screen[ 512] = COLOR_SUB(Pixels[5], SubScreen[ 512]); else Screen[ 512] = Pixels[5];
   	    if (SubScreen[ 256]) Screen[ 256] = COLOR_SUB(Pixels[6], SubScreen[ 256]); else Screen[ 256] = Pixels[6];
   	    if (SubScreen[   0]) Screen[   0] = COLOR_SUB(Pixels[7], SubScreen[   0]); else Screen[   0] = Pixels[7];
}



__inline void NORMAL_SUB_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
/*
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
	   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
   	    		*ZB=index_spr; \
   	    	} \
   	    	Screen+=256; SubScreen+=256; Pixels++; ZB+=256;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
*/
   	    if (!(solid&0x80)&&(ZB[   0]>index_spr)) { if (SubScreen[   0]) Screen[   0] = COLOR_SUB(Pixels[0], SubScreen[   0]); else Screen[   0] = Pixels[0]; ZB[   0]=index_spr;}
   	    if (!(solid&0x40)&&(ZB[ 256]>index_spr)) { if (SubScreen[ 256]) Screen[ 256] = COLOR_SUB(Pixels[1], SubScreen[ 256]); else Screen[ 256] = Pixels[1]; ZB[ 256]=index_spr;}
   	    if (!(solid&0x20)&&(ZB[ 512]>index_spr)) { if (SubScreen[ 512]) Screen[ 512] = COLOR_SUB(Pixels[2], SubScreen[ 512]); else Screen[ 512] = Pixels[2]; ZB[ 512]=index_spr;}
   	    if (!(solid&0x10)&&(ZB[ 768]>index_spr)) { if (SubScreen[ 768]) Screen[ 768] = COLOR_SUB(Pixels[3], SubScreen[ 768]); else Screen[ 768] = Pixels[3]; ZB[ 768]=index_spr;}
   	    if (!(solid&0x08)&&(ZB[1024]>index_spr)) { if (SubScreen[1024]) Screen[1024] = COLOR_SUB(Pixels[4], SubScreen[1024]); else Screen[1024] = Pixels[4]; ZB[1024]=index_spr;}
   	    if (!(solid&0x04)&&(ZB[1280]>index_spr)) { if (SubScreen[1280]) Screen[1280] = COLOR_SUB(Pixels[5], SubScreen[1280]); else Screen[1280] = Pixels[5]; ZB[1280]=index_spr;}
   	    if (!(solid&0x02)&&(ZB[1536]>index_spr)) { if (SubScreen[1536]) Screen[1536] = COLOR_SUB(Pixels[6], SubScreen[1536]); else Screen[1536] = Pixels[6]; ZB[1536]=index_spr;}
   	    if (!(solid&0x01)&&(ZB[1792]>index_spr)) { if (SubScreen[1792]) Screen[1792] = COLOR_SUB(Pixels[7], SubScreen[1792]); else Screen[1792] = Pixels[7]; ZB[1792]=index_spr;}
}

__inline void NORMAL_SUB_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
/*
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
	   	    Screen+=256; SubScreen+=256; Pixels++; ZB+=256;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
*/
   	    if (ZB[   0]>index_spr) { if (SubScreen[   0]) Screen[   0] = COLOR_SUB(Pixels[0], SubScreen[   0]); else Screen[   0] = Pixels[0]; ZB[   0]=index_spr;}
   	    if (ZB[ 256]>index_spr) { if (SubScreen[ 256]) Screen[ 256] = COLOR_SUB(Pixels[1], SubScreen[ 256]); else Screen[ 256] = Pixels[1]; ZB[ 256]=index_spr;}
   	    if (ZB[ 512]>index_spr) { if (SubScreen[ 512]) Screen[ 512] = COLOR_SUB(Pixels[2], SubScreen[ 512]); else Screen[ 512] = Pixels[2]; ZB[ 512]=index_spr;}
   	    if (ZB[ 768]>index_spr) { if (SubScreen[ 768]) Screen[ 768] = COLOR_SUB(Pixels[3], SubScreen[ 768]); else Screen[ 768] = Pixels[3]; ZB[ 768]=index_spr;}
   	    if (ZB[1024]>index_spr) { if (SubScreen[1024]) Screen[1024] = COLOR_SUB(Pixels[4], SubScreen[1024]); else Screen[1024] = Pixels[4]; ZB[1024]=index_spr;}
   	    if (ZB[1280]>index_spr) { if (SubScreen[1280]) Screen[1280] = COLOR_SUB(Pixels[5], SubScreen[1280]); else Screen[1280] = Pixels[5]; ZB[1280]=index_spr;}
   	    if (ZB[1536]>index_spr) { if (SubScreen[1536]) Screen[1536] = COLOR_SUB(Pixels[6], SubScreen[1536]); else Screen[1536] = Pixels[6]; ZB[1536]=index_spr;}
   	    if (ZB[1792]>index_spr) { if (SubScreen[1792]) Screen[1792] = COLOR_SUB(Pixels[7], SubScreen[1792]); else Screen[1792] = Pixels[7]; ZB[1792]=index_spr;}
}

__inline void FLIPPED_SUB_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
/*   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset + 7*256;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset + 7*256;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset + 7*256;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
   	    		if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   		*ZB=index_spr; }\
   	    	Screen-=256; SubScreen-=256; Pixels++; ZB-=256;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
*/
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
   	    if (!(solid&0x80)&&(ZB[1792]>index_spr)) { if (SubScreen[1792]) Screen[1792] = COLOR_SUB(Pixels[0], SubScreen[1792]); else Screen[1792] = Pixels[0]; ZB[1792]=index_spr;}
   	    if (!(solid&0x40)&&(ZB[1536]>index_spr)) { if (SubScreen[1536]) Screen[1536] = COLOR_SUB(Pixels[1], SubScreen[1536]); else Screen[1536] = Pixels[1]; ZB[1536]=index_spr;}
   	    if (!(solid&0x20)&&(ZB[1280]>index_spr)) { if (SubScreen[1280]) Screen[1280] = COLOR_SUB(Pixels[2], SubScreen[1280]); else Screen[1280] = Pixels[2]; ZB[1280]=index_spr;}
   	    if (!(solid&0x10)&&(ZB[1024]>index_spr)) { if (SubScreen[1024]) Screen[1024] = COLOR_SUB(Pixels[3], SubScreen[1024]); else Screen[1024] = Pixels[3]; ZB[1024]=index_spr;}
   	    if (!(solid&0x08)&&(ZB[ 768]>index_spr)) { if (SubScreen[ 768]) Screen[ 768] = COLOR_SUB(Pixels[4], SubScreen[ 768]); else Screen[ 768] = Pixels[4]; ZB[ 768]=index_spr;}
   	    if (!(solid&0x04)&&(ZB[ 512]>index_spr)) { if (SubScreen[ 512]) Screen[ 512] = COLOR_SUB(Pixels[5], SubScreen[ 512]); else Screen[ 512] = Pixels[5]; ZB[ 512]=index_spr;}
   	    if (!(solid&0x02)&&(ZB[ 256]>index_spr)) { if (SubScreen[ 256]) Screen[ 256] = COLOR_SUB(Pixels[6], SubScreen[ 256]); else Screen[ 256] = Pixels[6]; ZB[ 256]=index_spr;}
   	    if (!(solid&0x01)&&(ZB[   0]>index_spr)) { if (SubScreen[   0]) Screen[   0] = COLOR_SUB(Pixels[7], SubScreen[   0]); else Screen[   0] = Pixels[7]; ZB[   0]=index_spr;}
}

__inline void FLIPPED_SUB_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
/*   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset + 7*256;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset + 7*256;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset + 7*256;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
   		    Screen-=256; SubScreen-=256; Pixels++; ZB-=256;
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		#undef FN
*/
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
   	    if (ZB[1792]>index_spr) { if (SubScreen[1792]) Screen[1792] = COLOR_SUB(Pixels[0], SubScreen[1792]); else Screen[1792] = Pixels[0]; ZB[1792]=index_spr;}
   	    if (ZB[1536]>index_spr) { if (SubScreen[1536]) Screen[1536] = COLOR_SUB(Pixels[1], SubScreen[1536]); else Screen[1536] = Pixels[1]; ZB[1536]=index_spr;}
   	    if (ZB[1280]>index_spr) { if (SubScreen[1280]) Screen[1280] = COLOR_SUB(Pixels[2], SubScreen[1280]); else Screen[1280] = Pixels[2]; ZB[1280]=index_spr;}
   	    if (ZB[1024]>index_spr) { if (SubScreen[1024]) Screen[1024] = COLOR_SUB(Pixels[3], SubScreen[1024]); else Screen[1024] = Pixels[3]; ZB[1024]=index_spr;}
   	    if (ZB[ 768]>index_spr) { if (SubScreen[ 768]) Screen[ 768] = COLOR_SUB(Pixels[4], SubScreen[ 768]); else Screen[ 768] = Pixels[4]; ZB[ 768]=index_spr;}
   	    if (ZB[ 512]>index_spr) { if (SubScreen[ 512]) Screen[ 512] = COLOR_SUB(Pixels[5], SubScreen[ 512]); else Screen[ 512] = Pixels[5]; ZB[ 512]=index_spr;}
   	    if (ZB[ 256]>index_spr) { if (SubScreen[ 256]) Screen[ 256] = COLOR_SUB(Pixels[6], SubScreen[ 256]); else Screen[ 256] = Pixels[6]; ZB[ 256]=index_spr;}
   	    if (ZB[   0]>index_spr) { if (SubScreen[   0]) Screen[   0] = COLOR_SUB(Pixels[7], SubScreen[   0]); else Screen[   0] = Pixels[7]; ZB[   0]=index_spr;}
}


__inline void NORMAL_SUB1_2_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;       	    
/*   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen+=256; SubScreen+=256; Pixels++;   	    	   	    
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
*/
   	    if (!(solid&0x80)) { if (SubScreen[   0]) Screen[   0] = COLOR_SUB1_2(Pixels[0], SubScreen[   0]); else Screen[   0] = Pixels[0];}
   	    if (!(solid&0x40)) { if (SubScreen[ 256]) Screen[ 256] = COLOR_SUB1_2(Pixels[1], SubScreen[ 256]); else Screen[ 256] = Pixels[1];}
   	    if (!(solid&0x20)) { if (SubScreen[ 512]) Screen[ 512] = COLOR_SUB1_2(Pixels[2], SubScreen[ 512]); else Screen[ 512] = Pixels[2];}
   	    if (!(solid&0x10)) { if (SubScreen[ 768]) Screen[ 768] = COLOR_SUB1_2(Pixels[3], SubScreen[ 768]); else Screen[ 768] = Pixels[3];}
   	    if (!(solid&0x08)) { if (SubScreen[1024]) Screen[1024] = COLOR_SUB1_2(Pixels[4], SubScreen[1024]); else Screen[1024] = Pixels[4];}
   	    if (!(solid&0x04)) { if (SubScreen[1280]) Screen[1280] = COLOR_SUB1_2(Pixels[5], SubScreen[1280]); else Screen[1280] = Pixels[5];}
   	    if (!(solid&0x02)) { if (SubScreen[1536]) Screen[1536] = COLOR_SUB1_2(Pixels[6], SubScreen[1536]); else Screen[1536] = Pixels[6];}
   	    if (!(solid&0x01)) { if (SubScreen[1792]) Screen[1792] = COLOR_SUB1_2(Pixels[7], SubScreen[1792]); else Screen[1792] = Pixels[7];}
}

__inline void NORMAL_SUB1_2_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;       	       	    
/*   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen+=256; SubScreen+=256; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN   	    
	   	#undef FN
*/
   	    if (SubScreen[   0]) Screen[   0] = COLOR_SUB1_2(Pixels[0], SubScreen[   0]); else Screen[   0] = Pixels[0];
   	    if (SubScreen[ 256]) Screen[ 256] = COLOR_SUB1_2(Pixels[1], SubScreen[ 256]); else Screen[ 256] = Pixels[1];
   	    if (SubScreen[ 512]) Screen[ 512] = COLOR_SUB1_2(Pixels[2], SubScreen[ 512]); else Screen[ 512] = Pixels[2];
   	    if (SubScreen[ 768]) Screen[ 768] = COLOR_SUB1_2(Pixels[3], SubScreen[ 768]); else Screen[ 768] = Pixels[3];
   	    if (SubScreen[1024]) Screen[1024] = COLOR_SUB1_2(Pixels[4], SubScreen[1024]); else Screen[1024] = Pixels[4];
   	    if (SubScreen[1280]) Screen[1280] = COLOR_SUB1_2(Pixels[5], SubScreen[1280]); else Screen[1280] = Pixels[5];
   	    if (SubScreen[1536]) Screen[1536] = COLOR_SUB1_2(Pixels[6], SubScreen[1536]); else Screen[1536] = Pixels[6];
   	    if (SubScreen[1792]) Screen[1792] = COLOR_SUB1_2(Pixels[7], SubScreen[1792]); else Screen[1792] = Pixels[7];
}

__inline void FLIPPED_SUB1_2_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{   	    
/*   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset +7*256;    
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset +7*256;       	    
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen-=256; SubScreen-=256; Pixels++;   	    	   	    	
   	    	
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
*/
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    if (!(solid&0x80)) {if (SubScreen[1792]) Screen[1792] = COLOR_SUB1_2(Pixels[0], SubScreen[1792]); else Screen[1792] = Pixels[0];}
   	    if (!(solid&0x40)) {if (SubScreen[1536]) Screen[1536] = COLOR_SUB1_2(Pixels[1], SubScreen[1536]); else Screen[1536] = Pixels[1];}
   	    if (!(solid&0x20)) {if (SubScreen[1280]) Screen[1280] = COLOR_SUB1_2(Pixels[2], SubScreen[1280]); else Screen[1280] = Pixels[2];}
   	    if (!(solid&0x10)) {if (SubScreen[1024]) Screen[1024] = COLOR_SUB1_2(Pixels[3], SubScreen[1024]); else Screen[1024] = Pixels[3];}
   	    if (!(solid&0x08)) {if (SubScreen[ 768]) Screen[ 768] = COLOR_SUB1_2(Pixels[4], SubScreen[ 768]); else Screen[ 768] = Pixels[4];}
   	    if (!(solid&0x04)) {if (SubScreen[ 512]) Screen[ 512] = COLOR_SUB1_2(Pixels[5], SubScreen[ 512]); else Screen[ 512] = Pixels[5];}
   	    if (!(solid&0x02)) {if (SubScreen[ 256]) Screen[ 256] = COLOR_SUB1_2(Pixels[6], SubScreen[ 256]); else Screen[ 256] = Pixels[6];}
   	    if (!(solid&0x01)) {if (SubScreen[   0]) Screen[   0] = COLOR_SUB1_2(Pixels[7], SubScreen[   0]); else Screen[   0] = Pixels[7];}
}

__inline void FLIPPED_SUB1_2_16_O (uint32 Offset,uint16 *Pixels)
{
/*   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset+7*256;    
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset+7*256;       	       	    
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen-=256; SubScreen-=256; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	#undef FN
*/
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    if (SubScreen[1792]) Screen[1792] = COLOR_SUB1_2(Pixels[0], SubScreen[1792]); else Screen[1792] = Pixels[0];
   	    if (SubScreen[1536]) Screen[1536] = COLOR_SUB1_2(Pixels[1], SubScreen[1536]); else Screen[1536] = Pixels[1];
   	    if (SubScreen[1280]) Screen[1280] = COLOR_SUB1_2(Pixels[2], SubScreen[1280]); else Screen[1280] = Pixels[2];
   	    if (SubScreen[1024]) Screen[1024] = COLOR_SUB1_2(Pixels[3], SubScreen[1024]); else Screen[1024] = Pixels[3];
   	    if (SubScreen[ 768]) Screen[ 768] = COLOR_SUB1_2(Pixels[4], SubScreen[ 768]); else Screen[ 768] = Pixels[4];
   	    if (SubScreen[ 512]) Screen[ 512] = COLOR_SUB1_2(Pixels[5], SubScreen[ 512]); else Screen[ 512] = Pixels[5];
   	    if (SubScreen[ 256]) Screen[ 256] = COLOR_SUB1_2(Pixels[6], SubScreen[ 256]); else Screen[ 256] = Pixels[6];
   	    if (SubScreen[   0]) Screen[   0] = COLOR_SUB1_2(Pixels[7], SubScreen[   0]); else Screen[   0] = Pixels[7];
}



__inline void NORMAL_SUB1_2_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
/*
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
	   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
   	    		*ZB=index_spr; \
   	    	} \
   	    	Screen+=256; SubScreen+=256; Pixels++; ZB+=256;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
*/
   	    if (!(solid&0x80)&&(ZB[   0]>index_spr)) { if (SubScreen[   0]) Screen[   0] = COLOR_SUB1_2(Pixels[0], SubScreen[   0]); else Screen[   0] = Pixels[0]; ZB[   0]=index_spr;}
   	    if (!(solid&0x40)&&(ZB[ 256]>index_spr)) { if (SubScreen[ 256]) Screen[ 256] = COLOR_SUB1_2(Pixels[1], SubScreen[ 256]); else Screen[ 256] = Pixels[1]; ZB[ 256]=index_spr;}
   	    if (!(solid&0x20)&&(ZB[ 512]>index_spr)) { if (SubScreen[ 512]) Screen[ 512] = COLOR_SUB1_2(Pixels[2], SubScreen[ 512]); else Screen[ 512] = Pixels[2]; ZB[ 512]=index_spr;}
   	    if (!(solid&0x10)&&(ZB[ 768]>index_spr)) { if (SubScreen[ 768]) Screen[ 768] = COLOR_SUB1_2(Pixels[3], SubScreen[ 768]); else Screen[ 768] = Pixels[3]; ZB[ 768]=index_spr;}
   	    if (!(solid&0x08)&&(ZB[1024]>index_spr)) { if (SubScreen[1024]) Screen[1024] = COLOR_SUB1_2(Pixels[4], SubScreen[1024]); else Screen[1024] = Pixels[4]; ZB[1024]=index_spr;}
   	    if (!(solid&0x04)&&(ZB[1280]>index_spr)) { if (SubScreen[1280]) Screen[1280] = COLOR_SUB1_2(Pixels[5], SubScreen[1280]); else Screen[1280] = Pixels[5]; ZB[1280]=index_spr;}
   	    if (!(solid&0x02)&&(ZB[1536]>index_spr)) { if (SubScreen[1536]) Screen[1536] = COLOR_SUB1_2(Pixels[6], SubScreen[1536]); else Screen[1536] = Pixels[6]; ZB[1536]=index_spr;}
   	    if (!(solid&0x01)&&(ZB[1792]>index_spr)) { if (SubScreen[1792]) Screen[1792] = COLOR_SUB1_2(Pixels[7], SubScreen[1792]); else Screen[1792] = Pixels[7]; ZB[1792]=index_spr;}
}

__inline void NORMAL_SUB1_2_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
/*
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
	   	    Screen+=256; SubScreen+=256; Pixels++; ZB+=256;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
*/
   	    if (ZB[   0]>index_spr) { if (SubScreen[   0]) Screen[   0] = COLOR_SUB1_2(Pixels[0], SubScreen[   0]); else Screen[   0] = Pixels[0]; ZB[   0]=index_spr;}
   	    if (ZB[ 256]>index_spr) { if (SubScreen[ 256]) Screen[ 256] = COLOR_SUB1_2(Pixels[1], SubScreen[ 256]); else Screen[ 256] = Pixels[1]; ZB[ 256]=index_spr;}
   	    if (ZB[ 512]>index_spr) { if (SubScreen[ 512]) Screen[ 512] = COLOR_SUB1_2(Pixels[2], SubScreen[ 512]); else Screen[ 512] = Pixels[2]; ZB[ 512]=index_spr;}
   	    if (ZB[ 768]>index_spr) { if (SubScreen[ 768]) Screen[ 768] = COLOR_SUB1_2(Pixels[3], SubScreen[ 768]); else Screen[ 768] = Pixels[3]; ZB[ 768]=index_spr;}
   	    if (ZB[1024]>index_spr) { if (SubScreen[1024]) Screen[1024] = COLOR_SUB1_2(Pixels[4], SubScreen[1024]); else Screen[1024] = Pixels[4]; ZB[1024]=index_spr;}
   	    if (ZB[1280]>index_spr) { if (SubScreen[1280]) Screen[1280] = COLOR_SUB1_2(Pixels[5], SubScreen[1280]); else Screen[1280] = Pixels[5]; ZB[1280]=index_spr;}
   	    if (ZB[1536]>index_spr) { if (SubScreen[1536]) Screen[1536] = COLOR_SUB1_2(Pixels[6], SubScreen[1536]); else Screen[1536] = Pixels[6]; ZB[1536]=index_spr;}
   	    if (ZB[1792]>index_spr) { if (SubScreen[1792]) Screen[1792] = COLOR_SUB1_2(Pixels[7], SubScreen[1792]); else Screen[1792] = Pixels[7]; ZB[1792]=index_spr;}
}

__inline void FLIPPED_SUB1_2_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
/*   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset + 7*256;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset + 7*256;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset + 7*256;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
   	    		if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   		*ZB=index_spr; }\
   	    	Screen-=256; SubScreen-=256; Pixels++; ZB-=256;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
*/
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
   	    if (!(solid&0x80)&&(ZB[1792]>index_spr)) { if (SubScreen[1792]) Screen[1792] = COLOR_SUB1_2(Pixels[0], SubScreen[1792]); else Screen[1792] = Pixels[0]; ZB[1792]=index_spr;}
   	    if (!(solid&0x40)&&(ZB[1536]>index_spr)) { if (SubScreen[1536]) Screen[1536] = COLOR_SUB1_2(Pixels[1], SubScreen[1536]); else Screen[1536] = Pixels[1]; ZB[1536]=index_spr;}
   	    if (!(solid&0x20)&&(ZB[1280]>index_spr)) { if (SubScreen[1280]) Screen[1280] = COLOR_SUB1_2(Pixels[2], SubScreen[1280]); else Screen[1280] = Pixels[2]; ZB[1280]=index_spr;}
   	    if (!(solid&0x10)&&(ZB[1024]>index_spr)) { if (SubScreen[1024]) Screen[1024] = COLOR_SUB1_2(Pixels[3], SubScreen[1024]); else Screen[1024] = Pixels[3]; ZB[1024]=index_spr;}
   	    if (!(solid&0x08)&&(ZB[ 768]>index_spr)) { if (SubScreen[ 768]) Screen[ 768] = COLOR_SUB1_2(Pixels[4], SubScreen[ 768]); else Screen[ 768] = Pixels[4]; ZB[ 768]=index_spr;}
   	    if (!(solid&0x04)&&(ZB[ 512]>index_spr)) { if (SubScreen[ 512]) Screen[ 512] = COLOR_SUB1_2(Pixels[5], SubScreen[ 512]); else Screen[ 512] = Pixels[5]; ZB[ 512]=index_spr;}
   	    if (!(solid&0x02)&&(ZB[ 256]>index_spr)) { if (SubScreen[ 256]) Screen[ 256] = COLOR_SUB1_2(Pixels[6], SubScreen[ 256]); else Screen[ 256] = Pixels[6]; ZB[ 256]=index_spr;}
   	    if (!(solid&0x01)&&(ZB[   0]>index_spr)) { if (SubScreen[   0]) Screen[   0] = COLOR_SUB1_2(Pixels[7], SubScreen[   0]); else Screen[   0] = Pixels[7]; ZB[   0]=index_spr;}
}

__inline void FLIPPED_SUB1_2_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
/*   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset + 7*256;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset + 7*256;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset + 7*256;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
   		    Screen-=256; SubScreen-=256; Pixels++; ZB-=256;
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		#undef FN
*/
   	    uint16 *Screen = (uint16 *) GPUPack.GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GPUPack.GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GPUPack.GFX.ZBuffer + Offset;
   	    if (ZB[1792]>index_spr) { if (SubScreen[1792]) Screen[1792] = COLOR_SUB1_2(Pixels[0], SubScreen[1792]); else Screen[1792] = Pixels[0]; ZB[1792]=index_spr;}
   	    if (ZB[1536]>index_spr) { if (SubScreen[1536]) Screen[1536] = COLOR_SUB1_2(Pixels[1], SubScreen[1536]); else Screen[1536] = Pixels[1]; ZB[1536]=index_spr;}
   	    if (ZB[1280]>index_spr) { if (SubScreen[1280]) Screen[1280] = COLOR_SUB1_2(Pixels[2], SubScreen[1280]); else Screen[1280] = Pixels[2]; ZB[1280]=index_spr;}
   	    if (ZB[1024]>index_spr) { if (SubScreen[1024]) Screen[1024] = COLOR_SUB1_2(Pixels[3], SubScreen[1024]); else Screen[1024] = Pixels[3]; ZB[1024]=index_spr;}
   	    if (ZB[ 768]>index_spr) { if (SubScreen[ 768]) Screen[ 768] = COLOR_SUB1_2(Pixels[4], SubScreen[ 768]); else Screen[ 768] = Pixels[4]; ZB[ 768]=index_spr;}
   	    if (ZB[ 512]>index_spr) { if (SubScreen[ 512]) Screen[ 512] = COLOR_SUB1_2(Pixels[5], SubScreen[ 512]); else Screen[ 512] = Pixels[5]; ZB[ 512]=index_spr;}
   	    if (ZB[ 256]>index_spr) { if (SubScreen[ 256]) Screen[ 256] = COLOR_SUB1_2(Pixels[6], SubScreen[ 256]); else Screen[ 256] = Pixels[6]; ZB[ 256]=index_spr;}
   	    if (ZB[   0]>index_spr) { if (SubScreen[   0]) Screen[   0] = COLOR_SUB1_2(Pixels[7], SubScreen[   0]); else Screen[   0] = Pixels[7]; ZB[   0]=index_spr;}
}



/*********************************************************************/

void softDrawTile16New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
	
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED16_O (Offset, (uint16*)bp);
	    }
	    else
	    {
		    //VFLIP
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
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
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}

void softDrawClippedTile16New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED16_O (Offset, (uint16*)bp);
	    }
	    else
	    {
		    //VFLIP
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
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
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}

void softDrawTile16NewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
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
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
		}
	}
}

void softDrawClippedTile16NewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
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
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
		}
	}
}

void softDrawHiResTile16New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    
	if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
	   	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
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
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}

void softDrawHiResClippedTile16New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    

    
	if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
	   	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
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
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}

/****************** ADD ***********************/
void softDrawTile16ADDNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);

    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_ADD_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_ADD_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);
	    }
	    else
	    {
		    //VFLIP
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);
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
			   	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}

void softDrawClippedTile16ADDNew (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_ADD_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_ADD_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);
	    }
	    else
	    {
		    //VFLIP
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);
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
			   	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}

void softDrawTile16ADDNewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= StartLine*256; //align to tile multiple    
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
		Offset -= StartLine*256; //align to tile multiple    
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
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_ADD_16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_ADD_16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
		}
	}
}

void softDrawClippedTile16ADDNewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= StartLine*256; //align to tile multiple    
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

		Offset -= StartLine*256; //align to tile multiple    
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
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_ADD_16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_ADD_16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
		}
	}
}

void softDrawHiResTile16ADDNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    
	if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
	   	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= StartLine*256; //align to tile multiple    
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
		Offset -= StartLine*256; //align to tile multiple    
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
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}

void softDrawHiResClippedTile16ADDNew (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    

    
	if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
	   	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= StartLine*256; //align to tile multiple    
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

		Offset -= StartLine*256; //align to tile multiple    
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
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}

/****************** ADD1_2 ***********************/
void softDrawTile16ADD1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    {
		    //VFLIP
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);
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
			   	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}

void softDrawClippedTile16ADD1_2New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    {
		    //VFLIP
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);
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
			   	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}

void softDrawTile16ADD1_2NewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= StartLine*256; //align to tile multiple    
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
		Offset -= StartLine*256; //align to tile multiple    
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
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_ADD1_2_16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_ADD1_2_16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
		}
	}
}

void softDrawClippedTile16ADD1_2NewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= StartLine*256; //align to tile multiple    
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

		Offset -= StartLine*256; //align to tile multiple    
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
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_ADD1_2_16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_ADD1_2_16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
		}
	}
}

void softDrawHiResTile16ADD1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    
	if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
	   	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= StartLine*256; //align to tile multiple    
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
		Offset -= StartLine*256; //align to tile multiple    
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
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}

void softDrawHiResClippedTile16ADD1_2New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    

    
	if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
	   	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= StartLine*256; //align to tile multiple    
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

		Offset -= StartLine*256; //align to tile multiple    
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
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);

		}
	}
}


/****************** SUB ***********************/
void softDrawTile16SUBNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_SUB_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_SUB_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);
	    }
	    else
	    {
		    //VFLIP
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);
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
			   	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}

void softDrawClippedTile16SUBNew (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_SUB_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_SUB_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);
	    }
	    else
	    {
		    //VFLIP
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);
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
			   	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}

void softDrawTile16SUBNewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= StartLine*256; //align to tile multiple    
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
		Offset -= StartLine*256; //align to tile multiple    
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
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_SUB_16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_SUB_16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
		}
	}
}

void softDrawClippedTile16SUBNewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= StartLine*256; //align to tile multiple    
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

		Offset -= StartLine*256; //align to tile multiple    
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
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_SUB_16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_SUB_16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
		}
	}
}

void softDrawHiResTile16SUBNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    
	if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
	   	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= StartLine*256; //align to tile multiple    
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
		Offset -= StartLine*256; //align to tile multiple    
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
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}

void softDrawHiResClippedTile16SUBNew (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    

    
	if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
	   	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= StartLine*256; //align to tile multiple    
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

		Offset -= StartLine*256; //align to tile multiple    
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
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);

		}
	}
}

/****************** SUB1_2 ***********************/
void softDrawTile16SUB1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    {
		    //VFLIP
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);
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
			   	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}

void softDrawClippedTile16SUB1_2New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= StartLine*256; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    {
		    //VFLIP
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);
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
			   	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}

void softDrawTile16SUB1_2NewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= StartLine*256; //align to tile multiple    
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
		Offset -= StartLine*256; //align to tile multiple    
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
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_SUB1_2_16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_SUB1_2_16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
		}
	}
}

void softDrawClippedTile16SUB1_2NewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
    	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= StartLine*256; //align to tile multiple    
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

		Offset -= StartLine*256; //align to tile multiple    
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
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED_SUB1_2_16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED_SUB1_2_16_SPR_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp,index_spr);

		}
	}
}

void softDrawHiResTile16SUB1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    
    
	if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
	   	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= StartLine*256; //align to tile multiple    
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
		Offset -= StartLine*256; //align to tile multiple    
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
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}

void softDrawHiResClippedTile16SUB1_2New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = GPUPack.BG.TileAddress + ((Tile & 0x3ff) << GPUPack.BG.TileShift);
    if (Tile & 0x100) TileAddr += GPUPack.BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (GPUPack.BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GPUPack.GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & GPUPack.BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GPUPack.GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette];
    	Col = (((Tile >> 10) & GPUPack.BG.PaletteMask) << GPUPack.BG.PaletteShift) + GPUPack.BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &GPUPack.BG.Buffer[(TileNumber = (TileAddr >> GPUPack.BG.TileShift)) *(128+8)];
    

    
	if ((!GPUPack.BG.Buffered [TileNumber<<1])|(GPUPack.BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	GPUPack.BG.Buffered[TileNumber<<1] = softConvertTile16New (pCache, TileAddr,GPUPack.GFX.ScreenColors);
	   	GPUPack.BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (GPUPack.BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x0000;break;
		case 1:solid_lineclip=0x0180;break;
		case 2:solid_lineclip=0x03C0;break;
		case 3:solid_lineclip=0x07E0;break;
		case 4:solid_lineclip=0x0FF0;break;
		case 5:solid_lineclip=0x1FF8;break;
		case 6:solid_lineclip=0x3FFC;break;
		case 7:solid_lineclip=0x7FFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0xFE7F;break;
		case 2:solid_lineclip|=0xFC3F;break;
		case 3:solid_lineclip|=0xF81F;break;
		case 4:solid_lineclip|=0xF00F;break;
		case 5:solid_lineclip|=0xE007;break;
		case 6:solid_lineclip|=0xC003;break;
		case 7:solid_lineclip|=0x8001;break;

	}
	
	if (solid_lineclip==0xFFFF) return;
	
	if ( (GPUPack.BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= StartLine*256; //align to tile multiple    
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

		Offset -= StartLine*256; //align to tile multiple    
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
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset--)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
	    }
	    else
	    {
		    //VFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset++)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,(solid_lineclip>>8)|*headerbp);
		}
	}
}



void softDrawTile16NewFastSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
	softDrawTile16New(Tile,Offset,StartLine,LineCount);
}

void softDrawClippedTile16NewFastSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
	softDrawClippedTile16New (Tile,Offset,StartPixel,Width,StartLine,LineCount);
}
