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
/*#include <string.h>
#include <ctype.h>
#include <stdlib.h>*/
#endif
#if defined(__unix) || defined(__linux) || defined(__sun) || defined(__DJGPP)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "snapshot.h"
#include "snaporig.h"
#include "memmap.h"
#include "snes9x.h"
#include "65c816.h"
#include "ppu.h"
#include "cpuexec.h"
#include "display.h"
#include "apu.h"
#include "soundux.h"
#include "sa1.h"
#include "srtc.h"
#include "sdd1.h"

extern uint8 SRAM[];

#ifdef ZSNES_FX
START_EXTERN_C
void S9xSuperFXPreSaveState ();
void S9xSuperFXPostSaveState ();
void S9xSuperFXPostLoadState ();
END_EXTERN_C
#endif

bool8 S9xUnfreezeZSNES (const char *filename);

typedef struct {
    int offset;
    int size;
    int type;
} FreezeData;

enum {
    INT_V, uint8_ARRAY_V, uint16_ARRAY_V, uint32_ARRAY_V
};

#define Offset(field,structure) \
	((int) (((char *) (&(((structure)NULL)->field))) - ((char *) NULL)))

#define COUNT(ARRAY) (sizeof (ARRAY) / sizeof (ARRAY[0]))

#undef OFFSET
#define OFFSET(f) Offset(f,struct SCPUState *)

static FreezeData SnapCPU [] = {
    {OFFSET (Flags), 4, INT_V},
    {OFFSET (BranchSkip), 1, INT_V},
    {OFFSET (NMIActive), 1, INT_V},
    {OFFSET (IRQActive), 1, INT_V},
    {OFFSET (WaitingForInterrupt), 1, INT_V},
    {OFFSET (WhichEvent), 1, INT_V},
    {OFFSET (Cycles), 4, INT_V},
    {OFFSET (NextEvent), 4, INT_V},
    {OFFSET (V_Counter), 4, INT_V},
    {OFFSET (MemSpeed), 4, INT_V},
    {OFFSET (MemSpeedx2), 4, INT_V},
    {OFFSET (FastROMSpeed), 4, INT_V}
};

#undef OFFSET
#define OFFSET(f) Offset(f,struct SRegisters *)

static FreezeData SnapRegisters [] = {
    {OFFSET (PB),  1, INT_V},
    {OFFSET (DB),  1, INT_V},
    {OFFSET (P.W), 2, INT_V},
    {OFFSET (A.W), 2, INT_V},
    {OFFSET (D.W), 2, INT_V},
    {OFFSET (S.W), 2, INT_V},
    {OFFSET (X.W), 2, INT_V},
    {OFFSET (Y.W), 2, INT_V},
    {OFFSET (PC),  2, INT_V}
};

#undef OFFSET
#define OFFSET(f) Offset(f,struct SPPU *)

static FreezeData SnapPPU [] = {
    {OFFSET (BGMode), 1, INT_V},
    {OFFSET (BG3Priority), 1, INT_V},
    {OFFSET (Brightness), 1, INT_V},
    {OFFSET (VMA.High), 1, INT_V},
    {OFFSET (VMA.Increment), 1, INT_V},
    {OFFSET (VMA.Address), 2, INT_V},
    {OFFSET (VMA.Mask1), 2, INT_V},
    {OFFSET (VMA.FullGraphicCount), 2, INT_V},
    {OFFSET (VMA.Shift), 2, INT_V},
    {OFFSET (BG[0].SCBase), 2, INT_V},
    {OFFSET (BG[0].VOffset), 2, INT_V},
    {OFFSET (BG[0].HOffset), 2, INT_V},
    {OFFSET (BG[0].BGSize), 1, INT_V},
    {OFFSET (BG[0].NameBase), 2, INT_V},
    {OFFSET (BG[0].SCSize), 2, INT_V},

    {OFFSET (BG[1].SCBase), 2, INT_V},
    {OFFSET (BG[1].VOffset), 2, INT_V},
    {OFFSET (BG[1].HOffset), 2, INT_V},
    {OFFSET (BG[1].BGSize), 1, INT_V},
    {OFFSET (BG[1].NameBase), 2, INT_V},
    {OFFSET (BG[1].SCSize), 2, INT_V},

    {OFFSET (BG[2].SCBase), 2, INT_V},
    {OFFSET (BG[2].VOffset), 2, INT_V},
    {OFFSET (BG[2].HOffset), 2, INT_V},
    {OFFSET (BG[2].BGSize), 1, INT_V},
    {OFFSET (BG[2].NameBase), 2, INT_V},
    {OFFSET (BG[2].SCSize), 2, INT_V},

    {OFFSET (BG[3].SCBase), 2, INT_V},
    {OFFSET (BG[3].VOffset), 2, INT_V},
    {OFFSET (BG[3].HOffset), 2, INT_V},
    {OFFSET (BG[3].BGSize), 1, INT_V},
    {OFFSET (BG[3].NameBase), 2, INT_V},
    {OFFSET (BG[3].SCSize), 2, INT_V},

    {OFFSET (CGFLIP), 1, INT_V},
    {OFFSET (CGDATA), 256, uint16_ARRAY_V},
    {OFFSET (FirstSprite), 1, INT_V},
#define O(N) \
    {OFFSET (OBJ[N].HPos), 2, INT_V}, \
    {OFFSET (OBJ[N].VPos), 2, INT_V}, \
    {OFFSET (OBJ[N].Name), 2, INT_V}, \
    {OFFSET (OBJ[N].VFlip), 1, INT_V}, \
    {OFFSET (OBJ[N].HFlip), 1, INT_V}, \
    {OFFSET (OBJ[N].Priority), 1, INT_V}, \
    {OFFSET (OBJ[N].Palette), 1, INT_V}, \
    {OFFSET (OBJ[N].Size), 1, INT_V}

    O(  0), O(  1), O(  2), O(  3), O(  4), O(  5), O(  6), O(  7),
    O(  8), O(  9), O( 10), O( 11), O( 12), O( 13), O( 14), O( 15),
    O( 16), O( 17), O( 18), O( 19), O( 20), O( 21), O( 22), O( 23),
    O( 24), O( 25), O( 26), O( 27), O( 28), O( 29), O( 30), O( 31),
    O( 32), O( 33), O( 34), O( 35), O( 36), O( 37), O( 38), O( 39),
    O( 40), O( 41), O( 42), O( 43), O( 44), O( 45), O( 46), O( 47),
    O( 48), O( 49), O( 50), O( 51), O( 52), O( 53), O( 54), O( 55),
    O( 56), O( 57), O( 58), O( 59), O( 60), O( 61), O( 62), O( 63),
    O( 64), O( 65), O( 66), O( 67), O( 68), O( 69), O( 70), O( 71),
    O( 72), O( 73), O( 74), O( 75), O( 76), O( 77), O( 78), O( 79),
    O( 80), O( 81), O( 82), O( 83), O( 84), O( 85), O( 86), O( 87),
    O( 88), O( 89), O( 90), O( 91), O( 92), O( 93), O( 94), O( 95),
    O( 96), O( 97), O( 98), O( 99), O(100), O(101), O(102), O(103),
    O(104), O(105), O(106), O(107), O(108), O(109), O(110), O(111),
    O(112), O(113), O(114), O(115), O(116), O(117), O(118), O(119),
    O(120), O(121), O(122), O(123), O(124), O(125), O(126), O(127),
#undef O
    {OFFSET (OAMPriorityRotation), 1, INT_V},
    {OFFSET (OAMAddr), 2, INT_V},
    {OFFSET (OAMFlip), 1, INT_V},
    {OFFSET (OAMTileAddress), 2, INT_V},
    {OFFSET (IRQVBeamPos), 2, INT_V},
    {OFFSET (IRQHBeamPos), 2, INT_V},
    {OFFSET (VBeamPosLatched), 2, INT_V},
    {OFFSET (HBeamPosLatched), 2, INT_V},
    {OFFSET (HBeamFlip), 1, INT_V},
    {OFFSET (VBeamFlip), 1, INT_V},
    {OFFSET (HVBeamCounterLatched), 1, INT_V},
    {OFFSET (MatrixA), 2, INT_V},
    {OFFSET (MatrixB), 2, INT_V},
    {OFFSET (MatrixC), 2, INT_V},
    {OFFSET (MatrixD), 2, INT_V},
    {OFFSET (CentreX), 2, INT_V},
    {OFFSET (CentreY), 2, INT_V},
    {OFFSET (Joypad1ButtonReadPos), 1, INT_V},
    {OFFSET (Joypad2ButtonReadPos), 1, INT_V},
    {OFFSET (Joypad3ButtonReadPos), 1, INT_V},
    {OFFSET (CGADD), 1, INT_V},
    {OFFSET (FixedColourRed), 1, INT_V},
    {OFFSET (FixedColourGreen), 1, INT_V},
    {OFFSET (FixedColourBlue), 1, INT_V},
    {OFFSET (SavedOAMAddr), 2, INT_V},
    {OFFSET (ScreenHeight), 2, INT_V},
    {OFFSET (WRAM), 4, INT_V},
    {OFFSET (ForcedBlanking), 1, INT_V},
    {OFFSET (OBJNameSelect), 2, INT_V},
    {OFFSET (OBJSizeSelect), 1, INT_V},
    {OFFSET (OBJNameBase), 2, INT_V},
    {OFFSET (OAMReadFlip), 1, INT_V},
    {OFFSET (VTimerEnabled), 1, INT_V},
    {OFFSET (HTimerEnabled), 1, INT_V},
    {OFFSET (HTimerPosition), 2, INT_V},
    {OFFSET (Mosaic), 1, INT_V},
    {OFFSET (Mode7HFlip), 1, INT_V},
    {OFFSET (Mode7VFlip), 1, INT_V},
    {OFFSET (Mode7Repeat), 1, INT_V},
    {OFFSET (Window1Left), 1, INT_V},
    {OFFSET (Window1Right), 1, INT_V},
    {OFFSET (Window2Left), 1, INT_V},
    {OFFSET (Window2Right), 1, INT_V},
#define O(N) \
    {OFFSET (ClipWindowOverlapLogic[N]), 1, INT_V}, \
    {OFFSET (ClipWindow1Enable[N]), 1, INT_V}, \
    {OFFSET (ClipWindow2Enable[N]), 1, INT_V}, \
    {OFFSET (ClipWindow1Inside[N]), 1, INT_V}, \
    {OFFSET (ClipWindow2Inside[N]), 1, INT_V}

    O(0), O(1), O(2), O(3), O(4), O(5),

#undef O

    {OFFSET (CGFLIPRead), 1, INT_V},
    {OFFSET (Need16x8Mulitply), 1, INT_V},
    {OFFSET (BGMosaic), 4, uint8_ARRAY_V},
    {OFFSET (OAMData), 512 + 32, uint8_ARRAY_V},
    {OFFSET (Need16x8Mulitply), 1, INT_V},
    {OFFSET (MouseSpeed), 2, uint8_ARRAY_V}
};

