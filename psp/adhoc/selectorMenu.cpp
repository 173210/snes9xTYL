
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <string.h>

#include <string>
#include <algorithm>
#include <vector>

extern "C" {
#include "pg.h"
}

using namespace std;

extern "C" void sceNetEtherNtostr(const char *, const char *);

#define NUM_ENTRIES 32

struct PspEntry
{
	char m_Name[256];
	char m_Mac[6];

	PspEntry(const unsigned char *mac, const char *name, int nameLen)
	{
		memcpy(m_Mac, mac, 6);
		if(nameLen)
		{
			strncpy(m_Name, name, nameLen);
			m_Name[nameLen] = '\0';
		}
		else
			m_Name[0] = '\0';
	}
};

typedef	vector<PspEntry> PspList;

class PspSelector
{
private:
	PspList m_PspList;
//	int GetPspList();

	int m_Pos;
	int m_Max;

public:
	void DisplayPspList();
	void ClearPspList();

	int AddPsp(const unsigned char *mac, const char *name, int length);
	int RemPsp(const unsigned char *mac);
	void DownList(void)
	{
		if(m_Pos < m_Max-1)
			m_Pos++;
	};
	void UpList(void)
	{
		if(m_Pos > 0)
			m_Pos--;
	};

	int GetPsp(unsigned char *mac, char *name);
	PspSelector();
	int m_Quit;
};

PspSelector::PspSelector(void)
{
	m_Pos = 0;	
	m_Max = 0;
}

void PspSelector::ClearPspList(void)
{
	m_Pos = 0;
	m_Max = 0;
	m_PspList.clear();
}

int PspSelector::AddPsp(const unsigned char *mac, const char *name, int length)
{
	int i;

	// Only want to show up servers
	if(length == 1)
		return 0;

	for(i=0; i<m_Max; i++)
	{
		// Don't add multiple entries
		if(strcmp(m_PspList[i].m_Mac, (const char *)mac)==0)
			return 0;
	}

	m_PspList.push_back(PspEntry(mac, name, length));
	m_Max++;

	return 1;
}

int PspSelector::RemPsp(const unsigned char *mac)
{
	int i;
	PspList::iterator iter;

	for(iter=m_PspList.begin(); iter != m_PspList.end(); iter++)
	{
		// Don't add multiple entries
		if(strcmp((*iter).m_Mac, (const char *)mac)==0)
		{
			m_PspList.erase(iter);
			
			if(m_Pos == i)
				m_Pos = 0;

			if(m_Pos > i)
				m_Pos--;
			
			m_Max--;

			return 0;
		}
	}

	return -1;
}

void PspSelector::DisplayPspList()
{
	int i=0;

	char tempStr[20];
	char str[256];
	int col;

	if(m_Max == 0)
	{
		//pspDebugScreenPrintf("Waiting for another PSP to join\n");
		mh_print(0,8,(char*)"Waiting for another PSP to join",0xFFFF);
	}

	for(i=0; i<m_Max; i++)
	{
		if(i == m_Pos)
		{
			//pspDebugScreenSetTextColor(0xFF00);
			col=31<<5;
		}
		else
		{
			//pspDebugScreenSetTextColor(0xFFFF);
			col=0xFFFF;
		}

		sceNetEtherNtostr(m_PspList[i].m_Mac, tempStr);

		//pspDebugScreenPrintf("  %s: %s\n", tempStr, m_PspList[i].m_Name);
		sprintf(str,"  %s: %s\n", tempStr, m_PspList[i].m_Name);
		mh_print(0,8+i*8,(char*)str,col);
	}
}

int PspSelector::GetPsp(unsigned char *mac, char *name)
{
	if(m_Max == 0)
		return -1;

	memcpy(mac, m_PspList[m_Pos].m_Mac, 6);
	strcpy(name, m_PspList[m_Pos].m_Name);

	return 1;
}

PspSelector pspSel;

extern "C" int AddPsp(const unsigned char *mac, const char *name, int length)
{
	return pspSel.AddPsp(mac, name, length);
}

extern "C" int DelPsp(const unsigned char *mac)
{
	return pspSel.RemPsp(mac);
}

extern "C" void DownList(void)
{
	pspSel.DownList();
}

extern "C" void UpList(void)
{
	pspSel.UpList();
}

extern "C" void DisplayPspList(void)
{
	pspSel.DisplayPspList();
}

extern "C" int GetPspEntry(unsigned char *mac, char *name)
{
	return pspSel.GetPsp(mac, name);
}

extern "C" void ClearPspList(void)
{
	pspSel.ClearPspList();
}
