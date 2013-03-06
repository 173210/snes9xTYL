#include "psp.h"

extern int os9x_language;

char *s9xTYL_lang[LANGUAGE_NUMBER]={
	"english"
};

#define MSG_TOTAL_ENTRIES 256
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
	{MENU_STATUS_GENERIC_FREERAM,"Free RAM : %dKo",0},
	{MENU_STATUS_GENERIC_CHANGEMUSIC,"SELECT to change music",0},

	{MENU_TITLE_GENERIC_BAT,"%02d%c%02d Bat.:%s%s%s%02d%%%s Tmp.%dC",0},
	{MENU_TITLE_GENERIC_BAT_TIME,"(%02dh%02dm)",0},
	{MENU_TITLE_GENERIC_BAT_PLG,"Plg.",0},
	{MENU_TITLE_GENERIC_BAT_CHRG,"Chrg.",0},
	{MENU_TITLE_GENERIC_BAT_LOW,"Low!",0},

	{MENU_CHANGE_VALUE,"  ,   change value",0},
	{MENU_CHANGE_VALUE,"  ,   change value -  ,  fast",0},
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
	{MENU_GAME_SAVINGDEFAULTSETTINGS,"Saving current settings as default profile",30},
	{MENU_GAME_CONFIRMRESET,"Reset Snes ?",-2},
	{MENU_MISC_SAVINGJPEG,"Saving JPEG snapshot...",10},
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

		{MENU_STATUS_CONTROLS_INPUT,SJIS_CIRCLE " Detect mode   " SJIS_STAR "  " SJIS_CROSS " Menu  ",0},
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
		{MENU_VIDEO_FSKIP_AUTO,"AUTO",0},

		{MENU_STATUS_VIDEO_SCRCALIB,"PAD : SCREEN POSITION  " SJIS_STAR "  ANALOG STICK : SCREEN SIZE  " SJIS_STAR "  " SJIS_TRIANGLE " default  " SJIS_STAR "  " SJIS_CROSS " Exit",0},

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

		{MENU_STATUS_MISC_HACKDEBUG,SJIS_CROSS " Main Menu   ",0},
		{MENU_STATUS_MISC_HACKDEBUG_FUNC,SJIS_CIRCLE " OK     " SJIS_STAR "    " SJIS_CROSS " Main Menu   ",0},
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
		{MENU_ABOUT_VERSION_GCCVER,"With GCC %s",0}
};

