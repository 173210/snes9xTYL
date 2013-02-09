#include "psp.h"

#include <pspaudio.h>

#define BOX_COLOR (18<<10)|(4<<5)|4
#define FRAME_COLOR (12<<10)|(2<<5)|2

#define ZIPBOX_COLOR (26<<0)|(23<<5)|(31<<10)
#define ZIPFRAME_COLOR (16<<0)|(13<<5)|(23<<10)

#define TEXT_COLOR  (12<<10)|(31<<5)|31
#define TEXT_COLOR_OK  (4<<10)|(29<<5)|4
#define TEXT_COLOR_CANCEL  (10<<10)|(6<<5)|29
////////////////////////////////////////////////////////////////////////////////////////
// Message box
////////////////////////////////////////////////////////////////////////////////////////
void msgBox(const char *msg,int delay_vblank) {
	int len;
	char str[60];
	
	pgCopyScreen();
	
	memcpy(str,msg,50);
	str[50]=0;
	len=strlen(msg);
	if (len>=50) {
		str[50-3]=str[50-2]=str[50-1]='.';
		len=49;
	}
	len*=8;
	
	pgFillBox(240-len/2-20-1,136-10-1,240+len/2+20+1,136+18+1,BOX_COLOR);
	pgDrawFrame(240-len/2-20,136-10,240+len/2+20,136+18,FRAME_COLOR);
	
	pgPrintCenter(17,TEXT_COLOR,str);	
	
	pgScreenFlipV();
	if (delay_vblank<=0) delay_vblank=1;
	pgWaitVn(delay_vblank);
}

////////////////////////////////////////////////////////////////////////////////////////
// Input box : in fact confirm box...
////////////////////////////////////////////////////////////////////////////////////////
int inputBox(char *msg) {		
	int pad,y;
	char *str;
	int l=strlen(msg);
	str=(char*)malloc(l+32);
	strcpy(str,msg);
	strcat(str,"\n\n" SJIS_CIRCLE " OK, " SJIS_CROSS " CANCEL");
	y=msgBoxLinesRaw(str,-1);
	sprintf(str,SJIS_CIRCLE " OK           ");
	mh_printCenter(y,str,TEXT_COLOR_OK);
	sprintf(str,"       " SJIS_CROSS " CANCEL");
	mh_printCenter(y,str,TEXT_COLOR_CANCEL);
	pgScreenFlipV();
	while (get_pad()) pgWaitV();
	while (1){
		pad=get_pad();
		if (pad) while (get_pad()) pgWaitV();
		if (pad&PSP_CTRL_CIRCLE) return 1;			
		if (pad&PSP_CTRL_CROSS) return 0;
	}
	free(str);	
}

////////////////////////////////////////////////////////////////////////////////////////
// input box OK : in fact confirm OK box...
////////////////////////////////////////////////////////////////////////////////////////
int inputBoxOK(char *msg) {		
	int pad;
	char *str;
	int l=strlen(msg);
	int y;
	str=(char*)malloc(l+32);
	strcpy(str,msg);
	strcat(str,"\n\n" SJIS_CIRCLE "," SJIS_CROSS " Close");
	y=msgBoxLinesRaw(str,-1);
	sprintf(str,SJIS_CIRCLE "," SJIS_CROSS "      ");
	mh_printCenter(y,str,TEXT_COLOR_OK);
	pgScreenFlipV();
	while (get_pad()) pgWaitV();
	while (1){
		pad=get_pad();
		if (pad) while (get_pad()) pgWaitV();
		if (pad&PSP_CTRL_CIRCLE) return 1;			
		if (pad&PSP_CTRL_CROSS) return 0;
	}
	free(str);
}

