/******************************************************************************

	clock.prx

******************************************************************************/

#include <pspsdk.h>
#include <pspctrl.h>

#include <psppower.h>

PSP_MODULE_INFO("clock", PSP_MODULE_KERNEL, 0, 0);
PSP_MAIN_THREAD_ATTR(0);


/******************************************************************************
	prototypes
******************************************************************************/




/******************************************************************************
	local variables
******************************************************************************/

static volatile int cpuclock;
static volatile int old_cpuclock;
static SceUID home_thread;
static volatile int home_active;

/******************************************************************************
	functions
******************************************************************************/
#include <systemctrl.h>
static int home_button_thread(SceSize args, void *argp)
{

	home_active = 1;
	cpuclock=333;
	old_cpuclock=333;
	while (home_active)
	{
		if(old_cpuclock!=cpuclock)
		{
				switch (cpuclock){
					//case 266:scePowerSetClockFrequency(266,266,133);break;
					//case 300:scePowerSetClockFrequency(300,300,150);break;
					//case 333:scePowerSetClockFrequency(333,333,166);break;
					//default :scePowerSetClockFrequency(222,222,111);
					case 266:sctrlHENSetSpeed(266,133);break;
					case 300:sctrlHENSetSpeed(300,150);break;
					case 333:sctrlHENSetSpeed(333,166);break;
					default :sctrlHENSetSpeed(222,111);
			}
			old_cpuclock=cpuclock;
		}
		sceKernelDelayThread(10 * 1000);
	}

	sceKernelExitDeleteThread(0);

	return 0;
}

void PowerSetClockFrequency(int clock)
{
	cpuclock=clock;
}


int module_start(SceSize args, void *argp)
{

	home_active = 0;
	home_thread = sceKernelCreateThread("Clock Thread",
								home_button_thread,
								0x11,
								0x200,
								0,
								NULL);

	if (home_thread >= 0)
		sceKernelStartThread(home_thread, 0, 0);

	return 0;
}


int module_stop(void)
{
	if (home_thread >= 0)
	{
		home_active = 0;
		sceKernelDelayThread(20 * 1000);
	}
	return 0;
}
