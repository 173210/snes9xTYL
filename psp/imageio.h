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

void image_free(IMAGE* image);

IMAGE* load_png(int fd);
IMAGE* load_bmp(FILE *fd);
IMAGE* load_bmp_buffer(unsigned char *buf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
