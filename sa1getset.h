INLINE uint8 S9xSA1GetByte (uint32 address)
{
    uint8 *GetAddress = SA1Pack_SA1.Map [(address >> MEMMAP_SHIFT) & MEMMAP_MASK];
    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
	return (*(GetAddress + (address & 0xffff)));

    switch ((int) GetAddress)
    {
    case CMemory::MAP_PPU:
	return (S9xGetSA1 (address & 0xffff));
    case CMemory::MAP_LOROM_SRAM:
    case CMemory::MAP_SA1RAM:
	return (*(SRAM + (address & 0xffff)));
    case CMemory::MAP_BWRAM:
	return (*(SA1Pack_SA1.BWRAM + ((address & 0x7fff) - 0x6000)));
    case CMemory::MAP_BWRAM_BITMAP:
	address -= 0x600000;
	if (SA1Pack_SA1.VirtualBitmapFormat == 2)
	    return ((SRAM [(address >> 2) & 0xffff] >> ((address & 3) << 1)) & 3);
	else
	    return ((SRAM [(address >> 1) & 0xffff] >> ((address & 1) << 2)) & 15);
    case CMemory::MAP_BWRAM_BITMAP2:
	address = (address & 0xffff) - 0x6000;
	if (SA1Pack_SA1.VirtualBitmapFormat == 2)
	    return ((SA1Pack_SA1.BWRAM [(address >> 2) & 0xffff] >> ((address & 3) << 1)) & 3);
	else
	    return ((SA1Pack_SA1.BWRAM [(address >> 1) & 0xffff] >> ((address & 1) << 2)) & 15);

    case CMemory::MAP_DEBUG:
    default:
#ifdef DEBUGGER
//	printf ("R(B) %06x\n", address);
#endif

	return (0);
    }
}

INLINE uint16 S9xSA1GetWord (uint32 address)
{
	//return (S9xSA1GetByte (address) | (S9xSA1GetByte (address + 1) << 8));
    uint8 *GetAddress = SA1Pack_SA1.Map [(address >> MEMMAP_SHIFT) & MEMMAP_MASK];
    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
		return (*(GetAddress + (address+1 & 0xffff)))<<8 | (*(GetAddress + (address & 0xffff)));
	else
		return (S9xSA1GetByte (address) | (S9xSA1GetByte (address + 1) << 8));
/*    uint8 *GetAddress = SA1Pack_SA1.Map [(address >> MEMMAP_SHIFT) & MEMMAP_MASK];
    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
	return (*(GetAddress + (address+1 & 0xffff)))<<8 | (*(GetAddress + (address & 0xffff)));

    switch ((int) GetAddress)
    {
    case CMemory::MAP_PPU:
	return (S9xGetSA1 (address+1 & 0xffff)) <<8 | (S9xGetSA1 (address & 0xffff));
    case CMemory::MAP_LOROM_SRAM:
    case CMemory::MAP_SA1RAM:
	return (*(SRAM + (address+1 & 0xffff))) << 8 | (*(SRAM + (address & 0xffff)));
    case CMemory::MAP_BWRAM:
	return (*(SA1Pack_SA1.BWRAM + ((address+1 & 0x7fff) - 0x6000))) << 8 | (*(SA1Pack_SA1.BWRAM + ((address & 0x7fff) - 0x6000)));
    case CMemory::MAP_BWRAM_BITMAP:
	address -= 0x600000;
	if (SA1Pack_SA1.VirtualBitmapFormat == 2)
	    return ((SRAM [(address+1 >> 2) & 0xffff] >> ((address+1 & 3) << 1)) & 3) << 8 | ((SRAM [(address >> 2) & 0xffff] >> ((address & 3) << 1)) & 3);
	else
	    return ((SRAM [(address+1 >> 1) & 0xffff] >> ((address+1 & 1) << 2)) & 15) << 8 | ((SRAM [(address >> 1) & 0xffff] >> ((address & 1) << 2)) & 15);
    case CMemory::MAP_BWRAM_BITMAP2:
	address = (address & 0xffff) - 0x6000;
	if (SA1Pack_SA1.VirtualBitmapFormat == 2)
	    return ((SA1Pack_SA1.BWRAM [(address+1 >> 2) & 0xffff] >> ((address+1 & 3) << 1)) & 3)<<8 | ((SA1Pack_SA1.BWRAM [(address >> 2) & 0xffff] >> ((address & 3) << 1)) & 3);
	else
	    return ((SA1Pack_SA1.BWRAM [(address+1 >> 1) & 0xffff] >> ((address+1 & 1) << 2)) & 15)<<8 | ((SA1Pack_SA1.BWRAM [(address >> 1) & 0xffff] >> ((address & 1) << 2)) & 15);

    case CMemory::MAP_DEBUG:
    default:
#ifdef DEBUGGER
//	printf ("R(B) %06x\n", address);
#endif
	return (0);
    }
*/
}

