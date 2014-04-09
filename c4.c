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

#ifndef __GP32__
//#include <stdlib.h>
#endif

#include "c4.h"
//#include "memmap.h"


#include <math.h>

//#define abs(i) (i>=0?i:-i)

//extern "C" {


short C4WFXVal;
short C4WFYVal;
short C4WFZVal;
short C4WFX2Val;
short C4WFY2Val;
short C4WFDist;
short C4WFScale;

static float tanval;
static float c4x, c4y, c4z;
static float c4x2, c4y2, c4z2;




void C4TransfWireFrame ()
{
    c4x = (float) C4WFXVal;
    c4y = (float) C4WFYVal;
    c4z = (float) C4WFZVal - 0x95;
    
    // Rotate X
    tanval = -(float) C4WFX2Val * 3.14159265 * 2 / 128;
    c4y2 = c4y * cosf (tanval) - c4z * sinf (tanval);
    c4z2 = c4y * sinf (tanval) + c4z * cosf (tanval);
    
    // Rotate Y
    tanval = -(float)C4WFY2Val*3.14159265*2/128;
    c4x2 = c4x * cosf (tanval) + c4z2 * sinf (tanval);
    c4z = c4x * - sinf (tanval) + c4z2 * cosf (tanval);
    
    // Rotate Z
    tanval = -(float) C4WFDist * 3.14159265*2 / 128;
    c4x = c4x2 * cosf (tanval) - c4y2 * sinf (tanval);
    c4y = c4x2 * sinf (tanval) + c4y2 * cosf (tanval);
    
    // Scale
    C4WFXVal = (short) (c4x*(float)C4WFScale/(0x90*(c4z+0x95))*0x95);
    C4WFYVal = (short) (c4y*(float)C4WFScale/(0x90*(c4z+0x95))*0x95);
}

void C4TransfWireFrame2 ()
{
    c4x = (float)C4WFXVal;
    c4y = (float)C4WFYVal;
    c4z = (float)C4WFZVal;
    
    // Rotate X
    tanval = -(float) C4WFX2Val * 3.14159265 * 2 / 128;
    c4y2 = c4y * cosf (tanval) - c4z * sinf (tanval);
    c4z2 = c4y * sinf (tanval) + c4z * cosf (tanval);
    
    // Rotate Y
    tanval = -(float) C4WFY2Val * 3.14159265 * 2 / 128;
    c4x2 = c4x * cosf (tanval) + c4z2 * sinf (tanval);
    c4z = c4x * -sinf (tanval) + c4z2 * cosf (tanval);
    
    // Rotate Z
    tanval = -(float)C4WFDist * 3.14159265 * 2 / 128;
    c4x = c4x2 * cosf (tanval) - c4y2 * sinf (tanval);
    c4y = c4x2 * sinf (tanval) + c4y2 * cosf (tanval);
    
    // Scale
    C4WFXVal =(short)(c4x * (float)C4WFScale / 0x100);
    C4WFYVal =(short)(c4y * (float)C4WFScale / 0x100);
}

void C4CalcWireFrame ()
{
    C4WFXVal = C4WFX2Val - C4WFXVal;
    C4WFYVal = C4WFY2Val - C4WFYVal;
    if (abs ((float)C4WFXVal) > abs ((float)C4WFYVal))
    {
        C4WFDist = abs ((float)C4WFXVal) + 1;
        C4WFYVal = (short) (256 * (float) C4WFYVal / abs ((float)C4WFXVal));
        if (C4WFXVal < 0)
            C4WFXVal = -256;
        else 
            C4WFXVal = 256;
    }
    else
    {
        if (C4WFYVal != 0) 
        {
            C4WFDist = abs((float)C4WFYVal)+1;
            C4WFXVal = (short) (256 * (float)C4WFXVal / abs ((float)C4WFYVal));
            if (C4WFYVal < 0)
                C4WFYVal = -256;
            else 
                C4WFYVal = 256;
        }
        else 
            C4WFDist = 0;
    }
}

short C41FXVal;
short C41FYVal;
short C41FAngleRes;
short C41FDist;
short C41FDistVal;

void C4Op1F ()
{
    if (C41FXVal == 0) 
    {
        if (C41FYVal > 0) 
            C41FAngleRes = 0x80;
        else 
            C41FAngleRes = 0x180;
    }
    else 
    {
        tanval = (float) C41FYVal / C41FXVal;
        C41FAngleRes = (short) (atanf (tanval) / (3.141592675 * 2) * 512);
        C41FAngleRes = C41FAngleRes;
        if (C41FXVal< 0) 
            C41FAngleRes += 0x100;
        C41FAngleRes &= 0x1FF;
    }
}

void C4Op15()
{
    tanval = sqrtf ((float) C41FYVal * C41FYVal + (float) C41FXVal * C41FXVal);
    C41FDist = (short) tanval;
}

void C4Op0D()
{
    tanval = sqrtf ((float) C41FYVal * C41FYVal + (float) C41FXVal * C41FXVal);
    tanval = C41FDistVal / tanval;
    C41FYVal = (short) (C41FYVal * tanval * 0.99);
    C41FXVal = (short) (C41FXVal * tanval * 0.98);
}

#ifdef ZSNES_C4
void C4LoaDMem(char *C4RAM)
{
  memmove(C4RAM+(READ_WORD(C4RAM+0x1f45)&0x1fff), 
          S9xGetMemPointer(READ_3WORD(C4RAM+0x1f40)),
          READ_WORD(C4RAM+0x1f43));
}
#endif
//}//end extern C

