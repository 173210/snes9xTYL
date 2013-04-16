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
 
 
#ifndef __GP32__ 
//#include <string.h>
//#include <ctype.h>
#endif

extern const char *S9xGetSaveFilename( const char *e );

#ifdef __linux
#include <unistd.h>
#endif

#include "snes9x.h"
#include "memmap.h"
#include "cpuexec.h"
#include "ppu.h"
#include "display.h"

extern "C" {
#include "cheats.h"
}

#include "apu.h"
#include "sa1.h"
#include "srtc.h"
#include "sdd1.h"

#include "soundux.h"


#ifdef UNZIP_SUPPORT
#include "unzip.h"
#endif


#ifndef ZSNES_FX
#include "fxemu.h"
extern struct FxInit_s SuperFX;
#else
START_EXTERN_C
extern uint8 *SFXPlotTable;
END_EXTERN_C
#endif

static uint8 bytes0x2000 [0x2000];

extern uint8*sdd1_buffer;
/*extern int *Echo;// [32768];
extern int *DummyEchoBuffer;// [SOUND_BUFFER_SIZE];
extern int *MixBuffer;// [SOUND_BUFFER_SIZE];
extern int *EchoBuffer;*/

//add azz 080517
extern char LaunchDir[256];
#include "bsx.h"

extern "C" {
uint32 caCRC32(uint8 *array, uint32 size, register uint32 crc32 = 0xFFFFFFFF);
}
bool8 CMemory::AllASCII (uint8 *b, int size)
{
    for (int i = 0; i < size; i++)
    {
	if (b[i] < 32 || b[i] > 126)
	    return (FALSE);
    }
    return (TRUE);
}

int CMemory::ScoreHiROM (bool8 skip_header)
{
    int score = 0;
    int o = skip_header ? 0xff00 + 0x200 : 0xff00;
	


	if(ROM [o + 0xd5] & 0x1)
		score+=2;

	//Mode23 is SA-1
	if(ROM [o + 0xd5] == 0x23)
		score-=2;

	if(ROM [o+0xd4] == 0x20)
		score +=2;

    if ((ROM [o + 0xdc] + (ROM [o + 0xdd] << 8) +
		ROM [o + 0xde] + (ROM [o + 0xdf] << 8)) == 0xffff)
	{
		score += 2;
		if(0!=(ROM [o + 0xde] + (ROM [o + 0xdf] << 8)))
			score++;
	}
	
    if (ROM [o + 0xda] == 0x33)
		score += 2;
    if ((ROM [o + 0xd5] & 0xf) < 4)
		score += 2;
    if (!(ROM [o + 0xfd] & 0x80))
		score -= 6;
    if ((ROM [o + 0xfc]|(ROM [o + 0xfd]<<8))>0xFFB0)
		score -= 2; //reduced after looking at a scan by Cowering
    if (CalculatedSize > 1024 * 1024 * 3)
		score += 4;
    if ((1 << (ROM [o + 0xd7] - 7)) > 48)
		score -= 1;
    if (!AllASCII (&ROM [o + 0xb0], 6))
		score -= 1;
    if (!AllASCII (&ROM [o + 0xc0], ROM_NAME_LEN - 1))
		score -= 1;
	
    return (score);
}	

int CMemory::ScoreLoROM (bool8 skip_header)
{
    int score = 0;
    int o = skip_header ? 0x7f00 + 0x200 : 0x7f00;
	


	if(!(ROM [o + 0xd5] & 0x1))
		score+=3;

	//Mode23 is SA-1
	if(ROM [o + 0xd5] == 0x23)
		score+=2;

    if ((ROM [o + 0xdc] + (ROM [o + 0xdd] << 8) +
		ROM [o + 0xde] + (ROM [o + 0xdf] << 8)) == 0xffff)
	{
		score += 2;
		if(0!=(ROM [o + 0xde] + (ROM [o + 0xdf] << 8)))
			score++;
	}
	
    if (ROM [o + 0xda] == 0x33)
		score += 2;
    if ((ROM [o + 0xd5] & 0xf) < 4)
		score += 2;
    if (CalculatedSize <= 1024 * 1024 * 16)
		score += 2;
    if (!(ROM [o + 0xfd] & 0x80))
		score -= 6;
	if ((ROM [o + 0xfc]|(ROM [o + 0xfd]<<8))>0xFFB0)
		score -= 2;//reduced per Cowering suggestion
    if ((1 << (ROM [o + 0xd7] - 7)) > 48)
		score -= 1;
    if (!AllASCII (&ROM [o + 0xb0], 6))
		score -= 1;
    if (!AllASCII (&ROM [o + 0xc0], ROM_NAME_LEN - 1))
		score -= 1;
	
    return (score);
}
	
/*char *CMemory::Safe (const char *s)
{
    static char *safe = NULL;
    static int safe_len = 0;

    int len = strlen (s);
    if (!safe || len + 1 > safe_len)
    {
	if (safe)
	    free ((char *) safe);
	safe = (char *) malloc (safe_len = len + 1);
    }

    for (int i = 0; i < len; i++)
    {
	if (s [i] >= 32 && s [i] < 127)
	    safe [i] = s[i];
	else
	    safe [i] = '?';
    }
    safe [len] = 0;
    return (safe);
}*/

/**********************************************************************************************/
/* Init()                                                                                     */
/* This function allocates all the memory needed by the emulator                              */
/**********************************************************************************************/

//extern unsigned int __attribute__((aligned(16))) clut256[256*3];
extern u16 *clut256;
extern u8 *tile_texture[3];
bool8 CMemory::Init ()
{
//DEBUGS("9a");
    //RAM	    = (uint8 *) malloc (0x20000);
//DEBUGS("9b");    
//    SRAM    = (uint8 *) malloc (0x20000+MAX_RTC_INDEX+16);
//    VRAM    = (uint8 *) malloc (0x10000);
//DEBUGS("9c");    
    //FillRAM = (uint8 *) malloc(0x8000);
    C4RAM   = (uint8 *) malloc(0x2000);
    
    sdd1_buffer = (uint8 *) malloc (0x10000);

//DEBUGS("9d");	
	//try to use storage instead of heap mem	

  //ROM     = (uint8 *) calloc (MAX_ROM_SIZE + 0x200 + 0x8000,1);
//DEBUGS("9e");
  //FillRAM = NULL;

	clut256=(u16*)(0x44000000+2*512*272*2+256*240*2+3*256*256*2);
	tile_texture[0]=(u8*)(0x44000000+2*512*272*2+256*240*2+3*256*256*2+256*2*3);
	tile_texture[1]=tile_texture[0]+512*512;
	tile_texture[2]=tile_texture[1]+512*512;  //here we're very close to the 2MB limit...
//	VRAMmode7=tile_texture[2]+512*512;
	

    IPPU.TileCache [TILE_2BIT] = (uint8 *) malloc (MAX_2BIT_TILES * (128+8));
    IPPU.TileCache [TILE_4BIT] = (uint8 *) malloc (MAX_4BIT_TILES * (128+8));
    IPPU.TileCache [TILE_8BIT] = (uint8 *) malloc (MAX_8BIT_TILES * (128+8));
    IPPU.TileCache8 [TILE_2BIT] = (uint8 *) malloc (MAX_2BIT_TILES * (64));
    IPPU.TileCache8 [TILE_4BIT] = (uint8 *) malloc (MAX_4BIT_TILES * (64));
    IPPU.TileCache8 [TILE_8BIT] = (uint8 *) malloc (MAX_8BIT_TILES * (64));
    IPPU.TileCached [TILE_2BIT] = (uint8 *) malloc ((MAX_2BIT_TILES+MAX_4BIT_TILES+MAX_8BIT_TILES) * 2);
    IPPU.TileCached [TILE_4BIT] = IPPU.TileCached [TILE_2BIT]+MAX_2BIT_TILES * 2;
    IPPU.TileCached [TILE_8BIT] = IPPU.TileCached [TILE_4BIT]+MAX_4BIT_TILES * 2;
//   DEBUGS("9f");
    
    
    if (/*!Echo||!DummyEchoBuffer||!MixBuffer||!EchoBuffer||*/!sdd1_buffer || !C4RAM || /*!FillRAM ||*/ !RAM/* || !SRAM || !VRAM*/ || !ROM ||
        !IPPU.TileCache [TILE_2BIT] || !IPPU.TileCache [TILE_4BIT] ||
	!IPPU.TileCache [TILE_8BIT] || !IPPU.TileCached [TILE_2BIT] ||
	!IPPU.TileCached [TILE_4BIT] ||	!IPPU.TileCached [TILE_8BIT])
    {
#ifdef __PSP__
	psp_msg(ERR_CANNOT_ALLOC_MEM, MSG_DEFAULT);
	pgwaitPress();
#else
    	S9xMessage(-1,0,"Cannot allocate memory");
#endif
	Deinit ();
	return (FALSE);
    }
	
    // FillRAM uses first 32K of ROM image area, otherwise space just
    // wasted. Might be read by the SuperFX code.

  //  FillRAM = ROM;
    

    // Add 0x8000 to ROM image pointer to stop SuperFX code accessing
    // unallocated memory (can cause crash on some ports).
 //   ROM += 0x8000;

    //C4RAM    = ROM + 0x400000 + 8192 * 8;
//    ::ROM    = ROM;
//    ::SRAM   = SRAM;
//    ::RegRAM = FillRAM;

#ifdef ZSNES_FX
    SFXPlotTable = ROM + 0x400000;
#else
    SuperFX.pvRegisters = &FillRAM [0x3000];
    SuperFX.nRamBanks = 2;	// Most only use 1.  1=64KB, 2=128KB=1024Mb
    SuperFX.pvRam = ::SRAM;
    SuperFX.nRomBanks = (2 * 1024 * 1024) / (32 * 1024);
    SuperFX.pvRom = (uint8 *) ROM;
#endif

    ZeroMemory (IPPU.TileCached [TILE_2BIT], MAX_2BIT_TILES);
    ZeroMemory (IPPU.TileCached [TILE_4BIT], MAX_4BIT_TILES);
    ZeroMemory (IPPU.TileCached [TILE_8BIT], MAX_8BIT_TILES);
    
    tile_askforreset(-1);
		

    SDD1Data = NULL;
    SDD1Index = NULL;

    return (TRUE);
}

void CMemory::Deinit ()
{
	if (sdd1_buffer){free(sdd1_buffer);sdd1_buffer=NULL;}
	
/*    if (RAM)
    {free ((uint8*)RAM);RAM = NULL;}   */   
    //if (ROM)
    //{ROM-=0x8000;free(ROM);ROM = NULL;}    
    //if (FillRAM)
    //{free((uint8*)FillRAM);FillRAM = NULL;}
    if (C4RAM)
    {free((uint8*)C4RAM);C4RAM = NULL;}

    if (IPPU.TileCache [TILE_2BIT])
    {free ((char *) IPPU.TileCache [TILE_2BIT]);IPPU.TileCache [TILE_2BIT] = NULL;}
    if (IPPU.TileCache [TILE_4BIT])
    {free ((char *) IPPU.TileCache [TILE_4BIT]);IPPU.TileCache [TILE_4BIT] = NULL;}
    if (IPPU.TileCache [TILE_8BIT])
    {free ((char *) IPPU.TileCache [TILE_8BIT]);IPPU.TileCache [TILE_8BIT] = NULL;}
    
    if (IPPU.TileCache8 [TILE_2BIT])
    {free ((char *) IPPU.TileCache8 [TILE_2BIT]);IPPU.TileCache8 [TILE_2BIT] = NULL;}
    if (IPPU.TileCache8 [TILE_4BIT])
    {free ((char *) IPPU.TileCache8 [TILE_4BIT]);IPPU.TileCache8 [TILE_4BIT] = NULL;}
    if (IPPU.TileCache8 [TILE_8BIT])
    {free ((char *) IPPU.TileCache8 [TILE_8BIT]);IPPU.TileCache8 [TILE_8BIT] = NULL;}

    if (IPPU.TileCached [TILE_2BIT])
    {free ((char *) IPPU.TileCached [TILE_2BIT]);IPPU.TileCached [TILE_2BIT] = NULL;}
/*    if (IPPU.TileCached [TILE_4BIT])
    {
	free ((char *) IPPU.TileCached [TILE_4BIT]);
	IPPU.TileCached [TILE_4BIT] = NULL;
    }
    if (IPPU.TileCached [TILE_8BIT])
    {
	free ((char *) IPPU.TileCached [TILE_8BIT]);
	IPPU.TileCached [TILE_8BIT] = NULL;
    }*/

    FreeSDD1Data ();
}



void CMemory::FreeSDD1Data ()
{
    if (SDD1Index)
    {
	free ((char *) SDD1Index);
	SDD1Index = NULL;
    }
    if (SDD1Data)
    {
	free ((char *) SDD1Data);
	SDD1Data = NULL;
    }
}

void CMemory::DeInterleavedRom(bool8 Tales)
{
	int i;
	CPUPack.CPU.TriedInterleavedMode2 = TRUE;		  				
	int nblocks = CalculatedSize >> 16;						
	int blocks [256];

		if (Tales)
		{
	    	nblocks = 0x60;
	   	 	for (i = 0; i < 0x40; i += 2)
	   	 	{
				blocks [i + 0] = (i >> 1) + 0x20;
				blocks [i + 1] = (i >> 1) + 0x00;
	    	}
	    	for (i = 0; i < 0x80; i += 2)
	    	{
				blocks [i + 0x40] = (i >> 1) + 0x80;
				blocks [i + 0x41] = (i >> 1) + 0x40;
	    	}
	    	LoROM = FALSE;
	    	HiROM = TRUE;
		}
		else
		if (Settings.ForceInterleaved2)
		{
	    	for (i = 0; i < nblocks * 2; i++)
	    	{
				blocks [i] = (i & (0xff^0x1e)) | ((i & 2) << 2) | ((i & 4) << 2) |
			     ((i & 8) >> 2) | ((i & 16) >> 2);
	    	}
		}
		else
		{
	    	bool8 t = LoROM;
	    	LoROM = HiROM;
	    	HiROM = t;
		    for (i = 0; i < nblocks; i++)
		    {
				blocks [i * 2] = i + nblocks;
				blocks [i * 2 + 1] = i;
	    	}
		}

		uint8 *tmp = (uint8 *) malloc (0x8000);
		if (tmp)
		{
			S9xMessage(0,0,"DeInterleaving");
		    for (i = 0; i < nblocks * 2; i++)
	    	{
				for (int j = i; j < nblocks * 2; j++)
				{
				    if (blocks [j] == i)
				    {
				    	if (blocks[j]!=blocks[i])
				    	{
							memcpy (tmp, ROM + blocks [j] * 0x8000UL, 0x8000);							
							
							memcpy (ROM+ blocks [j] * 0x8000UL,ROM + blocks [i] * 0x8000UL, 0x8000);
							
							memcpy(ROM+ blocks [i] * 0x8000UL, tmp, 0x8000);
							
						}
						uint8 b = blocks [j];
						blocks [j] = blocks [i];
						blocks [i] = b;
						break;
				    }
				}
		    }
		    free ((char *) tmp);
		}

}
void FixMapPSP()
{
	return;
	for(int i=0;i<MEMMAP_NUM_BLOCKS;i++)
	{
		if(Map [i]>=(uint8 *)CMemory::MAP_LAST && Memory.BlockIsRAM[i] == TRUE)
			Map [i] = (uint8*)((uint) (Map[i])|0x80000000);
	}
}

