
#ifndef PSP_ADHOC_H
#define PSP_ADHOC_H

int sceNetAdhocInit( void );
int sceNetAdhocTerm( void );

/////////////////////////////////////////////////////////////////////////////////////////////////
// Adhoc control procedures

// Based on lumines - 0 0 0 0 55 4c 55 53 31 30 30 30 32
struct productStruct
{
	int unknown;	// 0 in lumines
	char product[9]; // "ULUS10002" in lumines
} productStruct;

// return 0 on success
int sceNetAdhocctlInit(int unk1,	// 0x2000 in lumines
				int unk2,	// 0x30 in lumines
				struct productStruct *product);

int sceNetAdhocctlConnect(int *unk1); // Zeros in lumines

int sceNetAdhocctlDisconnect(void);

// returns 0
// event is zero at the start, needs to be 1 before continuing
int sceNetAdhocctlGetState(int *event);

int sceNetAdhocctlTerm(void);

/////////////////////////////////////////////////////////////////////////////////////////////////
// Pdp procedures

// return 1 in lumines, probably an ID for Pdp procs
int sceNetAdhocPdpCreate(char *mac,	// Contains the result from sceWlanGetEtherAddr
				 int port,	// 0x309 in lumines
				 unsigned int unk2,	// 0x400 in lumines
				 int unk3);	// 0 in lumines

// returns 0
int sceNetAdhocPdpDelete(int id,
				 int unk1); // 0 in lumines

// destMacAddr can be set to {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} to set it as a broadcast
// address, I am not sure how well this will work though
int sceNetAdhocPdpSend(int pdpId,
				char *destMacAddr,
				int16 port,
				void *data,
				unsigned int dataLength,
				int unk6,	// 0 in lumines
				int unk7);	// 1 in lumines

int sceNetAdhocPdpRecv(int pdpId,
				char *srcMacAddr,
				int *port,
				void *data,
				void *dataLength,	// Pass in the value of the buffer size
				int unk6,
				int unk7);

typedef struct pdpStatStruct
{
	struct pdpStatStruct *next;	// 0 in lumines
	int pdpId;		// 1 in lumines
	char mac[6];	// mac address
	int16 port;		// 309 in lumines - same as unk1 in PdpCreate
	unsigned int rcvdData;	// I found that I needed to decrease this each time I called recv
				// I did not go back and check if that was required though
} pdpStatStruct;

// This gets data for all of the open ports, in lumines the size is
// set to 20 in order to ensure only the first entry is received
// returns 0
int sceNetAdhocGetPdpStat(int *size,
				  pdpStatStruct *stat);


/////////////////////////////////////////////////////////////////////////////////////////////////
// Matching procedures

#define MATCHING_JOINED 	0x1	// Another PSP has joined
#define MATCHING_SELECTED 	0x2	// Another PSP selected to match
#define MATCHING_REJECTED	0x4   // The request has been rejected
#define MATCHING_CANCELED	0x5   // The request has been cancelled
#define MATCHING_ESTABLISHED	0x7	// Both PSP's have agreed to connect, at this point Lumines
						// closes the connection and creates a new one with just the
						// two PSP's in it.
#define MATCHING_DISCONNECT   0xa	// A PSP has quit, this does not include when the PSP crashes

// return 0 on success
int sceNetAdhocMatchingInit(int unk1);       // 0x20000 in lumines

// Not sure what the values passed in are, I would presume one is
// the usrData that was used in sceNetAdhocMatchingStart
typedef void (*MatchingCallback)(int, int, int, int, int);

// returns 1 in lumines, probably an ID for matching procs
int sceNetAdhocMatchingCreate(int unk1,	// 3
			  int unk2,	// 0xa
			  int port,	// 0x22b
			  int unk4,	// 0x800
			  int unk5,	// 0x2dc6c0
			  int unk6,	// 0x5b8d80
			  int unk7,	// 3
			  int unk8,	// 0x7a120
			  MatchingCallback callback);

int sceNetAdhocMatchingStop(int matchingId);

int sceNetAdhocMatchingDelete(int matchingId);

int sceNetAdhocMatchingTerm(void);

// returns 0
int sceNetAdhocMatchingStart(int matchingId, 	// 1 in lumines (presuming what is returned from create)
			 int unk1,		// 0x10
			 int unk2,		// 0x2000
			 int unk3,		// 0x10
			 int unk4,		// 0x2000
			 int usrDataSize,	// 0x18 in lumines
			 char *usrData);	// Lumines uses this to pass across data to the callback proc
						// in Lumines it contains, username, mac address and some extra
						// info, ensure that this is not 0 if the dataSize is set,
						// otherwise it causes a trap

int sceNetAdhocMatchingSelectTarget(int matchingId,
						char *mac,
						int unk3,	// 0x0
						int unk4);	// 0x0

int sceNetAdhocMatchingCancelTarget(int matchingId, // Not collected traces, but presume it is this
						int unk2);	    // This will possibly be the mac

/////////////////////////////////////////////////////////////////////////////////////////////////
// Not investigated the following interfaces

int sceNetAdhocMatchingSetHelloOpt(int unk1,
						int unk2,
						int unk3);

#endif
