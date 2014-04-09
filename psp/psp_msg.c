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

		[MENU_TITLE_GENERIC_BAT] = "%02d%c%02d Bat.:%s%s%s%02d%%%s Tmp.%d��",
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
		[ERR_OUT_OF_MEM] = "�������O�ł��B",
		[ERR_READ_MEMSTICK] = "�������[�X�e�B�b�N�̓ǂݍ��݂ŃG���[���������܂����B\n�������[�X�e�B�b�N���m�F���Ă��������B\n(PC�̃J�[�h���[�_�[���g���Ă�������)\n�������[�X�e�B�b�N���j�����Ă���\\��������܂��B",

		[ERR_INIT_SNES] = "�������̖���SFC���������ł��܂���",
		[ERR_LOADING_ROM] = "ROM�ǂݍ��݃G���[",
		[ERR_INIT_GFX] = "�O���t�B�b�N�X���������ł��܂���",

		[ERR_CANNOT_ALLOC_MEM] = "�����������蓖�Ă��܂���",

		[ERR_USB_STARTING_USBBUS] = "USB�o�X�h���C�o�J�n�G���[ (0x%08X)\n",
		[ERR_USB_STARTING_USBMASS] = "USB��e�ʋL�����u�h���C�o�J�n�G���[ (0x%08X)\n",
		[ERR_USB_SETTING_CAPACITY] = "USB��e�ʋL�����u�h���C�o�e�ʐݒ�G���[ (0x%08X)\n",
		//confirm
		[ASK_EXIT] = "Snes9XTYL���I�����܂���?",
		[ASK_DELETE] = "�t�@�C�����폜���܂���?",
		[ASK_SAVEDIR] = "�Z�[�u�f�B���N�g����������܂���B\n"
			"���̃G�~�����[�^�[������f�B���N�g���ɍ쐬����ɂ�" SJIS_CIRCLE "�{�^���������Ă��������B\n"
			"SAVEDATA�f�B���N�g���ɍ쐬����ɂ�" SJIS_CROSS "�{�^���������Ă��������B\n",
		//info
		[INFO_USB_ON] = "USB���L��������܂���",
		[INFO_USB_OFF] = "USB������������܂���",
		[INFO_DELETING] = "�폜���Ă��܂�...",
		[INFO_EXITING] = "�I�����Ă��܂��B���΂炭���҂���������...",
		//filer
		[FILER_STATUS_CANEXIT1] = "%s ���s " SJIS_SQUARE " �W���ݒ�Ŏ��s  " SJIS_STAR "  %s �Q�[���ɖ߂�  "  SJIS_STAR "  " SJIS_TRIANGLE " %s  "  SJIS_STAR "  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT " �ړ�",
		[FILER_STATUS_NOEXIT1] = "%s ���s " SJIS_SQUARE " �W���ݒ�Ŏ��s  "  SJIS_STAR "  " SJIS_TRIANGLE " %s          " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT " ���X�g���ړ�",
		[FILER_STATUS_CANEXIT2] = "%s ���s %s �Q�[���ɖ߂� " SJIS_TRIANGLE " %s  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT " ���X�g���ړ�",
		[FILER_STATUS_NOEXIT2] = "%s ���s " SJIS_TRIANGLE " %s                  " SJIS_UP "," SJIS_DOWN "," SJIS_LEFT "," SJIS_RIGHT " ���X�g���ړ�",

		[FILER_STATUS_PARDIR] = "���",

		[FILER_TITLE] = "[" EMUNAME_VERSION "] - �t�@�C���I��",

		[FILER_HELP_WINDOW1] = "�W���ƒʐM�ΐ탂�[�h��؂�ւ���",
		[FILER_HELP_WINDOW2] = "�ɂ�START�������Ă�������",
#ifdef FW3X
		[FILER_HELP_WINDOW3] = " ",
#ifdef HOME_HOOK
		[FILER_HELP_WINDOW4] = "�I������ɂ�HOME�������Ă�������",
#else
		[FILER_HELP_WINDOW4] = "�I������ɂ�L+R�������Ă�������",
#endif
#else
#ifdef HOME_HOOK
		[FILER_HELP_WINDOW3] = "�I������ɂ�HOME�������Ă�������",
#else
		[FILER_HELP_WINDOW3] = "�I������ɂ�L+R�������Ă�������",
#endif
		[FILER_HELP_WINDOW4] = "USB�ݵ̐؂�ւ���R�������Ă�������",