////////////////////////////////////////////////////////////////////////////////////////
// Multiple lines message box
////////////////////////////////////////////////////////////////////////////////////////
int msgBoxLines(const char *msg,int delay_vblank) {
	int len,width;
	unsigned char c;
	char str[256];
	int i,j,x,y,lines,ret;
	
	if (!msg) return;
  if (!(msg[0])) return;
	
	pgCopyScreen();
	
	lines=1;	
	j=0;
	len=0;
	width=0;
	while (c=msg[len]) {
		if (((c>=0x80) && (c<0xa0)) || (c>=0xe0)) {
			len++;
		} else {
			if (c=='\n') {
				lines++;			
				if (width<(len-j)) width=len-j;
				j=len+1;
			}
		}
		len++;
	}	
	if (width<(len-j)) width=len-j;
	if (!width) width=strlen(msg);
	
	y=(272-lines*10)/2;
	x=(480-width*5)/2;
	
	pgFillBox(x-10-1,y-5-1,x+width*5+10+1,y+lines*10+5,BOX_COLOR);
	pgDrawFrame(x-10,y-5,x+width*5+10,y+lines*10+5-1,FRAME_COLOR);
	
	len=0;
	for (i=0;i<lines;i++){
		j=0;
		while (c=msg[len]) {
			if (((c>=0x80) && (c<0xa0)) || (c>=0xe0)) {
				str[j++]=msg[len++];
			} else {
				if (c=='\n') break;
			}
			str[j++]=msg[len++];
		}
		if (msg[len]=='\n') len++;
		str[j]=0;				
		mh_printCenter(y,str,TEXT_COLOR);
		ret=y;
		y+=10;
	}
			
	
	pgScreenFlipV();
	if (delay_vblank>0) pgWaitVn(delay_vblank);
	return ret;
}

int msgBoxLinesRaw(const char *msg,int rev_line) {
	int len,width;
	unsigned char c;
	char str[256];
	int i,j,x,y,lines,ret;
	
	if (!msg) return;
  if (!(msg[0])) return;
	
	pgCopyScreen();
	
	lines=1;	
	j=0;
	len=0;
	width=0;
	while (c=msg[len]) {
		if (((c>=0x80) && (c<0xa0)) || (c>=0xe0)) {			
			len++;
		} else {
			if (c=='\n') {
				lines++;			
				if (width<(len-j)) width=len-j;
				j=len+1;
			}
		}
		len++;
	}	
	if (width<(len-j)) width=len-j;
	if (!width) width=strlen(msg);
	
	y=(272-lines*10)/2;
	x=(480-width*5)/2;
	
	pgFillBox(x-10-1,y-5-1,x+width*5+10+1,y+lines*10+5,BOX_COLOR);
	pgDrawFrame(x-10,y-5,x+width*5+10,y+lines*10+5-1,FRAME_COLOR);
	
	len=0;
	for (i=0;i<lines;i++){
		j=0;
		while (c=msg[len]) {
			if (((c>=0x80) && (c<0xa0)) || (c>=0xe0)) {
				str[j++]=msg[len++];
			} else {
				if (c=='\n') break;
			}
			str[j++]=msg[len++];
		}
		if (msg[len]=='\n') len++;
		str[j]=0;
		if (i==rev_line) {
			width=mh_length(str);
			x=(480-width)>>1;
			pgFillBox(x-1,y-1,x+width+1,y+10-1,0);
			pgDrawFrame(x-1,y-1,x+width+1,y+10-1,7<<10);
			mh_printCenter(y,str,28<<5);
		}
		else mh_printCenter(y,str,TEXT_COLOR);
		ret=y;
		y+=10;
	}	
	return ret;				
}

void msgBoxLinesRawPosLimit(int x,int y,int w,int h,const char *msg) {
	int len,width;
	unsigned char c;
	char str[256];
	int x0,y0,i,j,lines;
	
	x0=x;y0=y;
	
	if (!msg) return;
  if (!(msg[0])) return;
	
	lines=1;	
	j=0;
	len=0;
	width=0;
	while (c=msg[len]) {
		if (((c>=0x80) && (c<0xa0)) || (c>=0xe0)) {			
			len++;
		} else {
			if (c=='\n') {
				lines++;			
				if (width<(len-j)) width=len-j;
				j=len+1;
			}
		}
		len++;
	}	
	if (width<(len-j)) width=len-j;
	if (!width) width=strlen(msg);
	
	pgFillBoxHalfer(x,y,x+w,y+h);
	//now draw frame
	//...
	pgDrawFrame(x-1,y-1,x+w,y+h+1,12|(12<<5)|(12<<10));
	pgDrawFrame(x-2,y-2,x+w,y+h+2,24|(24<<5)|(24<<10));
	pgDrawFrame(x-3,y-3,x+w,y+h+3,31|(31<<5)|(31<<10));
			
	len=0;
	for (i=0;i<lines;i++){
		j=0;
		while (c=msg[len]) {
			if (((c>=0x80) && (c<0xa0)) || (c>=0xe0)) {
				str[j++]=msg[len++];
			} else {
				if (c=='\n') break;
			}
			str[j++]=msg[len++];
		}
		if (msg[len]=='\n') len++;
		str[j]=0;		
		mh_printLimit(x+5,y+5,x0+w-10,y0+h-10,str,TEXT_COLOR);
		y+=10;
	}
}


