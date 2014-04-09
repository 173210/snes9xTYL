#ifndef me_csc_h
#define me_csc_h

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
	);

#endif