#endif


		[FILER_HELP_WINDOW5] = "�폜����ɂ�SELECT�������Ă�������",

		[FILE_IPS_APPLYING] = "IPS�p�b�`��������܂��� : %s\n�T�C�Y : %dKo\n�K�p���Ă��܂�....",
		[FILE_IPS_PATCHSUCCESS] = "IPS�p�b�`�̓K�p�ɐ������܂���",

		[MENU_STATUS_GENERIC_MSG1] = "�߂�ɂ�%s�������Ă�������",
		[MENU_STATUS_GENERIC_NEEDRELOAD] = "�ύX��K�p����ɂ͍ēǂݍ��݂��K�v��������܂���",
		[MENU_STATUS_GENERIC_NEEDRESET] = "�ύX��K�p����ɂ̓��Z�b�g���K�v��������܂���",
		[MENU_STATUS_GENERIC_FREERAM] = "�󂫃����� : %dKo",
		[MENU_STATUS_GENERIC_CHANGEMUSIC] = "���y��ς���ɂ�SELECT�{�^�����A",

		[MENU_TITLE_GENERIC_BAT] = "%02d%c%02d �d�r:%s%s%s�c��%02d%%%s ���x.%d��",
		[MENU_TITLE_GENERIC_BAT_TIME] = "(%02d����%02d��)",
		[MENU_TITLE_GENERIC_BAT_PLG] = "AC.",
		[MENU_TITLE_GENERIC_BAT_CHRG] = "�[�d��.",
		[MENU_TITLE_GENERIC_BAT_LOW] = "�c�ʒቺ.",

		[MENU_CHANGE_VALUE] = "  ,   �l��ύX",
		[MENU_CHANGE_VALUE_WITH_FAST] = "  ,   �l��ύX   -    ,  ����",
		[MENU_CANCEL_VALIDATE] = "  ,   �L�����Z�� -    �K�p",
		[MENU_DEFAULT_VALUE] = "   �W���l",
		[MENU_YES] = "�͂�",
		[MENU_NO] = "������",
		[MENU_NOT_IMPLEMENTED] = "�܂���������Ă��܂���B\n\n",

		[MENU_SOUND_MODE_NOTEMUL] = "�G�~�����[�g���Ȃ�",
		[MENU_SOUND_MODE_EMULOFF] = "�G�~�����[�g���邪�o�͂��Ȃ�",
		[MENU_SOUND_MODE_EMULON] = "�G�~�����[�g���A�o�͂���",

		[MENU_STATE_CHOOSEDEL] = "�폜����X���b�g��I��ł�������",
		[MENU_STATE_CHOOSELOAD] = "�ǂݍ��ރX���b�g��I��ł�������",
		[MENU_STATE_CHOOSESAVE] = "�ۑ�����X���b�g��I��ł�������",
		[MENU_STATE_FREESLOT] = "��",
		[MENU_STATE_SCANNING] = "�g�p�ς݃X���b�g�𒲂ׂĂ��܂�...",


		[ERR_INIT_OSK] = "OSK���������ł��܂���",
		[ERR_ADD_CODE] = "�`�[�g�R�[�h��ǉ��ł��܂���B���ɓ��͂���Ă���R�[�h���������܂��B",

		[MENU_CHEATS_ENTERGF] = "GoldFinger�R�[�h����͂��Ă�������",
		[MENU_CHEATS_ENTERPAR] = "�v���A�N�V�������v���C�R�[�h����͂��Ă�������",
		[MENU_CHEATS_ENTERGG] = "Game Genie�R�[�h����͂��Ă�������",
		[MENU_CHEATS_ENTERRAW] = "RAW�R�[�h����͂��Ă�������\n���� : �A�h���X - �V�����l",
		[MENU_CHEATS_ENTERNAME] = "���O����͂��Ă�������",
		[MENU_CHEATS_CONFIRMREMALL] = "���ׂẴR�[�h���폜���܂���?",
		[MENU_CHEATS_PREVPAGE] = "L - �O�̃y�[�W",
		[MENU_CHEATS_NEXTPAGE] = "R - ���̃y�[�W",

		[MENU_STATE_CONFIRMLOAD] = "���݂̃Q�[���i�s�������܂��B\n���s���܂���?",
		[MENU_STATE_CONFIRMDELETE] = "���̃X�e�[�g�Z�[�u���폜���܂���?",
		[MENU_STATE_CONFIRMSAVE] = "���̃X�e�[�g�Z�[�u�ɏ㏑�����܂���?",
		[MENU_STATE_ISSAVING] = "�X�e�[�g�Z�[�u���Ă��܂�...",
		[MENU_STATE_ISLOADING] = "��Ԃ�ǂݍ���ł��܂�...",
		[MENU_STATE_ISDELETING] = "�폜���Ă��܂�...",
		[MENU_STATE_ISIMPORTING] = "�X�e�[�g�Z�[�u���C���|�[�g���Ă��܂�...",
		[MENU_STATE_ISEXPORTINGS9X] = "SNES9X�X�e�[�g�Z�[�u���G�N�X�|�[�g���Ă��܂�...",
		[MENU_STATE_NOSTATE] = "�g�p�\\�ȃX�e�[�g�Z�[�u������܂���",
		[MENU_STATE_AUTOSAVETIMER] = "%d����",
		[MENU_STATE_AUTOSAVETIMER_OFF] = "�I�t",
		[MENU_STATE_WARNING_LOWBAT] = "�d�r�c�ʂ��ቺ���Ă��܂��B\n�Z�[�u���ɓd�����؂��ƃ������[�X�e�B�b�N���j������\\��������܂��B\n���s���܂���?",
		[MENU_GAME_SAVINGDEFAULTSETTINGS] = "���݂̐ݒ��W���ݒ�Ƃ��ĕۑ����Ă��܂�",
		[MENU_GAME_CONFIRMRESET] = "SFC�����Z�b�g���܂���?",
		[MENU_MISC_SAVINGJPEG] = "JPEG�X�i�b�v�V���b�g��ۑ����Ă��܂�...",

		[MENU_MISC_BGMUSIC_RAND] = "�����_���ɍĐ�",
		[MENU_MISC_BGMUSIC_ORDER] = "���ɍĐ�",

		[MENU_MISC_BGMUSIC_UNKNOWN] = "�s��",
		[MENU_MISC_BGMUSIC_GAMETITLE] = "�Q�[�� : ",
		[MENU_MISC_BGMUSIC_TITLE] = "�薼 : ",
		[MENU_MISC_BGMUSIC_AUTHOR] = "��� : ",


		// GAME
		[MENU_ICONS_GAME] = "�Q�[��",
		[MENU_ICONS_GAME_RESET] = "SFC�����Z�b�g����",
		[MENU_ICONS_GAME_NEW] = "�V�����Q�[����ǂݍ���",
		[MENU_ICONS_GAME_DEFAULTSETTINGS] = "�W���ݒ���쐬����",
		[MENU_ICONS_GAME_DEFAULTSETTINGS_HELP] = "���݂̐ݒ����ʂɐݒ�t�@�C�����Ȃ�\n�S�Ă�ROM�Ɏg����W���ݒ�Ƃ���\n�ۑ����܂��B",
		[MENU_ICONS_GAME_EXIT] = "Snes9xTYL���I������",
		// LOAD/SAVE
		[MENU_ICONS_LOADSAVE] = "���[�h/�Z�[�u",
		[MENU_ICONS_LOADSAVE_LOADSTATE] = "�X�e�[�g���[�h",
		[MENU_ICONS_LOADSAVE_LOADSTATE_HELP] = "�ȑO�ۑ������X�e�[�g�Z�[�u��\n�ǂݍ��݂܂��B"
			"�X�e�[�g�Z�[�u��SFC\n�̏�Ԃ̃X�i�b�v�V���b�g�̂悤��\n���̂ŁA���ׂĕۑ�����܂��B",
		[MENU_ICONS_LOADSAVE_SAVESTATE] = "�X�e�[�g�Z�[�u",
		[MENU_ICONS_LOADSAVE_SAVESTATE_HELP] = "�X�e�[�g�Z�[�u���܂��B",
		[MENU_ICONS_LOADSAVE_DELETESTATE] = "�X�e�[�g�Z�[�u���폜",
		[MENU_ICONS_LOADSAVE_DELETESTATE_HELP] = "�X�e�[�g�Z�[�u�̃Z�[�u�f�[�^��\n�폜���܂��B",
		[MENU_ICONS_LOADSAVE_IMPORTSTATE] = "�X�e�[�g�Z�[�u���C���|�[�g",
		[MENU_ICONS_LOADSAVE_IMPORTSTATE_HELP] = "uosnes9x��ȑO�̃o�[�W��������\n�X�e�[�g�Z�[�u�̃Z�[�u�f�[�^��\n�C���|�[�g���܂��B",

		[MENU_ICONS_LOADSAVE_EXPORTS9XSTATE] = "SNES9X�̃X�e�[�g�Z�[�u�ɃG�N�X�|�[�g",
		[MENU_ICONS_LOADSAVE_EXPORTS9XSTATE_HELP] = "SNES9X�t�H�[�}�b�g���g����\n�X�e�[�g�Z�[�u�̃Z�[�u�f�[�^��\n�G�N�X�|�[�g���܂��B",

		[MENU_ICONS_LOADSAVE_AUTOSAVETIMER] = "�I�[�g�X�e�[�g�Z�[�u�^�C�}�[",
		[MENU_ICONS_LOADSAVE_AUTOSAVETIMER_HELP] = "����I�ɃI�[�g�X�e�[�g�Z�[�u��\n�����܂��B",
		[MENU_ICONS_LOADSAVE_AUTOSRAM] = "SRAM�X�V���ɃI�[�g�Z�[�u����",
		[MENU_ICONS_LOADSAVE_AUTOSRAM_HELP] = "�ύX���s���邽�т�SRAM��\n�ۑ������悤�ɂ��܂��B",
		// CONTROLS
		[MENU_ICONS_CONTROLS] = "�R���g���[��",
		[MENU_ICONS_CONTROLS_REDEFINE] = "�Đݒ�",
		[MENU_ICONS_CONTROLS_REDEFINE_HELP] = "�{�^�����蓖�Ă��Đݒ肵�܂��B",
		[MENU_ICONS_CONTROLS_INPUT] = "�G�~�����[�g�����R���g���[��",
		[MENU_ICONS_CONTROLS_INPUT_HELP] = "���ݗL���ɂ���SFC�R���g���[����\n�I��ł��������B",

		[MENU_CONTROLS_INPUT] = "�R���g���[�� %d",
		[MENU_CONTROLS_INPUT_PRESS] = "%s�Ɋ��蓖�Ă�{�^���������Ă�������",
		[MENU_CONTROLS_INPUT_NOFORMENU] = "���j���[�ɃA�N�Z�X���邽�߂ɐݒ肳�ꂽ�{�^������܂���B\n�{�^����I��ł��������B",
