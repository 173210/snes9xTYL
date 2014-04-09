/*******************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 
  (c) Copyright 1996 - 2003 Gary Henderson (gary.henderson@ntlworld.com) and
                            Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2003 Matthew Kendora and
                            Brad Jorsch (anomie@users.sourceforge.net)
 

                      
  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003 zsKnight (zsknight@zsnes.com),
                            _Demo_ (_demo_@zsnes.com), and
                            Nach (n-a-c-h@users.sourceforge.net)
                                          
  C4 C++ code
  (c) Copyright 2003 Brad Jorsch

  DSP-1 emulator code
  (c) Copyright 1998 - 2003 Ivar (ivar@snes9x.com), _Demo_, Gary Henderson,
                            John Weidman (jweidman@slip.net),
                            neviksti (neviksti@hotmail.com), and
                            Kris Bleakley (stinkfish@bigpond.com)
 
  DSP-2 emulator code
  (c) Copyright 2003 Kris Bleakley, John Weidman, neviksti, Matthew Kendora, and
                     Lord Nightmare (lord_nightmare@users.sourceforge.net

  OBC1 emulator code
  (c) Copyright 2001 - 2003 zsKnight, pagefault (pagefault@zsnes.com)
  Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002 Matthew Kendora with research by
                     zsKnight, John Weidman, and Dark Force

  S-RTC C emulator code
  (c) Copyright 2001 John Weidman
  
  Super FX x86 assembler emulator code 
  (c) Copyright 1998 - 2003 zsKnight, _Demo_, and pagefault 

  Super FX C emulator code 
  (c) Copyright 1997 - 1999 Ivar and Gary Henderson.



 
  Specific ports contains the works of other authors. See headers in
  individual files.
 
  Snes9x homepage: http://www.snes9x.com
 
  Permission to use, copy, modify and distribute Snes9x in both binary and
  source form, for non-commercial purposes, is hereby granted without fee,
  providing that this license information and copyright notice appear with
  all copies and any derived work.
 
  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software.
 
  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes
  charging money for Snes9x or software derived from Snes9x.
 
  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.
 
  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
*******************************************************************************/

#include "stdlib.h"

#include "snes9x.h"
#include "memmap.h"
#include "ppu.h"
#include "sdd1.h"
#include "display.h"


#ifdef __linux
#include <unistd.h>
#endif

void S9xSetSDD1MemoryMap (uint32 bank, uint32 value)
{
    bank = 0xc00 + bank * 0x100;
    value = value * 1024 * 1024;

    int c;

    for (c = 0; c < 0x100; c += 16)
    {
	uint8 *block = &ROM [value + (c << 12)];
	int i;

	for (i = c; i < c + 16; i++)
	    Map [i + bank] = block;
    }
}

void S9xResetSDD1 ()
{
    memset (&ROM_GLOBAL [0x4800], 0, 4);
    for (int i = 0; i < 4; i++)
    {
	ROM_GLOBAL [0x4804 + i] = i;
	S9xSetSDD1MemoryMap (i, i);
    }
}

void S9xSDD1PostLoadState ()
{
    for (int i = 0; i < 4; i++)
	S9xSetSDD1MemoryMap (i, ROM_GLOBAL [0x4804 + i]);
}

void S9xSDD1SaveLoggedData ()
{
/*    if (Memory.SDD1LoggedDataCount != Memory.SDD1LoggedDataCountPrev)
    {
	qsort (Memory.SDD1LoggedData, Memory.SDD1LoggedDataCount, 8,
	       S9xCompareSDD1LoggedDataEntries);

	FILE *fs = fopen (S9xGetFilename (".dat"), "wb");

	if (fs)
	{
	    fwrite (Memory.SDD1LoggedData, 8,
		    Memory.SDD1LoggedDataCount, fs);
	    fclose (fs);
#if defined(__linux)
	    chown (S9xGetFilename (".dat"), getuid (), getgid ());
#endif
	}
	Memory.SDD1LoggedDataCountPrev = Memory.SDD1LoggedDataCount;
    }*/
}

void S9xSDD1LoadLoggedData ()
{
/*    FILE *fs = fopen (S9xGetFilename (".dat"), "rb");

    Memory.SDD1LoggedDataCount = Memory.SDD1LoggedDataCountPrev = 0;

    if (fs)
    {
	int c = fread (Memory.SDD1LoggedData, 8, 
		       MEMMAP_MAX_SDD1_LOGGED_ENTRIES, fs);

	if (c != EOF)
	    Memory.SDD1LoggedDataCount = Memory.SDD1LoggedDataCountPrev = c;
	fclose (fs);
    }*/
}