int CMemory::LoadROMMore(int TotalFileSize,int &retry_count)
{
	bool8 Interleaved = FALSE;
    bool8 Tales = FALSE;
	int orig_hi_score, orig_lo_score;
    int hi_score, lo_score;
    int i;

    orig_hi_score = hi_score = ScoreHiROM (FALSE);
    orig_lo_score = lo_score = ScoreLoROM (FALSE);
    
    if (HeaderCount == 0 && !Settings.ForceNoHeader &&
	((hi_score > lo_score && ScoreHiROM (TRUE) > hi_score) ||
	 (hi_score <= lo_score && ScoreLoROM (TRUE) > lo_score)))
    {
	memmove (ROM, ROM + 512, TotalFileSize - 512);

	TotalFileSize -= 512;
	//S9xMessage (S9X_INFO, S9X_HEADER_WARNING, 
		//    "Try specifying the -nhd command line option if the game doesn't work\n");
    }

    CalculatedSize = (TotalFileSize / 0x2000) * 0x2000;
    ZeroMemory (ROM + CalculatedSize, MAX_ROM_SIZE - CalculatedSize);
    
	    
    // Check for cherryroms.com DAIKAIJYUMONOGATARI2

    if (CalculatedSize == 0x500000 && 
	strncmp ((const char *)&ROM [0x40ffc0], "DAIKAIJYUMONOGATARI2", 20) == 0 &&
	strncmp ((const char *)&ROM [0x40ffb0], "18AE6J", 6) == 0 &&
	memcmp (&ROM[0x40ffb0], &ROM [0xffb0], 0x30))
    {
    	//Change
    	
		memmove (&ROM[0x100000], ROM, 0x500000);
		memmove (ROM, &ROM[0x500000], 0x100000);
    }

    Interleaved = Settings.ForceInterleaved || Settings.ForceInterleaved2;
    if (Settings.ForceLoROM || (!Settings.ForceHiROM && lo_score >= hi_score))
    {
		LoROM = TRUE;
		HiROM = FALSE;
		// Ignore map type byte if not 0x2x or 0x3x
		if ((ROM [0x7fd5] & 0xf0) == 0x20 || (ROM [0x7fd5] & 0xf0) == 0x30)
		{
		    switch (ROM [0x7fd5] & 0xf)
		    {
			    case 1:
				if (strncmp ((char *) &ROM [0x7fc0], "TREASURE HUNTER G", 17) != 0)
				    Interleaved = TRUE;
				break;
			    case 2:
#if 0
					if (!Settings.ForceLoROM &&
					    strncmp ((char *) &ROM [0x7fc0], "SUPER FORMATION SOCCE", 21) != 0 &&
					    strncmp ((char *) &ROM [0x7fc0], "Star Ocean", 10) != 0)
					{
					    LoROM = FALSE;
					    HiROM = TRUE;
					}
#endif
				break;
			    case 5:
					Interleaved = TRUE;
					Tales = TRUE;
				break;
		    }
		}
    }
    else
    {
		if ((ROM [0xffd5] & 0xf0) == 0x20 || (ROM [0xffd5] & 0xf0) == 0x30)
		{
		    switch (ROM [0xffd5] & 0xf)
		    {
			    case 0:
			    case 3:
				Interleaved = TRUE;
				break;
		    }
		}
		LoROM = FALSE;
		HiROM = TRUE;		
    }


if (!Settings.ForceHiROM && !Settings.ForceLoROM &&
	!Settings.ForceInterleaved && !Settings.ForceInterleaved2 &&
	!Settings.ForceNotInterleaved && !Settings.ForcePAL &&
	!Settings.ForceSuperFX && !Settings.ForceDSP1 &&
	!Settings.ForceSA1 && !Settings.ForceC4 &&
	!Settings.ForceSDD1)
    {
    
	    if(strncmp((char *) &ROM [0x7fc0], "YOSHI'S ISLAND", 14) == 0&&(*(uint16*)&ROM[0x7FDE])==57611&&ROM[0x10002]==0xA9)
		{
			Interleaved=true;
			Settings.ForceInterleaved2=true;
		}

    
		if (strncmp ((char *) &ROM [0x7fc0], "YUYU NO QUIZ DE GO!GO!", 22) == 0)
		{
		    LoROM = TRUE;
		    HiROM = FALSE;
		    Interleaved = FALSE;		  
		}
		else 
		if (strncmp ((char *) &ROM [0x7fc0], "SP MOMOTAROU DENTETSU2", 22) == 0)
		{
		    LoROM = TRUE;
		    HiROM = FALSE;
		    Interleaved = FALSE;		  
		}
		else 
		if (CalculatedSize == 0x100000 && 
		    strncmp ((char *) &ROM [0xffc0], "WWF SUPER WRESTLEMANIA", 22) == 0)
		{
		    int cvcount;		
		    
		    memmove (&ROM[0x100000] , ROM, 0x100000);
		    
		    for (cvcount = 0; cvcount < 16; cvcount++)
		    {
				memmove (&ROM[0x8000 * cvcount], &ROM[0x10000 * cvcount + 0x100000 + 0x8000], 0x8000);
				
				memmove (&ROM[0x8000 * cvcount + 0x80000], &ROM[0x10000 * cvcount + 0x100000], 0x8000);
				
		    }
		    LoROM = TRUE;
		    HiROM = FALSE;
	    	ZeroMemory (ROM + CalculatedSize, MAX_ROM_SIZE - CalculatedSize);
		}
    }
   
    if (!Settings.ForceNotInterleaved && Interleaved)
    {
		S9xMessage(0,0,"interleaved ROM");

    	DeInterleavedRom(Tales);

#ifdef __PSP__
		psp_msg(CONV_DONE, MSG_DEFAULT);
#else
		S9xMessage(0,0,"conversion done");
#endif

		hi_score = ScoreHiROM (FALSE);
		lo_score = ScoreLoROM (FALSE);

		if ((HiROM &&
		     (lo_score >= hi_score || hi_score < 0)) ||
		    (LoROM && 
		     (hi_score > lo_score || lo_score < 0)))
		{
		    if (retry_count == 0)
		    {
#ifdef __PSP__
				psp_msg(ROM_LIED, MSG_DEFAULT);
#else
				S9xMessage(0,0,"ROM lied about its type! Trying again.");
#endif

				Settings.ForceNotInterleaved = TRUE;
				Settings.ForceInterleaved = FALSE;
				retry_count++;
				//goto again;
				return 1;
		    }
		}
    }
    
    FreeSDD1Data ();
    InitROM (Tales);
    FixMapPSP();
    return 0;
}
#ifdef _BSX_151_
int CMemory::LoadROMMore_151(int TotalFileSize,int &retry_count)
{
	bool8 Interleaved = FALSE;
    bool8 Tales = FALSE;
	int orig_hi_score, orig_lo_score;
    int hi_score, lo_score;
    int i;

    orig_hi_score = hi_score = ScoreHiROM (FALSE);
    orig_lo_score = lo_score = ScoreLoROM (FALSE);
    
    if (HeaderCount == 0 && !Settings.ForceNoHeader &&
	((hi_score > lo_score && ScoreHiROM (TRUE) > hi_score) ||
	 (hi_score <= lo_score && ScoreLoROM (TRUE) > lo_score)))
    {
	memmove (ROM, ROM + 512, TotalFileSize - 512);

	TotalFileSize -= 512;
	//S9xMessage (S9X_INFO, S9X_HEADER_WARNING, 
		//    "Try specifying the -nhd command line option if the game doesn't work\n");
    }

    CalculatedSize = (TotalFileSize / 0x2000) * 0x2000;
    ZeroMemory (ROM + CalculatedSize, MAX_ROM_SIZE - CalculatedSize);
    ExtendedFormat = NOPE;
	    
    // Check for cherryroms.com DAIKAIJYUMONOGATARI2

    if (CalculatedSize == 0x500000 && 
	strncmp ((const char *)&ROM [0x40ffc0], "DAIKAIJYUMONOGATARI2", 20) == 0 &&
	strncmp ((const char *)&ROM [0x40ffb0], "18AE6J", 6) == 0 &&
	memcmp (&ROM[0x40ffb0], &ROM [0xffb0], 0x30))
    {
    	//Change
    	
		memmove (&ROM[0x100000], ROM, 0x500000);
		memmove (ROM, &ROM[0x500000], 0x100000);
    }
	//151
	if (CalculatedSize > 0x400000 &&
		(ROM[0x7fd5] + (ROM[0x7fd6] << 8)) != 0x4332 && // exclude S-DD1
		(ROM[0x7fd5] + (ROM[0x7fd6] << 8)) != 0x4532 &&
		(ROM[0xffd5] + (ROM[0xffd6] << 8)) != 0xF93a && // exclude SPC7110
		(ROM[0xffd5] + (ROM[0xffd6] << 8)) != 0xF53a)
		ExtendedFormat = YEAH;
	if (ExtendedFormat != NOPE)
	{
		//int	swappedhirom, swappedlorom;

		//swappedhirom = ScoreHiROM(FALSE, 0x400000);
		//swappedlorom = ScoreLoROM(FALSE, 0x400000);

		//// set swapped here
		//if (max(swappedlorom, swappedhirom) >= max(lo_score, hi_score))
		//{
		//	ExtendedFormat = BIGFIRST;
		//	hi_score = swappedhirom;
		//	lo_score = swappedlorom;
		//	RomHeader += 0x400000;
		//}
		//else
			ExtendedFormat = SMALLFIRST;
	}

    Interleaved = Settings.ForceInterleaved || Settings.ForceInterleaved2;
    if (Settings.ForceLoROM || (!Settings.ForceHiROM && lo_score >= hi_score))
    {
		LoROM = TRUE;
		HiROM = FALSE;
		// Ignore map type byte if not 0x2x or 0x3x
		if ((ROM [0x7fd5] & 0xf0) == 0x20 || (ROM [0x7fd5] & 0xf0) == 0x30)
		{
		    switch (ROM [0x7fd5] & 0xf)
		    {
			    case 1:
				if (strncmp ((char *) &ROM [0x7fc0], "TREASURE HUNTER G", 17) != 0)
				    Interleaved = TRUE;
				break;
			    case 2:
#if 0
					if (!Settings.ForceLoROM &&
					    strncmp ((char *) &ROM [0x7fc0], "SUPER FORMATION SOCCE", 21) != 0 &&
					    strncmp ((char *) &ROM [0x7fc0], "Star Ocean", 10) != 0)
					{
					    LoROM = FALSE;
					    HiROM = TRUE;
					}
#endif
				break;
			    case 5:
					Interleaved = TRUE;
					Tales = TRUE;
				break;
		    }
		}
    }
    else
    {
		if ((ROM [0xffd5] & 0xf0) == 0x20 || (ROM [0xffd5] & 0xf0) == 0x30)
		{
		    switch (ROM [0xffd5] & 0xf)
		    {
			    case 0:
			    case 3:
				Interleaved = TRUE;
				break;
		    }
		}
		LoROM = FALSE;
		HiROM = TRUE;		
    }


if (!Settings.ForceHiROM && !Settings.ForceLoROM &&
	!Settings.ForceInterleaved && !Settings.ForceInterleaved2 &&
	!Settings.ForceNotInterleaved && !Settings.ForcePAL &&
	!Settings.ForceSuperFX && !Settings.ForceDSP1 &&
	!Settings.ForceSA1 && !Settings.ForceC4 &&
	!Settings.ForceSDD1)
    {
    
	    if(strncmp((char *) &ROM [0x7fc0], "YOSHI'S ISLAND", 14) == 0&&(*(uint16*)&ROM[0x7FDE])==57611&&ROM[0x10002]==0xA9)
		{
			Interleaved=true;
			Settings.ForceInterleaved2=true;
		}

    
		if (strncmp ((char *) &ROM [0x7fc0], "YUYU NO QUIZ DE GO!GO!", 22) == 0)
		{
		    LoROM = TRUE;
		    HiROM = FALSE;
		    Interleaved = FALSE;		  
		}
		else 
		if (strncmp ((char *) &ROM [0x7fc0], "SP MOMOTAROU DENTETSU2", 22) == 0)
		{
		    LoROM = TRUE;
		    HiROM = FALSE;
		    Interleaved = FALSE;		  
		}
		else 
		if (CalculatedSize == 0x100000 && 
		    strncmp ((char *) &ROM [0xffc0], "WWF SUPER WRESTLEMANIA", 22) == 0)
		{
		    int cvcount;		
		    
		    memmove (&ROM[0x100000] , ROM, 0x100000);
		    
		    for (cvcount = 0; cvcount < 16; cvcount++)
		    {
				memmove (&ROM[0x8000 * cvcount], &ROM[0x10000 * cvcount + 0x100000 + 0x8000], 0x8000);
				
				memmove (&ROM[0x8000 * cvcount + 0x80000], &ROM[0x10000 * cvcount + 0x100000], 0x8000);
				
		    }
		    LoROM = TRUE;
		    HiROM = FALSE;
	    	ZeroMemory (ROM + CalculatedSize, MAX_ROM_SIZE - CalculatedSize);
		}
    }
   
    if (!Settings.ForceNotInterleaved && Interleaved)
    {
		S9xMessage(0,0,"interleaved ROM");

    	DeInterleavedRom(Tales);

#ifdef __PSP__
		psp_msg(CONV_DONE, MSG_DEFAULT);
#else
		S9xMessage(0,0,"conversion done");
#endif

		hi_score = ScoreHiROM (FALSE);
		lo_score = ScoreLoROM (FALSE);

		if ((HiROM &&
		     (lo_score >= hi_score || hi_score < 0)) ||
		    (LoROM && 
		     (hi_score > lo_score || lo_score < 0)))
		{
		    if (retry_count == 0)
		    {
#ifdef __PSP__
				psp_msg(ROM_LIED, MSG_DEFAULT);
#else
				S9xMessage(0,0,"ROM lied about its type! Trying again.");
#endif

				Settings.ForceNotInterleaved = TRUE;
				Settings.ForceInterleaved = FALSE;
				retry_count++;
				//goto again;
				return 1;
		    }
		}
    }
    
    FreeSDD1Data ();
    InitROM (Tales);
    
    return 0;
}

#endif
/**********************************************************************************************/
/* LoadROM()                                                                                  */
/* This function loads a Snes-Backup image                                                    */
/**********************************************************************************************/
extern char rom_filename[256];
extern char shortrom_filename[64];
bool8 CMemory::LoadROM (const char *filename)
{	
    unsigned long FileSize = 0;
    int retry_count = 0;
    //STREAM ROMFile;                
    FILE *ROMFile;
    int current_pos;
    char ext [4 + 1];
    char fname [256 + 1];
    char name [256 + 1];
    int i;
    
    

    memset (&SNESGameFixes, 0, sizeof(SNESGameFixes));
    SNESGameFixes.SRAMInitialValue = 0x60;

    memset (bytes0x2000, 0, 0x2000);
    CPUPack.CPU.TriedInterleavedMode2 = FALSE;

    CalculatedSize = 0;
    

again:
//    _splitpath (filename, drive, dir, name, ext);
//    _makepath (fname, drive, dir, name, ext);
	strcpy(name,shortrom_filename);
	strcpy(fname,filename);
	char *p = strrchr(rom_filename,'.');
	if (p == NULL) return FALSE;
	strcpy(ext, p + 1);
	
	

  
  
	
#if defined(__WIN32__)
    memmove (&ext [0], &ext[1], 4);
#endif

    int32 TotalFileSize = 0;
    

#ifdef UNZIP_SUPPORT

    if (strcasecmp (ext, "zip") == 0)
    {
    	//S9xMessage(0,0,"File is zipped...\nSearching for a valide ROM inside");
		//bool8 LoadZip (const char *, int32 *, int32 *);

		//if (!LoadZip (fname, &TotalFileSize, &HeaderCount))
	    //	return (FALSE);
	    unzFile zip_file = 0;    
	    unz_file_info unzinfo;
	    char snes_file[256]; //yoyo
		char *p,*q;
		char *pext;
		uint8 *ptr=(uint8*)ROM;
		

	    
	   // S9xMessage(1,0,"yo zip1");  
	    zip_file = unzOpen(filename);
	 //   S9xMessage(1,0,"yo zip2");  
	
	    			                              
        if (!zip_file) return FALSE;                                        

   //     S9xMessage(1,0,"yo zip3");  
        unzGoToFirstFile (zip_file);                    
 //       S9xMessage(1,0,"yo zip4");  
 
 

        for (;;) 
        {
        	if (unzGetCurrentFileInfo(zip_file, &unzinfo, snes_file, sizeof(snes_file), NULL, NULL, NULL, NULL) != UNZ_OK) return FALSE;

            p = (char*)(strrchr(snes_file, '.') + 1);
	
                                
                if (strcasecmp(p, "smc") == 0) break;
                if (strcasecmp(p, "sfc") == 0) break;
                if (strcasecmp(p, "swc") == 0) break;
                if (strcasecmp(p, "bin") == 0) break;
                if (strcasecmp(p, "fig") == 0) break;
//                if (compare(p, ".1") == 0) break;
                if (unzGoToNextFile(zip_file) != UNZ_OK) return FALSE;
         }     
//         S9xMessage(1,0,"opening in zip file ,");             
         unzOpenCurrentFile (zip_file);
//         S9xMessage(1,0,snes_file);


         
		 FileSize = unzinfo.uncompressed_size;	 	 	 
//		 char tmp[64];
//         sprintf(tmp,"size : %d",FileSize);
//	     S9xMessage(1,0,tmp);

		 {char str[64];sprintf(str,psp_msg_string(LOADING_ROM),FileSize>>10);msgBoxLines(str,0);msgBoxLines(str,0); }
		 
		 		unzReadCurrentFile(zip_file,(void*)(ptr), FileSize, psp_showProgressBar);

		 
//		 S9xMessage(1,0,"ok");

		 
         unzCloseCurrentFile (zip_file);
         unzClose (zip_file);    
         
         int calc_size = (FileSize / 0x2000) * 0x2000;

	    if ((FileSize - calc_size == 512 && !Settings.ForceNoHeader) ||
			Settings.ForceHeader)
	    {
			memmove (ptr, ptr + 512, calc_size);
			
			HeaderCount++;
			FileSize -= 512;
	    }
	    TotalFileSize += FileSize;
		strcpy (ROMFilename, fname);		
    }
    else
#endif
    {
    
    
	//if ((ROMFile = OPEN_STREAM (fname, "rb")) == NULL)
	    //return (FALSE);
	 ROMFile = fopen(fname,"rb");
	 if (!ROMFile) return false;
	    
	strcpy (ROMFilename, fname);

	HeaderCount = 0;
	uint8 *ptr = ROM;	
	bool8 more = FALSE;
	do
	{								
	    //FileSize = READ_STREAM (ptr, MAX_ROM_SIZE + 0x200 - (ptr - ROM), ROMFile);
	    fseek(ROMFile,0,SEEK_END);
	    FileSize=ftell(ROMFile);
	    fseek(ROMFile,0,SEEK_SET);
			current_pos=FileSize;
			
			{char str[64];sprintf(str,psp_msg_string(LOADING_ROM),FileSize>>10);msgBoxLines(str,0);msgBoxLines(str,0); }
			
			while (current_pos>0) {    	    
	    	psp_showProgressBar(current_pos,FileSize);
	    	if (current_pos>0x8000) {
	    		fread(ptr,0x8000,1,ROMFile);
	    		ptr+=0x8000;
	    		current_pos-=0x8000;
	    	} else {
	    		fread(ptr,current_pos,1,ROMFile);
	    		ptr+=current_pos;
	    		current_pos=0;
	    	}
	    }
	    psp_showProgressBar(FileSize-current_pos,FileSize);
	    ptr=ROM;
	    fclose(ROMFile);
	    	    
	    //CLOSE_STREAM (ROMFile);	    	  
	    
	    int calc_size = (FileSize / 0x2000) * 0x2000;

	    if ((FileSize - calc_size == 512 && !Settings.ForceNoHeader) ||	Settings.ForceHeader) {
				memmove (ptr, ptr + 512, calc_size);			
				HeaderCount++;
				FileSize -= 512;
	    }	    
	    TotalFileSize += FileSize;

	    int len;
	    if (ptr - ROM < MAX_ROM_SIZE + 0x200 && (isdigit (ext [0]) && ext [1] == 0 && ext [0] < '9')) {
				more = TRUE;
				ext [0]++;
#if defined(__WIN32__)||defined(__GP32__)||defined(__PSP__)
                memmove (&ext [1], &ext [0], 4);
                ext [0] = '.';
#endif
			
		//_makepath (fname, drive, dir, name, ext);
	    }
	    else
	    if (ptr- ROM < MAX_ROM_SIZE + 0x200 &&
		(((len = strlen (name)) == 7 || len == 8) &&
		 strncasecmp (name, "sf", 2) == 0 &&
		 isdigit (name [2]) && isdigit (name [3]) && isdigit (name [4]) &&
		 isdigit (name [5]) && isalpha (name [len - 1])))
	    {
		more = TRUE;
		name [len - 1]++;
#if defined(__WIN32__)||defined(__GP32__)||defined(__PSP__)
                memmove (&ext [1], &ext [0], 4);
                ext [0] = '.';
#endif
		
		//_makepath (fname, drive, dir, name, ext);
	    }
	    else
		{ more = FALSE; break;}
	//} while (more && (ROMFile = OPEN_STREAM (fname, "rb")) != NULL);
	} while (more && (ROMFile = fopen (fname, "rb")) != NULL);
    }

    /*if (HeaderCount == 0)
	S9xMessage (S9X_INFO, S9X_HEADERS_INFO, "No ROM file header found.");
    else
    {
	if (HeaderCount == 1)
	    S9xMessage (S9X_INFO, S9X_HEADERS_INFO,
			"Found ROM file header (and ignored it).");
	else
	    S9xMessage (S9X_INFO, S9X_HEADERS_INFO,
			"Found multiple ROM file headers (and ignored them).");
    }*/

    CheckForIPSPatch (filename, HeaderCount != 0, TotalFileSize,".ips");
    CheckForIPSPatch (filename, HeaderCount != 0, TotalFileSize,".ips1");
    CheckForIPSPatch (filename, HeaderCount != 0, TotalFileSize,".ips2");
    CheckForIPSPatch (filename, HeaderCount != 0, TotalFileSize,".ips3");
    CheckForIPSPatch (filename, HeaderCount != 0, TotalFileSize,".ips4");
    
    
  

    // More 
#ifndef _BSX_151_
	if (LoadROMMore(TotalFileSize,retry_count))
#else
	if (LoadROMMore_151(TotalFileSize,retry_count))
#endif
    {
    	goto again;
    }
	
    S9xLoadCheatFile (S9xGetSaveFilename(".cht"));
    S9xInitCheatData ();
    S9xApplyCheats ();

    S9xReset ();

    return (TRUE);
}

