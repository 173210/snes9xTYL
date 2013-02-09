#ifndef SPC_H
#define SPC_H

//#include "TapWave.h"

//#include <string.h>

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed char int8;
typedef signed short int16;
//typedef signed int int32;

struct SIAPU
{
	uint8  *ExtendedPage;
	uint32 Address;
	uint32 _DirectPage;
	uint8  Bit;
	uint8  _Carry;
	uint8  _Zero;
	uint8  _Overflow;
};

typedef union
{
#ifdef WORDS_BIGENDIAN
    struct { uint8 Y, A; } B;
#else
    struct { uint8 A, Y; } B;
#endif
    uint16 W;
} YAndA;

struct SAPURegisters
{
	uint8  P;
	YAndA YA;
	uint8  X;
	uint8  S;
	uint16 PC;
};

//***************************** changed from const in PocketSPC and moved to top of spc.cpp and main.cpp
//const int TS_CYC = 1024000/32000;

class spc
{
private:
	unsigned char * SPC_RAM;
	unsigned char * DSPregs;

	int DSPindex;

	int cycles;

	struct SIAPU IAPU;
	struct SAPURegisters APURegisters;

	unsigned char PORTR[4], PORTW[4];

	int T0_cycle_latch, T1_cycle_latch, T2_cycle_latch;
	uint8 T0_counter, T1_counter, T2_counter;

	uint16 T0_position, T0_target;
	uint16 T1_position, T1_target;
	uint16 T2_position, T2_target;

	uint8 ReadByte(uint16 address);
	uint16 ReadWord(uint16 address);
	void WriteByte(uint16 address, uint8 data);
	void WriteWord(uint16 address, uint16 data);

	void exec();

	void PackStatus();
	void UnpackStatus();

public:
	spc(void * pSPC_RAM, void * pDSPregs);

	void reset();
	void setstate(int pPC, int pA, int pX, int pY, int pP, int pSP);
	void run(int cyc);
	void writeport(int port, char data);
	int readport(int port);
};

#endif
