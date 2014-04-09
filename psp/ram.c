/* RAM simple check functions source */


// *** INCLUDES ***

#include "ram.h"


// *** FUNCTIONS ***

u32 ramAvailableLineareMax (void)
{
 u32 size, sizeblock;
 u8 *ram;


 // Init variables
 size = 0;
 sizeblock = RAM_BLOCK;

 // Check loop
 while (sizeblock)
 {
  // Increment size
  size += sizeblock;

  // Allocate ram
  ram = malloc(size);

  // Check allocate
  if (!(ram))
  {
   // Restore old size
   size -= sizeblock;

   // Size block / 2
   sizeblock >>= 1;
  }
  else
   free(ram);
 }

 return size;
}

u32 ramAvailable (void)
{
 u8 **ram, **temp;
 u32 size, count, x;


 // Init variables
 ram = NULL;
 size = 0;
 count = 0;

 // Check loop
 for (;;)
 {
  // Check size entries
  if (!(count % 10))
  {
   // Allocate more entries if needed
   temp = realloc(ram,sizeof(u8 *) * (count + 10));
   if (!(temp)) break;
 
   // Update entries and size (size contains also size of entries)
   ram = temp;
   size += (sizeof(u8 *) * 10);
  }

  // Find max lineare size available
  x = ramAvailableLineareMax();
  if (!(x)) break;

  // Allocate ram
  ram[count] = malloc(x);
  if (!(ram[count])) break;

  // Update variables
  size += x;
  count++;
 }

 // Free ram
 if (ram)
 {
  for (x=0;x<count;x++) free(ram[x]);
  free(ram);
 }

 return size;
}


