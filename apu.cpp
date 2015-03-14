/*******************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 
  (c) Copyright 1996 - 2002 Gary Henderson (gary.henderson@ntlworld.com) and
                            Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2001 - 2004 John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2004 Brad Jorsch (anomie@users.sourceforge.net),
                            funkyass (funkyass@spam.shaw.ca),
                            Joel Yliluoma (http://iki.fi/bisqwit/)
                            Kris Bleakley (codeviolation@hotmail.com),
                            Matthew Kendora,
                            Nach (n-a-c-h@users.sourceforge.net),
                            Peter Bortas (peter@bortas.org) and
                            zones (kasumitokoduck@yahoo.com)

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003 zsKnight (zsknight@zsnes.com),
                            _Demo_ (_demo_@zsnes.com), and Nach

  C4 C++ code
  (c) Copyright 2003 Brad Jorsch

  DSP-1 emulator code
  (c) Copyright 1998 - 2004 Ivar (ivar@snes9x.com), _Demo_, Gary Henderson,
                            John Weidman, neviksti (neviksti@hotmail.com),
                            Kris Bleakley, Andreas Naive

  DSP-2 emulator code
  (c) Copyright 2003 Kris Bleakley, John Weidman, neviksti, Matthew Kendora, and
                     Lord Nightmare (lord_nightmare@users.sourceforge.net

  OBC1 emulator code
  (c) Copyright 2001 - 2004 zsKnight, pagefault (pagefault@zsnes.com) and
                            Kris Bleakley
  Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002 Matthew Kendora with research by
                     zsKnight, John Weidman, and Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003 Brad Jorsch with research by
                     Andreas Naive and John Weidman
 
  S-RTC C emulator code
  (c) Copyright 2001 John Weidman
  
  ST010 C++ emulator code
  (c) Copyright 2003 Feather, Kris Bleakley, John Weidman and Matthew Kendora

  Super FX x86 assembler emulator code 
  (c) Copyright 1998 - 2003 zsKnight, _Demo_, and pagefault 

  Super FX C emulator code 
  (c) Copyright 1997 - 1999 Ivar, Gary Henderson and John Weidman


  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004 Marcus Comstedt (marcus@mc.pp.se) 

 
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

#include "psp.h"

#include "snes9x.h"
#include "spc700.h"
#include "apu.h"
#include "soundux.h"
//#include "cpuexec.h"

extern int NoiseFreq [32];

#ifdef ME_SOUND
volatile int32 * Uncache_APU_Cycles_p;
bool8 Uncache_IAPU_APUExecuting;
volatile bool8 * Uncache_IAPU_APUExecuting_p = (bool8*)UNCACHE_PTR(&Uncache_IAPU_APUExecuting);
uint8 *Uncache_APU_OutPorts_p;
//#undef IAPU
//SIAPU* Uncache_IAPU_p=(SIAPU*)(&APUPack.IAPU);
//#define IAPU (*Uncache_IAPU_p)
#endif
bool8 S9xInitAPU ()
{
#ifdef PSP		
	  uint8 *apu_ram,*apu_ramc;
	  uint8 *apu_vars;
	  
	  	  	  	  	  
	  apu_ram = (uint8 *) malloc (0x10000 );
	  apu_ramc = (uint8 *) malloc (0x10000 );
//    apu_vars=(uint8*)malloc(4*10+65536*2*4*2+65536*2*2);
            
//		IAPUuncached = (struct SIAPU *)UNCACHE_PTR(malloc(sizeof(struct SIAPU )));
//    APUuncached = (struct SAPU *)UNCACHE_PTR(malloc(sizeof(struct SAPU )));
//    APURegistersUncached = (struct SAPURegisters *)UNCACHE_PTR(malloc(sizeof(struct SAPURegisters )));
    //azz
    //IAPU = (struct SIAPU *)(malloc(sizeof(struct SIAPU )));
    //APU = (struct SAPU *)(malloc(sizeof(struct SAPU )));
    //APURegisters = (struct SAPURegisters *)(malloc(sizeof(struct SAPURegisters )));
#ifdef ME_SOUND 
	Uncache_APU_Cycles_p=(int*)UNCACHE_PTR(&(APUPack.APU.Cycles));//(int*)UNCACHE_PTR(malloc(4));
	Uncache_APU_OutPorts_p=(uint8*)UNCACHE_PTR(APUPack.APU.OutPorts);
    apu_vars=(uint8*)malloc(4*10+65536*2*4*2+65536*2*2);
    apu_cycles_left_p=(int*)UNCACHE_PTR(((int)apu_vars)+4*0);
    apu_glob_cycles_p=(int*)UNCACHE_PTR(((int)apu_vars)+4*1);
    apu_init_after_load_=(int*)UNCACHE_PTR(((int)apu_vars)+4*2);
    apu_event1_cpt1_p=(int*)UNCACHE_PTR(((int)apu_vars)+4*3);
    apu_event2_cpt1_p=(int*)UNCACHE_PTR(((int)apu_vars)+4*4);
    apu_event1_cpt2_p=(int*)UNCACHE_PTR(((int)apu_vars)+4*5);
    apu_event2_cpt2_p=(int*)UNCACHE_PTR(((int)apu_vars)+4*6);
    apu_can_execute_p=(int*)UNCACHE_PTR(((int)apu_vars)+4*7);    
    apu_ram_write_cpt1_p=(int*)UNCACHE_PTR(((int)apu_vars)+4*8);
    apu_ram_write_cpt2_p=(int*)UNCACHE_PTR(((int)apu_vars)+4*9);
    apu_event1=(int*)UNCACHE_PTR(((int)apu_vars)+4*10);        
    apu_event2=(int*)UNCACHE_PTR(((int)apu_vars)+4*10+65536*2*4);
    apu_ram_write_log=(unsigned short*)UNCACHE_PTR(((int)apu_vars)+4*10+65536*2*4*2);

#endif
    //force write		    
    sceKernelDcacheWritebackInvalidateAll();                
    
    IAPUuncached.RAM = (uint8*)UNCACHE_PTR(apu_ram);        
    IAPU.RAM = apu_ramc;
#else
    (IAPUuncached.RAM) = (uint8 *) malloc (0x10000);
    (IAPUuncached.ShadowRAM) = (uint8 *) malloc (0x10000);
    (IAPUuncached.CachedSamples) = (uint8 *) malloc (0x40000);
#endif // PSP
    
    if (!(IAPUuncached.RAM) /*|| !(IAPUuncached.ShadowRAM) || !(IAPUuncached.CachedSamples)*/){
			S9xDeinitAPU ();
			return (FALSE);
    }

		memset((IAPUuncached.RAM), 0, 0x10000);
	
    return (TRUE);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xDeinitAPU () {
	/*if ((IAPUuncached.RAM)) {
		free ((char *) NORMAL_PTR(IAPUuncached.RAM));
		(IAPUuncached.RAM) = NULL;
  }
  if ((IAPUuncached)) {
		free ((char *) NORMAL_PTR(IAPUuncached));
		(IAPUuncached) = NULL;
  }
  if ((APUuncached)) {
		free ((char *) NORMAL_PTR(APUuncached));
		(APUuncached) = NULL;
  }
  if ((APURegistersUncached)) {
		free ((char *) NORMAL_PTR(APURegistersUncached));
		(APURegistersUncached) = NULL;
  }
  */
  if ((apu_cycles_left)) {
		free ((char *) NORMAL_PTR(apu_cycles_left));
		(apu_cycles_left) = NULL;
  }

	
}