#ifdef HOME_HOOK
		[MENU_CONTROLS_INPUT_DEFAULT] = "�W���v���t�@�C����I�����Ă�������\n\n"
			SJIS_CROSS " - �W��, SFC�̏\\���{�^����PSP�̏\\���{�^���Ɋ��蓖�Ă��܂��B\n\n"
			SJIS_CIRCLE " - �W��, SFC�̏\\���{�^����PSP�̃A�i���O�X�e�B�b�N�Ɋ��蓖�Ă��܂��B\n\n"
			SJIS_SQUARE " - �W��, SFC�̏\\���{�^����PSP�̏\\���{�^���ƃA�i���O�X�e�B�b�N�Ɋ��蓖�Ă��܂��B\n\n"
			SJIS_TRIANGLE " �L�����Z��\n",
#else
		[MENU_CONTROLS_INPUT_DEFAULT] = "�W���v���t�@�C����I�����Ă�������\n\n"
			SJIS_CROSS " - �W��, SFC�̏\\���{�^����PSP�̏\\���{�^���Ɋ��蓖�Ă��܂��B\n���j���[�̓A�i���O�X�e�B�b�N���Ɋ��蓖�Ă܂��B\n\n"
			SJIS_CIRCLE " - �W��, SFC�̏\\���{�^����PSP�̃A�i���O�X�e�B�b�N�Ɋ��蓖�Ă��܂��B\n���j���[�͏\\���{�^�����Ɋ��蓖�Ă܂��B\n\n"
			SJIS_SQUARE " - Default, SFC�̏\\���{�^����PSP�̏\\���{�^���ƃA�i���O�X�e�B�b�N�Ɋ��蓖�Ă��܂��B\n���j���[��L+R�g���K�[�Ɋ��蓖�Ă܂��B\n\n"
			SJIS_TRIANGLE " �L�����Z��\n",