void S9xDeinterleaveMode2 ()
{
    //S9xMessage (S9X_INFO, S9X_ROM_INTERLEAVED_INFO,
		//"ROM image is in interleaved format - converting...");
	S9xMessage(0,0,"deinterleaved mofe 2");

    int nblocks = Memory.CalculatedSize >> 15;
    int step = 64;

    while (nblocks <= step)
	step >>= 1;
	
    nblocks = step;
    uint8 blocks [256];
    int i;

    for (i = 0; i < nblocks * 2; i++)
    {
	blocks [i] = (i & ~0x1e) | ((i & 2) << 2) | ((i & 4) << 2) |
		    ((i & 8) >> 2) | ((i & 16) >> 2);
    }

    uint8 *tmp = (uint8 *) malloc (0x8000);

    if (tmp)
    {

	for (i = 0; i < nblocks * 2; i++)
	{
	    for (int j = i; j < nblocks * 2; j++)
	    {
		if (blocks [j] == i)
		{
			if (blocks[j]!=blocks[i])
			{
			    memmove (tmp, &ROM [(uint32)(blocks [j]) * 0x8000], 0x8000);
			    
			    
			    memcpy(ROM+blocks [j] * 0x8000UL,ROM + blocks [i] * 0x8000UL, 0x8000);
			    
		    	memcpy(ROM+blocks [i] * 0x8000UL,tmp, 0x8000);
		    	
		    }
		    uint8 b = blocks [j];
		    blocks [j] = blocks [i];
		    blocks [i] = b;
		    break;
		}
	    }
	}
	free ((char *) tmp);
    }
    Memory.InitROM (FALSE);
    S9xReset ();
}

#ifdef _BSX_151_
void CMemory::ParseSNESHeader (uint8 *RomHeader)
{
	bool8	bs = Settings.BS & !Settings.BSXItself;

	strncpy(ROMName, (char *) &RomHeader[0x10], ROM_NAME_LEN - 1);
	if (bs)
		memset(ROMName + 16, 0x20, ROM_NAME_LEN - 17);

	if (bs)
	{
		if (!(((RomHeader[0x29] & 0x20) && CalculatedSize <  0x100000) ||
			 (!(RomHeader[0x29] & 0x20) && CalculatedSize == 0x100000)))
			printf("BS: Size mismatch\n");

		// FIXME
		int	p = 0;
		while ((1 << p) < (int) CalculatedSize)
			p++;
		ROMSize = p - 10;
	}
	else
		ROMSize = RomHeader[0x27];

	SRAMSize  = bs ? 5 /* BS-X */    : RomHeader[0x28];
	ROMSpeed  = bs ? RomHeader[0x28] : RomHeader[0x25];
	ROMType   = bs ? 0xE5 /* BS-X */ : RomHeader[0x26];
//	ROMRegion = bs ? 0               : RomHeader[0x29];//remove azz 080517

	ROMChecksum           = RomHeader[0x2E] + (RomHeader[0x2F] << 8);
	ROMComplementChecksum = RomHeader[0x2C] + (RomHeader[0x2D] << 8);

	memmove(ROMId, &RomHeader[0x02], 4);

	if (RomHeader[0x2A] == 0x33)
		memmove(CompanyId, &RomHeader[0x00], 2);
	else
		sprintf(CompanyId, "%02X", RomHeader[0x2A]);
}
void CMemory::BS_151()
{
	CalculatedChecksum = 0;

	if (HiROM)
    {
		if (Settings.BS)
			/* Do nothing */;
		//else
		//if (Settings.SPC7110)
		//	Map_SPC7110HiROMMap();
		//else
		//if (ExtendedFormat != NOPE)
		//	Map_ExtendedHiROMMap();
		//else
		//if (Multi.cartType == 3)
		//	Map_SameGameHiROMMap();
		//else
		//	Map_HiROMMap();
    }
    else
    {
		if (Settings.BS)
			/* Do nothing */;
/*		else
		if (Settings.SETA && Settings.SETA != ST_018)
			Map_SetaDSPLoROMMap();
		else
		if (Settings.SuperFX)
			Map_SuperFXLoROMMap();
		else
		if (Settings.SA1)
			Map_SA1LoROMMap();
		else
		if (Settings.SDD1)
			Map_SDD1LoROMMap();
		else
		if (ExtendedFormat != NOPE)
			Map_JumboLoROMMap();
		else
		if (strncmp(ROMName, "WANDERERS FROM YS", 17) == 0)
			Map_NoMAD1LoROMMap();
		else
		if (strncmp(ROMName, "SOUND NOVEL-TCOOL", 17) == 0 ||
			strncmp(ROMName, "DERBY STALLION 96", 17) == 0)
			Map_ROM24MBSLoROMMap();
		else
		if (strncmp(ROMName, "THOROUGHBRED BREEDER3", 21) == 0 ||
			strncmp(ROMName, "RPG-TCOOL 2", 11) == 0)
			Map_SRAM512KLoROMMap();
		else
		if (strncmp(ROMName, "ADD-ON BASE CASSETE", 19) == 0)
		{
			if (Multi.cartType == 4)
			{
				SRAMSize = Multi.sramSizeA;
				Map_SufamiTurboLoROMMap();
			}
			else
			{
				SRAMSize = 5;
				Map_SufamiTurboPseudoLoROMMap();
			}
		}
		else
			Map_LoROMMap()*/;
    }

	/*Checksum_Calculate();

	bool8 isChecksumOK = (ROMChecksum + ROMComplementChecksum == 0xffff) &
						 (ROMChecksum == CalculatedChecksum);*/

	//// Build more ROM information

	// CRC32
	if (!Settings.BS || Settings.BSXItself) // Not BS Dump
		ROMCRC32 = caCRC32(ROM, CalculatedSize);
	else // Convert to correct format before scan
	{
		int offset = HiROM ? 0xffc0 : 0x7fc0;
		// Backup
		uint8 BSMagic0 = ROM[offset + 22],
			  BSMagic1 = ROM[offset + 23];
		// uCONSRT standard
		ROM[offset + 22] = 0x42;
		ROM[offset + 23] = 0x00;
		// Calc
		ROMCRC32 = caCRC32(ROM, CalculatedSize);
		// Convert back
		ROM[offset + 22] = BSMagic0;
		ROM[offset + 23] = BSMagic1;
	}

	// NTSC/PAL
	if (Settings.ForceNTSC)
		Settings.PAL = FALSE;
	else
	if (Settings.ForcePAL)
		Settings.PAL = TRUE;
	else
	//if (!Settings.BS && (ROMRegion >= 2) && (ROMRegion <= 12))
		Settings.PAL = TRUE;
	//else
	//	Settings.PAL = FALSE;

	if (Settings.PAL)
	{
		Settings.FrameTime = Settings.FrameTimePAL;
		ROMFramesPerSecond = 50;
	}
	else
	{
		Settings.FrameTime = Settings.FrameTimeNTSC;
		ROMFramesPerSecond = 60;
	}

	// truncate cart name
	ROMName[ROM_NAME_LEN - 1] = 0;
	if (strlen(ROMName))
	{
		char *p = ROMName + strlen(ROMName);
		if (p > ROMName + 21 && ROMName[20] == ' ')
			p = ROMName + 21;
		while (p > ROMName && *(p - 1) == ' ')
			p--;
		*p = 0;
	}

	// SRAM size
	SRAMMask = SRAMSize ? ((1 << (SRAMSize + 3)) * 128) - 1 : 0;

	//// checksum
	//if (!isChecksumOK || ((uint32) CalculatedSize > (uint32) (((1 << (ROMSize - 7)) * 128) * 1024)))
	//{
	//	if (Settings.DisplayColor == 0xffff || Settings.DisplayColor != BUILD_PIXEL(31, 0, 0))
	//	{
	//		Settings.DisplayColor = BUILD_PIXEL(31, 31, 0);
	//		SET_UI_COLOR(255, 255, 0);
	//	}
	//}

	//if (Multi.cartType == 4)
	//{
	//	Settings.DisplayColor = BUILD_PIXEL(0, 16, 31);
	//	SET_UI_COLOR(0, 128, 255);
	//}

	//// Initialize emulation

	//Timings.H_Max_Master = SNES_CYCLES_PER_SCANLINE;
	//Timings.H_Max        = Timings.H_Max_Master;
	//Timings.HBlankStart  = SNES_HBLANK_START_HC;
	//Timings.HBlankEnd    = SNES_HBLANK_END_HC;
	//Timings.HDMAInit     = SNES_HDMA_INIT_HC;
	//Timings.HDMAStart    = SNES_HDMA_START_HC;
	//Timings.RenderPos    = SNES_RENDER_START_HC;
	//Timings.V_Max_Master = Settings.PAL ? SNES_MAX_PAL_VCOUNTER : SNES_MAX_NTSC_VCOUNTER;
	//Timings.V_Max        = Timings.V_Max_Master;
	/* From byuu: The total delay time for both the initial (H)DMA sync (to the DMA clock),
	   and the end (H)DMA sync (back to the last CPU cycle's mcycle rate (6, 8, or 12)) always takes between 12-24 mcycles.
	   Possible delays: { 12, 14, 16, 18, 20, 22, 24 }
	   XXX: Snes9x can't emulate this timing :( so let's use the average value... */
	//Timings.DMACPUSync   = 18;

	//IAPU.OneCycle = SNES_APU_ONE_CYCLE_SCALED;

//	CPU.FastROMSpeed = 0;
//	ResetSpeedMap();

//	IPPU.TotalEmulatedFrames = 0;

	Settings.Shutdown = Settings.ShutdownMaster;

	//// Hack games

	//ApplyROMFixes();

	//// Show ROM information
	//char displayName[ROM_NAME_LEN];

	//strcpy(RawROMName, ROMName);
	//sprintf(displayName, "%s", SafeANK(ROMName));
	//sprintf(ROMName, "%s", Safe(ROMName));
	//sprintf(ROMId, "%s", Safe(ROMId));
	//sprintf(CompanyId, "%s", Safe(CompanyId));

	//sprintf(String, "\"%s\" [%s] %s, %s, Type: %s, Mode: %s, TV: %s, S-RAM: %s, ROMId: %s Company: %2.2s CRC32: %08X",
	//	displayName, isChecksumOK ? "checksum ok" : ((Multi.cartType == 4) ? "no checksum" : "bad checksum"),
	//	MapType(), Size(), KartContents(), MapMode(), TVStandard(), StaticRAMSize(), ROMId, CompanyId, ROMCRC32);
	//S9xMessage(S9X_INFO, S9X_ROM_INFO, String);

	// XXX: Please remove port specific codes
#ifdef __WIN32__
#ifndef _XBOX
	EnableMenuItem(GUI.hMenu, IDM_ROM_INFO, MF_ENABLED);
#endif
#ifdef RTC_DEBUGGER
	if (Settings.SPC7110RTC)
		EnableMenuItem(GUI.hMenu, IDM_7110_RTC, MF_ENABLED);
	else
		EnableMenuItem(GUI.hMenu, IDM_7110_RTC, MF_GRAYED);
#endif
#endif

	Settings.ForceHiROM = Settings.ForceLoROM = FALSE;
	Settings.ForceHeader = Settings.ForceNoHeader = FALSE;
	Settings.ForceInterleaved = Settings.ForceNotInterleaved = Settings.ForceInterleaved2 = FALSE;

	//if (stopMovie)
	//	S9xMovieStop(TRUE);

	//if (PostRomInitFunc)
	//	PostRomInitFunc();

 //   S9xVerifyControllers();
}
#endif
void InitNewMap();
void CMemory::InitROM (bool8 Interleaved)
{
#ifndef ZSNES_FX
    SuperFX.nRomBanks = CalculatedSize >> 15;
#endif
    Settings.MultiPlayer5Master = Settings.MultiPlayer5;
    Settings.MouseMaster = Settings.Mouse;
    Settings.SuperScopeMaster = Settings.SuperScope;
    Settings.DSP1Master = Settings.ForceDSP1;
    Settings.SuperFX = FALSE;
    Settings.SA1 = FALSE;
    Settings.C4 = FALSE;
    Settings.SDD1 = FALSE;
    Settings.SRTC = FALSE;

	Settings.BS = FALSE;//add azz 080517

#ifdef _BSX_151_
	memset(ROMId, 0, 5);
	memset(CompanyId, 0, 3);

	uint8	*RomHeader = ROM + 0x7FB0;
	if (ExtendedFormat == BIGFIRST)
		RomHeader += 0x400000;
	if (HiROM)
		RomHeader += 0x8000;

	S9xInitBSX(); // Set BS header before parsing

	ParseSNESHeader(RomHeader);
	Map_Initialize ();
	if (Settings.BS)
	{
		{
			int offset = HiROM ? 0xffc0 : 0x7fc0;
			// Backup
			uint8 BSMagic0 = ROM[offset + 22],
				  BSMagic1 = ROM[offset + 23];
			// uCONSRT standard
			ROM[offset + 22] = 0x42;
			ROM[offset + 23] = 0x00;
			// Calc
			ROMCRC32 = caCRC32(ROM, CalculatedSize);
			// Convert back
			ROM[offset + 22] = BSMagic0;
			ROM[offset + 23] = BSMagic1;
		}
		BS_151();
	}
#endif
	if (!Settings.BS)
	{
    ZeroMemory (BlockIsRAM, MEMMAP_NUM_BLOCKS);
    ZeroMemory (BlockIsROM, MEMMAP_NUM_BLOCKS);

    memset (ROMId, 0, 5);
    memset (CompanyId, 0, 3);
	}

    if (!Settings.BS)
	if (Memory.HiROM)
	{
	Memory.SRAMSize = ROM [0xffd8];
	strncpy (ROMName, (char *) &ROM[0xffc0], ROM_NAME_LEN - 1);
	ROMSpeed = ROM [0xffd5];
	ROMType = ROM [0xffd6];
	ROMSize = ROM [0xffd7];
	ROMChecksum = ROM [0xffde] + (ROM [0xffdf] << 8);
	ROMComplementChecksum = ROM [0xffdc] + (ROM [0xffdd] << 8);
	
	memmove (ROMId, &ROM [0xffb2], 4);
	memmove (CompanyId, &ROM [0xffb0], 2);
	

	// Try to auto-detect the DSP1 chip
	if (!Settings.ForceNoDSP1 &&
	    (ROMType & 0xf) >= 3 && (ROMType & 0xf0) == 0)
	    Settings.DSP1Master = TRUE;

	Settings.SDD1 = Settings.ForceSDD1;
	if ((ROMType & 0xf0) == 0x40){
	    Settings.SDD1 = !Settings.ForceNoSDD1;
	}
  	    
	
	if (Settings.BS)
#ifdef _BSX_151_
		;
#else
	    BSHiROMMap () ;//remove azz 080517;
#endif
	else
	if ((ROMSpeed & ~0x10) == 0x25)
	    TalesROMMap (Interleaved);
	else 
	if ((ROMSpeed & ~0x10) == 0x22 &&
	    strncmp (ROMName, "Super Street Fighter", 20) != 0)
	{
	    AlphaROMMap ();
	}
	else{
		
	    HiROMMap ();
	    
	    }
    }
    else
	{
	Memory.HiROM = FALSE;
	Memory.SRAMSize = ROM [0x7fd8];
	ROMSpeed = ROM [0x7fd5];
	ROMType = ROM [0x7fd6];
	ROMSize = ROM [0x7fd7];
	ROMChecksum = ROM [0x7fde] + (ROM [0x7fdf] << 8);
	ROMComplementChecksum = ROM [0x7fdc] + (ROM [0x7fdd] << 8);
	memmove (ROMId, &ROM [0x7fb2], 4);
	memmove (CompanyId, &ROM [0x7fb0], 2);

	strncpy (ROMName, (char *) &ROM[0x7fc0], ROM_NAME_LEN - 1);
	Settings.SuperFX = Settings.ForceSuperFX;

	if ((ROMType & 0xf0) == 0x10)
	    Settings.SuperFX = !Settings.ForceNoSuperFX;

	// Try to auto-detect the DSP1 chip
	if (!Settings.ForceNoDSP1 &&
	    (ROMType & 0xf) >= 3 && (ROMType & 0xf0) == 0)
	    Settings.DSP1Master = TRUE;

	Settings.SDD1 = Settings.ForceSDD1;
	if ((ROMType & 0xf0) == 0x40)
	    Settings.SDD1 = !Settings.ForceNoSDD1;

	/*if (Settings.SDD1)
	    S9xLoadSDD1Data ();*/

	Settings.C4 = Settings.ForceC4;
	if ((ROMType & 0xf0) == 0xf0 &&
            (strncmp (ROMName, "MEGAMAN X", 9) == 0 ||
             strncmp (ROMName, "ROCKMAN X", 9) == 0))
	{
	    Settings.C4 = !Settings.ForceNoC4;
	}

	if (Settings.SuperFX)
	{
	    //::SRAM = ROM + 1024 * 1024 * 4;
	    SuperFXROMMap ();
	    Settings.MultiPlayer5Master = FALSE;
	    //Settings.MouseMaster = FALSE;
	    //Settings.SuperScopeMaster = FALSE;
	    Settings.DSP1Master = FALSE;
	    Settings.SA1 = FALSE;
	    Settings.C4 = FALSE;
	    Settings.SDD1 = FALSE;
	}
	else
	if (Settings.ForceSA1 ||
	    (!Settings.ForceNoSA1 && (ROMSpeed & ~0x10) == 0x23 && 
	     (ROMType & 0xf) > 3 && (ROMType & 0xf0) == 0x30))
	{
	    Settings.SA1 = TRUE;
	    Settings.MultiPlayer5Master = FALSE;
	    //Settings.MouseMaster = FALSE;
	    //Settings.SuperScopeMaster = FALSE;
	    Settings.DSP1Master = FALSE;
	    Settings.C4 = FALSE;
	    Settings.SDD1 = FALSE;
	    SA1ROMMap ();
	}
	else
	if ((ROMSpeed & ~0x10) == 0x25)
	    TalesROMMap (Interleaved);
	else
	if (strncmp ((char *) &ROM [0x7fc0], "SOUND NOVEL-TCOOL", 17) == 0 ||
	    strncmp ((char *) &ROM [0x7fc0], "DERBY STALLION 96", 17) == 0)
	{
	    LoROM24MBSMap ();
	    Settings.DSP1Master = FALSE;
	}
	else
	if (strncmp ((char *) &ROM [0x7fc0], "THOROUGHBRED BREEDER3", 21) == 0 ||
	    strncmp ((char *) &ROM [0x7fc0], "RPG-TCOOL 2", 11) == 0)
	{
	    SRAM512KLoROMMap ();
	    Settings.DSP1Master = FALSE;
	}
	else
	if (strncmp ((char *) &ROM [0x7fc0], "DEZAEMON  ", 10) == 0)
	{
	    Settings.DSP1Master = FALSE;
	    SRAM1024KLoROMMap ();
	}
	else
	if (strncmp ((char *) &ROM [0x7fc0], "ADD-ON BASE CASSETE", 19) == 0)
	{
	    Settings.MultiPlayer5Master = FALSE;
	    Settings.MouseMaster = FALSE;
	    Settings.SuperScopeMaster = FALSE;
	    Settings.DSP1Master = FALSE;
 	    SufamiTurboLoROMMap(); 
	    Memory.SRAMSize = 3;
	}
	else
	if ((ROMSpeed & ~0x10) == 0x22 &&
	    strncmp (ROMName, "Super Street Fighter", 20) != 0)
	{
	    AlphaROMMap ();
	}
	else
	    LoROMMap ();
    }

    int power2 = 0;
    int size = CalculatedSize;

    while (size >>= 1)
	power2++;

    size = 1 << power2;
    uint32 remainder = CalculatedSize - size;

    /*uint32 */sum1 = 0;
    /*uint32 */sum2 = 0;

    int i;

    for (i = 0; i < size; i++)
	sum1 += ROM [i];

    for (i = 0; i < (int) remainder; i++)
	sum2 += ROM [size + i];

    if (remainder)
    {
	//for Tengai makyou
	if (CalculatedSize == 0x500000 && Memory.HiROM && 
	    strncmp ((const char *)&ROM[0xffb0], "18AZ", 4) == 0 &&
	    !memcmp(&ROM[0xffd5], "\x3a\xf9\x0d\x03\x00\x33\x00", 7))
	    sum1 += sum2;
	else
	    sum1 += sum2 * (size / remainder);
    }

    sum1 &= 0xffff;
    
    //now take a CRC32
    ROMCRC32 = caCRC32(&ROM[0], CalculatedSize);
	g_ROMCRC32=ROMCRC32;
    if (Settings.ForceNTSC)
	Settings.PAL = FALSE;
    else
    if (Settings.ForcePAL)
	Settings.PAL = TRUE;
    else
    if (Memory.HiROM)
	// Country code
	Settings.PAL = ROM [0xffd9] >= 2;
    else
	Settings.PAL = ROM [0x7fd9] >= 2;
    
    if (Settings.PAL)
    {
	Settings.FrameTime = Settings.FrameTimePAL;
	ROMFramesPerSecond = 50;
    }
    else
    {
	Settings.FrameTime = Settings.FrameTimeNTSC;
	ROMFramesPerSecond = 60;
    }
	
    ROMName[ROM_NAME_LEN - 1] = 0;
    if (strlen (ROMName))
    {
	char *p = ROMName + strlen (ROMName) - 1;

	while (p > ROMName && *(p - 1) == ' ')
	    p--;
	*p = 0;
    }
    if (Settings.SuperFX)
    {
	SRAMMask = 0xffff;
	Memory.SRAMSize = 16;
    }
    else
    {
	SRAMMask = Memory.SRAMSize ?
		    ((1 << (Memory.SRAMSize + 3)) * 128) - 1 : 0;
    }

    (IAPUuncached.OneCycle) = ONE_APU_CYCLE;
    Settings.Shutdown = Settings.ShutdownMaster;

    ApplyROMFixes ();

#ifdef __PSP__
    sprintf (String, "\"%s\" [%s] %s, %s\n%s: %s, %s: %s, TV: %s, S-RAM: %s",
	     ROMName,
	     (ROMChecksum + ROMComplementChecksum != 0xffff ||
	      ROMChecksum != sum1) ? "bad checksum" : "checksum ok",
	     MapType (),
	     Size (),
	     psp_msg_string(TYPE),
	     KartContents (),
	     psp_msg_string(MODE),
	     MapMode (),
	     TVStandard (),
	     StaticRAMSize ());
#else
    sprintf (String, "\"%s\" [%s] %s, %s\nType: %s, Mode: %s, TV: %s, S-RAM: %s",
	     ROMName,
	     (ROMChecksum + ROMComplementChecksum != 0xffff ||
	      ROMChecksum != sum1) ? "bad checksum" : "checksum ok",
	     MapType (),
	     Size (),
	     KartContents (),
	     MapMode (),
	     TVStandard (),
	     StaticRAMSize ());
#endif

    for (i =0; i < 4; i++)
    	if (strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ", ROMName[i]) == NULL) break;
    if (i == 4) sprintf (String, "%s\nROMId: %s  %s: %2.2s",
			String, ROMId, psp_msg_string(COMPANY), CompanyId);

    InitNewMap();
    S9xMessage (0/*S9X_INFO*/, S9X_ROM_INFO, String);
}


bool8 CMemory::LoadSRAM (char *filename)
{
    int lsize = Memory.SRAMSize ?
	       (1 << (Memory.SRAMSize + 3)) * 128 : 0;	       	 

    memset (SRAM, SNESGameFixes.SRAMInitialValue, 0x20000);
    

  /*debug*/
//  DBG_BREAK

 

 

    if (lsize > 0x20000)
	lsize = 0x20000;
    
    if (lsize)
    {
		FILE *file;
//		STREAM SRAMFile;			
//		S9xMessage(1,0,filename);
		if ((file = fopen (filename, "rb")))
//		if ((SRAMFile = OPEN_STREAM (filename, "rb")))	   
		{
		    int len = fread ((char*) SRAM, 1, 0x20000, file);
		    fclose (file);
//				int len = READ_STREAM ((char*) SRAM, 0x20000, SRAMFile);
//		    CLOSE_STREAM (SRAMFile);
		    		      		    
		    if (len - lsize == 512)
		    {
				// S-RAM file has a header - remove it
				memmove (SRAM, SRAM + 512, lsize);
		    }
		    
		    if (len == lsize + SRTC_SRAM_PAD)
		    {
				S9xSRTCPostLoadState ();
				S9xResetSRTC ();
				rtc.index = -1;
				rtc.mode = MODE_READ;
		    }
		    else S9xHardResetSRTC ();	
			return (TRUE);
		}
#ifdef _BSX_151_
		else //add azz 080517
		if (Settings.BS && !Settings.BSXItself)
		{
			// The BS game's SRAM was not found
			// Try to read BS-X.srm instead
			char	path[_MAX_PATH + 1];
			int size=lsize;

			strcpy(path, LaunchDir);
			//strcat(path, SLASH_STR);
			strcat(path, "BS-X.srm");
			
			file = fopen(path, "rb");
			if (file)
			{
				int len = fread((char *) SRAM, 1, 0x20000, file);
				fclose(file);
				if (len - size == 512)
					memmove(SRAM, SRAM + 512, size);

#ifdef __PSP__
				psp_msg_string(SRAM_NOTFOUND, MSG_DEFAULT);
#else
				S9xMessage(S9X_INFO, S9X_ROM_INFO, "The SRAM file wasn't found: BS-X.srm was read instead.");
#endif
				S9xHardResetSRTC();
				return (TRUE);
			}
			else
			{
#ifdef __PSP__
				psp_msg_string(SRAM_BSX_NOTFOUND, MSG_DEFAULT);
#else
				S9xMessage(S9X_INFO, S9X_ROM_INFO, "The SRAM file wasn't found, BS-X.srm wasn't found either.");
#endif
				S9xHardResetSRTC();
				return (FALSE);
			}
		}
#endif
		S9xHardResetSRTC ();
		return (FALSE);
    }
    if (Settings.SDD1)
	S9xSDD1LoadLoggedData ();

    return (TRUE);
}

bool8 CMemory::SaveSRAM (char *filename)
{
	char text[32];
    int lsize = Memory.SRAMSize ?
	       (1 << (Memory.SRAMSize + 3)) * 128 : 0;
	
	  /*debug*/
//  DBG_BREAK
	       
    if (Settings.SRTC)
    {
		lsize += SRTC_SRAM_PAD;
		S9xSRTCPreSaveState ();
    }

    if (Settings.SDD1)
    {
		S9xSDD1SaveLoggedData ();
	}

    if (lsize > 0x20000)
	lsize = 0x20000;

    if (lsize && *ROMFilename)
    {
		FILE *file;
//		STREAM SRAMFile;
		//S9xMessage(0,0,"Saving SRAM");
		if ((file = fopen (filename, "wb")))
//		if ((SRAMFile = OPEN_STREAM (filename, "wb")))
		{
		    fwrite ((char *) SRAM, lsize, 1, file);
		    fclose (file);
//			WRITE_STREAM ((char*) SRAM, lsize, SRAMFile);
//		    CLOSE_STREAM (SRAMFile);
#if defined(__linux)
		    chown (filename, getuid (), getgid ());
#endif
		    return (TRUE);
		}
    }
    return (FALSE);
}

void CMemory::FixROMSpeed ()
{
    int c;

    for (c = 0x800; c < 0x1000; c++)
    {
	if (BlockIsROM [c])
	    MemorySpeed [c] = (uint8) CPUPack.CPU.FastROMSpeed;
    }
}

void CMemory::WriteProtectROM ()
{
    memmove ((void *) WriteMap, (void *) Map, MEMMAP_NUM_BLOCKS*4);
    for (int c = 0; c < 0x1000; c++)
    {
	if (BlockIsROM [c])
	    WriteMap [c] = (uint8 *) MAP_NONE;
    }
}

void CMemory::MapRAM ()
{
    int c;

    // Banks 7e->7f, RAM
    for (c = 0; c < 16; c++)
    {
	Map [c + 0x7e0] = RAM;
	Map [c + 0x7f0] = RAM + 0x10000;
	BlockIsRAM [c + 0x7e0] = TRUE;
	BlockIsRAM [c + 0x7f0] = TRUE;
	BlockIsROM [c + 0x7e0] = FALSE;
	BlockIsROM [c + 0x7f0] = FALSE;
    }

    // Banks 70->77, S-RAM
    for (c = 0; c < 0x80; c++)
    {
	Map [c + 0x700] = (uint8 *) MAP_LOROM_SRAM;
	BlockIsRAM [c + 0x700] = TRUE;
	BlockIsROM [c + 0x700] = FALSE;
    }
}

void CMemory::MapExtraRAM ()
{
    int c;

    // Banks 7e->7f, RAM
    for (c = 0; c < 16; c++)
    {
	Map [c + 0x7e0] = RAM;
	Map [c + 0x7f0] = RAM + 0x10000;
	BlockIsRAM [c + 0x7e0] = TRUE;
	BlockIsRAM [c + 0x7f0] = TRUE;
	BlockIsROM [c + 0x7e0] = FALSE;
	BlockIsROM [c + 0x7f0] = FALSE;
    }

    // Banks 70->73, S-RAM
    for (c = 0; c < 16; c++)
    {
	Map [c + 0x700] = ::SRAM;
	Map [c + 0x710] = ::SRAM + 0x8000;
	Map [c + 0x720] = ::SRAM + 0x10000;
	Map [c + 0x730] = ::SRAM + 0x18000;

	BlockIsRAM [c + 0x700] = TRUE;
	BlockIsROM [c + 0x700] = FALSE;
	BlockIsRAM [c + 0x710] = TRUE;
	BlockIsROM [c + 0x710] = FALSE;
	BlockIsRAM [c + 0x720] = TRUE;
	BlockIsROM [c + 0x720] = FALSE;
	BlockIsRAM [c + 0x730] = TRUE;
	BlockIsROM [c + 0x730] = FALSE;
    }
}
#ifdef _BSX_151_
//add azz 080517
void CMemory::map_WriteProtectROM (void)
{
	memmove((void *) WriteMap, (void *) Map, MEMMAP_NUM_BLOCKS*4);

	for (int c = 0; c < 0x1000; c++)
	{
		if (BlockIsROM[c])
			WriteMap[c] = (uint8 *) MAP_NONE;
	}
}
void CMemory::Map_Initialize (void)
{
	for (int c = 0; c < 0x1000; c++)
	{
		Map[c]      = (uint8 *) MAP_NONE;
		WriteMap[c] = (uint8 *) MAP_NONE;
		BlockIsROM[c] = FALSE;
		BlockIsRAM[c] = FALSE;
	}
}
#endif
void CMemory::LoROMMap ()
{
    int c;
    int i;

    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
	Map [c + 0] = Map [c + 0x800] = RAM;
	Map [c + 1] = Map [c + 0x801] = RAM;
	BlockIsRAM [c + 0] = BlockIsRAM [c + 0x800] = TRUE;
	BlockIsRAM [c + 1] = BlockIsRAM [c + 0x801] = TRUE;

	Map [c + 2] = Map [c + 0x802] = (uint8 *) MAP_PPU;
	Map [c + 3] = Map [c + 0x803] = (uint8 *) MAP_PPU;
	Map [c + 4] = Map [c + 0x804] = (uint8 *) MAP_CPU;
	Map [c + 5] = Map [c + 0x805] = (uint8 *) MAP_CPU;
	if (Settings.DSP1Master)
	{
	    Map [c + 6] = Map [c + 0x806] = (uint8 *) MAP_DSP;
	    Map [c + 7] = Map [c + 0x807] = (uint8 *) MAP_DSP;
	}
	else
	if (Settings.C4)
	{
	    Map [c + 6] = Map [c + 0x806] = (uint8 *) MAP_C4;
	    Map [c + 7] = Map [c + 0x807] = (uint8 *) MAP_C4;
	}
	else
	{
	    Map [c + 6] = Map [c + 0x806] = (uint8 *) bytes0x2000 - 0x6000;
	    Map [c + 7] = Map [c + 0x807] = (uint8 *) bytes0x2000 - 0x6000;
	}

	for (i = c + 8; i < c + 16; i++)
	{
	    Map [i] = Map [i + 0x800] = &ROM [(c << 11) % CalculatedSize] - 0x8000;
	    BlockIsROM [i] = BlockIsROM [i + 0x800] = TRUE;
	}

	for (i = c; i < c + 16; i++)
	{
	    int ppu = i & 15;
	    
	    MemorySpeed [i] = 
		MemorySpeed [i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
	}
    }

    if (Settings.DSP1Master)
    {
	// Banks 30->3f and b0->bf
	for (c = 0x300; c < 0x400; c += 16)
	{
	    for (i = c + 8; i < c + 16; i++)
	    {
		Map [i] = Map [i + 0x800] = (uint8 *) MAP_DSP;
		BlockIsROM [i] = BlockIsROM [i + 0x800] = FALSE;
	    }
	}
    }

    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
	for (i = c; i < c + 8; i++)
	    Map [i + 0x400] = Map [i + 0xc00] = &ROM [(c << 11) % CalculatedSize];

	for (i = c + 8; i < c + 16; i++)
	    Map [i + 0x400] = Map [i + 0xc00] = &ROM [((c << 11) + 0x200000) % CalculatedSize - 0x8000];

	for (i = c; i < c + 16; i++)	
	{
	    MemorySpeed [i + 0x400] = MemorySpeed [i + 0xc00] = SLOW_ONE_CYCLE;
	    BlockIsROM [i + 0x400] = BlockIsROM [i + 0xc00] = TRUE;
	}
    }

    if (Settings.DSP1Master)
    {
	for (c = 0; c < 0x100; c++)
	{
	    Map [c + 0xe00] = (uint8 *) MAP_DSP;
	    MemorySpeed [c + 0xe00] = SLOW_ONE_CYCLE;
	    BlockIsROM [c + 0xe00] = FALSE;
	}
    }
    MapRAM ();
    WriteProtectROM ();
}