void changeCodeVal(u32 idx,int dir,unsigned char *fmt,unsigned char *code) {
	if ((!fmt)||(!code)) return;
	if (strlen(fmt)!=strlen(code)) return;
	if (idx>=strlen(fmt)) return;	
	code[idx]+=dir;
	if (dir<0) {		
		switch (fmt[idx]) {
			case 'X': //hexa
				if (code[idx]<'0') code[idx]='F';
				if ((code[idx]<'A')&&(code[idx]>'9')) code[idx]='9';
				break;
			case '9': //digit
				if (code[idx]<'0') code[idx]='9';				
				break;
			case 'a': //alpha lowercase
				if (code[idx]<'a') code[idx]='z';				
				break;
			case 'A': //alpha uppercase
				if (code[idx]<'A') code[idx]='Z';				
				break;
		}
	} else {
		switch (fmt[idx]) {
			case 'X': //hexa
				if (code[idx]>'F') code[idx]='0';
				if ((code[idx]>'9')&&(code[idx]<'A')) code[idx]='A';
				break;
			case '9': //digit
				if (code[idx]>'9') code[idx]='0';
				break;
			case 'a': //alpha lowercase
				if (code[idx]>'z') code[idx]='a';
				break;
			case 'A': //alpha uppercase
				if (code[idx]>'Z') code[idx]='A';
				break;
		}
	}
}

/////////////////////////////////////////////////
//  InputCodeBox : handle input of a formatted code
//
//   format is a printf like string
//      X => hexadecimal
//      9 => decimal
//      A => alpha uppercase
//      a => alpha lowcase
//
//  example : "%A%a%a%a : %X%X%X%X-%9%9"
/////////////////////////////////////////////////
static unsigned char msgCodeBox[2048];
static unsigned char newCode[256];
static unsigned char newFmt[256];
static unsigned char tmpstr[100];
int InputCodeBox(char *msg,char *fmt,char *code) {
	int i,c,j,l,cpt;
	int pad_cnt,old_pad,new_pad;
	int selmax,sel=0;
	if ((!code)||(!msg)||(!fmt)) return -1;
			
	//compute the code length using fmt string
	i=0;j=0;c=0;selmax=0;
	while (fmt[i]) {
		if (c==0) {
			if (fmt[i]=='%') c=1;
		} else if (c==1) {
			if (strchr("Aa9X",fmt[i])) {
				selmax++;
				newFmt[j++]=fmt[i];
			}
			c=0;
		}
		i++;
	}
	//exit if code length is null or if code is shorter than needed
	
	if ((!selmax)||(strlen(code)!=selmax)) return -2;
	
	//init the local code value
	strcpy(newCode,code);
	
	pad_cnt=old_pad=new_pad=0;
	//main loop
	for (;;) {
		cpt++;
		//input scan		
		new_pad=0;
	  if (!pad_cnt) {
	   	new_pad=get_pad();    	
	   	if (new_pad) {
				if (old_pad==new_pad) pad_cnt=2;
				else pad_cnt=4;
			} 
			old_pad=new_pad;   	
	  }
	  else pad_cnt--;
	  pgWaitV();
		
		//build the message to display		
		
		
		sprintf(msgCodeBox,"%s\n\n",msg);
		
		//put in l the line where the code will be input
		l=0;i=0; while (msgCodeBox[i]) {if (msgCodeBox[i]=='\n') l++; i++;}
		
		
		
		//this is NOT optimized :p
		tmpstr[0]=0;
		i=0;c=0;j=0;
		while (fmt[i]) {
			if (c==0) {
				if (fmt[i]=='%') c=1;
				else {
					sprintf(msgCodeBox,"%s%c",msgCodeBox,fmt[i]);
					strcat(tmpstr," ");
				}
			} else if (c==1) {
				if (strchr("Aa9X",fmt[i])) {
					sprintf(msgCodeBox,"%s%c",msgCodeBox,newCode[j]);
					if ((j==sel)&&(cpt>>3)&1) strcat(tmpstr,"^");
					else strcat(tmpstr," ");
					j++;
				} else {
					sprintf(msgCodeBox,"%s%c",msgCodeBox,fmt[i]);
					strcat(tmpstr," ");
				}
				c=0;
			}
			i++;
		}
		sprintf(msgCodeBox,"%s\n%s\n" SJIS_CIRCLE " ok    " SJIS_CROSS " cancel",msgCodeBox,tmpstr);
				
		//display it
		msgBoxLinesRaw(msgCodeBox,l);
		
		pgScreenFlipV();
		
		//wait for user input & handle it
		
		if (new_pad&PSP_CTRL_CIRCLE) {				
			strcpy(code,newCode);
			return 0;			
		}
		//code cancelled
		if (new_pad&PSP_CTRL_CROSS) return 1;
		//move right
		if (new_pad&PSP_CTRL_RIGHT) {
			if (sel<selmax-1) {sel++;os9x_beep1();}			
		}
		if (new_pad&PSP_CTRL_LEFT) {
			if (sel>0) {sel--;os9x_beep1();}			
		}
		if (new_pad&PSP_CTRL_UP) {
			changeCodeVal(sel,1,newFmt,newCode);
			os9x_beep2();			
		}
		if (new_pad&PSP_CTRL_DOWN) {
			changeCodeVal(sel,-1,newFmt,newCode);
			os9x_beep2();			
		}
	}
}


