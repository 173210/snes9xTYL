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

#ifdef NETPLAY_SUPPORT

/*#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <sys/types.h>

#ifndef __WIN32__
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#if defined (__WIN32__)
#include <winsock.h>
#include <process.h>

#define ioctl ioctlsocket
#define close closesocket
#define read(a,b,c) recv(a, b, c, 0)
#define write(a,b,c) send(a, b, c, 0)
#else

#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef __SVR4
#include <sys/stropts.h>
#endif
#endif

#ifdef USE_THREADS
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#endif
*/
#include "snes9x.h"
#include "cpuexec.h"
#include "netplay.h"
#include "memmap.h"
#include "snapshot.h"
#include "display.h"

extern "C" {
#include "psp/nlh.h"
int msgBoxLines(const char *msg,int delay_vblank=10);
}

void S9xNPClientLoop (void *);
bool8 S9xNPLoadROM (uint32 len);
bool8 S9xNPLoadROMDialog (const char *);
bool8 S9xNPGetROMImage (uint32 len);
void S9xNPGetSRAMData (uint32 len);
void S9xNPGetFreezeFile (uint32 len);

unsigned long START = 0;

bool8 S9xNPConnectToServer (const char *hostname, int port,
                            const char *rom_name)
{
    if (!S9xNPInitialise ())
        return (FALSE);

    S9xNPDisconnect ();

    NetPlay.MySequenceNum = 0;
    NetPlay.ServerSequenceNum = 0;
    NetPlay.Connected = FALSE;
    NetPlay.Abort = FALSE;
    NetPlay.Player = 0;
    NetPlay.Paused = FALSE;
    NetPlay.PercentageComplete = 0;
    NetPlay.Socket = 0;
    if (NetPlay.ServerHostName)
        free ((char *) NetPlay.ServerHostName);
    NetPlay.ServerHostName = strdup (hostname);
    if (NetPlay.ROMName)
        free ((char *) NetPlay.ROMName);
    NetPlay.ROMName = strdup (rom_name);
    NetPlay.Port = port;
    NetPlay.PendingWait4Sync = FALSE;


    return (TRUE);
}