#undef OFFSET
#define OFFSET(f) Offset(f,struct SDMA *)

static FreezeData SnapDMA [] = {
#define O(N) \
    {OFFSET (TransferDirection) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (AAddressFixed) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (AAddressDecrement) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (TransferMode) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (ABank) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (AAddress) + N * sizeof (struct SDMA), 2, INT_V}, \
    {OFFSET (Address) + N * sizeof (struct SDMA), 2, INT_V}, \
    {OFFSET (BAddress) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (TransferBytes) + N * sizeof (struct SDMA), 2, INT_V}, \
    {OFFSET (HDMAIndirectAddressing) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (IndirectAddress) + N * sizeof (struct SDMA), 2, INT_V}, \
    {OFFSET (IndirectBank) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (Repeat) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (LineCount) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (FirstLine) + N * sizeof (struct SDMA), 1, INT_V}

    O(0), O(1), O(2), O(3), O(4), O(5), O(6), O(7)
#undef O
};

#undef OFFSET
#define OFFSET(f) Offset(f,struct SAPU *)

static FreezeData SnapAPU [] = {
    {OFFSET (Cycles), 4, INT_V},
    {OFFSET (ShowROM), 1, INT_V},
    {OFFSET (Flags), 1, INT_V},
    {OFFSET (KeyedChannels), 1, INT_V},
    {OFFSET (OutPorts), 4, uint8_ARRAY_V},
    {OFFSET (DSP), 0x80, uint8_ARRAY_V},
    {OFFSET (ExtraRAM), 64, uint8_ARRAY_V},
    {OFFSET (Timer), 3, uint16_ARRAY_V},
    {OFFSET (TimerTarget), 3, uint16_ARRAY_V},
    {OFFSET (TimerEnabled), 3, uint8_ARRAY_V},
    {OFFSET (TimerValueWritten), 3, uint8_ARRAY_V}
};

#undef OFFSET
#define OFFSET(f) Offset(f,struct SAPURegisters *)

static FreezeData SnapAPURegisters [] = {
    {OFFSET (P), 1, INT_V},
    {OFFSET (YA.W), 2, INT_V},
    {OFFSET (X), 1, INT_V},
    {OFFSET (S), 1, INT_V},
    {OFFSET (PC), 2, INT_V},
};

#undef OFFSET
#define OFFSET(f) Offset(f,SSoundData *)

static FreezeData SnapSoundData [] = {
    {OFFSET (master_volume_left), 2, INT_V},
    {OFFSET (master_volume_right), 2, INT_V},
    {OFFSET (echo_volume_left), 2, INT_V},
    {OFFSET (echo_volume_right), 2, INT_V},
    {OFFSET (echo_enable), 4, INT_V},
    {OFFSET (echo_feedback), 4, INT_V},
    {OFFSET (echo_ptr), 4, INT_V},
    {OFFSET (echo_buffer_size), 4, INT_V},
    {OFFSET (echo_write_enabled), 4, INT_V},
    {OFFSET (echo_channel_enable), 4, INT_V},
    {OFFSET (pitch_mod), 4, INT_V},
    {OFFSET (dummy), 3, uint32_ARRAY_V},
#define O(N) \
    {OFFSET (channels [N].state), 4, INT_V}, \
    {OFFSET (channels [N].type), 4, INT_V}, \
    {OFFSET (channels [N].volume_left), 2, INT_V}, \
    {OFFSET (channels [N].volume_right), 2, INT_V}, \
    {OFFSET (channels [N].hertz), 4, INT_V}, \
    {OFFSET (channels [N].count), 4, INT_V}, \
    {OFFSET (channels [N].loop), 1, INT_V}, \
    {OFFSET (channels [N].envx), 4, INT_V}, \
    {OFFSET (channels [N].left_vol_level), 2, INT_V}, \
    {OFFSET (channels [N].right_vol_level), 2, INT_V}, \
    {OFFSET (channels [N].envx_target), 2, INT_V}, \
    {OFFSET (channels [N].env_error), 4, INT_V}, \
    {OFFSET (channels [N].erate), 4, INT_V}, \
    {OFFSET (channels [N].direction), 4, INT_V}, \
    {OFFSET (channels [N].attack_rate), 4, INT_V}, \
    {OFFSET (channels [N].decay_rate), 4, INT_V}, \
    {OFFSET (channels [N].sustain_rate), 4, INT_V}, \
    {OFFSET (channels [N].release_rate), 4, INT_V}, \
    {OFFSET (channels [N].sustain_level), 4, INT_V}, \
    {OFFSET (channels [N].sample), 2, INT_V}, \
    {OFFSET (channels [N].decoded), 16, uint16_ARRAY_V}, \
    {OFFSET (channels [N].previous16), 2, uint16_ARRAY_V}, \
    {OFFSET (channels [N].sample_number), 2, INT_V}, \
    {OFFSET (channels [N].last_block), 1, INT_V}, \
    {OFFSET (channels [N].needs_decode), 1, INT_V}, \
    {OFFSET (channels [N].block_pointer), 4, INT_V}, \
    {OFFSET (channels [N].sample_pointer), 4, INT_V}, \
    {OFFSET (channels [N].mode), 4, INT_V}

    O(0), O(1), O(2), O(3), O(4), O(5), O(6), O(7)
#undef O
};