void psp_showProgressBar(int pos,int len) {
	int i,x,y;	
	i=240-100+(len-pos)*230/len;
	if (i>240+100) i=240+100;
	//pgCopyScreen();		
	pgFillBox(240-100,186-7,i,186+7,ZIPBOX_COLOR);
	if (i<240+100) pgFillBox(i+1,186-7,240+100,186+7,0);		
	pgDrawFrame(240-100-1,186-7-1,240+100+1,186+7+1,ZIPFRAME_COLOR);		
	pgScreenFlip();
}

typedef struct {float x;float y;float vx;float vy;} t_stars;

#include "snd_jump.c"
#include "snd_hit.c"
#include "snd_coin.c"
#include "snd_logo4.c"

#include "gfx_sprites.c"
#include "gfx_logo.c"
#include "gfx_snes.c"
#include "gfx_emu.c"
#include "gfx_for.c"
#include "gfx_psp.c"
#include "gfx_laxer3a.c"
#include "gfx_yoyofr.c"
#define STARS_NB 128

#define timercmp(a, b, CMP)	(((a)->tv_sec == (b)->tv_sec) ? ((a)->tv_usec CMP (b)->tv_usec) : ((a)->tv_sec CMP (b)->tv_sec))

extern char LaunchDir[256];
extern char str_tmp[256];
void intro_anim() {
	int snd_handle[7];
	struct timeval intro_next,intro_cur;
	IMAGE *img_sprites,*img_logo,*img_snes,*img_emu,*img_for,*img_psp,*img_laxer3a,*img_yoyofr;
	//FILE *f;
	int anim_step,x,y,i,a,b,ysw,c,addfade_inv,j,sx,sy,starsfade,sc,stars_cpt,stars_snd;
	int addfade[7];
	int *my_sin;
	t_stars *stars;
	
	stars=(t_stars*)malloc(STARS_NB*sizeof(t_stars));
	if (!stars) {return;}
	
	my_sin=(int*)malloc(128*sizeof(int));
	if (!my_sin) {free(stars); return;}
							
	img_sprites = load_bmp_buffer(gfx_sprites);		
	if (!img_sprites) return;			
	img_logo = load_bmp_buffer(gfx_logo);	
	if (!img_logo) {image_free(img_sprites);return;}
	img_snes = load_bmp_buffer(gfx_snes);	
	if (!img_snes) {image_free(img_sprites);image_free(img_logo);return;}
	img_emu = load_bmp_buffer(gfx_emu);	
	if (!img_emu) {image_free(img_sprites);image_free(img_logo);image_free(img_snes);return;}
	img_for = load_bmp_buffer(gfx_for);	
	if (!img_for) {image_free(img_sprites);image_free(img_logo);image_free(img_snes);image_free(img_emu);return;}
	img_psp = load_bmp_buffer(gfx_psp);	
	if (!img_psp) {image_free(img_sprites);image_free(img_logo);image_free(img_snes);image_free(img_emu);image_free(img_for);return;}
	img_laxer3a = load_bmp_buffer(gfx_laxer3a);	
	if (!img_laxer3a) {image_free(img_sprites);image_free(img_logo);image_free(img_snes);image_free(img_emu);image_free(img_for);image_free(img_psp);return;}
	img_yoyofr = load_bmp_buffer(gfx_yoyofr);	
	if (!img_yoyofr) {image_free(img_sprites);image_free(img_logo);image_free(img_snes);image_free(img_emu);image_free(img_for);image_free(img_psp);image_free(img_laxer3a);return;}
				
	
	
	for (i=0;i<128;i++) my_sin[i]=(int)(255-abs(255*sin(i*3.14519f/128)));
		
	for (i=0;i<8;i++) {
		sceAudioChRelease(i);		
	}		
		
	//sound stuff for intro	
	snd_handle[0]=sceAudioChReserve( -1, ((size_snd_jump-44)/2)&(~63), PSP_AUDIO_FORMAT_MONO );		
	snd_handle[1]=sceAudioChReserve( -1, ((size_snd_hit-44)/2)&(~63), PSP_AUDIO_FORMAT_MONO);		
	for (i=2;i<7;i++) snd_handle[i]=sceAudioChReserve( -1, ((size_snd_coin-44)/2)&(~63), PSP_AUDIO_FORMAT_MONO );

#ifndef ME_SOUND
	scePowerSetClockFrequency(300,300,150);
#endif								
		
	anim_step=0;
	starsfade=0;
	stars_cpt=0;
	stars_snd=2;
	//old_x=0;
	//old2_x=0;
	
	for (i=0;i<16;i++) {	
		x=i*2;
		pgFillvram(x|(x<<5)|(x<<10));
		pgScreenFlipV();
	}
	
	struct timeval now;
	sceKernelLibcGettimeofday( &now, 0 );		
	srand((now.tv_usec+now.tv_sec*1000000));
	
	for (j=0;j<STARS_NB;j++) {
		stars[j].x=240;
		stars[j].y=200-12-32-8;
		stars[j].vx=((float)((rand()%256)-128))/32.0f;
		stars[j].vy=-((float)(rand()%256))/64.0f-2.0f;
	}
	
	y=200;
	x=0;
	a=0;
	b=0;				
	c=128;
	for (i=0;i<7;i++) addfade[i]=0xFF;
	addfade_inv=0;
	OSPC_PlayBuffer((char*)snd_logo4,size_snd_logo4,1,MAXVOLUME);
	
	
	sceKernelLibcGettimeofday( &intro_next, 0 );
  intro_next.tv_usec+=20*1000;
	
	for (i=0;i<120+40+8+24+32+120+300;i++) {						
		pgFillvram(0xFFFFFF);
		
		if (get_pad()) break;
	
	//logo
		if (i>=(120+40+8+24+32+120+300-32)) {
			for (j=0;j<7;j++) {
				if (addfade[j]<0xFF) {addfade[j]+=8;if (addfade[j]>0xFF) addfade[j]=0xFF;}				
				switch (j) {
					case 0:image_put_clip((480-img_logo->width)/2,70,img_logo,0,addfade[0]|(addfade[0]<<8)|(addfade[0]<<16),0,0,img_logo->width,img_logo->height,0xFFFFFF);break;
					case 1:image_put_clip(90,160,img_snes,0,addfade[j]|(addfade[j]<<8)|(addfade[j]<<16),0,0,img_snes->width,img_snes->height,0xFFFFFF);break;
					case 2:image_put_clip(90+70+10,160,img_emu,0,addfade[j]|(addfade[j]<<8)|(addfade[j]<<16),0,0,img_emu->width,img_emu->height,0xFFFFFF);break;
					case 3:image_put_clip(90+70+10+106+10,160,img_for,0,addfade[j]|(addfade[j]<<8)|(addfade[j]<<16),0,0,img_for->width,img_for->height,0xFFFFFF);break;
					case 4:image_put_clip(90+70+10+106+10+36+10,160,img_psp,0,addfade[j]|(addfade[j]<<8)|(addfade[j]<<16),0,0,img_psp->width,img_psp->height,0xFFFFFF);break;
					case 5:image_put_clip(10,220,img_laxer3a,0,addfade[j]|(addfade[j]<<8)|(addfade[j]<<16),0,0,img_laxer3a->width,img_laxer3a->height,0xFFFFFF);break;
					case 6:image_put_clip(360,220,img_yoyofr,0,addfade[j]|(addfade[j]<<8)|(addfade[j]<<16),0,0,img_yoyofr->width,img_yoyofr->height,0xFFFFFF);break;
				}
			}
		} else {				
			if (c<96) {
				if (addfade[0]) {addfade[0]-=8;if (addfade[0]<0) addfade[0]=0;}
				image_put_clip((480-img_logo->width)/2,70,img_logo,0,addfade[0]|(addfade[0]<<8)|(addfade[0]<<16),0,0,img_logo->width,img_logo->height,0xFFFFFF);			
			}
			for (j=1;j<7;j++) {
				if (c<64-j*8) {
					if (addfade[j]) {addfade[j]-=8;if (addfade[j]<0) addfade[j]=0;}
				}
				switch (j) {
					case 1:image_put_clip(90,160,img_snes,0,addfade[j]|(addfade[j]<<8)|(addfade[j]<<16),0,0,img_snes->width,img_snes->height,0xFFFFFF);break;
					case 2:image_put_clip(90+70+10,160,img_emu,0,addfade[j]|(addfade[j]<<8)|(addfade[j]<<16),0,0,img_emu->width,img_emu->height,0xFFFFFF);break;
					case 3:image_put_clip(90+70+10+106+10,160,img_for,0,addfade[j]|(addfade[j]<<8)|(addfade[j]<<16),0,0,img_for->width,img_for->height,0xFFFFFF);break;
					case 4:image_put_clip(90+70+10+106+10+36+10,160,img_psp,0,addfade[j]|(addfade[j]<<8)|(addfade[j]<<16),0,0,img_psp->width,img_psp->height,0xFFFFFF);break;
					case 5:image_put_clip(10,220,img_laxer3a,0,addfade[j]|(addfade[j]<<8)|(addfade[j]<<16),0,0,img_laxer3a->width,img_laxer3a->height,0xFFFFFF);break;
					case 6:image_put_clip(360,220,img_yoyofr,0,addfade[j]|(addfade[j]<<8)|(addfade[j]<<16),0,0,img_yoyofr->width,img_yoyofr->height,0xFFFFFF);break;
				}
			}
		}
		
		if (c<128) {
			addfade_inv=((c|(c<<8)|(c<<16))<<1)^0xFFFFFF;
			if (c) c--;
			if ((!(i&1))&&(stars_cpt<STARS_NB)) {
				stars_cpt++;
				if (!(i&7)) {
  				if (!sceAudioGetChannelRestLen(snd_handle[stars_snd])) {
						sceAudioOutput (snd_handle[stars_snd], MAXVOLUME/4, (char*)(&(snd_coin[44])));					
						stars_snd++;
						if (stars_snd>6) stars_snd=2;
					}
				}
			}
			for (j=0;j<stars_cpt;j++) {
				stars[j].x+=stars[j].vx;
				stars[j].y+=stars[j].vy;
				stars[j].vy+=0.1f;
				sx=(int)(stars[j].x);
				sy=(int)(stars[j].y);
				if ((sx>0)&&(sx<480-16)&&(sy>0)&&(sy<272-16)) {
					sc=(my_sin[(j+starsfade)&127]<<0) |
						 (my_sin[(j*2-starsfade)&127]<<8) |
						 (my_sin[(j*3+starsfade)&127]<<16);						 
					image_put_clip(sx,sy,img_sprites,0,sc,16,32,16,16,0x00FF00);
				}
			}
			starsfade++;
		}
		
		//switch
		ysw=200-12-32-(8*sin(b*3.14159f/16)*sin(b*3.14159f/16));
		if (b) b--;
		if (addfade_inv<0xFE0000) image_put_clip(240,ysw,img_sprites,0,addfade_inv,0,32,16,16,0x00FF00);
		
		
		if (i<120) {			
			//mario walking
			if ((x>0)&&(x<480-16))
			switch ((anim_step/5)&3) {
				case 0:image_put_clip(x,y-1,img_sprites,0,0,5*16,0*16,16,32,0x00FF00);break;
				case 1:image_put_clip(x,y,img_sprites,0,0,3*16,0*16,16,32,0x00FF00);break;				
				case 2:image_put_clip(x,y-2,img_sprites,0,0,4*16,0*16,16,32,0x00FF00);break;				
				case 3:image_put_clip(x,y,img_sprites,0,0,3*16,0*16,16,32,0x00FF00);break;				
			}
			x+=2;
			anim_step++;
		} else if (i<120+40){
			image_put_clip(x,y,img_sprites,0,0,0*16,0*16,16,32,0x00FF00);
		} else if (i<120+40+16){
			//mario jump - take off
			
			if (i==(120+40)) sceAudioOutput(snd_handle[0], MAXVOLUME/2, (char*)(&(snd_jump[44])));
			
			y=200-(32*sin(a*3.14159f/32)*sin(a*3.14159f/32));
			a++;		
			
			image_put_clip(x,y,img_sprites,0,0,1*16,0*16,16,32,0x00FF00);
			
		}	else if (i<120+40+16+16){			
			//mario jump - fall
			
			if (i==(120+40+16)) {
				//hit the bonus brick
				b=16;
				sceAudioOutput(snd_handle[1], MAXVOLUME/1, (char*)(&(snd_hit[44])));	
			}
										
			if (i==120+40+16+15) {
				//stars & logo coming
				c=127;				
			}
										
			y=200-(32*sin(a*3.14159f/32)*sin(a*3.14159f/32));
			a++;			
			image_put_clip(x,y-2,img_sprites,0,addfade_inv,2*16,0*16,16,32,0x00FF00);
		}	else if ((i<120+40+16+16+120)&&(x<480)){
			//mario walking			
			if ((x>0)&&(x<480-16))
			switch ((anim_step/5)&3) {
				case 0:image_put_clip(x,y-1,img_sprites,0,0,5*16,0*16,16,32,0x00FF00);break;
				case 1:image_put_clip(x,y,img_sprites,0,0,3*16,0*16,16,32,0x00FF00);break;				
				case 2:image_put_clip(x,y-2,img_sprites,0,0,4*16,0*16,16,32,0x00FF00);break;				
				case 3:image_put_clip(x,y,img_sprites,0,0,3*16,0*16,16,32,0x00FF00);break;				
			}
			x+=2;
			anim_step++;
		}
		
		//pgScreenFlipV();
		pgScreenFlipV2();
		
		for (;;) {
			sceKernelLibcGettimeofday( &intro_cur, 0 );
			if ( timercmp( &intro_next, &intro_cur, < ) ){
				break;
  		}
  	}
  	intro_next=intro_cur;
  	intro_next.tv_usec+=20*1000; //ms
  	while ( intro_next.tv_usec >= 1000000 ){
	  	intro_next.tv_sec += 1;
      intro_next.tv_usec -= 1000000;
	  }
		
	}
		
	for (i=0;i<7;i++) {
		sceAudioChRelease(snd_handle[i]);		
	}		
	image_free(img_sprites);
	image_free(img_logo);
	image_free(img_snes);
	image_free(img_emu);
	image_free(img_for);
	image_free(img_psp);
	image_free(img_laxer3a);
	image_free(img_yoyofr);
	free(my_sin);
	free(stars);
	
	OSPC_StopPlay();
#ifndef ME_SOUND
	scePowerSetClockFrequency(222,222,111);
#endif	
	for (i=15;i>=0;i--) {	
		x=i*2;
		pgFillvram(x|(x<<5)|(x<<10));
		pgScreenFlipV();
	}
	
	while (get_pad()) ;
}