void CMemory::HiROMMap ()
{
    int c;
    int i;

    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
	Map [c + 0] = Map [c + 0x800] = RAM;
	BlockIsRAM [c + 0] = BlockIsRAM [c + 0x800] = TRUE;
	Map [c + 1] = Map [c + 0x801] = RAM;
	BlockIsRAM [c + 1] = BlockIsRAM [c + 0x801] = TRUE;

	Map [c + 2] = Map [c + 0x802] = (uint8 *) MAP_PPU;
	Map [c + 3] = Map [c + 0x803] = (uint8 *) MAP_PPU;
	Map [c + 4] = Map [c + 0x804] = (uint8 *) MAP_CPU;
	Map [c + 5] = Map [c + 0x805] = (uint8 *) MAP_CPU;
	if (Settings.DSP1Master)
	{
	    Map [c + 6] = Map [c + 0x806] = (uint8 *) MAP_DSP;
	    Map [c + 7] = Map [c + 0x807] = (uint8 *) MAP_DSP;
	}
	else
	{
	    Map [c + 6] = Map [c + 0x806] = (uint8 *) MAP_NONE;
	    Map [c + 7] = Map [c + 0x807] = (uint8 *) MAP_NONE;
	}
	    
	for (i = c + 8; i < c + 16; i++)
	{
	    Map [i] = Map [i + 0x800] = &ROM [(c << 12) % CalculatedSize];
	    BlockIsROM [i] = BlockIsROM [i + 0x800] = TRUE;
	}

	for (i = c; i < c + 16; i++)
	{
	    int ppu = i & 15;
	    
	    MemorySpeed [i] = 
		MemorySpeed [i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
	}
    }

    // Banks 30->3f and b0->bf, address ranges 6000->7fff is S-RAM.
    for (c = 0; c < 16; c++)
    {
	Map [0x306 + (c << 4)] = (uint8 *) MAP_HIROM_SRAM;
	Map [0x307 + (c << 4)] = (uint8 *) MAP_HIROM_SRAM;
	Map [0xb06 + (c << 4)] = (uint8 *) MAP_HIROM_SRAM;
	Map [0xb07 + (c << 4)] = (uint8 *) MAP_HIROM_SRAM;
	BlockIsRAM [0x306 + (c << 4)] = TRUE;
	BlockIsRAM [0x307 + (c << 4)] = TRUE;
	BlockIsRAM [0xb06 + (c << 4)] = TRUE;
	BlockIsRAM [0xb07 + (c << 4)] = TRUE;
    }

    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
	for (i = c; i < c + 16; i++)
	{
	    Map [i + 0x400] = Map [i + 0xc00] = &ROM [(c << 12) % CalculatedSize];
	    MemorySpeed [i + 0x400] = MemorySpeed [i + 0xc00] = SLOW_ONE_CYCLE;
	    BlockIsROM [i + 0x400] = BlockIsROM [i + 0xc00] = TRUE;
	}
    }

    MapRAM ();
    WriteProtectROM ();
}