#undef OFFSET
#define OFFSET(f) Offset(f,struct SSA1Registers *)

static FreezeData SnapSA1Registers [] = {
    {OFFSET (PB),  1, INT_V},
    {OFFSET (DB),  1, INT_V},
    {OFFSET (P.W), 2, INT_V},
    {OFFSET (A.W), 2, INT_V},
    {OFFSET (D.W), 2, INT_V},
    {OFFSET (S.W), 2, INT_V},
    {OFFSET (X.W), 2, INT_V},
    {OFFSET (Y.W), 2, INT_V},
    {OFFSET (PC),  2, INT_V}
};

#undef OFFSET
#define OFFSET(f) Offset(f,struct SSA1 *)

static FreezeData SnapSA1 [] = {
    {OFFSET (Flags), 4, INT_V},
    {OFFSET (NMIActive), 1, INT_V},
    {OFFSET (IRQActive), 1, INT_V},
    {OFFSET (WaitingForInterrupt), 1, INT_V},
    {OFFSET (op1), 2, INT_V},
    {OFFSET (op2), 2, INT_V},
    {OFFSET (arithmetic_op), 4, INT_V},
    {OFFSET (sum), 8, INT_V},
    {OFFSET (overflow), 1, INT_V}
};
#ifdef _BSX_151_
#undef OFFSET
#define OFFSET(f) Offset(f,struct SBSX *)

static FreezeData SnapBSX [] = {
    {OFFSET (dirty), 1, INT_V},
    {OFFSET (dirty2), 1, INT_V},
    {OFFSET (bootup), 1, INT_V},
    {OFFSET (flash_enable), 1, INT_V},
    {OFFSET (write_enable), 1, INT_V},
    {OFFSET (read_enable), 1, INT_V},
    {OFFSET (flash_command), 4, INT_V},
    {OFFSET (old_write), 4, INT_V},
    {OFFSET (new_write), 4, INT_V},
    {OFFSET (out_index), 1, INT_V},
    {OFFSET (output), 32, uint8_ARRAY_V},
    {OFFSET (PPU), 32, uint8_ARRAY_V},
    {OFFSET (MMC), 16, uint8_ARRAY_V},
    {OFFSET (prevMMC), 16, uint8_ARRAY_V},
    {OFFSET (test2192), 32, uint8_ARRAY_V},
};
#endif

static char ROMFilename [_MAX_PATH];
//static char SnapshotFilename [_MAX_PATH];

static void Freeze (STREAM);
static int Unfreeze (STREAM);
void FreezeStruct (STREAM stream, const char *name, void *base, FreezeData *fields,
		   int num_fields);
void FreezeBlock (STREAM stream, const char *name, uint8 *block, int size);

int UnfreezeStruct (STREAM stream, const char *name, void *base, FreezeData *fields,
		    int num_fields);
int UnfreezeBlock (STREAM stream, const char *name, uint8 *block, int size);

bool8 Snapshot (const char *filename)
{
    return (S9xFreezeGame (filename));
}

bool8 S9xFreezeGame (const char *filename)
{
    STREAM stream = NULL;
    if (S9xOpenSnapshotFile (filename, FALSE, &stream))
    {
		Freeze (stream);

		S9xCloseSnapshotFile (stream);
		return (TRUE);
    }
    return (FALSE);
}

bool8 S9xLoadSnapshot (const char *filename)
{
    return (S9xUnfreezeGame (filename));
}

bool8 S9xUnfreezeGame_Internal (const char *filename);
void debug_dump(const char* filename);
bool8 S9xUnfreezeGame (const char *filename)
{
	bool8 bRet;
	S9xSuspendSoundProcess();
	bRet = S9xUnfreezeGame_Internal(filename);
	//debug_dump("LoadSnap");
	S9xResumeSoundProcess();
	return bRet;
}

bool8 S9xUnfreezeGame_Internal (const char *filename)
{


    if (S9xUnfreezeZSNES (filename))
	return (TRUE);


//	  if (S9xLoadOrigSnapshot (filename))
//	return (TRUE);
	

    STREAM snapshot = NULL;

    if (S9xOpenSnapshotFile (filename, TRUE, &snapshot))
    {    
	int result;

	if ((result = Unfreeze (snapshot)) != SUCCESS)
	{
	    switch (result)
	    {
	    case WRONG_FORMAT:
		S9xMessage (S9X_ERROR, S9X_WRONG_FORMAT, 
			    "File not in Snes9x freeze format");
		//S9xReset();
		break;
	    case WRONG_VERSION:
		S9xMessage (S9X_ERROR, S9X_WRONG_VERSION,
			    "Incompatable Snes9x freeze file format version");
		//S9xReset();
		break;
	    default:
	    case FILE_NOT_FOUND:
		sprintf (String, "ROM image \"%s\" for freeze file not found",
			 ROMFilename);
		S9xMessage (S9X_ERROR, S9X_ROM_NOT_FOUND, String);
		break;
	    }
	    S9xCloseSnapshotFile (snapshot);
	    return (FALSE);
	}
	S9xCloseSnapshotFile (snapshot);
	return (TRUE);
    }

    
    return (FALSE);
}

static void Freeze_Internal (STREAM stream);

static void Freeze (STREAM stream)
{
	S9xSuspendSoundProcess();
	//debug_dump("SaveSnap");
	Freeze_Internal(stream);
	S9xResumeSoundProcess();
}

static void Freeze_Internal (STREAM stream)
{
    char buffer[1024];
    int i;

    

    S9xSetSoundMute (TRUE);
#ifdef ZSNES_FX
    if (Settings.SuperFX)
	S9xSuperFXPreSaveState ();
#endif

    S9xSRTCPreSaveState ();

    for (i = 0; i < 8; i++)
    {
	SoundData.channels [i].previous16 [0] = (int16) SoundData.channels [i].previous [0];
	SoundData.channels [i].previous16 [1] = (int16) SoundData.channels [i].previous [1];
    }
    sprintf (buffer, "%s:%04d\n", SNAPSHOT_MAGIC, SNAPSHOT_VERSION);
    WRITE_STREAM (buffer, strlen (buffer), stream);
    sprintf (buffer, "NAM:%06d:%s%c", strlen (ROMFilename) + 1,
	     ROMFilename, 0);
    WRITE_STREAM (buffer, strlen (buffer) + 1, stream);
    FreezeStruct (stream, "CPU", &CPUPack.CPU, SnapCPU, COUNT (SnapCPU));
    FreezeStruct (stream, "REG", &CPUPack.Registers, SnapRegisters, COUNT (SnapRegisters));
    FreezeStruct (stream, "PPU", &PPU, SnapPPU, COUNT (SnapPPU));
    FreezeStruct (stream, "DMA", DMA, SnapDMA, COUNT (SnapDMA));

// RAM and VRAM
    FreezeBlock (stream, "VRA", VRAM, 0x10000);
    FreezeBlock (stream, "RAM", RAM, 0x20000);
    FreezeBlock (stream, "SRA", SRAM, 0x20000);
    FreezeBlock (stream, "FIL", FillRAM, 0x8000);
    if (Settings.APUEnabled)
    {
// APU
	FreezeStruct (stream, "APU", &APUPack.APU, SnapAPU, COUNT (SnapAPU));
	FreezeStruct (stream, "ARE", &APURegistersUncached, SnapAPURegisters,
		      COUNT (SnapAPURegisters));
	FreezeBlock (stream, "ARA", (IAPUuncached.RAM), 0x10000);
	FreezeStruct (stream, "SOU", &SoundData, SnapSoundData,
		      COUNT (SnapSoundData));
    }
    if (Settings.SA1)
    {
	SA1Pack_SA1Registers.PC = SA1Pack_SA1.PC - SA1Pack_SA1.PCBase;
	S9xSA1PackStatus ();
	FreezeStruct (stream, "SA1", &SA1Pack_SA1, SnapSA1, COUNT (SnapSA1));
	FreezeStruct (stream, "SAR", &SA1Pack_SA1Registers, SnapSA1Registers, 
		      COUNT (SnapSA1Registers));
    }
#ifdef _BSX_151_
	if (Settings.BS)
	{
        FreezeStruct (stream, "BSX", &BSX, SnapBSX, COUNT (SnapBSX));
	}
#endif

    S9xSetSoundMute (FALSE);
#ifdef ZSNES_FX
    if (Settings.SuperFX)
	S9xSuperFXPostSaveState ();
#endif	
}

