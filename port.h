/*
 * Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 *
 * (c) Copyright 1996 - 2001 Gary Henderson (gary.henderson@ntlworld.com) and
 *                           Jerremy Koot (jkoot@snes9x.com)
 *
 * Super FX C emulator code 
 * (c) Copyright 1997 - 1999 Ivar (ivar@snes9x.com) and
 *                           Gary Henderson.
 * Super FX assembler emulator code (c) Copyright 1998 zsKnight and _Demo_.
 *
 * DSP1 emulator code (c) Copyright 1998 Ivar, _Demo_ and Gary Henderson.
 * C4 asm and some C emulation code (c) Copyright 2000 zsKnight and _Demo_.
 * C4 C code (c) Copyright 2001 Gary Henderson (gary.henderson@ntlworld.com).
 *
 * DOS port code contains the works of other authors. See headers in
 * individual files.
 *
 * Snes9x homepage: http://www.snes9x.com
 *
 * Permission to use, copy, modify and distribute Snes9x in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Snes9x is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for Snes9x or software derived from Snes9x.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so everyone can benefit from the modifications
 * in future versions.
 *
 * Super NES and Super Nintendo Entertainment System are trademarks of
 * Nintendo Co., Limited and its subsidiary companies.
 */
#ifndef _PORT_H_
#define _PORT_H_


//#define __debug_io_gb__

#if defined(__cplusplus) || defined(c_plusplus)
	#define EXTERN_C extern "C"
	#define START_EXTERN_C extern "C" {
	#define END_EXTERN_C }
#else
	#define EXTERN_C extern
	#define START_EXTERN_C
	#define END_EXTERN_C
#endif


#define SDD1_DECOMP
extern int g_debuginfo;

#ifdef __PSP__
//	#define __GP32_APUCACHE__
	#define CHECK_SOUND()


//	#undef  ZLIB
	#define ZLIB
	#define UNZIP_SUPPORT
	#define NOT_SUPPORT_MOUSE
	#define NOT_SUPPORT_SUPRESCOPE
	
	//#define SPC700_SHUTDOWN
	#define CPU_SHUTDOWN
	#define VAR_CYCLES
	#define SPC700_C	
	//#define EXECUTE_SUPERFX_PER_LINE  not used anymore
	#define OLD_COLOUR_BLENDING
	
#endif


#ifdef __GP32__
	//#define __GP32_APUCACHE__

	#define CHECK_SOUND()

	#define VERSION_MAJOR 0
	#define VERSION_MINOR 3

	long *gp32_fopen (char *fname,char *mode);
	void gp32_fclose (long *s);
	long gp32_fread (unsigned char *ptr,long lg,long *s);
	long gp32_fwrite (unsigned char *ptr,long lg,long *s);
	void gp32_fseek (long position,int ref,long *s);

	//#undef ZLIB
	//#define ZLIB
	//#define UNZIP_SUPPORT


	#define SPC700_SHUTDOWN
	#define CPU_SHUTDOWN
	#define VAR_CYCLES
	#define SPC700_C
	//#define ZSNES_C4
	//#define ZSNES_FX
	#define EXECUTE_SUPERFX_PER_LINE
	//#define THREADCPU


	//#define USE_GLIDE
	//#define USE_OPENGL
	//#define NETPLAY_SUPPORT
	//#define FMOD_SUPPORT
	#define OLD_COLOUR_BLENDING

#endif



#ifdef __PSP__
			
	/*#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <time.h>
	#include <math.h>*/
	#include "psp/psp.h"
	

	#undef byte
	#undef word
	#undef dword
	#undef qword
	
	#define islower(a) ((a>='a')&&(a<='z'))
	#define isdigit(a) ((a>='0')&&(a<='9'))
	#define isalpha(a) (((a>='0')&&(a<='9'))||((a>='0')&&(a<='9'))||((a>='A')&&(a<='Z')))
	#define toupper(a) (islower(a)?a+'A'-'a':a)
	#define _NEWPPU_
	
		

	//#define PROFILING
	#ifdef PROFILING
		void gp32_profile_start(int a);
		void gp32_profile_end(int a);
		#define PROF_START(a) gp32_profile_start(a)
		#define PROF_END(a) gp32_profile_end(a)
	#endif


#endif//end __PSP__

#define PIXEL_FORMAT BGR555
//#define PIXEL_FORMAT BGR444
#undef GFX_MULTI_FORMAT


#ifndef snes9x_types_defined
#define snes9x_types_defined

typedef unsigned char /*unsigned int*/ bool8;


typedef signed char int8;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;


#include "pixform.h"

#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif



#ifndef __WIN32__

	#ifndef PATH_MAX
		#define PATH_MAX 256
	#endif
	#ifndef _MAX_DIR
		#define _MAX_DIR PATH_MAX
	#endif
	#ifndef _MAX_DRIVE
		#define _MAX_DRIVE 10 //ms0: 
	#endif
	#ifndef _MAX_FNAME
		#define _MAX_FNAME PATH_MAX
	#endif
	#ifndef _MAX_EXT
		#define _MAX_EXT 10 //.zip,.smc,.smf,....,.bin,.jpg,.srm,.zaX PATH_MAX
	#endif
	#ifndef _MAX_PATH
		#define _MAX_PATH PATH_MAX
	#endif
	
	#define ZeroMemory(a,b) memset((a),0,(b))
/*	#define ZeroMemory(a,b) memset_((a),0,(b))
	inline void memset_(void * dest,int val,int Size)
	{
		unsigned char* d=(char*)dest;
		unsigned char* to;
		for(to=d+Size;d<to;d++)
			*d=val;
	}*/
	/*void _makepath (char *path, const char *drive, const char *dir,
			const char *fname, const char *ext);
	void _splitpath (const char *path, char *drive, char *dir, char *fname,
			 char *ext);*/
#else // __WIN32__
#endif

/*#ifndef __MWERKS__
	#define strcasecmp stricmp
	#define strncasecmp strnicmp
#else
	#define strcasecmp strcmp
	#define strncasecmp strncmp
#endif
*/

EXTERN_C void S9xGenerateSound ();

#if defined (__DJGPP)||defined(__GP32__)
	#define SLASH_STR "\\"
	#define SLASH_CHAR '\\'
#else
	#define SLASH_STR "/"
	#define SLASH_CHAR '/'
#endif

#ifdef __linux
	typedef void (*SignalHandler)(int);
	#define SIG_PF SignalHandler
#endif

#if defined(__i386__) || defined(__i486__) || defined(__i586__) || \
    defined(__WIN32__) || defined(__alpha__)
	#define LSB_FIRST
	#define FAST_LSB_WORD_ACCESS

#else

	#if defined(__GP32__)||defined(__PSP__)
		#define LSB_FIRST
		#define STATIC static
		#define FASTCALL 
		#undef INLINE		
		#define INLINE inline
		#define VOID void
	#else
		#define MSB_FIRST
	#endif

#endif

#ifndef TITLE
	#define TITLE "Snes9x"
#endif

	#define STATIC static
#endif

#define yo_rand rand
//int yo_rand(void);


#endif
