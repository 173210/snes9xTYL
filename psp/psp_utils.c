#include "unzip.h"

int snd_beep1_handle[6];
int snd_beep1_current;
//int snd_beep2_handle[6];
//int snd_beep2_current;
#include "snd_beep1.c"
#include "snd_beep2.c"


#include "data/icon_cheats.c"
#include "data/icon_game.c"
#include "data/icon_misc.c"
#include "data/icon_pad.c"
#include "data/icon_save.c"
#include "data/icon_sound.c"
#include "data/icon_video.c"
#include "data/icon_about.c"



void check_settings(){
	int i;	
	if (os9x_apu_ratio<16) os9x_apu_ratio=192; //default value
	if (os9x_apu_ratio>512) os9x_apu_ratio=192; //default value
	os9x_hack=os9x_hack&(PPU_IGNORE_FIXEDCOLCHANGES|PPU_IGNORE_WINDOW|PPU_IGNORE_ADDSUB|PPU_IGNORE_PALWRITE|GFX_FASTMODE7|HIRES_FIX);
	if ((os9x_sndfreq!=11025)&&(os9x_sndfreq!=22050)&&(os9x_sndfreq!=33075)&&(os9x_sndfreq!=44100)) os9x_sndfreq=44100;
	if (os9x_padindex>5) os9x_padindex=0;		
	//for an access to menu
#ifndef HOME_HOOK
	for (i=0;i<PSP_BUTTONS_TOTAL;i++) if (os9x_inputs[i]==OS9X_MENUACCESS) break;		
	if (i==PSP_BUTTONS_TOTAL) os9x_inputs[PSP_TL_TR]=OS9X_MENUACCESS;

	if ((i>=PSP_AUP)&&(i<=PSP_ARIGHT)) {
		if (!os9x_inputs_analog) os9x_inputs[PSP_TL_TR]=OS9X_MENUACCESS;
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void load_background(){	
	/*FILE *fd;
	sprintf(str_tmp,"%sDATA/logo.bmp",LaunchDir);
	fd = fopen(str_tmp,"rb");
	if (fd==NULL) {		
		msgBoxLines("can't load bg image\n",60);
		bg_img=NULL;
		return;
	}			
	bg_img = load_bmp(fd);	
	fclose(fd); */

	FILE *jpegFile;	
	unzFile zip_file;
	unz_file_info unzinfo;
	char str[256],*buffer;	
	int l,height;
	sprintf(str,"%sDATA/logo.zip",LaunchDir);
		
	
	zip_file = 0;    
	bg_img=NULL;
	
	zip_file = unzOpen(str);
	if (zip_file) {
		unz_global_info pglobal_info;
		struct timeval now;
		int num;
		sceKernelLibcGettimeofday( &now, 0 );		
		srand((now.tv_usec+now.tv_sec*1000000));
		
		unzGetGlobalInfo(zip_file,&pglobal_info);
		
		if ((bg_img_num<0)||(bg_img_num>=pglobal_info.number_entry)) {
			do {
				num=rand()%pglobal_info.number_entry;
			} while (num==bg_img_num);
			bg_img_num=num;
		} else  num=bg_img_num;
						
		unzGoToFirstFile(zip_file);
		while (num--) {
			unzGoToNextFile(zip_file);
		}
		if (unzGetCurrentFileInfo(zip_file, &unzinfo, str, sizeof(str), NULL, NULL, NULL, NULL) != UNZ_OK) {
			return;
		}
		unzOpenCurrentFile (zip_file);
		
		sprintf(str,"%sDATA/logo.jpg",LaunchDir);
		jpegFile=fopen(str,"wb");
		if (!jpegFile) {
			unzCloseCurrentFile (zip_file);
    	unzClose (zip_file);   
    	return;
    }		
		buffer=(char*)malloc(4096);
		l = unzinfo.uncompressed_size;
		for (;;) {
			if (l>4096) {				
				unzReadCurrentFile(zip_file,(void*)(buffer), 4096, NULL);
				fwrite(buffer,1,4096,jpegFile);
				l-=4096;
			} else {
				unzReadCurrentFile(zip_file,(void*)(buffer), l, NULL);
				fwrite(buffer,1,l,jpegFile);
				break;
			}
		}
		fclose(jpegFile);
		unzCloseCurrentFile (zip_file);
    unzClose (zip_file);   
		
		free(buffer);

		bg_img=(IMAGE*)malloc(sizeof(IMAGE));
		bg_img->width = 480;
		bg_img->height = 272;
		bg_img->bit = 15;
		bg_img->pixels = malloc(480*272*2);
		bg_img->n_palette = 0;
		bg_img->palette = NULL;
		memset(bg_img->pixels,0,480*272*2);
		if (!read_JPEG_file (str,(u16*)bg_img->pixels,480,272,480,&height)) {
			free(bg_img->pixels);
			free(bg_img);
			bg_img=NULL;			
		}
		remove(str);
		
		if (bg_img) {
			u16 *src=(u16*)(bg_img->pixels);
			u16 col;
			float lum=0;
			for (l=480*272;l;l--) {
				col=*src++;
				lum+=(float)(((col>>10))*((col>>10)) + ((col>>5)&31)*((col>>5)&31) + ((col&31))*((col&31)))/(3.0f*31.0f*31.0f);
			}
			lum=lum/(480*272);
			if (lum<0.1f) bg_img_mul=0xE0E0E0;
			else if (lum<0.5f) bg_img_mul=0xC0C0C0;
			else bg_img_mul=0x7F7F7F;
			//sprintf(str,"lum : %f",lum);
			//msgBoxLines(str,60);
		}
	}
}
	
////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
START_EXTERN_C
extern IMAGE *icons[8];
extern int icons_col[8];
END_EXTERN_C
void load_icons(){	
	int i;
	for (i=0;i<8;i++) icons[i]=NULL;
	
	icons[0] = load_bmp_buffer(icon_game);	
	icons_col[0]=0xFF00;
	icons[1] = load_bmp_buffer(icon_save);	
	icons_col[1]=0xFF00;		
	icons[2] = load_bmp_buffer(icon_pad);	
	icons_col[2]=0xFF;
	icons[3] = load_bmp_buffer(icon_video);	
	icons_col[3]=0xFF00;	
	icons[4] = load_bmp_buffer(icon_sound);	
	icons_col[4]=0xFF00;
	icons[5] = load_bmp_buffer(icon_misc);	
	icons_col[5]=0xFF00;
	icons[6] = load_bmp_buffer(icon_cheats);	
	icons_col[6]=0xFF00;	
	icons[7] = load_bmp_buffer(icon_about);
	icons_col[7]=0xFF00;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void show_background(int mul,int add){	
		if (bg_img) image_put_mul((SCREEN_WIDTH-bg_img->width)/2,(SCREEN_HEIGHT-bg_img->height)/2,bg_img,mul,add);	
		else pgFillvram(add);	
	}


////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
//crc = 0 for default
int save_rom_settings(int game_crc32,char *name){
	FILE *f;
	char tmp_str[256];
	int l;	
	if (game_crc32)	sprintf(tmp_str,"%sPROFILES/s9xTYL_%08X.ini",LaunchDir,game_crc32);
	else sprintf(tmp_str,"%sPROFILES/s9xTYL_default.ini",LaunchDir);
	f = fopen(tmp_str,"wb");			
	if (!f){
		ErrorMsg("cannot save settings");
		return -1;
	}
	l=(VERSION_MAJOR<<16)|VERSION_MINOR;
	fwrite(&l,1,4,f);	
	fwrite(&os9x_apuenabled,1,4,f);
	fwrite(&os9x_sndfreq,1,4,f);
	fwrite(&os9x_softrendering,1,4,f);
	fwrite(&os9x_fskipvalue,1,4,f);
	fwrite(&os9x_speedlimit,1,4,f);
	fwrite(&os9x_vsync,1,4,f);
	fwrite(&os9x_showfps,1,4,f);
	fwrite(&os9x_cpuclock,1,4,f);
	fwrite(&os9x_render,1,4,f);
	fwrite(&os9x_smoothing,1,4,f);	
	fwrite(&os9x_OBJ,1,4,f);
	fwrite(&os9x_BG0,1,4,f);
	fwrite(&os9x_BG1,1,4,f);
	fwrite(&os9x_BG2,1,4,f);
	fwrite(&os9x_BG3,1,4,f);
	fwrite(&os9x_easy,1,4,f);
	fwrite(&os9x_fastsprite,1,4,f);
	fwrite(&os9x_gammavalue,1,4,f);
	fwrite(&os9x_hack,1,4,f);
	fwrite(&os9x_forcepal_ntsc,1,4,f);
	fwrite(&os9x_autosavetimer,1,4,f);
	fwrite(&os9x_inputs[0],1,32*4,f);
	strncpy(tmp_str,name,63);
	tmp_str[63]=0;
	fwrite(tmp_str,1,64,f);
	fwrite(&os9x_autosavesram,1,4,f);
	fwrite(&os9x_screenTop,1,4,f);
	fwrite(&os9x_screenLeft,1,4,f);
	fwrite(&os9x_screenWidth,1,4,f);
	fwrite(&os9x_screenHeight,1,4,f);
	fwrite(&os9x_apu_ratio,1,4,f);
	fwrite(&os9x_padindex,1,4,f);
	fwrite(&os9x_inputs_analog,1,4,f);
	fwrite(&os9x_fpslimit,1,4,f);
	fwrite(&os9x_SA1_exec,1,4,f);

	fclose(f);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
//crc = 0 for default
int load_rom_settings(int game_crc32){
	FILE *f;
	char tmp_str[256],rom_name[64];	
	int l,i;
	if (game_crc32)	sprintf(tmp_str,"%sPROFILES/s9xTYL_%08X.ini",LaunchDir,game_crc32);
	else sprintf(tmp_str,"%sPROFILES/s9xTYL_default.ini",LaunchDir);
	f = fopen(tmp_str,"rb");
	if (!f){
		//debug_log("cannot load settings");
		return -1;
	}
	fread(&l,1,4,f);
	if (l<((VERSION_MAJOR_COMP<<16)|VERSION_MINOR_COMP)){
		psp_msg(SETTINGS_IGNORED, MSG_DEFAULT);
		fclose(f);
		return -2;
	}

#define READ_SETTING(a) \
if (fread(&l,1,4,f)==4) a=l; \
else {fclose(f);check_settings();return -3;}
	//{char st[16];sprintf(st,"%s : %d",#a,a);msgBoxLines(st,60);}

//not bigger than tmp_str size, 256
#define READ_SETTING_SIZE(buff,sz) \
if (fread(tmp_str,1,sz,f)==sz) memcpy(buff,tmp_str,sz); \
else {fclose(f);check_settings();return -3;}
	
	READ_SETTING(os9x_apuenabled)
	READ_SETTING(os9x_sndfreq)
	READ_SETTING(os9x_softrendering)
	READ_SETTING(os9x_fskipvalue)
	READ_SETTING(os9x_speedlimit)
	READ_SETTING(os9x_vsync)
	READ_SETTING(os9x_showfps)
	READ_SETTING(os9x_cpuclock)
	READ_SETTING(os9x_render)
	READ_SETTING(os9x_smoothing)	
	READ_SETTING(os9x_OBJ)
	READ_SETTING(os9x_BG0)
	READ_SETTING(os9x_BG1)
	READ_SETTING(os9x_BG2)
	READ_SETTING(os9x_BG3)
	READ_SETTING(os9x_easy)
	READ_SETTING(os9x_fastsprite)
	READ_SETTING(os9x_gammavalue)		
	READ_SETTING(os9x_hack)
	READ_SETTING(os9x_forcepal_ntsc)
	READ_SETTING(os9x_autosavetimer)
	READ_SETTING_SIZE(&os9x_inputs[0],32*4)
	READ_SETTING_SIZE(rom_name,64)
	READ_SETTING(os9x_autosavesram)
	READ_SETTING(os9x_screenTop)
	READ_SETTING(os9x_screenLeft)
	READ_SETTING(os9x_screenWidth)
	READ_SETTING(os9x_screenHeight)
	READ_SETTING(os9x_apu_ratio)
	READ_SETTING(os9x_padindex)
	READ_SETTING(os9x_inputs_analog)
	READ_SETTING(os9x_fpslimit)
	if (fread(&l,1,4,f)==4) os9x_SA1_exec=l; 
	fclose(f);
	
	check_settings();
	
	rom_name[63]=0;	
	sprintf(tmp_str,psp_msg_string(SETTINGS_FOUND),rom_name);
	msgBoxLines(tmp_str,30);
	
	
		
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
//crc = 0 for default
int save_buffer_settings(char *buffer){	
	char tmp_str[256];
	int buffer_ofs=0;
	int l;	
	
	l=(VERSION_MAJOR<<16)|VERSION_MINOR;
	memcpy(&(buffer[buffer_ofs]),&l,4);	buffer_ofs+=4;
	memcpy(&(buffer[buffer_ofs]),&os9x_apuenabled,4);buffer_ofs+=4;
	memcpy(&(buffer[buffer_ofs]),&os9x_sndfreq,4);buffer_ofs+=4;
	memcpy(&(buffer[buffer_ofs]),&os9x_softrendering,4);buffer_ofs+=4;
	memcpy(&(buffer[buffer_ofs]),&os9x_fskipvalue,4);buffer_ofs+=4;
	memcpy(&(buffer[buffer_ofs]),&os9x_speedlimit,4);buffer_ofs+=4;
	memcpy(&(buffer[buffer_ofs]),&os9x_vsync,4);buffer_ofs+=4;
	//memcpy(&(buffer[buffer_ofs]),&os9x_showfps,4);buffer_ofs+=4;
	memcpy(&(buffer[buffer_ofs]),&os9x_cpuclock,4);buffer_ofs+=4;
	memcpy(&(buffer[buffer_ofs]),&os9x_render,4);buffer_ofs+=4;
	//memcpy(&(buffer[buffer_ofs]),&os9x_smoothing,4);	buffer_ofs+=4;
	//memcpy(&(buffer[buffer_ofs]),&os9x_OBJ,4);buffer_ofs+=4;
	//memcpy(&(buffer[buffer_ofs]),&os9x_BG0,4);buffer_ofs+=4;
	//memcpy(&(buffer[buffer_ofs]),&os9x_BG1,4);buffer_ofs+=4;
	//memcpy(&(buffer[buffer_ofs]),&os9x_BG2,4);buffer_ofs+=4;
	//memcpy(&(buffer[buffer_ofs]),&os9x_BG3,4);buffer_ofs+=4;
	//memcpy(&(buffer[buffer_ofs]),&os9x_easy,4);buffer_ofs+=4;
	//memcpy(&(buffer[buffer_ofs]),&os9x_fastsprite,4);buffer_ofs+=4;
	//memcpy(&(buffer[buffer_ofs]),&os9x_gammavalue,4);buffer_ofs+=4;
	memcpy(&(buffer[buffer_ofs]),&os9x_hack,4);buffer_ofs+=4;
	memcpy(&(buffer[buffer_ofs]),&os9x_forcepal_ntsc,4);buffer_ofs+=4;
	memcpy(&(buffer[buffer_ofs]),&os9x_autosavetimer,4);buffer_ofs+=4;
	//memcpy(&(buffer[buffer_ofs]),&os9x_inputs[0],32*4);buffer_ofs+=32*4;
	//strncpy(tmp_str,name,63);
	//tmp_str[63]=0;
	//memcpy(&(buffer[buffer_ofs]),tmp_str,64);buffer_ofs+=64;
	memcpy(&(buffer[buffer_ofs]),&os9x_autosavesram,4);buffer_ofs+=4;
	//memcpy(&(buffer[buffer_ofs]),&os9x_screenTop,4);buffer_ofs+=4;
	//memcpy(&(buffer[buffer_ofs]),&os9x_screenLeft,4);buffer_ofs+=4;
	//memcpy(&(buffer[buffer_ofs]),&os9x_screenWidth,4);buffer_ofs+=4;
	//memcpy(&(buffer[buffer_ofs]),&os9x_screenHeight,4);buffer_ofs+=4;
	memcpy(&(buffer[buffer_ofs]),&os9x_apu_ratio,4);buffer_ofs+=4;
	memcpy(&(buffer[buffer_ofs]),&os9x_fpslimit,4);buffer_ofs+=4;
	
		
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
//crc = 0 for default
int load_buffer_settings(char *buffer){
	char tmp_str[256],rom_name[64];
	int buffer_ofs=0;
	int l;
	
	memcpy(&l,buffer,4);
	buffer_ofs+=4;
	if (l!=((VERSION_MAJOR<<16)|VERSION_MINOR)){
		psp_msg(SETTINGS_DIFF, MSG_DEFAULT);
		return -2;
	}
		
#define READ_SETTING(a) \
	memcpy(&a,&(buffer[buffer_ofs]),4); \
	buffer_ofs+=4;
	
//not bigger than tmp_str size, 256
#define READ_SETTING_SIZE(buff,sz) \
	memcpy(buff,&(buffer[buffer_ofs]),sz); \
	buffer_ofs+=sz;
	
	READ_SETTING(os9x_apuenabled)
	READ_SETTING(os9x_sndfreq)
	READ_SETTING(os9x_softrendering)
	READ_SETTING(os9x_fskipvalue)
	READ_SETTING(os9x_speedlimit)
	READ_SETTING(os9x_vsync)
	//READ_SETTING(os9x_showfps)
	READ_SETTING(os9x_cpuclock)
	READ_SETTING(os9x_render)
	//READ_SETTING(os9x_smoothing)	
	//READ_SETTING(os9x_OBJ)
	//READ_SETTING(os9x_BG0)
	//READ_SETTING(os9x_BG1)
	//READ_SETTING(os9x_BG2)
	//READ_SETTING(os9x_BG3)
	//READ_SETTING(os9x_easy)
	//READ_SETTING(os9x_fastsprite)
	//READ_SETTING(os9x_gammavalue)		
	READ_SETTING(os9x_hack)
	READ_SETTING(os9x_forcepal_ntsc)
	READ_SETTING(os9x_autosavetimer)
	//READ_SETTING_SIZE(&os9x_inputs[0],32*4)
	//READ_SETTING_SIZE(rom_name,64)
	READ_SETTING(os9x_autosavesram)
	//READ_SETTING(os9x_screenTop)
	//READ_SETTING(os9x_screenLeft)
	//READ_SETTING(os9x_screenWidth)
	//READ_SETTING(os9x_screenHeight)
	READ_SETTING(os9x_apu_ratio)
	READ_SETTING(os9x_fpslimit)

	check_settings();

	rom_name[63]=0;

	//sprintf(tmp_str,"Settings received!\n\n""%s""",rom_name);
	//msgBoxLines(tmp_str,30);


	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int save_settings(void){
	FILE *f;
	char tmp_str[256];
	int l;
	sprintf(tmp_str,"%ss9xTYL.ini",LaunchDir);
	f = fopen(tmp_str,"wb");
	if (!f){
		ErrorMsg("cannot save settings");
		return -1;
	}
	l=(VERSION_MAJOR<<16)|VERSION_MINOR;
	fwrite(&l,1,4,f);
	fwrite(romPath,1,256,f);
	fwrite(lastRom,1,256,f);
	fwrite(&os9x_menumusic,1,4,f);
	fwrite(&os9x_menufx,1,4,f);
	fwrite(&os9x_usballowed,1,4,f);
	fwrite(&bg_img_num,1,4,f);
	fwrite(&os9x_menupadbeep,1,4,f);
	fwrite(&os9x_autostart,1,4,f);
	fclose(f);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int load_settings(void){
	FILE *f;
	char tmp_str[256];
	int l;
	sprintf(tmp_str,"%ss9xTYL.ini",LaunchDir);
	f = fopen(tmp_str,"rb");
	if (!f){
		//debug_log("cannot load settings");
		return -1;
	}
	fread(&l,1,4,f);
	if (l<((VERSION_MAJOR_COMP<<16)|VERSION_MINOR_COMP)){
		psp_msg(SETTINGS_IGNORED, MSG_DEFAULT);
		fclose(f);
		return -2;
	}

#define READ_SETTING(a) \
if (fread(&l,1,4,f)==4) a=l; \
else {fclose(f);return -3;}

//not bigger than tmp_str size, 256
#define READ_SETTING_SIZE(buff,sz) \
if (fread(tmp_str,1,sz,f)==sz) memcpy(buff,tmp_str,sz); \
else {fclose(f);return -3;}

	READ_SETTING_SIZE(romPath,256)
	READ_SETTING_SIZE(lastRom,256)
	READ_SETTING(os9x_menumusic)
	READ_SETTING(os9x_menufx)
	READ_SETTING(os9x_usballowed)
	READ_SETTING(bg_img_num)
	READ_SETTING(os9x_menupadbeep)
	if (fread(&l, 1, 4, f) == 4) os9x_autostart = l;

	fclose(f);
	return 0;
}




////////////////////////////////////////////////////////////////////////////////////////
//check for needed dirs & create them if needed
////////////////////////////////////////////////////////////////////////////////////////
void checkdirs() { 
	int fd;
	char path[256];
	sprintf(path,"%sSAVES",LaunchDir);
	fd = sceIoDopen(path);
	if (fd<0) {
		sceIoMkdir(path, 0777);
	} else {sceIoDclose(fd);}

	sprintf(path,"%sPROFILES",LaunchDir);
	fd = sceIoDopen(path);
	if (fd<0) {
		sceIoMkdir(path, 0777);
	} else {sceIoDclose(fd);}
}


////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
#include "utf8_sjis.c"
void getsysparam(){
	char sVal[256];
	int iVal;

	os9x_timezone=0;
	os9x_daylsavings=0;
	os9x_nickname[0]='\0';

	if (sceUtilityGetSystemParamString(PSP_SYSTEMPARAM_ID_STRING_NICKNAME,sVal,256)!=PSP_SYSTEMPARAM_RETVAL_FAIL){
		//get nick name
		//now convert to sjis
		int i=0,j=0;
		unsigned int utf8;
		while (sVal[i]) {
			utf8=(uint8)sVal[i++];
			utf8=(utf8<<8)|(uint8)sVal[i++];
			utf8=(utf8<<8)|(uint8)sVal[i++];

			for (int k=0;k<sjis_xlate_entries;k++) {
				if (utf8==sjis_xlate[k].utf8) {
					os9x_nickname[j++]=sjis_xlate[k].sjis>>8;
					os9x_nickname[j++]=sjis_xlate[k].sjis&0xFF;
					break;
				}
			}
		}
		os9x_nickname[j]=0;
	}
	if (sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_TIMEZONE,&iVal)!=PSP_SYSTEMPARAM_RETVAL_FAIL){
		//get timezone
		os9x_timezone=iVal;
	}
	if (sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_DAYLIGHTSAVINGS,&iVal)!=PSP_SYSTEMPARAM_RETVAL_FAIL){
		//get timezone
		os9x_daylsavings=iVal;
	}

	if (sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE,&os9x_language)==PSP_SYSTEMPARAM_RETVAL_FAIL)
		//get language
			os9x_language=PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_savesnap() {
	u16 *snes_image;
	if (os9x_softrendering<2)	snes_image=(u16*)(0x44000000+512*272*2*2);
	else snes_image=(u16*)(0x44000000+2*512*272*2+256*240*2+2*256*256*2);

	write_JPEG_file ((char*)S9xGetSaveFilename(".jpg"),75,snes_image,256,os9x_snesheight,256); 
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_loadsnap(char *fname,u16 *snes_image,int *height) {	
	if (read_JPEG_file (fname,snes_image,256,240,256,height)) return 1;	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_remove(const char *ext) {
	FILE *savefile;
	const char *save_filename;
	os9x_externstate_mode=0;

	save_filename=S9xGetSaveFilename (ext);	
	savefile=fopen(save_filename,"rb");
	if (savefile) {
  	fclose(savefile);
		remove(save_filename);
		return 1;
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
char *os9x_shortfilename(char *filename){
	static char fname [256];
	char drive [_MAX_DRIVE + 1];
	char dir [_MAX_DIR + 1];
	char ext [_MAX_EXT + 1];
	_splitpath (filename, drive, dir, fname, ext);
	strcat(fname,".");
	strcat(fname,ext);
	return fname;
}

char *os9x_filename_ext(char *filename){
	char fname [256];
	char drive [_MAX_DRIVE + 1];
	char dir [_MAX_DIR + 1];
	static char ext [_MAX_EXT + 1];
	_splitpath (filename, drive, dir, fname, ext);
	return ext;
}



////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
//#include "snesadvance.c"

int os9x_findhacks(int game_crc32){
	int i=0,j;
	int _crc32;
	char c;
	char str[256];
	unsigned int size_snesadvance;
	unsigned char *snesadvance;
	FILE *f;

	sprintf(str,"%sDATA/snesadvance.dat",LaunchDir);
	f=fopen(str,"rb");
	if (!f) return 0;
	fseek(f,0,SEEK_END);
	size_snesadvance=ftell(f);
	fseek(f,0,SEEK_SET);
	snesadvance=(unsigned char*)malloc(size_snesadvance);
	fread(snesadvance,1,size_snesadvance,f);
	fclose(f);

	for (;;) {
		//get crc32
		j=i;
		while ((i<size_snesadvance)&&(snesadvance[i]!='|')) i++;
		if (i==size_snesadvance) {free(snesadvance);return 0;}
		//we have (snesadvance[i]=='|')
		//convert crc32 to int
		_crc32=0;
		while (j<i) {
			c=snesadvance[j];
			if ((c>='0')&&(c<='9'))	_crc32=(_crc32<<4)|(c-'0');
			else if ((c>='A')&&(c<='F'))	_crc32=(_crc32<<4)|(c-'A'+10);
			else if ((c>='a')&&(c<='f'))	_crc32=(_crc32<<4)|(c-'a'+10);
			j++;
		}
		if (game_crc32==_crc32) {
			//int p=0;
			for (;;) {
				int adr,val;

				i++;
				j=i;
				while ((i<size_snesadvance)&&(snesadvance[i]!=0x0D)&&(snesadvance[i]!=',')) {
					if (snesadvance[i]=='|') j=i+1;
					i++;
				}
				if (i==size_snesadvance) {free(snesadvance);return 0;}
				memcpy(str,&snesadvance[j],i-j);
				str[i-j]=0;
				sscanf(str,"%X=%X",&adr,&val);
				//sprintf(str,"read : %X=%X",adr,val);
				//pgPrintAllBG(32,31-p++,0xFFFF,str);
				
				if ((val==0x42)||((val&0xFF00)==0x4200)) {
					if (val&0xFF00) {
						ROM[adr]=(val>>8)&0xFF;
						ROM[adr+1]=val&0xFF;
					} else ROM[adr]=val;
				}

				if (snesadvance[i]==0x0D) {free(snesadvance);return 1;}
			}

		}
		while ((i<size_snesadvance)&&(snesadvance[i]!=0x0A)) i++;
		if (i==size_snesadvance) {free(snesadvance);return 0;}
		i++; //new line
	}
}

void os9x_beep1() {
	if (!os9x_menupadbeep) return;
	if (!sceAudioGetChannelRestLen(snd_beep1_handle[snd_beep1_current])) {
		sceAudioOutput(snd_beep1_handle[snd_beep1_current], MAXVOLUME/12, (char*)(&(snd_beep1[44])));	
		snd_beep1_current++;
		if (snd_beep1_current==6) snd_beep1_current=0;
	}
}

void os9x_beep2() {
	if (!os9x_menupadbeep) return;
	/*if (!sceAudioGetChannelRestLen(snd_beep2_handle[snd_beep2_current])) {
		sceAudioOutput(snd_beep2_handle[snd_beep2_current], MAXVOLUME/12, (char*)(&(snd_beep2[44])));	
		snd_beep2_current++;
		if (snd_beep2_current==3) snd_beep2_current=0;
	}*/
	if (!sceAudioGetChannelRestLen(snd_beep1_handle[snd_beep1_current])) {
		sceAudioOutput(snd_beep1_handle[snd_beep1_current], MAXVOLUME/12, (char*)(&(snd_beep2[44])));	
		snd_beep1_current++;
		if (snd_beep1_current==6) snd_beep1_current=0;
	}
}

const uint32 crc32Table[256] = {
  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
  0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
  0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
  0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
  0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
  0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
  0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
  0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
  0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
  0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
  0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
  0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
  0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
  0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
  0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
  0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
  0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
  0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
  0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
  0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
  0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
  0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
  0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
  0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
  0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
  0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
  0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
  0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
  0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
  0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
  0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
  0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
  0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
  0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
  0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
  0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
  0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
  0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
  0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
  0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
  0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
  0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
  0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

//CRC32 for char arrays
uint32 caCRC32(uint8 *array, uint32 size, register uint32 crc32) {
  for (register uint32 i = 0; i < size; i++) {
    crc32 = ((crc32 >> 8) & 0x00FFFFFF) ^ crc32Table[(crc32 ^ array[i]) & 0xFF];
  }
  return ~crc32;
}

void net_send_state() {
#ifdef USE_ADHOC
	char *filename;
	uint8 c;
	unsigned int length;

	os9x_save(".znt");
	filename=(char*)S9xGetSaveFilename (".znt");
	//send file
	if (psp_net_send_file(filename)) {
		psp_msg(ADHOC_NETWORKERR_1, MSG_DEFAULT);
		os9x_netplay=0;adhocTerm();os9x_adhoc_active=0;
	}
	//reset netplay related
	os9x_updatepadFrame = 0;os9x_oldframe=0;
	os9x_snespad=0;memset(os9x_netsnespad,0,sizeof(os9x_netsnespad));memset(os9x_netcrc32,0,sizeof(os9x_netcrc32));
	os9x_netsynclost=0;
	os9x_oldsnespad=0;
	os9x_updatepadcpt=0;os9x_padfirstcall=1;
	//load received state
	if (!os9x_load(".znt")) {
		psp_msg(ADHOC_CANNOTFIND, MSG_DEFAULT);
	}
	remove(filename);
	//send a sync packet
	length=1;
	adhocRecvSendAck(&c,&length);
#endif
}

void net_flush_net(int to_send) {
#ifdef USE_ADHOC
	unsigned int length;
	pkt_send[0]=to_send;
	adhocSend(pkt_send, NET_PKT_LEN);
	psp_msg(ADHOC_FLUSHING, MSG_DEFAULT);
	do {
		length=NET_PKT_LEN;
	} while (adhocRecvBlocked(pkt_recv, &length,RECV_MAX_RETRY)>0);
#endif
}

int net_waitpause_state(int show_menu){
	unsigned int file_size,length;
	unsigned int crc32,rlen;
	int ret;
	char *filename;
	uint8 c;
	FILE *f;

	before_pause();
	if (show_menu) {
		if (!os9x_lowbat) {
			if (CPUPack.CPU.SRAMModified) {
				Memory.SaveSRAM( (char*)S9xGetSaveFilename(".SRM") );
				CPUPack.CPU.SRAMModified=0;
			}
		}
		//initUSBdrivers();
		root_menu();
#ifndef FW3X
		endUSBdrivers();
#endif
		os9x_specialaction=0;
		if (!os9x_lowbat) {
			if (menu_modified) {
				save_rom_settings(Memory.ROMCRC32,Memory.ROMName);
				save_settings();
			}
		}
		if ((in_emu==1)&&os9x_netplay&&os9x_getnewfile) {
			psp_msg(ADHOC_CLOSING, MSG_DEFAULT);
#ifdef USE_ADHOC
			os9x_netplay=0;adhocTerm();os9x_adhoc_active=0;
#else
			os9x_netplay=0;os9x_adhoc_active=0;
#endif
		}
	}

#ifdef USE_ADHOC
	if (os9x_netplay) {
		psp_msg(ADHOC_WAITING_OTHER, MSG_DEFAULT);

		//filename																																			
		filename=(char*)S9xGetSaveFilename (".znt");
		if ((ret=psp_net_recv_file(filename))<0) {
			psp_msg(ADHOC_NETWORKERR_1, MSG_DEFAULT);
			os9x_netplay=0;adhocTerm();os9x_adhoc_active=0;
			after_pause();
			return ret;
		}

		//reset netplay related
		os9x_updatepadFrame = 0;os9x_oldframe=0;
		os9x_snespad=0;memset(os9x_netsnespad,0,sizeof(os9x_netsnespad));memset(os9x_netcrc32,0,sizeof(os9x_netcrc32));
		os9x_netsynclost=0;
		os9x_oldsnespad=0;
		os9x_updatepadcpt=0;os9x_padfirstcall=1;

		//load received state
		if (!os9x_load(".znt")) {
			psp_msg(ADHOC_CANNOTFIND, MSG_DEFAULT);
		}
		remove(filename);

		//sync stuff
		c=0;
		adhocSendRecvAck(&c,1);
	}
#endif
	after_pause();
	return 0;
}

void net_send_settings() {
#ifdef USE_ADHOC
	unsigned int length;
	FILE *f;
	char filename[256];
	uint8 c;
	char buffer[256];



	sprintf(filename,"%stmp.ini",LaunchDir);
	memset(buffer,0,256);
	save_buffer_settings(buffer);
	f = fopen(filename,"wb");
	fwrite(buffer,1,256,f);
	fclose(f);

	if (psp_net_send_file(filename)) {
		psp_msg(ADHOC_NETWORKERR_1, MSG_DEFAULT);
		os9x_netplay=0;adhocTerm();os9x_adhoc_active=0;
	} else {
		//reset netplay related
		os9x_updatepadFrame = 0;os9x_oldframe=0;
		os9x_snespad=0;memset(os9x_netsnespad,0,sizeof(os9x_netsnespad));memset(os9x_netcrc32,0,sizeof(os9x_netcrc32));
		os9x_netsynclost=0;
		os9x_oldsnespad=0;
		os9x_updatepadcpt=0;os9x_padfirstcall=1;
		//send a sync packet
		length=1;
		adhocRecvSendAck(&c,&length);
	}
	remove(filename);
#endif
}
void net_receive_settings() {
#ifdef USE_ADHOC
	unsigned int file_size,length;
	unsigned int crc32,rlen;
	int ret;
	char filename[256];
	uint8 c;
	char buffer[256];
	FILE *f;

	sprintf(filename,"%stmp.ini",LaunchDir);

	psp_msg(ADHOC_WAITING_OTHER, MSG_DEFAULT);

	//filename																																									
	if ((ret=psp_net_recv_file(filename))<0) {
		psp_msg(ADHOC_NETWORKERR_1, MSG_DEFAULT);
		os9x_netplay=0;adhocTerm();os9x_adhoc_active=0;
	} else {
		//reset netplay related
		os9x_updatepadFrame = 0;os9x_oldframe=0;
		os9x_snespad=0;memset(os9x_netsnespad,0,sizeof(os9x_netsnespad));memset(os9x_netcrc32,0,sizeof(os9x_netcrc32));
		os9x_netsynclost=0;
		os9x_oldsnespad=0;
		os9x_updatepadcpt=0;os9x_padfirstcall=1;

		//load received state
		memset(buffer,0,256);
		f=fopen(filename,"rb");
		fread(buffer,1,256,f);
		fclose(f);
		load_buffer_settings(buffer);													
		//sync stuff
		c=0;
		adhocSendRecvAck(&c,1);
	}
	remove(filename);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void check_battery() {
	if (scePowerIsBatteryExist()) {
		int oldvalue=os9x_lowbat;
		os9x_lowbat=scePowerIsLowBattery();
		if ((os9x_lowbat)&&(!oldvalue)) {
			psp_msg(BAT_ISLOW, MSG_DEFAULT);
			pgFillAllvram(0);
			//reset timer for synchro stuff
		} else if ((!os9x_lowbat)&&(oldvalue)) {
			pgPrintAllBG(0,33,0xffff,"                               ");
		}
	}
}

void os9x_savecheats() {
	S9xSaveCheatFile( (char*)S9xGetSaveFilename( ".cht" ) );
}

//helper function to make things easier
int LoadStartModule(char *path) {
	u32 loadResult;
	u32 startResult;
	int status;
	loadResult = sceKernelLoadModule(path, 0, NULL);
	if (loadResult & 0x80000000) return -1;
	else startResult = sceKernelStartModule(loadResult, 0, NULL, &status, NULL);
	if (loadResult != startResult) return -2;
	return 0;
}

#ifndef FW3X
void loadUSBdrivers(void) {
	//start necessary drivers
	LoadStartModule("flash0:/kd/semawm.prx");
	LoadStartModule("flash0:/kd/usbstor.prx");
	LoadStartModule("flash0:/kd/usbstormgr.prx");
	LoadStartModule("flash0:/kd/usbstorms.prx");
	LoadStartModule("flash0:/kd/usbstorboot.prx");
}

int initUSBdrivers(void) {
	//connect USB
	uint32 state = sceUsbGetState();
	if (state & PSP_USB_ACTIVATED) return 0;
	int retVal = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
	if (retVal != 0) {
		sprintf(str_tmp,psp_msg_string(ERR_USB_STARTING_USBBUS), retVal);
			msgBoxLines(str_tmp,60);
			return -1;
	}
	retVal = sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0);
	if (retVal != 0) {
		sprintf(str_tmp,psp_msg_string(ERR_USB_STARTING_USBMASS),retVal);
		msgBoxLines(str_tmp,60);
		return -2;
	}
	retVal = sceUsbstorBootSetCapacity(0x800000);
	if (retVal != 0) {
		sprintf(str_tmp,psp_msg_string(ERR_USB_SETTING_CAPACITY),retVal);
		msgBoxLines(str_tmp,60);
		return -3;
	}
	retVal = sceUsbActivate(0x1C8);
	if (retVal) return -4;

  return 0;
}

int endUSBdrivers(void) {
	uint32 state = sceUsbGetState();
	if (state & PSP_USB_ACTIVATED) {
		sceUsbDeactivate(0x1C8);
		sceUsbStop(PSP_USBSTOR_DRIVERNAME,0,NULL);
		sceUsbStop(PSP_USBBUS_DRIVERNAME,0,NULL);
	}
}
#endif //FW3X