static int Unfreeze (STREAM stream)
{
    char buffer [_MAX_PATH + 1];
    char rom_fname [_MAX_PATH + 1];
    
    int result;
    int tmp;

    int version;
    int len = strlen (SNAPSHOT_MAGIC) + 1 + 4 + 1;
    
    

    if ((tmp=READ_STREAM (buffer, len, stream)) != len)
    {
		return (WRONG_FORMAT);
	}

    if (strncmp (buffer, SNAPSHOT_MAGIC, strlen (SNAPSHOT_MAGIC)) != 0)
    {
	
		return (WRONG_FORMAT);
	}

    if ((version = atoi (&buffer [strlen (SNAPSHOT_MAGIC) + 1])) > SNAPSHOT_VERSION)
    {
		return (WRONG_VERSION);
	}


    if ((result = UnfreezeBlock (stream, "NAM", (uint8 *) rom_fname, _MAX_PATH)) != SUCCESS)
    {
		return (result);
	}

    if (strcasecmp (rom_fname, ROMFilename) != 0 &&
		strcasecmp (S9xBasename (rom_fname), S9xBasename (ROMFilename)) != 0)
    {
//		S9xMessage (S9X_WARNING, S9X_FREEZE_ROM_NAME,
//		    "Current loaded ROM image doesn't match that required by freeze-game file.");
    }
    
    

    uint32 old_flags = CPUPack.CPU.Flags;
    uint32 sa1_old_flags = SA1Pack_SA1.Flags;
    S9xReset ();
    S9xSetSoundMute (TRUE);

    if ((result = UnfreezeStruct (stream, "CPU", &CPUPack.CPU, SnapCPU, COUNT (SnapCPU))) != SUCCESS)
	{
		return (result);
	}


    Memory.FixROMSpeed ();
    CPUPack.CPU.Flags |= old_flags & (DEBUG_MODE_FLAG | TRACE_FLAG |
			      SINGLE_STEP_FLAG | FRAME_ADVANCE_FLAG);
    if ((result = UnfreezeStruct (stream, "REG", &CPUPack.Registers, SnapRegisters, COUNT (SnapRegisters))) != SUCCESS)
    {
		return (result);
	}

    if ((result = UnfreezeStruct (stream, "PPU", &PPU, SnapPPU, COUNT (SnapPPU))) != SUCCESS)
	{
		return (result);
	}
	

    IPPU.ColorsChanged = TRUE;
    IPPU.OBJChanged = TRUE;
    CPUPack.CPU.InDMA = FALSE;
    S9xFixColourBrightness ();
    IPPU.RenderThisFrame = FALSE;

    if ((result = UnfreezeStruct (stream, "DMA", DMA, SnapDMA, 
				  COUNT (SnapDMA))) != SUCCESS)
	{
		return (result);
	}

    if ((result = UnfreezeBlock (stream, "VRA", VRAM, 0x10000)) != SUCCESS)
	{
		return (result);		
	}

    if ((result = UnfreezeBlock (stream, "RAM", RAM, 0x20000)) != SUCCESS)
	{
		return (result);
	}

    if ((result = UnfreezeBlock (stream, "SRA", SRAM, 0x20000)) != SUCCESS)
	{
		return (result);
	}

    if ((result = UnfreezeBlock (stream, "FIL", FillRAM, 0x8000)) != SUCCESS)
	{
		return (result);
	}

  if (UnfreezeStruct (stream, "APU", &APUuncached, SnapAPU, COUNT (SnapAPU)) == SUCCESS) {
		if ((result = UnfreezeStruct (stream, "ARE", &APURegistersUncached, SnapAPURegisters,COUNT (SnapAPURegisters))) != SUCCESS) {
		    return (result);
		}		    
		if ((result = UnfreezeBlock (stream, "ARA", (IAPUuncached.RAM), 0x10000)) != SUCCESS) {
		    return (result);
		}		    
		if ((result = UnfreezeStruct (stream, "SOU", &SoundData, SnapSoundData,COUNT (SnapSoundData))) != SUCCESS) {
			return (result);
		}
	    	    
		S9xSetSoundMute (FALSE);
#ifdef ME_SOUND		
		apu_init_after_load|=2;

		(IAPUuncached.PC) = (IAPUuncached.RAM) + (APURegistersUncached.PC);
		S9xAPUUnpackStatus ();
		if (APUCheckDirectPage ()) (IAPUuncached.DirectPage) = (IAPUuncached.RAM) + 0x100;
		else(IAPUuncached.DirectPage) = (IAPUuncached.RAM);
		memcpy((void*)Uncache_APU_OutPorts,APUPack.APU.OutPorts, 4); 
#else		
		apu_init_after_load|=2;
		//(IAPUuncached.PC) = (IAPUuncached.RAM) + (APURegistersUncached.PC);
		//S9xAPUUnpackStatus ();
		//if (APUCheckDirectPage ()) (IAPUuncached.DirectPage) = (IAPUuncached.RAM) + 0x100;
		//else(IAPUuncached.DirectPage) = (IAPUuncached.RAM);
#endif	

		Settings.APUEnabled = TRUE;
		(IAPU_APUExecuting) = TRUE;
  } else {
		Settings.APUEnabled = FALSE;
		(IAPU_APUExecuting) = FALSE;
		S9xSetSoundMute (TRUE);
 }
    if ((result = UnfreezeStruct (stream, "SA1", &SA1Pack_SA1, SnapSA1, 
				  COUNT(SnapSA1))) == SUCCESS)
    {
	if ((result = UnfreezeStruct (stream, "SAR", &SA1Pack_SA1Registers, 
				      SnapSA1Registers, COUNT (SnapSA1Registers))) != SUCCESS)
	{
		return (result);
	}

	S9xFixSA1AfterSnapshotLoad ();
	SA1Pack_SA1.Flags |= sa1_old_flags & (TRACE_FLAG);
    }

#ifdef _BSX_151_
	// BS
	if (Settings.BS)
	{
		if ((result = UnfreezeStruct (stream, "BSX", &BSX, SnapBSX, COUNT (SnapBSX))) != SUCCESS)
			return (result);
	}
#endif

#ifdef ME_SOUND
	S9xFixSoundAfterSnapshotLoad ();
#else    
    //S9xFixSoundAfterSnapshotLoad ();
#endif    
    CPUPack.ICPU.ShiftedPB = CPUPack.Registers.PB << 16;
    CPUPack.ICPU.ShiftedDB = CPUPack.Registers.DB << 16;
    S9xSetPCBase (CPUPack.ICPU.ShiftedPB + CPUPack.Registers.PC);
    
    S9xUnpackStatus ();
    S9xFixCycles ();
    S9xReschedule ();
#ifdef ZSNES_FX
    if (Settings.SuperFX)
	S9xSuperFXPostLoadState ();
#endif

    S9xSRTCPostLoadState ();
    if (Settings.SDD1)	S9xSDD1PostLoadState ();
    
//    (IAPUuncached.NextAPUTimerPos) = CPUPack.CPU.Cycles * 10000L;
//	(IAPUuncached.APUTimerCounter) = 0; 

//    sceKernelDcacheWritebackInvalidateAll();
    return (SUCCESS);
}

