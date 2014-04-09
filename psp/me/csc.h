#ifndef csc_h
#define csc_h

struct csc_struct
	{
	unsigned char *cy;
	unsigned char *cu;
	unsigned char *cv;
	int l0;
	int l1;
	int l2;
	unsigned char *out;
	int width;
	int height;
	int line_size;
	};

void csc_asm(struct csc_struct *p);
void csc_c  (struct csc_struct *p);

#endif
