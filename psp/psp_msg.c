#include "psp.h"

extern int os9x_language;

char *s9xTYL_lang[LANGUAGE_NUMBER]={
	"english"
};

#define MSG_TOTAL_ENTRIES 143
typedef struct {
	int msg_id;
	char *msg;
	int len;  // MSG_DEFAULT : to use default
						// >=0 : wait for len frames, 
					  //  -1 : wait for a confirmation before closing dialog
					  //  -2 : yes/no dialog
} t_err_entry;

t_err_entry s9xTYL_msg_en[MSG_TOTAL_ENTRIES]= {
//english
		//error
		{ERR_OUT_OF_MEM,"Out of memory",60},
		{ERR_READ_MEMSTICK,"Error reading memstick.\nPlease verify it (using a PC card reader), it may be corrupted.",60},
		//confirm
  	{ASK_EXIT,"Exit Snes9XTYL ?",-2},
  	{ASK_DELETE,"Delete file ?",-2},
  	//info
  	{INFO_USB_ON,"USB activated",30},
  	{INFO_USB_OFF,"USB disabled",30},
  	{INFO_DELETING,"Deleting...",0},
  	{INFO_EXITING,"Exiting, please wait...",0},
  	//filer
  	{FILER_STATUS_CANEXIT1,SJIS_CIRCLE " RUN " SJIS_SQUARE " RUN default settings  " SJIS_STAR "  " SJIS_CROSS " TO GAME  "  SJIS_STAR "  " SJIS_TRIANGLE " Par. dir.  "  SJIS_STAR "  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT ", to browse",0},
  	{FILER_STATUS_NOEXIT1,SJIS_CIRCLE " RUN " SJIS_SQUARE " RUN default settings  "  SJIS_STAR "  " SJIS_TRIANGLE " Parent dir.          " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT ", to browse list",0},
  	{FILER_STATUS_CANEXIT2,SJIS_CIRCLE " RUN " SJIS_CROSS "BACK TO GAME " SJIS_TRIANGLE " Parent dir.  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT ", to browse list",0},
  	{FILER_STATUS_NOEXIT2,SJIS_CIRCLE " RUN " SJIS_TRIANGLE " Parent dir.                  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT ", to browse list",0},
  	
  	{FILER_HELP_WINDOW1,"Press START to switch between",0},
  	{FILER_HELP_WINDOW2,"NORMAL and NETPLAY mode.",0},
#ifdef FW3X
  	{FILER_HELP_WINDOW3," ",0},
#ifdef HOME_HOOK
  	{FILER_HELP_WINDOW4,"Press HOME to exit.",0},
#else
    {FILER_HELP_WINDOW4,"Press L+R to exit.",0},
#endif
#else
#ifdef HOME_HOOK
    {FILER_HELP_WINDOW3,"Press HOME to exit.",0},
#else
    {FILER_HELP_WINDOW3,"Press L+R to exit.",0},
#endif
  	{FILER_HELP_WINDOW4,"Press R to switch USB on/off.",0},
#endif


  	{FILER_HELP_WINDOW5,"Press SELECT to delete file.",0},
  	
  	{FILE_IPS_PATCHSUCCESS,"IPS Patch applied successfully",0},
  	
  	{MENU_STATUS_GENERIC_MSG1,"Press " SJIS_CROSS " to return",0},
  	{MENU_STATUS_GENERIC_NEEDRELOAD,"Changes may require a RELOAD to take effect",0},
  	{MENU_STATUS_GENERIC_NEEDRESET,"Changes may require a RESET to take effect",0},
  	
  	{MENU_SOUND_MODE_NOTEMUL,"not emulated",0},
		{MENU_SOUND_MODE_EMULOFF,"emulated, no output",0},
  	{MENU_SOUND_MODE_EMULON,"emulated, output",0},  	
  	
  	{MENU_STATE_CHOOSEDEL,"Choose a slot to DELETE",0},
  	{MENU_STATE_CHOOSELOAD,"Choose a slot to LOAD",0},
  	{MENU_STATE_CHOOSESAVE,"Choose a slot to SAVE",0},
  	{MENU_STATE_FREESLOT,"FREE",0},  	
  	{MENU_STATE_SCANNING,"Scanning used slots...",0},  	
  	
  	  	
  	{ERR_INIT_OSK,"cannot init OSK",0},
  	{ERR_ADD_CODE,"Cannot add cheat code, too many already enterred!",0},
  	
  	{MENU_CHEATS_ENTERGF,"Enter a GoldFinger code",0},
  	{MENU_CHEATS_ENTERPAR,"Enter a Pro Action Replay code",0},
  	{MENU_CHEATS_ENTERGG,"Enter a Game Genie code",0},
  	{MENU_CHEATS_ENTERRAW,"Enter a RAW code\nFormat is ADDRESS - NEW VALUE",0},  	  	
  	
  	{MENU_STATE_CONFIRMLOAD,"Current game progress will be lost.\nContinue ?",0},
		{MENU_STATE_CONFIRMDELETE,"Delete current state ?",0},
		{MENU_STATE_CONFIRMSAVE,"Overwrite current savestate ?",0},
		{MENU_STATE_ISSAVING,"Saving state...",0},
		{MENU_STATE_ISLOADING,"Loading state...",0},
		{MENU_STATE_ISIMPORTING,"Importing state...",0},
		{MENU_STATE_ISEXPORTINGS9X,"Exporting SNES9X state...",0},		
		{MENU_STATE_NOSTATE,"No state available...",0},
		{MENU_GAME_SAVINGDEFAULTSETTINGS,"Saving current settings as default profile",0},
		{MENU_GAME_CONFIRMRESET,"Reset Snes ?",0},
		{MENU_MISC_SAVINGJPEG,"Saving JPEG snapshot...",0},
  	
  	
  	// GAME
  	{MENU_ICONS_GAME,"GAME",0},
  	{MENU_ICONS_GAME_RESET,"Reset SNES",0},  	
  	{MENU_ICONS_GAME_NEW,"Load new game",0},
		{MENU_ICONS_GAME_DEFAULTSETTINGS,"Make Settings DEFAULT",0},
		{MENU_ICONS_GAME_DEFAULTSETTINGS_HELP,"Make current settings the default\nfor all roms when no specific\none exists",0},		
		{MENU_ICONS_GAME_EXIT,"Exit Snes9xTYL",0},
		// LOAD/SAVE
		{MENU_ICONS_LOADSAVE,"LOAD/SAVE",0},
		{MENU_ICONS_LOADSAVE_LOADSTATE,"Load state",0},
		{MENU_ICONS_LOADSAVE_LOADSTATE_HELP,"Load a previously saved state.\nSavestate is like a snapshot\nof the snes state, everything\nis saved.",0},		
		{MENU_ICONS_LOADSAVE_SAVESTATE,"Save state",0},
		{MENU_ICONS_LOADSAVE_SAVESTATE_HELP,"Save state",0},		
		{MENU_ICONS_LOADSAVE_DELETESTATE,"Delete state",0},
		{MENU_ICONS_LOADSAVE_DELETESTATE_HELP,"Delete state",0},		
		{MENU_ICONS_LOADSAVE_IMPORTSTATE,"Import state",0},
		{MENU_ICONS_LOADSAVE_IMPORTSTATE_HELP,"Import a state from a previous\nversion or from uosnes9x.",0},		
		
		{MENU_ICONS_LOADSAVE_EXPORTS9XSTATE,"Export SNES9X state",0},
		{MENU_ICONS_LOADSAVE_EXPORTS9XSTATE_HELP,"Export a state using SNES9X format.",0},		
		
		{MENU_ICONS_LOADSAVE_AUTOSAVETIMER,"Autosavestate timer",0},
		{MENU_ICONS_LOADSAVE_AUTOSAVETIMER_HELP,"Allows to have an auto\nsavestate performed regularly.",0},		
		{MENU_ICONS_LOADSAVE_AUTOSRAM,"Autosave updated SRAM",0},
		{MENU_ICONS_LOADSAVE_AUTOSRAM_HELP,"Allows to have SRAM saved\nafter each change.",0},		
		// CONTROLS	
		{MENU_ICONS_CONTROLS,"CONTROLS",0},		
		{MENU_ICONS_CONTROLS_REDEFINE,"Redefine",0},
		{MENU_ICONS_CONTROLS_REDEFINE_HELP,"Redefine buttons mapping.",0},		
		{MENU_ICONS_CONTROLS_INPUT,"Emulated input",0},	
		{MENU_ICONS_CONTROLS_INPUT_HELP,"Choose current active\nsnes controller.",0},			
		// VIDEO OPTIONS
		{MENU_ICONS_VIDEO,"VIDEO",0},
		{MENU_ICONS_VIDEO_MODE,"Video mode",0},
		{MENU_ICONS_VIDEO_MODE_HELP,"Change the aspect ratio\n4:3 is adviced",0},		
		{MENU_ICONS_VIDEO_ENGINE,"Engine",0},	
		{MENU_ICONS_VIDEO_ENGINE_HELP,"Hardware accelerated mode\nis not always the fastest",0},			
		{MENU_ICONS_VIDEO_SLIMITONOFF,"Speed limit switch",0},
		{MENU_ICONS_VIDEO_SLIMITONOFF_HELP,"Turn on to limit the speed\nof the game.",0},		
		{MENU_ICONS_VIDEO_SLIMITVALUE,"Speed limit fps",0},
		{MENU_ICONS_VIDEO_SLIMITVALUE_HELP,"Only used if speed limit\nhas been turned on.",0},		
		{MENU_ICONS_VIDEO_FSKIP,"Frameskip",0},
		{MENU_ICONS_VIDEO_FSKIP_HELP,"Choose a fixed value if you\nhave some sprites\ndisappearing.",0},		
		{MENU_ICONS_VIDEO_SMOOTHING,"Smoothing",0},		
		{MENU_ICONS_VIDEO_SMOOTHING_HELP,"Should be on since it costs\nnothing.",0},				
		{MENU_ICONS_VIDEO_VSYNC,"VSync",0},
		{MENU_ICONS_VIDEO_VSYNC_HELP,"This costs lots of fps\nand should be turned off.",0},		
		{MENU_ICONS_VIDEO_PALASNTSC,"Render PAL as NTSC",0},
		{MENU_ICONS_VIDEO_PALASNTSC_HELP,"Force PAL games to be emulated\nlike NTSC ones : 224 lines\ninstead of 240.",0},		
		{MENU_ICONS_VIDEO_GAMMA,"Gamma correction",0},
		{MENU_ICONS_VIDEO_GAMMA_HELP,"Allows brighter rendering",0},		
		{MENU_ICONS_VIDEO_SCRCALIB,"Screen calibration",0},	
		{MENU_ICONS_VIDEO_SCRCALIB_HELP,"Help to get rid of the nasty\nblack borders.",0},			
		{MENU_ICONS_VIDEO_SHOWFPS,"Show FPS",0},
		{MENU_ICONS_VIDEO_SHOWFPS_HELP,"First line is emulated frames\nSecond one is real fps.",0},		
		// SOUND OPTIONS
		{MENU_ICONS_SOUND,"SOUND",0},
		{MENU_ICONS_SOUND_MODE,"Sound mode",0},
		{MENU_ICONS_SOUND_MODE_HELP,"not emulated is the fastest\nbut some games won't run.",0},		
		{MENU_ICONS_SOUND_FREQ,"Output frequency",0},
		{MENU_ICONS_SOUND_FREQ_HELP,"The lower the faster\nThe higher the best sound quality",0},		
		{MENU_ICONS_SOUND_APURATIO,"APU Cycles ratio",0},	
		{MENU_ICONS_SOUND_APURATIO_HELP,"For advanced users knowing\nwhat they do.",0},			
		// MISC OPTIONS
		{MENU_ICONS_MISC,"MISC",0},	
		{MENU_ICONS_MISC_FILEVIEWER,"View file",0},	
		{MENU_ICONS_MISC_FILEVIEWER_HELP,"Minimalist file viewer",0},			
		{MENU_ICONS_MISC_PSPCLOCK,"PSP Clockspeed",0},	
		{MENU_ICONS_MISC_PSPCLOCK_HELP,"value>222Mhz is NOT overclocking\nit's safe but battery runs out\nfaster",0},			
		{MENU_ICONS_MISC_HACKDEBUG,"Hack/debug menu",0},
		{MENU_ICONS_MISC_HACKDEBUG_HELP,"For advanced users.\nHelp to tune the rendering and\ngain some speed.",0},		
		{MENU_ICONS_MISC_SNAPSHOT,"Snapshot",0},
		{MENU_ICONS_MISC_SNAPSHOT_HELP,"Take a snapshot of the current\ngame screen.\nUsed in file browser.",0},		
		{MENU_ICONS_MISC_RANDBG,"Randomize BG",0},
		{MENU_ICONS_MISC_RANDBG_HELP,"Randomize BG from the DATA/logo.zip\nfile.",0},		
		{MENU_ICONS_MISC_BGMUSIC,"Menu background music",0},
		{MENU_ICONS_MISC_BGMUSIC_HELP,"Menu background music requires\nmake PSP running at 300Mhz\nMusic are from DATA/music.zip file.\nSPC format only.",0},		
		{MENU_ICONS_MISC_BGFX,"Menu background fx",0},	
		{MENU_ICONS_MISC_BGFX_HELP,"Nice FX from PS2DEV spline sample.",0},			
		{MENU_ICONS_MISC_PADBEEP,"Pad beep",0},	
		{MENU_ICONS_MISC_PADBEEP_HELP,"The beep sounds when browsing\nmenu and files.",0},			
		// CHEATS
		{MENU_ICONS_CHEATS,"CHEATS",0},
		{MENU_ICONS_CHEATS_ADDRAW,"Add a RAW code",0},
		{MENU_ICONS_CHEATS_ADDRAW_HELP,"Add a RAW code",0},		
		{MENU_ICONS_CHEATS_ADDGG,"Add a Game Genie code",0},
		{MENU_ICONS_CHEATS_ADDGG_HELP,"Add a Game Genie code",0},		
		{MENU_ICONS_CHEATS_ADDPAR,"Add a Pro Action Replay code",0},
		{MENU_ICONS_CHEATS_ADDPAR_HELP,"Add a Pro Action Replay code",0},		
		{MENU_ICONS_CHEATS_ADDGF,"Add a GoldFinger code",0},
		{MENU_ICONS_CHEATS_ADDGF_HELP,"Add a GoldFinger code",0},		
		{MENU_ICONS_CHEATS_DISABLE,"Disable code",0},
		{MENU_ICONS_CHEATS_DISABLE_HELP,"Disable code",0},		
		{MENU_ICONS_CHEATS_DISABLEALL,"Disable all codes",0},
		{MENU_ICONS_CHEATS_DISABLEALL_HELP,"Disable all codes",0},		
		{MENU_ICONS_CHEATS_ENABLE,"Enable code",0},
		{MENU_ICONS_CHEATS_ENABLE_HELP,"Enable code",0},		
		{MENU_ICONS_CHEATS_ENABLEALL,"Enable all codes",0},
		{MENU_ICONS_CHEATS_ENABLEALL_HELP,"Enable all codes",0},		
		{MENU_ICONS_CHEATS_REMOVE,"Remove code",0},
		{MENU_ICONS_CHEATS_REMOVE_HELP,"Remove code",0},		
		{MENU_ICONS_CHEATS_REMOVEALL,"Remove all codes",0},
		{MENU_ICONS_CHEATS_REMOVEALL_HELP,"Remove all codes",0},		
		// ABOUT
		{MENU_ICONS_ABOUT,"ABOUT",0},
		{MENU_ICONS_ABOUT_CREDITS,"Credits",0},
		{MENU_ICONS_ABOUT_VERSION,"Version infos",0},
};

t_err_entry s9xTYL_msg_ja[MSG_TOTAL_ENTRIES]= {
//japanese
		//error
		{ERR_OUT_OF_MEM,"Out of memory",60},
		{ERR_READ_MEMSTICK,"Error reading memstick.\nPlease verify it (using a PC card reader), it may be corrupted.",60},
		//confirm
  	{ASK_EXIT,"Exit Snes9XTYL ?",-2},
  	{ASK_DELETE,"Delete file ?",-2},
  	//info
  	{INFO_USB_ON,"USB activated",30},
  	{INFO_USB_OFF,"USB disabled",30},
  	{INFO_DELETING,"Deleting...",0},
  	{INFO_EXITING,"Exiting, please wait...",0},
  	//filer
  	{FILER_STATUS_CANEXIT1,SJIS_CIRCLE " RUN " SJIS_SQUARE " RUN default settings  " SJIS_STAR "  " SJIS_CROSS " TO GAME  "  SJIS_STAR "  " SJIS_TRIANGLE " Par. dir.  "  SJIS_STAR "  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT ", to browse",0},
  	{FILER_STATUS_NOEXIT1,SJIS_CIRCLE " RUN " SJIS_SQUARE " RUN default settings  "  SJIS_STAR "  " SJIS_TRIANGLE " Parent dir.          " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT ", to browse list",0},
  	{FILER_STATUS_CANEXIT2,SJIS_CIRCLE " RUN " SJIS_CROSS "BACK TO GAME " SJIS_TRIANGLE " Parent dir.  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT ", to browse list",0},
  	{FILER_STATUS_NOEXIT2,SJIS_CIRCLE " RUN " SJIS_TRIANGLE " Parent dir.                  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT ", to browse list",0},
  	
  	{FILER_HELP_WINDOW1,"Press START to switch between",0},
  	{FILER_HELP_WINDOW2,"NORMAL and NETPLAY mode.",0},
#ifdef FW3X
  	{FILER_HELP_WINDOW3," ",0},
#ifdef HOME_HOOK
  	{FILER_HELP_WINDOW4,"Press HOME to exit.",0},
#else
    {FILER_HELP_WINDOW4,"Press L+R to exit.",0},
#endif
#else
#ifdef HOME_HOOK
    {FILER_HELP_WINDOW3,"Press HOME to exit.",0},
#else
    {FILER_HELP_WINDOW3,"Press L+R to exit.",0},
#endif
  	{FILER_HELP_WINDOW4,"Press R to switch USB on/off.",0},
#endif


  	{FILER_HELP_WINDOW5,"Press SELECT to delete file.",0},
  	
  	{FILE_IPS_PATCHSUCCESS,"IPS Patch applied successfully",0},
  	
  	{MENU_STATUS_GENERIC_MSG1,"Press " SJIS_CROSS " to return",0},
  	{MENU_STATUS_GENERIC_NEEDRELOAD,"Changes may require a RELOAD to take effect",0},
  	{MENU_STATUS_GENERIC_NEEDRESET,"Changes may require a RESET to take effect",0},
  	
  	{MENU_SOUND_MODE_NOTEMUL,"not emulated",0},
		{MENU_SOUND_MODE_EMULOFF,"emulated, no output",0},
  	{MENU_SOUND_MODE_EMULON,"emulated, output",0},  	
  	
  	{MENU_STATE_CHOOSEDEL,"Choose a slot to DELETE",0},
  	{MENU_STATE_CHOOSELOAD,"Choose a slot to LOAD",0},
  	{MENU_STATE_CHOOSESAVE,"Choose a slot to SAVE",0},
  	{MENU_STATE_FREESLOT,"FREE",0},  	
  	{MENU_STATE_SCANNING,"Scanning used slots...",0},  	
  	
  	  	
  	{ERR_INIT_OSK,"cannot init OSK",0},
  	{ERR_ADD_CODE,"Cannot add cheat code, too many already enterred!",0},
  	
  	{MENU_CHEATS_ENTERGF,"Enter a GoldFinger code",0},
  	{MENU_CHEATS_ENTERPAR,"Enter a Pro Action Replay code",0},
  	{MENU_CHEATS_ENTERGG,"Enter a Game Genie code",0},
  	{MENU_CHEATS_ENTERRAW,"Enter a RAW code\nFormat is ADDRESS - NEW VALUE",0},  	  	
  	
  	{MENU_STATE_CONFIRMLOAD,"Current game progress will be lost.\nContinue ?",0},
		{MENU_STATE_CONFIRMDELETE,"Delete current state ?",0},
		{MENU_STATE_CONFIRMSAVE,"Overwrite current savestate ?",0},
		{MENU_STATE_ISSAVING,"Saving state...",0},
		{MENU_STATE_ISLOADING,"Loading state...",0},
		{MENU_STATE_ISIMPORTING,"Importing state...",0},
		{MENU_STATE_ISEXPORTINGS9X,"Exporting SNES9X state...",0},		
		{MENU_STATE_NOSTATE,"No state available...",0},
		{MENU_GAME_SAVINGDEFAULTSETTINGS,"Saving current settings as default profile",0},
		{MENU_GAME_CONFIRMRESET,"Reset Snes ?",0},
		{MENU_MISC_SAVINGJPEG,"Saving JPEG snapshot...",0},
  	
  	
  	// GAME
  	{MENU_ICONS_GAME,"GAME",0},
  	{MENU_ICONS_GAME_RESET,"Reset SNES",0},  	
  	{MENU_ICONS_GAME_NEW,"Load new game",0},
		{MENU_ICONS_GAME_DEFAULTSETTINGS,"Make Settings DEFAULT",0},
		{MENU_ICONS_GAME_DEFAULTSETTINGS_HELP,"Make current settings the default\nfor all roms when no specific\none exists",0},		
		{MENU_ICONS_GAME_EXIT,"Exit Snes9xTYL",0},
		// LOAD/SAVE
		{MENU_ICONS_LOADSAVE,"LOAD/SAVE",0},
		{MENU_ICONS_LOADSAVE_LOADSTATE,"Load state",0},
		{MENU_ICONS_LOADSAVE_LOADSTATE_HELP,"Load a previously saved state.\nSavestate is like a snapshot\nof the snes state, everything\nis saved.",0},		
		{MENU_ICONS_LOADSAVE_SAVESTATE,"Save state",0},
		{MENU_ICONS_LOADSAVE_SAVESTATE_HELP,"Save state",0},		
		{MENU_ICONS_LOADSAVE_DELETESTATE,"Delete state",0},
		{MENU_ICONS_LOADSAVE_DELETESTATE_HELP,"Delete state",0},		
		{MENU_ICONS_LOADSAVE_IMPORTSTATE,"Import state",0},
		{MENU_ICONS_LOADSAVE_IMPORTSTATE_HELP,"Import a state from a previous\nversion or from uosnes9x.",0},		
		
		{MENU_ICONS_LOADSAVE_EXPORTS9XSTATE,"Export SNES9X state",0},
		{MENU_ICONS_LOADSAVE_EXPORTS9XSTATE_HELP,"Export a state using SNES9X format.",0},		
		
		{MENU_ICONS_LOADSAVE_AUTOSAVETIMER,"Autosavestate timer",0},
		{MENU_ICONS_LOADSAVE_AUTOSAVETIMER_HELP,"Allows to have an auto\nsavestate performed regularly.",0},		
		{MENU_ICONS_LOADSAVE_AUTOSRAM,"Autosave updated SRAM",0},
		{MENU_ICONS_LOADSAVE_AUTOSRAM_HELP,"Allows to have SRAM saved\nafter each change.",0},		
		// CONTROLS	
		{MENU_ICONS_CONTROLS,"CONTROLS",0},		
		{MENU_ICONS_CONTROLS_REDEFINE,"Redefine",0},
		{MENU_ICONS_CONTROLS_REDEFINE_HELP,"Redefine buttons mapping.",0},		
		{MENU_ICONS_CONTROLS_INPUT,"Emulated input",0},	
		{MENU_ICONS_CONTROLS_INPUT_HELP,"Choose current active\nsnes controller.",0},			
		// VIDEO OPTIONS
		{MENU_ICONS_VIDEO,"VIDEO",0},
		{MENU_ICONS_VIDEO_MODE,"Video mode",0},
		{MENU_ICONS_VIDEO_MODE_HELP,"Change the aspect ratio\n4:3 is adviced",0},		
		{MENU_ICONS_VIDEO_ENGINE,"Engine",0},	
		{MENU_ICONS_VIDEO_ENGINE_HELP,"Hardware accelerated mode\nis not always the fastest",0},			
		{MENU_ICONS_VIDEO_SLIMITONOFF,"Speed limit switch",0},
		{MENU_ICONS_VIDEO_SLIMITONOFF_HELP,"Turn on to limit the speed\nof the game.",0},		
		{MENU_ICONS_VIDEO_SLIMITVALUE,"Speed limit fps",0},
		{MENU_ICONS_VIDEO_SLIMITVALUE_HELP,"Only used if speed limit\nhas been turned on.",0},		
		{MENU_ICONS_VIDEO_FSKIP,"Frameskip",0},
		{MENU_ICONS_VIDEO_FSKIP_HELP,"Choose a fixed value if you\nhave some sprites\ndisappearing.",0},		
		{MENU_ICONS_VIDEO_SMOOTHING,"Smoothing",0},		
		{MENU_ICONS_VIDEO_SMOOTHING_HELP,"Should be on since it costs\nnothing.",0},				
		{MENU_ICONS_VIDEO_VSYNC,"VSync",0},
		{MENU_ICONS_VIDEO_VSYNC_HELP,"This costs lots of fps\nand should be turned off.",0},		
		{MENU_ICONS_VIDEO_PALASNTSC,"Render PAL as NTSC",0},
		{MENU_ICONS_VIDEO_PALASNTSC_HELP,"Force PAL games to be emulated\nlike NTSC ones : 224 lines\ninstead of 240.",0},		
		{MENU_ICONS_VIDEO_GAMMA,"Gamma correction",0},
		{MENU_ICONS_VIDEO_GAMMA_HELP,"Allows brighter rendering",0},		
		{MENU_ICONS_VIDEO_SCRCALIB,"Screen calibration",0},	
		{MENU_ICONS_VIDEO_SCRCALIB_HELP,"Help to get rid of the nasty\nblack borders.",0},			
		{MENU_ICONS_VIDEO_SHOWFPS,"Show FPS",0},
		{MENU_ICONS_VIDEO_SHOWFPS_HELP,"First line is emulated frames\nSecond one is real fps.",0},		
		// SOUND OPTIONS
		{MENU_ICONS_SOUND,"SOUND",0},
		{MENU_ICONS_SOUND_MODE,"Sound mode",0},
		{MENU_ICONS_SOUND_MODE_HELP,"not emulated is the fastest\nbut some games won't run.",0},		
		{MENU_ICONS_SOUND_FREQ,"Output frequency",0},
		{MENU_ICONS_SOUND_FREQ_HELP,"The lower the faster\nThe higher the best sound quality",0},		
		{MENU_ICONS_SOUND_APURATIO,"APU Cycles ratio",0},	
		{MENU_ICONS_SOUND_APURATIO_HELP,"For advanced users knowing\nwhat they do.",0},			
		// MISC OPTIONS
		{MENU_ICONS_MISC,"MISC",0},	
		{MENU_ICONS_MISC_FILEVIEWER,"View file",0},	
		{MENU_ICONS_MISC_FILEVIEWER_HELP,"Minimalist file viewer",0},			
		{MENU_ICONS_MISC_PSPCLOCK,"PSP Clockspeed",0},	
		{MENU_ICONS_MISC_PSPCLOCK_HELP,"value>222Mhz is NOT overclocking\nit's safe but battery runs out\nfaster",0},			
		{MENU_ICONS_MISC_HACKDEBUG,"Hack/debug menu",0},
		{MENU_ICONS_MISC_HACKDEBUG_HELP,"For advanced users.\nHelp to tune the rendering and\ngain some speed.",0},		
		{MENU_ICONS_MISC_SNAPSHOT,"Snapshot",0},
		{MENU_ICONS_MISC_SNAPSHOT_HELP,"Take a snapshot of the current\ngame screen.\nUsed in file browser.",0},		
		{MENU_ICONS_MISC_RANDBG,"Randomize BG",0},
		{MENU_ICONS_MISC_RANDBG_HELP,"Randomize BG from the DATA/logo.zip\nfile.",0},		
		{MENU_ICONS_MISC_BGMUSIC,"Menu background music",0},
		{MENU_ICONS_MISC_BGMUSIC_HELP,"Menu background music requires\nmake PSP running at 300Mhz\nMusic are from DATA/music.zip file.\nSPC format only.",0},		
		{MENU_ICONS_MISC_BGFX,"Menu background fx",0},	
		{MENU_ICONS_MISC_BGFX_HELP,"Nice FX from PS2DEV spline sample.",0},			
		{MENU_ICONS_MISC_PADBEEP,"Pad beep",0},	
		{MENU_ICONS_MISC_PADBEEP_HELP,"The beep sounds when browsing\nmenu and files.",0},			
		// CHEATS
		{MENU_ICONS_CHEATS,"CHEATS",0},
		{MENU_ICONS_CHEATS_ADDRAW,"Add a RAW code",0},
		{MENU_ICONS_CHEATS_ADDRAW_HELP,"Add a RAW code",0},		
		{MENU_ICONS_CHEATS_ADDGG,"Add a Game Genie code",0},
		{MENU_ICONS_CHEATS_ADDGG_HELP,"Add a Game Genie code",0},		
		{MENU_ICONS_CHEATS_ADDPAR,"Add a Pro Action Replay code",0},
		{MENU_ICONS_CHEATS_ADDPAR_HELP,"Add a Pro Action Replay code",0},		
		{MENU_ICONS_CHEATS_ADDGF,"Add a GoldFinger code",0},
		{MENU_ICONS_CHEATS_ADDGF_HELP,"Add a GoldFinger code",0},		
		{MENU_ICONS_CHEATS_DISABLE,"Disable code",0},
		{MENU_ICONS_CHEATS_DISABLE_HELP,"Disable code",0},		
		{MENU_ICONS_CHEATS_DISABLEALL,"Disable all codes",0},
		{MENU_ICONS_CHEATS_DISABLEALL_HELP,"Disable all codes",0},		
		{MENU_ICONS_CHEATS_ENABLE,"Enable code",0},
		{MENU_ICONS_CHEATS_ENABLE_HELP,"Enable code",0},		
		{MENU_ICONS_CHEATS_ENABLEALL,"Enable all codes",0},
		{MENU_ICONS_CHEATS_ENABLEALL_HELP,"Enable all codes",0},		
		{MENU_ICONS_CHEATS_REMOVE,"Remove code",0},
		{MENU_ICONS_CHEATS_REMOVE_HELP,"Remove code",0},		
		{MENU_ICONS_CHEATS_REMOVEALL,"Remove all codes",0},
		{MENU_ICONS_CHEATS_REMOVEALL_HELP,"Remove all codes",0},		
		// ABOUT
		{MENU_ICONS_ABOUT,"ABOUT",0},
		{MENU_ICONS_ABOUT_CREDITS,"Credits",0},
		{MENU_ICONS_ABOUT_VERSION,"Version infos",0},
};

////////////////////////////////////////////////////////////////////////////////////////
// psp_msg_string
// -------------- 
//		input : message id
//		output : string
//		comments : return the asked string in current language
////////////////////////////////////////////////////////////////////////////////////////
char *psp_msg_string(int num) {
	int msg_num=0;
	if (os9x_language == LANGUAGE_JAPANESE)
		while (msg_num<MSG_TOTAL_ENTRIES) {
			if (num==s9xTYL_msg_ja[msg_num].msg_id) return s9xTYL_msg_ja[msg_num].msg;
			msg_num++;
		}
	else
		while (msg_num<MSG_TOTAL_ENTRIES) {
			if (num==s9xTYL_msg_en[msg_num].msg_id) return s9xTYL_msg_en[msg_num].msg;
			msg_num++;
		}
	return "unknown string";//NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
// psp_msg
// -------------- 
//		input : message id, display length
//		output : none
//		comments : show the message in current language.
//							 len allow display length value and special dialog selection
//							 ("yes/no" box or "ok" box).
////////////////////////////////////////////////////////////////////////////////////////
int psp_msg(int num,int len) {	
	int msg_num=0;

	if (os9x_language == LANGUAGE_JAPANESE)
		while (msg_num<MSG_TOTAL_ENTRIES) {
			if (num==s9xTYL_msg_ja[msg_num].msg_id) break;
			msg_num++;
		}
	else
		while (msg_num<MSG_TOTAL_ENTRIES) {
			if (num==s9xTYL_msg_en[msg_num].msg_id) break;
			msg_num++;
		}

	if (msg_num==MSG_TOTAL_ENTRIES) return 0;

	if (os9x_language == LANGUAGE_JAPANESE) {
		if (len==MSG_DEFAULT)	len=s9xTYL_msg_ja[msg_num].len;
		if (len>=0)	msgBoxLines(s9xTYL_msg_ja[msg_num].msg,s9xTYL_msg_ja[msg_num].len);
		else if (len==-1) inputBoxOK(s9xTYL_msg_ja[msg_num].msg);
		else if (len==-2) return inputBox(s9xTYL_msg_ja[msg_num].msg);
	}
	else {
		if (len==MSG_DEFAULT)	len=s9xTYL_msg_en[msg_num].len;
		if (len>=0)	msgBoxLines(s9xTYL_msg_en[msg_num].msg,s9xTYL_msg_en[msg_num].len);
		else if (len==-1) inputBoxOK(s9xTYL_msg_en[msg_num].msg);
		else if (len==-2) return inputBox(s9xTYL_msg_en[msg_num].msg);
	}

	return 0;
}

