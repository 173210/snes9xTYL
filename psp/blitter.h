#ifndef _blit_h_
#define _blit_h_



void blit_init(void);
void blit_reinit(void);

static inline void blit_shutdown()
{
	sceGuTerm();
}

void guDrawBuffer(u16* video_buffer,int src_w,int src_h,int src_pitch,int dst_w,int dst_h);
void guClear(int dst_w,int dst_h);



#endif
