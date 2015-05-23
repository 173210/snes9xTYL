
// primitive graphics for Hello World sce

#include "psp.h"
#include "pg.h"

#include "font.c"
#include "fontNaga12.c"
#include "imageio.h"
#include <math.h>

#define ANTIALIAS_FACTOR 18
#define ANTIALIAS_CODE(x1,y1,sz) \
		tot=(buffer[(y1+1)*sz+(x1+1)]*10+\
		(buffer[(y1)*sz+(x1)]+buffer[(y1)*sz+(x1+1)]+buffer[(y1+1)*sz+(x1+2)]+\
		buffer[(y1+2)*sz+(x1)]+buffer[(y1+2)*sz+(x1+1)]+buffer[(y1+2)*sz+(x1+2)]+\
		buffer[(y1+1)*sz+(x1)]+buffer[(y1)*sz+(x1+1)]));

//static unsigned char nullbuffer[10240];

int pg_shadow=1;
int pg_init_with_bg=0;

//variables
//char *pg_vramtop=(char *)0x04000000;
#define pg_vramtop ((char *)0x04000000)
long pg_screenmode;
long pg_showframe;
long pg_drawframe=0;
unsigned long pgc_csr_x[2], pgc_csr_y[2];
unsigned long pgc_fgcolor[2], pgc_bgcolor[2];
char pgc_fgdraw[2], pgc_bgdraw[2];
char pgc_mag[2];



#define timercmp(a, b, CMP)	(((a)->tv_sec == (b)->tv_sec) ? ((a)->tv_usec CMP (b)->tv_usec) : ((a)->tv_sec CMP (b)->tv_sec))

//void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag)
//{
//	unsigned char *vptr0;		//pointer to vram
//	unsigned char *vptr;		//pointer to vram
//	unsigned char *cfont;		//pointer to font
//	unsigned long cx,cy;
//	unsigned long b;
//	char mx,my;
//
//	if (ch>255) return;
//	cfont=font+ch*8;
//	vptr0=pgGetVramAddr(x,y);
//	for (cy=0; cy<8; cy++) {
//		for (my=0; my<mag; my++) {
//			vptr=vptr0;
//			b=0x80;
//			for (cx=0; cx<8; cx++) {
//				for (mx=0; mx<mag; mx++) {
//					if ((*cfont&b)!=0) {
//						if (drawfg) *(unsigned short *)vptr=color;
//					} else {
//						if (drawbg) *(unsigned short *)vptr=bgcolor;
//					}
//					vptr+=PIXELSIZE*2;
//				}
//				b=b>>1;
//			}
//			vptr0+=LINESIZE*2;
//		}
//		cfont++;
//	}
//}


static void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag)
{
	char *vptr0;		//pointer to vram
	char *vptr;		//pointer to vram
	unsigned char *cfont;		//pointer to font
	unsigned long cx,cy;
	unsigned long b;
	char mx,my;
	unsigned char buffer[10*10];
	int rr,vv,bb,r,v,bl;
	int tot,fcol;


	if (ch>255) return;
		
	memset(buffer,0,10*10);
	r=color&31;
	v=(color>>5)&31;
	bl=(color>>10)&31;
		
	cfont=(unsigned char *)font+ch*8;
	for (cy=0; cy<8; cy++) {		
			b=0x80;
			for (cx=0; cx<8; cx++) {				
					if ((*cfont&b)) buffer[(cy+1)*10+cx]=1;
				b=b>>1;
			}		
		cfont++;
	}
	
	vptr0=pgGetVramAddr(x,y);
	for (cy=0; cy<8; cy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			b=0x80;
			for (cx=0; cx<8; cx++) {
				
				ANTIALIAS_CODE(cx,cy,10)
		
				rr=r*tot/ANTIALIAS_FACTOR;
				vv=v*tot/ANTIALIAS_FACTOR;
				bb=bl*tot/ANTIALIAS_FACTOR;
				
				fcol=(rr)|(vv<<5)|(bb<<10);
						
				for (mx=0; mx<mag; mx++) {
						if (fcol) {
							if (drawfg) *(unsigned short *)vptr=fcol;
						} else {
							if (drawbg) *(unsigned short *)vptr=bgcolor;
						}
						vptr+=PIXELSIZE*2;
					}
					b=b>>1;
			}
			vptr0+=LINESIZE*2;
		}
		cfont++;
	}		
}

static void pgPutChar_shadow(unsigned long x,unsigned long y,unsigned char ch,char drawfg,char drawbg,char mag)
{
	char *vptr0;		//pointer to vram
	char *vptr;		//pointer to vram
	unsigned char *cfont;		//pointer to font
	unsigned long cx,cy;
	unsigned long b;
	char mx,my;
	unsigned char buffer[10*10];
	int rr,vv,bb,r,v;
	int tot,col,fcol;


	if (ch>255) return;
		
	memset(buffer,0,10*10);
	
		
	cfont=(unsigned char *)font+ch*8;
	for (cy=0; cy<8; cy++) {		
			b=0x80;
			for (cx=0; cx<8; cx++) {				
					if ((*cfont&b)) buffer[(cy+1)*10+cx]=1;
				b=b>>1;
			}		
		cfont++;
	}
	
	vptr0=pgGetVramAddr(x,y);
	for (cy=0; cy<8; cy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			b=0x80;
			for (cx=0; cx<8; cx++) {
				
				ANTIALIAS_CODE(cx,cy,10)
				
				if (tot) {			
					for (mx=0; mx<mag; mx++) {
						
						col=*(unsigned short *)vptr;
						r=col&31; v=(col>>5)&31; b=(col>>10)&31;
						rr=r-tot;if (rr<0) rr=0;
						vv=v-tot;if (vv<0) vv=0;
						bb=b-tot;if (bb<0) bb=0;
							
						fcol=(rr)|(vv<<5)|(bb<<10);
						
						if (fcol) {
							if (drawfg) *(unsigned short *)vptr=fcol;
						} else {
							if (drawbg) *(unsigned short *)vptr=0;
						}
						
						vptr+=PIXELSIZE*2;
					}										
				}
				
					b=b>>1;
			}
			vptr0+=LINESIZE*2;
		}
		cfont++;
	}		
}