t_err_entry s9xTYL_msg_ja[MSG_TOTAL_ENTRIES]= {
//japanese
		//error
		{ERR_OUT_OF_MEM,"メモリ外です。",60},
		{ERR_READ_MEMSTICK,"メモリースティックの読み込みでエラーが発生しました。\nメモリースティックを確認してください。\n(PCのカードリーダーを使ってください)\nメモリースティックが破損している可能性があります。",60},
		//confirm
  	{ASK_EXIT,"Snes9XTYLを終了しますか?",-2},
  	{ASK_DELETE,"ファイルを削除しますか?",-2},
  	//info
  	{INFO_USB_ON,"USBが有効化されました",30},
  	{INFO_USB_OFF,"USBが無効化されました",30},
  	{INFO_DELETING,"削除しています...",0},
  	{INFO_EXITING,"終了しています。しばらくお待ちください...",0},
  	//filer
  	{FILER_STATUS_CANEXIT1,SJIS_CIRCLE " 実行 " SJIS_SQUARE " 標準設定で実行  " SJIS_STAR "  " SJIS_CROSS " ゲームに戻る  "  SJIS_STAR "  " SJIS_TRIANGLE " 上へ  "  SJIS_STAR "  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT " 移動",0},
  	{FILER_STATUS_NOEXIT1,SJIS_CIRCLE " 実行 " SJIS_SQUARE " 標準設定で実行  "  SJIS_STAR "  " SJIS_TRIANGLE " 上へ          " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT " リストを移動",0},
  	{FILER_STATUS_CANEXIT2,SJIS_CIRCLE " 実行 " SJIS_CROSS "ゲームに戻る " SJIS_TRIANGLE " 上へ  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT " リストを移動",0},
  	{FILER_STATUS_NOEXIT2,SJIS_CIRCLE " 実行 " SJIS_TRIANGLE " 上へ                  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT " リストを移動",0},
  	
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
  	
  	{FILE_IPS_PATCHSUCCESS,"IPSパッチの適用に成功しました",0},
  	
  	{MENU_STATUS_GENERIC_MSG1,"戻るには" SJIS_CROSS "を押してください",0},
  	{MENU_STATUS_GENERIC_NEEDRELOAD,"変更を適用するには再読み込みが必要かもしれません",0},
  	{MENU_STATUS_GENERIC_NEEDRESET,"変更を適用するにはリセットが必要かもしれません",0},
	{MENU_STATUS_GENERIC_FREERAM,"空きメモリ : %dKo",0},
	{MENU_STATUS_GENERIC_CHANGEMUSIC,"音楽を変えるにはSELECTボタンを押してください",0},

	{MENU_TITLE_GENERIC_BAT,"%02d%c%02d 電池:%s%s%s残り%02d%%%s 温度.%d℃",0},
	{MENU_TITLE_GENERIC_BAT_TIME,"(%02d時間%02d分)",0},
	{MENU_TITLE_GENERIC_BAT_PLG,"AC.",0},
	{MENU_TITLE_GENERIC_BAT_CHRG,"充電中.",0},
	{MENU_TITLE_GENERIC_BAT_LOW,"残量低下.",0},

	{MENU_CHANGE_VALUE,"  ,   値を変更",0},
	{MENU_CHANGE_VALUE,"  ,   値を変更   -    ,  高速",0},
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
	{MENU_CHEATS_CONFIRMREMALL,"すべてのコードを削除しますか?",-2},

  	{MENU_STATE_CONFIRMLOAD,"現在のゲーム進行が失われます。\n続行しますか?",-2},
	{MENU_STATE_CONFIRMDELETE,"このステートセーブを削除しますか?",-2},
	{MENU_STATE_CONFIRMSAVE,"このステートセーブに上書きしますか?",-2},
	{MENU_STATE_ISSAVING,"ステートセーブしています...",10},
	{MENU_STATE_ISLOADING,"状態を読み込んでいます...",10},
	{MENU_STATE_ISDELETING,"削除しています...",10},
	{MENU_STATE_ISIMPORTING,"ステートセーブをインポートしています...",10},
	{MENU_STATE_ISEXPORTINGS9X,"SNES9Xステートセーブをエクスポートしています...",10},
	{MENU_STATE_NOSTATE,"使用可能なステートセーブがありません",10},
	{MENU_STATE_AUTOSAVETIMER,"%d分毎",0},
	{MENU_STATE_AUTOSAVETIMER_OFF,"オフ",0},
	{MENU_GAME_SAVINGDEFAULTSETTINGS,"現在の設定を標準設定として保存しています",30},
	{MENU_GAME_CONFIRMRESET,"Snesをリセットしますか?",-2},
	{MENU_MISC_SAVINGJPEG,"JPEGスナップショットを保存しています...",10},
	{MENU_MISC_BGMUSIC_UNKNOWN,"不明",0},
	{MENU_MISC_BGMUSIC_GAMETITLE,"ゲーム : ",0},
	{MENU_MISC_BGMUSIC_TITLE,"題名 : ",0},
	{MENU_MISC_BGMUSIC_AUTHOR,"作者 : ",0},


  	// GAME
  	{MENU_ICONS_GAME,"ゲーム",0},
  	{MENU_ICONS_GAME_RESET,"SNESをリセットする",0},
  	{MENU_ICONS_GAME_NEW,"新しいゲームを読み込む",0},
	{MENU_ICONS_GAME_DEFAULTSETTINGS,"標準設定を作成する",0},
	{MENU_ICONS_GAME_DEFAULTSETTINGS_HELP,"現在の設定を特別に設定ファイルがない\n全てのROMに使われる標準設定として\n保存します。",0},
	{MENU_ICONS_GAME_EXIT,"Snes9xTYLを終了する",0},
	// LOAD/SAVE
	{MENU_ICONS_LOADSAVE,"ロード/セーブ",0},
	{MENU_ICONS_LOADSAVE_LOADSTATE,"ステートロード",0},
	{MENU_ICONS_LOADSAVE_LOADSTATE_HELP,"以前保存したステートセーブを\n読み込みます。ステートセーブはSnes\nの状態のスナップショットのような\nもので、すべて保存されます。",0},
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
	{MENU_ICONS_LOADSAVE_AUTOSRAM,"SRAMが更新されたときにオートセーブする",0},
	{MENU_ICONS_LOADSAVE_AUTOSRAM_HELP,"変更が行われるたびにSRAMが\n保存されるようにします。",0},
	// CONTROLS
	{MENU_ICONS_CONTROLS,"コントローラ",0},
	{MENU_ICONS_CONTROLS_REDEFINE,"再設定",0},
	{MENU_ICONS_CONTROLS_REDEFINE_HELP,"ボタン割り当てを再設定します。",0},
	{MENU_ICONS_CONTROLS_INPUT,"エミュレートされるコントローラ",0},
	{MENU_ICONS_CONTROLS_INPUT_HELP,"現在有効にするSnesコントローラを\n選んでください。",0},

	{MENU_CONTROLS_INPUT,"コントローラ %d",0},
	{MENU_CONTROLS_INPUT_PRESS,"%sに割り当てるボタンを押してください",0},
	{MENU_CONTROLS_INPUT_NOFORMENU,"メニューにアクセスするために設定されたボタンありません。\nボタンを選んでください。",30},
#ifdef HOME_HOOK
	{MENU_CONTROLS_INPUT_DEFAULT,"標準プロファイルを選択してください\n\n"\
				SJIS_CROSS " - 標準, SNESの十\字ボタンがPSPの十\字ボタンに割り当てられます。\n\n"\
				SJIS_CIRCLE " - 標準, SNESの十\字ボタンがPSPのアナログスティックに割り当てられます。\n\n"\
				SJIS_SQUARE " - 標準, SNESの十\字ボタンがPSPの十\字ボタンとアナログスティックに割り当てられます。\n\n"\
				SJIS_TRIANGLE " キャンセル\n",0},
#else
	{MENU_CONTROLS_INPUT_DEFAULT,"標準プロファイルを選択してください\n\n"\
				SJIS_CROSS " - 標準, SNESの十\字ボタンがPSPの十\字ボタンに割り当てられます。\nメニューはアナログスティック左に割り当てます。\n\n"\
				SJIS_CIRCLE " - 標準, SNESの十\字ボタンがPSPのアナログスティックに割り当てられます。\nメニューは十\字ボタン左に割り当てます。\n\n"\
				SJIS_SQUARE " - Default, SNESの十\字ボタンがPSPの十\字ボタンとアナログスティックに割り当てられます。\nメニューはL+Rトリガーに割り当てます。"\
				SJIS_TRIANGLE " キャンセル\n",0},
#endif

	{MENU_STATUS_CONTROLS_INPUT,SJIS_CIRCLE " 検出モード   " SJIS_STAR "  " SJIS_CROSS " メニュー  ",0},
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
	{MENU_ICONS_VIDEO_PALASNTSC_HELP,"PALのゲームを240本の垂直解像度の\n代わりに224本にしてNTSCのものように\nエミュレートするよう強制します。",0},		
	{MENU_ICONS_VIDEO_GAMMA,"ガンマ補正",0},
	{MENU_ICONS_VIDEO_GAMMA_HELP,"より明るめな描画をさせます。",0},
	{MENU_ICONS_VIDEO_SCRCALIB,"画面位置補正",0},
	{MENU_ICONS_VIDEO_SCRCALIB_HELP,"邪魔な黒い枠を取り除くのに\n役立ちます。",0},
	{MENU_ICONS_VIDEO_SHOWFPS,"FPS表\示",0},
	{MENU_ICONS_VIDEO_SHOWFPS_HELP,"1行目はエミュレートされる\nフレーム数を表\示します。\n二行目は本当のFPSを表\示します。",0},

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
	{MENU_VIDEO_FSKIP_AUTO,"自動",0},

	{MENU_STATUS_VIDEO_SCRCALIB,"十\字ボタン : 画面位置  " SJIS_STAR "  アナログスティック : 画面サイズ  " SJIS_STAR "  " SJIS_TRIANGLE " 標準  " SJIS_STAR "  " SJIS_CROSS " 終了",0},

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
	{MENU_ICONS_MISC_RANDBG,"ランダムバックグラウンド",0},
	{MENU_ICONS_MISC_RANDBG_HELP,"DATA/logo.zipから\nランダムに背景を表\示します。",0},
	{MENU_ICONS_MISC_BGMUSIC,"メニューBGM",0},
	{MENU_ICONS_MISC_BGMUSIC_HELP,"メニューBGMはPSPを300MHzで\n動作させる必要があります。\n音楽はDATA/music.zipから再生します。\nSPCフォーマットのみです。",0},
	{MENU_ICONS_MISC_BGFX,"メニューバックグラウンドCG",0},
	{MENU_ICONS_MISC_BGFX_HELP,"PS2DEV spline sampleの素敵なCGです。",0},
	{MENU_ICONS_MISC_PADBEEP,"パッドビープ",0},
	{MENU_ICONS_MISC_PADBEEP_HELP,"ビープ音がメニューやファイル一覧を\n移動しているときに鳴ります。",0},

	{MENU_STATUS_MISC_HACKDEBUG,SJIS_CROSS " メインメニュー   ",0},
	{MENU_STATUS_MISC_HACKDEBUG_FUNC,SJIS_CIRCLE " OK     " SJIS_STAR "    " SJIS_CROSS " メインメニュー   ",0},
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
	{MENU_ABOUT_VERSION_GCCVER,"使用したGCC : %s",0}
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