int FreezeSize (int size, int type)
{
    switch (type)
    {
	    case uint16_ARRAY_V:
		return (size * 2);
    	case uint32_ARRAY_V:
		return (size * 4);
	    default:
		return (size);
    }
}

void FreezeStruct (STREAM stream, const char *name, void *base, FreezeData *fields,
		   int num_fields)
{
    // Work out the size of the required block
    int len = 0;
    int i;
    int j;

    for (i = 0; i < num_fields; i++)
    {
	if (fields [i].offset + FreezeSize (fields [i].size, 
					    fields [i].type) > len)
	    len = fields [i].offset + FreezeSize (fields [i].size, 
						  fields [i].type);
    }

//    uint8 *block = new uint8 [len];
    uint8 *block = (uint8*)malloc(len);
    uint8 *ptr = block;
    uint16 word;
    uint32 dword;
    int64  qword;

    // Build the block ready to be streamed out
    for (i = 0; i < num_fields; i++)
    {
	switch (fields [i].type)
	{
	case INT_V:
	    switch (fields [i].size)
	    {
	    case 1:
		*ptr++ = *((uint8 *) base + fields [i].offset);
		break;
	    case 2:
		word = *((uint16 *) ((uint8 *) base + fields [i].offset));
		*ptr++ = (uint8) (word >> 8);
		*ptr++ = (uint8) word;
		break;
	    case 4:
		dword = *((uint32 *) ((uint8 *) base + fields [i].offset));
		*ptr++ = (uint8) (dword >> 24);
		*ptr++ = (uint8) (dword >> 16);
		*ptr++ = (uint8) (dword >> 8);
		*ptr++ = (uint8) dword;
		break;
	    case 8:
		qword = *((int64 *) ((uint8 *) base + fields [i].offset));
		*ptr++ = (uint8) (qword >> 56);
		*ptr++ = (uint8) (qword >> 48);
		*ptr++ = (uint8) (qword >> 40);
		*ptr++ = (uint8) (qword >> 32);
		*ptr++ = (uint8) (qword >> 24);
		*ptr++ = (uint8) (qword >> 16);
		*ptr++ = (uint8) (qword >> 8);
		*ptr++ = (uint8) qword;
		break;
	    }
	    break;
	case uint8_ARRAY_V:
	    memmove (ptr, (uint8 *) base + fields [i].offset, fields [i].size);
	    ptr += fields [i].size;
	    break;
	case uint16_ARRAY_V:
	    for (j = 0; j < fields [i].size; j++)
	    {
		word = *((uint16 *) ((uint8 *) base + fields [i].offset + j * 2));
		*ptr++ = (uint8) (word >> 8);
		*ptr++ = (uint8) word;
	    }
	    break;
	case uint32_ARRAY_V:
	    for (j = 0; j < fields [i].size; j++)
	    {
		dword = *((uint32 *) ((uint8 *) base + fields [i].offset + j * 4));
		*ptr++ = (uint8) (dword >> 24);
		*ptr++ = (uint8) (dword >> 16);
		*ptr++ = (uint8) (dword >> 8);
		*ptr++ = (uint8) dword;
	    }
	    break;
	}
    }

    FreezeBlock (stream, name, block, len);
 //   delete block;
 	free(block);
}

void FreezeBlock (STREAM stream, const char *name, uint8 *block, int size)
{
    char *buffer;// [512];
    buffer=(char*)malloc(512);
    sprintf (buffer, "%s:%06d:", name, size);
    WRITE_STREAM (buffer, strlen (buffer), stream);
    WRITE_STREAM (block, size, stream);
    free(buffer);
    
}

int UnfreezeStruct (STREAM stream, const char *name, void *base, FreezeData *fields,
		     int num_fields)
{
    // Work out the size of the required block
    int len = 0;
    int i;
    int j;

    for (i = 0; i < num_fields; i++)
    {
	if (fields [i].offset + FreezeSize (fields [i].size, 
					    fields [i].type) > len)
	    len = fields [i].offset + FreezeSize (fields [i].size, 
						  fields [i].type);
    }

//    uint8 *block = new uint8 [len];
	uint8 *block = (uint8*)malloc(len);
    uint8 *ptr = block;
    uint16 word;
    uint32 dword;
    int64  qword;
    int result;

    if ((result = UnfreezeBlock (stream, name, block, len)) != SUCCESS)
    {
//	delete block;
 	free(block);
	return (result);
    }

    // Unpack the block of data into a C structure
    for (i = 0; i < num_fields; i++)
    {
	switch (fields [i].type)
	{
	case INT_V:
	    switch (fields [i].size)
	    {
	    case 1:
		*((uint8 *) base + fields [i].offset) = *ptr++;
		break;
	    case 2:
		word  = *ptr++ << 8;
		word |= *ptr++;
		*((uint16 *) ((uint8 *) base + fields [i].offset)) = word;
		break;
	    case 4:
		dword  = *ptr++ << 24;
		dword |= *ptr++ << 16;
		dword |= *ptr++ << 8;
		dword |= *ptr++;
		*((uint32 *) ((uint8 *) base + fields [i].offset)) = dword;
		break;
	    case 8:
		qword  = (int64) *ptr++ << 56;
		qword |= (int64) *ptr++ << 48;
		qword |= (int64) *ptr++ << 40;
		qword |= (int64) *ptr++ << 32;
		qword |= (int64) *ptr++ << 24;
		qword |= (int64) *ptr++ << 16;
		qword |= (int64) *ptr++ << 8;
		qword |= (int64) *ptr++;
		*((int64 *) ((uint8 *) base + fields [i].offset)) = qword;
		break;
	    }
	    break;
	case uint8_ARRAY_V:
	    memmove ((uint8 *) base + fields [i].offset, ptr, fields [i].size);
	    ptr += fields [i].size;
	    break;
	case uint16_ARRAY_V:
	    for (j = 0; j < fields [i].size; j++)
	    {
		word  = *ptr++ << 8;
		word |= *ptr++;
		*((uint16 *) ((uint8 *) base + fields [i].offset + j * 2)) = word;
	    }
	    break;
	case uint32_ARRAY_V:
	    for (j = 0; j < fields [i].size; j++)
	    {
		dword  = *ptr++ << 24;
		dword |= *ptr++ << 16;
		dword |= *ptr++ << 8;
		dword |= *ptr++;
		*((uint32 *) ((uint8 *) base + fields [i].offset + j * 4)) = dword;
	    }
	    break;
	}
    }

//    delete block;
 	free(block);
    return (result);
}

int UnfreezeBlock (STREAM stream, const char *name, uint8 *block, int size)
{
    char buffer [20];
    int len = 0;
    int rem = 0;
    int tmp;
    
    if (READ_STREAM (buffer, 11, stream) != 11 ||
	strncmp (buffer, name, 3) != 0 || buffer [3] != ':' ||
	(len = atoi (&buffer [4])) == 0)
    {

//	    S9xMessage(0,0,"Block length is 0");
		return (WRONG_FORMAT);
    }

    if (len > size)
    {
	rem = len - size;
	len = size;
    }
    
    if ((tmp=READ_STREAM (block, len, stream)) != len)
    {
		return (WRONG_FORMAT);
	}
	
    if (rem)
    {
//	char *junk = new char [rem];
	char *junk = (char*)malloc(rem);
	READ_STREAM (junk, rem, stream);
//	delete junk;
	free(junk);
	
    }
	
    return (SUCCESS);
}