#endif

		[MENU_STATUS_CONTROLS_INPUT] = "%s ���o���[�h   " SJIS_STAR "  %s ���j���[  ",
		[MENU_STATUS_CONTROLS_INPUT_0] = SJIS_STAR "  " SJIS_UP "," SJIS_DOWN " �I�� " SJIS_LEFT "," SJIS_RIGHT " �l��ύX  " SJIS_STAR "  SELECT �W���ݒ�",
		[MENU_STATUS_CONTROLS_INPUT_HELP_0] = SJIS_STAR "      " SJIS_TRIANGLE " �w���v       " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " �I�� " SJIS_LEFT "," SJIS_RIGHT " �l��ύX",
		[MENU_STATUS_CONTROLS_INPUT_HELP_1] = SJIS_STAR "     " SJIS_TRIANGLE "   �w���v      " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " �I�� " SJIS_LEFT "," SJIS_RIGHT " �l��ύX",
		[MENU_STATUS_CONTROLS_INPUT_HELP_2] = SJIS_STAR "    " SJIS_TRIANGLE "     �w���v     " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " �I�� " SJIS_LEFT "," SJIS_RIGHT " �l��ύX",
		[MENU_STATUS_CONTROLS_INPUT_HELP_3] = SJIS_STAR "   " SJIS_TRIANGLE "       �w���v    " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " �I�� " SJIS_LEFT "," SJIS_RIGHT " �l��ύX",

		[MENU_TITLE_CONTROLS_INPUT] = "[" EMUNAME_VERSION "] - ���͐ݒ�",

		// VIDEO OPTIONS
		[MENU_ICONS_VIDEO] = "�r�f�I",
		[MENU_ICONS_VIDEO_MODE] = "�o�̓��[�h",
		[MENU_ICONS_VIDEO_MODE_HELP] = "�A�X�y�N�g���ύX���܂��B\n4:3�����E����܂��B",
		[MENU_ICONS_VIDEO_ENGINE] = "�G���W��",
		[MENU_ICONS_VIDEO_ENGINE_HELP] = "�n�[�h�E�F�A�N�Z�����[�g���[�h��\n�����ő��Ȃ킯�ł͂���܂���B",
		[MENU_ICONS_VIDEO_SLIMITONOFF] = "���x�����؂�ւ�",
		[MENU_ICONS_VIDEO_SLIMITONOFF_HELP] = "�Q�[���̑��x�𐧌�����ɂ�\n�I���ɂ��Ă��������B",
		[MENU_ICONS_VIDEO_SLIMITVALUE] = "FPS���x����",
		[MENU_ICONS_VIDEO_SLIMITVALUE_HELP] = "���x�������I���̂Ƃ��̂�\n�g���܂��B",
		[MENU_ICONS_VIDEO_FSKIP] = "�t���[���X�L�b�v",
		[MENU_ICONS_VIDEO_FSKIP_HELP] = "�\\������Ȃ��X�v���C�g������ꍇ��\n�Œ�l��I��ł��������B",
		[MENU_ICONS_VIDEO_SMOOTHING] = "�X���[�V���O",
		[MENU_ICONS_VIDEO_SMOOTHING_HELP] = "���̉e�����Ȃ��Ƃ��̂�\n�I���ɂ��ׂ��ł��B",
		[MENU_ICONS_VIDEO_VSYNC] = "��������",
		[MENU_ICONS_VIDEO_VSYNC_HELP] = "����͑�����FPS���]���ɂ���̂�\n�I�t�ɂ��ׂ��ł��B",
		[MENU_ICONS_VIDEO_PALASNTSC] = "PAL��NTSC�Ƃ��ĕ`�悷��",
		[MENU_ICONS_VIDEO_PALASNTSC_HELP] = "PAL�̃Q�[����240�{�̐����𑜓x��\n�����224�{�ɂ���NTSC�̂��̂̂悤��\n�G�~�����[�g����悤�������܂��B",
		[MENU_ICONS_VIDEO_GAMMA] = "�K���}�␳",
		[MENU_ICONS_VIDEO_GAMMA_HELP] = "��薾��߂ȕ`��������܂��B",
		[MENU_ICONS_VIDEO_SCRCALIB] = "��ʈʒu�␳",
		[MENU_ICONS_VIDEO_SCRCALIB_HELP] = "�ז��ȍ����g����菜���̂�\n�𗧂��܂��B",
		[MENU_ICONS_VIDEO_SHOWFPS] = "FPS�\\��",
		[MENU_ICONS_VIDEO_SHOWFPS_HELP] = "1�s�ڂ̓G�~�����[�g�����\n�t���[������\\�����܂��B\n2�s�ڂ͖{����FPS��\\�����܂��B",

		[MENU_VIDEO_MODE_1_1] = "���{",
		[MENU_VIDEO_MODE_ZOOM_FIT] = "��ʂɍ��킹�Ċg��",
		[MENU_VIDEO_MODE_ZOOM_4_3RD] = "4/3�{",
		[MENU_VIDEO_MODE_ZOOM_WIDE] = "���C�h�Ɋg��",
		[MENU_VIDEO_MODE_FULLSCREEN] = "�S��ʕ\\��",
		[MENU_VIDEO_MODE_FULLSCREEN_CLIPPED] = "�ꕔ��؂����đS��ʕ\\��",
		[MENU_VIDEO_ENGINE_APPROX] = "�قڐ��m�ȃ\\�t�g�E�F�A�`��",
		[MENU_VIDEO_ENGINE_ACCUR] = "���m�ȃ\\�t�g�E�F�A�`��",
		[MENU_VIDEO_ENGINE_ACCEL] = "PSP�A�N�Z�����[�^",
		[MENU_VIDEO_ENGINE_ACCEL_ACCUR] = "PSP����ڰ�+���m�ȿ�ĳ���`��",
		[MENU_VIDEO_ENGINE_ACCEL_APPROX] = "PSP����ڰ�+�قڐ��m�ȿ�ĳ���`��",
		[MENU_VIDEO_SLIMITVALUE_AUTO] = "���� (PAL/NTSC�𔻕ʂ���)",
		[MENU_VIDEO_FSKIP_MODE_AUTO] = "%d�ȉ��Ŏ����I�ɕύX",
		[MENU_VIDEO_FSKIP_CHANGEAUTO_AUTO] = "   �ݒ�l�ȉ��Ŏ����I�ɕύX",
		[MENU_VIDEO_FSKIP_CHANGEAUTO_FIXED] = "   �Œ�",

		[MENU_STATUS_VIDEO_SCRCALIB] = "�\\���{�^�� : ��ʈʒu  " SJIS_STAR "  �A�i���O�X�e�B�b�N : ��ʃT�C�Y  " SJIS_STAR "  " SJIS_TRIANGLE " �W��  " SJIS_STAR "  %s �I��",

		// SOUND OPTIONS
		[MENU_ICONS_SOUND] = "����",
		[MENU_ICONS_SOUND_MODE] = "�T�E���h���[�h",
		[MENU_ICONS_SOUND_MODE_HELP] = "�G�~�����[�g���Ȃ��̂��ő��ł���\n�������̃Q�[���͓��삵�܂���B",
		[MENU_ICONS_SOUND_FREQ] = "�o�͎��g��",
		[MENU_ICONS_SOUND_FREQ_HELP] = "�Ⴂ�ق��������Ȃ�܂��B\n�����ق����������悭�Ȃ�܂��B",
		[MENU_ICONS_SOUND_APURATIO] = "APU������",
		[MENU_ICONS_SOUND_APURATIO_HELP] = "���ꂪ�������邩�m���Ă���\n���x�ȃ��[�U�[�����ł��B",
		// MISC OPTIONS
		[MENU_ICONS_MISC] = "���̑�",
		[MENU_ICONS_MISC_FILEVIEWER] = "�t�@�C���\\��",
		[MENU_ICONS_MISC_FILEVIEWER_HELP] = "�ȒP�ȃt�@�C���r���[���[�ł��B",
		[MENU_ICONS_MISC_PSPCLOCK] = "PSP�N���b�N���x",
		[MENU_ICONS_MISC_PSPCLOCK_HELP] = "222MHz�ȏ�̓I�[�o�[�N���b�N�ł�\n����܂���B\n���S�ł����d�r�𑬂�����܂��B",
		[MENU_ICONS_MISC_HACKDEBUG] = "�n�b�N/�f�o�b�O���j���[",
		[MENU_ICONS_MISC_HACKDEBUG_HELP] = "���x�ȃ��[�U�[�����ł��B\n�`��𒲐����ăX�s�[�h��\n����̂ɖ𗧂��܂��B",
		[MENU_ICONS_MISC_SNAPSHOT] = "�X�i�b�v�V���b�g",
		[MENU_ICONS_MISC_SNAPSHOT_HELP] = "���݂̃Q�[���̉�ʂ�\n�X�i�b�v�V���b�g���B��܂��B\n�t�@�C���ꗗ�Ŏg���܂��B",
		[MENU_ICONS_MISC_SWAPBG] = "�w�i�؂�ւ�",
		[MENU_ICONS_MISC_SWAPBG_HELP] = "DATA/logo.zip����\n�w�i��\\�����܂��B",
		[MENU_ICONS_MISC_BGMUSIC] = "���j���[BGM",
		[MENU_ICONS_MISC_BGMUSIC_HELP] = "���j���[BGM��PSP��300MHz��\n���삳����K�v������܂��B\n���y��DATA/music.zip����Đ����܂��B\nSPC�t�H�[�}�b�g�݂̂ł��B",
		[MENU_ICONS_MISC_BGFX] = "���j���[�o�b�N�O���E���hCG",
		[MENU_ICONS_MISC_BGFX_HELP] = "PS2DEV spline sample�̑f�G��CG�ł��B",
		[MENU_ICONS_MISC_PADBEEP] = "�p�b�h�r�[�v",
		[MENU_ICONS_MISC_PADBEEP_HELP] = "�r�[�v�������j���[��t�@�C���ꗗ��\n�ړ����Ă���Ƃ��ɖ�܂��B",
		[MENU_ICONS_MISC_AUTOSTART] = "�Ō�ɋN������ROM�������N������",
		[MENU_ICONS_MISC_AUTOSTART_HELP] = "�u�͂��v�ɂ���ƁA�G�~�����[�^�[��\n�N�������Ƃ��Ɏ����I��\n�Ō�ɋN������ROM���N�����܂��B",
		[MENU_ICONS_MISC_OSK] = "OSK",
		[MENU_ICONS_MISC_OSK_HELP] = "OSK�Ƃ͉�ʏ�̃L�[�{�[�h�̂��Ƃł��B\nDanzeff OSK�̓A���t�@�x�b�g��PSP��\n���͂���ۂɍœK�ł��B\nSCE����OSK�͓��{���ePSP��\n���͂���ۂɍœK�ł��B",

		[MENU_MUSIC_SWAPBG_NODATA] = "�f�[�^������܂���",
		[MENU_MISC_SWAPBG_RAND] = "   �����_��",

		[MENU_MISC_OSK_DANZEFF] = "Danzeff",
		[MENU_MISC_OSK_OFFICIAL] = "����",

		[MENU_STATUS_MISC_HACKDEBUG] = "%s ���C�����j���[   ",
		[MENU_STATUS_MISC_HACKDEBUG_FUNC] = "%s OK     " SJIS_STAR "    %s ���C�����j���[   ",
		[MENU_STATUS_MISC_HACKDEBUG_0] = SJIS_STAR "    " SJIS_UP "," SJIS_DOWN " �I�� " SJIS_LEFT "," SJIS_RIGHT " �l��ύX",
		[MENU_STATUS_MISC_HACKDEBUG_HELP_0] = SJIS_STAR "      " SJIS_TRIANGLE " �w���v       " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " �I�� " SJIS_LEFT "," SJIS_RIGHT " �l��ύX",
		[MENU_STATUS_MISC_HACKDEBUG_HELP_1] = SJIS_STAR "     " SJIS_TRIANGLE "   �w���v      " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " �I�� " SJIS_LEFT "," SJIS_RIGHT " �l��ύX",
		[MENU_STATUS_MISC_HACKDEBUG_HELP_2] = SJIS_STAR "    " SJIS_TRIANGLE "     �w���v     " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " �I�� " SJIS_LEFT "," SJIS_RIGHT " �l��ύX",
		[MENU_STATUS_MISC_HACKDEBUG_HELP_3] = SJIS_STAR "   " SJIS_TRIANGLE "       �w���v    " SJIS_STAR "      " SJIS_UP "," SJIS_DOWN " �I�� " SJIS_LEFT "," SJIS_RIGHT " �l��ύX",

		[MENU_TITLE_MISC_HACKDEBUG] = "[" EMUNAME_VERSION "] - ���j���[",

		// CHEATS
		[MENU_ICONS_CHEATS] = "�`�[�g",
		[MENU_ICONS_CHEATS_ADDRAW] = "RAW�R�[�h�ǉ�",
		[MENU_ICONS_CHEATS_ADDRAW_HELP] = "RAW�R�[�h��ǉ����܂��B",
		[MENU_ICONS_CHEATS_ADDGG] = "Game Genie�R�[�h�ǉ�",
		[MENU_ICONS_CHEATS_ADDGG_HELP] = "Game Genie�R�[�h��ǉ����܂��B",
		[MENU_ICONS_CHEATS_ADDPAR] = "�v���A�N�V�������v���C�R�[�h�ǉ�",
		[MENU_ICONS_CHEATS_ADDPAR_HELP] = "�v���A�N�V�������v���C�R�[�h��\n�ǉ����܂��B",
		[MENU_ICONS_CHEATS_ADDGF] = "GoldFinger�R�[�h�ǉ�",
		[MENU_ICONS_CHEATS_ADDGF_HELP] = "GoldFinger�R�[�h��ǉ����܂��B",
		[MENU_ICONS_CHEATS_DISABLE] = "�R�[�h������",
		[MENU_ICONS_CHEATS_DISABLE_HELP] = "�R�[�h�𖳌������܂��B",
		[MENU_ICONS_CHEATS_DISABLEALL] = "�S�R�[�h������",
		[MENU_ICONS_CHEATS_DISABLEALL_HELP] = "�S�ẴR�[�h�𖳌������܂��B",
		[MENU_ICONS_CHEATS_ENABLE] = "�R�[�h�L����",
		[MENU_ICONS_CHEATS_ENABLE_HELP] = "�R�[�h��L���ɂ��܂��B",
		[MENU_ICONS_CHEATS_ENABLEALL] = "�S�R�[�h�L����",
		[MENU_ICONS_CHEATS_ENABLEALL_HELP] = "�S�ẴR�[�h��L�������܂��B",
		[MENU_ICONS_CHEATS_REMOVE] = "�R�[�h�폜",
		[MENU_ICONS_CHEATS_REMOVE_HELP] = "�R�[�h���폜���܂��B",
		[MENU_ICONS_CHEATS_REMOVEALL] = "�S�R�[�h�폜",
		[MENU_ICONS_CHEATS_REMOVEALL_HELP] = "�S�ẴR�[�h���폜���܂��B",
		// ABOUT
		[MENU_ICONS_ABOUT] = "���̃\\�t�g�ɂ���",
		[MENU_ICONS_ABOUT_CREDITS] = "�N���W�b�g",
		[MENU_ICONS_ABOUT_VERSION] = "�o�[�W�������",

		[MENU_ABOUT_VERSION_TIMESTAMP] = "�r���h���� : %s",
		[MENU_ABOUT_VERSION_GCCVER] = "�g�p����GCC : %s",

		//Others
		// Adhoc
		[ADHOC_CONNECTING] = "�ڑ����Ă��܂�...\n",
		[ADHOC_SELECTORRETURN] = "�ڑ�����T�[�o�[��I�Ԃ�," SJIS_TRIANGLE "�{�^���Ŗ߂��Ă��������B",
		[ADHOC_WAITING] = "%s���ڑ������F����̂�҂��Ă��܂��B\n�L�����Z������ɂ�%s�{�^���������Ă��������B\n",
		[ADHOC_REQUESTED] = "%s���ڑ���v�����Ă��܂��B\n�ڑ������F����ɂ�%s�{�^����,�L�����Z������ɂ�%s�{�^���������Ă��������B\n",
		[ADHOC_CONNECTED] = "�ڑ����܂���",
		[ADHOC_STATE] = "  �ڑ��� %d/1\n",
		[ADHOC_UNKNOWNCOMMAND] = "�s���ȃR�}���h %02X �ł��B",
		[ADHOC_SYNCLOST_SERVER] = "�T�[�o�[�������s : �ē������܂�",
		[ADHOC_SYNCLOST_CLIENT] = "�N���C�A���g�������s : �ē������܂�",
		[ADHOC_CLOSING] = "�ڑ�����Ă��܂�",
		[ADHOC_DRIVERLOAD_ERR] = "�l�b�g�h���C�o�ǂݍ��݃G���[",
		[ADHOC_INIT_ERR] = "�A�h�z�b�N�Q�[���̏������ɖ�肪�������܂���\n",
		[ADHOC_LOST_CONNECTION] = "�ڑ��������܂���",
		[ADHOC_CORRUPTED_PKT] = "�j�������p�P�b�g",
		[ADHOC_STATE_SIZE] = "�X�e�[�g�f�[�^�T�C�Y : %d",
		[ADHOC_FILE_ERR_RECEIVING] = "�X�e�[�g�t�@�C����M�G���[",
		[ADHOC_FILE_ERR_SENDING] = "�X�e�[�g�t�@�C�����M�G���[",
		[ADHOC_STILL] = "�c�� : %d",
		[ADHOC_WAITING_OTHER] = "���̃v���C���[��҂��Ă��܂�\n\n�ڑ�����ʐM�ΐ����߂�ɂ�" SJIS_TRIANGLE "�{�^���������Ă�������\n",
		[ADHOC_NETWORKERR_1] = "�l�b�g���[�N�G���[1",
		[ADHOC_CANNOTFIND] = "�X�e�[�g�Z�[�u��������܂���",
		[ADHOC_FLUSHING] = "�l�b�g���[�N���������Ă��܂��B���΂炭���҂����������B",
		// LOAD/SAVE
		[LOADSAVE_AUTOSAVETIMER] = "�����Z�[�u���Ă��܂�...",
		[LOADSAVE_EXPORTS9XSTATE] = "snes9xTYL�t�@�C����������܂���",
		// VIDEO
		[VIDEO_ENGINE_APPROX] = "�P�ꃂ�[�h : �قڐ��m�ȃ\\�t�g�E�F�A�`��",
		[VIDEO_ENGINE_ACCUR] = "�P�ꃂ�[�h : ���m�ȃ\\�t�g�E�F�A�`��",
		[VIDEO_ENGINE_ACCEL] = "�P�ꃂ�[�h : PSP�A�N�Z�����[�^",
		[VIDEO_ENGINE_ACCEL_ACCUR] = "�������[�h : PSP�A�N�Z�����[�^ + ���m�ȃ\\�t�g�E�F�A�`��",
		[VIDEO_ENGINE_ACCEL_APPROX] = "�������[�h : PSP�A�N�Z�����[�^ + �قڐ��m�ȃ\\�t�g�E�F�A�`��",
		[VIDEO_FSKIP_AUTO] = "�t���[���X�L�b�v : ����",
		[VIDEO_FSKIP_MANUAL] = "�t���[���X�L�b�v : %d",
		// Scroll Message
		[SCROLL_TITLE] = "   ,    �ړ�   -  ,  ����         ",
		[SCROLL_STATUS_0] = "%d/%d�s��  -  %d/%d�y�[�W",
		[SCROLL_STATUS_1] = "   �I��,        �w���v",
		[SCROLL_HELP] = "Snes9xTYL - �t�@�C���r���[���[\n\n"
			SJIS_TRIANGLE "�{�^���Ō�����,���̌�%s�{�^���Ŏ�������," SJIS_SQUARE "�{�^���őO���������܂��B\n"
			SJIS_UP "," SJIS_DOWN "�{�^���ŕ��͂��X�N���[����, L,R�{�^���ł�葁���X�N���[�����܂��B\n%s�ŏI�����܂��B\n\n"
			"�Ō�̈ʒu�͓����t�@�C�����J�����Έێ�����܂��B\n�������ق��̃t�@�C�����J���ƌ��ɖ߂�܂��B\n\n"
			"%s�{�^���������Ă�������",
		[SCROLL_SEARCHING] = "�������Ă��܂�...",
		[SCROLL_STRNOTFOUND] = "������͌�����܂���ł���",
		[SCROLL_DISCLAIMER] = "�Ɛӎ���",
		[SCROLL_OSK_DESC] = "�������镶�������͂��Ă�������",
		//Settings
		[SETTINGS_NOTCOMPLETE] = "���ݒ�t�@�C�������S�ł͂���܂���\n\n�ȑO�̃o�[�W�����̂��̂̉\\��������܂��B\n\n�V�����ݒ肪�W���l�Őݒ肳��܂��B",
		[SETTINGS_NOTFOUND] = "�ݒ肪������܂���B�W���ݒ���g���܂��B",
		[SETTINGS_FORCING_DEFAULT] = "�����I�ɕW���ݒ���g�p���Ă��܂�",
		[SETTINGS_IGNORED] = "�ȑO�̌݊����̂Ȃ��o�[�W������INI�t�@�C���ł��B�t�@�C���͖�������܂����B",
		[SETTINGS_FOUND] = "�ݒ肪������܂���\n\n""%s""",
		[SETTINGS_DIFF] = "�قȂ�o�[�W������INI�t�@�C���ł��B",
		//Others
		[APU_DEBUGGING] = "APU�f�o�b�O",
		[BGMUSIC_PLAYING] = "SPC�t�@�C�����Đ����Ă��܂�...",
		[FOUND_SRAM] = "SRAM�t�@�C����������܂���",
		[CANCEL] = "�L�����Z��",
		[INPUTBOX_OK] = "\n\n      ����",