bool8 S9xNPConnect ()
{
    struct sockaddr_in address;
    struct hostent *hostinfo;
    unsigned int addr;
    int err;
    
    address.sin_family = AF_INET;
    address.sin_port = htons (NetPlay.Port);
    S9xNPSetAction ("Looking up server's hostname...");
    //if ((int) (addr = inet_addr (NetPlay.ServerHostName)) == -1)
    if ((int) (addr = sceNetInetInetAddr (NetPlay.ServerHostName)) == -1)
    {
	/*if ((hostinfo = gethostbyname (NetPlay.ServerHostName)))
	{
	    memcpy ((char *)&address.sin_addr, hostinfo->h_addr,
		    hostinfo->h_length);
	}
	else*/
	{
            S9xNPSetError ("\
Unable to look up server's IP address from hostname.\n\n\
Unknown hostname or may be your nameserver isn't set\n\
up correctly?");
	    return (FALSE);
	}
    }
    else
    {
	memcpy ((char *)&address.sin_addr, &addr, sizeof (addr));
    }

    S9xNPSetAction ("Creating network socket...");
    //if ((NetPlay.Socket = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    if ((NetPlay.Socket = sceNetInetSocket (AF_INET, SOCK_STREAM, 0)) < 0)
    
    {
        S9xNPSetError ("Creating network socket failed.");
	return (FALSE);
    }

    S9xNPSetAction ("Trying to connect to Snes9X server...");

    //if (connect (NetPlay.Socket, (struct sockaddr *) &address, sizeof (address)) < 0)
    if (err=sceNetInetConnect(NetPlay.Socket, (struct sockaddr *) &address, sizeof (address)) < 0)
    {
        char buf [100];
        //sceNetInetGetErrno();
				/*if (errno == ECONNREFUSED)
        {
            S9xNPSetError ("\
Connection to remote server socket refused:\n\n\
Is there actually a Snes9X NetPlay server running\n\
on the remote machine on this port?");
        }
        else*/
        {
            sprintf (buf, "Connection to server failed with error number %d",err);
            S9xNPDisconnect ();
        }
	return (FALSE);
    }
    NetPlay.Connected = TRUE;

    S9xNPSetAction ("Sending 'HELLO' message...");
    /* Send the server a HELLO packet*/
    int len = 7 + 4 + strlen (NetPlay.ROMName) + 1;
    uint8 *tmp = (uint8*)malloc(len);
    uint8 *ptr = tmp;

    *ptr++ = NP_CLNT_MAGIC;
    *ptr++ = NetPlay.MySequenceNum++;
    *ptr++ = NP_CLNT_HELLO;
    WRITE_LONG (ptr, len);
    ptr += 4;
    WRITE_LONG (ptr, Settings.FrameTime);
    ptr += 4;
    strcpy ((char *) ptr, NetPlay.ROMName);

    if (!S9xNPSendData (NetPlay.Socket, tmp, len))
    {
        S9xNPSetError ("Sending 'HELLO' message failed.");
	S9xNPDisconnect ();
	//delete tmp;
	free(tmp);
	return (FALSE);
    }
    //delete tmp;
    free(tmp);

    S9xNPSetAction ("Waiting for 'HELLO' reply from server...");

    uint8 header [7];

    if (!S9xNPGetData (NetPlay.Socket, header, 7) || 
        header [0] != NP_SERV_MAGIC || header [1] != 0 || 
        (header [2] & 0x1f) != NP_SERV_HELLO)
    {
        S9xNPSetError ("Error in 'HELLO' reply packet received from server.");
	S9xNPDisconnect ();
	return (FALSE);
    }
    len = READ_LONG (&header [3]);
    if (len > 256)
    {
        S9xNPSetError ("Error in 'HELLO' reply packet received from server.");
	S9xNPDisconnect ();
	return (FALSE);
    }
    uint8 *data = (uint8*)malloc(len);//new uint8 [len];
    if (!S9xNPGetData (NetPlay.Socket, data, len - 7))
    {
        S9xNPSetError ("Error in 'HELLO' reply packet received from server.");
        //delete data;
        free(data);
	S9xNPDisconnect ();
	return (FALSE);
    }

    if (data [0] != NP_VERSION)
    {
        S9xNPSetError ("\
The Snes9X NetPlay server implements a different\n\
version of the protocol. Disconnecting.");
        //delete data;
        free(data);
	S9xNPDisconnect ();
        return (FALSE);
    }

    NetPlay.FrameCount = READ_LONG (&data [2]);

    if (!(header [2] & 0x80) &&
        strcmp ((char *) data + 4 + 2, NetPlay.ROMName) != 0)
    {
        if (!S9xNPLoadROMDialog ((char *) data + 4 + 2))
        {
            //delete data;
            free(data);
            S9xNPDisconnect ();
            return (FALSE);
        }
    }
    NetPlay.Player = data [1];
    //delete data;
    free(data);

    NetPlay.PendingWait4Sync = TRUE;
    Settings.NetPlay = TRUE;
    S9xNPResetJoypadReadPos ();
    NetPlay.ServerSequenceNum = 1;
    
    S9xNPSetAction ("Sending 'READY' to the server...");

    return (S9xNPSendReady ((header [2] & 0x80) ? 
                            NP_CLNT_WAITING_FOR_ROM_IMAGE : 
                            NP_CLNT_READY));
}

bool8 S9xNPSendReady (uint8 op)
{
    uint8 ready [7];
    uint8 *ptr = ready;
    *ptr++ = NP_CLNT_MAGIC;
    *ptr++ = NetPlay.MySequenceNum++;
    *ptr++ = op;
    WRITE_LONG (ptr, 7);
    ptr += 4;

    if (!S9xNPSendData (NetPlay.Socket, ready, 7))
    {
	S9xNPDisconnect ();
        S9xNPSetError ("Sending 'READY' message failed.");
	return (FALSE);
    }

    return (TRUE);
}

bool8 S9xNPSendPause (bool8 paused)
{
    uint8 pause [7];
    uint8 *ptr = pause;
    *ptr++ = NP_CLNT_MAGIC;
    *ptr++ = NetPlay.MySequenceNum++;
    *ptr++ = NP_CLNT_PAUSE | (paused ? 0x80 : 0);
    WRITE_LONG (ptr, 7);
    ptr += 4;

    if (!S9xNPSendData (NetPlay.Socket, pause, 7))
    {
        S9xNPSetError ("Sending 'PAUSE' message failed.");
	S9xNPDisconnect ();
	return (FALSE);
    }

    return (TRUE);
}


