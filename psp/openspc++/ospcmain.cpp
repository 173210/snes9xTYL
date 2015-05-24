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



main.c: implements functions intended for external use of the libopenspc
library.

 ************************************************************************/

//#include "TapWave.h"


//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#include "snes9x.h"


#include "psp/openspc++/dsp.h"
#include "psp/openspc++/spc.h"
#include "psp/openspc++/openspc.h"

#undef NO_CLEAR_ECHO

extern int TS_CYC;


/************************/

/************************/


/**** Internal (static) functions ****/

typedef struct
	{
		unsigned char ident[37],
		              PCLow,PCHigh,
		              A,
		              X,
		              Y,
		              P,
		              SP,
		              junk[212],
		              RAM[65536],
		              DSP[128];
	} t_SPC_FILE;

int
openspc::Load_SPC (void *buf, size_t size)
{
	const char ident[] = "SNES-SPC700 Sound File Data";
	t_SPC_FILE  *spc_file;
	
	
	if (size < (size_t) sizeof (t_SPC_FILE))
		return 1;
	spc_file = (t_SPC_FILE *) buf;

	if (memcmp (buf, ident, strlen (ident)))
	{
		return 2;
	}
	memcpy (SPC_RAM, spc_file->RAM, 65536);
	pspc->setstate (((int)spc_file->PCHigh<<8)+spc_file->PCLow, spc_file->A,
	 spc_file->X, spc_file->Y, spc_file->P, 0x100 + spc_file->SP);
	memcpy (DSPregs, spc_file->DSP, 128);
	return 0;
}


/**** Exported library interfaces ****/

int
openspc::init (void *buf, size_t size)
{
	int ret;
#ifndef NO_CLEAR_ECHO
	int start, len;
#endif
	mix_left = 0;

	pspc->reset ();

	pdsp->reset ();
	

	ret = Load_SPC (buf, size);

/* New file formats could go on from here, for example:
	if(ret==1)
		ret=Load_FOO(buf,size);
	...
*/
#ifndef NO_CLEAR_ECHO
	/* Because the emulator that generated the SPC file most likely did
	   not correctly support echo, it is probably necessary to zero out
	   the echo region of memory to prevent pops and clicks as playback
	   begins. */
	if (!(DSPregs[0x6C] & 0x20))
	{
		start = (unsigned char) DSPregs[0x6D] << 8;
		len = (unsigned char) DSPregs[0x7D] << 11;
		if (start + len > 0x10000)
			len = 0x10000 - start;

		memset (&SPC_RAM[start], 0, len);

	}
#endif
	return ret;
}

int
openspc::run (int cyc, short *s_buf, int s_size)
{
	int i;
	if ((cyc < 0) && (s_size) && (s_buf == NULL))
	{
		s_size &= ~3;
		if (mix_left)
			pspc->run (mix_left);
		pspc->run (TS_CYC * (s_size >> 2));
		mix_left = 0;
		return s_size;
	}
	if ((cyc < 0)
		|| ((s_buf != NULL) && (cyc >= (s_size >> 2) * TS_CYC + mix_left)))
	{
		s_size &= ~3;
		if (mix_left)
			pspc->run (mix_left);
		for (i = 0; i < s_size; i += 4, s_buf += 2)
		{
			pdsp->update (s_buf);
			pspc->run (TS_CYC);
		}
		mix_left = 0;
		return s_size;
	}
	if (cyc < mix_left)
	{
		pspc->run (cyc);
		mix_left -= cyc;
		return 0;
	}
	if (mix_left)
	{
		pspc->run (mix_left);
		cyc -= mix_left;
	}
	for (i = 0; cyc >= TS_CYC; i += 4, cyc -= TS_CYC, s_buf += 2)
	{
		pdsp->update (s_buf);
		pspc->run (TS_CYC);
	}
	if (cyc)
	{
		pdsp->update (s_buf);
		pspc->run (cyc);
		mix_left = TS_CYC - cyc;
		i += 4;
	}
	return i;
}

void
openspc::setmute (int m)
{
	pdsp->setmask (~m & 0xFF);
}

void
openspc::writeport (int port, char data)
{
	pspc->writeport (port, data);
}

char
openspc::readport (int port)
{
	return pspc->readport (port);
}

openspc::openspc ()
{
	memset(&SPC_RAM, 0, 65536);
	memset(&DSPregs, 0, 256);
	pspc = new spc (&SPC_RAM, &DSPregs);	
/*	pspc = (spc*)ljz_malloc(sizeof(spc));
	((spc*)pspc)->spc(&SPC_RAM, &DSPregs);*/	
	pdsp = new dsp (&SPC_RAM, &DSPregs);
/*	pdsp = (dsp*)ljz_malloc(sizeof(dsp));
	((dsp*)pdsp)->dsp(&SPC_RAM, &DSPregs);*/
}

openspc::~openspc ()
{
	delete pdsp;
	delete pspc;
	/*pdsp->~dsp();
	pspc->~spc();
	ljz_free(pdsp);
	ljz_free(pspc);*/
}
