#ifndef me_sound_h
#define me_sound_h

void me_sound
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
