#include "psp.h"

#include "memmap.h"


extern "C" {
#include "pspadhoc.h"

extern int g_Server;
int save_buffer_settings(uint8 *buffer);
int load_buffer_settings(uint8 *buffer);

uint32 caCRC32(uint8 *array, uint32 size, register uint32 crc32 = 0xFFFFFFFF);
}

extern char os9x_nickname[256];
extern char shortrom_filename[64];

extern int os9x_netplay,os9x_conId; //netplay : 1 is client, 2 is server
extern int os9x_padindex,os9x_netpadindex;

__attribute__((aligned(64))) uint8 net_pkt_recv[NET_PKT_LEN],net_pkt_send[NET_PKT_LEN];
__attribute__((aligned(64))) uint8 net_buffer[0x2000];
char str_id[256];

int psp_initadhocgame(void){
	int retval;
	//testing net stuff
	//pspDebugScreenInit();

	sprintf(str_id,"%s\\/%s",os9x_nickname,shortrom_filename);

	if((adhocInit(str_id) >= 0) && (adhocSelect() >=0)) { //init adhoc
		if (g_Server) os9x_conId = 1;
		else os9x_conId = 2;
		//buffer=(uint8*)malloc(256);
		//sceKernelDelayThread(1000000); //wait
		//pspDebugScreenInit();

		if (os9x_conId==1) { //server
			//send settings			is now done in psp.cpp
			/*save_buffer_settings(buffer);
			sceKernelDelayThread(3000000); //wait

			printf("Sending settings data to the client\n");
			err = adhocSendRecvAck(buffer, 256);
			if (!err) printf("Done sending data\n");
			else {
					printf("Error %d!",err);
					retval=-2;
					sceKernelDelayThread(1000000); //wait
			}*/
			//server is pl1
			os9x_netpadindex=0;
		} else { //client
			/*int size = 0;

			// receive the settings data
			printf("Waiting for settings data\n");
			length = 256;
			err = adhocRecvSendAck(buffer, &length);
			if (!err) {
				printf("Received data from server %d bytes\n",length);
				load_buffer_settings(buffer);
		  } else {
					printf("Error %d!",err);
					retval=-3;
					sceKernelDelayThread(1000000); //wait
			}*/
			//client is pl2
			os9x_netpadindex=1;
		}
		retval=0;
		//free(buffer);

	} else retval=-1;

	sceDisplaySetMode( 0, SCREEN_WIDTH, SCREEN_HEIGHT );
	sceDisplaySetFrameBuf( (char*)VRAM_ADDR, 512, 1, 1 );
	pgScreenFrame(2,0);

	return retval;
}