void CMemory::TalesROMMap (bool8 Interleaved)
{
    int c;
    int i;

    uint32 OFFSET0 = 0x400000;
    uint32 OFFSET1 = 0x400000;
    uint32 OFFSET2 = 0x000000;

    if (Interleaved)
    {
	OFFSET0 = 0x000000;
	OFFSET1 = 0x000000;
	OFFSET2 = 0x200000;
    }

    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
	Map [c + 0] = Map [c + 0x800] = RAM;
	Map [c + 1] = Map [c + 0x801] = RAM;
	BlockIsRAM [c + 0] = BlockIsRAM [c + 0x800] = TRUE;
	BlockIsRAM [c + 1] = BlockIsRAM [c + 0x801] = TRUE;

	Map [c + 2] = Map [c + 0x802] = (uint8 *) MAP_PPU;
	Map [c + 3] = Map [c + 0x803] = (uint8 *) MAP_PPU;
	Map [c + 4] = Map [c + 0x804] = (uint8 *) MAP_CPU;
	Map [c + 5] = Map [c + 0x805] = (uint8 *) MAP_CPU;
	Map [c + 6] = Map [c + 0x806] = (uint8 *) MAP_NONE;
	Map [c + 7] = Map [c + 0x807] = (uint8 *) MAP_NONE;
	for (i = c + 8; i < c + 16; i++)
	{
	    Map [i] = &ROM [((c << 12) + OFFSET0) % CalculatedSize];
	    Map [i + 0x800] = &ROM [((c << 12) + OFFSET0) % CalculatedSize];
	    BlockIsROM [i] = TRUE;
	    BlockIsROM [i + 0x800] = TRUE;
	}

	for (i = c; i < c + 16; i++)
	{
	    int ppu = i & 15;
	    
	    MemorySpeed [i] = 
		MemorySpeed [i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
	}
    }
    
    // Banks 30->3f and b0->bf, address ranges 6000->7ffff is S-RAM.
    for (c = 0; c < 16; c++)
    {
	Map [0x306 + (c << 4)] = (uint8 *) MAP_HIROM_SRAM;
	Map [0x307 + (c << 4)] = (uint8 *) MAP_HIROM_SRAM;
	Map [0xb06 + (c << 4)] = (uint8 *) MAP_HIROM_SRAM;
	Map [0xb07 + (c << 4)] = (uint8 *) MAP_HIROM_SRAM;
	BlockIsRAM [0x306 + (c << 4)] = TRUE;
	BlockIsRAM [0x307 + (c << 4)] = TRUE;
	BlockIsRAM [0xb06 + (c << 4)] = TRUE;
	BlockIsRAM [0xb07 + (c << 4)] = TRUE;
    }

    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
	for (i = c; i < c + 8; i++)
	{
	    Map [i + 0x400] = &ROM [((c << 12) + OFFSET1) % CalculatedSize];
	    Map [i + 0x408] = &ROM [((c << 12) + OFFSET1) % CalculatedSize];
	    Map [i + 0xc00] = &ROM [((c << 12) + OFFSET2) % CalculatedSize];
	    Map [i + 0xc08] = &ROM [((c << 12) + OFFSET2) % CalculatedSize];
	    BlockIsROM [i + 0x400] = TRUE;
	    BlockIsROM [i + 0x408] = TRUE;
	    BlockIsROM [i + 0xc00] = TRUE;
	    BlockIsROM [i + 0xc08] = TRUE;
	    MemorySpeed [i + 0x400] = MemorySpeed [i + 0xc00] = SLOW_ONE_CYCLE;
	    MemorySpeed [i + 0x408] = MemorySpeed [i + 0xc08] = SLOW_ONE_CYCLE;
	}
    }
    MapRAM ();
    WriteProtectROM ();
}

void CMemory::AlphaROMMap ()
{
    int c;
    int i;

    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
	Map [c + 0] = Map [c + 0x800] = RAM;
	Map [c + 1] = Map [c + 0x801] = RAM;
	BlockIsRAM [c + 0] = BlockIsRAM [c + 0x800] = TRUE;
	BlockIsRAM [c + 1] = BlockIsRAM [c + 0x801] = TRUE;

	Map [c + 2] = Map [c + 0x802] = (uint8 *) MAP_PPU;
	Map [c + 3] = Map [c + 0x803] = (uint8 *) MAP_PPU;
	Map [c + 4] = Map [c + 0x804] = (uint8 *) MAP_CPU;
	Map [c + 5] = Map [c + 0x805] = (uint8 *) MAP_CPU;
	Map [c + 6] = Map [c + 0x806] = (uint8 *) MAP_DSP;
	Map [c + 7] = Map [c + 0x807] = (uint8 *) MAP_DSP;

	for (i = c + 8; i < c + 16; i++)
	{
	    Map [i] = Map [i + 0x800] = &ROM [c << 11] - 0x8000;
	    BlockIsROM [i] = TRUE;
	}

	for (i = c; i < c + 16; i++)
	{
	    int ppu = i & 15;
	    
	    MemorySpeed [i] = 
		MemorySpeed [i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
	}
    }

    // Banks 40->7f and c0->ff

    for (c = 0; c < 0x400; c += 16)
    {
	for (i = c; i < c + 16; i++)
	{
	    Map [i + 0x400] = &ROM [(c << 12) % CalculatedSize];
	    Map [i + 0xc00] = &ROM [(c << 12) % CalculatedSize];
	    MemorySpeed [i + 0x400] = MemorySpeed [i + 0xc00] = SLOW_ONE_CYCLE;
	    BlockIsROM [i + 0x400] = BlockIsROM [i + 0xc00] = TRUE;
	}
    }

    MapRAM ();
    WriteProtectROM ();
}

void DetectSuperFxRamSize()
{
	if(ROM[0x7FDA]==0x33)
	{
		Memory.SRAMSize=ROM[0x7FBD];
	}
	else
	{
		if(strncmp(Memory.ROMName, "STAR FOX 2", 10)==0)
		{
			Memory.SRAMSize=6;
		}
		else Memory.SRAMSize=5;
	}
}

void CMemory::SuperFXROMMap ()
{
    int c;
    int i;
    
    DetectSuperFxRamSize();
    
    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
		Map [c + 0] = Map [c + 0x800] = RAM;
		Map [c + 1] = Map [c + 0x801] = RAM;
		BlockIsRAM [c + 0] = BlockIsRAM [c + 0x800] = TRUE;
		BlockIsRAM [c + 1] = BlockIsRAM [c + 0x801] = TRUE;
	
		Map [c + 2] = Map [c + 0x802] = (uint8 *) MAP_PPU;
		Map [c + 3] = Map [c + 0x803] = (uint8 *) MAP_PPU;
		Map [c + 4] = Map [c + 0x804] = (uint8 *) MAP_CPU;
		Map [c + 5] = Map [c + 0x805] = (uint8 *) MAP_CPU;
		Map [0x006 + c] = Map [0x806 + c] = (uint8 *) ::SRAM - 0x6000;
		Map [0x007 + c] = Map [0x807 + c] = (uint8 *) ::SRAM - 0x6000;
		BlockIsRAM [0x006 + c] = BlockIsRAM [0x007 + c] = BlockIsRAM [0x806 + c] = BlockIsRAM [0x807 + c] = TRUE;

		for (i = c + 8; i < c + 16; i++)
		{
			Map [i] = Map [i + 0x800] = &ROM [c << 11] - 0x8000;
			BlockIsROM [i] = BlockIsROM [i + 0x800] = TRUE;
		}
    }
    
    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
		for (i = c; i < c + 16; i++)
		{
			Map [i + 0x400] = Map [i + 0xc00] = &ROM [(c << 12) % CalculatedSize];
			BlockIsROM [i + 0x400] = BlockIsROM [i + 0xc00] = TRUE;
		}
    }
	
    // Banks 7e->7f, RAM
    for (c = 0; c < 16; c++)
    {
		Map [c + 0x7e0] = RAM;
		Map [c + 0x7f0] = RAM + 0x10000;
		BlockIsRAM [c + 0x7e0] = TRUE;
		BlockIsRAM [c + 0x7f0] = TRUE;
		BlockIsROM [c + 0x7e0] = FALSE;
		BlockIsROM [c + 0x7f0] = FALSE;
    }
	
    // Banks 70->71, S-RAM
    for (c = 0; c < 32; c++)
    {
		Map [c + 0x700] = ::SRAM + (((c >> 4) & 1) << 16);
		BlockIsRAM [c + 0x700] = TRUE;
		BlockIsROM [c + 0x700] = FALSE;
    }
	
    // Replicate the first 2Mb of the ROM at ROM + 2MB such that each 32K
    // block is repeated twice in each 64K block.
    for (c = 0; c < 64; c++)
    {
		memmove (&ROM [0x200000 + c * 0x10000], &ROM [c * 0x8000], 0x8000);
		memmove (&ROM [0x208000 + c * 0x10000], &ROM [c * 0x8000], 0x8000);
    }
	
    WriteProtectROM ();
}

void CMemory::SA1ROMMap ()
{
    int c;
    int i;

    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
	Map [c + 0] = Map [c + 0x800] = RAM;
	Map [c + 1] = Map [c + 0x801] = RAM;
	BlockIsRAM [c + 0] = BlockIsRAM [c + 0x800] = TRUE;
	BlockIsRAM [c + 1] = BlockIsRAM [c + 0x801] = TRUE;

	Map [c + 2] = Map [c + 0x802] = (uint8 *) MAP_PPU;
	Map [c + 3] = Map [c + 0x803] = (uint8 *) &FillRAM [0x3000] - 0x3000;
	Map [c + 4] = Map [c + 0x804] = (uint8 *) MAP_CPU;
	Map [c + 5] = Map [c + 0x805] = (uint8 *) MAP_CPU;
	Map [c + 6] = Map [c + 0x806] = (uint8 *) MAP_BWRAM;
	Map [c + 7] = Map [c + 0x807] = (uint8 *) MAP_BWRAM;
	for (i = c + 8; i < c + 16; i++)
	{
	    Map [i] = Map [i + 0x800] = &ROM [c << 11] - 0x8000;
	    BlockIsROM [i] = BlockIsROM [i + 0x800] = TRUE;
	}

	for (i = c; i < c + 16; i++)
	{
	    int ppu = i & 15;
	    
	    MemorySpeed [i] = 
		MemorySpeed [i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
	}
    }

    // Banks 40->7f
    for (c = 0; c < 0x400; c += 16)
    {
	for (i = c; i < c + 16; i++)
	    Map [i + 0x400] = (uint8 *) &SRAM [(c << 12) & 0x1ffff];

	for (i = c; i < c + 16; i++)
	{
	    MemorySpeed [i + 0x400] = SLOW_ONE_CYCLE;
	    BlockIsROM [i + 0x400] = FALSE;
	}
    }

    // c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
	for (i = c;  i < c + 16; i++)
	{
	    Map [i + 0xc00] = &ROM [(c << 12) % CalculatedSize];
	    MemorySpeed [i + 0xc00] = SLOW_ONE_CYCLE;
	    BlockIsROM [i + 0xc00] = TRUE;
	}
    }

    for (c = 0; c < 16; c++)
    {
	Map [c + 0x7e0] = RAM;
	Map [c + 0x7f0] = RAM + 0x10000;
	BlockIsRAM [c + 0x7e0] = TRUE;
	BlockIsRAM [c + 0x7f0] = TRUE;
	BlockIsROM [c + 0x7e0] = FALSE;
	BlockIsROM [c + 0x7f0] = FALSE;
    }
    WriteProtectROM ();

    // Now copy the map and correct it for the SA1 CPUPack.CPU.
    memmove ((void *) SA1Pack_SA1.WriteMap, (void *) WriteMap, sizeof (WriteMap));
    memmove ((void *) SA1Pack_SA1.Map, (void *) Map, MEMMAP_NUM_BLOCKS*4);

    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
	SA1Pack_SA1.Map [c + 0] = SA1Pack_SA1.Map [c + 0x800] = &FillRAM [0x3000];
	SA1Pack_SA1.Map [c + 1] = SA1Pack_SA1.Map [c + 0x801] = (uint8 *) MAP_NONE;
	SA1Pack_SA1.WriteMap [c + 0] = SA1Pack_SA1.WriteMap [c + 0x800] = &FillRAM [0x3000];
	SA1Pack_SA1.WriteMap [c + 1] = SA1Pack_SA1.WriteMap [c + 0x801] = (uint8 *) MAP_NONE;
    }

    // Banks 60->6f
    for (c = 0; c < 0x100; c++)
	SA1Pack_SA1.Map [c + 0x600] = SA1Pack_SA1.WriteMap [c + 0x600] = (uint8 *) MAP_BWRAM_BITMAP;
    
    BWRAM = SRAM;
}

void CMemory::LoROM24MBSMap ()
{
    int c;
    int i;

    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
	Map [c + 0] = Map [c + 0x800] = RAM;
	Map [c + 1] = Map [c + 0x801] = RAM;
	BlockIsRAM [c + 0] = BlockIsRAM [c + 0x800] = TRUE;
	BlockIsRAM [c + 1] = BlockIsRAM [c + 0x801] = TRUE;

	Map [c + 2] = Map [c + 0x802] = (uint8 *) MAP_PPU;
	Map [c + 3] = Map [c + 0x803] = (uint8 *) MAP_PPU;
	Map [c + 4] = Map [c + 0x804] = (uint8 *) MAP_CPU;
	Map [c + 5] = Map [c + 0x805] = (uint8 *) MAP_CPU;
        Map [c + 6] = Map [c + 0x806] = (uint8 *) MAP_NONE;
        Map [c + 7] = Map [c + 0x807] = (uint8 *) MAP_NONE;

	for (i = c + 8; i < c + 16; i++)
	{
	    Map [i] = Map [i + 0x800] = &ROM [c << 11] - 0x8000;
	    BlockIsROM [i] = BlockIsROM [i + 0x800] = TRUE;
	}

	for (i = c; i < c + 16; i++)
	{
	    int ppu = i & 15;
	    
	    MemorySpeed [i] = 
		MemorySpeed [i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
	}
    }

    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x200; c += 16)
    {
	Map [c + 0x800] = RAM;
	Map [c + 0x801] = RAM;
	BlockIsRAM [c + 0x800] = TRUE;
	BlockIsRAM [c + 0x801] = TRUE;

	Map [c + 0x802] = (uint8 *) MAP_PPU;
	Map [c + 0x803] = (uint8 *) MAP_PPU;
	Map [c + 0x804] = (uint8 *) MAP_CPU;
	Map [c + 0x805] = (uint8 *) MAP_CPU;
        Map [c + 0x806] = (uint8 *) MAP_NONE;
	Map [c + 0x807] = (uint8 *) MAP_NONE;

	for (i = c + 8; i < c + 16; i++)
	{
	    Map [i + 0x800] = &ROM [c << 11] - 0x8000 + 0x200000;
	    BlockIsROM [i + 0x800] = TRUE;
	}

	for (i = c; i < c + 16; i++)
	{
	    int ppu = i & 15;
	    
	    MemorySpeed [i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
	}
    }

    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
	for (i = c; i < c + 8; i++)
	    Map [i + 0x400] = Map [i + 0xc00] = &ROM [(c << 11) + 0x200000];

	for (i = c + 8; i < c + 16; i++)
	    Map [i + 0x400] = Map [i + 0xc00] = &ROM [(c << 11) + 0x200000 - 0x8000];

	for (i = c; i < c + 16; i++)
	{
	    MemorySpeed [i + 0x400] = MemorySpeed [i + 0xc00] = SLOW_ONE_CYCLE;
	    BlockIsROM [i + 0x400] = BlockIsROM [i + 0xc00] = TRUE;
	}
    }

    MapExtraRAM ();
    WriteProtectROM ();
}

void CMemory::SufamiTurboLoROMMap ()
{
    int c;
    int i;

    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
	Map [c + 0] = Map [c + 0x800] = RAM;
	Map [c + 1] = Map [c + 0x801] = RAM;
	BlockIsRAM [c + 0] = BlockIsRAM [c + 0x800] = TRUE;
	BlockIsRAM [c + 1] = BlockIsRAM [c + 0x801] = TRUE;

	Map [c + 2] = Map [c + 0x802] = (uint8 *) MAP_PPU;
	Map [c + 3] = Map [c + 0x803] = (uint8 *) MAP_PPU;
	Map [c + 4] = Map [c + 0x804] = (uint8 *) MAP_CPU;
	Map [c + 5] = Map [c + 0x805] = (uint8 *) MAP_CPU;
	if (Settings.DSP1Master)
	{
	    Map [c + 6] = Map [c + 0x806] = (uint8 *) MAP_DSP;
	    Map [c + 7] = Map [c + 0x807] = (uint8 *) MAP_DSP;
	}
	else
	{
	    Map [c + 6] = Map [c + 0x806] = (uint8 *) MAP_NONE;
	    Map [c + 7] = Map [c + 0x807] = (uint8 *) MAP_NONE;
	}
	for (i = c + 8; i < c + 16; i++)
	{
	    Map [i] = Map [i + 0x800] = &ROM [c << 11] - 0x8000;
	    BlockIsROM [i] = BlockIsROM [i + 0x800] = TRUE;
	}

	for (i = c; i < c + 16; i++)
	{
	    int ppu = i & 15;
	    
	    MemorySpeed [i] = 
		MemorySpeed [i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
	}
    }

    if (Settings.DSP1Master)
    {
	// Banks 30->3f and b0->bf
	for (c = 0x300; c < 0x400; c += 16)
	{
	    for (i = c + 8; i < c + 16; i++)
	    {
		Map [i] = Map [i + 0x800] = (uint8 *) MAP_DSP;
		BlockIsROM [i] = BlockIsROM [i + 0x800] = FALSE;
	    }
	}
    }

    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
	for (i = c; i < c + 8; i++)
	    Map [i + 0x400] = Map [i + 0xc00] = &ROM [(c << 11) + 0x200000];

	for (i = c + 8; i < c + 16; i++)
	    Map [i + 0x400] = Map [i + 0xc00] = &ROM [(c << 11) + 0x200000 - 0x8000];

	for (i = c; i < c + 16; i++)
	{
	    MemorySpeed [i + 0x400] = MemorySpeed [i + 0xc00] = SLOW_ONE_CYCLE;
	    BlockIsROM [i + 0x400] = BlockIsROM [i + 0xc00] = TRUE;
	}
    }

    if (Settings.DSP1Master)
    {
	for (c = 0; c < 0x100; c++)
	{
	    Map [c + 0xe00] = (uint8 *) MAP_DSP;
	    MemorySpeed [c + 0xe00] = SLOW_ONE_CYCLE;
	    BlockIsROM [c + 0xe00] = FALSE;
	}
    }

    // Banks 7e->7f, RAM
    for (c = 0; c < 16; c++)
    {
	Map [c + 0x7e0] = RAM;
	Map [c + 0x7f0] = RAM + 0x10000;
	BlockIsRAM [c + 0x7e0] = TRUE;
	BlockIsRAM [c + 0x7f0] = TRUE;
	BlockIsROM [c + 0x7e0] = FALSE;
	BlockIsROM [c + 0x7f0] = FALSE;
    }

    // Banks 60->67, S-RAM
    for (c = 0; c < 0x80; c++)
    {
	Map [c + 0x600] = (uint8 *) MAP_LOROM_SRAM;
	BlockIsRAM [c + 0x600] = TRUE;
	BlockIsROM [c + 0x600] = FALSE;
    }

    WriteProtectROM ();
}

