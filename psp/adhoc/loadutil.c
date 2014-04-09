#include "psp.h"
#include "loadutil.h"

////////////////////////////////////////////////////////////////////
// SLIME NOTE: Module library linkage trickery
//  we need to be in special kernel mode to load flash0: libraries
//   (created kernel thread + kernel memory module flag set)
//  for everything else program runs in user mode
//  the system does not patch the stubs properly, so we do it ourselves

// slightly cleaned up version

static u32 FindProcEntry(u32 oid, u32 nid)
{
    SceModule* modP = sceKernelFindModuleByUID(oid);
    if (modP == NULL)
    {
        printf("ERROR: Failed to find mod $%x\n", oid);
        return 0;
    }
    SceLibraryEntryTable* entP = (SceLibraryEntryTable*)modP->ent_top;
    while ((u32)entP < ((u32)modP->ent_top + modP->ent_size))
    {
        int count = entP->stubcount + entP->vstubcount;
        u32* nidtable = (u32*)entP->entrytable;
        int i;
        for (i = 0; i < count; i++)
        {
            if (nidtable[i] == nid)
            {
                u32 procAddr = nidtable[count+i];
                return procAddr;
            }
        }
        entP++;
    }
    return 0;
}

static int PatchMyLibraryEntries(SceModuleInfo* modInfoPtr, SceUID oidLib)
{
    //dumb version
    int nPatched = 0;

    int* stubPtr; // 20 byte structure
    for (stubPtr = modInfoPtr->stub_top;
        stubPtr + 5 <= (int*)modInfoPtr->stub_end;
        stubPtr += 5)
    {
        int count = (stubPtr[2] >> 16) & 0xFFFF;
        int* idPtr = (int*)stubPtr[3];
        int* procPtr = (int*)stubPtr[4];

        if (stubPtr[1] != 0x90000)
            continue;   // skip non-lazy loaded modules
        while (count--)
        {
            if (procPtr[0] == 0x54C && procPtr[1] == 0)
            {
                // SWI - scan for NID
                u32 proc = FindProcEntry(oidLib, *idPtr);
                if (proc != 0)
                {
                    if (((u32)procPtr & 0xF0000000) != (proc & 0xF0000000))
                    {
                        // if not in user space we can't use it
                        //dprintf("!! NOT PATCH\n");
                    }
                    else
                    {
                        u32 val = (proc & 0x03FFFFFF) >> 2;
                        procPtr[0] = 0x0A000000 | val;
                        procPtr[1] = 0;
                        
                        nPatched++;
                    }
                }
            }
            idPtr++;
            procPtr += 2; // 2 opcodes
        }
    }
    return nPatched;
}

SceUID LoadAndStartAndPatch(SceModuleInfo* modInfoPtr, const char* szFile)
    // return oid or error code
{
    SceUID oid;
    int err;
    int startStatus; // ignored

	//printf("file=%s\n", szFile);

    oid = sceKernelLoadModule(szFile, 0, NULL);
    if (oid & 0x80000000)
	{
		pspDebugScreenInit();
		printf("error1\n");
        return oid; // error code
	}

    err = sceKernelStartModule(oid, 0, 0, &startStatus, 0);
    if (err != oid)
	{
		pspDebugScreenInit();
		printf("error2\n");
        return err;
	}

    PatchMyLibraryEntries(modInfoPtr, oid);
    return oid;
}


extern void sceKernelIcacheInvalidateAll();

void FlushCaches()
{
    // not sure if these are necessary, but to be extra safe
    sceKernelDcacheWritebackAll();
    sceKernelIcacheInvalidateAll();
}
