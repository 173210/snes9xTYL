#include <pspkernel.h>
#include "me.h"
#include "csc.h"
#include "me_csc.h"


void me_csc_asm(struct csc_struct *p)
	{
	csc_asm(p);
	me_sceKernelDcacheWritebackInvalidateAll();
	}

void me_csc
	(
	volatile struct me_struct *nocache,
	unsigned char *cy,
	unsigned char *cu,
	unsigned char *cv,
	int l0,
	int l1,
	int l2,
	unsigned char *out,
	int width,
	int height,
	int line_size
	)
	{
	struct csc_struct me;
	struct csc_struct main_cpu;


	int height_div_2 = height >> 1;
	int height_div_4 = height >> 2;

	me.cy              = cy;
	me.cu              = cu;
	me.cv              = cv;
	me.l0              = l0;
	me.l1              = l1;
	me.l2              = l2;
	me.out             = out;
	me.width           = width;
	me.height          = height_div_2;
	me.line_size       = line_size;

	main_cpu.cy        = cy + height_div_2 * l0;
	main_cpu.cu        = cu + height_div_4 * l1;
	main_cpu.cv        = cv + height_div_4 * l2;
	main_cpu.l0        = l0;
	main_cpu.l1        = l1;
	main_cpu.l2        = l2;
	main_cpu.out       = out + height_div_2 * (line_size << 2);
	main_cpu.width     = width;
	main_cpu.height    = height_div_2;
	main_cpu.line_size = line_size;

	sceKernelDcacheWritebackInvalidateAll();


	me_start(nocache, &me_csc_asm, &me);

	csc_asm(&main_cpu);
	sceKernelDcacheWritebackInvalidateAll();

	me_wait(nocache);
	}
