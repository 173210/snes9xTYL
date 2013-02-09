	.set noreorder

#include "pspstub.s"

	STUB_START "clock",0x40090000,0x00010005
	STUB_FUNC  0x20231F48,PowerSetClockFrequency
	STUB_END
