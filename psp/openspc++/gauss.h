﻿#ifndef GAUSS_H
#define GAUSS_H

//#include "TapWave.h"

static const int gauss[]={
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 
	0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 
	0x001, 0x001, 0x001, 0x002, 0x002, 0x002, 0x002, 0x002, 
	0x002, 0x002, 0x003, 0x003, 0x003, 0x003, 0x003, 0x004, 
	0x004, 0x004, 0x004, 0x004, 0x005, 0x005, 0x005, 0x005, 
	0x006, 0x006, 0x006, 0x006, 0x007, 0x007, 0x007, 0x008, 
	0x008, 0x008, 0x009, 0x009, 0x009, 0x00A, 0x00A, 0x00A, 
	0x00B, 0x00B, 0x00B, 0x00C, 0x00C, 0x00D, 0x00D, 0x00E, 
	0x00E, 0x00F, 0x00F, 0x00F, 0x010, 0x010, 0x011, 0x011, 
	0x012, 0x013, 0x013, 0x014, 0x014, 0x015, 0x015, 0x016, 
	0x017, 0x017, 0x018, 0x018, 0x019, 0x01A, 0x01B, 0x01B, 
	0x01C, 0x01D, 0x01D, 0x01E, 0x01F, 0x020, 0x020, 0x021, 
	0x022, 0x023, 0x024, 0x024, 0x025, 0x026, 0x027, 0x028, 
	0x029, 0x02A, 0x02B, 0x02C, 0x02D, 0x02E, 0x02F, 0x030, 
	0x031, 0x032, 0x033, 0x034, 0x035, 0x036, 0x037, 0x038, 
	0x03A, 0x03B, 0x03C, 0x03D, 0x03E, 0x040, 0x041, 0x042, 
	0x043, 0x045, 0x046, 0x047, 0x049, 0x04A, 0x04C, 0x04D, 
	0x04E, 0x050, 0x051, 0x053, 0x054, 0x056, 0x057, 0x059, 
	0x05A, 0x05C, 0x05E, 0x05F, 0x061, 0x063, 0x064, 0x066, 
	0x068, 0x06A, 0x06B, 0x06D, 0x06F, 0x071, 0x073, 0x075, 
	0x076, 0x078, 0x07A, 0x07C, 0x07E, 0x080, 0x082, 0x084, 
	0x086, 0x089, 0x08B, 0x08D, 0x08F, 0x091, 0x093, 0x096, 
	0x098, 0x09A, 0x09C, 0x09F, 0x0A1, 0x0A3, 0x0A6, 0x0A8, 
	0x0AB, 0x0AD, 0x0AF, 0x0B2, 0x0B4, 0x0B7, 0x0BA, 0x0BC, 
	0x0BF, 0x0C1, 0x0C4, 0x0C7, 0x0C9, 0x0CC, 0x0CF, 0x0D2, 
	0x0D4, 0x0D7, 0x0DA, 0x0DD, 0x0E0, 0x0E3, 0x0E6, 0x0E9, 
	0x0EC, 0x0EF, 0x0F2, 0x0F5, 0x0F8, 0x0FB, 0x0FE, 0x101, 
	0x104, 0x107, 0x10B, 0x10E, 0x111, 0x114, 0x118, 0x11B, 
	0x11E, 0x122, 0x125, 0x129, 0x12C, 0x130, 0x133, 0x137, 
	0x13A, 0x13E, 0x141, 0x145, 0x148, 0x14C, 0x150, 0x153, 
	0x157, 0x15B, 0x15F, 0x162, 0x166, 0x16A, 0x16E, 0x172, 
	0x176, 0x17A, 0x17D, 0x181, 0x185, 0x189, 0x18D, 0x191, 
	0x195, 0x19A, 0x19E, 0x1A2, 0x1A6, 0x1AA, 0x1AE, 0x1B2, 
	0x1B7, 0x1BB, 0x1BF, 0x1C3, 0x1C8, 0x1CC, 0x1D0, 0x1D5, 
	0x1D9, 0x1DD, 0x1E2, 0x1E6, 0x1EB, 0x1EF, 0x1F3, 0x1F8, 
	0x1FC, 0x201, 0x205, 0x20A, 0x20F, 0x213, 0x218, 0x21C, 
	0x221, 0x226, 0x22A, 0x22F, 0x233, 0x238, 0x23D, 0x241, 
	0x246, 0x24B, 0x250, 0x254, 0x259, 0x25E, 0x263, 0x267, 
	0x26C, 0x271, 0x276, 0x27B, 0x280, 0x284, 0x289, 0x28E, 
	0x293, 0x298, 0x29D, 0x2A2, 0x2A6, 0x2AB, 0x2B0, 0x2B5, 
	0x2BA, 0x2BF, 0x2C4, 0x2C9, 0x2CE, 0x2D3, 0x2D8, 0x2DC, 
	0x2E1, 0x2E6, 0x2EB, 0x2F0, 0x2F5, 0x2FA, 0x2FF, 0x304, 
	0x309, 0x30E, 0x313, 0x318, 0x31D, 0x322, 0x326, 0x32B, 
	0x330, 0x335, 0x33A, 0x33F, 0x344, 0x349, 0x34E, 0x353, 
	0x357, 0x35C, 0x361, 0x366, 0x36B, 0x370, 0x374, 0x379, 
	0x37E, 0x383, 0x388, 0x38C, 0x391, 0x396, 0x39B, 0x39F, 
	0x3A4, 0x3A9, 0x3AD, 0x3B2, 0x3B7, 0x3BB, 0x3C0, 0x3C5, 
	0x3C9, 0x3CE, 0x3D2, 0x3D7, 0x3DC, 0x3E0, 0x3E5, 0x3E9, 
	0x3ED, 0x3F2, 0x3F6, 0x3FB, 0x3FF, 0x403, 0x408, 0x40C, 
	0x410, 0x415, 0x419, 0x41D, 0x421, 0x425, 0x42A, 0x42E, 
	0x432, 0x436, 0x43A, 0x43E, 0x442, 0x446, 0x44A, 0x44E, 
	0x452, 0x455, 0x459, 0x45D, 0x461, 0x465, 0x468, 0x46C, 
	0x470, 0x473, 0x477, 0x47A, 0x47E, 0x481, 0x485, 0x488, 
	0x48C, 0x48F, 0x492, 0x496, 0x499, 0x49C, 0x49F, 0x4A2, 
	0x4A6, 0x4A9, 0x4AC, 0x4AF, 0x4B2, 0x4B5, 0x4B7, 0x4BA, 
	0x4BD, 0x4C0, 0x4C3, 0x4C5, 0x4C8, 0x4CB, 0x4CD, 0x4D0, 
	0x4D2, 0x4D5, 0x4D7, 0x4D9, 0x4DC, 0x4DE, 0x4E0, 0x4E3, 
	0x4E5, 0x4E7, 0x4E9, 0x4EB, 0x4ED, 0x4EF, 0x4F1, 0x4F3, 
	0x4F5, 0x4F6, 0x4F8, 0x4FA, 0x4FB, 0x4FD, 0x4FF, 0x500, 
	0x502, 0x503, 0x504, 0x506, 0x507, 0x508, 0x50A, 0x50B, 
	0x50C, 0x50D, 0x50E, 0x50F, 0x510, 0x511, 0x511, 0x512, 
	0x513, 0x514, 0x514, 0x515, 0x516, 0x516, 0x517, 0x517, 
	0x517, 0x518, 0x518, 0x518, 0x518, 0x518, 0x519, 0x519};
#endif