#ifdef ME_SOUND
		[BAT_ISLOW] = "�d�r�c�ʂ��ቺ���Ă��邽�߁A����SRAM��ݒ�̕ۑ��A�X�e�[�g�Z�[�u�������ɂ���Ă��܂��B\n\n���̉�ʂ�3�b�ŕ��܂��B",
#else
		[BAT_ISLOW] = "�d�r�c�ʂ��ቺ���Ă��邽�߁A����SRAM��ݒ�̕ۑ��A�X�e�[�g�Z�[�u�������ɂ���Ă��܂��B\n\n"
			"PSP���X���[�v���[�h�ɂ��Ă��Ƃŏ[�d���邱�Ƃ��ł��܂��B\n\n���̉�ʂ�3�b�ŕ��܂��B",
#endif
		[CONV_DONE] = "�ϊ�����",
		[ROM_LIED] = "ROM����Ԉ�����^�C�v�̏����擾���܂����B�Ď��s���܂��B",
		[LOADING_ROM] = "%dKo�ǂݍ���ł��܂�...",
		[TYPE] = "�^�C�v",
		[MODE] = "���[�h",
		[COMPANY] = "������",
#ifdef _BSX_151_
		[SRAM_NOTFOUND] = "SRAM�t�@�C����������܂���ł����BBS-X.srm������ɓǂݍ��܂�܂��B",
		[SRAM_BSX_NOTFOUND] = "SRAM�t�@�C���������炸�ABS-X.srm��������܂���ł����B",
#endif
		[CORRUPT] = "�s��",
		[ROM_ONLY] = "ROM�̂�",
		[EXTENDING] = "�K�p�͈͂��g�債�Ă��܂�",
		[EXTENDING_TARGET] = "�K�p�͈͂�%d����%d�Ɋg�債�Ă��܂�\n",
		[INIT_OSK] = "OSK�����������Ă��܂�...",
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
