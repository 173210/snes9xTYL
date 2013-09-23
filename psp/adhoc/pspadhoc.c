////////////////////////////////////////////////////////////////////
// PSP Adhoc
//
//   Code was used from PspPet's PSP network library helper for all
//   of the patching of modules 

////////////////////////////////////////////////////////////////////
// Reverse engineering source material:
// "fair use" for making compatible software.
// All call sequences and parameter values were taken from Lumines 
// by patching the network procs to return the parameters passed in
// and value returned.
////////////////////////////////////////////////////////////////////

#include "psp.h"
#include <pspwlan.h>
#include <pspnet.h>
#include <pspnet_adhoc.h>
#include <pspnet_adhocctl.h>
#include <pspnet_adhocmatching.h>
#include <stdio.h>

#include "loadutil.h" // Must be linked with KMEM memory access

#include "selectorMenu.h"

#define printf pspDebugScreenPrintf
#define printf2(a) msgBoxLines(a,0)

extern int os9x_btn_positive_code;
extern int os9x_btn_negative_code;

////////////////////////////////////////////////////////////////////
// system entries

unsigned char g_mac[6];
int g_matchingid=0;
int g_matchEvent=0;
int g_matchOptLen=0;
char g_matchOptData[1000];
int matchChanged = 0;

////////////////////////////////////////////////////////////////////

// call from KERNEL thread
int adhocLoadDrivers(SceModuleInfo* modInfoPtr)
{
#ifdef FW3X
	sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);         // AHMAN
	sceUtilityLoadNetModule(PSP_NET_MODULE_ADHOC);          // AHMAN
#else
	// libraries load where they want, patch workaround needed
	LoadAndStartAndPatch(modInfoPtr, "flash0:/kd/ifhandle.prx"); // kernel
	LoadAndStartAndPatch(modInfoPtr, "flash0:/kd/memab.prx");
	LoadAndStartAndPatch(modInfoPtr, "flash0:/kd/pspnet_adhoc_auth.prx");
	LoadAndStartAndPatch(modInfoPtr, "flash0:/kd/pspnet.prx");
	LoadAndStartAndPatch(modInfoPtr, "flash0:/kd/pspnet_adhoc.prx");
	LoadAndStartAndPatch(modInfoPtr, "flash0:/kd/pspnet_adhocctl.prx");
	LoadAndStartAndPatch(modInfoPtr, "flash0:/kd/pspnet_adhoc_matching.prx");
	FlushCaches();
#endif
	
	return 0;
}

char temp[100];
char debug[1000];

void matchingCallback(int matchingid, int event, unsigned char *mac, int optlen, void *optdata)
{
	// This is called when there is an event, dont seem to be able to call
	// procs from here otherwise it causes a crash

	if(event==PSP_ADHOC_MATCHING_EVENT_HELLO)
	{
		AddPsp(mac, (char *)optdata, optlen);
	}
	else if(event==PSP_ADHOC_MATCHING_EVENT_DISCONNECT)
	{
		DelPsp(mac);
	}
	else
	{
		g_matchingid = matchingid;
		g_matchEvent = event;
		g_matchOptLen = optlen;

		// Copy across the optional data
		strncpy(g_matchOptData, optdata, optlen);

		// And the MAC address
		memcpy(g_mac, mac, sizeof(char) * 6);

		matchChanged = 1;
	}
}

int pdpId = 0;
int matchingId = 0;

SceCtrlData pspPad;
bool g_NetInit = false;
bool g_NetAdhocInit = false;
bool g_NetAdhocctlInit = false;
bool g_NetAdhocctlConnect = false;
bool g_NetAdhocPdpCreate = false;
bool g_NetAdhocMatchingInit = false;
bool g_NetAdhocMatchingCreate = false;
bool g_NetAdhocMatchingStart = false;