/*void S9xNPClientLoop (void *)
{
    NetPlay.Waiting4EmulationThread = FALSE;

    if (S9xNPConnect ())
    {
        S9xClearPause (PAUSE_NETPLAY_CONNECT);
        while (NetPlay.Connected)
        {
            if (S9xNPWaitForHeartBeat ())
            {
                LONG prev;
                if (!ReleaseSemaphore (GUI.ClientSemaphore, 1, &prev))
                {
                    S9xNPSetWarning ("NetPlay: Client may be out of sync with server.");
                }
                else
                {
                    if (!NetPlay.Waiting4EmulationThread && 
                        prev == (int) NetPlay.MaxBehindFrameCount)
                    {
                        NetPlay.Waiting4EmulationThread = TRUE;
                        S9xNPSendPause (TRUE);
                    }
                }
            }
            else
                S9xNPDisconnect ();
        }
    }
    else
    {
        S9xClearPause (PAUSE_NETPLAY_CONNECT);
    }
}*/


bool8 S9xNPWaitForHeartBeat ()
{
    uint8 header [3 + 4 + 4 * 5];

    while (S9xNPGetData (NetPlay.Socket, header, 3 + 4))
    {
        if (header [0] != NP_SERV_MAGIC)
        {
            S9xNPSetError ("Bad magic value from server while waiting for heart-beat message\n");
            S9xNPDisconnect ();
            return (FALSE);
        }
        if (header [1] != NetPlay.ServerSequenceNum)
        {
            char buf [200];
            sprintf (buf, "Unexpected message sequence number from server, expected %d, got %d\n", NetPlay.ServerSequenceNum, header [1]);
            S9xNPSetWarning (buf);
            NetPlay.ServerSequenceNum = header [1] + 1;
        }
        else
            NetPlay.ServerSequenceNum++;
        
        if ((header [2] & 0x1f) == NP_SERV_JOYPAD)
        {
        	debug_log("NP_SERV_JOYPAD");
            // Top 2 bits + 1 of opcode is joypad data count.
            int num = (header [2] >> 6) + 1;

            if (num)
            {
                if (!S9xNPGetData (NetPlay.Socket, header + 3 + 4, num * 4))
                {
                    S9xNPSetError ("Error while receiving 'JOYPAD' message.");
                    S9xNPDisconnect ();
                    return (FALSE);
                }
            }
            NetPlay.Frame [NetPlay.JoypadWriteInd] = READ_LONG (&header [3]);

            for (int i = 0; i < num; i++)
            {
                NetPlay.Joypads [NetPlay.JoypadWriteInd][i] = 
                    READ_LONG (&header [3 + 4 + i * sizeof (uint32)]);
            }
            NetPlay.Paused = (header [2] & 0x20) != 0;

            NetPlay.JoypadWriteInd = (NetPlay.JoypadWriteInd + 1) % NP_JOYPAD_HIST_SIZE;
            
            if (NetPlay.JoypadWriteInd != (NetPlay.JoypadReadInd + 1) % NP_JOYPAD_HIST_SIZE)
            {
                //printf ("(%d)", (NetPlay.JoypadWriteInd - NetPlay.JoypadReadInd) % NP_JOYPAD_HIST_SIZE); fflush (stdout);
            }
//printf ("CLIENT: HB: @%d\n", S9xGetMilliTime () - START);
            return (TRUE);
        }
        else
        {
            uint32 len = READ_LONG (&header [3]);
	    switch (header [2] & 0x1f)
	    {
	    case NP_SERV_RESET:
	    	debug_log("NP_SERV_RESET");
                S9xNPDiscardHeartbeats ();
		S9xReset ();
                NetPlay.FrameCount = READ_LONG (&header [3]);
                S9xNPResetJoypadReadPos ();
                S9xNPSendReady ();
                break;
	    case NP_SERV_PAUSE:
	    	debug_log("NP_SERV_PAUSE");
                NetPlay.Paused = (header [2] & 0x20) != 0;
                break;
            case NP_SERV_LOAD_ROM:
            	debug_log("NP_SERV_LOAD_ROM");
                S9xNPDiscardHeartbeats ();
                if (S9xNPLoadROM (len - 7))
                    S9xNPSendReady (NP_CLNT_LOADED_ROM);
                break;
            case NP_SERV_ROM_IMAGE:
            	debug_log("NP_SERV_ROM_IMAGE");
                S9xNPDiscardHeartbeats ();
                if (S9xNPGetROMImage (len - 7))
                    S9xNPSendReady (NP_CLNT_RECEIVED_ROM_IMAGE);
                break;
            case NP_SERV_SRAM_DATA:
            	debug_log("NP_SERV_SRAM_DATA");
                S9xNPDiscardHeartbeats ();
                S9xNPGetSRAMData (len - 7);
                break;
            case NP_SERV_FREEZE_FILE:
            	debug_log("NP_SERV_FREEZE_FILE");
                S9xNPDiscardHeartbeats ();
                S9xNPGetFreezeFile (len - 7);
                S9xNPResetJoypadReadPos ();
                S9xNPSendReady ();
                break;
            default:
            	debug_log("default: disconnect");
                S9xNPDisconnect ();
                return (FALSE);
	    }
	}
    }

    S9xNPDisconnect ();
    return (FALSE);
}

