#include "psp.h"

extern int os9x_language;

const char *s9xTYL_msg_ml[MSG_TOTAL_LANG][MSG_TOTAL_ENTRIES] = {
	[MSG_EN] = {
	//english
		//error
		[ERR_OUT_OF_MEM] = "Out of memory",
		[ERR_READ_MEMSTICK] = "Error reading memstick.\nPlease verify it (using a PC card reader), it may be corrupted.",

		[ERR_INIT_SNES] = "Cannot init snes, memory issue",
		[ERR_LOADING_ROM] = "Error while loading rom",
		[ERR_INIT_GFX] = "Cannot initialize graphics",

		[ERR_CANNOT_ALLOC_MEM] = "Cannot allocate memory",

		[ERR_USB_STARTING_USBBUS] = "Error starting USB Bus driver (0x%08X)\n",
		[ERR_USB_STARTING_USBMASS] = "Error starting USB Mass Storage driver (0x%08X)\n",
		[ERR_USB_SETTING_CAPACITY] = "Error setting capacity with USB Mass Storage driver (0x%08X)\n",
		//confirm
		[ASK_EXIT] = "Exit Snes9XTYL ?",
		[ASK_DELETE] = "Delete file ?",
		[ASK_SAVEDIR] = "The save directory is not found.\n"
			"To make the directory in the directory where this emulator is, Press " SJIS_CIRCLE  "\n"
			"To make the directory in SAVEDATA directory, Press " SJIS_CROSS "\n",
		//info
		[INFO_USB_ON] = "USB activated",
		[INFO_USB_OFF] = "USB disabled",
		[INFO_DELETING] = "Deleting...",
		[INFO_EXITING] = "Exiting, please wait...",
		//filer
		[FILER_STATUS_CANEXIT1] = "%s RUN " SJIS_SQUARE " RUN default settings  " SJIS_STAR "  %s TO GAME  "  SJIS_STAR "  " SJIS_TRIANGLE " %s  "  SJIS_STAR "  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT ", to browse",
		[FILER_STATUS_NOEXIT1] = "%s RUN " SJIS_SQUARE " RUN default settings  "  SJIS_STAR "  " SJIS_TRIANGLE " %s          " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT ", to browse list",
		[FILER_STATUS_CANEXIT2] = "%s RUN %s BACK TO GAME " SJIS_TRIANGLE " %s  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT ", to browse list",
		[FILER_STATUS_NOEXIT2] = "%s RUN " SJIS_TRIANGLE " %s                  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT ", to browse list",

		[FILER_STATUS_PARDIR] = "Parent dir.",

		[FILER_TITLE] = "[" EMUNAME_VERSION "] - Choose a file",

		[FILER_HELP_WINDOW1] = "Press START to switch between",
		[FILER_HELP_WINDOW2] = "NORMAL and NETPLAY mode.",
#ifdef FW3X
		[FILER_HELP_WINDOW3] = " ",
#ifdef HOME_HOOK
		[FILER_HELP_WINDOW4] = "Press HOME to exit.",
#else
		[FILER_HELP_WINDOW4] = "Press L+R to exit.",
#endif
#else
#ifdef HOME_HOOK
		[FILER_HELP_WINDOW3] = "Press HOME to exit.",
#else
		[FILER_HELP_WINDOW3] = "Press L+R to exit.",
#endif
		[FILER_HELP_WINDOW4 = "Press R to switch USB on/off.",
#endif


		[FILER_HELP_WINDOW5] = "Press SELECT to delete file.",

		[FILE_IPS_APPLYING] = "Found IPS patch : %s\nSize is : %dKo\nApplying ....",
		[FILE_IPS_PATCHSUCCESS] = "IPS Patch applied successfully",

		[MENU_STATUS_GENERIC_MSG1] = "Press %s to return",
		[MENU_STATUS_GENERIC_NEEDRELOAD] = "Changes may require a RELOAD to take effect",
		[MENU_STATUS_GENERIC_NEEDRESET] = "Changes may require a RESET to take effect",
		[MENU_STATUS_GENERIC_FREERAM] = "Free RAM : %dKo",
		[MENU_STATUS_GENERIC_CHANGEMUSIC] = "SELECT to change music",

		[MENU_TITLE_GENERIC_BAT] = "%02d%c%02d Bat.:%s%s%s%02d%%%s Tmp.%d℃",
		[MENU_TITLE_GENERIC_BAT_TIME] = "(%02dh%02dm)",
		[MENU_TITLE_GENERIC_BAT_PLG] = "Plg.",
		[MENU_TITLE_GENERIC_BAT_CHRG] = "Chrg.",
		[MENU_TITLE_GENERIC_BAT_LOW] = "Low!",

		[MENU_CHANGE_VALUE] = "  ,   change value",
		[MENU_CHANGE_VALUE_WITH_FAST] = "  ,   change value -  ,  fast",
		[MENU_CANCEL_VALIDATE] = "  ,   cancel   -      validate",
		[MENU_DEFAULT_VALUE] = "   default value",
		[MENU_YES] = "yes",
		[MENU_NO] = "no",
		[MENU_NOT_IMPLEMENTED] = "Not yet implemented.\n\n",

		[MENU_SOUND_MODE_NOTEMUL] = "not emulated",
		[MENU_SOUND_MODE_EMULOFF] = "emulated, no output",
		[MENU_SOUND_MODE_EMULON] = "emulated, output",

		[MENU_STATE_CHOOSEDEL] = "Choose a slot to DELETE",
		[MENU_STATE_CHOOSELOAD] = "Choose a slot to LOAD",
		[MENU_STATE_CHOOSESAVE] = "Choose a slot to SAVE",
		[MENU_STATE_FREESLOT] = "FREE",
		[MENU_STATE_SCANNING] = "Scanning used slots...",


		[ERR_INIT_OSK] = "cannot init OSK",
		[ERR_ADD_CODE] = "Cannot add cheat code, too many already enterred!",

		[MENU_CHEATS_ENTERGF] = "Enter a GoldFinger code",
		[MENU_CHEATS_ENTERPAR] = "Enter a Pro Action Replay code",
		[MENU_CHEATS_ENTERGG] = "Enter a Game Genie code",
		[MENU_CHEATS_ENTERRAW] = "Enter a RAW code\nFormat is ADDRESS - NEW VALUE",
		[MENU_CHEATS_CONFIRMREMALL] = "Remove all codes ?",
		[MENU_CHEATS_ENTERNAME] = "Enter Name",
		[MENU_CHEATS_PREVPAGE] = "L - Prev. Page",
		[MENU_CHEATS_NEXTPAGE] = "R - Next Page",

		[MENU_STATE_CONFIRMLOAD] = "Current game progress will be lost.\nContinue ?",
		[MENU_STATE_CONFIRMDELETE] = "Delete current state ?",
		[MENU_STATE_CONFIRMSAVE] = "Overwrite current savestate ?",
		[MENU_STATE_ISLOADING] = "Loading state...",
		[MENU_STATE_ISSAVING] = "Saving state...",
		[MENU_STATE_ISDELETING] = "Deleting...",
		[MENU_STATE_ISIMPORTING] = "Importing state...",
		[MENU_STATE_ISEXPORTINGS9X] = "Exporting SNES9X state...",
		[MENU_STATE_NOSTATE] = "No state available...",
		[MENU_STATE_AUTOSAVETIMER] = "every %dmin.",
		[MENU_STATE_AUTOSAVETIMER_OFF] = "off",
		[MENU_STATE_WARNING_LOWBAT] = "Battery is low.\nIf PSP turns off while saving, Memory Stick may be broken.\nContinue?",
		[MENU_GAME_SAVINGDEFAULTSETTINGS] = "Saving current settings as default profile",
		[MENU_GAME_CONFIRMRESET] = "Reset Snes ?",
		[MENU_MISC_SAVINGJPEG] = "Saving JPEG snapshot...",

		[MENU_MISC_BGMUSIC_RAND] = "Play randomly",
		[MENU_MISC_BGMUSIC_ORDER] = "Play in order",

		[MENU_MISC_BGMUSIC_UNKNOWN] = "unknown",
		[MENU_MISC_BGMUSIC_GAMETITLE] = "Playing : ",
		[MENU_MISC_BGMUSIC_TITLE] = "Title : ",
		[MENU_MISC_BGMUSIC_AUTHOR] = "Author : ",


		// GAME
		[MENU_ICONS_GAME] = "GAME",
		[MENU_ICONS_GAME_RESET] = "Reset SNES",
		[MENU_ICONS_GAME_NEW] = "Load new game",
		[MENU_ICONS_GAME_DEFAULTSETTINGS] = "Make Settings DEFAULT",
		[MENU_ICONS_GAME_DEFAULTSETTINGS_HELP] = "Make current settings the default\nfor all roms when no specific\none exists",
		[MENU_ICONS_GAME_EXIT] = "Exit Snes9xTYL",
		// LOAD/SAVE
		[MENU_ICONS_LOADSAVE] = "LOAD/SAVE",
		[MENU_ICONS_LOADSAVE_LOADSTATE] = "Load state",
		[MENU_ICONS_LOADSAVE_LOADSTATE_HELP] = "Load a previously saved state.\nSavestate is like a snapshot\nof the snes state, everything\nis saved.",
		[MENU_ICONS_LOADSAVE_SAVESTATE] = "Save state",
		[MENU_ICONS_LOADSAVE_SAVESTATE_HELP] = "Save state",
		[MENU_ICONS_LOADSAVE_DELETESTATE] = "Delete state",
		[MENU_ICONS_LOADSAVE_DELETESTATE_HELP] = "Delete state",
		[MENU_ICONS_LOADSAVE_IMPORTSTATE] = "Import state",
		[MENU_ICONS_LOADSAVE_IMPORTSTATE_HELP] = "Import a state from a previous\nversion or from uosnes9x.",

		[MENU_ICONS_LOADSAVE_EXPORTS9XSTATE] = "Export SNES9X state",
		[MENU_ICONS_LOADSAVE_EXPORTS9XSTATE_HELP] = "Export a state using SNES9X format.",

		[MENU_ICONS_LOADSAVE_AUTOSAVETIMER] = "Autosavestate timer",
		[MENU_ICONS_LOADSAVE_AUTOSAVETIMER_HELP] = "Allows to have an auto\nsavestate performed regularly.",
		[MENU_ICONS_LOADSAVE_AUTOSRAM] = "Autosave updated SRAM",
		[MENU_ICONS_LOADSAVE_AUTOSRAM_HELP] = "Allows to have SRAM saved\nafter each change.",
		// CONTROLS
		[MENU_ICONS_CONTROLS] = "CONTROLS",
		[MENU_ICONS_CONTROLS_REDEFINE] = "Redefine",
		[MENU_ICONS_CONTROLS_REDEFINE_HELP] = "Redefine buttons mapping.",
		[MENU_ICONS_CONTROLS_INPUT] = "Emulated input",
		[MENU_ICONS_CONTROLS_INPUT_HELP] = "Choose current active\nsnes controller.",

		[MENU_CONTROLS_INPUT] = "Joypad #%d",
		[MENU_CONTROLS_INPUT_PRESS] = "Press a button for %s",
		[MENU_CONTROLS_INPUT_NOFORMENU] = "No button defined for MENU Access!\nPlease choose one",
#ifdef HOME_HOOK
		[MENU_CONTROLS_INPUT_DEFAULT] = "Choose a default profile : \n\n"
			SJIS_CROSS " - Default, SNES pad mapped to PSP pad.\n\n"
			SJIS_CIRCLE " - Default, SNES pad mapped to PSP stick.\n\n"
			SJIS_SQUARE " - Default, SNES pad mapped to PSP pad&stick.\n\n"
			SJIS_TRIANGLE " Cancel\n",
#else
		[MENU_CONTROLS_INPUT_DEFAULT] = "Choose a default profile : \n\n"
			SJIS_CROSS " - Default, SNES pad mapped to PSP pad. GUI on stick left.\n\n"
			SJIS_CIRCLE " - Default, SNES pad mapped to PSP stick. GUI on pad left.\n\n"
			SJIS_SQUARE " - Default, SNES pad mapped to PSP pad&stick. GUI on LTrg.+RTrg.\n\n"
			SJIS_TRIANGLE " Cancel\n",
#endif

		[MENU_STATUS_CONTROLS_INPUT] = "%s Detect mode   " SJIS_STAR "  %s Menu  ",
		[MENU_STATUS_CONTROLS_INPUT_0] = SJIS_STAR "  " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value  " SJIS_STAR "  SELECT default profiles",
		[MENU_STATUS_CONTROLS_INPUT_HELP_0] = SJIS_STAR "      " SJIS_TRIANGLE " Help       " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value",
		[MENU_STATUS_CONTROLS_INPUT_HELP_1] = SJIS_STAR "     " SJIS_TRIANGLE "   Help      " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value",
		[MENU_STATUS_CONTROLS_INPUT_HELP_2] = SJIS_STAR "    " SJIS_TRIANGLE "     Help     " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value",
		[MENU_STATUS_CONTROLS_INPUT_HELP_3] = SJIS_STAR "   " SJIS_TRIANGLE "       Help    " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value",

		[MENU_TITLE_CONTROLS_INPUT] = "[" EMUNAME_VERSION "] - Inputs Config",

		// VIDEO OPTIONS
		[MENU_ICONS_VIDEO] = "VIDEO",
		[MENU_ICONS_VIDEO_MODE] = "Video mode",
		[MENU_ICONS_VIDEO_MODE_HELP] = "Change the aspect ratio\n4:3 is adviced",
		[MENU_ICONS_VIDEO_ENGINE] = "Engine",
		[MENU_ICONS_VIDEO_ENGINE_HELP] = "Hardware accelerated mode\nis not always the fastest",
		[MENU_ICONS_VIDEO_SLIMITONOFF] = "Speed limit switch",
		[MENU_ICONS_VIDEO_SLIMITONOFF_HELP] = "Turn on to limit the speed\nof the game.",
		[MENU_ICONS_VIDEO_SLIMITVALUE] = "Speed limit fps",
		[MENU_ICONS_VIDEO_SLIMITVALUE_HELP] = "Only used if speed limit\nhas been turned on.",
		[MENU_ICONS_VIDEO_FSKIP] = "Frameskip",
		[MENU_ICONS_VIDEO_FSKIP_HELP] = "Choose a fixed value if you\nhave some sprites\ndisappearing.",
		[MENU_ICONS_VIDEO_SMOOTHING] = "Smoothing",
		[MENU_ICONS_VIDEO_SMOOTHING_HELP] = "Should be on since it costs\nnothing.",
		[MENU_ICONS_VIDEO_VSYNC] = "VSync",
		[MENU_ICONS_VIDEO_VSYNC_HELP] = "This costs lots of fps\nand should be turned off.",
		[MENU_ICONS_VIDEO_PALASNTSC] = "Render PAL as NTSC",
		[MENU_ICONS_VIDEO_PALASNTSC_HELP] = "Force PAL games to be emulated\nlike NTSC ones : 224 lines\ninstead of 240.",
		[MENU_ICONS_VIDEO_GAMMA] = "Gamma correction",
		[MENU_ICONS_VIDEO_GAMMA_HELP] = "Allows brighter rendering",
		[MENU_ICONS_VIDEO_SCRCALIB] = "Screen calibration",
		[MENU_ICONS_VIDEO_SCRCALIB_HELP] = "Help to get rid of the nasty\nblack borders.",
		[MENU_ICONS_VIDEO_SHOWFPS] = "Show FPS",
		[MENU_ICONS_VIDEO_SHOWFPS_HELP] = "First line is emulated frames\nSecond one is real fps.",

		[MENU_VIDEO_MODE_1_1] = "1:1",
		[MENU_VIDEO_MODE_ZOOM_FIT] = "zoom fit",
		[MENU_VIDEO_MODE_ZOOM_4_3RD] = "zoom 4/3",
		[MENU_VIDEO_MODE_ZOOM_WIDE] = "zoom wide",
		[MENU_VIDEO_MODE_FULLSCREEN] = "fullscreen",
		[MENU_VIDEO_MODE_FULLSCREEN_CLIPPED] = "fullscreen clipped",
		[MENU_VIDEO_ENGINE_APPROX] = "approx. software",
		[MENU_VIDEO_ENGINE_ACCUR] = "accur. software",
		[MENU_VIDEO_ENGINE_ACCEL] = "PSP accelerated",
		[MENU_VIDEO_ENGINE_ACCEL_ACCUR] = "PSP accel. + accur. soft.",
		[MENU_VIDEO_ENGINE_ACCEL_APPROX] = "PSP accel. + approx. soft.",
		[MENU_VIDEO_SLIMITVALUE_AUTO] = "AUTO (detect PAL/NTSC)",
		[MENU_VIDEO_FSKIP_MODE_AUTO] = "auto change below %d",
		[MENU_VIDEO_FSKIP_CHANGEAUTO_AUTO] = "   auto change below set value",
		[MENU_VIDEO_FSKIP_CHANGEAUTO_FIXED] = "   fixed",

		[MENU_STATUS_VIDEO_SCRCALIB] = "PAD : SCREEN POSITION  " SJIS_STAR "  ANALOG STICK : SCREEN SIZE  " SJIS_STAR "  " SJIS_TRIANGLE " default  " SJIS_STAR "  %s Exit",

		// SOUND OPTIONS
		[MENU_ICONS_SOUND] = "SOUND",
		[MENU_ICONS_SOUND_MODE] = "Sound mode",
		[MENU_ICONS_SOUND_MODE_HELP] = "not emulated is the fastest\nbut some games won't run.",
		[MENU_ICONS_SOUND_FREQ] = "Output frequency",
		[MENU_ICONS_SOUND_FREQ_HELP] = "The lower the faster\nThe higher the best sound quality",
		[MENU_ICONS_SOUND_APURATIO] = "APU Cycles ratio",
		[MENU_ICONS_SOUND_APURATIO_HELP] = "For advanced users knowing\nwhat they do.",
		// MISC OPTIONS
		[MENU_ICONS_MISC] = "MISC",
		[MENU_ICONS_MISC_FILEVIEWER] = "View file",
		[MENU_ICONS_MISC_FILEVIEWER_HELP] = "Minimalist file viewer",
		[MENU_ICONS_MISC_PSPCLOCK] = "PSP Clockspeed",
		[MENU_ICONS_MISC_PSPCLOCK_HELP] = "value>222Mhz is NOT overclocking\nit's safe but battery runs out\nfaster",
		[MENU_ICONS_MISC_HACKDEBUG] = "Hack/debug menu",
		[MENU_ICONS_MISC_HACKDEBUG_HELP] = "For advanced users.\nHelp to tune the rendering and\ngain some speed.",
		[MENU_ICONS_MISC_SNAPSHOT] = "Snapshot",
		[MENU_ICONS_MISC_SNAPSHOT_HELP] = "Take a snapshot of the current\ngame screen.\nUsed in file browser.",
		[MENU_ICONS_MISC_SWAPBG] = "Swap BG",
		[MENU_ICONS_MISC_SWAPBG_HELP] = "Swap BG from the DATA/logo.zip\nfile.",
		[MENU_ICONS_MISC_BGMUSIC] = "Menu background music",
		[MENU_ICONS_MISC_BGMUSIC_HELP] = "Menu background music requires\nmake PSP running at 300Mhz\nMusic are from DATA/music.zip file.\nSPC format only.",
		[MENU_ICONS_MISC_BGFX] = "Menu background fx",
		[MENU_ICONS_MISC_BGFX_HELP] = "Nice FX from PS2DEV spline sample.",
		[MENU_ICONS_MISC_PADBEEP] = "Pad beep",
		[MENU_ICONS_MISC_PADBEEP_HELP] = "The beep sounds when browsing\nmenu and files.",
		[MENU_ICONS_MISC_AUTOSTART] = "Last ROM Auto-Start",
		[MENU_ICONS_MISC_AUTOSTART_HELP] = "If you set to [yes],the ROM\nwhich was loaded at last\n will start automatically\nwhen the emulator started.",
		[MENU_ICONS_MISC_OSK] = "OSK",
		[MENU_ICONS_MISC_OSK_HELP] = "OSK is On-Screen Keyboard.\nDanzeff OSK is useful\nfor typing alphabet on PSP\nSCE's official OSK is useful\nfor typing Japanese or on PS Vita.",

		[MENU_MUSIC_SWAPBG_NODATA] = "No Data",
		[MENU_MISC_SWAPBG_RAND] = "   Random",

		[MENU_MISC_OSK_DANZEFF] = "Danzeff",
		[MENU_MISC_OSK_OFFICIAL] = "Official",

		[MENU_STATUS_MISC_HACKDEBUG] = "%s Main Menu   ",
		[MENU_STATUS_MISC_HACKDEBUG_FUNC] = "%s OK     " SJIS_STAR "    %s Main Menu   ",
		[MENU_STATUS_MISC_HACKDEBUG_0] = SJIS_STAR "    " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value",

		[MENU_STATUS_MISC_HACKDEBUG_HELP_0] = SJIS_STAR "      " SJIS_TRIANGLE " Help       " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value",
		[MENU_STATUS_MISC_HACKDEBUG_HELP_1] = SJIS_STAR "     " SJIS_TRIANGLE "   Help      " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value",
		[MENU_STATUS_MISC_HACKDEBUG_HELP_2] = SJIS_STAR "    " SJIS_TRIANGLE "     Help     " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value",
		[MENU_STATUS_MISC_HACKDEBUG_HELP_3] = SJIS_STAR "   " SJIS_TRIANGLE "       Help    " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " Select " SJIS_LEFT "," SJIS_RIGHT " Change value",

		[MENU_TITLE_MISC_HACKDEBUG] = "[" EMUNAME_VERSION "] - Menu",

		// CHEATS
		[MENU_ICONS_CHEATS] = "CHEATS",
		[MENU_ICONS_CHEATS_ADDRAW] = "Add a RAW code",
		[MENU_ICONS_CHEATS_ADDRAW_HELP] = "Add a RAW code",
		[MENU_ICONS_CHEATS_ADDGG] = "Add a Game Genie code",
		[MENU_ICONS_CHEATS_ADDGG_HELP] = "Add a Game Genie code",
		[MENU_ICONS_CHEATS_ADDPAR] = "Add a Pro Action Replay code",
		[MENU_ICONS_CHEATS_ADDPAR_HELP] = "Add a Pro Action Replay code",
		[MENU_ICONS_CHEATS_ADDGF] = "Add a GoldFinger code",
		[MENU_ICONS_CHEATS_ADDGF_HELP] = "Add a GoldFinger code",
		[MENU_ICONS_CHEATS_DISABLE] = "Disable code",
		[MENU_ICONS_CHEATS_DISABLE_HELP] = "Disable code",
		[MENU_ICONS_CHEATS_DISABLEALL] = "Disable all codes",
		[MENU_ICONS_CHEATS_DISABLEALL_HELP] = "Disable all codes",
		[MENU_ICONS_CHEATS_ENABLE] = "Enable code",
		[MENU_ICONS_CHEATS_ENABLE_HELP] = "Enable code",
		[MENU_ICONS_CHEATS_ENABLEALL] = "Enable all codes",
		[MENU_ICONS_CHEATS_ENABLEALL_HELP] = "Enable all codes",
		[MENU_ICONS_CHEATS_REMOVE] = "Remove code",
		[MENU_ICONS_CHEATS_REMOVE_HELP] = "Remove code",
		[MENU_ICONS_CHEATS_REMOVEALL] = "Remove all codes",
		[MENU_ICONS_CHEATS_REMOVEALL_HELP] = "Remove all codes",
		// ABOUT
		[MENU_ICONS_ABOUT] = "ABOUT",
		[MENU_ICONS_ABOUT_CREDITS] = "Credits",
		[MENU_ICONS_ABOUT_VERSION] = "Version infos",

		[MENU_ABOUT_VERSION_TIMESTAMP] = "Build on %s",
		[MENU_ABOUT_VERSION_GCCVER] = "With GCC %s",

		//Others
		// Adhoc
		[ADHOC_CONNECTING] = "Connecting...\n",
		[ADHOC_SELECTORRETURN] = "Select a server to connect to, or triangle to return",
		[ADHOC_WAITING] = "Waiting for %s to accept the connection\nTo cancel press O\n",
		[ADHOC_REQUESTED] = "%s has requested a connection\nTo accept the connection press X, to cancel press O\n",
		[ADHOC_CONNECTED] = "Connected",
		[ADHOC_STATE] = "  connection state %d of 1\n",
		[ADHOC_UNKNOWNCOMMAND] = "Unknown command %02X !",
		[ADHOC_SYNCLOST_SERVER] = "SERVER Lost sync : resync!!!",
		[ADHOC_SYNCLOST_CLIENT] = "CLIENT Lost sync : resync!!!",
		[ADHOC_CLOSING] = "closing connection",
		[ADHOC_DRIVERLOAD_ERR] = "Net driver load error",
		[ADHOC_INIT_ERR] = "Issue with init adhoc game\n",
		[ADHOC_LOST_CONNECTION] = "Lost connection!",
		[ADHOC_CORRUPTED_PKT] = "corrupted pkt",
		[ADHOC_STATE_SIZE] = "state size : %d",
		[ADHOC_FILE_ERR_RECEIVING] = "file error while receiving state!",
		[ADHOC_FILE_ERR_SENDING] = "file error while sending state!",
		[ADHOC_STILL] = "still : %d",
		[ADHOC_WAITING_OTHER] = "Waiting for other player\n\nPress " SJIS_TRIANGLE " to close connection and quit netplay\n",
		[ADHOC_NETWORKERR_1] = "network error 1",
		[ADHOC_CANNOTFIND] = "cannot find save state!",
		[ADHOC_FLUSHING] = "flushing network, please wait a few seconds",
		// LOAD/SAVE
		[LOADSAVE_AUTOSAVETIMER] = "Autosaving...",
		[LOADSAVE_EXPORTS9XSTATE] = "Found a snes9xTYL file",
		// VIDEO
		[VIDEO_ENGINE_APPROX] = "Simple mode : accurate software",
		[VIDEO_ENGINE_ACCUR] = "Simple mode : approx. software",
		[VIDEO_ENGINE_ACCEL] = "Simple mode : PSP accelerated",
		[VIDEO_ENGINE_ACCEL_ACCUR] = "Mixed modes : PSP accelerated + accurate software",
		[VIDEO_ENGINE_ACCEL_APPROX] = "Mixed modes : PSP accelerated + approx. software",
		[VIDEO_FSKIP_AUTO] = "Frameskip : AUTO",
		[VIDEO_FSKIP_MANUAL] = "Frameskip : %d",
		// Scroll Message
		[SCROLL_TITLE] = "   ,   to move -  ,  for fast mode",
		[SCROLL_STATUS_0] = "Line %d/%d  -  Page %d/%d",
		[SCROLL_STATUS_1] = "   exit,        help  ",
		[SCROLL_HELP] = "Snes9xTYL - fileviewer\n\n"
			SJIS_TRIANGLE " Find, then %s Find next, " SJIS_SQUARE " Find previous\n" 
			SJIS_UP "," SJIS_DOWN " scroll text, L,R scroll faster\n%s exit\n\n"
			"Last position is keeped if same file is reopened.\nHowever it will be reset if another file is opened.\n\n"
			"Press %s",
		[SCROLL_SEARCHING] = "Searching...",
		[SCROLL_STRNOTFOUND] = "String not found!",
		[SCROLL_DISCLAIMER] = "Disclaimer",
		[SCROLL_OSK_DESC] = "Enter String to find",
		//Settings
		[SETTINGS_NOTCOMPLETE] = "!!Settings file not complete!!\n\nProbably coming from a previous version.\n\nNew settings will be set with default values",
		[SETTINGS_NOTFOUND] = "No settings found, using default",
		[SETTINGS_FORCING_DEFAULT] = "Forcing default settings",
		[SETTINGS_IGNORED] = "ini file from a previous incompatible version, ignored!",
		[SETTINGS_FOUND] = "Settings found!\n\n""%s""",
		[SETTINGS_DIFF] = "ini file from a different version.",
		//Others
		[APU_DEBUGGING] = "APU DEBUGGING",
		[BGMUSIC_PLAYING] = "Playing spc file...",
		[FOUND_SRAM] = "Found an SRAM file",
		[CANCEL] = "CANCEL",
		[INPUTBOX_OK] = "\n\n      Close",
#ifdef ME_SOUND
		[BAT_ISLOW] = "Battery is low, saving is now disabled (SRAM,states and settings).\n\nThis window will close in 3 seconds.",
#else
		[BAT_ISLOW] = "Battery is low, saving is now disabled (SRAM,states and settings).\n\nYou can still put your PSP in sleep mode and charge battery later.\n\nThis window will close in 3 seconds.",
#endif
		[CONV_DONE] = "conversion done",
		[ROM_LIED] = "ROM lied about its type! Trying again.",
		[LOADING_ROM] = "Loading %dKo...",
		[TYPE] = "Type",
		[MODE] = "Mode",
		[COMPANY] = "Company",
#ifdef _BSX_151_
		[SRAM_NOTFOUND] = "The SRAM file wasn't found: BS-X.srm was read instead.",
		[SRAM_BSX_NOTFOUND] = "The SRAM file wasn't found, BS-X.srm wasn't found either.",
#endif
		[CORRUPT] = "Corrupt",
		[ROM_ONLY] = "ROM only",
		[EXTENDING] = "Extending",
		[EXTENDING_TARGET] = "Extending target from %d to %d\n",
		[INIT_OSK] = "Initializing OSK...",
	},
	[MSG_JA] = {
	//japanese
		//error
		[ERR_OUT_OF_MEM] = "メモリ外です。",
		[ERR_READ_MEMSTICK] = "メモリースティックの読み込みでエラーが発生しました。\nメモリースティックを確認してください。\n(PCのカードリーダーを使ってください)\nメモリースティックが破損している可能\性があります。",

		[ERR_INIT_SNES] = "メモリの問題でSFCを初期化できません",
		[ERR_LOADING_ROM] = "ROM読み込みエラー",
		[ERR_INIT_GFX] = "グラフィックスを初期化できません",

		[ERR_CANNOT_ALLOC_MEM] = "メモリを割り当てられません",

		[ERR_USB_STARTING_USBBUS] = "USBバスドライバ開始エラー (0x%08X)\n",
		[ERR_USB_STARTING_USBMASS] = "USB大容量記憶装置ドライバ開始エラー (0x%08X)\n",
		[ERR_USB_SETTING_CAPACITY] = "USB大容量記憶装置ドライバ容量設定エラー (0x%08X)\n",
		//confirm
		[ASK_EXIT] = "Snes9XTYLを終了しますか?",
		[ASK_DELETE] = "ファイルを削除しますか?",
		[ASK_SAVEDIR] = "セーブディレクトリが見つかりません。\n"
			"このエミュレーターがあるディレクトリに作成するには" SJIS_CIRCLE "ボタンを押してください。\n"
			"SAVEDATAディレクトリに作成するには" SJIS_CROSS "ボタンを押してください。\n",
		//info
		[INFO_USB_ON] = "USBが有効化されました",
		[INFO_USB_OFF] = "USBが無効化されました",
		[INFO_DELETING] = "削除しています...",
		[INFO_EXITING] = "終了しています。しばらくお待ちください...",
		//filer
		[FILER_STATUS_CANEXIT1] = "%s 実行 " SJIS_SQUARE " 標準設定で実行  " SJIS_STAR "  %s ゲームに戻る  "  SJIS_STAR "  " SJIS_TRIANGLE " %s  "  SJIS_STAR "  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT " 移動",
		[FILER_STATUS_NOEXIT1] = "%s 実行 " SJIS_SQUARE " 標準設定で実行  "  SJIS_STAR "  " SJIS_TRIANGLE " %s          " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT " リストを移動",
		[FILER_STATUS_CANEXIT2] = "%s 実行 %s ゲームに戻る " SJIS_TRIANGLE " %s  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT " リストを移動",
		[FILER_STATUS_NOEXIT2] = "%s 実行 " SJIS_TRIANGLE " %s                  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT " リストを移動",

		[FILER_STATUS_PARDIR] = "上へ",

		[FILER_TITLE] = "[" EMUNAME_VERSION "] - ファイル選択",

		[FILER_HELP_WINDOW1] = "標準と通信対戦モードを切り替える",
		[FILER_HELP_WINDOW2] = "にはSTARTを押してください",
#ifdef FW3X
		[FILER_HELP_WINDOW3] = " ",
#ifdef HOME_HOOK
		[FILER_HELP_WINDOW4] = "終了するにはHOMEを押してください",
#else
		[FILER_HELP_WINDOW4] = "終了するにはL+Rを押してください",
#endif
#else
#ifdef HOME_HOOK
		[FILER_HELP_WINDOW3] = "終了するにはHOMEを押してください",
#else
		[FILER_HELP_WINDOW3] = "終了するにはL+Rを押してください",
#endif
		[FILER_HELP_WINDOW4] = "USBｵﾝｵﾌ切り替えにRを押してください",
#endif


		[FILER_HELP_WINDOW5] = "削除するにはSELECTを押してください",

		[FILE_IPS_APPLYING] = "IPSパッチが見つかりました : %s\nサイズ : %dKo\n適用しています....",
		[FILE_IPS_PATCHSUCCESS] = "IPSパッチの適用に成功しました",

		[MENU_STATUS_GENERIC_MSG1] = "戻るには%sを押してください",
		[MENU_STATUS_GENERIC_NEEDRELOAD] = "変更を適用するには再読み込みが必要かもしれません",
		[MENU_STATUS_GENERIC_NEEDRESET] = "変更を適用するにはリセットが必要かもしれません",
		[MENU_STATUS_GENERIC_FREERAM] = "空きメモリ : %dKo",
		[MENU_STATUS_GENERIC_CHANGEMUSIC] = "音楽を変えるにはSELECTボタンを、",

		[MENU_TITLE_GENERIC_BAT] = "%02d%c%02d 電池:%s%s%s残り%02d%%%s 温度.%d℃",
		[MENU_TITLE_GENERIC_BAT_TIME] = "(%02d時間%02d分)",
		[MENU_TITLE_GENERIC_BAT_PLG] = "AC.",
		[MENU_TITLE_GENERIC_BAT_CHRG] = "充電中.",
		[MENU_TITLE_GENERIC_BAT_LOW] = "残量低下.",

		[MENU_CHANGE_VALUE] = "  ,   値を変更",
		[MENU_CHANGE_VALUE_WITH_FAST] = "  ,   値を変更   -    ,  高速",
		[MENU_CANCEL_VALIDATE] = "  ,   キャンセル -    適用",
		[MENU_DEFAULT_VALUE] = "   標準値",
		[MENU_YES] = "はい",
		[MENU_NO] = "いいえ",
		[MENU_NOT_IMPLEMENTED] = "まだ実装されていません。\n\n",

		[MENU_SOUND_MODE_NOTEMUL] = "エミュレートしない",
		[MENU_SOUND_MODE_EMULOFF] = "エミュレートするが出力しない",
		[MENU_SOUND_MODE_EMULON] = "エミュレートし、出力する",

		[MENU_STATE_CHOOSEDEL] = "削除するスロットを選んでください",
		[MENU_STATE_CHOOSELOAD] = "読み込むスロットを選んでください",
		[MENU_STATE_CHOOSESAVE] = "保存するスロットを選んでください",
		[MENU_STATE_FREESLOT] = "空き",
		[MENU_STATE_SCANNING] = "使用済みスロットを調べています...",


		[ERR_INIT_OSK] = "OSKを初期化できません",
		[ERR_ADD_CODE] = "チートコードを追加できません。既に入力されているコードが多すぎます。",

		[MENU_CHEATS_ENTERGF] = "GoldFingerコードを入力してください",
		[MENU_CHEATS_ENTERPAR] = "プロアクションリプレイコードを入力してください",
		[MENU_CHEATS_ENTERGG] = "Game Genieコードを入力してください",
		[MENU_CHEATS_ENTERRAW] = "RAWコードを入力してください\n書式 : アドレス - 新しい値",
		[MENU_CHEATS_ENTERNAME] = "名前を入力してください",
		[MENU_CHEATS_CONFIRMREMALL] = "すべてのコードを削除しますか?",
		[MENU_CHEATS_PREVPAGE] = "L - 前のページ",
		[MENU_CHEATS_NEXTPAGE] = "R - 次のページ",

		[MENU_STATE_CONFIRMLOAD] = "現在のゲーム進行が失われます。\n続行しますか?",
		[MENU_STATE_CONFIRMDELETE] = "このステートセーブを削除しますか?",
		[MENU_STATE_CONFIRMSAVE] = "このステートセーブに上書きしますか?",
		[MENU_STATE_ISSAVING] = "ステートセーブしています...",
		[MENU_STATE_ISLOADING] = "状態を読み込んでいます...",
		[MENU_STATE_ISDELETING] = "削除しています...",
		[MENU_STATE_ISIMPORTING] = "ステートセーブをインポートしています...",
		[MENU_STATE_ISEXPORTINGS9X] = "SNES9Xステートセーブをエクスポートしています...",
		[MENU_STATE_NOSTATE] = "使用可能\なステートセーブがありません",
		[MENU_STATE_AUTOSAVETIMER] = "%d分毎",
		[MENU_STATE_AUTOSAVETIMER_OFF] = "オフ",
		[MENU_STATE_WARNING_LOWBAT] = "電池残量が低下しています。\nセーブ中に電源が切れるとメモリースティックが破損する可能\性があります。\n続行しますか?",
		[MENU_GAME_SAVINGDEFAULTSETTINGS] = "現在の設定を標準設定として保存しています",
		[MENU_GAME_CONFIRMRESET] = "SFCをリセットしますか?",
		[MENU_MISC_SAVINGJPEG] = "JPEGスナップショットを保存しています...",

		[MENU_MISC_BGMUSIC_RAND] = "ランダムに再生",
		[MENU_MISC_BGMUSIC_ORDER] = "順に再生",

		[MENU_MISC_BGMUSIC_UNKNOWN] = "不明",
		[MENU_MISC_BGMUSIC_GAMETITLE] = "ゲーム : ",
		[MENU_MISC_BGMUSIC_TITLE] = "題名 : ",
		[MENU_MISC_BGMUSIC_AUTHOR] = "作者 : ",


		// GAME
		[MENU_ICONS_GAME] = "ゲーム",
		[MENU_ICONS_GAME_RESET] = "SFCをリセットする",
		[MENU_ICONS_GAME_NEW] = "新しいゲームを読み込む",
		[MENU_ICONS_GAME_DEFAULTSETTINGS] = "標準設定を作成する",
		[MENU_ICONS_GAME_DEFAULTSETTINGS_HELP] = "現在の設定を特別に設定ファイルがない\n全てのROMに使われる標準設定として\n保存します。",
		[MENU_ICONS_GAME_EXIT] = "Snes9xTYLを終了する",
		// LOAD/SAVE
		[MENU_ICONS_LOADSAVE] = "ロード/セーブ",
		[MENU_ICONS_LOADSAVE_LOADSTATE] = "ステートロード",
		[MENU_ICONS_LOADSAVE_LOADSTATE_HELP] = "以前保存したステートセーブを\n読み込みます。"
			"ステートセーブはSFC\nの状態のスナップショットのような\nもので、すべて保存されます。",
		[MENU_ICONS_LOADSAVE_SAVESTATE] = "ステートセーブ",
		[MENU_ICONS_LOADSAVE_SAVESTATE_HELP] = "ステートセーブします。",
		[MENU_ICONS_LOADSAVE_DELETESTATE] = "ステートセーブを削除",
		[MENU_ICONS_LOADSAVE_DELETESTATE_HELP] = "ステートセーブのセーブデータを\n削除します。",
		[MENU_ICONS_LOADSAVE_IMPORTSTATE] = "ステートセーブをインポート",
		[MENU_ICONS_LOADSAVE_IMPORTSTATE_HELP] = "uosnes9xや以前のバージョンから\nステートセーブのセーブデータを\nインポートします。",

		[MENU_ICONS_LOADSAVE_EXPORTS9XSTATE] = "SNES9Xのステートセーブにエクスポート",
		[MENU_ICONS_LOADSAVE_EXPORTS9XSTATE_HELP] = "SNES9Xフォーマットを使って\nステートセーブのセーブデータを\nエクスポートします。",

		[MENU_ICONS_LOADSAVE_AUTOSAVETIMER] = "オートステートセーブタイマー",
		[MENU_ICONS_LOADSAVE_AUTOSAVETIMER_HELP] = "定期的にオートステートセーブを\nさせます。",
		[MENU_ICONS_LOADSAVE_AUTOSRAM] = "SRAM更新時にオートセーブする",
		[MENU_ICONS_LOADSAVE_AUTOSRAM_HELP] = "変更が行われるたびにSRAMが\n保存されるようにします。",
		// CONTROLS
		[MENU_ICONS_CONTROLS] = "コントローラ",
		[MENU_ICONS_CONTROLS_REDEFINE] = "再設定",
		[MENU_ICONS_CONTROLS_REDEFINE_HELP] = "ボタン割り当てを再設定します。",
		[MENU_ICONS_CONTROLS_INPUT] = "エミュレートされるコントローラ",
		[MENU_ICONS_CONTROLS_INPUT_HELP] = "現在有効にするSFCコントローラを\n選んでください。",

		[MENU_CONTROLS_INPUT] = "コントローラ %d",
		[MENU_CONTROLS_INPUT_PRESS] = "%sに割り当てるボタンを押してください",
		[MENU_CONTROLS_INPUT_NOFORMENU] = "メニューにアクセスするために設定されたボタンありません。\nボタンを選んでください。",
#ifdef HOME_HOOK
		[MENU_CONTROLS_INPUT_DEFAULT] = "標準プロファイルを選択してください\n\n"
			SJIS_CROSS " - 標準, SFCの十\字ボタンがPSPの十\字ボタンに割り当てられます。\n\n"
			SJIS_CIRCLE " - 標準, SFCの十\字ボタンがPSPのアナログスティックに割り当てられます。\n\n"
			SJIS_SQUARE " - 標準, SFCの十\字ボタンがPSPの十\字ボタンとアナログスティックに割り当てられます。\n\n"
			SJIS_TRIANGLE " キャンセル\n",
#else
		[MENU_CONTROLS_INPUT_DEFAULT] = "標準プロファイルを選択してください\n\n"
			SJIS_CROSS " - 標準, SFCの十\字ボタンがPSPの十\字ボタンに割り当てられます。\nメニューはアナログスティック左に割り当てます。\n\n"
			SJIS_CIRCLE " - 標準, SFCの十\字ボタンがPSPのアナログスティックに割り当てられます。\nメニューは十\字ボタン左に割り当てます。\n\n"
			SJIS_SQUARE " - Default, SFCの十\字ボタンがPSPの十\字ボタンとアナログスティックに割り当てられます。\nメニューはL+Rトリガーに割り当てます。\n\n"
			SJIS_TRIANGLE " キャンセル\n",
#endif

		[MENU_STATUS_CONTROLS_INPUT] = "%s 検出モード   " SJIS_STAR "  %s メニュー  ",
		[MENU_STATUS_CONTROLS_INPUT_0] = SJIS_STAR "  " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更  " SJIS_STAR "  SELECT 標準設定",
		[MENU_STATUS_CONTROLS_INPUT_HELP_0] = SJIS_STAR "      " SJIS_TRIANGLE " ヘルプ       " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更",
		[MENU_STATUS_CONTROLS_INPUT_HELP_1] = SJIS_STAR "     " SJIS_TRIANGLE "   ヘルプ      " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更",
		[MENU_STATUS_CONTROLS_INPUT_HELP_2] = SJIS_STAR "    " SJIS_TRIANGLE "     ヘルプ     " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更",
		[MENU_STATUS_CONTROLS_INPUT_HELP_3] = SJIS_STAR "   " SJIS_TRIANGLE "       ヘルプ    " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更",

		[MENU_TITLE_CONTROLS_INPUT] = "[" EMUNAME_VERSION "] - 入力設定",

		// VIDEO OPTIONS
		[MENU_ICONS_VIDEO] = "ビデオ",
		[MENU_ICONS_VIDEO_MODE] = "出力モード",
		[MENU_ICONS_VIDEO_MODE_HELP] = "アスペクト比を変更します。\n4:3が推薦されます。",
		[MENU_ICONS_VIDEO_ENGINE] = "エンジン",
		[MENU_ICONS_VIDEO_ENGINE_HELP] = "ハードウェアクセラレートモードが\nいつも最速なわけではありません。",
		[MENU_ICONS_VIDEO_SLIMITONOFF] = "速度制限切り替え",
		[MENU_ICONS_VIDEO_SLIMITONOFF_HELP] = "ゲームの速度を制限するには\nオンにしてください。",
		[MENU_ICONS_VIDEO_SLIMITVALUE] = "FPS速度制限",
		[MENU_ICONS_VIDEO_SLIMITVALUE_HELP] = "速度制限がオンのときのみ\n使われます。",
		[MENU_ICONS_VIDEO_FSKIP] = "フレームスキップ",
		[MENU_ICONS_VIDEO_FSKIP_HELP] = "表\示されないスプライトがある場合は\n固定値を選んでください。",
		[MENU_ICONS_VIDEO_SMOOTHING] = "スムーシング",
		[MENU_ICONS_VIDEO_SMOOTHING_HELP] = "何の影響もないときのみ\nオンにすべきです。",
		[MENU_ICONS_VIDEO_VSYNC] = "垂直同期",
		[MENU_ICONS_VIDEO_VSYNC_HELP] = "これは多くのFPSを犠牲にするので\nオフにすべきです。",
		[MENU_ICONS_VIDEO_PALASNTSC] = "PALをNTSCとして描画する",
		[MENU_ICONS_VIDEO_PALASNTSC_HELP] = "PALのゲームを240本の垂直解像度の\n代わりに224本にしてNTSCのもののように\nエミュレートするよう強制します。",
		[MENU_ICONS_VIDEO_GAMMA] = "ガンマ補正",
		[MENU_ICONS_VIDEO_GAMMA_HELP] = "より明るめな描画をさせます。",
		[MENU_ICONS_VIDEO_SCRCALIB] = "画面位置補正",
		[MENU_ICONS_VIDEO_SCRCALIB_HELP] = "邪魔な黒い枠を取り除くのに\n役立ちます。",
		[MENU_ICONS_VIDEO_SHOWFPS] = "FPS表\示",
		[MENU_ICONS_VIDEO_SHOWFPS_HELP] = "1行目はエミュレートされる\nフレーム数を表\示します。\n2行目は本当のFPSを表\示します。",

		[MENU_VIDEO_MODE_1_1] = "等倍",
		[MENU_VIDEO_MODE_ZOOM_FIT] = "画面に合わせて拡大",
		[MENU_VIDEO_MODE_ZOOM_4_3RD] = "4/3倍",
		[MENU_VIDEO_MODE_ZOOM_WIDE] = "ワイドに拡大",
		[MENU_VIDEO_MODE_FULLSCREEN] = "全画面表\示",
		[MENU_VIDEO_MODE_FULLSCREEN_CLIPPED] = "一部を切り取って全画面表\示",
		[MENU_VIDEO_ENGINE_APPROX] = "ほぼ正確なソ\フトウェア描画",
		[MENU_VIDEO_ENGINE_ACCUR] = "正確なソ\フトウェア描画",
		[MENU_VIDEO_ENGINE_ACCEL] = "PSPアクセラレータ",
		[MENU_VIDEO_ENGINE_ACCEL_ACCUR] = "PSPｱｸｾﾗﾚｰﾀ+正確なｿﾌﾄｳｪｱ描画",
		[MENU_VIDEO_ENGINE_ACCEL_APPROX] = "PSPｱｸｾﾗﾚｰﾀ+ほぼ正確なｿﾌﾄｳｪｱ描画",
		[MENU_VIDEO_SLIMITVALUE_AUTO] = "自動 (PAL/NTSCを判別する)",
		[MENU_VIDEO_FSKIP_MODE_AUTO] = "%d以下で自動的に変更",
		[MENU_VIDEO_FSKIP_CHANGEAUTO_AUTO] = "   設定値以下で自動的に変更",
		[MENU_VIDEO_FSKIP_CHANGEAUTO_FIXED] = "   固定",

		[MENU_STATUS_VIDEO_SCRCALIB] = "十\字ボタン : 画面位置  " SJIS_STAR "  アナログスティック : 画面サイズ  " SJIS_STAR "  " SJIS_TRIANGLE " 標準  " SJIS_STAR "  %s 終了",

		// SOUND OPTIONS
		[MENU_ICONS_SOUND] = "音声",
		[MENU_ICONS_SOUND_MODE] = "サウンドモード",
		[MENU_ICONS_SOUND_MODE_HELP] = "エミュレートしないのが最速ですが\nいくつかのゲームは動作しません。",
		[MENU_ICONS_SOUND_FREQ] = "出力周波数",
		[MENU_ICONS_SOUND_FREQ_HELP] = "低いほうが速くなります。\n高いほうが音質がよくなります。",
		[MENU_ICONS_SOUND_APURATIO] = "APU周期比",
		[MENU_ICONS_SOUND_APURATIO_HELP] = "それが何をするか知っている\n高度なユーザー向けです。",
		// MISC OPTIONS
		[MENU_ICONS_MISC] = "その他",
		[MENU_ICONS_MISC_FILEVIEWER] = "ファイル表\示",
		[MENU_ICONS_MISC_FILEVIEWER_HELP] = "簡単なファイルビューワーです。",
		[MENU_ICONS_MISC_PSPCLOCK] = "PSPクロック速度",
		[MENU_ICONS_MISC_PSPCLOCK_HELP] = "222MHz以上はオーバークロックでは\nありません。\n安全ですが電池を速く消費します。",
		[MENU_ICONS_MISC_HACKDEBUG] = "ハック/デバッグメニュー",
		[MENU_ICONS_MISC_HACKDEBUG_HELP] = "高度なユーザー向けです。\n描画を調整してスピードを\n得るのに役立ちます。",
		[MENU_ICONS_MISC_SNAPSHOT] = "スナップショット",
		[MENU_ICONS_MISC_SNAPSHOT_HELP] = "現在のゲームの画面の\nスナップショットを撮ります。\nファイル一覧で使われます。",
		[MENU_ICONS_MISC_SWAPBG] = "背景切り替え",
		[MENU_ICONS_MISC_SWAPBG_HELP] = "DATA/logo.zipから\n背景を表\示します。",
		[MENU_ICONS_MISC_BGMUSIC] = "メニューBGM",
		[MENU_ICONS_MISC_BGMUSIC_HELP] = "メニューBGMはPSPを300MHzで\n動作させる必要があります。\n音楽はDATA/music.zipから再生します。\nSPCフォーマットのみです。",
		[MENU_ICONS_MISC_BGFX] = "メニューバックグラウンドCG",
		[MENU_ICONS_MISC_BGFX_HELP] = "PS2DEV spline sampleの素敵なCGです。",
		[MENU_ICONS_MISC_PADBEEP] = "パッドビープ",
		[MENU_ICONS_MISC_PADBEEP_HELP] = "ビープ音がメニューやファイル一覧を\n移動しているときに鳴ります。",
		[MENU_ICONS_MISC_AUTOSTART] = "最後に起動したROMを自動起動する",
		[MENU_ICONS_MISC_AUTOSTART_HELP] = "「はい」にすると、エミュレーターを\n起動したときに自動的に\n最後に起動したROMが起動します。",
		[MENU_ICONS_MISC_OSK] = "OSK",
		[MENU_ICONS_MISC_OSK_HELP] = "OSKとは画面上のキーボードのことです。\nDanzeff OSKはアルファベットをPSPで\n入力する際に最適です。\nSCE公式OSKは日本語やePSPで\n入力する際に最適です。",

		[MENU_MUSIC_SWAPBG_NODATA] = "データがありません",
		[MENU_MISC_SWAPBG_RAND] = "   ランダム",

		[MENU_MISC_OSK_DANZEFF] = "Danzeff",
		[MENU_MISC_OSK_OFFICIAL] = "公式",

		[MENU_STATUS_MISC_HACKDEBUG] = "%s メインメニュー   ",
		[MENU_STATUS_MISC_HACKDEBUG_FUNC] = "%s OK     " SJIS_STAR "    %s メインメニュー   ",
		[MENU_STATUS_MISC_HACKDEBUG_0] = SJIS_STAR "    " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更",
		[MENU_STATUS_MISC_HACKDEBUG_HELP_0] = SJIS_STAR "      " SJIS_TRIANGLE " ヘルプ       " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更",
		[MENU_STATUS_MISC_HACKDEBUG_HELP_1] = SJIS_STAR "     " SJIS_TRIANGLE "   ヘルプ      " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更",
		[MENU_STATUS_MISC_HACKDEBUG_HELP_2] = SJIS_STAR "    " SJIS_TRIANGLE "     ヘルプ     " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更",
		[MENU_STATUS_MISC_HACKDEBUG_HELP_3] = SJIS_STAR "   " SJIS_TRIANGLE "       ヘルプ    " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " 選択 " SJIS_LEFT "," SJIS_RIGHT " 値を変更",

		[MENU_TITLE_MISC_HACKDEBUG] = "[" EMUNAME_VERSION "] - メニュー",

		// CHEATS
		[MENU_ICONS_CHEATS] = "チート",
		[MENU_ICONS_CHEATS_ADDRAW] = "RAWコード追加",
		[MENU_ICONS_CHEATS_ADDRAW_HELP] = "RAWコードを追加します。",
		[MENU_ICONS_CHEATS_ADDGG] = "Game Genieコード追加",
		[MENU_ICONS_CHEATS_ADDGG_HELP] = "Game Genieコードを追加します。",
		[MENU_ICONS_CHEATS_ADDPAR] = "プロアクションリプレイコード追加",
		[MENU_ICONS_CHEATS_ADDPAR_HELP] = "プロアクションリプレイコードを\n追加します。",
		[MENU_ICONS_CHEATS_ADDGF] = "GoldFingerコード追加",
		[MENU_ICONS_CHEATS_ADDGF_HELP] = "GoldFingerコードを追加します。",
		[MENU_ICONS_CHEATS_DISABLE] = "コード無効化",
		[MENU_ICONS_CHEATS_DISABLE_HELP] = "コードを無効化します。",
		[MENU_ICONS_CHEATS_DISABLEALL] = "全コード無効化",
		[MENU_ICONS_CHEATS_DISABLEALL_HELP] = "全てのコードを無効化します。",
		[MENU_ICONS_CHEATS_ENABLE] = "コード有効化",
		[MENU_ICONS_CHEATS_ENABLE_HELP] = "コードを有効にします。",
		[MENU_ICONS_CHEATS_ENABLEALL] = "全コード有効化",
		[MENU_ICONS_CHEATS_ENABLEALL_HELP] = "全てのコードを有効化します。",
		[MENU_ICONS_CHEATS_REMOVE] = "コード削除",
		[MENU_ICONS_CHEATS_REMOVE_HELP] = "コードを削除します。",
		[MENU_ICONS_CHEATS_REMOVEALL] = "全コード削除",
		[MENU_ICONS_CHEATS_REMOVEALL_HELP] = "全てのコードを削除します。",
		// ABOUT
		[MENU_ICONS_ABOUT] = "このソ\フトについて",
		[MENU_ICONS_ABOUT_CREDITS] = "クレジット",
		[MENU_ICONS_ABOUT_VERSION] = "バージョン情報",

		[MENU_ABOUT_VERSION_TIMESTAMP] = "ビルド日時 : %s",
		[MENU_ABOUT_VERSION_GCCVER] = "使用したGCC : %s",

		//Others
		// Adhoc
		[ADHOC_CONNECTING] = "接続しています...\n",
		[ADHOC_SELECTORRETURN] = "接続するサーバーを選ぶか," SJIS_TRIANGLE "ボタンで戻ってください。",
		[ADHOC_WAITING] = "%sが接続を承認するのを待っています。\nキャンセルするには%sボタンを押してください。\n",
		[ADHOC_REQUESTED] = "%sが接続を要求しています。\n接続を承認するには%sボタンを,キャンセルするには%sボタンを押してください。\n",
		[ADHOC_CONNECTED] = "接続しました",
		[ADHOC_STATE] = "  接続状況 %d/1\n",
		[ADHOC_UNKNOWNCOMMAND] = "不明なコマンド %02X です。",
		[ADHOC_SYNCLOST_SERVER] = "サーバー同期失敗 : 再同期します",
		[ADHOC_SYNCLOST_CLIENT] = "クライアント同期失敗 : 再同期します",
		[ADHOC_CLOSING] = "接続を閉じています",
		[ADHOC_DRIVERLOAD_ERR] = "ネットドライバ読み込みエラー",
		[ADHOC_INIT_ERR] = "アドホックゲームの初期化に問題が発生しました\n",
		[ADHOC_LOST_CONNECTION] = "接続を失いました",
		[ADHOC_CORRUPTED_PKT] = "破損したパケット",
		[ADHOC_STATE_SIZE] = "ステートデータサイズ : %d",
		[ADHOC_FILE_ERR_RECEIVING] = "ステートファイル受信エラー",
		[ADHOC_FILE_ERR_SENDING] = "ステートファイル送信エラー",
		[ADHOC_STILL] = "残り : %d",
		[ADHOC_WAITING_OTHER] = "他のプレイヤーを待っています\n\n接続を閉じ通信対戦をやめるには" SJIS_TRIANGLE "ボタンを押してください\n",
		[ADHOC_NETWORKERR_1] = "ネットワークエラー1",
		[ADHOC_CANNOTFIND] = "ステートセーブが見つかりません",
		[ADHOC_FLUSHING] = "ネットワークを消去しています。しばらくお待ちください。",
		// LOAD/SAVE
		[LOADSAVE_AUTOSAVETIMER] = "自動セーブしています...",
		[LOADSAVE_EXPORTS9XSTATE] = "snes9xTYLファイルが見つかりました",
		// VIDEO
		[VIDEO_ENGINE_APPROX] = "単一モード : ほぼ正確なソ\フトウェア描画",
		[VIDEO_ENGINE_ACCUR] = "単一モード : 正確なソ\フトウェア描画",
		[VIDEO_ENGINE_ACCEL] = "単一モード : PSPアクセラレータ",
		[VIDEO_ENGINE_ACCEL_ACCUR] = "複合モード : PSPアクセラレータ + 正確なソ\フトウェア描画",
		[VIDEO_ENGINE_ACCEL_APPROX] = "複合モード : PSPアクセラレータ + ほぼ正確なソ\フトウェア描画",
		[VIDEO_FSKIP_AUTO] = "フレームスキップ : 自動",
		[VIDEO_FSKIP_MANUAL] = "フレームスキップ : %d",
		// Scroll Message
		[SCROLL_TITLE] = "   ,    移動   -  ,  高速         ",
		[SCROLL_STATUS_0] = "%d/%d行目  -  %d/%dページ",
		[SCROLL_STATUS_1] = "   終了,        ヘルプ",
		[SCROLL_HELP] = "Snes9xTYL - ファイルビューワー\n\n"
			SJIS_TRIANGLE "ボタンで検索し,その後%sボタンで次を検索," SJIS_SQUARE "ボタンで前を検索します。\n"
			SJIS_UP "," SJIS_DOWN "ボタンで文章をスクロールし, L,Rボタンでより早くスクロールします。\n%sで終了します。\n\n"
			"最後の位置は同じファイルが開かれれば維持されます。\nしかしほかのファイルを開くと元に戻ります。\n\n"
			"%sボタンを押してください",
		[SCROLL_SEARCHING] = "検索しています...",
		[SCROLL_STRNOTFOUND] = "文字列は見つかりませんでした",
		[SCROLL_DISCLAIMER] = "免責事項",
		[SCROLL_OSK_DESC] = "検索する文字列を入力してください",
		//Settings
		[SETTINGS_NOTCOMPLETE] = "※設定ファイルが完全ではありません※\n\n以前のバージョンのものの可能\性があります。\n\n新しい設定が標準値で設定されます。",
		[SETTINGS_NOTFOUND] = "設定が見つかりません。標準設定を使います。",
		[SETTINGS_FORCING_DEFAULT] = "強制的に標準設定を使用しています",
		[SETTINGS_IGNORED] = "以前の互換性のないバージョンのINIファイルです。ファイルは無視されました。",
		[SETTINGS_FOUND] = "設定が見つかりました\n\n""%s""",
		[SETTINGS_DIFF] = "異なるバージョンのINIファイルです。",
		//Others
		[APU_DEBUGGING] = "APUデバッグ",
		[BGMUSIC_PLAYING] = "SPCファイルを再生しています...",
		[FOUND_SRAM] = "SRAMファイルが見つかりました",
		[CANCEL] = "キャンセル",
		[INPUTBOX_OK] = "\n\n      閉じる",
#ifdef ME_SOUND
		[BAT_ISLOW] = "電池残量が低下しているため、現在SRAMや設定の保存、ステートセーブが無効にされています。\n\nこの画面は3秒で閉じます。",
#else
		[BAT_ISLOW] = "電池残量が低下しているため、現在SRAMや設定の保存、ステートセーブが無効にされています。\n\n"
			"PSPをスリープモードにしてあとで充電することができます。\n\nこの画面は3秒で閉じます。",
#endif
		[CONV_DONE] = "変換完了",
		[ROM_LIED] = "ROMから間違ったタイプの情報を取得しました。再試行します。",
		[LOADING_ROM] = "%dKo読み込んでいます...",
		[TYPE] = "タイプ",
		[MODE] = "モード",
		[COMPANY] = "制作会社",
#ifdef _BSX_151_
		[SRAM_NOTFOUND] = "SRAMファイルが見つかりませんでした。BS-X.srmが代わりに読み込まれます。",
		[SRAM_BSX_NOTFOUND] = "SRAMファイルが見つからず、BS-X.srmも見つかりませんでした。",
#endif
		[CORRUPT] = "不明",
		[ROM_ONLY] = "ROMのみ",
		[EXTENDING] = "適用範囲を拡大しています",
		[EXTENDING_TARGET] = "適用範囲を%dから%dに拡大しています\n",
		[INIT_OSK] = "OSKを初期化しています...",
	}
};

const char **s9xTYL_msg = s9xTYL_msg_ml[MSG_EN];

void psp_msg_init() {
	int msg_lang;

	switch(os9x_language) {
		case PSP_SYSTEMPARAM_LANGUAGE_JAPANESE:
			msg_lang = MSG_JA;
			break;
		default:
			return;
	}

	s9xTYL_msg = s9xTYL_msg_ml[msg_lang];
}
