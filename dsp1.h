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

#ifndef _DSP1_H_
#define _DSP1_H_

void DSP1SetByte(uint8 byte, uint16 address);
uint8 DSP1GetByte(uint16 address);

// Simple vector and matrix types
//typedef float MATRIX[3][3];
//typedef float VECTOR[3];

//enum AttitudeMatrix { MatrixA, MatrixB, MatrixC };

struct SDSP1 {
    bool8 waiting4command;
    bool8 first_parameter;
    uint8 command;
    uint32 in_count;
    uint32 in_index;
    uint32 out_count;
    uint32 out_index;
	short Op02FX;
	short Op02FY;
	short Op02FZ;
	short Op02LFE;
	short Op02LES;
	unsigned short Op02AAS;
	unsigned short Op02AZS;
	unsigned short Op02VOF;
	unsigned short Op02VVA;
	short ScrDispl;
	short Op0AVS;
	short Op0AA;
	short Op0AB;
	short Op0AC;
	short Op0AD;

	float RXRes,RYRes;

	float NAzsB,NAasB;
	float ViewerXc;
	float ViewerYc;
	float ViewerZc;
	float CenterX,CenterY;
	//output was 512 for DSP-2 work, updated to reflect current thinking on DSP-3
	union {
		uint8 output [12];
		uint16 output16 [6];
		uint32 output32 [3];
	};
	union {
	    uint8 parameters [512];
		uint16 parameters16 [256];
	};
	short matrixC[3][3];
	short matrixB[3][3];
	short matrixA[3][3];

	
	
/*
    // Attitude matrices
    MATRIX vMa;
    MATRIX vMb;
    MATRIX vMc;
    
    // Matrix and translaton vector for
    // transforming a 3D position into the global coordinate system,
    // from the view space coordinate system.
    MATRIX vM;
    VECTOR vT;

    // Focal distance
    float vFov;

    // A precalculated value for optimization
    float vPlaneD;
    
    // Raster position of horizon
    float vHorizon;

    // Convert a 2D screen coordinate to a 3D ground coordinate in global coordinate system.
    void ScreenToGround(VECTOR &v, float X2d, float Y2d);

    MATRIX &GetMatrix( AttitudeMatrix Matrix );*/
};


START_EXTERN_C
void S9xResetDSP1 ();

#define S9xGetDSP(Address) DSP1GetByte(Address)
#define S9xSetDSP(Byte, Address) DSP1SetByte(Byte, Address)
//uint8 S9xGetDSP (uint16 Address);
//void S9xSetDSP (uint8 Byte, uint16 Address);
END_EXTERN_C

#ifndef __GP32__ 
extern struct SDSP1 DSP1;
#else
EXTERN_C struct SDSP1 DSP1;
#endif

#endif
