#ifndef __os9xgp_openspc_h__
#define __os9xgp_openspc_h__


typedef enum
{
  SPC_EMULATOR_UNKNOWN = 0,
  SPC_EMULATOR_ZSNES,
  SPC_EMULATOR_SNES9X
} SPC_EmulatorType;

typedef struct SPC_ID666
{
  char songname[33];
  char gametitle[33];
  char dumper[17];
  char comments[33];
  char author[33];
  int playtime;
  int fadetime;
  SPC_EmulatorType emulator;
} SPC_ID666;

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif
int OSPC_Init(void);

int OSPC_Update();
int OSPC_Load(const char *fname);
int OSPC_LoadBuffer(char *buff,int len);
void OSPC_PlayBuffer(char *buff,int len,int release,int vol);
void OSPC_Close(void);
void OSPC_Play(char *fname,int release,int vol);
void OSPC_StopPlay();
SPC_ID666 *OSPC_GetID666(char *buf);
char *OSPC_SongName();
char *OSPC_GameTitle();
char *OSPC_Author();
int OSPC_IsFinished();

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