bool8 S9xSPCDump (const char *filename)
{
    static uint8 header [] = {
	'S', 'N', 'E', 'S', '-', 'S', 'P', 'C', '7', '0', '0', ' ',
	'S', 'o', 'u', 'n', 'd', ' ', 'F', 'i', 'l', 'e', ' ',
	'D', 'a', 't', 'a', ' ', 'v', '0', '.', '3', '0', 26, 26, 26
    };
    static uint8 version = {
	0x1e
    };

    FILE *fs;

    S9xSetSoundMute (TRUE);

    if (!(fs = fopen (filename, "wb")))
	return (FALSE);

    // The SPC file format:
    // 0000: header:	'SNES-SPC700 Sound File Data v0.30',26,26,26
    // 0036: version:	$1e
    // 0037: SPC700 PC:
    // 0039: SPC700 A:
    // 0040: SPC700 X:
    // 0041: SPC700 Y:
    // 0042: SPC700 P:
    // 0043: SPC700 S:
    // 0044: Reserved: 0, 0, 0, 0
    // 0048: Title of game: 32 bytes
    // 0000: Song name: 32 bytes
    // 0000: Name of dumper: 32 bytes
    // 0000: Comments: 32 bytes
    // 0000: Date of SPC dump: 4 bytes
    // 0000: Fade out time in milliseconds: 4 bytes
    // 0000: Fade out length in milliseconds: 2 bytes
    // 0000: Default channel enables: 1 bytes
    // 0000: Emulator used to dump .SPC files: 1 byte, 1 == ZSNES
    // 0000: Reserved: 36 bytes
    // 0256: SPC700 RAM: 64K
    // ----: DSP CPUPack.Registers: 256 bytes

    if (fwrite (header, sizeof (header), 1, fs) != 1 ||
	fputc (version, fs) == EOF ||
	fseek (fs, 37, SEEK_SET) == EOF ||
	fputc ((APURegistersUncached.PC) & 0xff, fs) == EOF ||
	fputc ((APURegistersUncached.PC) >> 8, fs) == EOF ||
	fputc ((APURegistersUncached.YA).B.A, fs) == EOF ||
	fputc ((APURegistersUncached.X), fs) == EOF ||
	fputc ((APURegistersUncached.YA).B.Y, fs) == EOF ||
	fputc ((APURegistersUncached.P), fs) == EOF ||
	fputc ((APURegistersUncached.S), fs) == EOF ||
	fseek (fs, 256, SEEK_SET) == EOF ||
	fwrite ((IAPUuncached.RAM), 0x10000, 1, fs) != 1 ||
	fwrite (APUuncached.DSP, 1, 256, fs) != 1 ||
	fwrite ((APUuncached.ExtraRAM), 64, 1, fs) != 1 ||
	fclose (fs) < 0)
    {
	S9xSetSoundMute (FALSE);
	return (FALSE);
    }
    S9xSetSoundMute (FALSE);
    return (TRUE);
}