int psp_net_recv_file(char *filename) {
	unsigned int file_size,length;
	unsigned int crc32,rlen;
	char *_net_buffer = (char *)net_buffer;
//	uint8 *buffer;
	FILE *f;

	// Added delay to avoid system hanging in some PSPs
	sceKernelDelayThread(100000);
	length=NET_PKT_LEN;
	if (adhocRecvSendAck(net_pkt_recv,&length)<0) {
		//pb while receiving data
		psp_msg(ADHOC_LOST_CONNECTION, MSG_DEFAULT);
		return -1;
	}
	length=NET_PKT_LEN;
	if (adhocRecvSendAck(net_pkt_recv,&length)<0) {
		//pb while receiving data
		psp_msg(ADHOC_LOST_CONNECTION, MSG_DEFAULT);
		return -1;
	}
	if ((net_pkt_recv[5]!=NET_MAGIC1)||(net_pkt_recv[6]!=NET_MAGIC2)||(net_pkt_recv[7]!=NET_MAGIC3)) {
		//pb while receiving data
		char str[64];sprintf(str,"%s : %02X%02X%02X%02X%02X%02X%02X%02X",psp_msg_string(ADHOC_CORRUPTED_PKT),net_pkt_recv[0],
									net_pkt_recv[1],net_pkt_recv[2],net_pkt_recv[3],net_pkt_recv[4],net_pkt_recv[5],net_pkt_recv[6],net_pkt_recv[7]);
		msgBoxLines(str,60*2);
		return -2;
	}
	file_size=(((int)net_pkt_recv[1])<<24)|(((int)net_pkt_recv[2])<<16)|(((int)net_pkt_recv[3])<<8)|(((int)net_pkt_recv[4])<<0);

	{char str[32];sprintf(str,psp_msg_string(ADHOC_STATE_SIZE),file_size);msgBoxLines(str,10);}

	//recv  file
	f=fopen(filename,"wb");
	if (!f) {
		psp_msg(ADHOC_FILE_ERR_RECEIVING, MSG_DEFAULT);
		return -3;
	}
	//buffer=(uint8*)malloc(0x1004);
	//if (!buffer) {msgBoxLines("malloc error while sending state!",60*1);sceKernelExitGame();}

	do {
		char str[64];sprintf(str,psp_msg_string(ADHOC_STILL),file_size);msgBoxLines(str,0);
		if (file_size>0x1000) rlen=0x1000;
		else rlen=file_size;
		for (;;) {
			length=rlen+4;
			if (adhocRecvSendAck(net_buffer,&length)<0) {
				psp_msg(ADHOC_FILE_ERR_RECEIVING, MSG_DEFAULT);
				return -4;
			}
			crc32=caCRC32(net_buffer+4,rlen);
			if (crc32!=*((unsigned int*)_net_buffer)) {
				net_buffer[0]=0;
				adhocSendRecvAck(net_buffer,1);
			} else {
				net_buffer[0]=1;
				adhocSendRecvAck(net_buffer,1);
				break;
			}
		}
		fwrite(net_buffer+4,1,rlen,f);
		file_size-=rlen;
	} while (file_size);
	fclose(f);
	//free(buffer);
	// Added delay to avoid system hanging in some PSPs
	sceKernelDelayThread(100000);
	return 0;
}

int psp_net_send_file(char *filename) {
	unsigned int length,crc32,l,rlen;
	char c;
	char *_net_buffer = (char *)net_buffer;
	//uint8 *buffer;
	FILE *f;

	// Added delay to avoid system hanging in some PSPs
	sceKernelDelayThread(100000);
	f=fopen(filename,"rb");
	if (!f) {
		psp_msg(ADHOC_FILE_ERR_SENDING, MSG_DEFAULT);
		return -1;
	}
	//buffer=(uint8*)malloc(0x1004);
	//if (!buffer) {
//		fclose(f);
//		msgBoxLines("malloc error while sending state!",60*1);
//		return -2;
//	}
	fseek(f,0,SEEK_END);
	length=ftell(f);
	fseek(f,0,SEEK_SET);

	{char str[32];sprintf(str,psp_msg_string(ADHOC_STATE_SIZE),length);msgBoxLines(str,10);}

	psp_msg(ADHOC_WAITING_OTHER, MSG_DEFAULT);

	net_pkt_send[0]=3;
	net_pkt_send[1]=(length>>24)&0xFF;
	net_pkt_send[2]=(length>>16)&0xFF;
	net_pkt_send[3]=(length>>8)&0xFF;
	net_pkt_send[4]=(length>>0)&0xFF;
	net_pkt_send[5]=NET_MAGIC1;
	net_pkt_send[6]=NET_MAGIC2;
	net_pkt_send[7]=NET_MAGIC3;
	adhocSendRecvAck(net_pkt_send, NET_PKT_LEN);
	adhocSendRecvAck(net_pkt_send, NET_PKT_LEN);  //first packet seems to be corrupted...

	do {
		char str[64];sprintf(str,psp_msg_string(ADHOC_STILL),length);msgBoxLines(str,0);
		if (length>0x1000) rlen=0x1000;
		else rlen=length;

		fread(net_buffer+4,1,rlen,f);
		length-=rlen;

		crc32=caCRC32(net_buffer+4,rlen);
		*((int*)_net_buffer)=crc32;
		for (;;) {
			if (adhocSendRecvAck(net_buffer,rlen+4)<0) {
				//free(buffer);
				fclose(f);
				return -3;
			}
			l=1;
			if (adhocRecvSendAck(&c,&l)<0) {
				//free(buffer);
				fclose(f);
				return -4;
			}
			if (c==1) break;
		}

	} while (length);
	fclose(f);
//	free(buffer);
	// Added delay to avoid system hanging in some PSPs
	sceKernelDelayThread(100000);
	return 0;
}