bool8 S9xNPLoadROMDialog (const char *rom_name)
{
    NetPlay.Answer = FALSE;
    return (NetPlay.Answer);
}

bool8 S9xNPLoadROM (uint32 len)
{
/*    uint8 *data = new uint8 [len];
    
    S9xNPSetAction ("Receiving ROM name...");
    if (!S9xNPGetData (NetPlay.Socket, data, len))
    {
        S9xNPSetError ("Error while receiving ROM name.");
        delete data;
        S9xNPDisconnect ();
        return (FALSE);
    }
    
    S9xNPSetAction ("Opening LoadROM dialog...");
    if (!S9xNPLoadROMDialog ((char *) data))
    {
        S9xNPSetError ("Disconnected from NetPlay server because you are playing a different game!");
        delete data;
        S9xNPDisconnect ();
        return (FALSE);
    }
    delete data;*/
    return (TRUE);
}

bool8 S9xNPGetROMImage (uint32 len)
{
/*    uint8 rom_info [5];

    S9xNPSetAction ("Receiving ROM information...");
    if (!S9xNPGetData (NetPlay.Socket, rom_info, 5))
    {
        S9xNPSetError ("Error while receiving ROM information.");
        S9xNPDisconnect ();
        return (FALSE);
    }
    uint32 CalculatedSize = READ_LONG (&rom_info [1]);
    if (CalculatedSize + 5 >= len || 
        CalculatedSize >= CMemory::MAX_ROM_SIZE)
    {
        S9xNPSetError ("Size error in ROM image data received from server.");
        S9xNPDisconnect ();
        return (FALSE);
    }
    
    Memory.HiROM = rom_info [0];
    Memory.LoROM = !Memory.HiROM;
    Memory.HeaderCount = 0;
    Memory.CalculatedSize = CalculatedSize;

    // Load up ROM image
    S9xNPSetAction ("Receiving ROM image...");
    if (!S9xNPGetData (NetPlay.Socket, Memory.ROM, Memory.CalculatedSize))
    {
        S9xNPSetError ("Error while receiving ROM image from server.");
        Settings.StopEmulation = TRUE; 
        S9xNPDisconnect ();
        return (FALSE);
    }
    S9xNPSetAction ("Receiving ROM filename...");
    uint32 filename_len = len - Memory.CalculatedSize - 5;
    if (filename_len > _MAX_PATH ||
        !S9xNPGetData (NetPlay.Socket, (uint8 *) Memory.ROMFilename, filename_len))
    {
        S9xNPSetError ("Error while receiving ROM filename from server.");
        S9xNPDisconnect ();
        Settings.StopEmulation = TRUE;
        return (FALSE);
    }
    Memory.InitROM (FALSE);
    S9xReset ();
    S9xNPResetJoypadReadPos ();
    Settings.StopEmulation = FALSE;

    */
    return (TRUE);
}