INLINE void S9xSA1SetByte (uint8 byte, uint32 address)
{
    uint8 *Setaddress = SA1Pack_SA1.WriteMap [(address >> MEMMAP_SHIFT) & MEMMAP_MASK];

    if (Setaddress >= (uint8 *) CMemory::MAP_LAST)
    {
	*(Setaddress + (address & 0xffff)) = byte;
	return;
    }

    switch ((int) Setaddress)
    {
    case CMemory::MAP_PPU:
	S9xSetSA1 (byte, address & 0xffff);
	return;
    case CMemory::MAP_SA1RAM:
    case CMemory::MAP_LOROM_SRAM:
	*(SRAM + (address & 0xffff)) = byte;
	return;
    case CMemory::MAP_BWRAM:
	*(SA1Pack_SA1.BWRAM + ((address & 0x7fff) - 0x6000)) = byte;
	return;
    case CMemory::MAP_BWRAM_BITMAP:
	address -= 0x600000;
	if (SA1Pack_SA1.VirtualBitmapFormat == 2)
	{
	    uint8 *ptr = &SRAM [(address >> 2) & 0xffff];
	    *ptr &= ~(3 << ((address & 3) << 1));
	    *ptr |= (byte & 3) << ((address & 3) << 1);
	}
	else
	{
	    uint8 *ptr = &SRAM [(address >> 1) & 0xffff];
	    *ptr &= ~(15 << ((address & 1) << 2));
	    *ptr |= (byte & 15) << ((address & 1) << 2);
	}
	break;
    case CMemory::MAP_BWRAM_BITMAP2:
	address = (address & 0xffff) - 0x6000;
	if (SA1Pack_SA1.VirtualBitmapFormat == 2)
	{
	    uint8 *ptr = &SA1Pack_SA1.BWRAM [(address >> 2) & 0xffff];
	    *ptr &= ~(3 << ((address & 3) << 1));
	    *ptr |= (byte & 3) << ((address & 3) << 1);
	}
	else
	{
	    uint8 *ptr = &SA1Pack_SA1.BWRAM [(address >> 1) & 0xffff];
	    *ptr &= ~(15 << ((address & 1) << 2));
	    *ptr |= (byte & 15) << ((address & 1) << 2);
	}
    default:
	return;
    }
}

INLINE void S9xSA1SetWord (uint16 Word, uint32 address)
{
    //S9xSA1SetByte ((uint8) Word, address);
    //S9xSA1SetByte ((uint8) (Word >> 8), address + 1);

	uint8 *Setaddress = SA1Pack_SA1.WriteMap [(address >> MEMMAP_SHIFT) & MEMMAP_MASK];

    if (Setaddress >= (uint8 *) CMemory::MAP_LAST)
    {
	*(Setaddress + (address & 0xffff)) = Word;
	*(Setaddress + (address+1 & 0xffff)) = Word>>8;
	return;
	}
	else
	{
		S9xSA1SetByte ((uint8) Word, address);
		S9xSA1SetByte ((uint8) (Word >> 8), address + 1);
		return;
	}
/*	uint8 *Setaddress = SA1Pack_SA1.WriteMap [(address >> MEMMAP_SHIFT) & MEMMAP_MASK];

    if (Setaddress >= (uint8 *) CMemory::MAP_LAST)
    {
	*(Setaddress + (address & 0xffff)) = Word;
	*(Setaddress + (address+1 & 0xffff)) = Word>>8;
	return;
    }

    switch ((int) Setaddress)
    {
    case CMemory::MAP_PPU:
	S9xSetSA1 (Word, address & 0xffff);
	S9xSetSA1 (Word>>8, address+1 & 0xffff);
	return;
    case CMemory::MAP_SA1RAM:
    case CMemory::MAP_LOROM_SRAM:
	*(SRAM + (address & 0xffff)) = Word;
	*(SRAM + (address+1 & 0xffff)) = Word>>8;
	return;
    case CMemory::MAP_BWRAM:
	*(SA1Pack_SA1.BWRAM + ((address & 0x7fff) - 0x6000)) = Word;
	*(SA1Pack_SA1.BWRAM + ((address+1 & 0x7fff) - 0x6000)) = Word>>8;
	return;
    case CMemory::MAP_BWRAM_BITMAP:
	address -= 0x600000;
	if (SA1Pack_SA1.VirtualBitmapFormat == 2)
	{
	    uint8 *ptr = &SRAM [(address >> 2) & 0xffff];
	    *ptr &= ~(3 << ((address & 3) << 1));
	    *ptr |= (Word & 3) << ((address & 3) << 1);
	    ptr = &SRAM [(address+1 >> 2) & 0xffff];
	    *ptr &= ~(3 << ((address+1 & 3) << 1));
	    *ptr |= (Word>>8 & 3) << ((address+1 & 3) << 1);
	}
	else
	{
	    uint8 *ptr = &SRAM [(address >> 1) & 0xffff];
	    *ptr &= ~(15 << ((address & 1) << 2));
	    *ptr |= (Word & 15) << ((address & 1) << 2);
	    ptr = &SRAM [(address+1 >> 1) & 0xffff];
	    *ptr &= ~(15 << ((address+1 & 1) << 2));
	    *ptr |= (Word>>8 & 15) << ((address+1 & 1) << 2);
	}
	break;
    case CMemory::MAP_BWRAM_BITMAP2:
	address = (address & 0xffff) - 0x6000;
	if (SA1Pack_SA1.VirtualBitmapFormat == 2)
	{
	    uint8 *ptr = &SA1Pack_SA1.BWRAM [(address >> 2) & 0xffff];
	    *ptr &= ~(3 << ((address & 3) << 1));
	    *ptr |= (Word & 3) << ((address & 3) << 1);
	    ptr = &SA1Pack_SA1.BWRAM [(address+1 >> 2) & 0xffff];
	    *ptr &= ~(3 << ((address+1 & 3) << 1));
	    *ptr |= (Word>>8 & 3) << ((address+1 & 3) << 1);
	}
	else
	{
	    uint8 *ptr = &SA1Pack_SA1.BWRAM [(address >> 1) & 0xffff];
	    *ptr &= ~(15 << ((address & 1) << 2));
	    *ptr |= (Word & 15) << ((address & 1) << 2);
	    ptr = &SA1Pack_SA1.BWRAM [(address+1 >> 1) & 0xffff];
	    *ptr &= ~(15 << ((address+1 & 1) << 2));
	    *ptr |= (Word>>8 & 15) << ((address+1 & 1) << 2);
	}
    default:
	return;
    }*/
}
