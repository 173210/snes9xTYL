static uint8 GetPPU_RAM (uint16 Address) {
	return (FillRAM[Address]);
}

static uint8 GetPPU_2102 (uint16 Address) {
	return (uint8)(PPU.OAMAddr);
}

static uint8 GetPPU_2103 (uint16 Address) {
	return (((PPU.OAMAddr >> 8) & 1) | (PPU.OAMPriorityRotation << 7));
}

static uint8 GetPPU_2116 (uint16 Address) {
	return (uint8)(PPU.VMA.Address);
}

static uint8 GetPPU_2117 (uint16 Address) {
	return (PPU.VMA.Address >> 8);
}

static uint8 GetPPU_2121 (uint16 Address) {
	return (PPU.CGADD);
}

static uint8 GetPPU_213x (uint16 Address) {
	// 16bit x 8bit multiply read result.
	if (PPU.Need16x8Mulitply) {
		int32 r = (int32) PPU.MatrixA * (int32) (PPU.MatrixB >> 8);

		FillRAM[0x2134] = (uint8) r;
		FillRAM[0x2135] = (uint8)(r >> 8);
		FillRAM[0x2136] = (uint8)(r >> 16);
		PPU.Need16x8Mulitply = FALSE;
	}
  return (FillRAM[Address]);
}

static uint8 GetPPU_2137 (uint16 Address) {
	// Latch h and v counters
	PPU.HVBeamCounterLatched = 1;
	PPU.VBeamPosLatched = (uint16) CPU.V_Counter;
	PPU.HBeamPosLatched = (uint16) ((CPU.Cycles * SNES_HCOUNTER_MAX) / Settings.H_Max);
	    
	// Causes screen flicker for Yoshi's Island if uncommented
	//CLEAR_IRQ_SOURCE (PPU_V_BEAM_IRQ_SOURCE | PPU_H_BEAM_IRQ_SOURCE);

	if (SNESGameFixes.NeedInit0x2137) PPU.VBeamFlip = 0;   //jyam sword world sfc2 & godzill		
	return (0);
}

static uint8 GetPPU_2138 (uint16 Address) {
	uint8 byte=0;
 	// Read OAM (sprite) control data
	if (!PPU.OAMReadFlip) {
		byte = PPU.OAMData [PPU.OAMAddr << 1];
	} else {
		byte = PPU.OAMData [(PPU.OAMAddr << 1) + 1];
		if (++PPU.OAMAddr >= 0x110) PPU.OAMAddr = 0;		
	}
	PPU.OAMReadFlip ^= 1;
  return (byte);
}

static uint8 GetPPU_2139 (uint16 Address) {
	uint8 byte=0;
 	// Read vram low byte
	if (IPPU.FirstVRAMRead) byte = VRAM[PPU.VMA.Address << 1];
	else if (PPU.VMA.FullGraphicCount) {
		uint32 addr = PPU.VMA.Address - 1;
		uint32 rem = addr & PPU.VMA.Mask1;
		uint32 address = (addr & ~PPU.VMA.Mask1) +
				 (rem >> PPU.VMA.Shift) +
				 ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3);
		byte = VRAM [((address << 1) - 2) & 0xFFFF];
	} else byte = VRAM[((PPU.VMA.Address << 1) - 2) & 0xffff];

	if (!PPU.VMA.High) {
		PPU.VMA.Address += PPU.VMA.Increment;
		IPPU.FirstVRAMRead = FALSE;
	}
	return (byte);
}

static uint8 GetPPU_213A (uint16 Address) {
	uint8 byte=0;
 	// Read vram high byte
	if (IPPU.FirstVRAMRead) byte = VRAM[((PPU.VMA.Address << 1) + 1) & 0xffff];
	else if (PPU.VMA.FullGraphicCount) {
		uint32 addr = PPU.VMA.Address - 1;
		uint32 rem = addr & PPU.VMA.Mask1;
		uint32 address = (addr & ~PPU.VMA.Mask1) +
				 (rem >> PPU.VMA.Shift) +
				 ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3);
		byte = VRAM [((address << 1) - 1) & 0xFFFF];
	} else byte = VRAM[((PPU.VMA.Address << 1) - 1) & 0xFFFF];
	if (PPU.VMA.High) {
		PPU.VMA.Address += PPU.VMA.Increment;
		IPPU.FirstVRAMRead = FALSE;
	}
	return (byte);
}

