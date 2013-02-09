#ifndef __selectorMenu_h__
#define __selectorMenu_h__

int AddPsp(char *mac, char *name, int length);
int DelPsp(char *mac);
void DisplayPspList(void);
void ClearPspList(void);

void DownList(void);
void UpList(void);

int GetPspEntry(char *mac, char *name);

#endif