bool8 S9xUnfreezeZSNES (const char *filename)
{
	STREAM fs;
    uint8 *t;//[4000];        
	
    if (!(fs = OPEN_STREAM (filename, "rb")))
		return (FALSE);
		
	t=(uint8*)malloc(4096);		
	
    if (READ_STREAM (t, 64, fs) == 64 &&
		strncmp ((char *) t, "ZSNES Save State File V0.6", 26) == 0)
    {
		S9xReset ();
		S9xSetSoundMute (TRUE);
		
		// 28 Curr cycle
		CPUPack.CPU.V_Counter = READ_WORD (&t [29]);
		// 33 instrset
		Settings.APUEnabled = t [36];
		
		// 34 bcycpl cycles per scanline
		// 35 cycphb cyclers per hblank
		
		CPUPack.Registers.A.W   = READ_WORD (&t [41]);
		CPUPack.Registers.DB    = t [43];
		CPUPack.Registers.PB    = t [44];
		CPUPack.Registers.S.W   = READ_WORD (&t [45]);
		CPUPack.Registers.D.W   = READ_WORD (&t [47]);
		CPUPack.Registers.X.W   = READ_WORD (&t [49]);
		CPUPack.Registers.Y.W   = READ_WORD (&t [51]);
		CPUPack.Registers.P.W   = READ_WORD (&t [53]);
		CPUPack.Registers.PC    = READ_WORD (&t [55]);
		
		READ_STREAM (t, 8, fs);
		READ_STREAM (t, 3019, fs);
		S9xSetCPU (t [2], 0x4200);
		FillRAM [0x4210] = t [3];
		PPU.IRQVBeamPos = READ_WORD (&t [4]);
		PPU.IRQHBeamPos = READ_WORD (&t [2527]);
		PPU.Brightness = t [6];
		PPU.ForcedBlanking = t [8] >> 7;
		
		int i;
		for (i = 0; i < 544; i++)
			S9xSetPPU (t [0464 + i], 0x2104);
		
		PPU.OBJNameBase = READ_WORD (&t [9]);
		PPU.OBJNameSelect = READ_WORD (&t [13]) - PPU.OBJNameBase;
		switch (t [18])
		{
		case 4:
			if (t [17] == 1)
				PPU.OBJSizeSelect = 0;
			else
				PPU.OBJSizeSelect = 6;
			break;
		case 16:
			if (t [17] == 1)
				PPU.OBJSizeSelect = 1;
			else
				PPU.OBJSizeSelect = 3;
			break;
		default:
		case 64:
			if (t [17] == 1)
				PPU.OBJSizeSelect = 2;
			else
				if (t [17] == 4)
					PPU.OBJSizeSelect = 4;
				else
					PPU.OBJSizeSelect = 5;
				break;
		}
		PPU.OAMAddr = READ_WORD (&t [25]);
		PPU.SavedOAMAddr =  READ_WORD (&t [27]);
		PPU.FirstSprite = t [29];
		PPU.BGMode = t [30];
		PPU.BG3Priority = t [31];
		PPU.BG[0].BGSize = (t [32] >> 0) & 1;
		PPU.BG[1].BGSize = (t [32] >> 1) & 1;
		PPU.BG[2].BGSize = (t [32] >> 2) & 1;
		PPU.BG[3].BGSize = (t [32] >> 3) & 1;
		PPU.Mosaic = t [33] + 1;
		PPU.BGMosaic [0] = (t [34] & 1) != 0;
		PPU.BGMosaic [1] = (t [34] & 2) != 0;
		PPU.BGMosaic [2] = (t [34] & 4) != 0;
		PPU.BGMosaic [3] = (t [34] & 8) != 0;
		PPU.BG [0].SCBase = READ_WORD (&t [35]) >> 1;
		PPU.BG [1].SCBase = READ_WORD (&t [37]) >> 1;
		PPU.BG [2].SCBase = READ_WORD (&t [39]) >> 1;
		PPU.BG [3].SCBase = READ_WORD (&t [41]) >> 1;
		PPU.BG [0].SCSize = t [67];
		PPU.BG [1].SCSize = t [68];
		PPU.BG [2].SCSize = t [69];
		PPU.BG [3].SCSize = t [70];
		PPU.BG[0].NameBase = READ_WORD (&t [71]) >> 1;
		PPU.BG[1].NameBase = READ_WORD (&t [73]) >> 1;
		PPU.BG[2].NameBase = READ_WORD (&t [75]) >> 1;
		PPU.BG[3].NameBase = READ_WORD (&t [77]) >> 1;
		PPU.BG[0].HOffset = READ_WORD (&t [79]);
		PPU.BG[1].HOffset = READ_WORD (&t [81]);
		PPU.BG[2].HOffset = READ_WORD (&t [83]);
		PPU.BG[3].HOffset = READ_WORD (&t [85]);
		PPU.BG[0].VOffset = READ_WORD (&t [89]);
		PPU.BG[1].VOffset = READ_WORD (&t [91]);
		PPU.BG[2].VOffset = READ_WORD (&t [93]);
		PPU.BG[3].VOffset = READ_WORD (&t [95]);
		PPU.VMA.Increment = READ_WORD (&t [97]) >> 1;
		PPU.VMA.High = t [99];
#ifndef CORRECT_VRAM_READS
                IPPU.FirstVRAMRead = t [100];
#endif
		S9xSetPPU (t [2512], 0x2115);
		PPU.VMA.Address = READ_DWORD (&t [101]);
		for (i = 0; i < 512; i++)
			S9xSetPPU (t [1488 + i], 0x2122);
		
		PPU.CGADD = (uint8) READ_WORD (&t [105]);
		FillRAM [0x212c] = t [108];
		FillRAM [0x212d] = t [109];
		PPU.ScreenHeight = READ_WORD (&t [111]);
		FillRAM [0x2133] = t [2526];
		FillRAM [0x4202] = t [113];
		FillRAM [0x4204] = t [114];
		FillRAM [0x4205] = t [115];
		FillRAM [0x4214] = t [116];
		FillRAM [0x4215] = t [117];
		FillRAM [0x4216] = t [118];
		FillRAM [0x4217] = t [119];
		PPU.VBeamPosLatched = READ_WORD (&t [122]);
		PPU.HBeamPosLatched = READ_WORD (&t [120]);
		PPU.Window1Left = t [127];
		PPU.Window1Right = t [128];
		PPU.Window2Left = t [129];
		PPU.Window2Right = t [130];
		S9xSetPPU (t [131] | (t [132] << 4), 0x2123);
		S9xSetPPU (t [133] | (t [134] << 4), 0x2124);
		S9xSetPPU (t [135] | (t [136] << 4), 0x2125);
		S9xSetPPU (t [137], 0x212a);
		S9xSetPPU (t [138], 0x212b);
		S9xSetPPU (t [139], 0x212e);
		S9xSetPPU (t [140], 0x212f);
		S9xSetPPU (t [141], 0x211a);
		PPU.MatrixA = READ_WORD (&t [142]);
		PPU.MatrixB = READ_WORD (&t [144]);
		PPU.MatrixC = READ_WORD (&t [146]);
		PPU.MatrixD = READ_WORD (&t [148]);
		PPU.CentreX = READ_WORD (&t [150]);
		PPU.CentreY = READ_WORD (&t [152]);
		// JoyAPos t[154]
		// JoyBPos t[155]
		FillRAM [0x2134] = t [156]; // Matrix mult
		FillRAM [0x2135] = t [157]; // Matrix mult
		FillRAM [0x2136] = t [158]; // Matrix mult
		PPU.WRAM = READ_DWORD (&t [161]);
		
		for (i = 0; i < 128; i++)
			S9xSetCPU (t [165 + i], 0x4300 + i);
		
		if (t [294])
			CPUPack.CPU.IRQActive |= PPU_V_BEAM_IRQ_SOURCE | PPU_H_BEAM_IRQ_SOURCE;
		
		S9xSetCPU (t [296], 0x420c);
		// hdmadata t[297] + 8 * 19
		PPU.FixedColourRed = t [450];
		PPU.FixedColourGreen = t [451];
		PPU.FixedColourBlue = t [452];
		S9xSetPPU (t [454], 0x2130);
		S9xSetPPU (t [455], 0x2131);
		// vraminctype ...
		
		READ_STREAM (RAM, 128 * 1024, fs);
		READ_STREAM (VRAM, 64 * 1024, fs);
		
		if (Settings.APUEnabled)
		{
			// SNES SPC700 RAM (64K)
			READ_STREAM ((IAPUuncached.RAM), 64 * 1024, fs);
			
			// Junk 16 bytes
			READ_STREAM (t, 16, fs);
			
			// SNES SPC700 state and internal ZSNES SPC700 emulation state
			READ_STREAM (t, 304, fs);
			
			(APURegistersUncached.PC)   = READ_DWORD (&t [0]);
			(APURegistersUncached.YA).B.A = t [4];
			(APURegistersUncached.X)    = t [8];
			(APURegistersUncached.YA).B.Y = t [12];
			(APURegistersUncached.P)    = t [16];
			(APURegistersUncached.S)    = t [24];
			
			(APUuncached.Cycles) = READ_DWORD (&t [32]);
			Uncache_APU_Cycles = APUuncached.Cycles;
			(APUuncached.ShowROM) = ((IAPUuncached.RAM) [0xf1] & 0x80) != 0;
			(APUuncached.OutPorts) [0] = t [36];
			(APUuncached.OutPorts) [1] = t [37];
			(APUuncached.OutPorts) [2] = t [38];
			(APUuncached.OutPorts) [3] = t [39];
			
			(APUuncached.TimerEnabled) [0] = (t [40] & 1) != 0;
			(APUuncached.TimerEnabled) [1] = (t [40] & 2) != 0;
			(APUuncached.TimerEnabled) [2] = (t [40] & 4) != 0;
			S9xSetAPUTimer (0xfa, t [41]);
			S9xSetAPUTimer (0xfb, t [42]);
			S9xSetAPUTimer (0xfc, t [43]);
			(APUuncached.Timer) [0] = t [44];
			(APUuncached.Timer) [1] = t [45];
			(APUuncached.Timer) [2] = t [46];
			
			memmove ((APUuncached.ExtraRAM), &t [48], 64);
			
			// Internal ZSNES sound DSP state
			READ_STREAM (t, 1068, fs);
			
			// SNES sound DSP register values
			READ_STREAM (t, 256, fs);
			
			uint8 saved = (IAPUuncached.RAM) [0xf2];
			
			for (i = 0; i < 128; i++)
			{
				switch (i)
				{
				case APU_KON:
				case APU_KOFF:
					break;
				case APU_FLG:
					t [i] &= ~APU_SOFT_RESET;
				default:
					(IAPUuncached.RAM) [0xf2] = i;
					S9xSetAPUDSP (t [i]);
					break;
				}
			}
			(IAPUuncached.RAM) [0xf2] = APU_KON;
			S9xSetAPUDSP (t [APU_KON]);
			(IAPUuncached.RAM) [0xf2] = saved;
			
			S9xSetSoundMute (FALSE);
//#ifdef ME_SOUND		
		apu_init_after_load|=2;
//#else				
//			(IAPUuncached.PC) = (IAPUuncached.RAM) + (APURegistersUncached.PC);
//			S9xAPUUnpackStatus ();
//			if (APUCheckDirectPage ())
//				(IAPUuncached.DirectPage) = (IAPUuncached.RAM) + 0x100;
//			else
//				(IAPUuncached.DirectPage) = (IAPUuncached.RAM);
//#endif		

			Settings.APUEnabled = TRUE;
			(IAPU_APUExecuting) = TRUE;
		}
		else
		{
			Settings.APUEnabled = FALSE;
			(IAPU_APUExecuting) = FALSE;
			S9xSetSoundMute (TRUE);
		}
		
		if (Settings.SuperFX)
		{
			READ_STREAM (SRAM, 64 * 1024, fs);
			SEEK_STREAM (64 * 1024, SEEK_CUR,fs);
			READ_STREAM (FillRAM + 0x7000, 692, fs);
		}
		if (Settings.SA1)
		{
			READ_STREAM (t, 2741, fs);
			S9xSetSA1 (t [4], 0x2200);  // Control
			S9xSetSA1 (t [12], 0x2203);	// ResetV low
			S9xSetSA1 (t [13], 0x2204); // ResetV hi
			S9xSetSA1 (t [14], 0x2205); // NMI low
			S9xSetSA1 (t [15], 0x2206); // NMI hi
			S9xSetSA1 (t [16], 0x2207); // IRQ low
			S9xSetSA1 (t [17], 0x2208); // IRQ hi
			S9xSetSA1 (((READ_DWORD (&t [28]) - (4096*1024-0x6000))) >> 13, 0x2224);
			S9xSetSA1 (t [36], 0x2201);
			S9xSetSA1 (t [41], 0x2209);
			
			SA1Pack_SA1Registers.A.W = READ_DWORD (&t [592]);
			SA1Pack_SA1Registers.X.W = READ_DWORD (&t [596]);
			SA1Pack_SA1Registers.Y.W = READ_DWORD (&t [600]);
			SA1Pack_SA1Registers.D.W = READ_DWORD (&t [604]);
			SA1Pack_SA1Registers.DB  = t [608];
			SA1Pack_SA1Registers.PB  = t [612];
			SA1Pack_SA1Registers.S.W = READ_DWORD (&t [616]);
			SA1Pack_SA1Registers.PC  = READ_DWORD (&t [636]);
			SA1Pack_SA1Registers.P.W = t [620] | (t [624] << 8);
			
			memmove (&FillRAM [0x3000], t + 692, 2 * 1024);
			
			READ_STREAM (SRAM, 64 * 1024, fs);
			SEEK_STREAM (64 * 1024, SEEK_CUR,fs);
			S9xFixSA1AfterSnapshotLoad ();
		}
/*		if(Settings.SPC7110)
		{
			uint32 temp;
			READ_STREAM(&s7r.bank50, 0x10000, fs);
			
			//NEWSYM SPCMultA, dd 0  4820-23
			READ_STREAM(&temp, 4, fs);
			
			s7r.reg4820=temp&(0x0FF);
			s7r.reg4821=(temp>>8)&(0x0FF);
			s7r.reg4822=(temp>>16)&(0x0FF);
			s7r.reg4823=(temp>>24)&(0x0FF);

			//NEWSYM SPCMultB, dd 0				4824-5
			READ_STREAM(&temp, 4,fs);
			s7r.reg4824=temp&(0x0FF);
			s7r.reg4825=(temp>>8)&(0x0FF);


			//NEWSYM SPCDivEnd, dd 0				4826-7
			READ_STREAM(&temp, 4,fs);
			s7r.reg4826=temp&(0x0FF);
			s7r.reg4827=(temp>>8)&(0x0FF);

			//NEWSYM SPCMulRes, dd 0				4828-B
			READ_STREAM(&temp,  4, fs);
			
			s7r.reg4828=temp&(0x0FF);
			s7r.reg4829=(temp>>8)&(0x0FF);
			s7r.reg482A=(temp>>16)&(0x0FF);
			s7r.reg482B=(temp>>24)&(0x0FF);
			
			//NEWSYM SPCDivRes, dd 0				482C-D
			READ_STREAM(&temp, 4,fs);
			s7r.reg482C=temp&(0x0FF);
			s7r.reg482D=(temp>>8)&(0x0FF);

			//NEWSYM SPC7110BankA, dd 020100h		4831-3
			READ_STREAM(&temp, 4, fs);
			
			s7r.reg4831=temp&(0x0FF);
			s7r.reg4832=(temp>>8)&(0x0FF);
			s7r.reg4833=(temp>>16)&(0x0FF);
			
			//NEWSYM SPC7110RTCStat, dd 0			4840,init,command, index
			READ_STREAM(&temp, 4, fs);
			
			s7r.reg4840=temp&(0x0FF);

//NEWSYM SPC7110RTC, db 00,00,00,00,00,00,01,00,01,00,00,00,00,00,0Fh,00
READ_STREAM(&temp, 4, fs);
if(Settings.SPC7110RTC)
{
	rtc_f9.reg[0]=temp&(0x0FF);
	rtc_f9.reg[1]=(temp>>8)&(0x0FF);
	rtc_f9.reg[2]=(temp>>16)&(0x0FF);
	rtc_f9.reg[3]=(temp>>24)&(0x0FF);
}
READ_STREAM(&temp, 4, fs);
if(Settings.SPC7110RTC)
{
	rtc_f9.reg[4]=temp&(0x0FF);
	rtc_f9.reg[5]=(temp>>8)&(0x0FF);
	rtc_f9.reg[6]=(temp>>16)&(0x0FF);
	rtc_f9.reg[7]=(temp>>24)&(0x0FF);
}
READ_STREAM(&temp, 4, fs);
if(Settings.SPC7110RTC)
{
	rtc_f9.reg[8]=temp&(0x0FF);
	rtc_f9.reg[9]=(temp>>8)&(0x0FF);
	rtc_f9.reg[10]=(temp>>16)&(0x0FF);
	rtc_f9.reg[11]=(temp>>24)&(0x0FF);
}
READ_STREAM(&temp, 4, fs);
if(Settings.SPC7110RTC)
{
	rtc_f9.reg[12]=temp&(0x0FF);
	rtc_f9.reg[13]=(temp>>8)&(0x0FF);
	rtc_f9.reg[14]=(temp>>16)&(0x0FF);
	rtc_f9.reg[15]=(temp>>24)&(0x0FF);
}
//NEWSYM SPC7110RTCB, db 00,00,00,00,00,00,01,00,01,00,00,00,00,01,0Fh,06
READ_STREAM(&temp, 4, fs);
READ_STREAM(&temp, 4, fs);
READ_STREAM(&temp, 4, fs);
READ_STREAM(&temp, 4, fs);

//NEWSYM SPCROMPtr, dd 0		4811-4813
			READ_STREAM(&temp, 4, fs);
			
			s7r.reg4811=temp&(0x0FF);
			s7r.reg4812=(temp>>8)&(0x0FF);
			s7r.reg4813=(temp>>16)&(0x0FF);
//NEWSYM SPCROMtoI, dd SPCROMPtr
			READ_STREAM(&temp, 4, fs);
//NEWSYM SPCROMAdj, dd 0      4814-5
			READ_STREAM(&temp, 4, fs);
			s7r.reg4814=temp&(0x0FF);
			s7r.reg4815=(temp>>8)&(0x0FF);
//NEWSYM SPCROMInc, dd 0		4816-7
			READ_STREAM(&temp, 4, fs);
			s7r.reg4816=temp&(0x0FF);
			s7r.reg4817=(temp>>8)&(0x0FF);
//NEWSYM SPCROMCom, dd 0		4818
READ_STREAM(&temp, 4, fs);
			
			s7r.reg4818=temp&(0x0FF);
//NEWSYM SPCCompPtr, dd 0  4801-4804 (+b50i) if"manual"
			READ_STREAM(&temp, 4, fs);

			//do table check
			
			s7r.reg4801=temp&(0x0FF);
			s7r.reg4802=(temp>>8)&(0x0FF);
			s7r.reg4803=(temp>>16)&(0x0FF);
			s7r.reg4804=(temp>>24)&(0x0FF);
///NEWSYM SPCDecmPtr, dd 0  4805-6   +b50i
			READ_STREAM(&temp, 4, fs);
			s7r.reg4805=temp&(0x0FF);
			s7r.reg4806=(temp>>8)&(0x0FF);
//NEWSYM SPCCompCounter, dd 0  4809-A
			READ_STREAM(&temp, 4, fs);
			s7r.reg4809=temp&(0x0FF);
			s7r.reg480A=(temp>>8)&(0x0FF);
//NEWSYM SPCCompCommand, dd 0  480B
READ_STREAM(&temp, 4, fs);
			
			s7r.reg480B=temp&(0x0FF);
//NEWSYM SPCCheckFix, dd 0		written(if 1, then set writtne to max value!)
READ_STREAM(&temp, 4, fs);
(temp&(0x0FF))?s7r.written=0x1F:s7r.written=0x00;
//NEWSYM SPCSignedVal, dd 0	482E
READ_STREAM(&temp, 4, fs);
			
			s7r.reg482E=temp&(0x0FF);
			
		}*/
		CLOSE_STREAM (fs);
		
		Memory.FixROMSpeed ();
		IPPU.ColorsChanged = TRUE;
		IPPU.OBJChanged = TRUE;
		CPUPack.CPU.InDMA = FALSE;
		S9xFixColourBrightness ();
		IPPU.RenderThisFrame = FALSE;
		
		S9xFixSoundAfterSnapshotLoad ();
		CPUPack.ICPU.ShiftedPB = CPUPack.Registers.PB << 16;
		CPUPack.ICPU.ShiftedDB = CPUPack.Registers.DB << 16;
		S9xSetPCBase (CPUPack.ICPU.ShiftedPB + CPUPack.Registers.PC);
		S9xUnpackStatus ();
		S9xFixCycles ();
		S9xReschedule ();
#ifdef ZSNES_FX
		if (Settings.SuperFX)
			S9xSuperFXPostLoadState ();
#endif
		free(t);
		return (TRUE);
    }
    CLOSE_STREAM (fs);
    
    free(t);
    
    return (FALSE);
   
}
