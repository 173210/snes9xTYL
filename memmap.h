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
#ifndef _memmap_h_
#define _memmap_h_
#include "bsx.h"
#include "snes9x.h"

#ifdef FAST_LSB_WORD_ACCESS
#define READ_WORD(s) (*(uint16 *) (s))
#define READ_DWORD(s) (*(uint32 *) (s))
#define WRITE_WORD(s, d) (*(uint16 *) (s) = (d)
#define WRITE_DWORD(s, d) (*(uint32 *) (s) = (d)
#else
#define READ_WORD(s) ( *(uint8 *) (s) |\
		      (*((uint8 *) (s) + 1) << 8))
#define READ_DWORD(s) ( *(uint8 *) (s) |\
		       (*((uint8 *) (s) + 1) << 8) |\
		       (*((uint8 *) (s) + 2) << 16) |\
		       (*((uint8 *) (s) + 3) << 24))
#define WRITE_WORD(s, d) *(uint8 *) (s) = (d), \
                         *((uint8 *) (s) + 1) = (d) >> 8
#define WRITE_DWORD(s, d) *(uint8 *) (s) = (uint8) (d), \
                          *((uint8 *) (s) + 1) = (uint8) ((d) >> 8),\
                          *((uint8 *) (s) + 2) = (uint8) ((d) >> 16),\
                          *((uint8 *) (s) + 3) = (uint8) ((d) >> 24)
#define WRITE_3WORD(s, d) *(uint8 *) (s) = (uint8) (d), \
                          *((uint8 *) (s) + 1) = (uint8) ((d) >> 8),\
                          *((uint8 *) (s) + 2) = (uint8) ((d) >> 16)
#define READ_3WORD(s) ( *(uint8 *) (s) |\
                       (*((uint8 *) (s) + 1) << 8) |\
                       (*((uint8 *) (s) + 2) << 16))
                          
#endif

#define MEMMAP_BLOCK_SIZE (0x1000)
#define MEMMAP_NUM_BLOCKS (0x1000000 / MEMMAP_BLOCK_SIZE)
#define MEMMAP_BLOCKS_PER_BANK (0x10000 / MEMMAP_BLOCK_SIZE)
#define MEMMAP_SHIFT 12
#define MEMMAP_MASK (MEMMAP_BLOCK_SIZE - 1)
#define MEMMAP_MAX_SDD1_LOGGED_ENTRIES (0x10000 / 8)

class CMemory {
public:
    bool8 LoadROM (const char *);
    int LoadROMMore(int TotalFileSize,int &retry_count);
    void  InitROM (bool8);
    bool8 LoadSRAM (char *);
    bool8 SaveSRAM (char *);
	bool8 LoadSRTC (void);
	bool8 SaveSRTC (void);
    bool8 Init ();
    void  Deinit ();
    bool8 TEMP_ReInit ();
    void  TEMP_DeInit ();    
    void  FreeSDD1Data ();
    
    /***********************/
    /* GP32 */
//    bool8 TEMP_DeInit();
//    bool8 TEMP_ReInit();    
    /***********************/
    
    
    void WriteProtectROM ();
    void FixROMSpeed ();
    void MapRAM ();
    void MapExtraRAM ();
//    char *Safe (const char *);
	void DeInterleavedRom(bool8 Tales);
    
    void LoROMMap ();
    void LoROM24MBSMap ();
    void SRAM512KLoROMMap ();
    void SRAM1024KLoROMMap ();
    void SufamiTurboLoROMMap ();
    void HiROMMap ();
    void SuperFXROMMap ();
    void TalesROMMap (bool8);
    void AlphaROMMap ();
    void SA1ROMMap ();
    void BSHiROMMap ();
	void SPC7110HiROMMap ();
    bool8 AllASCII (uint8 *b, int size);
    int  ScoreHiROM (bool8 skip_header);
    int  ScoreLoROM (bool8 skip_header);
    void ApplyROMFixes ();
    void CheckForIPSPatch (const char *rom_filename, bool8 header,
			   uint32 &rom_size,const char *ips_ext);
    
    const char *TVStandard ();
    const char *Speed ();
    const char *StaticRAMSize ();
    const char *MapType ();
    const char *MapMode ();
    const char *KartContents ();
    const char *Size ();
    const char *Headers ();
    const char *ROMID ();
    const char *CompanyID ();
#ifdef _BSX_151_
	//add azz 080517
	void CMemory::map_WriteProtectROM (void);
	void CMemory::Map_Initialize (void);
	void CMemory::ParseSNESHeader (uint8 *RomHeader);
	int CMemory::LoadROMMore_151(int TotalFileSize,int &retry_count);
	uint8 ExtendedFormat;
    enum { NOPE, YEAH, BIGFIRST, SMALLFIRST };
	void BS_151();
#endif
	
