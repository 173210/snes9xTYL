#ifndef __selectorMenu_h__
#define __selectorMenu_h__

int AddPsp(const unsigned char *mac, const char *name, int length);
int DelPsp(const unsigned char *mac);
void DisplayPspList(void);
void ClearPspList(void);

void DownList(void);
void UpList(void);

int GetPspEntry(const unsigned char *mac, const char *name);

#endif
