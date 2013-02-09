	.set noreorder

#include "pspimport.s"

	IMPORT_START   "sceSysreg_driver",0x00030000
	IMPORT_FUNC    "sceSysreg_driver",0xA9997109,sceSysregMeResetEnable371
	IMPORT_FUNC    "sceSysreg_driver",0x3199CF1C,sceSysregMeBusClockEnable371
	IMPORT_FUNC    "sceSysreg_driver",0x76220E94,sceSysregMeResetDisable371