static uint8 GetPPU_213B (uint16 Address) {
	uint8 byte=0;
 	    // Read palette data
	if (PPU.CGFLIPRead) byte = PPU.CGDATA [PPU.CGADD++] >> 8;
	else byte = PPU.CGDATA [PPU.CGADD] & 0xff;

	PPU.CGFLIPRead ^= 1;
	    
	return (byte);
}
	    
static uint8 GetPPU_213C (uint16 Address) {
	uint8 byte=0;
	// Horizontal counter value 0-339

	if (PPU.HBeamFlip) byte = PPU.HBeamPosLatched >> 8;
	else byte = (uint8)PPU.HBeamPosLatched;
	PPU.HBeamFlip ^= 1;
	return (byte);
}

static uint8 GetPPU_213D (uint16 Address) {
	uint8 byte=0;
	// Vertical counter value 0-262
	if (PPU.VBeamFlip) byte = PPU.VBeamPosLatched >> 8;
	else byte = (uint8)PPU.VBeamPosLatched;
	PPU.VBeamFlip ^= 1;
	return (byte);
}

static uint8 GetPPU_213E (uint16 Address) {
	return (SNESGameFixes._0x213E_ReturnValue);
}

static uint8 GetPPU_213F (uint16 Address)
{
   // NTSC/PAL and which field flags
	    PPU.VBeamFlip = PPU.HBeamFlip = 0;
	    
	    return ((Settings.PAL ? 0x10 : 0) | (FillRAM[0x213f] & 0xc0));
}

static uint8 GetPPU_APUR (uint16 Address) {
	uint8 byte=0;

//	CPU.Flags |= DEBUG_MODE_FLAG;
#ifdef SPC700_SHUTDOWN	
	(IAPUuncached->APUExecuting) = Settings.APUEnabled;
	(IAPUuncached->WaitCounter)++;
#endif
	if (Settings.APUEnabled) {
#ifdef CPU_SHUTDOWN
//		CPU.WaitAddress = CPU.PCAtOpcodeStart;
#endif	
		if (SNESGameFixes.APU_OutPorts_ReturnValueFix && Address >= 0x2140 && Address <= 0x2143 && !CPU.V_Counter) {
			return (uint8)((Address & 1) ? ((rand() & 0xff00) >> 8) :  (rand() & 0xff));
		}
		return ((APUuncached->OutPorts) [Address & 3]);
	}

	switch (Settings.SoundSkipMethod) {
		case 0:
	  case 1:
			CPU.BranchSkip = TRUE;
			break;
	  case 2:
			break;
	  case 3:
			CPU.BranchSkip = TRUE;
			break;
	}
	if (Address & 3 < 2) {
		int r = rand ();
		if (r & 2) {		
			if (r & 4){	    
				return (Address & 3 == 1 ? 0xaa : 0xbb);
			} else {		    
				return ((r >> 3) & 0xff);
			}
		}
	} else {
		int r = rand ();
		if (r & 2) {
			return ((r >> 3) & 0xff);
		}
	}
	return (FillRAM[Address]);
}

static uint8 GetPPU_2180 (uint16 Address) {
	uint8 byte=0;
	// Read WRAM

	byte = RAM [PPU.WRAM++];
	PPU.WRAM &= 0x1FFFF;
  return (byte);
}

static uint8 GetPPU_2190 (uint16 Address) {
	return 1;
}

static uint8 GetPPU_0 (uint16 Address) {
	uint8 byte=0;
	return byte;
}

