#include "snes9x.h"

#include "psp/openspc++/os9xZ_openspc.h"
#include "psp/openspc++/openspc.h"


#include "psp/psp.h"
extern "C" {	
#include <pspaudio.h>
#include "menu_fx.h"

int msgBoxLines(const char *msg,int delay_vblank=10);

void show_background(int fade,int add);
void show_batteryinfo();   
extern int bg_img_mul;

extern int os9x_lowbat;
}


#define BG_FADE_CST 0x202020 //0xD2D2D2

openspc *ospc;
char *spc_data;
int spc_data2free;


static __attribute__((aligned(64))) int16 snd_buffer[16384];

SPC_ID666 *OSPC_id=NULL;
int OSPC_sound_fd;
int OSPC_thread=-1;
volatile int OSPC_exit,OSPC_volume;
struct timeval OSPC_cur_time,OSPC_start_time;

extern "C" {
int OSPC_Init()
{
	ospc=new openspc();	
	return 0;
}

void OSPC_Close(void)
{	
	delete ospc;
	if (spc_data2free) free(spc_data);	
}

SPC_ID666 *OSPC_GetID666(char *buf)
{
  SPC_ID666 *id;
  char tmps[16];
  int i;

  id = (SPC_ID666 *)malloc(sizeof(*id));
  if (id == NULL)
    return NULL;

  
  if (buf[0x23] == 27) {  
      free(id);
      return NULL;
  }

  memcpy(id->songname,buf+0x2E,32);  
  id->songname[32] = '\0';

  memcpy(id->gametitle,buf+32+0x2E,32); 
  id->gametitle[32] = '\0';

  memcpy(id->dumper,buf+32*2+0x2E,16);
  id->dumper[16] = '\0';

  memcpy(id->comments,buf+32*2+17+0x2E,32);  
  id->comments[32] = '\0';

  switch (buf[0xD1]) {
  case 1:
      id->emulator = SPC_EMULATOR_ZSNES;
      break;
  case 2:
      id->emulator = SPC_EMULATOR_SNES9X;
      break;
  case 0:
  default:
      id->emulator = SPC_EMULATOR_UNKNOWN;
      break;
  }

  memcpy(id->author,buf+0xB0+1,32);  
  id->author[32] = '\0';
  
  tmps[0]=buf[0xA9];
  tmps[1]=buf[0xAA];
  tmps[2]=buf[0xAB];
  tmps[3]=0;
  id->playtime=0;
  i=0;
  while (tmps[i])
  {
	id->playtime*=10;
  	id->playtime+=tmps[i]-'0';
  	i++;
  }
  tmps[0]=buf[0xAC];
  tmps[1]=buf[0xAD];
  tmps[2]=buf[0xAE];
  tmps[3]=buf[0xAF];
  tmps[4]=0;
  id->fadetime=0;
  while (tmps[i])
  {
	id->fadetime*=10;
  	id->fadetime+=tmps[i]-'0';
  	i++;
  }
  

  return id;
  
}

int OSPC_Load(char *fname)
{
  int err_code;
  FILE  *f;  
  unsigned long filesize;
  f=fopen(fname,"rb");
  if (f) { 
  	fseek(f,0,SEEK_END);
  	filesize=ftell(f);
  	fseek(f,0,SEEK_SET); 	
  	spc_data=(char*)malloc(filesize);
  	spc_data2free=1;
		fread(spc_data,1,filesize,f);
		fclose(f);			
		return ospc->init(spc_data,filesize);
  } else  {
  	spc_data=NULL;
  	spc_data2free=0;
  	return -1;
  }    
}

int OSPC_LoadBuffer(char *buff,int len) {  
	spc_data2free=0;
	spc_data=buff;
	return ospc->init(buff,len);  
}

void OSPC_Stop(void)
{
}

int OSPC_PlayThread (SceSize ,void *) {
	for (;;) {			   	
		ospc->run(-1,(int16*)snd_buffer,(int)(1024*4));    			
		sceAudioOutputPannedBlocking( OSPC_sound_fd, OSPC_volume, OSPC_volume, (char*)snd_buffer);
				
		if (OSPC_exit) break;
  }
}

int OSPC_IsFinished() {
	int i;
	if (!OSPC_id) return 0;
	if (!(OSPC_id->playtime)) return 0;
	sceKernelLibcGettimeofday( &OSPC_cur_time, 0 );
	i=(OSPC_cur_time.tv_sec-OSPC_start_time.tv_sec)+(OSPC_cur_time.tv_usec-OSPC_start_time.tv_usec)/1000000;
	return i>(OSPC_id->playtime);
}

char *OSPC_SongName() {	
	if (!OSPC_id) return NULL;
	return OSPC_id->songname;
}

char *OSPC_GameTitle() {	
	if (!OSPC_id) return NULL;
	return OSPC_id->gametitle;
}

char *OSPC_Author() {	
	if (!OSPC_id) return NULL;
	return OSPC_id->author;
}


void OSPC_show_bg(u16 *menu_bg){
	u16 *src,*dst;
	int i;
	static int cpt_lowbat=0;
	
	if (!((cpt_lowbat++)&63)) {
		int oldvalue=os9x_lowbat;
		os9x_lowbat=scePowerIsLowBattery();
		if (oldvalue!=os9x_lowbat) {
			u16 *src,*dst;
			int i;
			dst=menu_bg;
			show_background(bg_img_mul,(os9x_lowbat?0x600000:0));	
			for (i=0;i<272;i++) {
				src = (u16*)pgGetVramAddr(0,i);						
				memcpy(dst,src,480*2);
				dst+=480;
			}
		}
	}
		
	src=menu_bg;
	for (i=0;i<272;i++) {
		dst = (u16*)pgGetVramAddr(0,i);						
		memcpy(dst,src,480*2);
		src+=480;
	}
		
}

void OSPC_Play(char *fname,int release,int vol) {
	  u16 *menu_bg;	  
		u16 *dst,*src;
    int i,j,pollcpt;    
    char str[256];    
    char *emulator[3]={"Unknown","Zsnes","Snes9x"};
    uint8 *scr;
   
    OSPC_Init();

    if (i=OSPC_Load(fname))
    {
    	sprintf(str,"Error at SPC loading, code : %d",i);
    	msgBoxLines(str,60);
    	//gp32_pause();
    	//GpAppExit();
    	return;
    }
     

	OSPC_id=OSPC_GetID666(spc_data);
	
  OSPC_sound_fd = sceAudioChReserve( -1, 1024, 0 );
  OSPC_exit=0;
  OSPC_volume=vol;
  OSPC_thread = sceKernelCreateThread( "OSPC Thread", (SceKernelThreadEntry)OSPC_PlayThread, 0x8, 256*1024, 0, 0 );    
  if (OSPC_thread<0) {
  	msgBoxLines("Cannot create OSPC playback thread",60);
  } else {
  	//init start time
#ifndef ME_SOUND
	  scePowerSetClockFrequency(266,266,133);
#endif
	  sceKernelLibcGettimeofday( &OSPC_start_time, 0 );
  	
  	sceKernelStartThread( OSPC_thread, 0, 0 );
  	
  	if (release) return;  		  		  			
  	//init bg
  	menu_bg=(u16*)malloc_64(480*272*2);
		dst=menu_bg;
		show_background(bg_img_mul,(os9x_lowbat?0x600000:0));	
		for (i=0;i<272;i++) {
			src = (u16*)pgGetVramAddr(0,i);						
			memcpy(dst,src,480*2);
			dst+=480;
		}
		//init fx
		fx_init();  		  			
  	for (;;) {
  		//show bg
  		OSPC_show_bg(menu_bg);
  		//show bg fx
  		fx_main(pgGetVramAddr(0,0));
  		
   		//batt infos
  		show_batteryinfo();
  		//music info
  		//draw frame
  		pgDrawFrame(14,14,20+230+5+1,75+1,12|(12<<5)|(12<<10));
  		pgDrawFrame(13,13,20+230+5+2,75+2,30|(30<<5)|(30<<10));
  		pgDrawFrame(12,12,20+230+5+3,75+3,12|(12<<5)|(12<<10));
  		pgFillBoxHalfer(15,15,20+230+5,75);
  		//
  		if (strlen(OSPC_id->gametitle)) sprintf(str,"Game : %s",OSPC_id->gametitle);
  		else sprintf(str,"Game : unknown");
  		mh_print(20,20,(char*)str,30|(30<<5)|(30<<10));
  		
  		if (strlen(OSPC_id->songname)) sprintf(str,"Song : %s",OSPC_id->songname);
  		else sprintf(str,"Song : unknown");  			
  		mh_print(20,30,(char*)str,30|(30<<5)|(30<<10));
  		
  		if (strlen(OSPC_id->dumper)) sprintf(str,"Dumper : %s",OSPC_id->dumper);
  		else sprintf(str,"Dumper : unknown");
  		mh_print(20,40,(char*)str,30|(30<<5)|(30<<10));
  		
  		if (strlen(OSPC_id->comments)) sprintf(str,"Comments : %s",OSPC_id->comments);
  		else sprintf(str,"Comments : unknown");
  		mh_print(20,50,(char*)str,30|(30<<5)|(30<<10));
  		
  		if (strlen(OSPC_id->author)) sprintf(str,"Author : %s",OSPC_id->author);
  		else sprintf(str,"Author : unknown");
  		mh_print(20,60,(char*)str,30|(30<<5)|(30<<10));
			//time infos
			//draw frame
			//draw frame
  		pgDrawFrame(14,94,20+65+5+1,116,8|(8<<5)|(16<<10));
  		pgDrawFrame(13,93,20+65+5+2,117,28|(28<<5)|(31<<10));
  		pgDrawFrame(12,92,20+65+5+3,118,8|(8<<5)|(16<<10));
			pgFillBoxHalfer(15,95,20+65+5,115);
			sceKernelLibcGettimeofday( &OSPC_cur_time, 0 );
			i=(OSPC_cur_time.tv_sec-OSPC_start_time.tv_sec)+(OSPC_cur_time.tv_usec-OSPC_start_time.tv_usec)/1000000;
  		sprintf(str,"%2d%c%.2d / %2d:%.2d",i/60,((i&1)?':':' '),i%60,OSPC_id->playtime/60,OSPC_id->playtime%60);
  		mh_print(20,100,(char*)str,(20)|(31<<5)|(18<<10));
  		  		
  		if (get_pad()) break;
  			
  		pgScreenFlip();
  	}
  	OSPC_exit=1;  	
  	sceKernelWaitThreadEnd( OSPC_thread, NULL );
		sceKernelDeleteThread( OSPC_thread );
		OSPC_thread=-1;
		
		free(menu_bg);
		fx_close();
  }    
  sceAudioChRelease( OSPC_sound_fd );
  OSPC_Stop();            
  OSPC_Close();    
  if (OSPC_id) free(OSPC_id);
}


void OSPC_PlayBuffer(char *buff,int len,int release,int vol)
{
    int i,j,pollcpt;
    char str[256];
    SPC_ID666 *id;
    char *emulator[3]={"Unknown","Zsnes","Snes9x"};
    uint8 *scr;
   
    OSPC_Init();

    if (i=OSPC_LoadBuffer(buff,len))
    {
    	sprintf(str,"Error at SPC loading, code : %d",i);
    	msgBoxLines(str,60);
    	//gp32_pause();
    	//GpAppExit();
    	return;
    }
     

	OSPC_id=OSPC_GetID666(spc_data);
		
  OSPC_sound_fd = sceAudioChReserve( -1, 1024, 0 );
  OSPC_exit=0;
  OSPC_volume=vol;
  OSPC_thread = sceKernelCreateThread( "OSPC Thread", (SceKernelThreadEntry)OSPC_PlayThread, 0x8, 256*1024, 0, 0 );
  if (OSPC_thread<0) {
  	msgBoxLines("Cannot create OSPC playback thread",60);
  } else {
  	//init start time		
		sceKernelLibcGettimeofday( &OSPC_start_time, 0 );
  	
  	sceKernelStartThread( OSPC_thread, 0, 0 );
  	
  	if (release) return;
  	
  	for (;;) {
  		pgWaitV();
  		if (get_pad()) break;
  	}
  	OSPC_exit=1;  	
  	sceKernelWaitThreadEnd( OSPC_thread, NULL );
		sceKernelDeleteThread( OSPC_thread );
		OSPC_thread=-1;
  }    
  sceAudioChRelease( OSPC_sound_fd );
  OSPC_Stop();            
  OSPC_Close();    
  if (OSPC_id) free(OSPC_id);
}

void OSPC_StopPlay(){
	if (OSPC_thread<0) return;
	OSPC_exit=1;  	
  sceKernelWaitThreadEnd( OSPC_thread, NULL );
	sceKernelDeleteThread( OSPC_thread );
	OSPC_thread=-1;
	sceAudioChRelease( OSPC_sound_fd );
  OSPC_Stop();            
  OSPC_Close(); 
  if (OSPC_id) free(OSPC_id);
}

}