	enum {
	MAP_PPU, MAP_CPU, MAP_DSP, MAP_LOROM_SRAM, MAP_HIROM_SRAM,
	MAP_NONE, MAP_DEBUG, MAP_C4, MAP_BWRAM, MAP_BWRAM_BITMAP,
	MAP_BWRAM_BITMAP2, MAP_SA1RAM
#ifdef _BSX_151_
	,MAP_BSX//add azz 20080517
#endif
	, MAP_SPC7110_ROM, MAP_SPC7110_DRAM, MAP_RONLY_SRAM
	, MAP_LAST
    };
    enum { MAX_ROM_SIZE = 0x600000 };
  //  enum { MAX_ROM_SIZE = 0x400000 };
	//uint32 MAX_ROM_SIZE;
	
    
//    uint8 *RAM;
//    uint8 *ROM;
//    uint8 *VRAM;
//    uint8 *SRAM;
//    uint8 *BWRAM;
//    uint8 *FillRAM;
//    uint8 *C4RAM;
    bool8 HiROM;
    bool8 LoROM;
    uint16 SRAMMask;
    uint8 SRAMSize;
//    uint8 *Map [MEMMAP_NUM_BLOCKS];
    uint8 *WriteMap [MEMMAP_NUM_BLOCKS];
    uint8 MemorySpeed [MEMMAP_NUM_BLOCKS];
    uint8 BlockIsRAM [MEMMAP_NUM_BLOCKS];
    uint8 BlockIsROM [MEMMAP_NUM_BLOCKS];
    char  ROMName [ROM_NAME_LEN];
    char  ROMId [5];
    char  CompanyId [3];
    uint8 ROMSpeed;
    uint8 ROMType;
    uint8 ROMSize;
    int32 ROMFramesPerSecond;
    int32 HeaderCount;
    uint32 CalculatedSize;
    uint32 CalculatedChecksum;
    uint32 ROMChecksum;
    uint32 ROMComplementChecksum;
    uint8  *SDD1Index;
    uint8  *SDD1Data;
    uint32 SDD1Entries;
    uint32 SDD1LoggedDataCountPrev;
    uint32 SDD1LoggedDataCount;
    uint8  SDD1LoggedData [MEMMAP_MAX_SDD1_LOGGED_ENTRIES];
    char ROMFilename [_MAX_PATH];
    
    uint32 sum1,sum2;
    uint32 ROMCRC32;
};
extern uint32 g_ROMCRC32;
START_EXTERN_C

struct SMEMMAPBLOCKS {
    int   func;
    uint8 MemorySpeed;
	uint8 Direct;
	uint8 BlockIsRAM;
    uint8 BlockIsROM;
};
#if 0
extern SMEMMAPBLOCKS GetByteMap [MEMMAP_NUM_BLOCKS];
extern SMEMMAPBLOCKS GetWordMap [MEMMAP_NUM_BLOCKS];
extern SMEMMAPBLOCKS SetByteMap [MEMMAP_NUM_BLOCKS];
extern SMEMMAPBLOCKS SetWordMap [MEMMAP_NUM_BLOCKS];
#endif
//extern uint8 *Map [];
extern uint8 ** const Map;
extern CMemory Memory;
extern uint8 SRAM[];
extern uint8 * const ROM;
//extern uint8 *RegRAM;
extern uint8 RAM[];
extern uint8 VRAM[];
//extern uint8 *VRAMmode7;
extern uint8 *BWRAM;
extern uint8 * const FillRAM;
extern uint8 *C4RAM;

extern uint8 *BSRAM;//add azz 080517
extern uint8 *BIOSROM; // BS

void S9xDeinterleaveMode2 ();
END_EXTERN_C

void S9xAutoSaveSRAM ();

#ifdef NO_INLINE_SET_GET
uint8 S9xGetByte (uint32 Address);
uint16 S9xGetWord (uint32 Address);
void S9xSetByte (uint8 Byte, uint32 Address);
void S9xSetWord (uint16 Byte, uint32 Address);
void S9xSetPCBase (uint32 Address);
uint8 *S9xGetMemPointer (uint32 Address);
uint8 *GetBasePointer (uint32 Address);
#else
#undef INLINE
#define INLINE inline
#include "getset.h"
#endif // NO_INLINE_SET_GET

#endif // _memmap_h_
