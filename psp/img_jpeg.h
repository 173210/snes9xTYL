#ifndef __img_jpeg_h__
#define __img_jpeg_h__

int write_JPEG_file (const char * filename, int quality,u16 *image_buffer,int image_width,int image_height,int image_pitch);
int read_JPEG_file (const char * filename,u16 *image_buffer,int image_width,int image_height,int image_pitch,int *height);

#endif
