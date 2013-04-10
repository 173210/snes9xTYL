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
	sprintf(str,"%s\n\n                    ", msg);
	y=msgBoxLinesRaw(str,-1);
	mh_printCenter(y,SJIS_CIRCLE " OK             ",TEXT_COLOR_OK);
	sprintf(str,"         " SJIS_CROSS " %s", psp_msg_string(CANCEL));
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
	strcat(str,psp_msg_string(INPUTBOX_OK));
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
		sprintf(msgCodeBox,"%s\n%s\n",msgCodeBox,tmpstr);

		//display it
		i = msgBoxLinesRaw(msgCodeBox,l);

		mh_printCenter(i,SJIS_CIRCLE " OK             ",TEXT_COLOR_OK);
		sprintf(msgCodeBox,"             " SJIS_CROSS " %s", psp_msg_string(CANCEL));
		mh_printCenter(i,msgCodeBox,TEXT_COLOR_CANCEL);

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
