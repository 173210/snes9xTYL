
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

#if 0
int save_bmp(const char *file,int width,int height,int bit,void *bits,int pitch)
{
	WORD bfType;
	struct {
		DWORD	bfSize;
		WORD	bfReserved1;
		WORD	bfReserved2;
		DWORD	bfOffBits;
	} bf;

	BITMAPINFOHEADER bi;
	
	int fd = open(file,O_WRONLY|O_BINARY|O_CREAT,_S_IREAD|_S_IWRITE);
	if (fd==-1) return -1;

	bf.bfSize = 
	bfType='M'*256+'B';
	bf.bfSize = 14+sizeof(BITMAPINFOHEADER)+width*height*2;
	bf.bfOffBits = 14+sizeof(BITMAPINFOHEADER);
	bf.bfReserved1 = 0;
	bf.bfReserved1 = 0;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = height;
	bi.biPlanes = 1;
	bi.biBitCount=bit; //16
	bi.biCompression = 0; //BI_RGB
	bi.biSizeImage = width*height*2;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	write(fd,&bfType,sizeof(bfType));
	write(fd,&bf,sizeof(bf));
	write(fd,&bi,sizeof(bi));

	char *src = (char*)bits + pitch * height;
	int y;
	for(y=height-1;y>=0;--y) {
		src -= pitch;
		write(fd,src,width*2);
	}
	close(fd);
	return 0;
}
#endif

IMAGE* image_alloc(int width,int height,int bit)
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


#if 0
static void png_read_data(png_structp ctx, png_bytep area, png_size_t size)
{
	int fd = (int)png_get_io_ptr(ctx);
	read(fd, area, size);
}





IMAGE* load_png(int fd)
{
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	png_bytep * row_pointers;

	int ckey = -1;
	png_color_16 *transv;

	IMAGE* image = NULL;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
					  NULL,NULL,NULL);
	if (png_ptr == NULL) goto err;
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) goto err;

	if ( setjmp(png_ptr->jmpbuf)) goto err;
	
	/* Set up the input control */
	png_set_read_fn(png_ptr, (void*)fd, png_read_data);

	/* Read PNG header info */
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
			&color_type, &interlace_type, NULL, NULL);

	/* tell libpng to strip 16 bit/color files down to 8 bits/color */
	png_set_strip_16(png_ptr) ;

	/* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
	 * byte into separate bytes (useful for paletted and grayscale images).
	 */
	png_set_packing(png_ptr);

	/* scale greyscale values to the range 0..255 */
	if(color_type == PNG_COLOR_TYPE_GRAY)
		png_set_expand(png_ptr);

	/* For images with a single "transparent colour", set colour key;
	   if more than one index has transparency, or if partially transparent
	   entries exist, use full alpha channel */
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
	        int num_trans;
		unsigned char *trans;
		png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans,
			     &transv);
		if(color_type == PNG_COLOR_TYPE_PALETTE) {
		    /* Check if all tRNS entries are opaque except one */
		    int i, t = -1;
		    for(i = 0; i < num_trans; i++)
			if(trans[i] == 0) {
			    if(t >= 0)
				break;
			    t = i;
			} else if(trans[i] != 255)
			    break;
		    if(i == num_trans) {
			/* exactly one transparent index */
			ckey = t;
		    } else {
			/* more than one transparent index, or translucency */
			png_set_expand(png_ptr);
		    }
		} else
		    ckey = 0; /* actual value will be set later */
	}

	if ( color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
		png_set_gray_to_rgb(png_ptr);

	png_read_update_info(png_ptr, info_ptr);

//	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
//			&color_type, &interlace_type, NULL, NULL);

	image = image_alloc(width,height,bit_depth*info_ptr->channels);

	row_pointers = (png_bytep*) alloca(sizeof(png_bytep)*height);

	char* p = image->pixels;
	int pitch = (width*bit_depth*info_ptr->channels+7)/8;
	int i;

	for (i = 0; i < height; i++) {
		row_pointers[i] = p; p+=pitch;
	}

	/* Read the entire image in one go */
	png_read_image(png_ptr, row_pointers);

	if (info_ptr->num_palette > 0) {
		COLOR *palette = image->palette;
		image->n_palette = info_ptr->num_palette;
		for(i=0; i<info_ptr->num_palette; i++) {
			palette[i].b = info_ptr->palette[i].blue;
			palette[i].g = info_ptr->palette[i].red;
			palette[i].r = info_ptr->palette[i].green;
		}
	}
err:
	png_destroy_read_struct(&png_ptr, info_ptr ? &info_ptr : (png_infopp)0,
								(png_infopp)0);

	return image;
}

#endif