int adhocInit(char *MatchingData)
{
	unsigned char mac[6];
	struct productStruct product;

	ClearPspList();

	strcpy(product.product, "ULUS99999");
	product.unknown = 0;

    u32 err;
	printf2("sceNetInit()\n");
    err = sceNetInit(0x20000, 0x20, 0x1000, 0x20, 0x1000);
    if (err != 0)
        return err;
	g_NetInit = true;

	printf2("sceNetAdhocInit()\n");
    err = sceNetAdhocInit();
    if (err != 0)
        return err;
	g_NetAdhocInit = true;
	
	printf2("sceNetAdhocctlInit()\n");
    err = sceNetAdhocctlInit(0x2000, 0x20, &product);
    if (err != 0)
        return err;
	g_NetAdhocctlInit = true;

    // Connect
    err = sceNetAdhocctlConnect(NULL);
    if (err != 0)
        return err;
	g_NetAdhocctlConnect = true;

    int stateLast = -1;
    printf2("Connecting...\n");
    while (1)
    {
        int state;
        err = sceNetAdhocctlGetState(&state);
        if (err != 0)
        {
        		pspDebugScreenInit();
            printf("sceNetApctlGetState returns $%x\n", err);
            sceKernelDelayThread(10*1000000); // 10sec to read before exit
			return -1;
        }
        if (state > stateLast)
        {
        		sprintf(temp,"  connection state %d of 1\n", state);
            printf2(temp);
            stateLast = state;
        }
        if (state == 1)
            break;  // connected

        // wait a little before polling again
        sceKernelDelayThread(50*1000); // 50ms
    }
    psp_msg(ADHOC_CONNECTING, MSG_DEFAULT);


	sceWlanGetEtherAddr(mac);


    printf2("sceNetAdhocPdpCreate\n");


	pdpId = sceNetAdhocPdpCreate(mac,
		     0x309,		// 0x309 in lumines
		     0x400, 	// 0x400 in lumines
		     0);		// 0 in lumines
	if(pdpId <= 0)
	{

		pspDebugScreenInit();
		printf("pdpId = %x\n", pdpId);
		return -1;
	}
	g_NetAdhocPdpCreate = true;

	printf2("sceNetAdhocMatchingInit()\n");
	
	err = sceNetAdhocMatchingInit(0x20000);
	if(err != 0)
	{
		pspDebugScreenInit();
		printf("error = %x\n", err);
	}
	g_NetAdhocMatchingInit = true;
	
	printf2("sceNetAdhocMatchingCreate()\n");
	matchingId = sceNetAdhocMatchingCreate( 3,
						0xa,
						0x22b,
						0x800,
						0x2dc6c0,
						0x5b8d80,
						3,
						0x7a120,
						(pspAdhocMatchingCallback)matchingCallback);

	if(matchingId < 0)
	{
		sprintf(temp,"matchingId = %x\n", matchingId);
		printf2(temp);
	}
	g_NetAdhocMatchingCreate = true;

	/*char tempStr[100];
	tempStr[0] = '\0';	
	if(strlen(MatchingData))
	{
		strncpy(tempStr, strrchr(MatchingData, '/')+1, 100);
		strrchr(tempStr, '.')[0] = '\0';
	}*
	printf("tempStr=%s\n", tempStr);*/

	printf2("sceNetAdhocMatchingStart()\n");
	
	err = sceNetAdhocMatchingStart(matchingId, 	// 1 in lumines (presuming what is returned from create)
			 0x10,		// 0x10
			 0x2000,		// 0x2000
			 0x10,		// 0x10
			 0x2000,		// 0x2000
			 strlen(MatchingData)+1,
			 MatchingData);		
	if(err != 0)
	{
		pspDebugScreenInit();
		printf("error = %x\n", err);
	}
	
	g_NetAdhocMatchingStart = true;

	// All the init functions have passed
	return 0;
}

int g_Server = 0;


// Define a few states for handling the adhoc events
#define PSP_LISTING		1
#define PSP_SELECTED	2
#define PSP_SELECTING	3
#define PSP_WAIT_EST	4
#define PSP_ESTABLISHED	5