void S9xNPGetSRAMData (uint32 len)
{
/*    if (len > 0x10000)
    {
        S9xNPSetError ("Length error in S-RAM data received from server.");
        S9xNPDisconnect ();
        return;
    }
    S9xNPSetAction ("Receiving S-RAM data...");
    if (len > 0 && !S9xNPGetData (NetPlay.Socket, ::SRAM, len))
    {
        S9xNPSetError ("Error while receiving S-RAM data from server.");
        S9xNPDisconnect ();
    }*/
}

void S9xNPGetFreezeFile (uint32 len)
{
/*    uint8 frame_count [4];

    S9xNPSetAction ("Receiving freeze file information...");
    if (!S9xNPGetData (NetPlay.Socket, frame_count, 4))
    {
        S9xNPSetError ("Error while receiving freeze file information from server.");
        S9xNPDisconnect ();
        return;
    }
    NetPlay.FrameCount = READ_LONG (frame_count);

    S9xNPSetAction ("Receiving freeze file...");
    uint8 *data = new uint8 [len];
    if (!S9xNPGetData (NetPlay.Socket, data, len - 4))
    {
        S9xNPSetError ("Error while receiving freeze file from server.");
        S9xNPDisconnect ();
        delete data;
        return;
    }

    //FIXME: Setting umask here wouldn't hurt.
    FILE *file;
#ifdef HAVE_MKSTEMP
    int fd;
    char fname[] = "/tmp/snes9x_fztmpXXXXXX";
    if ((fd = mkstemp(fname)) < 0)
    {
        if ((file = fdopen (fd, "wb")))
#else
    char fname [L_tmpnam];
    if (tmpnam (fname))
    {
        if ((file = fopen (fname, "wb")))
#endif
        {
            if (fwrite (data, 1, len, file) == len)
            {
                fclose(file);
                if (!S9xUnfreezeGame (fname))
                    S9xNPSetError ("Unable to load freeze file just received.");
            } else {
                S9xNPSetError ("Failed to write to temporary freeze file.");
                fclose (file);
            }
        } else
            S9xNPSetError ("Failed to create temporary freeze file.");
        remove (fname);
    } else
        S9xNPSetError ("Unable to get name for temporary freeze file.");
    delete data;*/
}

uint32 S9xNPGetJoypad (int which1)
{
    if (Settings.NetPlay && which1 < 5)
	return (NetPlay.Joypads [NetPlay.JoypadReadInd][which1]);

    return (0);
}

void S9xNPStepJoypadHistory ()
{
    if ((NetPlay.JoypadReadInd + 1) % NP_JOYPAD_HIST_SIZE != NetPlay.JoypadWriteInd)
    {
        NetPlay.JoypadReadInd = (NetPlay.JoypadReadInd + 1) % NP_JOYPAD_HIST_SIZE;
        if (NetPlay.FrameCount != NetPlay.Frame [NetPlay.JoypadReadInd])
        {
            S9xNPSetWarning ("This Snes9X session may be out of sync with the server.");
        }
    }
    else
    {
    }
}


void S9xNPResetJoypadReadPos ()
{
    NetPlay.JoypadWriteInd = 0;
    NetPlay.JoypadReadInd = NP_JOYPAD_HIST_SIZE - 1;
    for (int h = 0; h < NP_JOYPAD_HIST_SIZE; h++)
        memset ((void *) &NetPlay.Joypads [h], 0, sizeof (NetPlay.Joypads [0]));
}

bool8 S9xNPSendJoypadUpdate (uint32 joypad)
{
    uint8 data [7];
    uint8 *ptr = data;

    *ptr++ = NP_CLNT_MAGIC;
    *ptr++ = NetPlay.MySequenceNum++;
    *ptr++ = NP_CLNT_JOYPAD;

    joypad |= 0x80000000;

    WRITE_LONG (ptr, joypad);
    if (!S9xNPSendData (NetPlay.Socket, data, 7))
    {
        S9xNPSetError ("Error while sending joypad data server.");
	S9xNPDisconnect ();
	return (FALSE);
    }
    return (TRUE);
}

