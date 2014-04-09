// system entry trickery

// if library must be loaded into user memory/partition, but from
//   a kernel thread, we must manually patch the entry table
SceUID LoadAndStartAndPatch(SceModuleInfo* modInfoPtr, const char* szFile);
void FlushCaches();