void pgWaitVn(unsigned long count)
{
	for (; count>0; --count) {
		sceDisplayWaitVblankStart();
	}
}


void pgWaitV() {
	sceDisplayWaitVblankStart();
}


char *pgGetVramAddr(unsigned long x,unsigned long y)
{
	return pg_vramtop+pg_drawframe*FRAMESIZE+x*PIXELSIZE*2+y*LINESIZE*2+0x40000000;
}

void pgCopyScreen(void){
	char *src,*dst;
	dst=pgGetVramAddr(0,0);
	pg_drawframe^=1;
	src=pgGetVramAddr(0,0);
	pg_drawframe^=1;
	memcpy(dst,src,FRAMESIZE);
}

void pgPrint(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {
		pgPutChar_shadow(x*8+1,y*8+1,*str,1,0,1);
		pgPutChar(x*8,y*8,color,0,*str,1,0,1);
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
}


void pgPrintCenter(unsigned long y,unsigned long color,const char *str){
	unsigned long x=(480-strlen(str)*8)>>4;
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {
		pgPutChar_shadow(x*8+1,y*8+1,*str,1,0,1);
		pgPutChar(x*8,y*8,color,0,*str,1,0,1);
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
}


void pgPrintSel(unsigned long x,unsigned long y,unsigned long color,char *str){
	char *str2;
	int xm,ym;
			
	xm=x;ym=y;
	str2=str;
	while (*str!=0 && xm<CMAX_X && ym<CMAX_Y) {					
		str++;
		xm++;
		if (xm>=CMAX_X) {
			xm=0;
			ym++;
		}
	}
	pgFillBoxHalfer(16,y*8,464,ym*8+8);
	pgPrint(x,y,color,str2);
}


void pgPrintBG(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {
		pgPutChar_shadow(x*8+1,y*8+1,*str,1,1,1);
		pgPutChar(x*8,y*8,color,0,*str,1,1,1);
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
}

void pgPrintBGRev(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	pg_drawframe^=1;
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {		
		pgPutChar_shadow(x*8+1,y*8+1,*str,1,1,1);
		pgPutChar(x*8,y*8,color,0,*str,1,1,1);
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
	pg_drawframe^=1;
}

void pgPrintAllBG(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {
		pgPutChar_shadow(x*8+1,y*8+1,*str,1,1,1);
		pgPutChar(x*8,y*8,color,0,*str,1,1,1);
		pg_drawframe^=1;
		pgPutChar_shadow(x*8+1,y*8+1,*str,1,1,1);
		pgPutChar(x*8,y*8,color,0,*str,1,1,1);
		pg_drawframe^=1;
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
}


void pgPrint4(unsigned long x,unsigned long y,unsigned long color,unsigned long color2,const char *str)
{		
	while (*str!=0 && x<CMAX4_X && y<CMAX4_Y) {
		
		pgPutChar_shadow(x*32+1,y*32+1,*str,1,0,4);
		pgPutChar_shadow(x*32+1+1,y*32+1+1,*str,1,0,4);
		pgPutChar_shadow(x*32+2+1,y*32+2+1,*str,1,0,4);
		pgPutChar_shadow(x*32+3+1,y*32+3+1,*str,1,0,4);
		pgPutChar_shadow(x*32+4+1,y*32+4+1,*str,1,0,4);
		
		pgPutChar(x*32,y*32,color,0,*str,1,0,4);
		
		pgPutChar(x*32+1,y*32+1,color,0,*str,1,0,4);
		
		pgPutChar(x*32+2,y*32+2,color,0,*str,1,0,4);
		
		pgPutChar(x*32+3,y*32+3,color,0,*str,1,0,4);
		
		pgPutChar(x*32+4,y*32+4,color2,0,*str,1,0,4);
		str++;
		x++;
		if (x>=CMAX4_X) {
			x=0;
			y++;
		}
	}
}


// by kwn
//void Draw_Char_Hankaku(int x,int y,unsigned char c,int col) {
//	unsigned short *vr;
//	unsigned char  *fnt;
//	unsigned char  pt;
//	unsigned char ch;
//	int x1,y1;	
//
//	ch = c;
//
//	// mapping
//	if (ch<0x20)
//		ch = 0;
//	else if (ch<0x80)
//		ch -= 0x20;
//	else if (ch<0xa0)
//		ch = 0;
//	else
//		ch -= 0x40;
//
//	fnt = (unsigned char *)&hankaku_font10[ch*10];
//
//	// draw
//	vr = (unsigned short *)pgGetVramAddr(x,y);
//	for(y1=0;y1<10;y1++) {
//		pt = *fnt++;
//		for(x1=0;x1<5;x1++) {
//			if (pt & 1)
//				*vr = col;
//			vr++;
//			pt = pt >> 1;
//		}
//		vr += LINESIZE-5;
//	}
//}

static void Draw_Char_Hankaku(int x, int y, unsigned char c, int col,
	unsigned short *vbuff, int pitch, int init_with_bg)
{
	unsigned short *vr;
	unsigned char  *fnt;
	unsigned char  pt;
	unsigned char ch;
	unsigned char buffer[14*8];
	int x1,y1;	
	int rr,vv,bb,r,v,b;
	int tot;

	ch = c;

	// mapping
	if (ch<0x20)
		ch = 0;
	else if (ch<0x80)
		ch -= 0x20;
	else if (ch<0xa0)
		ch = 0;
	else
		ch -= 0x40;

	fnt = (unsigned char *)&hankaku_font12[ch*12];
	
	if (vbuff) vr=&vbuff[y*pitch+x];
	else {vr = (unsigned short *)pgGetVramAddr(x,y);	pitch=LINESIZE;}

	memset(buffer,0,14*8);
	
	// draw
	//vr = (unsigned short *)pgGetVramAddr(x,y);
	for(y1=0;y1<12;y1++) {
		pt = *fnt++;
		for(x1=0;x1<6;x1++) {
			if (pt & 1) buffer[(y1+1)*8+x1+1] = 1;
			pt = pt >> 1;
		}		
	}
	
	r=col&31;
	v=(col>>5)&31;
	b=(col>>10)&31;
	
	if (pg_init_with_bg) {
		for (y1=0;y1<12;y1++)
		for (x1=0;x1<6;x1++) {		
			if (buffer[(y1+1)*8+(x1+1)])
				vr[y1*pitch + x1]=col;
		}
	} else {
		for (y1=0;y1<12;y1++)
		for (x1=0;x1<6;x1++) {		
			ANTIALIAS_CODE(x1,y1,8)
			
			rr=r*tot/ANTIALIAS_FACTOR;
			vv=v*tot/ANTIALIAS_FACTOR;
			bb=b*tot/ANTIALIAS_FACTOR;
			
			if (rr||vv||bb) {			
				vr[y1*pitch + x1]=(rr)|(vv<<5)|(bb<<10);
			}
		}
	}
}



static void Draw_Char_Hankaku_shadow(int x, int y, unsigned char c,
	unsigned short *vbuff, int pitch, int init_with_bg)
{
	unsigned short *vr;
	unsigned char  *fnt;
	unsigned char  pt;
	unsigned char ch;
	unsigned char buffer[14*8];
	int x1,y1;	
	int rr,vv,bb,r,v,b,col;
	int tot;

	ch = c;

	// mapping
	if (ch<0x20)
		ch = 0;
	else if (ch<0x80)
		ch -= 0x20;
	else if (ch<0xa0)
		ch = 0;
	else
		ch -= 0x40;

	fnt = (unsigned char *)&hankaku_font12[ch*12];

	if (vbuff) vr=&vbuff[y*pitch+x];
	else {vr = (unsigned short *)pgGetVramAddr(x,y);	pitch=LINESIZE;}
	memset(buffer,0,14*8);
	
	// draw
	//vr = (unsigned short *)pgGetVramAddr(x,y);
	for(y1=0;y1<12;y1++) {
		pt = *fnt++;
		for(x1=0;x1<6;x1++) {
			if (pt & 1) buffer[(y1+1)*8+x1+1] = 1;
			pt = pt >> 1;
		}		
	}
	
	
	if (pg_init_with_bg) {
		for (y1=0;y1<12;y1++)
		for (x1=0;x1<6;x1++) {		
			if (buffer[(y1+1)*8+(x1+1)]) {
				col=vr[y1*pitch + x1];
				vr[y1*pitch + x1]=(((col>>11)&0xf)<<10)|(((col>>6)&0xf)<<5)|(((col>>1)&0xf)<<0);
			}
		}
	} else {
		for (y1=0;y1<12;y1++)
		for (x1=0;x1<6;x1++) {
			
			ANTIALIAS_CODE(x1,y1,8)
			
			if (tot) {			
				col=vr[y1*pitch + x1];
				r=col&31; v=(col>>5)&31; b=(col>>10)&31;
				rr=r-tot;if (rr<0) rr=0;
				vv=v-tot;if (vv<0) vv=0;
				bb=b-tot;if (bb<0) bb=0;
				
				vr[y1*pitch + x1]=(rr)|(vv<<5)|(bb<<10);
			}
		}
	}
}

// by kwn
//void Draw_Char_Zenkaku(int x,int y,unsigned char u,unsigned char d,int col) {
//	// ELISA100.FNTに存在しない文字
//	unsigned short font404[] = {
//		0xA2AF, 11,
//		0xA2C2, 8,
//		0xA2D1, 11,
//		0xA2EB, 7,
//		0xA2FA, 4,
//		0xA3A1, 15,
//		0xA3BA, 7,
//		0xA3DB, 6,
//		0xA3FB, 4,
//		0xA4F4, 11,
//		0xA5F7, 8,
//		0xA6B9, 8,
//		0xA6D9, 38,
//		0xA7C2, 15,
//		0xA7F2, 13,
//		0xA8C1, 720,
//		0xCFD4, 43,
//		0xF4A5, 1030,
//		0,0
//	};
//	unsigned short *vr;
//	unsigned short *fnt;
//	unsigned short pt;
//	int x1,y1;
//
//	unsigned long n;
//	unsigned short code;
//	int i, j;
//
//	// SJISコードの生成
//	code = u;
//	code = (code<<8) + d;
//
//	// SJISからEUCに変換
//	if(code >= 0xE000) code-=0x4000;
//	code = ((((code>>8)&0xFF)-0x81)<<9) + (code&0x00FF);
//	if((code & 0x00FF) >= 0x80) code--;
//	if((code & 0x00FF) >= 0x9E) code+=0x62;
//	else code-=0x40;
//	code += 0x2121 + 0x8080;
//
//	// EUCから恵梨沙フォントの番号を生成
//	n = (((code>>8)&0xFF)-0xA1)*(0xFF-0xA1)
//		+ (code&0xFF)-0xA1;
//	j=0;
//	while(font404[j]) {
//		if(code >= font404[j]) {
//			if(code <= font404[j]+font404[j+1]-1) {
//				n = -1;
//				break;
//			} else {
//				n-=font404[j+1];
//			}
//		}
//		j+=2;
//	}
//	fnt = (unsigned short *)&zenkaku_font10[n*10];
//
//	// draw
//	vr = (unsigned short *)pgGetVramAddr(x,y);
//	for(y1=0;y1<10;y1++) {
//		pt = *fnt++;
//		for(x1=0;x1<10;x1++) {
//			if (pt & 1)
//				*vr = col;
//			vr++;
//			pt = pt >> 1;
//		}
//		vr += LINESIZE-10;
//	}
//}


static void Draw_Char_Zenkaku(int x, int y, unsigned char u, unsigned char d,
	int col, unsigned short *vbuff, int pitch, int init_with_bg)
{
	// ELISA100.FNTに存在しない文字
	unsigned short font404[] = {
		0xA2AF, 11,
		0xA2C2, 8,
		0xA2D1, 11,
		0xA2EB, 7,
		0xA2FA, 4,
		0xA3A1, 15,
		0xA3BA, 7,
		0xA3DB, 6,
		0xA3FB, 4,
		0xA4F4, 11,
		0xA5F7, 8,
		0xA6B9, 8,
		0xA6D9, 38,
		0xA7C2, 15,
		0xA7F2, 13,
		0xA8C1, 720,
		0xCFD4, 43,
		0xF4A5, 1030,
		0,0
	};
	unsigned short *vr;
	unsigned short *fnt;
	unsigned short pt;
	int x1,y1;

	unsigned long n;
	unsigned short code;
	int j;

	unsigned char buffer[14*14];
	int rr,vv,bb,r,v,b;
	int tot;

	// SJISコードの生成
	code = u;
	code = (code<<8) + d;

	// SJISからEUCに変換
	if(code >= 0xE000) code-=0x4000;
	code = ((((code>>8)&0xFF)-0x81)<<9) + (code&0x00FF);
	if((code & 0x00FF) >= 0x80) code--;
	if((code & 0x00FF) >= 0x9E) code+=0x62;
	else code-=0x40;
	code += 0x2121 + 0x8080;

	// EUCから恵梨沙フォントの番号を生成
	n = (((code>>8)&0xFF)-0xA1)*(0xFF-0xA1)
		+ (code&0xFF)-0xA1;
	j=0;
	while(font404[j]) {
		if(code >= font404[j]) {
			if(code <= font404[j]+font404[j+1]-1) {
				n = -1;
				break;
			} else {
				n-=font404[j+1];
			}
		}
		j+=2;
	}
	fnt = (unsigned short *)&zenkaku_font12[n*12];

	// draw
	if (vbuff) vr=&vbuff[y*pitch+x];
	else {vr = (unsigned short *)pgGetVramAddr(x,y);	pitch=LINESIZE;}
	memset(buffer,0,14*14);		
	
	for(y1=0;y1<12;y1++) {
		pt = *fnt++;
		for(x1=0;x1<12;x1++) {
			if (pt & 1)	buffer[(y1+1)*14+x1+1] = 1;		
			pt = pt >> 1;
		}		
	}
	
	r=col&31;
	v=(col>>5)&31;
	b=(col>>10)&31;
		
	if (pg_init_with_bg) {
		for (y1=0;y1<12;y1++)
		for (x1=0;x1<12;x1++) {		
			if (buffer[(y1+1)*14+(x1+1)])
				vr[y1*pitch + x1]=col;
		}
	} else {
		for (y1=0;y1<12;y1++)
		for (x1=0;x1<12;x1++) {		
			ANTIALIAS_CODE(x1,y1,14)
			
			rr=r*tot/ANTIALIAS_FACTOR;
			vv=v*tot/ANTIALIAS_FACTOR;
			bb=b*tot/ANTIALIAS_FACTOR;
			
			if (rr||vv||bb) {			
				vr[y1*pitch + x1]=(rr)|(vv<<5)|(bb<<10);
			}
		}
	}
}

static void Draw_Char_Zenkaku_shadow(int x, int y,
	unsigned char u, unsigned char d,
	unsigned short *vbuff, int pitch, int init_with_bg)
{
	// ELISA100.FNTに存在しない文字
	unsigned short font404[] = {
		0xA2AF, 11,
		0xA2C2, 8,
		0xA2D1, 11,
		0xA2EB, 7,
		0xA2FA, 4,
		0xA3A1, 15,
		0xA3BA, 7,
		0xA3DB, 6,
		0xA3FB, 4,
		0xA4F4, 11,
		0xA5F7, 8,
		0xA6B9, 8,
		0xA6D9, 38,
		0xA7C2, 15,
		0xA7F2, 13,
		0xA8C1, 720,
		0xCFD4, 43,
		0xF4A5, 1030,
		0,0
	};
	unsigned short *vr;
	unsigned short *fnt;
	unsigned short pt;
	int x1,y1;

	unsigned long n;
	unsigned short code;
	int j;
	
	unsigned char buffer[14*14];	
	int rr,vv,bb,r,v,b,col;
	int tot;

	// SJISコードの生成
	code = u;
	code = (code<<8) + d;

	// SJISからEUCに変換
	if(code >= 0xE000) code-=0x4000;
	code = ((((code>>8)&0xFF)-0x81)<<9) + (code&0x00FF);
	if((code & 0x00FF) >= 0x80) code--;
	if((code & 0x00FF) >= 0x9E) code+=0x62;
	else code-=0x40;
	code += 0x2121 + 0x8080;

	// EUCから恵梨沙フォントの番号を生成
	n = (((code>>8)&0xFF)-0xA1)*(0xFF-0xA1)
		+ (code&0xFF)-0xA1;
	j=0;
	while(font404[j]) {
		if(code >= font404[j]) {
			if(code <= font404[j]+font404[j+1]-1) {
				n = -1;
				break;
			} else {
				n-=font404[j+1];
			}
		}
		j+=2;
	}
	fnt = (unsigned short *)&zenkaku_font12[n*12];

	// draw
	if (vbuff) vr=&vbuff[y*pitch+x];
	else {vr = (unsigned short *)pgGetVramAddr(x,y);	pitch=LINESIZE;}
	memset(buffer,0,14*14);
	
	for(y1=0;y1<12;y1++) {
		pt = *fnt++;
		for(x1=0;x1<12;x1++) {
			if (pt & 1)	buffer[(y1+1)*14+x1+1] = 1;		
			pt = pt >> 1;
		}		
	}
	
	if (pg_init_with_bg) {
		for (y1=0;y1<12;y1++)
		for (x1=0;x1<12;x1++) {		
			if (buffer[(y1+1)*14+(x1+1)])
				vr[y1*pitch + x1]=0;
		}
	} else {
		for (y1=0;y1<12;y1++)
		for (x1=0;x1<12;x1++) {		
			ANTIALIAS_CODE(x1,y1,14)
			
			if (tot) {			
				col=vr[y1*LINESIZE + x1];
				r=col&31; v=(col>>5)&31; b=(col>>10)&31;
				rr=r-tot;if (rr<0) rr=0;
				vv=v-tot;if (vv<0) vv=0;
				bb=b-tot;if (bb<0) bb=0;
				
				vr[y1*pitch + x1]=(rr)|(vv<<5)|(bb<<10);
			}
		}
	}
}

// by kwn
static int mh_print_buff(int x,int y,int Mx,int My,const char *str,int col,unsigned short *vbuff,int pitch) {
	unsigned char ch = 0,bef = 0;
	
	while(*str != 0) {
		ch = *str++;		 
		if (bef!=0) {
			if ((x+12<=Mx)&&(y+12<=My)&&(x>=0)&&(y>=0)) {
				if (pg_shadow) Draw_Char_Zenkaku_shadow(x+1,y+1,bef,ch,vbuff,pitch,pg_init_with_bg);
				Draw_Char_Zenkaku(x,y,bef,ch,col,vbuff,pitch,pg_init_with_bg);
			}
			x+=12;
			bef=0;
		} else {
			if (((ch>=0x80) && (ch<0xa0)) || (ch>=0xe0)) {
				bef = ch;
			} else {
				if ((x+6<=Mx)&&(y+12<=My)&&(x>=0)&&(y>=0)) {
					if (pg_shadow) Draw_Char_Hankaku_shadow(x+1,y+1,ch,vbuff,pitch,pg_init_with_bg);
					Draw_Char_Hankaku(x,y,ch,col,vbuff,pitch,pg_init_with_bg);
				}
				x+=6;
			}
		}
	}
	return x;
}

void mh_print_light(int x,int y,const char *str,int col,int smoothing) {
	unsigned short *scr=(unsigned short *)pgGetVramAddr(x-6,y-6);
	unsigned short buffer[480*24],buffer2[480*24];
	int len;
	int j,px,py,r,g,b,col1,col2,col3,col4,col0;
	
	memset(buffer,0,480*24*2);
	memset(buffer2,0,480*24*2);
	len=mh_print_buff(6,6,480,272,str,col,buffer,480);
	len=len-6+12;
	
	
	for (j=smoothing;j;j--) {
		for (py=1;py<23;py++) {
			for (px=1;px<len-1;px++) {
				col0=buffer[py*480+px];
				col1=buffer[py*480+px+1];
				col2=buffer[py*480+px-1];
				col3=buffer[(py+1)*480+px];
				col4=buffer[(py-1)*480+px];
				r=(((col0&31)+(col1&31)+(col2&31)+(col3&31)+(col4&31))/4);
				g=(((col0&(31<<5))+(col1&(31<<5))+(col2&(31<<5))+(col3&(31<<5))+(col4&(31<<5)))/4);
				b=(((col0&(31<<10))+(col1&(31<<10))+(col2&(31<<10))+(col3&(31<<10))+(col4&(31<<10)))/4);
				if (r>31) r=31;
				if (g>(31<<5)) g=31<<5;
				if (b>(31<<10)) b=31<<10;
				r&=31;g&=31<<5;b&=31<<10;
				buffer2[py*480+px]=r|g|b;
			}
		}
		for (py=0;py<24;py++) memcpy(&buffer[py*480],&buffer2[py*480],len*2);
	}
	
	/*pg_shadow=0;
	mh_print(x,y,str,col);
	pg_shadow=1;*/
															
	for (py=0;py<24;py++)
		for (px=0;px<len-1;px++) if (buffer[py*480+px]) {
			col0=scr[LINESIZE*py+px];
			if (!col0) scr[LINESIZE*py+px]=buffer[py*480+px];
			else {
				col1=buffer[py*480+px];
				r=(col0&31)+(col1&31);
				g=(col0&(31<<5))+(col1&(31<<5));
				b=(col0&(31<<10))+(col1&(31<<10));
				//r/=2;g/=2;b/=2;
				if (r>31) r=31;
				if (g>(31<<5)) g=31<<5;
				if (b>(31<<10)) b=31<<10;
				r&=31;g&=31<<5;b&=31<<10;
				scr[LINESIZE*py+px]=r|g|b;						
			}
		}
	
	//pg_shadow=0;
	pg_init_with_bg=1;
	mh_print(x,y,str,col);
	pg_init_with_bg=0;
	//pg_shadow=1;
	
}

void mh_print(int x,int y,const char *str,int col) {
	mh_print_buff(x,y,480,272,str,col,NULL,0);
}

void mh_printLimit(int x,int y,int Mx,int My,const char *str,int col) {
	mh_print_buff(x,y,Mx,My,str,col,NULL,0);
}


void mh_printSel(int x,int y,const char *str,int col) {	
	pgFillBoxHalfer(4,y,456,y+8);
	mh_print(x,y,str,col);
}

void mh_printSel_light(int x,int y,const char *str,int col,int smoothing) {
	pgFillBoxHalfer(4,y,456,y+8);
	mh_print_light(x,y,str,col,smoothing);
}


//yoyovoid mh_print(int x,int y,const char *str,int col) {
int mh_length(const char *str) {
	unsigned char ch = 0,bef = 0;
	int len=0;
	while(*str != 0) {
		ch = *str++;
		if (bef!=0) {		
			len+=12;
			bef=0;
		} else {
			if (((ch>=0x80) && (ch<0xa0)) || (ch>=0xe0)) {
				bef = ch;
			} else {			
				len+=6;
			}
		}
	}
	return len;
}

int mh_trimlength(const char *str) {
	char ch = 0,bef = 0;
	int len=0;
	int old_pos=0,pos=0;
	
	while(*str != 0) {
		ch = *str++;
		pos++;
		if (bef!=0) {		
			len+=12;
			if (len>480) return old_pos;
			old_pos=pos;
			bef=0;
		} else {
			if (((ch>=0x80) && (ch<0xa0)) || (ch>=0xe0)) {
				bef = ch;
			} else {			
				len+=6;
				if (len>480) return old_pos;
				old_pos=pos;
			}
		}		
	}
	return len;
}

void mh_printCenter(unsigned long y,const char *str,unsigned long color){
	unsigned long x=(480-mh_length(str))>>1;
	mh_print(x,y,str,color);	
}



void pgDrawFrame(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=pgGetVramAddr(0,0);
	for(i=x1; i<=x2; i++){
		((unsigned short *)vptr0)[i*PIXELSIZE + y1*LINESIZE] = color;
		((unsigned short *)vptr0)[i*PIXELSIZE + y2*LINESIZE] = color;
	}
	for(i=y1; i<=y2; i++){
		((unsigned short *)vptr0)[x1*PIXELSIZE + i*LINESIZE] = color;
		((unsigned short *)vptr0)[x2*PIXELSIZE + i*LINESIZE] = color;
	}
}

void pgFillBox(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	char *vptr0;		//pointer to vram
	unsigned long i, j;

	vptr0=pgGetVramAddr(0,0);
	for(i=y1; i<=y2; i++){
		for(j=x1; j<=x2; j++){
			((unsigned short *)vptr0)[j*PIXELSIZE + i*LINESIZE] = color;
		}
	}
}

void pgFillBoxHalfer(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2 )
{
	char *vptr0;		//pointer to vram
	unsigned long i, j;
	unsigned long color;

	vptr0=pgGetVramAddr(0,0);
	for(i=y1; i<=y2; i++){
		for(j=x1; j<=x2; j++){
			color = ((unsigned short *)vptr0)[j*PIXELSIZE + i*LINESIZE];
			((unsigned short *)vptr0)[j*PIXELSIZE + i*LINESIZE] = ((((color>>10)&31)>>1)<<10)|
			((((color>>5)&31)>>1)<<5)|((color&31)>>1);
		}
	}
}


void pgFillvram(unsigned long color)
{

	char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=pgGetVramAddr(0,0);
	for (i=0; i<FRAMESIZE/2; i++) {
		*(unsigned short *)vptr0=color;
		vptr0+=PIXELSIZE*2;
	}

}

void pgFillAllvram(unsigned long color)
{
	u32 *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=(u32*)((u8*)pg_vramtop+0x40000000);
	for (i=0; i<2*FRAMESIZE/2; i++) {
		*vptr0++=color;		
	}
}

void pgScreenFrame(long mode,long frame)
{	
	pg_screenmode=mode;
	frame=(frame?1:0);
	pg_showframe=frame;
	if (mode==0) {
		//screen off
		pg_drawframe=frame;
		sceDisplaySetFrameBuf(0,0,0,1);
	} else if (mode==1) {
		//show/draw same
		pg_drawframe=frame;
		sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	} else if (mode==2) {
		//show/draw different
		pg_drawframe=(frame?0:1);
		sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	}
}


void pgScreenFlip()
{
	pg_showframe=pg_drawframe;
	pg_drawframe++;
	pg_drawframe&=1;	
	sceDisplaySetFrameBuf(pg_vramtop+pg_showframe*FRAMESIZE,LINESIZE,PIXELSIZE,0);
}


void pgScreenFlipV()
{
	pgWaitV();
	pgScreenFlip();
}

void pgScreenFlipV2()
{
	//pgWaitV();
	sceKernelDelayThread(1000); //go to sleep to let other thread execute
	pgScreenFlip();
}

int get_pad(void)
{
	SceCtrlData paddata;

	memset(&paddata,0,sizeof(paddata));
	
	//sceCtrlReadBufferPositive(&paddata, 1);
  sceCtrlPeekBufferPositive(&paddata, 1);
  // kmg
  // Analog pad state
  if (paddata.Ly >= 0xD0) paddata.Buttons|=PSP_CTRL_DOWN;  // DOWN
  if (paddata.Ly <= 0x30) paddata.Buttons|=PSP_CTRL_UP;    // UP
  if (paddata.Lx <= 0x30) paddata.Buttons|=PSP_CTRL_LEFT;  // LEFT
  if (paddata.Lx >= 0xD0) paddata.Buttons|=PSP_CTRL_RIGHT; // RIGHT
  	
  	paddata.Buttons&=ALL_PAD_BUTTONS;
    
	return paddata.Buttons;	
}

int get_pad2(int *lx,int *ly)
{
	SceCtrlData paddata;

	memset(&paddata,0,sizeof(paddata));
	
	//sceCtrlReadBufferPositive(&paddata, 1);
  sceCtrlPeekBufferPositive(&paddata, 1);
  // kmg
  // Analog pad state
  if (lx) *lx=paddata.Lx;
  if (ly) *ly=paddata.Ly;
    	
  paddata.Buttons&=ALL_PAD_BUTTONS;
    
	return paddata.Buttons;	
}



void pgwaitPress(void){
		
		while (get_pad()&ALL_PAD_BUTTONS) ;
		while (!get_pad()) ;
		while (get_pad()&ALL_PAD_BUTTONS) ;
}


/******************************************************************************/

void pgPrintDec(int x,int y,short col,unsigned int dec)
{
    char string[12];
    int i,a;

    for(i=0;i<11;i++) {
        a = dec % 10;
    	dec/=10;
        string[10-i]=0x30+a;
//        if(dec=0 && a==0) break;
    }
    string[i]=0;
    a=0;
    while ((string[a]=='0')&&(a<i-1)) a++;
    pgPrint(x,y,col,string+a);
}


void image_put_mul(int x0,int y0,IMAGE* img,int mul,int add)
{
	unsigned short *dst = (unsigned short *)pgGetVramAddr(x0,y0);
	unsigned char* src = img->pixels;
	unsigned short* src16 = img->pixels;
	unsigned short pal[256];
	s32 r,g,b,aR,aG,aB,mulR,mulG,mulB;
	int i;	
	mulB=(mul>>0)&0xFF;
	mulG=(mul>>8)&0xFF;
	mulR=(mul>>16)&0xFF;
	aB=(add>>0)&0xFF;
	aG=(add>>8)&0xFF;
	aR=(add>>16)&0xFF;
	
	if (img->bit==8) {	
		for(i=0;i<img->n_palette;i++){
	  	r=(s32)(img->palette[i].r)+aR;
	  	g=(s32)(img->palette[i].g)+aG;
	  	b=(s32)(img->palette[i].b)+aB;
	  	r=r*mulR/255;
	  	g=g*mulG/255;
	  	b=b*mulB/255;
	  	if (r<0) r=0;if (r>255) r=255;
	  	if (g<0) g=0;if (g>255) g=255;
	  	if (b<0) b=0;if (b>255) b=255;
			pal[i] = RGB(r,g,b);
		}	
		int x,y;
		for(y=0;y<img->height;y++) {
			for(x=0;x<img->width;x++) {
				dst[x] = pal[*src++];
			}
			dst += 512;
		}
	}
	if (img->bit==24){		
		int x,y;
		for(y=0;y<img->height;y++) {
			for(x=0;x<img->width;x++) {												
				r=*src++;g=*src++;b=*src++;
				r+=aR;g+=aG;b+=aB;
				r=r*mulR/255;
	  		g=g*mulG/255;
	  		b=b*mulB/255;
				if (r<0) r=0;if (r>255) r=255;
				if (g<0) g=0;if (g>255) g=255;
				if (b<0) b=0;if (b>255) b=255;				
				dst[x] = ((b>>3)<<10)|((g>>3)<<5)|(r>>3);
			}
			dst += 512;
		}
	}
	if (img->bit==15){
		int x,y;
		for(y=0;y<img->height;y++) {
			for(x=0;x<img->width;x++) {												
				r=*src16++;
				b=(r>>10)<<3;g=((r>>5)&31)<<3;r=(r&31)<<3;
				r+=aR;g+=aG;b+=aB;
				r=r*mulR/255;
	  		g=g*mulG/255;
	  		b=b*mulB/255;
				if (r<0) r=0;if (r>255) r=255;
				if (g<0) g=0;if (g>255) g=255;
				if (b<0) b=0;if (b>255) b=255;
				dst[x] = ((b>>3)<<10)|((g>>3)<<5)|(r>>3);
			}
			dst += 512;
		}
	}
}

void image_put_light(int x0,int y0,IMAGE* img,int fade,int add,int transp_col,int smoothing)
{
	unsigned short buffer[64*64],buffer2[64*64],buffer3[64*64];
	unsigned short *dst = (unsigned short *)pgGetVramAddr(x0,y0);
	unsigned char* src = img->pixels;
	s32 r,g,b,fadeR,fadeG,fadeB,aR,aG,aB;
	int col1,col2,col3,col4,col0;
	int j,px,py;
	fadeB=(fade>>0)&0xFF;
	fadeG=(fade>>8)&0xFF;
	fadeR=(fade>>16)&0xFF;
	aB=(add>>0)&0xFF;
	aG=(add>>8)&0xFF;
	aR=(add>>16)&0xFF;
	
	memset(buffer,0,64*64*2);
	memset(buffer2,0,64*64*2);
		
	if (img->bit==24){		
		int x,y;
		for(y=0;y<48;y++) {						
			dst = &buffer[(y+8)*64+8];
			for(x=0;x<48;x++) {												
				r=*src++;g=*src++;b=*src++;
				if (((r<<16)|(g<<8)|b)!=transp_col) {
					r+=aR-fadeR;g+=aG-fadeG;b+=aB-fadeB;
					if (r<0) r=0;if (r>255) r=255;
					if (g<0) g=0;if (g>255) g=255;
					if (b<0) b=0;if (b>255) b=255;
					dst[x] = ((b>>3)<<10)|((g>>3)<<5)|(r>>3);
				}
			}			
		}
		memcpy(buffer3,buffer,64*64*2);
		
		dst = (unsigned short *)pgGetVramAddr(x0,y0);
		for (py=0+8;py<48+8;py++) 
		for (px=0+8;px<48+8;px++){
			if (buffer[py*64+px]) {
				dst[py*LINESIZE+px]=buffer[py*64+px];
				//buffer[py*64+px]=0xFFFF;
			}
		}
		
		
		
		for (j=smoothing;j;j--) {
			for (py=1;py<63;py++) {
				for (px=1;px<63;px++) {
					col0=buffer[py*64+px];
					col1=buffer[py*64+px+1];
					col2=buffer[py*64+px-1];
					col3=buffer[(py+1)*64+px];
					col4=buffer[(py-1)*64+px];
					r=(((col0&31)+(col1&31)+(col2&31)+(col3&31)+(col4&31))/4);
					g=(((col0&(31<<5))+(col1&(31<<5))+(col2&(31<<5))+(col3&(31<<5))+(col4&(31<<5)))/4);
					b=(((col0&(31<<10))+(col1&(31<<10))+(col2&(31<<10))+(col3&(31<<10))+(col4&(31<<10)))/4);
					if (r>31) r=31;
					if (g>(31<<5)) g=31<<5;
					if (b>(31<<10)) b=31<<10;
					r&=31;g&=31<<5;b&=31<<10;
					buffer2[py*64+px]=r|g|b;
				}
	 		}
			memcpy(buffer,buffer2,64*64*2);
	 	}
	 	
	 	
		
		
		dst = (unsigned short *)pgGetVramAddr(x0,y0);
		for (py=0;py<64;py++)
		for (px=0;px<64;px++) if (buffer[py*64+px]) {
			col0=dst[LINESIZE*py+px];
			if (!col0) dst[LINESIZE*py+px]=buffer[py*64+px];
			else {
				col1=buffer[py*64+px];
				r=(col0&31)*2+(col1&31);
				g=(col0&(31<<5))*2+(col1&(31<<5));
				b=(col0&(31<<10))*2+(col1&(31<<10));
				r/=2;g/=2;b/=2;
				if (r>31) r=31;
				if (g>(31<<5)) g=31<<5;
				if (b>(31<<10)) b=31<<10;
				r&=31;g&=31<<5;b&=31<<10;
				dst[LINESIZE*py+px]=r|g|b;						
			}
		}
	
		/*dst = pgGetVramAddr(x0,y0);
		for (py=0+8;py<48+8;py++) 
		for (px=0+8;px<48+8;px++){
			if (buffer3[py*64+px]) dst[py*LINESIZE+px]=buffer3[py*64+px];
		}*/
	
	}		
}
void image_put(int x0,int y0,IMAGE* img,int fade,int add,int transp_col,int sz)
{	
	unsigned short *dst = (unsigned short *)pgGetVramAddr(x0,y0);
	unsigned char* src = img->pixels;
	unsigned short pal[256];
	s32 r,g,b,fadeR,fadeG,fadeB,aR,aG,aB;
	int i;
	fadeB=(fade>>0)&0xFF;
	fadeG=(fade>>8)&0xFF;
	fadeR=(fade>>16)&0xFF;
	aB=(add>>0)&0xFF;
	aG=(add>>8)&0xFF;
	aR=(add>>16)&0xFF;
	
	if (img->bit==8) {	
		for(i=0;i<img->n_palette;i++){
	  	r=(s32)(img->palette[i].r)-fadeR+aR;if (r<0) r=0;if (r>255) r=255;
	  	g=(s32)(img->palette[i].g)-fadeG+aG;if (g<0) g=0;if (g>255) g=255;
	  	b=(s32)(img->palette[i].b)-fadeB+aB;if (b<0) b=0;if (b>255) b=255;
			pal[i] = RGB(r,g,b);
		}	
		int x,y;
		for(y=0;y<img->height;y++) {
			for(x=0;x<img->width;x++) {
				if (*src!=transp_col)	dst[x] = pal[*src];
				src++;
			}
			dst += 512;
		}
	}
	if (img->bit==24){		
		u32 x,y;
		for(y=0;y<img->height;y++) {
			dst = (unsigned short *)pgGetVramAddr(x0,y0+y*sz/256);
			for(x=0;x<img->width;x++) {												
				r=*src++;g=*src++;b=*src++;
				if (((r<<16)|(g<<8)|b)!=transp_col) {
					r+=aR-fadeR;g+=aG-fadeG;b+=aB-fadeB;
					if (r<0) r=0;if (r>255) r=255;
					if (g<0) g=0;if (g>255) g=255;
					if (b<0) b=0;if (b>255) b=255;
					dst[x*sz/256] = ((b>>3)<<10)|((g>>3)<<5)|(r>>3);
				}
			}
			//dst += 512;
		}
	}
}

void ErrorMsg(const char *msg){
	pgFillAllvram(0);
	pgScreenFrame(1,0);
	pgPrintCenter(11,(15<<10)|(31<<5)|31,msg);
	pgwaitPress();
}