void S9xNPDisconnect ()
{
    //close (NetPlay.Socket);
    sceNetInetClose(NetPlay.Socket);
    
    NetPlay.Socket = -1;
    NetPlay.Connected = FALSE;
    Settings.NetPlay = FALSE;
}

bool8 S9xNPSendData (int socket, const uint8 *data, int length)
{
    int len = length;
    const uint8 *ptr = data;

    NetPlay.PercentageComplete = 0;

    do
    {
        if (NetPlay.Abort)
            return (FALSE);

        int num_bytes = len;

        // Write the data in small chunks, allowing this thread to spot an
        // abort request from another thread.
        if (num_bytes > 512)
            num_bytes = 512;

	//int sent = write (socket, (char *) ptr, num_bytes);
	int sent = sceNetInetSend(socket, (char *) ptr, num_bytes,0);	
	if (sent < 0)
	{
		//sceNetInetGetErrno();
	    /*if (errno == EINTR 
#ifdef EAGAIN
		|| errno == EAGAIN
#endif
#ifdef EWOULDBLOCK
		|| errno == EWOULDBLOCK
#endif
		)
            {
		continue;
            }*/
	    return (FALSE);
	}
	else
	if (sent == 0)
	    return (FALSE);
	len -= sent;
	ptr += sent;

        NetPlay.PercentageComplete = (uint8) (((length - len) * 100) / length);
    } while (len > 0);

    return (TRUE);
}

bool8 S9xNPGetData (int socket, uint8 *data, int length)
{
    int len = length;
    uint8 *ptr = data;
    int chunk = length / 50;

    if (chunk < 1024)
        chunk = 1024;

    NetPlay.PercentageComplete = 0;
        
    do
    {
        if (NetPlay.Abort)
            return (FALSE);

        int num_bytes = len;

        // Read the data in small chunks, allowing this thread to spot an
        // abort request from another thread.
        if (num_bytes > chunk)
            num_bytes = chunk;

				//debug_log("get data");

        //int got = read (socket, (char *) ptr, num_bytes);
        int got = sceNetInetRecv (socket, (u8 *) ptr, num_bytes,0);
        
        {char st[32];sprintf(st,"got %d",got);
        	debug_log(st);
        }
        
        if (got < 0)
        {
        	//sceNetInetGetErrno();
	    /*if (errno == EINTR
#ifdef EAGAIN
		|| errno == EAGAIN
#endif
#ifdef EWOULDBLOCK
		|| errno == EWOULDBLOCK
#endif
#ifdef WSAEWOULDBLOCK
                || errno == WSAEWOULDBLOCK
#endif
		)
            {
		continue;
            }*/
#ifdef WSAEMSGSIZE
            if (errno != WSAEMSGSIZE)
                return (FALSE);
            else
            {
                got = num_bytes;
            }
#else
            return (FALSE);
#endif
        }
        else
        if (got == 0)
            return (FALSE);

        len -= got;
        ptr += got;

        if (!Settings.NetPlayServer && length > 1024)
        {
            NetPlay.PercentageComplete = (uint8) (((length - len) * 100) / length);
        }
            
    } while (len > 0);

    return (TRUE);
}

bool8 S9xNPInitialise ()
{
    return (TRUE); 
}

void S9xNPDiscardHeartbeats ()
{
    // Discard any pending heartbeats and wait for any frame that is currently
    // being emulated to complete.


    NetPlay.Waiting4EmulationThread = FALSE;
}

void S9xNPSetAction (const char *action, bool8 force)
{
    if (force || !Settings.NetPlayServer)
    {
        strncpy (NetPlay.ActionMsg, action, NP_MAX_ACTION_LEN - 1);
        NetPlay.ActionMsg [NP_MAX_ACTION_LEN - 1] = 0;
    }
}

void S9xNPSetError (const char *error)
{
    strncpy (NetPlay.ErrorMsg, error, NP_MAX_ACTION_LEN - 1);
    NetPlay.ErrorMsg [NP_MAX_ACTION_LEN - 1] = 0;

}

void S9xNPSetWarning (const char *warning)
{
    strncpy (NetPlay.WarningMsg, warning, NP_MAX_ACTION_LEN - 1);
    NetPlay.WarningMsg [NP_MAX_ACTION_LEN - 1] = 0;
}
#endif