void CMemory::SRAM512KLoROMMap ()
{
    int c;
    int i;

    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
	Map [c + 0] = Map [c + 0x800] = RAM;
	Map [c + 1] = Map [c + 0x801] = RAM;
	BlockIsRAM [c + 0] = BlockIsRAM [c + 0x800] = TRUE;
	BlockIsRAM [c + 1] = BlockIsRAM [c + 0x801] = TRUE;

	Map [c + 2] = Map [c + 0x802] = (uint8 *) MAP_PPU;
	Map [c + 3] = Map [c + 0x803] = (uint8 *) MAP_PPU;
	Map [c + 4] = Map [c + 0x804] = (uint8 *) MAP_CPU;
	Map [c + 5] = Map [c + 0x805] = (uint8 *) MAP_CPU;
	Map [c + 6] = Map [c + 0x806] = (uint8 *) MAP_NONE;
	Map [c + 7] = Map [c + 0x807] = (uint8 *) MAP_NONE;

	for (i = c + 8; i < c + 16; i++)
	{
	    Map [i] = Map [i + 0x800] = &ROM [c << 11] - 0x8000;
	    BlockIsROM [i] = BlockIsROM [i + 0x800] = TRUE;
	}

	for (i = c; i < c + 16; i++)
	{
	    int ppu = i & 15;
	    
	    MemorySpeed [i] = 
		MemorySpeed [i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
	}
    }

    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
	for (i = c; i < c + 8; i++)
	    Map [i + 0x400] = Map [i + 0xc00] = &ROM [(c << 11) + 0x200000];

	for (i = c + 8; i < c + 16; i++)
	    Map [i + 0x400] = Map [i + 0xc00] = &ROM [(c << 11) + 0x200000 - 0x8000];

	for (i = c; i < c + 16; i++)
	{
	    MemorySpeed [i + 0x400] = MemorySpeed [i + 0xc00] = SLOW_ONE_CYCLE;
	    BlockIsROM [i + 0x400] = BlockIsROM [i + 0xc00] = TRUE;
	}
    }

    MapExtraRAM ();
    WriteProtectROM ();
}

void CMemory::SRAM1024KLoROMMap ()
{
    int c;
    int i;

    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
	Map [c + 0] = Map [c + 0x800] = Map [c + 0x400] = Map [c + 0xc00] = RAM;
	Map [c + 1] = Map [c + 0x801] = Map [c + 0x401] = Map [c + 0xc01] = RAM;
	BlockIsRAM [c + 0] = BlockIsRAM [c + 0x800] = BlockIsRAM [c + 0x400] = BlockIsRAM [c + 0xc00] = TRUE;
	BlockIsRAM [c + 1] = BlockIsRAM [c + 0x801] = BlockIsRAM [c + 0x401] = BlockIsRAM [c + 0xc01] = TRUE;

	Map [c + 2] = Map [c + 0x802] = Map [c + 0x402] = Map [c + 0xc02] = (uint8 *) MAP_PPU;
	Map [c + 3] = Map [c + 0x803] = Map [c + 0x403] = Map [c + 0xc03] = (uint8 *) MAP_PPU;
	Map [c + 4] = Map [c + 0x804] = Map [c + 0x404] = Map [c + 0xc04] = (uint8 *) MAP_CPU;
	Map [c + 5] = Map [c + 0x805] = Map [c + 0x405] = Map [c + 0xc05] = (uint8 *) MAP_CPU;
	Map [c + 6] = Map [c + 0x806] = Map [c + 0x406] = Map [c + 0xc06] = (uint8 *) MAP_NONE;
	Map [c + 7] = Map [c + 0x807] = Map [c + 0x407] = Map [c + 0xc07] = (uint8 *) MAP_NONE;
	for (i = c + 8; i < c + 16; i++)
	{
	    Map [i] = Map [i + 0x800] = Map [i + 0x400] = Map [i + 0xc00] = &ROM [c << 11] - 0x8000;
	    BlockIsROM [i] = BlockIsROM [i + 0x800] = BlockIsROM [i + 0x400] = BlockIsROM [i + 0xc00] = TRUE;
	}

	for (i = c; i < c + 16; i++)
	{
	    int ppu = i & 15;
	    
	    MemorySpeed [i] = MemorySpeed [i + 0x800] = 
		MemorySpeed [i + 0x400] = MemorySpeed [i + 0xc00] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
	}
    }

    MapExtraRAM ();
    WriteProtectROM ();
}

void CMemory::BSHiROMMap ()
{
    int c;
    int i;

    // Banks 00->3f and 80->bf
    for (c = 0; c < 0x400; c += 16)
    {
	Map [c + 0] = Map [c + 0x800] = RAM;
	BlockIsRAM [c + 0] = BlockIsRAM [c + 0x800] = TRUE;
	Map [c + 1] = Map [c + 0x801] = RAM;
	BlockIsRAM [c + 1] = BlockIsRAM [c + 0x801] = TRUE;

	Map [c + 2] = Map [c + 0x802] = (uint8 *) MAP_PPU;
	Map [c + 3] = Map [c + 0x803] = (uint8 *) MAP_PPU;
	Map [c + 4] = Map [c + 0x804] = (uint8 *) MAP_CPU;
	// XXX: How large is SRAM??
	Map [c + 5] = Map [c + 0x805] = (uint8 *) SRAM;
	BlockIsRAM [c + 5] = BlockIsRAM [c + 0x805] = TRUE;
	Map [c + 6] = Map [c + 0x806] = (uint8 *) MAP_NONE;
	Map [c + 7] = Map [c + 0x807] = (uint8 *) MAP_NONE;
	    
	for (i = c + 8; i < c + 16; i++)
	{
	    Map [i] = Map [i + 0x800] = &ROM [(c << 12) % CalculatedSize];
	    BlockIsROM [i] = BlockIsROM [i + 0x800] = TRUE;
	}

	for (i = c; i < c + 16; i++)
	{
	    int ppu = i & 15;
	    
	    MemorySpeed [i] = 
		MemorySpeed [i + 0x800] = ppu >= 2 && ppu <= 3 ? ONE_CYCLE : SLOW_ONE_CYCLE;
	}
    }

    // Banks 60->7d offset 0000->7fff & 60->7f offset 8000->ffff PSRAM
    // XXX: How large is PSRAM?
    for (c = 0x600; c < 0x7e0; c += 16)
    {
	for (i = c; i < c + 8; i++)
	{
	    Map [i] = &ROM [0x400000 + (c << 11)];
	    BlockIsRAM [i] = TRUE;
	}
	for (i = c + 8; i < c + 16; i++)
	{
	    Map [i] = &ROM [0x400000 + (c << 11) - 0x8000];
	    BlockIsRAM [i] = TRUE;
	}
    }

    // Banks 40->7f and c0->ff
    for (c = 0; c < 0x400; c += 16)
    {
	for (i = c; i < c + 16; i++)
	{
	    Map [i + 0x400] = Map [i + 0xc00] = &ROM [(c << 12) % CalculatedSize];
	    MemorySpeed [i + 0x400] = MemorySpeed [i + 0xc00] = SLOW_ONE_CYCLE;
	    BlockIsROM [i + 0x400] = BlockIsROM [i + 0xc00] = TRUE;
	}
    }

    MapRAM ();
    WriteProtectROM ();
}

const char *CMemory::TVStandard ()
{
    return (Settings.PAL ? "PAL" : "NTSC");
}

const char *CMemory::Speed ()
{
    return (ROMSpeed & 0x10 ? "120ns" : "200ns");
}

const char *CMemory::MapType ()
{
    return (HiROM ? "HiROM" : "LoROM");
}

const char *CMemory::StaticRAMSize ()
{
    static char tmp [20];

    if (Memory.SRAMSize > 16)
#ifdef __PSP__
	return psp_msg_string(CORRUPT);
#else
	return ("Corrupt");
#endif
    sprintf (tmp, "%dKb", (SRAMMask + 1) / 1024);
    return (tmp);
}

const char *CMemory::Size ()
{
    static char tmp [20];

    if (ROMSize < 7 || ROMSize - 7 > 23)
#ifdef __PSP__
	return psp_msg_string(CORRUPT);
#else
	return ("Corrupt");
#endif
    sprintf (tmp, "%dMbits", 1 << (ROMSize - 7));
    return (tmp);
}

const char *CMemory::KartContents ()
{
    static char tmp [30];
    static const char *CoPro [16] = {
	"DSP1", "SuperFX", "OBC1", "SA-1", "S-DD1", "S-RTC", "CoPro#6",
	"CoPro#7", "CoPro#8", "CoPro#9", "CoPro#10", "CoPro#11", "CoPro#12",
	"CoPro#13", "CoPro#14", "CoPro-Custom"
    };
    static const char *Contents [3] = {
	"ROM", "ROM+RAM", "ROM+RAM+BAT"
    };
    if (ROMType == 0)
#ifdef __PSP__
	return psp_msg_string(ROM_ONLY);
#else
	return ("ROM only");
#endif

    sprintf (tmp, "%s", Contents [(ROMType & 0xf) % 3]);

    if ((ROMType & 0xf) >= 3)
	sprintf (tmp, "%s+%s", tmp, CoPro [(ROMType & 0xf0) >> 4]);

    return (tmp);
}

const char *CMemory::MapMode ()
{
    static char tmp [4];
    sprintf (tmp, "%02x", ROMSpeed & ~0x10);
    return (tmp);
}

const char *CMemory::ROMID ()
{
    return (ROMId);
}

void CMemory::ApplyROMFixes ()
{
    // Enable S-RTC (Real Time Clock) emulation for Dai Kaijyu Monogatari 2
    //Settings.SRTC = ((ROMType & 0xf0) >> 4) == 5;
	Settings.SRTC = (((ROMType & 0xff) << 8) + (ROMSpeed & 0xff))==0x5535;

    Settings.StrikeGunnerOffsetHack = strcmp (ROMName, "STRIKE GUNNER") == 0 ? 7 : 0;

    CPUPack.CPU.NMITriggerPoint = 4;
    if (strcmp (ROMName, "CACOMA KNIGHT") == 0)
	CPUPack.CPU.NMITriggerPoint = 25;

    // These games complain if the multi-player adaptor is 'connected'
    if (strcmp (ROMName, "TETRIS&Dr.MARIO") == 0 || 
        strcmp (ROMName, "JIGSAW PARTY") == 0 || 
        strcmp (ROMName, "SUPER PICROSS") == 0 || 
        strcmp (ROMName, "KIRBY NO KIRA KIZZU") == 0 || 
        strcmp (ROMName, "BLOCK") == 0 || 
        strncmp (ROMName, "SUPER BOMBLISS", 14) == 0 ||
	strcmp (ROMId, "ABOJ") == 0) 
    {
	Settings.MultiPlayer5Master = FALSE;
	Settings.MouseMaster = FALSE;
	Settings.SuperScopeMaster = FALSE;
    }

    // Games which spool sound samples between the SNES and sound CPU using
    // H-DMA as the sample is playing.
    if (strcmp (ROMName, "EARTHWORM JIM 2") == 0 ||
	strcmp (ROMName, "PRIMAL RAGE") == 0 ||
	strcmp (ROMName, "CLAY FIGHTER") == 0 ||
	strcmp (ROMName, "ClayFighter 2") == 0 ||
	strncasecmp (ROMName, "MADDEN", 6) == 0 ||
	strncmp (ROMName, "NHL", 3) == 0 ||
	strcmp (ROMName, "WeaponLord") == 0)
    {
	Settings.Shutdown = FALSE;
    }


    // Stunt Racer FX
    if (strcmp (ROMId, "CQ  ") == 0 ||
    // Illusion of Gaia
        strncmp (ROMId, "JG", 2) == 0 ||
	strcmp (ROMName, "GAIA GENSOUKI 1 JPN") == 0)
    {
	(IAPUuncached.OneCycle) = 13;
    }

    // RENDERING RANGER R2
    if (strcmp (ROMId, "AVCJ") == 0 ||
    // Star Ocean
	strncmp (ROMId, "ARF", 3) == 0 ||
    // Tales of Phantasia
	strncmp (ROMId, "ATV", 3) == 0 ||
    // Act Raiser 1 & 2
	strncasecmp (ROMName, "ACTRAISER", 9) == 0 ||
    // Soulblazer
	strcmp (ROMName, "SOULBLAZER - 1 USA") == 0 ||
	strncmp (ROMName, "SOULBLAZER 1",12) == 0 ||
	strcmp (ROMName, "SOULBLADER - 1") == 0 ||
    // Terranigma
	strncmp (ROMId, "AQT", 3) == 0 ||
    // Robotrek
	strncmp (ROMId, "E9 ", 3) == 0 ||
	strcmp (ROMName, "SLAP STICK 1 JPN") == 0 ||
    // ZENNIHON PURORESU2
	strncmp (ROMId, "APR", 3) == 0 ||
    // Bomberman 4
	strncmp (ROMId, "A4B", 3) == 0 ||
    // UFO KAMEN YAKISOBAN
	strncmp (ROMId, "Y7 ", 3) == 0 ||
	strncmp (ROMId, "Y9 ", 3) == 0 ||
    // Panic Bomber World
	strncmp (ROMId, "APB", 3) == 0 ||
	((strncmp (ROMName, "Parlor", 6) == 0 || 
          strcmp (ROMName, "HEIWA Parlor!Mini8") == 0 ||
	  strncmp (ROMName, "SANKYO Fever! Ì¨°ÊÞ°!", 21) == 0) &&
	 strcmp (CompanyId, "A0") == 0) ||
	strcmp (ROMName, "DARK KINGDOM") == 0 ||
	strcmp (ROMName, "ZAN3 SFC") == 0 ||
	strcmp (ROMName, "HIOUDEN") == 0 ||
	strcmp (ROMName, "ÃÝ¼É³À") == 0 ||
	strcmp (ROMName, "FORTUNE QUEST") == 0 ||
	strcmp (ROMName, "FISHING TO BASSING") == 0 ||
	strncmp (ROMName, "TokyoDome '95Battle 7", 21) == 0 ||
	strcmp (ROMName, "OHMONO BLACKBASS") == 0)
    {
	(IAPUuncached.OneCycle) = 15;
    }
    
    if (strcmp (ROMName, "BATMAN--REVENGE JOKER") == 0)
    {
	Memory.HiROM = FALSE;
	Memory.LoROM = TRUE;
	LoROMMap ();
    }
    Settings.StarfoxHack = strcmp (ROMName, "STAR FOX") == 0 ||
			   strcmp (ROMName, "STAR WING") == 0;
    Settings.WinterGold = strcmp (ROMName, "FX SKIING NINTENDO 96") == 0 ||
                          strcmp (ROMName, "DIRT RACER") == 0 ||
			  strcmp (ROMName, "Stunt Race FX") == 0 ||
			  Settings.StarfoxHack;
    Settings.ChuckRock = strcmp (ROMName, "CHUCK ROCK") == 0;
    Settings.Dezaemon = strcmp (ROMName, "DEZAEMON") == 0;
    
    if (strcmp (ROMName, "RADICAL DREAMERS") == 0 ||
	strcmp (ROMName, "TREASURE CONFLIX") == 0)
    {
	int c;

	for (c = 0; c < 0x80; c++)
	{
	    Map [c + 0x700] = ROM + 0x200000 + 0x1000 * (c & 0xf0);
	    BlockIsRAM [c + 0x700] = TRUE;
	    BlockIsROM [c + 0x700] = FALSE;
	}
	for (c = 0; c < 0x400; c += 16)
	{
	    Map [c + 5] = Map [c + 0x805] = ROM + 0x300000;
	    BlockIsRAM [c + 5] = BlockIsRAM [c + 0x805] = TRUE;
	}
	WriteProtectROM ();
    }

    Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 
		      Settings.CyclesPercentage) / 100;

    if (strcmp (ROMId, "ASRJ") == 0 && Settings.CyclesPercentage == 100)
	// Street Racer
	Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 95) / 100;

        // Power Rangers Fight
    if (strncmp (ROMId, "A3R", 3) == 0 ||
        // Clock Tower
	strncmp (ROMId, "AJE", 3) == 0)
	Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 103) / 100;
    
    if (strcmp (ROMId, "AWVP") == 0 || strcmp (ROMId, "AWVE") == 0 ||
	strcmp (ROMId, "AWVJ") == 0)
    {
	// Wrestlemania Arcade
#if 0
	if (Settings.CyclesPercentage == 100)
	    Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 140) / 100; // Fixes sound
