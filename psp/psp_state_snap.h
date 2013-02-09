#ifndef __psp_state_snap_h__
#define __psp_state_snap_h__


int os9x_savesnap(void);
int os9x_loadsnap(char *fname,u16 *snes_image,int *height);
int os9x_savesram(void);
int os9x_save(const char *ext);
int os9x_load(const char *ext);
int os9x_loadfname(const char *fname);
int os9x_loadzsnes(char *ext);

#endif
