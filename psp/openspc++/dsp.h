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



dsp.h: defines functions that emulate the DSP part of the SPC module.
The functions and data defined in this file must be shared because the
SPC core needs access to them, however they are not intended for external
library use, and their specific implementations and prototypes are subject
to change.

 ************************************************************************/

#ifndef DSP_H
#define DSP_H

//#include "TapWave.h"

struct voice_state
{
	unsigned long samp_id;	/* sample ID# at time sample was keyed on.
				   (lower 16-bits = start address,
				    upper 16-bits = loop address) */
	signed long smp1,smp2;	/* last two values decoded (for filter) */
	int mem_ptr,		/* where in memory is sample data to read */
	    header_cnt,		/* how long before another header (0-8) */
	    range,		/* last header's range */
	    filter,		/* last header's filter */
	    end,		/* 1=end before next block, 3=loop instead */
	    half,		/* 0=upper half,1=lower half */
	    mixfrac,		/* fractional part of sample position */
	    envcnt,		/* how long 'till update envelope */
	    envx,		/* last envelope height (0-0x7FFF) */
	    pitch,		/* last known pitch (4096 -> 32000Hz) */
	    sampptr,		/* Where in sampbuf we are */
	    on_cnt;		/* Is it time to turn on yet? */
	int envstate;	/* current envelope state */
	short sampbuf[4];	/* Buffer for Gaussian interpolation */
};

struct src_dir {unsigned short vptr,lptr;};

class dsp
{

private:
/**** Global Variables :P ****/

	enum { ATTACK=0,DECAY,SUSTAIN,DSPRELEASE };

	unsigned char *SPC_RAM;
	unsigned char *DSPregs;

	int keyed_on, keys;			/* 8-bits for 8 voices */
	struct voice_state voice_state[8];

	/*
	 * These are for the FIR echo filter 
	 */
#ifndef NO_ECHO
	short FIRlbuf[8], FIRrbuf[8];
	int FIRptr, echo_ptr;
#endif

	/*
	 * Noise stuff 
	 */
	int noise_cnt, noise_lev;

	int mask;

	int AdvanceEnvelope(int v);

public:
	void update(short *sound_ptr);
	void reset();
	void setmask(int m);
	int getmask();

	dsp (void *pSPC_RAM, void *pDSPregs);
};

/**** Macros ****/
/* The functions to actually read and write to the DSP registers must be
   implemented by the specific SPC core implementation, as this is too 
   specific to generalize.  However, by defining these macros, we can
   generalize the DSP's behavior while staying out of the SPC's internals,
   by requiring that the SPC core must use these macros at the appropriate
   times. */

/* All reads simply return the contents of the addressed register. */

/* This macro must be used INSTEAD OF a normal write to register 0x7C
   (ENDX) */
#define DSP_WRITE_7C(x) DSPregs[0x7C]=0

/* All other writes should store the value in the addressed register as
   expected. */

#endif /* #ifndef DSP_H */