int adhocReconnect(char *ssid)
{
	int err = 0;

	// Disconnect Wifi
	if(g_NetAdhocctlConnect)
	{		
		printf2("sceNetAdhocctlDisconnect\n");
		err = sceNetAdhocctlDisconnect();
		if(err != 0)
		{
			pspDebugScreenInit();
			printf(" returned %x\n", err);
		}
		g_NetAdhocctlConnect = false;
	}

	if(g_NetAdhocPdpCreate)
	{
		printf2("sceNetAdhocPdpDelete\n");
		err = sceNetAdhocPdpDelete(pdpId,0);
		if(err != 0)
		{
			pspDebugScreenInit();
			printf("sceNetAdhocPdpDelete returned %x\n", err);
		}
		g_NetAdhocPdpCreate = false;
	}

	if(g_NetAdhocMatchingStart)
	{
		printf2("sceNetAdhocMatchingStop\n");
		err = sceNetAdhocMatchingStop(matchingId);
		if(err != 0)
		{
			pspDebugScreenInit();
			printf("sceNetAdhocMatchingStop returned %x\n", err);
		}
		g_NetAdhocMatchingStart = false;
	}

	if(g_NetAdhocMatchingCreate)
	{
		printf2("sceNetAdhocMatchingDelete\n");
		err = sceNetAdhocMatchingDelete(matchingId);
		if(err != 0)
		{
			pspDebugScreenInit();
			printf("sceNetAdhocMatchingDelete returned %x\n", err);
		}
		g_NetAdhocMatchingCreate = false;
	}
	
	if(g_NetAdhocMatchingInit)
	{
		printf2("sceNetAdhocMatchingTerm\n");
		err = sceNetAdhocMatchingTerm();
		if(err != 0)
		{
			pspDebugScreenInit();
			printf("sceNetAdhocMatchingTerm returned %x\n", err);
		}
		g_NetAdhocMatchingInit = false;
	}

    int stateLast = -1;
    while (1)
    {
        int state;
        err = sceNetAdhocctlGetState(&state);
        if (err != 0)
        {
        	pspDebugScreenInit();
            printf("sceNetApctlGetState returns $%x\n", err);
            sceKernelDelayThread(10*1000000); // 10sec to read before exit
			return -1;
        }
        if (state > stateLast)
        {
        	sprintf(temp,psp_msg_string(ADHOC_STATE), state);
            printf2(temp);
            stateLast = state;
        }
        if (state == 0)
            break;  // connected

        // wait a little before polling again
        sceKernelDelayThread(50*1000); // 50ms
    }

	// Now connect to the PSP using a new SSID
    // Connect
    err = sceNetAdhocctlConnect(ssid);
    if (err != 0)
	{
		pspDebugScreenInit();
		printf("err=%x\n", err);
		for(;;)
			sceDisplayWaitVblankStart();
        return err;
	}
	g_NetAdhocctlConnect = true;
	
    stateLast = -1;
    psp_msg(ADHOC_CONNECTING, MSG_DEFAULT);
    while (1)
    {
        int state;
        err = sceNetAdhocctlGetState(&state);
        if (err != 0)
        {
        	pspDebugScreenInit();
            printf("sceNetApctlGetState returns $%x\n", err);
            sceKernelDelayThread(10*1000000); // 10sec to read before exit
			return -1;
        }
        if (state > stateLast)
        {
        	sprintf(temp,psp_msg_string(ADHOC_STATE), state);
            printf2(temp);
            stateLast = state;
        }
        if (state == 1)
            break;  // connected

        // wait a little before polling again
        sceKernelDelayThread(50*1000); // 50ms
    }

	unsigned char macAddr[6];
	sceWlanGetEtherAddr(macAddr);

	printf2("sceNetAdhocPdpCreate\n");
	pdpId = sceNetAdhocPdpCreate(macAddr,
		     0x309,		// 0x309 in lumines
		     0x800, 	// 0x400 in lumines
		     0);		// 0 in lumines
	if(pdpId <= 0)
	{
		pspDebugScreenInit();
		printf("pdpId = %x\n", pdpId);
		return -1;
	}
	g_NetAdhocPdpCreate = true;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
// This is just a quick and easy procedure to wait for a PSP to join, the following
// is what is done in Lumines:
//
//   - Wait for a PSP to join
//   - Add the PSP to the list of machines available
//   - Wait for the user to select a PSP
//   - Call sceMatchingSelectTarget
//   - The other PSP will receive MATCHING_SELECTED event
//   - The other PSP displays a message to accept or cancel
//   - If accepted the other PSP calls sceMatchingSelectTarget
//   - The first PSP receives PSP_ADHOC_MATCHING_EVENT_COMPLETE
// 
// In lumines they then close the connection and start a new adhoc connection
// with just those two machines in it.
///////////////////////////////////////////////////////////////////////////////////
int adhocSelect(void)
{
	SceCtrlData m_PspPad;
	unsigned char mac[6];
	char name[256];
	int currentState = PSP_LISTING;
	int oldButtons = 0;
	char tempStr[100];
	char str[256];
	
	sceDisplaySetMode( 0, SCREEN_WIDTH, SCREEN_HEIGHT );
	sceDisplaySetFrameBuf( (char*)VRAM_ADDR, 512, 1, 1 );
	pgFillAllvram(0);pgScreenFrame(2,0);

	for(;;)
	{
		sceCtrlReadBufferPositive(&m_PspPad, 1);
		//pspDebugScreenSetTextColor(0xFFFF);

		switch(currentState)
		{
			case PSP_LISTING:
			{
				//pspDebugScreenInit();
				//pspDebugScreenPrintf("Select a server to connect to, or triangle to return\n\n\n");
				pgFillAllvram(0);pgScreenFrame(2,0);
				mh_print(0,0,psp_msg_string(ADHOC_SELECTORRETURN),0xFFFF);
				
				DisplayPspList();
				
				pgScreenFlipV();

				g_Server = 0;

				if(m_PspPad.Buttons != oldButtons)
				{
					if(m_PspPad.Buttons & PSP_CTRL_UP)
					{
						UpList();
					}

					if(m_PspPad.Buttons & PSP_CTRL_DOWN)
					{
						DownList();
					}

					if(m_PspPad.Buttons & os9x_btn_positive_code)
					{
						if(GetPspEntry(mac, name) > 0)
						{
							currentState = PSP_SELECTING;
							sceNetAdhocMatchingSelectTarget(matchingId, mac, 0, 0);
						}
					}

					if(m_PspPad.Buttons & os9x_btn_negative_code)
						return -1;
				}		
				if(matchChanged)
				{
					if(g_matchEvent ==  PSP_ADHOC_MATCHING_EVENT_JOIN)
					{
						memcpy(mac, g_mac, 6);
						strcpy(name, g_matchOptData);
						currentState = PSP_SELECTED;
					}
				}
				break;
			}
			case PSP_SELECTING:
			{
				//pspDebugScreenInit();
				pgFillAllvram(0);pgScreenFrame(2,0);
				
				sceNetEtherNtostr(mac, tempStr);
				//printf("Waiting for %s to accept the connection\nTo cancel press O\n", tempStr);
				sprintf(str,psp_msg_string(ADHOC_WAITING), tempStr);
				mh_print(0,0,str,0xFFFF);
				pgScreenFlipV();

				if(m_PspPad.Buttons != oldButtons)
				{
					if(m_PspPad.Buttons & os9x_btn_negative_code)
					{
						sceNetAdhocMatchingCancelTarget(matchingId, mac);
						currentState = PSP_LISTING;
					}
				}

				if(matchChanged)
				{
					if(g_matchEvent == PSP_ADHOC_MATCHING_EVENT_JOIN)
					{
						sceNetAdhocMatchingCancelTarget(matchingId, mac);
					}
					else if(g_matchEvent == PSP_ADHOC_MATCHING_EVENT_COMPLETE)
					{
						currentState = PSP_ESTABLISHED;
					}
					else if(g_matchEvent == PSP_ADHOC_MATCHING_EVENT_REJECT)
					{
						currentState = PSP_LISTING;
					}
				}
				break;
			}
			case PSP_SELECTED:
			{
				g_Server = 1;

				//pspDebugScreenInit();
				pgFillAllvram(0);pgScreenFrame(2,0);
				
				sceNetEtherNtostr(mac, tempStr);
				sprintf(str,psp_msg_string(ADHOC_REQUESTED), tempStr);
				mh_print(0,0,str,0xFFFF);
				pgScreenFlipV();

				if(m_PspPad.Buttons != oldButtons)
				{
					if(m_PspPad.Buttons & os9x_btn_negative_code)
					{
						sceNetAdhocMatchingCancelTarget(matchingId, mac);
						currentState = PSP_LISTING;
					}
					if(m_PspPad.Buttons & os9x_btn_positive_code)
					{
						sceNetAdhocMatchingSelectTarget(matchingId, mac, 0, 0);
						currentState = PSP_WAIT_EST;
					}
				}

				if(matchChanged)
				{
					if(g_matchEvent == PSP_ADHOC_MATCHING_EVENT_CANCEL)
					{
						currentState = PSP_LISTING;
					}
				}
				break;
			}
			case PSP_WAIT_EST:
			{
				if(matchChanged)
				{
					if(g_matchEvent == PSP_ADHOC_MATCHING_EVENT_COMPLETE)
					{
						currentState = PSP_ESTABLISHED;
					}
				}
				break;
			}
		}

		matchChanged = 0;
		oldButtons = m_PspPad.Buttons;

		if(currentState == PSP_ESTABLISHED)
			break;

		sceDisplayWaitVblankStart();
	}

	unsigned char macAddr[6];
	unsigned char *tempMac;
	if(g_Server)
	{
		sceWlanGetEtherAddr(macAddr);
		tempMac = macAddr;
	}
	else
	{
		tempMac = mac;
	}

	sceNetEtherNtostr(tempMac, tempStr);

	char ssid[10];
	sprintf(ssid, "%c%c%c%c%c%c", tempStr[9], tempStr[10], tempStr[12], tempStr[13], 
			tempStr[15], tempStr[16]);
	adhocReconnect(ssid);

	// We only get here if both PSP's have agreed to connect
	//pspDebugScreenInit();
	//pspDebugScreenPrintf("Connected\n");
	pgFillAllvram(0);pgScreenFrame(2,0);
	mh_print(0,0,psp_msg_string(ADHOC_CONNECTED),0xFFFF);
	pgScreenFlipV();

	return 0;
}

int adhocSend(void *buffer, unsigned int length)
{
	int err=0;

	err = sceNetAdhocPdpSend(pdpId,
			&g_mac[0],
			0x309,
			buffer,
			length,
			0,	// 0 in lumines
			1);	// 1 in lumines

	return err;
}

int adhocRecv(void *buffer, unsigned int *length)
{
	int err=0;
	int pdpStatLength=20;
	unsigned char mac[6];
	pdpStatStruct pspStat;

	err = sceNetAdhocGetPdpStat(&pdpStatLength, &pspStat);
	if(err<0)
	{
		pspDebugScreenInit();
		printf("error calling GetPdpStat, err=%x\n", err);
		return err;
	}

	if (pspStat.rcvdData > 0)
	{
		err = sceNetAdhocPdpRecv(pdpId,
					mac,
					NULL,
					buffer,
					(void *)&length,
					0,	// 0 in lumines
					1);	// 1 in lumines
		if(err<0)
		{
			pspDebugScreenInit();
			printf("err=%x\n", err);
			return err;
		}
		else	
		{
			return 1;
		}
	}

	return 0;
}

int adhocRecvBlocked(void *buffer, unsigned int *length,int max_retry)
{
	int err=0;
	int retry_cpt=0;
		
	do {
		err = adhocRecv(buffer, length);
		if (err>0) break;
		sceKernelDelayThread(RECV_DELAY);
		if (max_retry>=0) {		
			retry_cpt++; if (retry_cpt>max_retry) return 0;
		} else {
			if (get_pad()&PSP_CTRL_TRIANGLE) return -1;
		}
	} while (err == 0);
	

	return err;
}

// Used to send the data and wait for an ack
int adhocSendRecvAck(void *buffer, int length)
{
	int err=0;
	int recvTemp=0;
	unsigned int recvLen=1;
	int tempLen = length;
	int sentCount = 0;
	do
	{
		if(tempLen > 0x400)
		{
			tempLen = 0x400;
		}

		err = adhocSend(buffer, tempLen);

		if(err < 0)
			return err;

		// wait for an ack
		err = adhocRecvBlocked(&recvTemp, &recvLen,-1);
		if(err < 0)
		{
			pspDebugScreenInit();
			printf("err=%x\n", err);
			return err;
		}

		buffer += 0x400;
		sentCount += 0x400;
		tempLen = length - sentCount;
	} while(sentCount < length);

	return err;
}

// Used to receive data and then send an ack
int adhocRecvSendAck(void *buffer, unsigned int *length)
{
	int temp=1;
	int err=0;
	int tempLen = *length;
	int rcvdCount = 0;

	do
	{
		if(tempLen > 0x400)
		{
			tempLen = 0x400;
		}

		err = adhocRecvBlocked(buffer, length,-1);
		if(err < 0)
			return err;

		err = adhocSend(&temp, 1);

		rcvdCount += 0x400;
		buffer += 0x400;
		tempLen = *length - rcvdCount;
	} while(rcvdCount < *length);

	// wait for an ack

	return err;
}

// The order that procedures are called was taken by patching
// Lumines network procs
int adhocTerm()
{
    u32 err;

	if(g_NetAdhocctlConnect)
	{
		printf2("sceNetAdhocctlDisconnect\n");
		err = sceNetAdhocctlDisconnect();
		if(err != 0)
		{
			pspDebugScreenInit();
			printf(" returned %x\n", err);
		}
		g_NetAdhocctlConnect = false;
	}

	if(g_NetAdhocPdpCreate)
	{
		printf2("sceNetAdhocPdpDelete\n");
		err = sceNetAdhocPdpDelete(pdpId,0);
		if(err != 0)
		{
			pspDebugScreenInit();
			printf("sceNetAdhocPdpDelete returned %x\n", err);
		}
		g_NetAdhocPdpCreate = false;
	}

	if(g_NetAdhocMatchingStart)
	{
		printf2("sceNetAdhocMatchingStop\n");
		err = sceNetAdhocMatchingStop(matchingId);
		if(err != 0)
		{
			pspDebugScreenInit();
			printf("sceNetAdhocMatchingStop returned %x\n", err);
		}
		g_NetAdhocMatchingStart = false;
	}

	if(g_NetAdhocMatchingCreate)
	{
		printf2("sceNetAdhocMatchingDelete\n");
		err = sceNetAdhocMatchingDelete(matchingId);
		if(err != 0)
		{
			pspDebugScreenInit();
			printf("sceNetAdhocMatchingDelete returned %x\n", err);
		}
		g_NetAdhocMatchingCreate = false;
	}
	
	if(g_NetAdhocMatchingInit)
	{
		printf2("sceNetAdhocMatchingTerm\n");
		err = sceNetAdhocMatchingTerm();
		if(err != 0)
		{
			pspDebugScreenInit();
			printf("sceNetAdhocMatchingTerm returned %x\n", err);
		}
		g_NetAdhocMatchingInit = false;
	}
	
	if(g_NetAdhocctlInit)
	{
		printf2("sceNetAdhocctlTerm\n");
		err = sceNetAdhocctlTerm();
		if(err != 0)
		{
			pspDebugScreenInit();
			printf("sceNetAdhocctlTerm returned %x\n", err);
		}
		g_NetAdhocctlInit = false;
	}

	if(g_NetAdhocInit)
	{
		printf2("sceNetAdhocTerm\n");
		err = sceNetAdhocTerm();
		if(err != 0)
		{
			pspDebugScreenInit();
			printf("sceNetAdhocTerm returned %x\n", err);
		}
		g_NetAdhocInit = false;
	}

	if(g_NetInit)
	{
		printf2("sceNetTerm\n");
		err = sceNetTerm();
		if(err != 0)
		{
			pspDebugScreenInit();
			printf("sceNetTerm returned %x\n", err);
		}
		g_NetInit = false;
	}

    return 0; // assume it worked
}

////////////////////////////////////////////////////////////////////
