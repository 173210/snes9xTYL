#ifndef _IMAGEIO_H_
#define _IMAGEIO_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
	unsigned char r,g,b,a;
} COLOR;

typedef struct {
	int width;
	int height;
	int bit;
	void *pixels;
	int n_palette;
	COLOR* palette;
} IMAGE;

IMAGE* image_alloc(int width,int height,int bit);
void image_free(IMAGE* image);

IMAGE* load_png(int fd);
IMAGE* load_bmp(FILE *fd);
IMAGE* load_bmp_buffer(unsigned char *buf);
int save_bmp(const char *file,int width,int height,int bit,void *bits,int pitch);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
