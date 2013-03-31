////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_getstate(char *ext,char *str_time) {
	STREAM file;
	const char *save_filename;		
	SceIoStat stat;
	ScePspDateTime *tfile;
	
	save_filename=S9xGetSaveFilename (ext);		
	if (sceIoGetstat(save_filename,&stat)>=0) {
			tfile=&(stat.st_mtime);	
			sprintf(str_time,"%4d/%02d/%02d %02d:%02d:%02d",tfile->year,tfile->month,tfile->day,
			tfile->hour,tfile->minute,tfile->second);
			S9xOpenSnapshotFile(save_filename,1,&file);
			S9xCloseSnapshotFile(file);								
			return 1;
	}				
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_savesram() {
	Memory.SaveSRAM( (char*)S9xGetSaveFilename(".SRM") );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_save(const char *ext)
{
	const char *save_filename;
	
	os9x_externstate_mode=0;
	save_filename=S9xGetSaveFilename (ext);	
	//msgBoxLines((char*)save_filename,10);	
	S9xFreezeGame(save_filename);	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_S9Xsave(const char *ext)
{
	const char *save_filename;
	
	os9x_externstate_mode=1;
	save_filename=S9xGetSaveFilename (ext);	
	//msgBoxLines((char*)save_filename,10);	
	S9xFreezeGame(save_filename);	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_ZSsave(const char *ext)
{
	const char *save_filename;
	
	os9x_externstate_mode=1;
	save_filename=S9xGetSaveFilename (ext);	
	//msgBoxLines((char*)save_filename,10);	
	S9xFreezeGame(save_filename);	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_load(const char *ext) {	
	FILE *savefile;
	const char *save_filename;
	os9x_externstate_mode=0;
	
	save_filename=S9xGetSaveFilename (ext);	
	savefile=fopen(save_filename,"rb");
  if (savefile) {
  	fclose(savefile);
		S9xUnfreezeGame(save_filename);
//		S9xInitUpdate();
		S9xReschedule ();
		return 1;
	} 
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_loadfname(const char *fname) {	
	FILE *savefile;
	char *ext;
	ext=strrchr(fname,'.');
	if (strlen(ext)==4) {
		if (!strcasecmp(ext,".srm")) {
			psp_msg(FOUND_SRAM, MSG_DEFAULT);
			Memory.LoadSRAM( (char*)fname );
			return 1;
		}
	}

	os9x_externstate_mode=1;
	savefile=fopen(fname,"rb");
  if (savefile) {
  	fclose(savefile);
		S9xUnfreezeGame(fname);
//		S9xInitUpdate();
		S9xReschedule ();
		return 1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_loadzsnes(char *ext) {
	FILE *savefile;
	const char *save_filename;
	
	os9x_externstate_mode=1;
	save_filename=S9xGetSaveFilename (ext);
	savefile=fopen(save_filename,"rb");
  if (savefile) {
 		fclose(savefile);
		S9xUnfreezeGame(save_filename);
//		S9xInitUpdate();
		S9xReschedule ();
		return 1;
	}
	return 0;
}
