#include <malloc.h>
#include "mem64.h"

void *malloc_64(int size)
{
	int mod_64 = size & 0x3f;

	if (mod_64 != 0) size += 64 - mod_64;

	return (void *)memalign(64, size);
}

void free_64(void *p)
{
	free(p);
}
