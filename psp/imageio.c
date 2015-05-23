
//#include <png.h>
//#include <unistd.h>
//#include <fcntl.h>
//#include <sys/stat.h>
//#include <malloc.h>
#include <stdio.h>
#include "psp.h"
#include "imageio.h"

typedef unsigned char BYTE;
typedef unsigned DWORD;
typedef int LONG;
typedef unsigned short WORD;

typedef struct tagRGBQUAD {
	BYTE	rgbBlue;
	BYTE	rgbGreen;
	BYTE	rgbRed;
	BYTE	rgbReserved;
} RGBQUAD,*LPRGBQUAD;

typedef struct tagBITMAPFILEHEADER {
	WORD	bfType;
	DWORD	bfSize;
	WORD	bfReserved1;
	WORD	bfReserved2;
	DWORD	bfOffBits;
} BITMAPFILEHEADER,*LPBITMAPFILEHEADER,*PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
	DWORD	biSize;
	LONG	biWidth;
	LONG	biHeight;
	WORD	biPlanes;
	WORD	biBitCount;
	DWORD	biCompression;
	DWORD	biSizeImage;
	LONG	biXPelsPerMeter;
	LONG	biYPelsPerMeter;
	DWORD	biClrUsed;
	DWORD	biClrImportant;
} BITMAPINFOHEADER,*LPBITMAPINFOHEADER,*PBITMAPINFOHEADER;

static IMAGE* image_alloc(int width,int height,int bit)
{
	IMAGE* image = malloc(sizeof(IMAGE));
	image->width = width;
	image->height = height;
	image->bit = bit;
	if (bit==15) bit = 16;
	image->pixels = malloc((width*bit+7)/8*height);
	if (bit<=8) {
		image->n_palette = 1<<bit;
		image->palette = malloc(sizeof(COLOR)*(1<<bit));
	} else {
		image->n_palette = 0;
		image->palette = NULL;
	}

	return image;
}

void image_free(IMAGE* image)
{
	if (image->pixels) free(image->pixels);
	if (image->palette) free(image->palette);
	free(image);
}

IMAGE* load_bmp(FILE *fd)
{
	WORD bfType;
	struct {
		DWORD	bfSize;
		WORD	bfReserved1;
		WORD	bfReserved2;
		DWORD	bfOffBits;
	} bf;

	RGBQUAD rgbquad[256];

	BITMAPINFOHEADER bi;
	fread(&bfType,1,sizeof(bfType),fd);

	if (bfType != 'M'*256+'B') return NULL;
	fread(&bf,1,sizeof(bf),fd);
	fread(&bi,1,sizeof(bi),fd);
	
	if (bi.biCompression!=0 || bi.biWidth > 480 || bi.biHeight >272)
		return NULL;
	
	IMAGE *image = image_alloc(bi.biWidth,bi.biHeight,bi.biBitCount);
	int pitch = ((bi.biWidth * bi.biBitCount + 31)/8)&-4;
	//char *buffer = malloc(bi.biSizeImage);
	//image->pixels = buffer;
	char *buffer = image->pixels;

	if (bi.biBitCount<=8) {
		int ncolor = bi.biClrUsed;
		if (ncolor==0) ncolor = 1<< bi.biBitCount;
		fread(rgbquad,1,sizeof(RGBQUAD)*ncolor,fd);
		int i;
		for(i=0;i<ncolor;i++) {
			image->palette[i].r = rgbquad[i].rgbRed;
			image->palette[i].g = rgbquad[i].rgbGreen;
			image->palette[i].b = rgbquad[i].rgbBlue;
		}

	}

	fseek(fd,bf.bfOffBits,SEEK_SET);
	int x,y;
	switch(bi.biBitCount) {
	case 8:
		for(y=bi.biHeight-1;y>=0;y--) {
			fread(buffer + pitch*y,1, pitch,fd);
		}
		break;

	case 24:
		for(y=bi.biHeight-1;y>=0;y--) {
			fread(buffer + pitch*y,1, pitch,fd);
			char *p = buffer + pitch*y;
			for(x=0;x<bi.biWidth;x++) {
				int t = p[0]; p[0] = p[2]; p[2] = t;
				p+=3;
			}
		}
	}

	return image;
}

IMAGE* load_bmp_buffer(unsigned char *buff)
{
	int offset=0;
	WORD bfType;
	struct {
		DWORD	bfSize;
		WORD	bfReserved1;
		WORD	bfReserved2;
		DWORD	bfOffBits;
	} bf;

	RGBQUAD rgbquad[256];

	BITMAPINFOHEADER bi;
	//fread(&bfType,1,sizeof(bfType),fd);
	memcpy(&bfType,buff+offset,sizeof(bfType));
	offset+=sizeof(bfType);
	
	if (bfType != 'M'*256+'B') return NULL;
	//fread(&bf,1,sizeof(bf),fd);
	memcpy(&bf,buff+offset,sizeof(bf));
	offset+=sizeof(bf);
	
	//fread(&bi,1,sizeof(bi),fd);
	memcpy(&bi,buff+offset,sizeof(bi));
	offset+=sizeof(bi);
	
	if (bi.biCompression!=0 || bi.biWidth > 480 || bi.biHeight >272)
		return NULL;
	
	IMAGE *image = image_alloc(bi.biWidth,bi.biHeight,bi.biBitCount);
	int pitch = ((bi.biWidth * bi.biBitCount + 31)/8)&-4;
	//char *buffer = malloc(bi.biSizeImage);
	//image->pixels = buffer;
	char *buffer = image->pixels;

	if (bi.biBitCount<=8) {
		int ncolor = bi.biClrUsed;
		if (ncolor==0) ncolor = 1<< bi.biBitCount;
		//fread(rgbquad,1,sizeof(RGBQUAD)*ncolor,fd);
		memcpy(rgbquad,buff+offset,sizeof(RGBQUAD)*ncolor);
		offset+=sizeof(RGBQUAD)*ncolor;
		int i;
		for(i=0;i<ncolor;i++) {
			image->palette[i].r = rgbquad[i].rgbRed;
			image->palette[i].g = rgbquad[i].rgbGreen;
			image->palette[i].b = rgbquad[i].rgbBlue;
		}

	}

	//fseek(fd,bf.bfOffBits,SEEK_SET);
	offset=bf.bfOffBits;
	
	int x,y;
	switch(bi.biBitCount) {
	case 8:
		for(y=bi.biHeight-1;y>=0;y--) {
			//fread(buffer + pitch*y,1, pitch,fd);
			memcpy(buffer + pitch*y,buff+offset,pitch);
			offset+=pitch;
		}
		break;

	case 24:
		for(y=bi.biHeight-1;y>=0;y--) {
			//fread(buffer + pitch*y,1, pitch,fd);
			memcpy(buffer + pitch*y,buff+offset,pitch);
			offset+=pitch;
			
			char *p = buffer + pitch*y;
			for(x=0;x<bi.biWidth;x++) {
				int t = p[0]; p[0] = p[2]; p[2] = t;
				p+=3;
			}
		}
	}

	return image;
}