#endif
	Settings.WrestlemaniaArcade = TRUE;
    }
    // Theme Park - disable offset-per-tile mode.
    if (strcmp (ROMId, "ATQP") == 0)
	Settings.WrestlemaniaArcade = TRUE;

    if (strncmp (ROMId, "A3M", 3) == 0 && Settings.CyclesPercentage == 100)
	// Mortal Kombat 3. Fixes cut off speech sample
	Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 110) / 100;

    if (strcmp (ROMName, "\x0bd\x0da\x0b2\x0d4\x0b0\x0bd\x0de") == 0 &&
	Settings.CyclesPercentage == 100)
	Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 101) / 100;

    if (strcmp (ROMName, "WILD TRAX") == 0 || 
	strcmp (ROMName, "YOSSY'S ISLAND") == 0 || 
	strcasecmp (ROMName, "YOSHI'S ISLAND") == 0)
	CPUPack.CPU.TriedInterleavedMode2 = TRUE;

    // Start Trek: Deep Sleep 9
    if (strncmp (ROMId, "A9D", 3) == 0 && Settings.CyclesPercentage == 100)
	Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 110) / 100;
    
    Settings.APURAMInitialValue = 0xff;

    if (strcmp (ROMName, "·­³Ô¸¥Ò¶ÞÐÃÝ¾²") == 0 ||
    	strcmp (ROMName, "KENTOUOU WORLDCHAMPIO") == 0 ||
    	strcmp (ROMName, "TKO SUPERCHAMPIONSHIP") == 0 ||
    	strcmp (ROMName, "TKO SUPER CHAMPIONSHI") == 0 ||
    	strcmp (ROMName, "IHATOVO STORY") == 0 ||
    	strcmp (ROMName, "WANDERERS FROM YS") == 0 ||
    	strcmp (ROMName, "SUPER GENTYOUHISHI") == 0 ||
    // Panic Bomber World
	strncmp (ROMId, "APB", 3) == 0)
    {
        Settings.APURAMInitialValue = 0;
    }

    Settings.DaffyDuck = strcmp (ROMName, "DAFFY DUCK: MARV MISS") == 0;
    Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;

    SA1Pack_SA1.WaitAddress = NULL;
    SA1Pack_SA1.WaitByteAddress1 = NULL;
    SA1Pack_SA1.WaitByteAddress2 = NULL;

    /* Bass Fishing */
    if (strcmp (ROMId, "ZBPJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x0093f1 >> MEMMAP_SHIFT] + 0x93f1;
	SA1Pack_SA1.WaitByteAddress1 = FillRAM + 0x304a;
    }
    /* DAISENRYAKU EXPERTWW2 */
    if (strcmp (ROMId, "AEVJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x0ed18d >> MEMMAP_SHIFT] + 0xd18d;
	SA1Pack_SA1.WaitByteAddress1 = FillRAM + 0x3000;
    }
    /* debjk2 */
    if (strcmp (ROMId, "A2DJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x008b62 >> MEMMAP_SHIFT] + 0x8b62;
    }
    /* Dragon Ballz HD */
    if (strcmp (ROMId, "AZIJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x008083 >> MEMMAP_SHIFT] + 0x8083;
	SA1Pack_SA1.WaitByteAddress1 = FillRAM + 0x3020;
    }
    /* SFC SDGUNDAMGNEXT */
    if (strcmp (ROMId, "ZX3J") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x0087f2 >> MEMMAP_SHIFT] + 0x87f2;
	SA1Pack_SA1.WaitByteAddress1 = FillRAM + 0x30c4;
    }
    /* ShougiNoHanamichi */
    if (strcmp (ROMId, "AARJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0xc1f85a >> MEMMAP_SHIFT] + 0xf85a;
	SA1Pack_SA1.WaitByteAddress1 = SRAM + 0x0c64;
	SA1Pack_SA1.WaitByteAddress2 = SRAM + 0x0c66;
    }
    /* KATO HIFUMI9DAN SYOGI */
    if (strcmp (ROMId, "A23J") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0xc25037 >> MEMMAP_SHIFT] + 0x5037;
	SA1Pack_SA1.WaitByteAddress1 = SRAM + 0x0c06;
	SA1Pack_SA1.WaitByteAddress2 = SRAM + 0x0c08;
    }
    /* idaten */
    if (strcmp (ROMId, "AIIJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0xc100be >> MEMMAP_SHIFT] + 0x00be;
	SA1Pack_SA1.WaitByteAddress1 = SRAM + 0x1002;
	SA1Pack_SA1.WaitByteAddress2 = SRAM + 0x1004;
    }
    /* igotais */
    if (strcmp (ROMId, "AITJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x0080b7 >> MEMMAP_SHIFT] + 0x80b7;
    }
    /* J96 DREAM STADIUM */
    if (strcmp (ROMId, "AJ6J") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0xc0f74a >> MEMMAP_SHIFT] + 0xf74a;
    }
    /* JumpinDerby */
    if (strcmp (ROMId, "AJUJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x00d926 >> MEMMAP_SHIFT] + 0xd926;
    }
    /* JKAKINOKI SHOUGI */
    if (strcmp (ROMId, "AKAJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x00f070 >> MEMMAP_SHIFT] + 0xf070;
    }
    /* HOSHI NO KIRBY 3 & KIRBY'S DREAM LAND 3 JAP & US */
    if (strcmp (ROMId, "AFJJ") == 0 || strcmp (ROMId, "AFJE") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x0082d4 >> MEMMAP_SHIFT] + 0x82d4;
	SA1Pack_SA1.WaitByteAddress1 = SRAM + 0x72a4;
    }
    /* KIRBY SUPER DELUXE JAP */
    if (strcmp (ROMId, "AKFJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x008c93 >> MEMMAP_SHIFT] + 0x8c93;
	SA1Pack_SA1.WaitByteAddress1 = FillRAM + 0x300a;
	SA1Pack_SA1.WaitByteAddress2 = FillRAM + 0x300e;
    }
    /* KIRBY SUPER DELUXE US */
    if (strcmp (ROMId, "AKFE") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x008cb8 >> MEMMAP_SHIFT] + 0x8cb8;
	SA1Pack_SA1.WaitByteAddress1 = FillRAM + 0x300a;
	SA1Pack_SA1.WaitByteAddress2 = FillRAM + 0x300e;
    }
    /* SUPER MARIO RPG JAP & US */
    if (strcmp (ROMId, "ARWJ") == 0 || strcmp (ROMId, "ARWE") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0xc0816f >> MEMMAP_SHIFT] + 0x816f;
	SA1Pack_SA1.WaitByteAddress1 = FillRAM + 0x3000;
    }
    /* marvelous.zip */
    if (strcmp (ROMId, "AVRJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x0085f2 >> MEMMAP_SHIFT] + 0x85f2;
	SA1Pack_SA1.WaitByteAddress1 = FillRAM + 0x3024;
    }
    /* AUGUSTA3 MASTERS NEW */
    if (strcmp (ROMId, "AO3J") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x00dddb >> MEMMAP_SHIFT] + 0xdddb;
	SA1Pack_SA1.WaitByteAddress1 = FillRAM + 0x37b4;
    }
    /* OSHABERI PARODIUS */
    if (strcmp (ROMId, "AJOJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x8084e5 >> MEMMAP_SHIFT] + 0x84e5;
    }
    /* PANIC BOMBER WORLD */
    if (strcmp (ROMId, "APBJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x00857a >> MEMMAP_SHIFT] + 0x857a;
    }
    /* PEBBLE BEACH NEW */
    if (strcmp (ROMId, "AONJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x00df33 >> MEMMAP_SHIFT] + 0xdf33;
	SA1Pack_SA1.WaitByteAddress1 = FillRAM + 0x37b4;
    }
    /* PGA EUROPEAN TOUR */
    if (strcmp (ROMId, "AEPE") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x003700 >> MEMMAP_SHIFT] + 0x3700;
	SA1Pack_SA1.WaitByteAddress1 = FillRAM + 0x3102;
    }
    /* PGA TOUR 96 */
    if (strcmp (ROMId, "A3GE") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x003700 >> MEMMAP_SHIFT] + 0x3700;
	SA1Pack_SA1.WaitByteAddress1 = FillRAM + 0x3102;
    }
    /* POWER RANGERS 4 */
    if (strcmp (ROMId, "A4RE") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x009899 >> MEMMAP_SHIFT] + 0x9899;
	SA1Pack_SA1.WaitByteAddress1 = FillRAM + 0x3000;
    }
    /* PACHISURO PALUSUPE */
    if (strcmp (ROMId, "AGFJ") == 0)
    {
	// Never seems to turn on the SA-1!
    }
    /* SD F1 GRAND PRIX */
    if (strcmp (ROMId, "AGFJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x0181bc >> MEMMAP_SHIFT] + 0x81bc;
    }
    /* SHOUGI MARJONG */
    if (strcmp (ROMId, "ASYJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x00f2cc >> MEMMAP_SHIFT] + 0xf2cc;
	SA1Pack_SA1.WaitByteAddress1 = SRAM + 0x7ffe;
	SA1Pack_SA1.WaitByteAddress2 = SRAM + 0x7ffc;
    }
    /* shogisai2 */
    if (strcmp (ROMId, "AX2J") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0x00d675 >> MEMMAP_SHIFT] + 0xd675;
    }

    /* SHINING SCORPION */
    if (strcmp (ROMId, "A4WJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0xc048be >> MEMMAP_SHIFT] + 0x48be;
    }
    /* SHIN SHOUGI CLUB */
    if (strcmp (ROMId, "AHJJ") == 0)
    {
	SA1Pack_SA1.WaitAddress = SA1Pack_SA1.Map [0xc1002a >> MEMMAP_SHIFT] + 0x002a;
	SA1Pack_SA1.WaitByteAddress1 = SRAM + 0x0806;
	SA1Pack_SA1.WaitByteAddress2 = SRAM + 0x0808;
    }

    // Additional game fixes by sanmaiwashi ...
    if (strcmp (ROMName, "SFX Å²Ä¶ÞÝÀÞÑÓÉ¶ÞÀØ 1") == 0) 
    {
	bytes0x2000 [0xb18] = 0x4c;
	bytes0x2000 [0xb19] = 0x4b;
	bytes0x2000 [0xb1a] = 0xea;
    }

    if (strcmp (ROMName, "GOGO ACKMAN3") == 0 || 
	strcmp (ROMName, "HOME ALONE") == 0)
    {
	// Banks 00->3f and 80->bf
	for (int c = 0; c < 0x400; c += 16)
	{
	    Map [c + 6] = Map [c + 0x806] = SRAM;
	    Map [c + 7] = Map [c + 0x807] = SRAM;
	    BlockIsROM [c + 6] = BlockIsROM [c + 0x806] = FALSE;
	    BlockIsROM [c + 7] = BlockIsROM [c + 0x807] = FALSE;
	    BlockIsRAM [c + 6] = BlockIsRAM [c + 0x806] = TRUE;
	    BlockIsRAM [c + 7] = BlockIsRAM [c + 0x807] = TRUE;
	}
	WriteProtectROM ();
    }

    if (strncmp (ROMName, "SWORD WORLD SFC", 15) == 0 ||
        strcmp (ROMName, "SFC ¶ÒÝ×²ÀÞ°") == 0)
    {
	(IAPUuncached.OneCycle) = 15;
	SNESGameFixes.NeedInit0x2137 = TRUE;
    }

    if (strncmp (ROMName, "SHIEN THE BLADE CHASE", 21) == 0)
	SNESGameFixes.Old_Read0x4200 = TRUE;

    if (strcmp (ROMName, "ºÞ¼Þ× ¶²¼Þ­³ÀÞ²¹¯¾Ý") == 0)
	SNESGameFixes.NeedInit0x2137 = TRUE;

    if (strcmp (ROMName, "UMIHARAKAWASE") == 0)
	SNESGameFixes.umiharakawaseFix = TRUE;

    if (strcmp (ROMName, "ALIENS vs. PREDATOR") == 0)
	SNESGameFixes.alienVSpredetorFix = TRUE;

    if (strcmp (ROMName, "demon's blazon") == 0 ||
	strcmp (ROMName, "demon's crest") == 0 ||
	strcmp (ROMName, "ROCKMAN X") == 0 ||
	strcmp (ROMName, "MEGAMAN X") == 0)
    {

	// CAPCOM's protect
	// Banks 0x808000, 0x408000 are mirroring.
	for (int c = 0; c < 8; c++)
	    Map [0x408 + c] = ROM - 0x8000;
    }

    if (strcmp (ROMName, "½°Êß°Ì§Ð½À") == 0 || 
	strcmp (ROMName, "½°Êß°Ì§Ð½À 2") == 0 ||
	strcmp (ROMName, "ZENKI TENCHIMEIDOU") == 0 ||
	strcmp (ROMName, "GANBA LEAGUE") == 0)
    {
	SNESGameFixes.APU_OutPorts_ReturnValueFix = TRUE;
    }

    // HITOMI3
    if (strcmp (ROMName, "HITOMI3") == 0)
    {
	Memory.SRAMSize = 1;
	SRAMMask = Memory.SRAMSize ?
			((1 << (Memory.SRAMSize + 3)) * 128) - 1 : 0;
    }

    if (strcmp (ROMName, "goemon 4") == 0)
	SNESGameFixes.SRAMInitialValue = 0x00;

    if (strcmp (ROMName, "PACHISLO ¹Ý·­³") == 0)
	SNESGameFixes._0x213E_ReturnValue = 1;

    if (strcmp (ROMName, "»Þ Ï°¼Þ¬Ý Ä³Ê²ÃÞÝ") == 0)
	SNESGameFixes.TouhaidenControllerFix = TRUE;

    if (strcmp (ROMName, "DRAGON KNIGHT 4") == 0)
    {
	// Banks 70->7e, S-RAM
	for (int c = 0; c < 0xe0; c++)
	{
	    Map [c + 0x700] = (uint8 *) MAP_LOROM_SRAM;
	    BlockIsRAM [c + 0x700] = TRUE;
	    BlockIsROM [c + 0x700] = FALSE;
	}
	WriteProtectROM ();
    }

    if (strncmp (ROMName, "LETs PACHINKO(", 14) == 0)
    {
	(IAPUuncached.OneCycle) = 15;
	if (!Settings.ForceNTSC && !Settings.ForcePAL)
	{
	    Settings.PAL = FALSE;
	    Settings.FrameTime = Settings.FrameTimeNTSC;
	    ROMFramesPerSecond = 60;
	}
    }

    if (strcmp (ROMName, "FURAI NO SIREN") == 0)
	SNESGameFixes.SoundEnvelopeHeightReading2 = TRUE;
#if 0
    if(strcmp (ROMName, "XBAND JAPANESE MODEM") == 0)
    {
	for (c = 0x200; c < 0x400; c += 16)
	{
	    for (int i = c; i < c + 16; i++)
	    {
		Map [i + 0x400] = Map [i + 0xc00] = &ROM[c * 0x1000];
		MemorySpeed [i + 0x400] = MemorySpeed [i + 0xc00] = 8;
		BlockIsRAM [i + 0x400] = BlockIsRAM [i + 0xc00] = TRUE;
		BlockIsROM [i + 0x400] = BlockIsROM [i + 0xc00] = FALSE;
	    }
	}
	WriteProtectROM ();
    }
#endif

#define RomPatch(adr,ov,nv) \
if (ROM [adr] == ov) \
    ROM [adr] = nv

    // Love Quest
    if (strcmp (ROMName, "LOVE QUEST") == 0)
    {
	RomPatch (0x1385ec, 0xd0, 0xea);
	RomPatch (0x1385ed, 0xb2, 0xea);
    }

    // Nangoku Syonen Papuwa Kun
    if (strcmp (ROMName, "NANGOKUSYONEN PAPUWA") == 0)
	RomPatch (0x1f0d1, 0xa0, 0x6b);

    // Tetsuwan Atom
    if (strcmp (ROMName, "Tetsuwan Atom") == 0)
    {
	RomPatch (0xe24c5, 0x90, 0xea);
	RomPatch (0xe24c6, 0xf3, 0xea);
    }

    // Oda Nobunaga
    if (strcmp (ROMName, "SFC ODA NOBUNAGA") == 0)
    {
	RomPatch (0x7497, 0x80, 0xea);
	RomPatch (0x7498, 0xd5, 0xea);
    }

    // Super Batter Up
    if (strcmp (ROMName, "Super Batter Up") == 0)
    {
	RomPatch (0x27ae0, 0xd0, 0xea);
	RomPatch (0x27ae1, 0xfa, 0xea);
    }

    // Super Professional Baseball 2
    if (strcmp (ROMName, "SUPER PRO. BASE BALL2") == 0)
    {
	RomPatch (0x1e4, 0x50, 0xea);
	RomPatch (0x1e5, 0xfb, 0xea);
    }
    
    if (strncmp(ROMName, "FF MYSTIC QUEST",15) == 0)
    {
    	if (Settings.CyclesPercentage == 100) 
    		Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * 120) / 100;
    }
}

#define IPS_EOF 0x00454F46l

#define IPS_READ(mac_a,mac_b) \
if (ips_avail<mac_b) { \
	char *temp_ptr=(char*)(mac_a);\
	int tmp_b=mac_b; \
	memcpy(temp_ptr,ips_data+ips_pos,ips_avail);ips_current+=ips_avail; tmp_b-=ips_avail; temp_ptr+=ips_avail;\
	ips_avail=fread(ips_data,1,0x1000,patch_file);\
	memcpy(temp_ptr,ips_data,tmp_b);ips_current+=tmp_b;ips_avail-=tmp_b;ips_pos=tmp_b; \
} else { \
	memcpy(mac_a,ips_data+ips_pos,mac_b);ips_avail-=mac_b;ips_current+=mac_b;ips_pos+=mac_b; \
} \
if (!(scr_update&3)) psp_showProgressBar(ips_size-ips_current,ips_size);\
scr_update++;

void CMemory::CheckForIPSPatch (const char *rom_filename, bool8 header,int32 &rom_size,char *ips_ext) {
	unsigned long FileSize = 0;
  int retry_count = 0;
  unsigned char bufferIPS[256];   
  char fname [256 + 1];
  int i,x,y,r,v,b,scr_update;
  FILE *patch_file;
  uint8 *ips_data;  	
  uint32 ips_size,ips_avail,ips_pos,ips_current;
  long  offset = header ? 512 : 0;
  uint32 Address;    
  int32 ofs;
  uint16 PatchSize,rlen;
  char str[256];  
  char *p;
  
  strcpy(fname,rom_filename);
  p=strrchr(fname,'.');
  if (!p) return;
  strcpy(p,ips_ext);
  patch_file = fopen(fname, "rb");
  if (!patch_file) {
   	//no patch
   	return;
  }        
  fseek(patch_file,0,SEEK_END);
  ips_size=ftell(patch_file);
  fseek(patch_file,0,SEEK_SET);
                    
  ips_data=(uint8*)malloc(0x1000); //small read buffer    
  if (!ips_data) {fclose (patch_file);return;}  
  ips_avail=0x1000;
  ips_pos=0;
  ips_current=0;
  scr_update=0;
  fread(ips_data,0x1000,1,patch_file);        
                
	//memcpy(fname,ips_data+ips_pos,5);ips_avail-=5;ips_current+=5;ips_pos+=5;
	IPS_READ(fname,5)
  if (strncmp (fname, "PATCH", 5) != 0) {
		fclose(patch_file);
		free(ips_data);
		return;
  }
	sprintf(str,psp_msg_string(FILE_IPS_APPLYING),ips_ext+1,ips_size>>10);
	sprintf(str,"Found IPS patch : %s\nSize is : %dKo\nApplying ....",ips_ext+1,ips_size>>10);
	msgBoxLines(str,0);
	msgBoxLines(str,0);
			
	x=0;r=0;v=0;b=16;
	for (;;){						    
    //memcpy(bufferIPS,ips_data+0x1000-ips_avail,3);ips_avail-=3;ips_current+=3;
    IPS_READ(bufferIPS,3)
        
   	bufferIPS[3]=0;
		if (!strcmp((char*)bufferIPS,"EOF")) {
			//success 
			msgBoxLines(psp_msg_string(FILE_IPS_PATCHSUCCESS),20);			
			break;
		}
      	  	
    Address=(((uint32)(bufferIPS[0]))<<16)|(((uint32)(bufferIPS[1]))<<8)|(((uint32)(bufferIPS[2]))<<0);
	  		
		//memcpy(bufferIPS,ips_pos,2);ips_pos+=2;
		IPS_READ(bufferIPS,2)
	  PatchSize=(((uint32)(bufferIPS[0]))<<8)|(((uint32)(bufferIPS[1]))<<0);
	    
  	if (!PatchSize) { //RLE compressed data	    	    
	    //memcpy(bufferIPS,ips_pos,2);ips_pos+=2;
	    IPS_READ(bufferIPS,2)
	    rlen=(((uint32)(bufferIPS[0]))<<8)|(((uint32)(bufferIPS[1]))<<0);	    
	    //memcpy(bufferIPS,ips_pos,1);ips_pos+=1;
	    IPS_READ(bufferIPS,1)
	  }

	  if (Address<rom_size) {		            
	    ofs=Address;	     
	    if (PatchSize) {		
		  	Address+=PatchSize;
		  	if (rom_size<Address) {
			    rom_size=Address;
		      //		      printf("Extending\n");
		    }
		  //		  printf("Regular patch %04X %d\n",Address,PatchSize);
		  	while (PatchSize--)	{		  		
		  		//memcpy(bufferIPS,ips_pos,1);ips_pos+=1;
		  		IPS_READ(bufferIPS,1)
		  		ROM[ofs]=bufferIPS[0];
		  		ofs++;		  				  		
		  	}
		  } else {
				Address+=rlen;
				if (rom_size<Address) {
		    	rom_size=Address;
		      //		       printf("Extending\n");
		      psp_msg(EXTENDING, MSG_DEFAULT);
		  	}
		  	//		  printf("RLE patcht %04X %d %d\n",Address,rlen,Byte);
		  	memset(ROM+ofs,bufferIPS[0],rlen);
		  	ofs+=rlen;
		  }
		}	else {	     
	     sprintf(str,psp_msg_string(EXTENDING_TARGET),rom_size,Address);
	     msgBoxLines(str,30);
	     
	     memset(ROM+rom_size,0,Address-rom_size);
	     rom_size=Address;
	  }	
	  //printf("File pos %d\n",ftell(fips));
  }
	free(ips_data);
	fclose(patch_file);
}

/*#undef INLINE
#define INLINE
#include "getset.h"*/
#if 0
#include "dsp1.h"
uint8 MapGetBytePPU(uint32 Address)
{
	if (!CPUPack.CPU.InDMA)
	    CPUPack.CPU.Cycles += ONE_CYCLE;
	return (S9xGetPPU (Address & 0xFFFF));
}
uint16 MapGetWordPPU(uint32 Address)
{
	if (!CPUPack.CPU.InDMA)
	    CPUPack.CPU.Cycles += TWO_CYCLES;
	return (S9xGetPPU (Address & 0xffff) |
		(S9xGetPPU ((Address + 1) & 0xffff) << 8));
}
uint16 MapGetWordCPU(uint32 Address)
{
		return (S9xGetCPU (Address & 0xffff) |
		(S9xGetCPU ((Address + 1) & 0xffff) << 8));
}
uint16 MapGetWordDSP(uint32 Address)
{
	return (S9xGetDSP (Address & 0xffff) |
		(S9xGetDSP ((Address + 1) & 0xffff) << 8));
}
uint8 MapGetByteLOROM_SRAM(uint32 Address)
{
	return (*(SRAM + ((((Address&0xFF0000)>>1) |(Address&0x7FFF)) &Memory.SRAMMask)));
}
uint16 MapGetWordLOROM_SRAM(uint32 Address)
{
    if(Memory.SRAMMask>=MEMMAP_MASK){
        return READ_WORD(SRAM + ((((Address&0xFF0000)>>1) |(Address&0x7FFF)) &Memory.SRAMMask));
    } else {
        /* no READ_WORD here, since if Memory.SRAMMask=0x7ff
         * then the high byte doesn't follow the low byte. */
        return
            (*(SRAM + ((((Address&0xFF0000)>>1) |(Address&0x7FFF)) &Memory.SRAMMask)))|
            ((*(SRAM + (((((Address+1)&0xFF0000)>>1) |((Address+1)&0x7FFF)) &Memory.SRAMMask)))<<8);
    }
}
uint8 MapGetByteHIROM_SRAM(uint32 Address)
{
	return (*(SRAM + (((Address & 0x7fff) - 0x6000 +
				  ((Address & 0xf0000) >> 3)) & Memory.SRAMMask)));
}
uint16 MapGetWordHIROM_SRAM(uint32 Address)
{
   if(Memory.SRAMMask>=MEMMAP_MASK){
        return READ_WORD(SRAM +
                         (((Address & 0x7fff) - 0x6000 +
                           ((Address & 0xf0000) >> 3)) & Memory.SRAMMask));
    } else {
        /* no READ_WORD here, since if Memory.SRAMMask=0x7ff
         * then the high byte doesn't follow the low byte. */
        return (*(SRAM +
                  (((Address & 0x7fff) - 0x6000 +
                    ((Address & 0xf0000) >> 3)) & Memory.SRAMMask)) |
                (*(SRAM +
                   ((((Address + 1) & 0x7fff) - 0x6000 +
                     (((Address + 1) & 0xf0000) >> 3)) & Memory.SRAMMask)) << 8));
    }
}
uint8 MapGetByteBWRAM(uint32 Address)
{
	return (*(BWRAM + ((Address & 0x7fff) - 0x6000)));
}
uint16 MapGetWordBWRAM(uint32 Address)
{
	return (*(BWRAM + ((Address & 0x7fff) - 0x6000)) |
		(*(BWRAM + (((Address + 1) & 0x7fff) - 0x6000)) << 8));
}
uint8 MapGetByteC4(uint32 Address)
{
	return (S9xGetC4 (Address & 0xffff));
}
uint16 MapGetWordC4(uint32 Address)
{
	return (S9xGetC4 (Address & 0xffff) |	
		(S9xGetC4 ((Address + 1) & 0xffff) << 8));	
}
uint8 MapGetByteNONE(uint32 Address)
{
	return ((Address >> 8) & 0xff);
}
uint16 MapGetWordNONE(uint32 Address)
{
	return (((Address >> 8) | (Address & 0xff00)) & 0xffff);
}
void  MapSetBytePPU (uint8 Byte, uint32 Address)
{
	if (!CPUPack.CPU.InDMA)
	    CPUPack.CPU.Cycles += ONE_CYCLE;
	S9xSetPPU (Byte, Address & 0xffff);
}
void  MapSetWordPPU (uint16 Word, uint32 Address)
{
	S9xSetPPU ((uint8) Word, Address & 0xffff);
	S9xSetPPU (Word >> 8, (Address & 0xffff) + 1);
}
void  MapSetByteCPU (uint8 Byte, uint32 Address)
{
		S9xSetCPU (Byte, Address & 0xffff);
}
void  MapSetWordCPU (uint16 Word, uint32 Address)
{
	S9xSetCPU ((uint8) Word, (Address & 0xffff));
	S9xSetCPU (Word >> 8, (Address & 0xffff) + 1);
}
void  MapSetWordDSP (uint16 Word, uint32 Address)
{
	S9xSetDSP ((uint8) Word, (Address & 0xffff));
	S9xSetDSP (Word >> 8, (Address & 0xffff) + 1);
}
void  MapSetByteLOROM_SRAM (uint8 Byte, uint32 Address)
{
    if (Memory.SRAMMask)
    {
        *(SRAM + ((((Address&0xFF0000)>>1)|(Address&0x7FFF))& Memory.SRAMMask))=Byte;
        CPUPack.CPU.SRAMModified = TRUE;
    }
}
void  MapSetWordLOROM_SRAM (uint16 Word, uint32 Address)
{
    if (Memory.SRAMMask) {
        if(Memory.SRAMMask>=MEMMAP_MASK){
            WRITE_WORD(SRAM + ((((Address&0xFF0000)>>1)|(Address&0x7FFF))&Memory.SRAMMask), Word);
        } else {
            /* no WRITE_WORD here, since if Memory.SRAMMask=0x7ff
             * then the high byte doesn't follow the low byte. */
            *(SRAM + ((((Address&0xFF0000)>>1)|(Address&0x7FFF))& Memory.SRAMMask)) = (uint8) Word;
            *(SRAM + (((((Address+1)&0xFF0000)>>1)|((Address+1)&0x7FFF))& Memory.SRAMMask)) = Word >> 8;
        }

        CPUPack.CPU.SRAMModified = TRUE;
    }
}
void  MapSetByteHIROM_SRAM (uint8 Byte, uint32 Address)
{
    if (Memory.SRAMMask)
    {
        *(SRAM + (((Address & 0x7fff) - 0x6000 +
                          ((Address & 0xf0000) >> 3)) & Memory.SRAMMask)) = Byte;
        CPUPack.CPU.SRAMModified = TRUE;
    }
}
void  MapSetWordHIROM_SRAM (uint16 Word, uint32 Address)
{
	if (Memory.SRAMMask)
	{
        if(Memory.SRAMMask>=MEMMAP_MASK){
            WRITE_WORD(SRAM +
                       (((Address & 0x7fff) - 0x6000 +
                         ((Address & 0xf0000) >> 3) & Memory.SRAMMask)), Word);
        } else {
            /* no WRITE_WORD here, since if Memory.SRAMMask=0x7ff
             * then the high byte doesn't follow the low byte. */
            *(SRAM +
              (((Address & 0x7fff) - 0x6000 +
                ((Address & 0xf0000) >> 3) & Memory.SRAMMask))) = (uint8) Word;
            *(SRAM +
              ((((Address + 1) & 0x7fff) - 0x6000 +
                (((Address + 1) & 0xf0000) >> 3) & Memory.SRAMMask))) = (uint8) (Word >> 8);
        }
	    CPUPack.CPU.SRAMModified = TRUE;
	}
}
void  MapSetByteBWRAM (uint8 Byte, uint32 Address)
{
	*(BWRAM + ((Address & 0x7fff) - 0x6000)) = Byte;
	CPUPack.CPU.SRAMModified = TRUE;	
}
void  MapSetWordBWRAM (uint16 Word, uint32 Address)
{
	*(BWRAM + ((Address & 0x7fff) - 0x6000)) = (uint8) Word;
	*(BWRAM + (((Address + 1) & 0x7fff) - 0x6000)) = (uint8) (Word >> 8);
	CPUPack.CPU.SRAMModified = TRUE;
}
void  MapSetByteSA1RAM (uint8 Byte, uint32 Address)
{
	*(SRAM + (Address & 0xffff)) = Byte;
	SA1Pack_SA1.Executing = !SA1Pack_SA1.Waiting;
}
void  MapSetWordSA1RAM (uint16 Word, uint32 Address)
{
	*(SRAM + (Address & 0xffff)) = (uint8) Word;
	*(SRAM + ((Address + 1) & 0xffff)) = (uint8) (Word >> 8);
	SA1Pack_SA1.Executing = !SA1Pack_SA1.Waiting;
}
void  MapSetWordC4 (uint16 Word, uint32 Address)
{
	S9xSetC4 (Word & 0xff, Address & 0xffff);
	S9xSetC4 ((uint8) (Word >> 8), (Address + 1) & 0xffff);
}
void  MapSetByteNONE (uint8 Byte, uint32 Address)
{}
#endif
void InitNewMap()
{
#if 0
	int block;
	for(int i=0;i<MEMMAP_NUM_BLOCKS;i++)
	{
	block=i;
	uint8 *GetAddress = Map [block];
	GetByteMap[i].BlockIsRAM = Memory.BlockIsRAM [block];
    GetByteMap[i].BlockIsROM = Memory.BlockIsROM [block];

	GetWordMap[i].BlockIsRAM = Memory.BlockIsRAM [block];
    GetWordMap[i].BlockIsROM = Memory.BlockIsROM [block];

	SetByteMap[i].BlockIsRAM = Memory.BlockIsRAM [block];
    SetByteMap[i].BlockIsROM = Memory.BlockIsROM [block];

	SetWordMap[i].BlockIsRAM = Memory.BlockIsRAM [block];
    SetWordMap[i].BlockIsROM = Memory.BlockIsROM [block];

	if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
    {
		GetByteMap[i].MemorySpeed = Memory.MemorySpeed [block];
//		GetByteMap[i].MemorySpeedx2 = Memory.MemorySpeed [block]<<1;
		GetByteMap[i].func=0;
		GetWordMap[i].MemorySpeed = Memory.MemorySpeed [block]<<1;
		GetWordMap[i].func=0;
	}
	else
	switch ((int) GetAddress)
    {

    case CMemory::MAP_PPU:
		GetByteMap[i].func=(int)MapGetBytePPU;
		GetByteMap[i].MemorySpeed=0;
		GetWordMap[i].func=(int)MapGetWordPPU;
		GetWordMap[i].MemorySpeed=0;
		break;
    case CMemory::MAP_CPU:
		GetByteMap[i].func=(int)S9xGetCPU;
		GetByteMap[i].MemorySpeed=ONE_CYCLE;
		GetWordMap[i].func=(int)MapGetWordCPU;
		GetWordMap[i].MemorySpeed=TWO_CYCLES;
		break;
    case CMemory::MAP_DSP:
		GetByteMap[i].func=(int)DSP1GetByte;
		GetByteMap[i].MemorySpeed=SLOW_ONE_CYCLE;
		GetWordMap[i].func=(int)MapGetWordDSP;
		GetWordMap[i].MemorySpeed=SLOW_ONE_CYCLE * 2;
		break;
    case CMemory::MAP_SA1RAM:
    case CMemory::MAP_LOROM_SRAM:
		GetByteMap[i].func=(int)MapGetByteLOROM_SRAM;
		GetByteMap[i].MemorySpeed=SLOW_ONE_CYCLE;
		GetWordMap[i].func=(int)MapGetWordLOROM_SRAM;
		GetWordMap[i].MemorySpeed=SLOW_ONE_CYCLE * 2;
		break;        
    case CMemory::MAP_HIROM_SRAM:
		GetByteMap[i].func=(int)MapGetByteHIROM_SRAM;
		GetByteMap[i].MemorySpeed=SLOW_ONE_CYCLE;
		GetWordMap[i].func=(int)MapGetWordHIROM_SRAM;
		GetWordMap[i].MemorySpeed=SLOW_ONE_CYCLE * 2;
		break;    
    case CMemory::MAP_DEBUG:
    case CMemory::MAP_BWRAM:
		GetByteMap[i].func=(int)MapGetByteBWRAM;
		GetByteMap[i].MemorySpeed=SLOW_ONE_CYCLE;
		GetWordMap[i].func=(int)MapGetWordBWRAM;
		GetWordMap[i].MemorySpeed=SLOW_ONE_CYCLE * 2;
		break;    
    case CMemory::MAP_C4:
		GetByteMap[i].func=(int)MapGetByteC4;
		GetByteMap[i].MemorySpeed=0;
		GetWordMap[i].func=(int)MapGetWordC4;
		GetWordMap[i].MemorySpeed=0;
		break;    
    default:
    case CMemory::MAP_NONE:
		GetByteMap[i].func=(int)MapGetByteNONE;
		GetByteMap[i].MemorySpeed=SLOW_ONE_CYCLE;
		GetWordMap[i].func=(int)MapGetWordNONE;
		GetWordMap[i].MemorySpeed=SLOW_ONE_CYCLE * 2;

	}
	
	block=i;
	uint8 *SetAddress = Memory.WriteMap [block];

    if (SetAddress >= (uint8 *) CMemory::MAP_LAST)
    {
		SetByteMap[i].MemorySpeed = Memory.MemorySpeed [block];
//		SetByteMap[i].MemorySpeedx2 = Memory.MemorySpeed [block]<<1;
		SetByteMap[i].func=0;
		SetWordMap[i].MemorySpeed = Memory.MemorySpeed [block]<<1;
		SetWordMap[i].func=0;
	}
	else
    switch ((int) SetAddress)
    {
    case CMemory::MAP_PPU:
		SetByteMap[i].func=(int)MapSetBytePPU;
		SetByteMap[i].MemorySpeed=0;
		SetWordMap[i].func=(int)MapSetWordPPU;
		SetWordMap[i].MemorySpeed=TWO_CYCLES;
		break;
    case CMemory::MAP_CPU:
		SetByteMap[i].func=(int)MapSetByteCPU;//S9xSetCPU;
		SetByteMap[i].MemorySpeed=ONE_CYCLE;
		SetWordMap[i].func=(int)MapSetWordCPU;
		SetWordMap[i].MemorySpeed=TWO_CYCLES;
		break;
    case CMemory::MAP_DSP:
		SetByteMap[i].func=(int)DSP1SetByte;
		SetByteMap[i].MemorySpeed=SLOW_ONE_CYCLE;
		SetWordMap[i].func=(int)MapSetWordDSP;
		SetWordMap[i].MemorySpeed=SLOW_ONE_CYCLE * 2;
		break;
    case CMemory::MAP_LOROM_SRAM:
		SetByteMap[i].func=(int)MapSetByteLOROM_SRAM;
		SetByteMap[i].MemorySpeed=SLOW_ONE_CYCLE;
		SetWordMap[i].func=(int)MapSetWordLOROM_SRAM;
		SetWordMap[i].MemorySpeed=SLOW_ONE_CYCLE * 2;
		break;
    case CMemory::MAP_HIROM_SRAM:
		SetByteMap[i].func=(int)MapSetByteHIROM_SRAM;
		SetByteMap[i].MemorySpeed=SLOW_ONE_CYCLE;
		SetWordMap[i].func=(int)MapSetWordHIROM_SRAM;
		SetWordMap[i].MemorySpeed=SLOW_ONE_CYCLE * 2;
		break;
    case CMemory::MAP_BWRAM:
		SetByteMap[i].func=(int)MapSetByteBWRAM;
		SetByteMap[i].MemorySpeed=SLOW_ONE_CYCLE;
		SetWordMap[i].func=(int)MapSetWordBWRAM;
		SetWordMap[i].MemorySpeed=SLOW_ONE_CYCLE * 2;
		break;
    case CMemory::MAP_DEBUG:
    case CMemory::MAP_SA1RAM:
		SetByteMap[i].func=(int)MapSetByteSA1RAM;
		SetByteMap[i].MemorySpeed=SLOW_ONE_CYCLE;
		SetWordMap[i].func=(int)MapSetWordSA1RAM;
		SetWordMap[i].MemorySpeed=SLOW_ONE_CYCLE;
		break;
    case CMemory::MAP_C4:
		SetByteMap[i].func=(int)S9xSetC4;
		SetByteMap[i].MemorySpeed=0;
		SetWordMap[i].func=(int)MapSetWordC4;
		SetWordMap[i].MemorySpeed=0;
		break;
    default:
    case CMemory::MAP_NONE:
		SetByteMap[i].func=(int)MapSetByteNONE;
		SetByteMap[i].MemorySpeed=SLOW_ONE_CYCLE;
		SetWordMap[i].func=(int)MapSetByteNONE;
		SetWordMap[i].MemorySpeed=SLOW_ONE_CYCLE*2;
		break;
    }










	}
#endif
}