EXTERN_C uint8 APUROM [64];

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xResetAPU ()
{

    int i;

    Settings.APUEnabled = Settings.NextAPUEnabled;
    
    //taken from sneese
    APUI00a=APUI00b=APUI00c=0;
    APUI01a=APUI01b=APUI01c=0;
    APUI02a=APUI02b=APUI02c=0;
    APUI03a=APUI03b=APUI03c=0;
    
    //
    
    apu_init_after_load=0;    
    apu_can_execute=0;
        
        
	memset((IAPUuncached.RAM),0, 0x10000);
	memset((IAPUuncached.RAM)+0x20, 0xFF, 0x20);
	memset((IAPUuncached.RAM)+0x60, 0xFF, 0x20);
	memset((IAPUuncached.RAM)+0xA0, 0xFF, 0x20);
	memset((IAPUuncached.RAM)+0xE0, 0xFF, 0x20);

	for(i=1;i<256;i++)
	{
		memcpy((IAPUuncached.RAM)+(i<<8), (IAPUuncached.RAM), 0x100);
	}

//    memcpy ((IAPUuncached.ShadowRAM), (IAPUuncached.RAM), 0x10000);
	
//    ZeroMemory ((IAPUuncached.CachedSamples), 0x40000);
    ZeroMemory ((APUPack.APU.OutPorts), 4);   
    ZeroMemory (Uncache_APU_OutPorts, 4);   
    memcpy (&(IAPUuncached.RAM) [0xffc0], APUROM, sizeof (APUROM));
    memcpy ((APUuncached.ExtraRAM), APUROM, sizeof (APUROM));
    (IAPUuncached.DirectPage) = (IAPUuncached.RAM);
    (IAPUuncached.PC) = (IAPUuncached.RAM) + (IAPUuncached.RAM) [0xfffe] + ((IAPUuncached.RAM) [0xffff] << 8);
    Uncache_APU_Cycles = 0;
    apu_cycles_left=0;
    apu_glob_cycles=0;    
    apu_event1_cpt1=0;
    apu_event1_cpt2=0;
    apu_event2_cpt1=0;
    apu_event2_cpt2=0;
    apu_ram_write_cpt1=0;
    apu_ram_write_cpt2=0;
    
    (APURegistersUncached.YA).W = 0;
    (APURegistersUncached.X) = 0;
    (APURegistersUncached.S) = 0xff;
    (APURegistersUncached.P) = 0;
    S9xAPUUnpackStatusUncached ();
    (APURegistersUncached.PC) = 0;
    (IAPU_APUExecuting) = Settings.APUEnabled;
#ifdef SPC700_SHUTDOWN
    (IAPUuncached.WaitAddress1) = NULL;
    (IAPUuncached.WaitAddress2) = NULL;
    (IAPUuncached.WaitCounter) = 0;
#endif
	//(IAPUuncached.NextAPUTimerPos) = 0;
//	(IAPUuncached.APUTimerCounter) = 0;
    (APUuncached.ShowROM) = TRUE;
    (IAPUuncached.RAM) [0xf1] = 0x80;
		
    for (i = 0; i < 3; i++)
    {
		(APUuncached.TimerEnabled) [i] = FALSE;
		(APUuncached.TimerValueWritten) [i] = 0;
		(APUuncached.TimerTarget) [i] = 0;
		(APUuncached.Timer) [i] = 0;
    }
    for (int j = 0; j < 0x80; j++)
		(APUuncached.DSP) [j] = 0;
	
    (IAPUuncached.TwoCycles) = (IAPUuncached.OneCycle) * 2;
	    	
    (APUuncached.DSP) [APU_ENDX] = 0;
    (APUuncached.DSP) [APU_KOFF] = 0;
    (APUuncached.DSP) [APU_KON] = 0;
    (APUuncached.DSP) [APU_FLG] = APU_MUTE | APU_ECHO_DISABLED;
    (APUuncached.KeyedChannels) = 0;
    
//#ifdef ME_SOUND
		apu_init_after_load|=1;				
//#else		
//    for (i = 0; i < 256; i++)
//		S9xAPUCycles [i] = S9xAPUCycleLengths [i] * (IAPUuncached.OneCycle);
//						
//    S9xResetSound (TRUE);
//    S9xSetEchoEnable (0);
//#endif    
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xSetAPUDSP (uint8 byte)
{
	static uint8 KeyOn;
	static uint8 KeyOnPrev;
//WAIT4MIXING()		
  uint8 reg = (IAPU.RAM) [0xf2];	
  int i;


    switch (reg)
    {
    case APU_FLG:
		if (byte & APU_SOFT_RESET)
		{
			APUPack.APU.DSP[reg] = APU_MUTE | APU_ECHO_DISABLED | (byte & 0x1f);
			APUPack.APU.DSP[APU_ENDX] = 0;
			APUPack.APU.DSP[APU_KOFF] = 0;
			APUPack.APU.DSP[APU_KON] = 0;
			S9xSetEchoWriteEnable (FALSE);
			// Kill sound
			S9xResetSound (FALSE);
		}
		else
		{
			S9xSetEchoWriteEnable (!(byte & APU_ECHO_DISABLED));
			if (byte & APU_MUTE)
			{
				S9xSetSoundMute (TRUE);
			}
			else
				S9xSetSoundMute (FALSE);
			
			SoundData.noise_hertz = NoiseFreq [byte & 0x1f];
			for (i = 0; i < 8; i++)
			{
				if (SoundData.channels [i].type == SOUND_NOISE)
					S9xSetSoundFrequency (i, SoundData.noise_hertz);
			}
		}
		break;
    case APU_NON:
		if (byte != APUPack.APU.DSP[APU_NON])
		{
			uint8 mask = 1;
			for (int c = 0; c < 8; c++, mask <<= 1)
			{
				int type;
				if (byte & mask)
				{
					type = SOUND_NOISE;
				}
				else
				{
					type = SOUND_SAMPLE;
				}
				S9xSetSoundType (c, type);
			}
		}
		break;
    case APU_MVOL_LEFT:
		if (byte != APUPack.APU.DSP[APU_MVOL_LEFT])
		{
			S9xSetMasterVolume ((signed char) byte,
				(signed char) APUPack.APU.DSP[APU_MVOL_RIGHT]);
		}
		break;
    case APU_MVOL_RIGHT:
		if (byte != APUPack.APU.DSP[APU_MVOL_RIGHT])
		{
			S9xSetMasterVolume ((signed char) APUPack.APU.DSP[APU_MVOL_LEFT],
				(signed char) byte);
		}
		break;
    case APU_EVOL_LEFT:
		if (byte != APUPack.APU.DSP[APU_EVOL_LEFT])
		{
			S9xSetEchoVolume ((signed char) byte,
				(signed char) APUPack.APU.DSP[APU_EVOL_RIGHT]);
		}
		break;
    case APU_EVOL_RIGHT:
		if (byte != APUPack.APU.DSP[APU_EVOL_RIGHT])
		{
			S9xSetEchoVolume ((signed char) APUPack.APU.DSP[APU_EVOL_LEFT],
				(signed char) byte);
		}
		break;
    case APU_ENDX:
		byte = 0;
		break;
		
    case APU_KOFF:
		//		if (byte)
		{
			uint8 mask = 1;
			for (int c = 0; c < 8; c++, mask <<= 1)
			{
				if ((byte & mask) != 0)
				{
					if ((APUPack.APU.KeyedChannels) & mask)
					{
						{
							KeyOnPrev&=~mask;
							(APUPack.APU.KeyedChannels) &= ~mask;
							APUPack.APU.DSP[APU_KON] &= ~mask;
							//APUPack.APU.DSP[APU_KOFF] |= mask;
							S9xSetSoundKeyOff (c);
						}
					}
				}
				else if((KeyOnPrev&mask)!=0)
				{
					KeyOnPrev&=~mask;
					(APUPack.APU.KeyedChannels) |= mask;
					//APUPack.APU.DSP[APU_KON] |= mask;
					APUPack.APU.DSP[APU_KOFF] &= ~mask;
					APUPack.APU.DSP[APU_ENDX] &= ~mask;
					S9xPlaySample (c);
				}
			}
		}
		//KeyOnPrev=0;
		APUPack.APU.DSP[APU_KOFF] = byte;
		return;
    case APU_KON:
		if (byte)
		{
			uint8 mask = 1;
			for (int c = 0; c < 8; c++, mask <<= 1)
			{
				if ((byte & mask) != 0)
				{
					// Pac-In-Time requires that channels can be key-on
					// regardeless of their current state.
					if((APUPack.APU.DSP[APU_KOFF] & mask) ==0)
					{
						KeyOnPrev&=~mask;
						(APUPack.APU.KeyedChannels) |= mask;
						//APUPack.APU.DSP[APU_KON] |= mask;
						//APUPack.APU.DSP[APU_KOFF] &= ~mask;
						APUPack.APU.DSP[APU_ENDX] &= ~mask;
						S9xPlaySample (c);
					}
					else KeyOn|=mask;
				}
			}
		}
		return;
		
    case APU_VOL_LEFT + 0x00:
    case APU_VOL_LEFT + 0x10:
    case APU_VOL_LEFT + 0x20:
    case APU_VOL_LEFT + 0x30:
    case APU_VOL_LEFT + 0x40:
    case APU_VOL_LEFT + 0x50:
    case APU_VOL_LEFT + 0x60:
    case APU_VOL_LEFT + 0x70:
		// At Shin Megami Tensei suggestion 6/11/00
		//	if (byte != APUPack.APU.DSP[reg])
		{
			S9xSetSoundVolume (reg >> 4, (signed char) byte,
				(signed char) APUPack.APU.DSP[reg + 1]);
		}
		break;
    case APU_VOL_RIGHT + 0x00:
    case APU_VOL_RIGHT + 0x10:
    case APU_VOL_RIGHT + 0x20:
    case APU_VOL_RIGHT + 0x30:
    case APU_VOL_RIGHT + 0x40:
    case APU_VOL_RIGHT + 0x50:
    case APU_VOL_RIGHT + 0x60:
    case APU_VOL_RIGHT + 0x70:
		// At Shin Megami Tensei suggestion 6/11/00
		//	if (byte != APUPack.APU.DSP[reg])
		{
			S9xSetSoundVolume (reg >> 4, (signed char) APUPack.APU.DSP[reg - 1],
				(signed char) byte);
		}
		break;
		
    case APU_P_LOW + 0x00:
    case APU_P_LOW + 0x10:
    case APU_P_LOW + 0x20:
    case APU_P_LOW + 0x30:
    case APU_P_LOW + 0x40:
    case APU_P_LOW + 0x50:
    case APU_P_LOW + 0x60:
    case APU_P_LOW + 0x70:
		S9xSetSoundHertz (reg >> 4, ((byte + (APUPack.APU.DSP[reg + 1] << 8)) & FREQUENCY_MASK) * 8);
		break;
		
    case APU_P_HIGH + 0x00:
    case APU_P_HIGH + 0x10:
    case APU_P_HIGH + 0x20:
    case APU_P_HIGH + 0x30:
    case APU_P_HIGH + 0x40:
    case APU_P_HIGH + 0x50:
    case APU_P_HIGH + 0x60:
    case APU_P_HIGH + 0x70:
		S9xSetSoundHertz (reg >> 4, 
			(((byte << 8) + APUPack.APU.DSP[reg - 1]) & FREQUENCY_MASK) * 8);
		break;
		
    case APU_SRCN + 0x00:
    case APU_SRCN + 0x10:
    case APU_SRCN + 0x20:
    case APU_SRCN + 0x30:
    case APU_SRCN + 0x40:
    case APU_SRCN + 0x50:
    case APU_SRCN + 0x60:
    case APU_SRCN + 0x70:
		if (byte != APUPack.APU.DSP[reg])
		{
			S9xSetSoundSample (reg >> 4, byte);
		}
		break;
		
    case APU_ADSR1 + 0x00:
    case APU_ADSR1 + 0x10:
    case APU_ADSR1 + 0x20:
    case APU_ADSR1 + 0x30:
    case APU_ADSR1 + 0x40:
    case APU_ADSR1 + 0x50:
    case APU_ADSR1 + 0x60:
    case APU_ADSR1 + 0x70:
		if (byte != APUPack.APU.DSP[reg])
		{
			{
				S9xFixEnvelope (reg >> 4, APUPack.APU.DSP[reg + 2], byte, 
					APUPack.APU.DSP[reg + 1]);
			}
		}
		break;
		
    case APU_ADSR2 + 0x00:
    case APU_ADSR2 + 0x10:
    case APU_ADSR2 + 0x20:
    case APU_ADSR2 + 0x30:
    case APU_ADSR2 + 0x40:
    case APU_ADSR2 + 0x50:
    case APU_ADSR2 + 0x60:
    case APU_ADSR2 + 0x70:
		if (byte != APUPack.APU.DSP[reg])
		{
			{
				S9xFixEnvelope (reg >> 4, APUPack.APU.DSP[reg + 1], APUPack.APU.DSP[reg - 1],
					byte);
			}
		}
		break;
		
    case APU_GAIN + 0x00:
    case APU_GAIN + 0x10:
    case APU_GAIN + 0x20:
    case APU_GAIN + 0x30:
    case APU_GAIN + 0x40:
    case APU_GAIN + 0x50:
    case APU_GAIN + 0x60:
    case APU_GAIN + 0x70:
		if (byte != APUPack.APU.DSP[reg])
		{
			{
				S9xFixEnvelope (reg >> 4, byte, APUPack.APU.DSP[reg - 2],
					APUPack.APU.DSP[reg - 1]);
			}
		}
		break;
		
    case APU_ENVX + 0x00:
    case APU_ENVX + 0x10:
    case APU_ENVX + 0x20:
    case APU_ENVX + 0x30:
    case APU_ENVX + 0x40:
    case APU_ENVX + 0x50:
    case APU_ENVX + 0x60:
    case APU_ENVX + 0x70:
		break;
		
    case APU_OUTX + 0x00:
    case APU_OUTX + 0x10:
    case APU_OUTX + 0x20:
    case APU_OUTX + 0x30:
    case APU_OUTX + 0x40:
    case APU_OUTX + 0x50:
    case APU_OUTX + 0x60:
    case APU_OUTX + 0x70:
		break;
		
    case APU_DIR:
		break;
		
    case APU_PMON:
		if (byte != APUPack.APU.DSP[APU_PMON])
		{
			S9xSetFrequencyModulationEnable (byte);
		}
		break;
		
    case APU_EON:
		if (byte != APUPack.APU.DSP[APU_EON])
		{
			S9xSetEchoEnable (byte);
		}
		break;
		
    case APU_EFB:
		S9xSetEchoFeedback ((signed char) byte);
		break;
		
    case APU_ESA:
		break;
		
    case APU_EDL:
		S9xSetEchoDelay (byte & 0xf);
		break;
		
    case APU_C0:
    case APU_C1:
    case APU_C2:
    case APU_C3:
    case APU_C4:
    case APU_C5:
    case APU_C6:
    case APU_C7:
		S9xSetFilterCoefficient (reg >> 4, (signed char) byte);
		break;
    default:
		// XXX
		//printf ("Write %02x to unknown APU register %02x\n", byte, reg);
		break;
    }
	
	KeyOnPrev|=KeyOn;
	KeyOn=0;
	
    if (reg < 0x80)
		APUPack.APU.DSP[reg] = byte;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
// ADSR mode
		static unsigned long AttackRate [16] = {
			4100, 2600, 1500, 1000, 640, 380, 260, 160,
				96, 64, 40, 24, 16, 10, 6, 1
		};
		static unsigned long DecayRate [8] = {
			1200, 740, 440, 290, 180, 110, 74, 37
		};
		static unsigned long SustainRate [32] = {
			~0, 38000, 28000, 24000, 19000, 14000, 12000, 9400,
				7100, 5900, 4700, 3500, 2900, 2400, 1800, 1500,
				1200, 880, 740, 590, 440, 370, 290, 220,
				180, 150, 110, 92, 74, 55, 37, 18
		};
		static unsigned long IncreaseRate [32] = {
				~0, 4100, 3100, 2600, 2000, 1500, 1300, 1000,
					770, 640, 510, 380, 320, 260, 190, 160,
					130, 96, 80, 64, 48, 40, 32, 24,
					20, 16, 12, 10, 8, 6, 4, 2
			};
			static unsigned long DecreaseRateExp [32] = {
				~0, 38000, 28000, 24000, 19000, 14000, 12000, 9400,
					7100, 5900, 4700, 3500, 2900, 2400, 1800, 1500,
					1200, 880, 740, 590, 440, 370, 290, 220,
					180, 150, 110, 92, 74, 55, 37, 18
			};
void S9xFixEnvelope (int channel, uint8 gain, uint8 adsr1, uint8 adsr2)
{
	
    if (adsr1 & 0x80)
    {
		
		// XXX: can DSP be switched to ADSR mode directly from GAIN/INCREASE/
		// DECREASE mode? And if so, what stage of the sequence does it start
		// at?
		if (S9xSetSoundMode (channel, MODE_ADSR))
		{
			// Hack for ROMs that use a very short attack rate, key on a 
			// channel, then switch to decay mode. e.g. Final Fantasy II.
			
			int attack = AttackRate [adsr1 & 0xf];
			
			if (attack == 1 && (!(Settings.SoundSync)
                ))
				attack = 0;
			
			S9xSetSoundADSR (channel, attack,
				DecayRate [(adsr1 >> 4) & 7],
				SustainRate [adsr2 & 0x1f],
				(adsr2 >> 5) & 7, 8);
		}
    }
    else
    {
		// Gain mode
		if ((gain & 0x80) == 0)
		{
			if (S9xSetSoundMode (channel, MODE_GAIN))
			{
				S9xSetEnvelopeRate (channel, 0, 0, gain & 0x7f);
				S9xSetEnvelopeHeight (channel, gain & 0x7f);
			}
		}
		else
		{
			
			if (gain & 0x40)
			{
				// Increase mode
				if (S9xSetSoundMode (channel, (gain & 0x20) ?
MODE_INCREASE_BENT_LINE :
				MODE_INCREASE_LINEAR))
				{
					S9xSetEnvelopeRate (channel, IncreaseRate [gain & 0x1f],
						1, 127);
				}
			}
			else
			{
				uint32 rate = (gain & 0x20) ? DecreaseRateExp [gain & 0x1f] / 2 :
			IncreaseRate [gain & 0x1f];
			int mode = (gain & 0x20) ? MODE_DECREASE_EXPONENTIAL
				: MODE_DECREASE_LINEAR;
			
			if (S9xSetSoundMode (channel, mode))
				S9xSetEnvelopeRate (channel, rate, -1, 0);
			}
		}
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xSetAPUControl (uint8 byte)
{
//WAIT4MIXING()
	//if (byte & 0x40)
	//printf ("*** Special SPC700 timing enabled\n");
    if ((byte & 1) != 0 && !(APUPack.APU.TimerEnabled) [0])
    {
		(APUPack.APU.Timer) [0] = 0;
		(IAPU.RAM) [0xfd] = 0;
		if (((APUPack.APU.TimerTarget) [0] = (IAPU.RAM) [0xfa]) == 0)
			(APUPack.APU.TimerTarget) [0] = 0x100;
    }
    if ((byte & 2) != 0 && !(APUPack.APU.TimerEnabled) [1])
    {
		(APUPack.APU.Timer) [1] = 0;
		(IAPU.RAM) [0xfe] = 0;
		if (((APUPack.APU.TimerTarget) [1] = (IAPU.RAM) [0xfb]) == 0)
			(APUPack.APU.TimerTarget) [1] = 0x100;
    }
    if ((byte & 4) != 0 && !(APUPack.APU.TimerEnabled) [2])
    {
		(APUPack.APU.Timer) [2] = 0;
		(IAPU.RAM) [0xff] = 0;
		if (((APUPack.APU.TimerTarget) [2] = (IAPU.RAM) [0xfc]) == 0)
			(APUPack.APU.TimerTarget) [2] = 0x100;
    }
    (APUPack.APU.TimerEnabled) [0] = byte & 1;
    (APUPack.APU.TimerEnabled) [1] = (byte & 2) >> 1;
    (APUPack.APU.TimerEnabled) [2] = (byte & 4) >> 2;
	
    if (byte & 0x10)
		(IAPU.RAM) [0xF4] = (IAPU.RAM) [0xF5] = 0;
	
    if (byte & 0x20)
		(IAPU.RAM) [0xF6] = (IAPU.RAM) [0xF7] = 0;
	
    if (byte & 0x80)
    {
		if (!(APUPack.APU.ShowROM))
		{
			memcpy (&(IAPU.RAM) [0xffc0], APUROM, sizeof (APUROM));
			(APUPack.APU.ShowROM) = TRUE;
		}
    }
    else
    {
		if ((APUPack.APU.ShowROM))
		{
			(APUPack.APU.ShowROM) = FALSE;
			memcpy (&(IAPU.RAM) [0xffc0], (APUPack.APU.ExtraRAM), sizeof (APUROM));
		}
    }
    (IAPU.RAM) [0xf1] = byte;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xSetAPUTimer (uint16 Address, uint8 byte)
{
    (IAPU.RAM) [Address] = byte;
	
    switch (Address)
    {
    case 0xfa:
		if (((APUPack.APU.TimerTarget) [0] = (IAPU.RAM) [0xfa]) == 0)
			(APUPack.APU.TimerTarget) [0] = 0x100;
		(APUPack.APU.TimerValueWritten) [0] = TRUE;
		break;
    case 0xfb:
		if (((APUPack.APU.TimerTarget) [1] = (IAPU.RAM) [0xfb]) == 0)
			(APUPack.APU.TimerTarget) [1] = 0x100;
		(APUPack.APU.TimerValueWritten) [1] = TRUE;
		break;
    case 0xfc:
		if (((APUPack.APU.TimerTarget) [2] = (IAPU.RAM) [0xfc]) == 0)
			(APUPack.APU.TimerTarget) [2] = 0x100;
		(APUPack.APU.TimerValueWritten) [2] = TRUE;
		break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
uint8 S9xGetAPUDSP ()
{
//WAIT4MIXING()
    uint8 reg = (IAPU.RAM) [0xf2] & 0x7f;
    uint8 byte = APUPack.APU.DSP[reg];
	
    switch (reg)
    {
    case APU_KON:
		break;
    case APU_KOFF:
		break;
    case APU_OUTX + 0x00:
    case APU_OUTX + 0x10:
    case APU_OUTX + 0x20:
    case APU_OUTX + 0x30:
    case APU_OUTX + 0x40:
    case APU_OUTX + 0x50:
    case APU_OUTX + 0x60:
    case APU_OUTX + 0x70:
		if (SoundData.channels [reg >> 4].state == SOUND_SILENT)
			return (0);
		return ((SoundData.channels [reg >> 4].sample >> 8) |
			(SoundData.channels [reg >> 4].sample & 0xff));
		
    case APU_ENVX + 0x00:
    case APU_ENVX + 0x10:
    case APU_ENVX + 0x20:
    case APU_ENVX + 0x30:
    case APU_ENVX + 0x40:
    case APU_ENVX + 0x50:
    case APU_ENVX + 0x60:
    case APU_ENVX + 0x70:
		return ((uint8) S9xGetEnvelopeHeight (reg >> 4));
		
    case APU_ENDX:
		// To fix speech in Magical Drop 2 6/11/00
		//	APUPack.APU.DSP[APU_ENDX] = 0;
		break;
    default:
		break;
    }
    return (byte);
}