uint8 (*GetPPU[])(uint16 Address) = {
	GetPPU_RAM,  GetPPU_RAM,  GetPPU_2102,  GetPPU_2103,  GetPPU_RAM, GetPPU_RAM, GetPPU_RAM, GetPPU_RAM,  //0x2107
	GetPPU_RAM,	GetPPU_RAM,	GetPPU_RAM, GetPPU_RAM,  GetPPU_RAM,  GetPPU_RAM,  GetPPU_RAM,  GetPPU_RAM,		 //0x210F
	GetPPU_RAM,  GetPPU_RAM,  GetPPU_RAM,  GetPPU_RAM,  GetPPU_RAM, GetPPU_RAM, GetPPU_2116, GetPPU_2117,  //0x2117
	GetPPU_RAM, GetPPU_RAM, GetPPU_RAM, GetPPU_RAM,  GetPPU_RAM,  GetPPU_RAM,  GetPPU_RAM,  GetPPU_RAM,    //0x211F
	GetPPU_RAM,  GetPPU_2121,  GetPPU_RAM,  GetPPU_RAM,  GetPPU_RAM, GetPPU_RAM, GetPPU_RAM, GetPPU_RAM,	 //0x2127
	GetPPU_RAM, GetPPU_RAM, GetPPU_RAM, GetPPU_RAM,  GetPPU_RAM,  GetPPU_RAM,  GetPPU_RAM,  GetPPU_RAM,		 //0x212F
	GetPPU_RAM,  GetPPU_RAM,  GetPPU_RAM,  GetPPU_RAM,  GetPPU_213x, GetPPU_213x, GetPPU_213x, GetPPU_2137, //0x2137
	GetPPU_2138, GetPPU_2139, GetPPU_213A, GetPPU_213B, GetPPU_213C, GetPPU_213D, GetPPU_213E, GetPPU_213F, //0x213F
	GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, //0x2147
	GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, //0x214F
	GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, //0x2157
	GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, //0x215F
	GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, //0x2167
	GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, //0x216F
	GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, //0x2177
	GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, GetPPU_APUR, //0x217F
	GetPPU_2180, GetPPU_RAM,  GetPPU_RAM,  GetPPU_RAM,  GetPPU_0,  GetPPU_0,  GetPPU_0,  GetPPU_0,  //0x2187
	GetPPU_0,  GetPPU_0,  GetPPU_0,  GetPPU_0,  GetPPU_0,  GetPPU_0,  GetPPU_0,  GetPPU_0,  //0x218F
	GetPPU_2190
};

uint8 S9xGetPPU (uint16 Address) {
	uint8 byte = 0;

	if(Address<0x2100)//not a real PPU reg
		return (byte); //treat as unmapped memory returning last byte on the bus
    if (Address <= 0x2190){
		return GetPPU[Address - 0x2100](Address);
    } else
    {
	if (Settings.SA1){

	    return (S9xGetSA1 (Address));}

	if (Address <= 0x2fff || Address >= 0x3000 + 768)
	{
	    switch (Address)
	    {
	    case 0x21c2:
	    
	        return (0x20);
	    case 0x21c3:
	    
	        return (0);
	    case 0x2800:
		// For Dai Kaijyu Monogatari II
		if (Settings.SRTC){
		
		    return (S9xGetSRTC (Address));}
		/*FALL*/
			    
	    default:
#ifdef DEBUGGER
	        missing.unknownppu_read = Address;
	        if (Settings.TraceUnknownRegisters)
		{
		    sprintf (String, "Unknown register read: $%04X\n", Address);
		    S9xMessage (S9X_TRACE, S9X_PPU_TRACE, String);
		}
#endif
		// XXX:
		
	        return (0); //FillRAM[Address]);
	    }
	}
	
	if (!Settings.SuperFX){
	
	    return (0x30);}
#ifdef ZSNES_FX
	if (Address < 0x3040)
	    byte = S9xSuperFXReadReg (Address);
	else
	    byte = FillRAM [Address];

#ifdef CPU_SHUTDOWN
	if (Address == 0x3030)
	    CPU.WaitAddress = CPU.PCAtOpcodeStart;
#endif	
	if (Address == 0x3031)
	    CLEAR_IRQ_SOURCE (GSU_IRQ_SOURCE);
#else
	byte = FillRAM [Address];

//if (Address != 0x3030 && Address != 0x3031)
//printf ("%04x\n", Address);
#ifdef CPU_SHUTDOWN
	if (Address == 0x3030)
	{
	    CPU.WaitAddress = CPU.PCAtOpcodeStart;
	}
	else
#endif
	if (Address == 0x3031)
	{
	    CLEAR_IRQ_SOURCE (GSU_IRQ_SOURCE);
	    FillRAM [0x3031] = byte & 0x7f;
	}
	return (byte);
#endif
    }

    return (byte);
}
