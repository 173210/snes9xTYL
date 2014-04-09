/************************************************************************

		Copyright (c) 2002 Brad Martin.

This file is part of OpenSPC.

OpenSPC is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

OpenSPC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with OpenSPC; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

This file contains the exported interface for libospcemu.

 ************************************************************************/

#ifndef OPENSPC_H
#define OPENSPC_H

//#include "TapWave.h"
//#include "palmdep.h"

#define size_t int

#include "psp/openspc++/spc.h"
#include "psp/openspc++/dsp.h"

class openspc
{
private:
	int mix_left;

	spc *pspc;
	dsp *pdsp;

	int align_dummy1;
	unsigned char SPC_RAM[65536];
	unsigned char DSPregs[256];

	int Load_SPC(void *buf, size_t size);

public:
	openspc ();
	~openspc ();

	int init(void *buf, size_t size);
/* This method is used to load a new state into the emulator.  buf points to
   the memory region containing the image to be loaded.  Can be an SPC file,
   or a ZSNES or Snes9X savestate (autodetected).  Returns 0 on success, 1
   on failure to identify file type, and a negative value for any other
   error (currently none are defined). */

	int run(int cyc, short *s_buf, int s_size);
/* This method performs the actual emulation.  cyc is the number of cycles
   desired to execute.  s_buf should point to an area of memory to render
   the sound output into, or NULL if this is not desired.  s_size is the
   size of s_buf, in bytes.  Execution will stop when either cyc cycles have
   been executed, or s_buf has been filled, whichever comes first.  If the
   number of cycles executed does not matter, pass a negative cyc value, and
   s_size will be used instead to determine the time to run.  s_size is
   ignored if s_buf is NULL, except in the case when cyc is also negative.
   Returns the amount of data (in bytes) rendered into s_buf. */

	void setmute(int m);

	void writeport(int port, char data);
/* This method is used to communicate with the SPC.  Port is set to one of
   the SPC's four ports used to receive input from the CPU, available to
   read from $F4-$F7 in the SPC's address space, and to write from $2140-
   $2143 in the CPU's address space.  data is the value you wish to make
   appear on the specified port for the SPC to read. */

	char readport(int port);
/* This method is used to receive communications from the SPC.  Port
   corresponds to the SPC's four ports used to send output to the CPU,
   available to write from $F4-$F7 in the SPC's address space, and to read
   from $2140-$2143 in the CPU's address space.  Returns the data written to
   the corresponding port by the SPC.  Note that these ports are entirely
   separate from the ports written to with OSPC_WritePortX above.  Data
   written with OSPC_WritePortX will not be available to be read using
   OSPC_ReadPortX.  Only that data which the SPC posts on these ports is
   visible from the outside. */
};

#endif /* #ifndef OPENSPC_H */

