#include "psp.h"

extern int os9x_language;
extern const char *os9x_btn_positive_str;
extern const char *os9x_btn_negative_str;

#define MSG_BTN_POSITIVE "\x81\x0B"
#define MSG_BTN_NEGATIVE "\x81\x0E"

#define MSG_TOTAL_ENTRIES 304
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

	{ERR_INIT_SNES,"Cannot init snes, memory issue",2*60},
	{ERR_LOADING_ROM,"Error while loading rom",60*2},
	{ERR_INIT_GFX,"Cannot initialize graphics",60*2},

	{ERR_CANNOT_ALLOC_MEM,"Cannot allocate memory",30},

	{ERR_USB_STARTING_USBBUS,"Error starting USB Bus driver (0x%08X)\n",0},
	{ERR_USB_STARTING_USBMASS,"Error starting USB Mass Storage driver (0x%08X)\n",0},
	{ERR_USB_SETTING_CAPACITY,"Error setting capacity with USB Mass Storage driver (0x%08X)\n",0},
	//confirm
	{ASK_EXIT,"Exit Snes9XTYL ?",-2},
	{ASK_DELETE,"Delete file ?",-2},
	{ASK_SAVEDIR,"The save directory is not found.\n"
		"To make the directory in the directory where this emulator is, Press " SJIS_CIRCLE  "\n"
		"To make the directory in SAVEDATA directory, Press " SJIS_CROSS "\n",
		0},
	//info
	{INFO_USB_ON,"USB activated",30},
	{INFO_USB_OFF,"USB disabled",30},
	{INFO_DELETING,"Deleting...",0},
	{INFO_EXITING,"Exiting, please wait...",0},
	//filer
	{FILER_STATUS_CANEXIT1,MSG_BTN_POSITIVE " RUN " SJIS_SQUARE " RUN default settings  " SJIS_STAR "  " MSG_BTN_NEGATIVE " TO GAME  "  SJIS_STAR "  " SJIS_TRIANGLE " %s  "  SJIS_STAR "  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT ", to browse",0},
	{FILER_STATUS_NOEXIT1,MSG_BTN_POSITIVE " RUN " SJIS_SQUARE " RUN default settings  "  SJIS_STAR "  " SJIS_TRIANGLE " %s          " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT ", to browse list",0},
	{FILER_STATUS_CANEXIT2,MSG_BTN_POSITIVE " RUN " MSG_BTN_NEGATIVE "BACK TO GAME " SJIS_TRIANGLE " %s  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT ", to browse list",0},
	{FILER_STATUS_NOEXIT2,MSG_BTN_POSITIVE " RUN " SJIS_TRIANGLE " %s                  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT ", to browse list",0},

	{FILER_STATUS_PARDIR, "Parent dir.", 0},

	{FILER_TITLE, "[" EMUNAME_VERSION "] - Choose a file", 0},
	
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

	{FILE_IPS_APPLYING,"Found IPS patch : %s\nSize is : %dKo\nApplying ....",0},
	{FILE_IPS_PATCHSUCCESS,"IPS Patch applied successfully",0},
	
	{MENU_STATUS_GENERIC_MSG1,"Press " MSG_BTN_NEGATIVE " to return",0},
	{MENU_STATUS_GENERIC_NEEDRELOAD,"Changes may require a RELOAD to take effect",0},
	{MENU_STATUS_GENERIC_NEEDRESET,"Changes may require a RESET to take effect",0},
	{MENU_STATUS_GENERIC_FREERAM,"Free RAM : %dKo",0},
	{MENU_STATUS_GENERIC_CHANGEMUSIC,"SELECT to change music",0},

	{MENU_TITLE_GENERIC_BAT,"%02d%c%02d Bat.:%s%s%s%02d%%%s Tmp.%dC",0},
	{MENU_TITLE_GENERIC_BAT_TIME,"(%02dh%02dm)",0},
	{MENU_TITLE_GENERIC_BAT_PLG,"Plg.",0},
	{MENU_TITLE_GENERIC_BAT_CHRG,"Chrg.",0},
	{MENU_TITLE_GENERIC_BAT_LOW,"Low!",0},

	{MENU_CHANGE_VALUE,"  ,   change value",0},
	{MENU_CHANGE_VALUE_WITH_FAST,"  ,   change value -  ,  fast",0},
	{MENU_CANCEL_VALIDATE,"  ,   cancel   -      validate",0},
	{MENU_DEFAULT_VALUE,"   default value",0},
	{MENU_YES,"yes",0},
	{MENU_NO,"no",0},
	{MENU_NOT_IMPLEMENTED,"Not yet implemented.\n\n",0},

	{MENU_SOUND_MODE_NOTEMUL,"not emulated",0},
	{MENU_SOUND_MODE_EMULOFF,"emulated, no output",0},
	{MENU_SOUND_MODE_EMULON,"emulated, output",0},

	{MENU_STATE_CHOOSEDEL,"Choose a slot to DELETE",0},
	{MENU_STATE_CHOOSELOAD,"Choose a slot to LOAD",0},
	{MENU_STATE_CHOOSESAVE,"Choose a slot to SAVE",0},
	{MENU_STATE_FREESLOT,"FREE",0},
	{MENU_STATE_SCANNING,"Scanning used slots...",0},


	{ERR_INIT_OSK,"cannot init OSK",20},
	{ERR_ADD_CODE,"Cannot add cheat code, too many already enterred!",60},

	{MENU_CHEATS_ENTERGF,"Enter a GoldFinger code",0},
	{MENU_CHEATS_ENTERPAR,"Enter a Pro Action Replay code",0},
	{MENU_CHEATS_ENTERGG,"Enter a Game Genie code",0},
	{MENU_CHEATS_ENTERRAW,"Enter a RAW code\nFormat is ADDRESS - NEW VALUE",0},
	{MENU_CHEATS_CONFIRMREMALL,"Remove all codes ?",-2},
	{MENU_CHEATS_ENTERNAME,"Enter Name",0},
	{MENU_CHEATS_PREVPAGE,"L - Prev. Page",0},
	{MENU_CHEATS_NEXTPAGE,"R - Next Page",0},

	{MENU_STATE_CONFIRMLOAD,"Current game progress will be lost.\nContinue ?",-2},
	{MENU_STATE_CONFIRMDELETE,"Delete current state ?",-2},
	{MENU_STATE_CONFIRMSAVE,"Overwrite current savestate ?",-2},
	{MENU_STATE_ISLOADING,"Loading state...",10},
	{MENU_STATE_ISSAVING,"Saving state...",10},
	{MENU_STATE_ISDELETING,"Deleting...",10},
	{MENU_STATE_ISIMPORTING,"Importing state...",10},
	{MENU_STATE_ISEXPORTINGS9X,"Exporting SNES9X state...",10},
	{MENU_STATE_NOSTATE,"No state available...",10},
	{MENU_STATE_AUTOSAVETIMER,"every %dmin.",0},
	{MENU_STATE_AUTOSAVETIMER_OFF,"off",0},
	{MENU_STATE_WARNING_LOWBAT,"Battery is low.\nIf PSP turns off while saving, Memory Stick may be broken.\nContinue?",-2},
	{MENU_GAME_SAVINGDEFAULTSETTINGS,"Saving current settings as default profile",30},
	{MENU_GAME_CONFIRMRESET,"Reset Snes ?",-2},
	{MENU_MISC_SAVINGJPEG,"Saving JPEG snapshot...",10},

	{MENU_MISC_BGMUSIC_RAND,"Play randomly",0},
	{MENU_MISC_BGMUSIC_ORDER,"Play in order",0},

	{MENU_MISC_BGMUSIC_UNKNOWN,"unknown",0},
	{MENU_MISC_BGMUSIC_GAMETITLE,"Playing : ",0},
	{MENU_MISC_BGMUSIC_TITLE,"Title : ",0},
	{MENU_MISC_BGMUSIC_AUTHOR,"Author : ",0},


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

	{MENU_CONTROLS_INPUT,"Joypad #%d",0},
	{MENU_CONTROLS_INPUT_PRESS,"Press a button for %s",0},
	{MENU_CONTROLS_INPUT_NOFORMENU,"No button defined for MENU Access!\nPlease choose one",30},
#ifdef HOME_HOOK
	{MENU_CONTROLS_INPUT_DEFAULT,"Choose a default profile : \n\n"\
				SJIS_CROSS " - Default, SNES pad mapped to PSP pad.\n\n"\
				SJIS_CIRCLE " - Default, SNES pad mapped to PSP stick.\n\n"\
				SJIS_SQUARE " - Default, SNES pad mapped to PSP pad&stick.\n\n"\
				SJIS_TRIANGLE " Cancel\n",0},
#else
	{MENU_CONTROLS_INPUT_DEFAULT,"Choose a default profile : \n\n"\
				SJIS_CROSS " - Default, SNES pad mapped to PSP pad. GUI on stick left.\n\n"\
				SJIS_CIRCLE " - Default, SNES pad mapped to PSP stick. GUI on pad left.\n\n"\
				SJIS_SQUARE " - Default, SNES pad mapped to PSP pad&stick. GUI on LTrg.+RTrg.\n\n"\
				SJIS_TRIANGLE " Cancel\n",0},
#endif

	{MENU_STATUS_CONTROLS_INPUT,MSG_BTN_POSITIVE " Detect mode   " SJIS_STAR "  " MSG_BTN_NEGATIVE " Menu  ",0},
	{MENU_STATUS_CONTROLS_INPUT_0,SJIS_STAR "  " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value  " SJIS_STAR "  SELECT default profiles",0},
	{MENU_STATUS_CONTROLS_INPUT_HELP_0,SJIS_STAR "      " SJIS_TRIANGLE " Help       " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value",0},
	{MENU_STATUS_CONTROLS_INPUT_HELP_1,SJIS_STAR "     " SJIS_TRIANGLE "   Help      " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value",0},
	{MENU_STATUS_CONTROLS_INPUT_HELP_2,SJIS_STAR "    " SJIS_TRIANGLE "     Help     " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value",0},
	{MENU_STATUS_CONTROLS_INPUT_HELP_3,SJIS_STAR "   " SJIS_TRIANGLE "       Help    " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value",0},

	{MENU_TITLE_CONTROLS_INPUT,"[" EMUNAME_VERSION "] - Inputs Config",0},

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

	{MENU_VIDEO_MODE_1_1,"1:1",0},
	{MENU_VIDEO_MODE_ZOOM_FIT,"zoom fit",0},
	{MENU_VIDEO_MODE_ZOOM_4_3RD,"zoom 4/3",0},
	{MENU_VIDEO_MODE_ZOOM_WIDE,"zoom wide",0},
	{MENU_VIDEO_MODE_FULLSCREEN,"fullscreen",0},
	{MENU_VIDEO_MODE_FULLSCREEN_CLIPPED,"fullscreen clipped",0},
	{MENU_VIDEO_ENGINE_APPROX,"approx. software",0},
	{MENU_VIDEO_ENGINE_ACCUR,"accur. software",0},
	{MENU_VIDEO_ENGINE_ACCEL,"PSP accelerated",0},
	{MENU_VIDEO_ENGINE_ACCEL_ACCUR,"PSP accel. + accur. soft.",0},
	{MENU_VIDEO_ENGINE_ACCEL_APPROX,"PSP accel. + approx. soft.",0},
	{MENU_VIDEO_SLIMITVALUE_AUTO,"AUTO (detect PAL/NTSC)",0},
	{MENU_VIDEO_FSKIP_MODE_AUTO,"auto change below %d",0},
	{MENU_VIDEO_FSKIP_CHANGEAUTO_AUTO,"   auto change below set value",0},
	{MENU_VIDEO_FSKIP_CHANGEAUTO_FIXED,"   fixed",0},

	{MENU_STATUS_VIDEO_SCRCALIB,"PAD : SCREEN POSITION  " SJIS_STAR "  ANALOG STICK : SCREEN SIZE  " SJIS_STAR "  " SJIS_TRIANGLE " default  " SJIS_STAR "  " MSG_BTN_NEGATIVE " Exit",0},

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
	{MENU_ICONS_MISC_SWAPBG,"Swap BG",0},
	{MENU_ICONS_MISC_SWAPBG_HELP,"Swap BG from the DATA/logo.zip\nfile.",0},
	{MENU_ICONS_MISC_BGMUSIC,"Menu background music",0},
	{MENU_ICONS_MISC_BGMUSIC_HELP,"Menu background music requires\nmake PSP running at 300Mhz\nMusic are from DATA/music.zip file.\nSPC format only.",0},
	{MENU_ICONS_MISC_BGFX,"Menu background fx",0},
	{MENU_ICONS_MISC_BGFX_HELP,"Nice FX from PS2DEV spline sample.",0},
	{MENU_ICONS_MISC_PADBEEP,"Pad beep",0},
	{MENU_ICONS_MISC_PADBEEP_HELP,"The beep sounds when browsing\nmenu and files.",0},
	{MENU_ICONS_MISC_AUTOSTART,"Last ROM Auto-Start",0},
	{MENU_ICONS_MISC_AUTOSTART_HELP,"If you set to [yes],the ROM\nwhich was loaded at last\n will start automatically\nwhen the emulator started.",0},
	{MENU_ICONS_MISC_OSK,"OSK",0},
	{MENU_ICONS_MISC_OSK_HELP,"OSK is On-Screen Keyboard.\nDanzeff OSK is useful\nfor typing alphabet on PSP\nSCE's official OSK is useful\nfor typing Japanese or on PS Vita.",0},

	{MENU_MUSIC_SWAPBG_NODATA,"No Data",0},
	{MENU_MISC_SWAPBG_RAND,"   Random",0},

	{MENU_MISC_OSK_DANZEFF,"Danzeff",0},
	{MENU_MISC_OSK_OFFICIAL,"Official",0},

	{MENU_STATUS_MISC_HACKDEBUG,MSG_BTN_NEGATIVE " Main Menu   ",0},
	{MENU_STATUS_MISC_HACKDEBUG_FUNC,MSG_BTN_POSITIVE " OK     " SJIS_STAR "    " MSG_BTN_NEGATIVE " Main Menu   ",0},
	{MENU_STATUS_MISC_HACKDEBUG_0,SJIS_STAR "    " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value"},

	{MENU_STATUS_MISC_HACKDEBUG_HELP_0,SJIS_STAR "      " SJIS_TRIANGLE " Help       " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value"},
	{MENU_STATUS_MISC_HACKDEBUG_HELP_1,SJIS_STAR "     " SJIS_TRIANGLE "   Help      " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value"},
	{MENU_STATUS_MISC_HACKDEBUG_HELP_2,SJIS_STAR "    " SJIS_TRIANGLE "     Help     " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value"},
	{MENU_STATUS_MISC_HACKDEBUG_HELP_3,SJIS_STAR "   " SJIS_TRIANGLE "       Help    " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value"},

	{MENU_TITLE_MISC_HACKDEBUG,"[" EMUNAME_VERSION "] - Menu",0},

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

	{MENU_ABOUT_VERSION_TIMESTAMP,"Build on %s",0},
	{MENU_ABOUT_VERSION_GCCVER,"With GCC %s",0},

	//Others
	// Adhoc
	{ADHOC_CONNECTING,"Connecting...\n",0},
	{ADHOC_SELECTORRETURN,"Select a server to connect to, or triangle to return",0},
	{ADHOC_WAITING,"Waiting for %s to accept the connection\nTo cancel press O\n",0},
	{ADHOC_REQUESTED,"%s has requested a connection\nTo accept the connection press X, to cancel press O\n",0},
	{ADHOC_CONNECTED,"Connected",0},
	{ADHOC_STATE,"  connection state %d of 1\n",0},
	{ADHOC_UNKNOWNCOMMAND,"Unknown command %02X !",0},
	{ADHOC_SYNCLOST_SERVER,"SERVER Lost sync : resync!!!",20},
	{ADHOC_SYNCLOST_CLIENT,"CLIENT Lost sync : resync!!!",20},
	{ADHOC_CLOSING,"closing connection",60},
	{ADHOC_DRIVERLOAD_ERR,"Net driver load error",60*2},
	{ADHOC_INIT_ERR,"Issue with init adhoc game\n",60},
	{ADHOC_LOST_CONNECTION,"Lost connection!",60},
	{ADHOC_CORRUPTED_PKT,"corrupted pkt",0},
	{ADHOC_STATE_SIZE,"state size : %d",0},
	{ADHOC_FILE_ERR_RECEIVING,"file error while receiving state!",60*1},
	{ADHOC_FILE_ERR_SENDING,"file error while sending state!",60*1},
	{ADHOC_STILL,"still : %d",0},
	{ADHOC_WAITING_OTHER,"Waiting for other player\n\nPress " SJIS_TRIANGLE " to close connection and quit netplay\n",10},
	{ADHOC_NETWORKERR_1,"network error 1",60*1},
	{ADHOC_CANNOTFIND,"cannot find save state!",60},
	{ADHOC_FLUSHING,"flushing network, please wait a few seconds",10},
	// LOAD/SAVE
	{LOADSAVE_AUTOSAVETIMER,"Autosaving...",0},
	{LOADSAVE_EXPORTS9XSTATE,"Found a snes9xTYL file",0},
	// VIDEO
	{VIDEO_ENGINE_APPROX,"Simple mode : accurate software",30},
	{VIDEO_ENGINE_ACCUR,"Simple mode : approx. software",30},
	{VIDEO_ENGINE_ACCEL,"Simple mode : PSP accelerated",30},
	{VIDEO_ENGINE_ACCEL_ACCUR,"Mixed modes : PSP accelerated + accurate software",30},
	{VIDEO_ENGINE_ACCEL_APPROX,"Mixed modes : PSP accelerated + approx. software",30},
	{VIDEO_FSKIP_AUTO,"Frameskip : AUTO",0},
	{VIDEO_FSKIP_MANUAL,"Frameskip : %d",0},
	// Scroll Message
	{SCROLL_TITLE,"   ,   to move -  ,  for fast mode",0},
	{SCROLL_STATUS_0,"Line %d/%d  -  Page %d/%d",0},
	{SCROLL_STATUS_1,"   exit,        help  ",0},
	{SCROLL_HELP,"Snes9xTYL - fileviewer\n\n" SJIS_TRIANGLE " Find, then " MSG_BTN_POSITIVE " Find next, " SJIS_SQUARE " Find previous\n" \
						SJIS_UP "," SJIS_DOWN " scroll text, L,R scroll faster\n" MSG_BTN_NEGATIVE " exit\n\nLast position is keeped if same file is reopened.\nHowever it will be reset if another file is opened.\n\n" \
						"Press " MSG_BTN_NEGATIVE,0},
	{SCROLL_SEARCHING,"Searching...",0},
	{SCROLL_STRNOTFOUND,"String not found!",30},
	{SCROLL_DISCLAIMER,"Disclaimer",0},
	{SCROLL_OSK_DESC,"Enter String to find",0},
	//Settings
	{SETTINGS_NOTCOMPLETE,"!!Settings file not complete!!\n\nProbably coming from a previous version.\n\nNew settings will be set with default values",60*3},
	{SETTINGS_NOTFOUND,"No settings found, using default",10},
	{SETTINGS_FORCING_DEFAULT,"Forcing default settings",10},
	{SETTINGS_IGNORED,"ini file from a previous incompatible version, ignored!",20},
	{SETTINGS_FOUND,"Settings found!\n\n""%s""",0},
	{SETTINGS_DIFF,"ini file from a different version.",20},
	//Others
	{APU_DEBUGGING,"APU DEBUGGING",10},
	{BGMUSIC_PLAYING,"Playing spc file...",0},
	{FOUND_SRAM,"Found an SRAM file",60},
	{CANCEL,"CANCEL",0},
	{INPUTBOX_OK,"\n\n      Close",0},
#ifdef ME_SOUND
	{BAT_ISLOW,"Battery is low, saving is now disabled (SRAM,states and settings).\n\nThis window will close in 3 seconds.",60*3},
#else
	{BAT_ISLOW,"Battery is low, saving is now disabled (SRAM,states and settings).\n\nYou can still put your PSP in sleep mode and charge battery later.\n\nThis window will close in 3 seconds.",60*3},
#endif
	{CONV_DONE,"conversion done",30},
	{ROM_LIED,"ROM lied about its type! Trying again.",30},
	{LOADING_ROM,"Loading %dKo...",0},
	{TYPE,"Type",0},
	{MODE,"Mode",0},
	{COMPANY,"Company",0},
#ifdef _BSX_151_
	{SRAM_NOTFOUND,"The SRAM file wasn't found: BS-X.srm was read instead.",30},
	{SRAM_BSX_NOTFOUND,"The SRAM file wasn't found, BS-X.srm wasn't found either.",30},
#endif
	{CORRUPT,"Corrupt",0},
	{ROM_ONLY,"ROM only",0},
	{EXTENDING,"Extending",30},
	{EXTENDING_TARGET,"Extending target from %d to %d\n",0},
	{INIT_OSK,"Initializing OSK...",0}
};

t_err_entry s9xTYL_msg_ja[MSG_TOTAL_ENTRIES]= {
//japanese
	//error
	{ERR_OUT_OF_MEM,"メモリ外です。",60},
	{ERR_READ_MEMSTICK,"メモリースティックの読み込みでエラーが発生しました。\nメモリースティックを確認してください。\n(PCのカードリーダーを使ってください)\nメモリースティックが破損している可能\性があります。",60},

	{ERR_INIT_SNES,"メモリの問題でSFCを初期化できません",2*60},
	{ERR_LOADING_ROM,"ROM読み込みエラー",60*2},
	{ERR_INIT_GFX,"グラフィックスを初期化できません",60*2},

	{ERR_CANNOT_ALLOC_MEM,"メモリを割り当てられません",30},

	{ERR_USB_STARTING_USBBUS,"USBバスドライバ開始エラー (0x%08X)\n",0},
	{ERR_USB_STARTING_USBMASS,"USB大容量記憶装置ドライバ開始エラー (0x%08X)\n",0},
	{ERR_USB_SETTING_CAPACITY,"USB大容量記憶装置ドライバ容量設定エラー (0x%08X)\n",0},
	//confirm
	{ASK_EXIT,"Snes9XTYLを終了しますか?",-2},
	{ASK_DELETE,"ファイルを削除しますか?",-2},
	{ASK_SAVEDIR,"セーブディレクトリが見つかりません。\n"
		"このエミュレーターがあるディレクトリに作成するには" SJIS_CIRCLE "ボタンを押してください。\n"
		"SAVEDATAディレクトリに作成するには" SJIS_CROSS "ボタンを押してください。\n",
		0},
	//info
	{INFO_USB_ON,"USBが有効化されました",30},
	{INFO_USB_OFF,"USBが無効化されました",30},
	{INFO_DELETING,"削除しています...",0},
	{INFO_EXITING,"終了しています。しばらくお待ちください...",0},
	//filer
	{FILER_STATUS_CANEXIT1,MSG_BTN_POSITIVE " 実行 " SJIS_SQUARE " 標準設定で実行  " SJIS_STAR "  " MSG_BTN_NEGATIVE " ゲームに戻る  "  SJIS_STAR "  " SJIS_TRIANGLE " %s  "  SJIS_STAR "  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT " 移動",0},
	{FILER_STATUS_NOEXIT1,MSG_BTN_POSITIVE " 実行 " SJIS_SQUARE " 標準設定で実行  "  SJIS_STAR "  " SJIS_TRIANGLE " %s          " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT " リストを移動",0},
	{FILER_STATUS_CANEXIT2,MSG_BTN_POSITIVE " 実行 " MSG_BTN_NEGATIVE "ゲームに戻る " SJIS_TRIANGLE " %s  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT " リストを移動",0},
	{FILER_STATUS_NOEXIT2,MSG_BTN_POSITIVE " 実行 " SJIS_TRIANGLE " %s                  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT " リストを移動",0},

	{FILER_STATUS_PARDIR, "上へ", 0},

	{FILER_TITLE, "[" EMUNAME_VERSION "] - ファイル選択", 0},
	
	{FILER_HELP_WINDOW1,"標準と通信対戦モードを切り替える",0},
	{FILER_HELP_WINDOW2,"にはSTARTを押してください",0},
#ifdef FW3X
	{FILER_HELP_WINDOW3," ",0},
#ifdef HOME_HOOK
	{FILER_HELP_WINDOW4,"終了するにはHOMEを押してください",0},
#else
	{FILER_HELP_WINDOW4,"終了するにはL+Rを押してください",0},
#endif
#else
#ifdef HOME_HOOK
	{FILER_HELP_WINDOW3,"終了するにはHOMEを押してください",0},
#else
	{FILER_HELP_WINDOW3,"終了するにはL+Rを押してください",0},
#endif
	{FILER_HELP_WINDOW4,"USBｵﾝｵﾌ切り替えにRを押してください",0},
#endif


	{FILER_HELP_WINDOW5,"削除するにはSELECTを押してください",0},

	{FILE_IPS_APPLYING,"IPSパッチが見つかりました : %s\nサイズ : %dKo\n適用しています....",0},
	{FILE_IPS_PATCHSUCCESS,"IPSパッチの適用に成功しました",0},

	{MENU_STATUS_GENERIC_MSG1,"戻るには" MSG_BTN_NEGATIVE "を押してください",0},
	{MENU_STATUS_GENERIC_NEEDRELOAD,"変更を適用するには再読み込みが必要かもしれません",0},
	{MENU_STATUS_GENERIC_NEEDRESET,"変更を適用するにはリセットが必要かもしれません",0},
	{MENU_STATUS_GENERIC_FREERAM,"空きメモリ : %dKo",0},
	{MENU_STATUS_GENERIC_CHANGEMUSIC,"音楽を変えるにはSELECTボタンを、",0},

	{MENU_TITLE_GENERIC_BAT,"%02d%c%02d 電池:%s%s%s残り%02d%%%s 温度.%d℃",0},
	{MENU_TITLE_GENERIC_BAT_TIME,"(%02d時間%02d分)",0},
	{MENU_TITLE_GENERIC_BAT_PLG,"AC.",0},
	{MENU_TITLE_GENERIC_BAT_CHRG,"充電中.",0},
	{MENU_TITLE_GENERIC_BAT_LOW,"残量低下.",0},

	{MENU_CHANGE_VALUE,"  ,   値を変更",0},
	{MENU_CHANGE_VALUE_WITH_FAST,"  ,   値を変更   -    ,  高速",0},
	{MENU_CANCEL_VALIDATE,"  ,   キャンセル -    適用",0},
	{MENU_DEFAULT_VALUE,"   標準値",0},
	{MENU_YES,"はい",0},
	{MENU_NO,"いいえ",0},
	{MENU_NOT_IMPLEMENTED,"まだ実装されていません。\n\n",0},

	{MENU_SOUND_MODE_NOTEMUL,"エミュレートしない",0},
	{MENU_SOUND_MODE_EMULOFF,"エミュレートするが出力しない",0},
	{MENU_SOUND_MODE_EMULON,"エミュレートし、出力する",0},
	
	{MENU_STATE_CHOOSEDEL,"削除するスロットを選んでください",0},
	{MENU_STATE_CHOOSELOAD,"読み込むスロットを選んでください",0},
	{MENU_STATE_CHOOSESAVE,"保存するスロットを選んでください",0},
	{MENU_STATE_FREESLOT,"空き",0},
	{MENU_STATE_SCANNING,"使用済みスロットを調べています...",0},


	{ERR_INIT_OSK,"OSKを初期化できません",20},
	{ERR_ADD_CODE,"チートコードを追加できません。既に入力されているコードが多すぎます。",60},

	{MENU_CHEATS_ENTERGF,"GoldFingerコードを入力してください",0},
	{MENU_CHEATS_ENTERPAR,"プロアクションリプレイコードを入力してください",0},
	{MENU_CHEATS_ENTERGG,"Game Genieコードを入力してください",0},
	{MENU_CHEATS_ENTERRAW,"RAWコードを入力してください\n書式 : アドレス - 新しい値",0},
	{MENU_CHEATS_ENTERNAME,"名前を入力してください",0},
	{MENU_CHEATS_CONFIRMREMALL,"すべてのコードを削除しますか?",-2},
	{MENU_CHEATS_PREVPAGE,"L - 前のページ",0},
	{MENU_CHEATS_NEXTPAGE,"R - 次のページ",0},

	{MENU_STATE_CONFIRMLOAD,"現在のゲーム進行が失われます。\n続行しますか?",-2},
	{MENU_STATE_CONFIRMDELETE,"このステートセーブを削除しますか?",-2},
	{MENU_STATE_CONFIRMSAVE,"このステートセーブに上書きしますか?",-2},
	{MENU_STATE_ISSAVING,"ステートセーブしています...",10},
	{MENU_STATE_ISLOADING,"状態を読み込んでいます...",10},
	{MENU_STATE_ISDELETING,"削除しています...",10},
	{MENU_STATE_ISIMPORTING,"ステートセーブをインポートしています...",10},
	{MENU_STATE_ISEXPORTINGS9X,"SNES9Xステートセーブをエクスポートしています...",10},
	{MENU_STATE_NOSTATE,"使用可能\なステートセーブがありません",10},
	{MENU_STATE_AUTOSAVETIMER,"%d分毎",0},
	{MENU_STATE_AUTOSAVETIMER_OFF,"オフ",0},
	{MENU_STATE_WARNING_LOWBAT,"電池残量が低下しています。\nセーブ中に電源が切れるとメモリースティックが破損する可能\性があります。\n続行しますか?",-2},
	{MENU_GAME_SAVINGDEFAULTSETTINGS,"現在の設定を標準設定として保存しています",30},
	{MENU_GAME_CONFIRMRESET,"SFCをリセットしますか?",-2},
	{MENU_MISC_SAVINGJPEG,"JPEGスナップショットを保存しています...",10},

	{MENU_MISC_BGMUSIC_RAND,"ランダムに再生",0},
	{MENU_MISC_BGMUSIC_ORDER,"順に再生",0},

	{MENU_MISC_BGMUSIC_UNKNOWN,"不明",0},
	{MENU_MISC_BGMUSIC_GAMETITLE,"ゲーム : ",0},
	{MENU_MISC_BGMUSIC_TITLE,"題名 : ",0},
	{MENU_MISC_BGMUSIC_AUTHOR,"作者 : ",0},


	// GAME
	{MENU_ICONS_GAME,"ゲーム",0},
	{MENU_ICONS_GAME_RESET,"SFCをリセットする",0},
	{MENU_ICONS_GAME_NEW,"新しいゲームを読み込む",0},
	{MENU_ICONS_GAME_DEFAULTSETTINGS,"標準設定を作成する",0},
	{MENU_ICONS_GAME_DEFAULTSETTINGS_HELP,"現在の設定を特別に設定ファイルがない\n全てのROMに使われる標準設定として\n保存します。",0},
	{MENU_ICONS_GAME_EXIT,"Snes9xTYLを終了する",0},
	// LOAD/SAVE
	{MENU_ICONS_LOADSAVE,"ロード/セーブ",0},
	{MENU_ICONS_LOADSAVE_LOADSTATE,"ステートロード",0},
	{MENU_ICONS_LOADSAVE_LOADSTATE_HELP,"以前保存したステートセーブを\n読み込みます。ステートセーブはSFC\nの状態のスナップショットのような\nもので、すべて保存されます。",0},
	{MENU_ICONS_LOADSAVE_SAVESTATE,"ステートセーブ",0},
	{MENU_ICONS_LOADSAVE_SAVESTATE_HELP,"ステートセーブします。",0},
	{MENU_ICONS_LOADSAVE_DELETESTATE,"ステートセーブを削除",0},
	{MENU_ICONS_LOADSAVE_DELETESTATE_HELP,"ステートセーブのセーブデータを\n削除します。",0},	
	{MENU_ICONS_LOADSAVE_IMPORTSTATE,"ステートセーブをインポート",0},
	{MENU_ICONS_LOADSAVE_IMPORTSTATE_HELP,"uosnes9xや以前のバージョンから\nステートセーブのセーブデータを\nインポートします。",0},

	{MENU_ICONS_LOADSAVE_EXPORTS9XSTATE,"SNES9Xのステートセーブにエクスポート",0},
	{MENU_ICONS_LOADSAVE_EXPORTS9XSTATE_HELP,"SNES9Xフォーマットを使って\nステートセーブのセーブデータを\nエクスポートします。",0},	

	{MENU_ICONS_LOADSAVE_AUTOSAVETIMER,"オートステートセーブタイマー",0},
	{MENU_ICONS_LOADSAVE_AUTOSAVETIMER_HELP,"定期的にオートステートセーブを\nさせます。",0},
	{MENU_ICONS_LOADSAVE_AUTOSRAM,"SRAM更新時にオートセーブする",0},
	{MENU_ICONS_LOADSAVE_AUTOSRAM_HELP,"変更が行われるたびにSRAMが\n保存されるようにします。",0},
	// CONTROLS
	{MENU_ICONS_CONTROLS,"コントローラ",0},
	{MENU_ICONS_CONTROLS_REDEFINE,"再設定",0},
	{MENU_ICONS_CONTROLS_REDEFINE_HELP,"ボタン割り当てを再設定します。",0},
	{MENU_ICONS_CONTROLS_INPUT,"エミュレートされるコントローラ",0},
	{MENU_ICONS_CONTROLS_INPUT_HELP,"現在有効にするSFCコントローラを\n選んでください。",0},

	{MENU_CONTROLS_INPUT,"コントローラ %d",0},
	{MENU_CONTROLS_INPUT_PRESS,"%sに割り当てるボタンを押してください",0},
	{MENU_CONTROLS_INPUT_NOFORMENU,"メニューにアクセスするために設定されたボタンありません。\nボタンを選んでください。",30},
#ifdef HOME_HOOK
	{MENU_CONTROLS_INPUT_DEFAULT,"標準プロファイルを選択してください\n\n"\
				SJIS_CROSS " - 標準, SFCの十\字ボタンがPSPの十\字ボタンに割り当てられます。\n\n"\
				SJIS_CIRCLE " - 標準, SFCの十\字ボタンがPSPのアナログスティックに割り当てられます。\n\n"\
				SJIS_SQUARE " - 標準, SFCの十\字ボタンがPSPの十\字ボタンとアナログスティックに割り当てられます。\n\n"\
				SJIS_TRIANGLE " キャンセル\n",0},
#else
	{MENU_CONTROLS_INPUT_DEFAULT,"標準プロファイルを選択してください\n\n"\
				SJIS_CROSS " - 標準, SFCの十\字ボタンがPSPの十\字ボタンに割り当てられます。\nメニューはアナログスティック左に割り当てます。\n\n"\
				SJIS_CIRCLE " - 標準, SFCの十\字ボタンがPSPのアナログスティックに割り当てられます。\nメニューは十\字ボタン左に割り当てます。\n\n"\
				SJIS_SQUARE " - Default, SFCの十\字ボタンがPSPの十\字ボタンとアナログスティックに割り当てられます。\nメニューはL+Rトリガーに割り当てます。\n\n"\
				SJIS_TRIANGLE " キャンセル\n",0},
#endif

	{MENU_STATUS_CONTROLS_INPUT,MSG_BTN_POSITIVE " 検出モード   " SJIS_STAR "  " MSG_BTN_NEGATIVE " メニュー  ",0},
	{MENU_STATUS_CONTROLS_INPUT_0,SJIS_STAR "  " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更  " SJIS_STAR "  SELECT 標準設定",0},
	{MENU_STATUS_CONTROLS_INPUT_HELP_0,SJIS_STAR "      " SJIS_TRIANGLE " ヘルプ       " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更",0},
	{MENU_STATUS_CONTROLS_INPUT_HELP_1,SJIS_STAR "     " SJIS_TRIANGLE "   ヘルプ      " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更",0},
	{MENU_STATUS_CONTROLS_INPUT_HELP_2,SJIS_STAR "    " SJIS_TRIANGLE "     ヘルプ     " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更",0},
	{MENU_STATUS_CONTROLS_INPUT_HELP_3,SJIS_STAR "   " SJIS_TRIANGLE "       ヘルプ    " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更",0},

	{MENU_TITLE_CONTROLS_INPUT,"[" EMUNAME_VERSION "] - 入力設定",0},

	// VIDEO OPTIONS
	{MENU_ICONS_VIDEO,"ビデオ",0},
	{MENU_ICONS_VIDEO_MODE,"出力モード",0},
	{MENU_ICONS_VIDEO_MODE_HELP,"アスペクト比を変更します。\n4:3が推薦されます。",0},
	{MENU_ICONS_VIDEO_ENGINE,"エンジン",0},
	{MENU_ICONS_VIDEO_ENGINE_HELP,"ハードウェアクセラレートモードが\nいつも最速なわけではありません。",0},
	{MENU_ICONS_VIDEO_SLIMITONOFF,"速度制限切り替え",0},
	{MENU_ICONS_VIDEO_SLIMITONOFF_HELP,"ゲームの速度を制限するには\nオンにしてください。",0},
	{MENU_ICONS_VIDEO_SLIMITVALUE,"FPS速度制限",0},
	{MENU_ICONS_VIDEO_SLIMITVALUE_HELP,"速度制限がオンのときのみ\n使われます。",0},
	{MENU_ICONS_VIDEO_FSKIP,"フレームスキップ",0},
	{MENU_ICONS_VIDEO_FSKIP_HELP,"表\示されないスプライトがある場合は\n固定値を選んでください。",0},
	{MENU_ICONS_VIDEO_SMOOTHING,"スムーシング",0},
	{MENU_ICONS_VIDEO_SMOOTHING_HELP,"何も犠牲にしないときのみ\nオンにすべきです。",0},
	{MENU_ICONS_VIDEO_VSYNC,"垂直同期",0},
	{MENU_ICONS_VIDEO_VSYNC_HELP,"これは多くのFPSを犠牲にするので\nオフにすべきです。",0},
	{MENU_ICONS_VIDEO_PALASNTSC,"PALをNTSCとして描画する",0},
	{MENU_ICONS_VIDEO_PALASNTSC_HELP,"PALのゲームを240本の垂直解像度の\n代わりに224本にしてNTSCのもののように\nエミュレートするよう強制します。",0},		
	{MENU_ICONS_VIDEO_GAMMA,"ガンマ補正",0},
	{MENU_ICONS_VIDEO_GAMMA_HELP,"より明るめな描画をさせます。",0},
	{MENU_ICONS_VIDEO_SCRCALIB,"画面位置補正",0},
	{MENU_ICONS_VIDEO_SCRCALIB_HELP,"邪魔な黒い枠を取り除くのに\n役立ちます。",0},
	{MENU_ICONS_VIDEO_SHOWFPS,"FPS表\示",0},
	{MENU_ICONS_VIDEO_SHOWFPS_HELP,"1行目はエミュレートされる\nフレーム数を表\示します。\n2行目は本当のFPSを表\示します。",0},

	{MENU_VIDEO_MODE_1_1,"等倍",0},
	{MENU_VIDEO_MODE_ZOOM_FIT,"画面に合わせて拡大",0},
	{MENU_VIDEO_MODE_ZOOM_4_3RD,"4/3倍",0},
	{MENU_VIDEO_MODE_ZOOM_WIDE,"ワイドに拡大",0},
	{MENU_VIDEO_MODE_FULLSCREEN,"全画面表\示",0},
	{MENU_VIDEO_MODE_FULLSCREEN_CLIPPED,"一部を切り取って全画面表\示",0},
	{MENU_VIDEO_ENGINE_APPROX,"ほぼ正確なソ\フトウェア描画",0},
	{MENU_VIDEO_ENGINE_ACCUR,"正確なソ\フトウェア描画",0},
	{MENU_VIDEO_ENGINE_ACCEL,"PSPアクセラレータ",0},
	{MENU_VIDEO_ENGINE_ACCEL_ACCUR,"PSPｱｸｾﾗﾚｰﾀ+正確なｿﾌﾄｳｪｱ描画",0},
	{MENU_VIDEO_ENGINE_ACCEL_APPROX,"PSPｱｸｾﾗﾚｰﾀ+ほぼ正確なｿﾌﾄｳｪｱ描画",0},
	{MENU_VIDEO_SLIMITVALUE_AUTO,"自動 (PAL/NTSCを判別する)",0},
	{MENU_VIDEO_FSKIP_MODE_AUTO,"%d以下で自動的に変更",0},
	{MENU_VIDEO_FSKIP_CHANGEAUTO_AUTO,"   設定値以下で自動的に変更",0},
	{MENU_VIDEO_FSKIP_CHANGEAUTO_FIXED,"   固定",0},

	{MENU_STATUS_VIDEO_SCRCALIB,"十\字ボタン : 画面位置  " SJIS_STAR "  アナログスティック : 画面サイズ  " SJIS_STAR "  " SJIS_TRIANGLE " 標準  " SJIS_STAR "  " MSG_BTN_NEGATIVE " 終了",0},

	// SOUND OPTIONS
	{MENU_ICONS_SOUND,"音声",0},
	{MENU_ICONS_SOUND_MODE,"サウンドモード",0},
	{MENU_ICONS_SOUND_MODE_HELP,"エミュレートしないのが最速ですが\nいくつかのゲームは動作しません。",0},
	{MENU_ICONS_SOUND_FREQ,"出力周波数",0},
	{MENU_ICONS_SOUND_FREQ_HELP,"低いほうが速くなります。\n高いほうが音質がよくなります。",0},
	{MENU_ICONS_SOUND_APURATIO,"APU周期比",0},
	{MENU_ICONS_SOUND_APURATIO_HELP,"それが何をするか知っている\n高度なユーザー向けです。",0},
	// MISC OPTIONS
	{MENU_ICONS_MISC,"その他",0},
	{MENU_ICONS_MISC_FILEVIEWER,"ファイル表\示",0},
	{MENU_ICONS_MISC_FILEVIEWER_HELP,"簡単なファイルビューワーです。",0},
	{MENU_ICONS_MISC_PSPCLOCK,"PSPクロック速度",0},	
	{MENU_ICONS_MISC_PSPCLOCK_HELP,"222MHz以上はオーバークロックでは\nありません。\n安全ですが電池を速く消費します。",0},
	{MENU_ICONS_MISC_HACKDEBUG,"ハック/デバッグメニュー",0},
	{MENU_ICONS_MISC_HACKDEBUG_HELP,"高度なユーザー向けです。\n描画を調整してスピードを\n得るのに役立ちます。",0},
	{MENU_ICONS_MISC_SNAPSHOT,"スナップショット",0},
	{MENU_ICONS_MISC_SNAPSHOT_HELP,"現在のゲームの画面の\nスナップショットを撮ります。\nファイル一覧で使われます。",0},
	{MENU_ICONS_MISC_SWAPBG,"背景切り替え",0},
	{MENU_ICONS_MISC_SWAPBG_HELP,"DATA/logo.zipから\n背景を表\示します。",0},
	{MENU_ICONS_MISC_BGMUSIC,"メニューBGM",0},
	{MENU_ICONS_MISC_BGMUSIC_HELP,"メニューBGMはPSPを300MHzで\n動作させる必要があります。\n音楽はDATA/music.zipから再生します。\nSPCフォーマットのみです。",0},
	{MENU_ICONS_MISC_BGFX,"メニューバックグラウンドCG",0},
	{MENU_ICONS_MISC_BGFX_HELP,"PS2DEV spline sampleの素敵なCGです。",0},
	{MENU_ICONS_MISC_PADBEEP,"パッドビープ",0},
	{MENU_ICONS_MISC_PADBEEP_HELP,"ビープ音がメニューやファイル一覧を\n移動しているときに鳴ります。",0},
	{MENU_ICONS_MISC_AUTOSTART,"最後に起動したROMを自動起動する",0},
	{MENU_ICONS_MISC_AUTOSTART_HELP,"「はい」にすると、エミュレーターを\n起動したときに自動的に\n最後に起動したROMが起動します。",0},
	{MENU_ICONS_MISC_OSK,"OSK",0},
	{MENU_ICONS_MISC_OSK_HELP,"OSKとは画面上のキーボードのことです。\nDanzeff OSKはアルファベットをPSPで\n入力する際に最適です。\nSCE公式OSKは日本語やePSPで\n入力する際に最適です。",0},

	{MENU_MUSIC_SWAPBG_NODATA,"データがありません",0},
	{MENU_MISC_SWAPBG_RAND,"   ランダム",0},

	{MENU_MISC_OSK_DANZEFF,"Danzeff",0},
	{MENU_MISC_OSK_OFFICIAL,"公式",0},

	{MENU_STATUS_MISC_HACKDEBUG,MSG_BTN_NEGATIVE " メインメニュー   ",0},
	{MENU_STATUS_MISC_HACKDEBUG_FUNC,MSG_BTN_POSITIVE " OK     " SJIS_STAR "    " MSG_BTN_NEGATIVE " メインメニュー   ",0},
	{MENU_STATUS_MISC_HACKDEBUG_0,SJIS_STAR "    " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更"},
	{MENU_STATUS_MISC_HACKDEBUG_HELP_0,SJIS_STAR "      " SJIS_TRIANGLE " ヘルプ       " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更"},
	{MENU_STATUS_MISC_HACKDEBUG_HELP_1,SJIS_STAR "     " SJIS_TRIANGLE "   ヘルプ      " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更"},
	{MENU_STATUS_MISC_HACKDEBUG_HELP_2,SJIS_STAR "    " SJIS_TRIANGLE "     ヘルプ     " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更"},
	{MENU_STATUS_MISC_HACKDEBUG_HELP_3,SJIS_STAR "   " SJIS_TRIANGLE "       ヘルプ    " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更"},

	{MENU_TITLE_MISC_HACKDEBUG,"[" EMUNAME_VERSION "] - メニュー",0},

	// CHEATS
	{MENU_ICONS_CHEATS,"チート",0},
	{MENU_ICONS_CHEATS_ADDRAW,"RAWコード追加",0},
	{MENU_ICONS_CHEATS_ADDRAW_HELP,"RAWコードを追加します。",0},
	{MENU_ICONS_CHEATS_ADDGG,"Game Genieコード追加",0},
	{MENU_ICONS_CHEATS_ADDGG_HELP,"Game Genieコードを追加します。",0},
	{MENU_ICONS_CHEATS_ADDPAR,"プロアクションリプレイコード追加",0},
	{MENU_ICONS_CHEATS_ADDPAR_HELP,"プロアクションリプレイコードを\n追加します。",0},
	{MENU_ICONS_CHEATS_ADDGF,"GoldFingerコード追加",0},
	{MENU_ICONS_CHEATS_ADDGF_HELP,"GoldFingerコードを追加します。",0},
	{MENU_ICONS_CHEATS_DISABLE,"コード無効化",0},
	{MENU_ICONS_CHEATS_DISABLE_HELP,"コードを無効化します。",0},
	{MENU_ICONS_CHEATS_DISABLEALL,"全コード無効化",0},
	{MENU_ICONS_CHEATS_DISABLEALL_HELP,"全てのコードを無効化します。",0},
	{MENU_ICONS_CHEATS_ENABLE,"コード有効化",0},
	{MENU_ICONS_CHEATS_ENABLE_HELP,"コードを有効にします。",0},
	{MENU_ICONS_CHEATS_ENABLEALL,"全コード有効化",0},
	{MENU_ICONS_CHEATS_ENABLEALL_HELP,"全てのコードを有効化します。",0},
	{MENU_ICONS_CHEATS_REMOVE,"コード削除",0},
	{MENU_ICONS_CHEATS_REMOVE_HELP,"コードを削除します。",0},
	{MENU_ICONS_CHEATS_REMOVEALL,"全コード削除",0},
	{MENU_ICONS_CHEATS_REMOVEALL_HELP,"全てのコードを削除します。",0},
	// ABOUT
	{MENU_ICONS_ABOUT,"このソ\フトについて",0},
	{MENU_ICONS_ABOUT_CREDITS,"クレジット",0},
	{MENU_ICONS_ABOUT_VERSION,"バージョン情報",0},

	{MENU_ABOUT_VERSION_TIMESTAMP,"ビルド日時 : %s",0},
	{MENU_ABOUT_VERSION_GCCVER,"使用したGCC : %s",0},

	//Others
	// Adhoc
	{ADHOC_CONNECTING,"接続しています...\n",0},
	{ADHOC_SELECTORRETURN,"接続するサーバーを選ぶか," SJIS_TRIANGLE "ボタンで戻ってください。",0},
	{ADHOC_WAITING,"%sが接続を承認するのを待っています。\nキャンセルするには" MSG_BTN_POSITIVE "ボタンを押してください。\n",0},
	{ADHOC_REQUESTED,"%sが接続を要求しています。\n接続を承認するには" MSG_BTN_NEGATIVE "ボタンを,キャンセルするには" MSG_BTN_POSITIVE "ボタンを押してください。\n",0},
	{ADHOC_CONNECTED,"接続しました",0},
	{ADHOC_STATE,"  接続状況 %d/1\n",0},
	{ADHOC_UNKNOWNCOMMAND,"不明なコマンド %02X です。",0},
	{ADHOC_SYNCLOST_SERVER,"サーバー同期失敗 : 再同期します",20},
	{ADHOC_SYNCLOST_CLIENT,"クライアント同期失敗 : 再同期します",20},
	{ADHOC_CLOSING,"接続を閉じています",60},
	{ADHOC_DRIVERLOAD_ERR,"ネットドライバ読み込みエラー",60*2},
	{ADHOC_INIT_ERR,"アドホックゲームの初期化に問題が発生しました\n",60},
	{ADHOC_LOST_CONNECTION,"接続を失いました",60},
	{ADHOC_CORRUPTED_PKT,"破損したパケット",0},
	{ADHOC_STATE_SIZE,"ステートデータサイズ : %d",0},
	{ADHOC_FILE_ERR_RECEIVING,"ステートファイル受信エラー",60*1},
	{ADHOC_FILE_ERR_SENDING,"ステートファイル送信エラー",60*1},
	{ADHOC_STILL,"残り : %d",0},
	{ADHOC_WAITING_OTHER,"他のプレイヤーを待っています\n\n接続を閉じ通信対戦をやめるには" SJIS_TRIANGLE "ボタンを押してください\n",10},
	{ADHOC_NETWORKERR_1,"ネットワークエラー1",60*1},
	{ADHOC_CANNOTFIND,"ステートセーブが見つかりません",60},
	{ADHOC_FLUSHING,"ネットワークを消去しています。しばらくお待ちください。",10},
	// LOAD/SAVE
	{LOADSAVE_AUTOSAVETIMER,"自動セーブしています...",0},
	{LOADSAVE_EXPORTS9XSTATE,"snes9xTYLファイルが見つかりました",0},
	// VIDEO
	{VIDEO_ENGINE_APPROX,"単一モード : ほぼ正確なソ\フトウェア描画",30},
	{VIDEO_ENGINE_ACCUR,"単一モード : 正確なソ\フトウェア描画",30},
	{VIDEO_ENGINE_ACCEL,"単一モード : PSPアクセラレータ",30},
	{VIDEO_ENGINE_ACCEL_ACCUR,"複合モード : PSPアクセラレータ + 正確なソ\フトウェア描画",30},
	{VIDEO_ENGINE_ACCEL_APPROX,"複合モード : PSPアクセラレータ + ほぼ正確なソ\フトウェア描画",30},
	{VIDEO_FSKIP_AUTO,"フレームスキップ : 自動",0},
	{VIDEO_FSKIP_MANUAL,"フレームスキップ : %d",0},
	// Scroll Message
	{SCROLL_TITLE,"   ,    移動   -  ,  高速         ",0},
	{SCROLL_STATUS_0,"%d/%d行目  -  %d/%dページ",0},
	{SCROLL_STATUS_1,"   終了,        ヘルプ",0},
	{SCROLL_HELP,"Snes9xTYL - ファイルビューワー\n\n" SJIS_TRIANGLE "ボタンで検索し,その後" MSG_BTN_POSITIVE "ボタンで次を検索," SJIS_SQUARE "ボタンで前を検索します。\n" \
			SJIS_UP "," SJIS_DOWN "ボタンで文章をスクロールし, L,Rボタンでより早くスクロールします。\n" MSG_BTN_NEGATIVE "で終了します。\n\n最後の位置は同じファイルが開かれれば維持されます。\nしかしほかのファイルを開くと元に戻ります。\n\n" \
			MSG_BTN_NEGATIVE "ボタンを押してください",0},
	{SCROLL_SEARCHING,"検索しています...",0},
	{SCROLL_STRNOTFOUND,"文字列は見つかりませんでした",30},
	{SCROLL_DISCLAIMER,"免責事項",0},
	{SCROLL_OSK_DESC,"検索する文字列を入力してください",0},
	//Settings
	{SETTINGS_NOTCOMPLETE,"※設定ファイルが完全ではありません※\n\n以前のバージョンのものの可能\性があります。\n\n新しい設定が標準値で設定されます。",60*3},
	{SETTINGS_NOTFOUND,"設定が見つかりません。標準設定を使います。",10},
	{SETTINGS_FORCING_DEFAULT,"強制的に標準設定を使用しています",10},
	{SETTINGS_IGNORED,"以前の互換性のないバージョンのINIファイルです。ファイルは無視されました。",20},
	{SETTINGS_FOUND,"設定が見つかりました\n\n""%s""",0},
	{SETTINGS_DIFF,"異なるバージョンのINIファイルです。",20},
	//Others
	{APU_DEBUGGING,"APUデバッグ",10},
	{BGMUSIC_PLAYING,"SPCファイルを再生しています...",0},
	{FOUND_SRAM,"SRAMファイルが見つかりました",60},
	{CANCEL,"キャンセル",0},
	{INPUTBOX_OK,"\n\n      閉じる",0},
#ifdef ME_SOUND
	{BAT_ISLOW,"電池残量が低下しているため、現在SRAMや設定の保存、ステートセーブが無効にされています。\n\nこの画面は3秒で閉じます。",60*3},
#else
	{BAT_ISLOW,"電池残量が低下しているため、現在SRAMや設定の保存、ステートセーブが無効にされています。\n\nPSPをスリープモードにしてあとで充電することができます。\n\nこの画面は3秒で閉じます。",60*3},
#endif
	{CONV_DONE,"変換完了",30},
	{ROM_LIED,"ROMから間違ったタイプの情報を取得しました。再試行します。",30},
	{LOADING_ROM,"%dKo読み込んでいます...",0},
	{TYPE,"タイプ",0},
	{MODE,"モード",0},
	{COMPANY,"制作会社",0},
#ifdef _BSX_151_
	{SRAM_NOTFOUND,"SRAMファイルが見つかりませんでした。BS-X.srmが代わりに読み込まれます。",30},
	{SRAM_BSX_NOTFOUND,"SRAMファイルが見つからず、BS-X.srmも見つかりませんでした。",30},
#endif
	{CORRUPT,"不明",0},
	{ROM_ONLY,"ROMのみ",0},
	{EXTENDING,"適用範囲を拡大しています",30},
	{EXTENDING_TARGET,"適用範囲を%dから%dに拡大しています\n",0},
	{INIT_OSK,"OSKを初期化しています...",0}
};

////////////////////////////////////////////////////////////////////////////////////////
// psp_msg_string
// -------------- 
//		input : message id
//		output : string
//		comments : return the asked string in current language
////////////////////////////////////////////////////////////////////////////////////////
char *psp_msg_string(int num) {
	char *p;
	char *src = (char *)"unknown string";
	int msg_num=0;
	if (os9x_language == PSP_SYSTEMPARAM_LANGUAGE_JAPANESE)
		while (msg_num<MSG_TOTAL_ENTRIES) {
			if (num==s9xTYL_msg_ja[msg_num].msg_id) src = (char *)s9xTYL_msg_ja[msg_num].msg;
			msg_num++;
		}
	else
		while (msg_num<MSG_TOTAL_ENTRIES) {
			if (num==s9xTYL_msg_en[msg_num].msg_id) src = (char *)s9xTYL_msg_en[msg_num].msg;
			msg_num++;
		}

	for (p = src; *p; p++)
		switch(*p) {
			case 0x0B:
				*p = os9x_btn_positive_str[1];
				break;
			case 0x0E:
				*p = os9x_btn_negative_str[1];
				break;
		}

	return src;
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
	char *p = (char *)"unknown string";
	int msg_num=0;

	if (os9x_language == PSP_SYSTEMPARAM_LANGUAGE_JAPANESE)
		while (msg_num<MSG_TOTAL_ENTRIES) {
			if (num==s9xTYL_msg_ja[msg_num].msg_id) {
				p = (char *)s9xTYL_msg_ja[msg_num].msg;
				break;
			}
			msg_num++;
		}
	else
		while (msg_num<MSG_TOTAL_ENTRIES) {
			if (num==s9xTYL_msg_en[msg_num].msg_id) {
				p = (char *)s9xTYL_msg_en[msg_num].msg;
				break;
			}
			msg_num++;
		}

	while (*p) {
		switch(*p) {
			case 0x0B:
				*p = os9x_btn_positive_str[1];
				break;
			case 0x0E:
				*p = os9x_btn_negative_str[1];
				break;
			}
		p++;
	}

	if (msg_num==MSG_TOTAL_ENTRIES) return 0;

	if (os9x_language == PSP_SYSTEMPARAM_LANGUAGE_JAPANESE) {
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

