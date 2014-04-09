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

// modify to little endian(PSP) version by ruka

#include "snes9x.h"
#include "dsp1.h"
#include "missing.h"
#include "memmap.h"


//#include "math.h"

#include "dsp1emu.c"
//#include "dsp2emu.c"

void S9xInitDSP1 ()
{
    static bool8 init = FALSE;
    
    if (!init)
    {
        InitDSP ();
        init = TRUE;
    }
}

void S9xResetDSP1 ()
{
    S9xInitDSP1 ();
    
    DSP1.waiting4command = TRUE;
    DSP1.in_count = 0;
    DSP1.out_count = 0;
    DSP1.in_index = 0;
    DSP1.out_index = 0;
    DSP1.first_parameter = TRUE;
}

void DSP1SetByte(uint8 byte, uint16 address)
{
    if( (address & 0xf000) == 0x6000 || (address & 0x7fff) < 0x4000 )
    {
//		if ((address & 1) == 0)
//		{
		if((DSP1.command==0x0A||DSP1.command==0x1A)&&DSP1.out_count!=0)
		{
			DSP1.out_count--;
			DSP1.out_index++;			
			return;
		}
		else if (DSP1.waiting4command)
		{
			DSP1.command = byte;
			DSP1.in_index = 0;
			DSP1.waiting4command = FALSE;
			DSP1.first_parameter = TRUE;
//			printf("Op%02X\n",byte);
			// Mario Kart uses 0x00, 0x02, 0x06, 0x0c, 0x28, 0x0a
			switch (byte)
			{
			case 0x00: DSP1.in_count = 2;	break;
			case 0x30:
			case 0x10: DSP1.in_count = 2;	break;
			case 0x20: DSP1.in_count = 2;	break;
			case 0x24:
			case 0x04: DSP1.in_count = 2;	break;
			case 0x08: DSP1.in_count = 3;	break;
			case 0x18: DSP1.in_count = 4;	break;
			case 0x28: DSP1.in_count = 3;	break;
			case 0x38: DSP1.in_count = 4;	break;
			case 0x2c:
			case 0x0c: DSP1.in_count = 3;	break;
			case 0x3c:
			case 0x1c: DSP1.in_count = 6;	break;
			case 0x32:
			case 0x22:
			case 0x12:
			case 0x02: DSP1.in_count = 7;	break;
			case 0x0a: DSP1.in_count = 1;	break;
			case 0x3a:
			case 0x2a:
			case 0x1a: 
				DSP1. command =0x1a;
				DSP1.in_count = 1;	break;
			case 0x16:
			case 0x26:
			case 0x36:
			case 0x06: DSP1.in_count = 3;	break;
			case 0x1e:
			case 0x2e:
			case 0x3e:
			case 0x0e: DSP1.in_count = 2;	break;
			case 0x05:
			case 0x35:
			case 0x31:
			case 0x01: DSP1.in_count = 4;	break;
			case 0x15:
			case 0x11: DSP1.in_count = 4;	break;
			case 0x25:
			case 0x21: DSP1.in_count = 4;	break;
			case 0x09:
			case 0x39:
			case 0x3d:
			case 0x0d: DSP1.in_count = 3;	break;
			case 0x19:
			case 0x1d: DSP1.in_count = 3;	break;
			case 0x29:
			case 0x2d: DSP1.in_count = 3;	break;
			case 0x33:
			case 0x03: DSP1.in_count = 3;	break;
			case 0x13: DSP1.in_count = 3;	break;
			case 0x23: DSP1.in_count = 3;	break;
			case 0x3b:
			case 0x0b: DSP1.in_count = 3;	break;
			case 0x1b: DSP1.in_count = 3;	break;
			case 0x2b: DSP1.in_count = 3;	break;
			case 0x34:
			case 0x14: DSP1.in_count = 6;	break;
			case 0x07:
			case 0x0f: DSP1.in_count = 1;	break;
			case 0x27:
			case 0x2F: DSP1.in_count=1; break;
			case 0x17:
			case 0x37:
			case 0x3F:
				DSP1.command=0x1f;
			case 0x1f: DSP1.in_count = 1;	break;
				//		    case 0x80: DSP1.in_count = 2;	break;
			default:
				//printf("Op%02X\n",byte);
			case 0x80:
				DSP1.in_count = 0;
				DSP1.waiting4command = TRUE;
				DSP1.first_parameter = TRUE;
				break;
			}
			DSP1.in_count<<=1;
		}
		else
		{
			DSP1.parameters [DSP1.in_index] = byte;
			DSP1.first_parameter = FALSE;
			DSP1.in_index++;
		}
		
		if (DSP1.waiting4command ||
			(DSP1.first_parameter && byte == 0x80))
		{
			DSP1.waiting4command = TRUE;
			DSP1.first_parameter = FALSE;
		}
		else if(DSP1.first_parameter && (DSP1.in_count != 0 || (DSP1.in_count==0&&DSP1.in_index==0)))
		{
		}
		else
		{
			if (DSP1.in_count)
			{
				if (--DSP1.in_count == 0)
				{
					// Actually execute the command
					DSP1.waiting4command = TRUE;
					DSP1.out_index = 0;
					switch (DSP1.command)
					{
					case 0x1f:
						DSP1.out_count=2048;
						break;
					case 0x00:	// Multiple
						{
						short Op00Multiplicand = (int16) DSP1.parameters16 [0];
						short Op00Multiplier = (int16) DSP1.parameters16 [1];
						
						short Op00Result= Op00Multiplicand * Op00Multiplier >> 15;
						
						DSP1.out_count = 2;
						DSP1.output16 [0] = Op00Result;
						}
						break;

					case 0x20:	// Multiple
						{
						short Op20Multiplicand = (int16) DSP1.parameters16 [0];
						short Op20Multiplier = (int16) DSP1.parameters16 [1];
						
						short Op20Result= Op20Multiplicand * Op20Multiplier >> 15;
						Op20Result++;
						
						DSP1.out_count = 2;
						DSP1.output16 [0] = Op20Result;
						}
						break;
						
					case 0x30:
					case 0x10:	// Inverse
						{
						signed short Op10Coefficient = (int16) DSP1.parameters16 [0];
						signed short Op10Exponent = (int16) DSP1.parameters16 [1];
						signed short Op10CoefficientR;
						signed short Op10ExponentR;

						DSP1_Inverse(Op10Coefficient, Op10Exponent, &Op10CoefficientR, &Op10ExponentR);
						
						DSP1.out_count = 4;
						DSP1.output16 [0] = Op10CoefficientR;
						DSP1.output16 [1] = Op10ExponentR;
						}
						break;
						
					case 0x24:
					case 0x04:	// Sin and Cos of angle
						{
						short Op04Angle = (int16) DSP1.parameters16 [0];
						short Op04Radius = (uint16) DSP1.parameters16 [1];
						
						short Op04Sin = DSP1_Sin(Op04Angle) * Op04Radius >> 15;
						short Op04Cos = DSP1_Cos(Op04Angle) * Op04Radius >> 15;
						
						DSP1.out_count = 4;
						DSP1.output16 [0] = Op04Sin;
						DSP1.output16 [1] = Op04Cos;
						}
						break;
						
					case 0x08:	// Radius
						{
						short Op08X = (int16) DSP1.parameters16 [0];
						short Op08Y = (int16) DSP1.parameters16 [1];
						short Op08Z = (int16) DSP1.parameters16 [2];
						
						int Op08Size = (Op08X * Op08X + Op08Y * Op08Y + Op08Z * Op08Z) << 1;
						
						DSP1.out_count = 4;
						DSP1.output32 [0] = Op08Size; 
						}
						break;
					case 0x18:	// Range
						{
						short Op18X = (int16) DSP1.parameters16 [0];
						short Op18Y = (int16) DSP1.parameters16 [1];
						short Op18Z = (int16) DSP1.parameters16 [2];
						short Op18R = (int16) DSP1.parameters16 [3];
						
						short Op18D = (Op18X * Op18X + Op18Y * Op18Y + Op18Z * Op18Z - Op18R * Op18R) >> 15;
						
						DSP1.out_count = 2;
						DSP1.output16 [0] = Op18D;
						}
						break;
					case 0x38:	// Range
						{
						short Op38X = (int16) DSP1.parameters16 [0];
						short Op38Y = (int16) DSP1.parameters16 [1];
						short Op38Z = (int16) DSP1.parameters16 [2];
						short Op38R = (int16) DSP1.parameters16 [3];
						
						short Op38D = (Op38X * Op38X + Op38Y * Op38Y + Op38Z * Op38Z - Op38R * Op38R) >> 15;
						Op38D++;
						
						DSP1.out_count = 2;
						DSP1.output16 [0] = Op38D;
						}
						break;
					case 0x28:	// Distance (vector length)
						{
						short Op28X = (int16) DSP1.parameters16 [0];
						short Op28Y = (int16) DSP1.parameters16 [1];
						short Op28Z = (int16) DSP1.parameters16 [2];
						int Radius = Op28X * Op28X + Op28Y * Op28Y + Op28Z * Op28Z;
						short Op28R;

						if (Radius == 0) Op28R = 0;
						else
						{
							short C, E;
							DSP1_Normalizefloat(Radius, &C, &E);
							if (E & 1) C = C * 0x4000 >> 15;

							short Pos = C * 0x0040 >> 15;

							short Node1 = DSP1ROM[(0x00d5 + Pos)&1023];
							short Node2 = DSP1ROM[(0x00d6 + Pos)&1023];

							Op28R = ((Node2 - Node1) * (C & 0x1ff) >> 9) + Node1;
							Op28R >>= (E >> 1);
						}
						DSP1.out_count = 2;
						DSP1.output16 [0] = Op28R;
						}
						break;
						
					case 0x2c:
					case 0x0c:	// Rotate (2D rotate)
						{
						short Op0CA = (int16) DSP1.parameters16 [0];
						short Op0CX1 = (int16) DSP1.parameters16 [1];
						short Op0CY1 = (int16) DSP1.parameters16 [2];
						
						short Op0CX2 = (Op0CY1 * DSP1_Sin(Op0CA) >> 15) + (Op0CX1 * DSP1_Cos(Op0CA) >> 15);
						short Op0CY2 = (Op0CY1 * DSP1_Cos(Op0CA) >> 15) - (Op0CX1 * DSP1_Sin(Op0CA) >> 15);

						
						DSP1.out_count = 4;
						DSP1.output16 [0] = Op0CX2;
						DSP1.output16 [1] = Op0CY2;
						}
						break;
						
					case 0x3c:
					case 0x1c:	// Polar (3D rotate)
						{
						short Op1CZ = DSP1.parameters16 [0];
						//MK: reversed X and Y on neviksti and John's advice.
						short Op1CY = DSP1.parameters16 [1];
						short Op1CX = DSP1.parameters16 [2];
						short Op1CXBR = DSP1.parameters16 [3];
						short Op1CYBR = DSP1.parameters16 [4];
						short Op1CZBR = DSP1.parameters16 [5];
						
						// Rotate Around Op1CZ1
						short Op1CX1 = (Op1CYBR * DSP1_Sin(Op1CZ) >> 15) + (Op1CXBR * DSP1_Cos(Op1CZ) >> 15);
						short Op1CY1 = (Op1CYBR * DSP1_Cos(Op1CZ) >> 15) - (Op1CXBR * DSP1_Sin(Op1CZ) >> 15);
						Op1CXBR = Op1CX1; Op1CYBR = Op1CY1;

						// Rotate Around Op1CY1
						short Op1CZ1 = (Op1CXBR * DSP1_Sin(Op1CY) >> 15) + (Op1CZBR * DSP1_Cos(Op1CY) >> 15);
						Op1CX1 = (Op1CXBR * DSP1_Cos(Op1CY) >> 15) - (Op1CZBR * DSP1_Sin(Op1CY) >> 15);
						short Op1CXAR = Op1CX1; Op1CZBR = Op1CZ1;

						// Rotate Around Op1CX1	
						Op1CY1 = (Op1CZBR * DSP1_Sin(Op1CX) >> 15) + (Op1CYBR * DSP1_Cos(Op1CX) >> 15);
						Op1CZ1 = (Op1CZBR * DSP1_Cos(Op1CX) >> 15) - (Op1CYBR * DSP1_Sin(Op1CX) >> 15);
						short Op1CYAR = Op1CY1; short Op1CZAR = Op1CZ1;
						
						DSP1.out_count = 6;
						DSP1.output16 [0] = Op1CXAR;
						DSP1.output16 [1] = Op1CYAR;
						DSP1.output16 [2] = Op1CZAR;
						}
						break;
						
					case 0x32:
					case 0x22:
					case 0x12:
					case 0x02:	// Parameter (Projection)
						
						DSPOp02 ();
						
						break;
						
					case 0x3a:  //1a Mirror
					case 0x2a:  //1a Mirror
					case 0x1a:	// Raster mode 7 matrix data
					case 0x0a:
						DSP1.Op0AVS = (short)DSP1.parameters16 [0];
						
						DSPOp0A ();
						
						DSP1.out_count = 8;
						DSP1.output16 [0] = DSP1.Op0AA;
						DSP1.output16 [1] = DSP1.Op0AB;
						DSP1.output16 [2] = DSP1.Op0AC;
						DSP1.output16 [3] = DSP1.Op0AD;
						DSP1.in_index=0;
						break;
						
					case 0x16:
					case 0x26:
					case 0x36:
					case 0x06:	// Project object
						DSPOp06 ();
						break;
					case 0x1e:
					case 0x2e:
					case 0x3e:
					case 0x0e:	// Target
						{
						short Op0EH = (int16) DSP1.parameters16 [0];
						short Op0EV = (int16) DSP1.parameters16 [1];
						
						// screen Directions UP
						GetRXYPos(Op0EV, Op0EH);
						
						DSP1.out_count = 4;
						DSP1.output16 [0] = (short)(DSP1.RXRes);
						DSP1.output16 [1] = (short)(DSP1.RYRes);
						}
						break;
						// Extra commands used by Pilot Wings
					case 0x05:
					case 0x35:
					case 0x31:
					case 0x01: // Set attitude matrix A
						DSPOp01 ();
						break;
					case 0x15:	
					case 0x11:	// Set attitude matrix B
						DSPOp11 ();
						break;
					case 0x25:
					case 0x21:	// Set attitude matrix C
						DSPOp21 ();
						break;
					case 0x09:
					case 0x39:
					case 0x3d:
					case 0x0d:	// Objective matrix A
						DSPOp0D ();
						break;
					case 0x19:
					case 0x1d:	// Objective matrix B
						DSPOp1D ();
						break;
					case 0x29:
					case 0x2d:	// Objective matrix C
						DSPOp2D ();
						break;
					case 0x33:
					case 0x03:	// Subjective matrix A
						DSPOp03 ();
						break;
					case 0x13:	// Subjective matrix B
						DSPOp13 ();
						break;
					case 0x23:	// Subjective matrix C
						DSPOp23 ();
						break;
					case 0x3b:
					case 0x0b:
						DSPOp0B ();
						break;
					case 0x1b:
						DSPOp1B ();
						break;
					case 0x2b:
						DSPOp2B ();
						break;
					case 0x34:
					case 0x14:	
						DSPOp14 ();
						break;
					case 0x27:
					case 0x2F:
						DSP1.out_count = 2;
						DSP1.output16 [0] = 0x100;
						break;
					case 0x07:
					case 0x0F:
						DSP1.out_count = 2;
						DSP1.output16 [0] = 0x0000;
						break;
					default:
						break;
					}
				}
			}
		}
    }
}

uint8 DSP1GetByte(uint16 address)
{
	uint8 t;
    if ((address & 0xf000) == 0x6000 ||
		(address&0x7fff) < 0x4000)
    {
		if (DSP1.out_count)
		{
			t = (uint8) DSP1.output [DSP1.out_index];
			DSP1.out_index++;
			if (--DSP1.out_count == 0)
			{
				if (DSP1.command == 0x1a || DSP1.command == 0x0a)
				{
					DSPOp0A ();
					DSP1.out_count = 8;
					DSP1.out_index = 0;
					DSP1.output16 [0] = DSP1.Op0AA;
					DSP1.output16 [1] = DSP1.Op0AB;
					DSP1.output16 [2] = DSP1.Op0AC;
					DSP1.output16 [3] = DSP1.Op0AD;
				}
				if(DSP1.command==0x1f)
				{
					t=((uint8*)DSP1ROM)[DSP1.out_index-1];
				}
			}
			DSP1.waiting4command = TRUE;
		}
		else
		{
			// Top Gear 3000 requires this value....
	//		if(4==Settings.DSPVersion)
				t = 0xff;
			//Ballz3d requires this one:
	//		else t = 0x00;
		}
    }
    else t = 0x80;
	return t;
}

