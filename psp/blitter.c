#include "psp.h"

#include "tile_psp.h"

/*extern "C" */long pg_drawframe;

extern int os9x_vsync,swap_buf,os9x_smoothing,os9x_snesheight;
extern int os9x_screenLeft,os9x_screenTop,os9x_screenHeight,os9x_screenWidth;

#define SLICE_SIZE 64 // change this to experiment with different page-cache sizes
//static unsigned short __attribute__((aligned(16))) pixels[512*272];


void blit_reinit(void){	
	sceGuStart(GU_DIRECT,list);	
	swap_buf=0;
	sceGuDrawBuffer(GU_PSM_5551,(void*)0,512);
	sceGuDispBuffer(480,272,(void*)FRAMESIZE,512);	
	sceGuOffset(2048 - (480/2),2048 - (272/2));
	sceGuViewport(2048,2048,480,272);
	
	sceGuDepthBuffer((void*)(FRAMESIZE*2),256);
	
	
	
	sceGuEnable(GU_DEPTH_TEST);
	sceGuDepthRange(256,0);
	sceGuDepthOffset(0);		
	sceGuDepthFunc(GU_GREATER);
	
	sceGuFinish();
	sceGuSync(0,0);
		
	
	sceGuStart(GU_DIRECT,list);				
	
	sceGuEnable(GU_SCISSOR_TEST);	
	sceGuScissor(0,0,256,240);
	
	sceGuEnable(GU_TEXTURE_2D);
	sceGuTexMode(GU_PSM_T8,0,0,0); // 8-bit image
	sceGuTexFilter(GU_NEAREST,GU_NEAREST);
		
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
	sceGuTexScale(1.0f/512.0f,1.0f/512.0f);
	sceGuTexOffset(0,0);
	
	//sceGuEnable(GU_ALPHA_TEST);
	//sceGuAlphaFunc(GU_EQUAL,0,0x1);	
		
	sceGuClutMode(GU_PSM_5551,0,255,0);
	
	sceGuClearColor(0);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	
	sceGuFinish();
	sceGuSync(0,0);
	sceDisplayWaitVblankStart();
	
	//cache stuff
	tile_reset_cache();
}

void blit_init(void){
	//cache stuff
	tile_init_cache();
	
	// setup GU	
	sceGuInit();		
	sceGuDisplay(1);
	blit_reinit();
}

void guClear(int dst_w,int dst_h){
	unsigned int cx,cy;
	sceGuStart(GU_DIRECT,list);
	sceGuEnable(GU_SCISSOR_TEST);
	
	#ifdef RELEASE
  cx=(480-dst_w)/2;
  cy=(272-dst_h)/2;
#else
	cx=cy=0;
#endif	    
  sceGuScissor(cx,cy,cx+dst_w,cy+dst_h);
  
  sceGuClearColor(0);
  sceGuClear(GU_COLOR_BUFFER_BIT);
  sceGuFinish();
  sceGuSync(0,0);
}

typedef struct {
    unsigned int* start;
    unsigned int* current;
    int parent_context;
} GuDisplayList;
extern GuDisplayList* gu_list; // guInternal.h
extern int g_cbid;

void guDrawBuffer(u16* video_buffer,int src_w,int src_h,int src_pitch,int dst_w,int dst_h){
  unsigned int j,cx,cy;
  struct Vertex* vertices,*vertices_ptr;
  sceGuStart(GU_DIRECT,list);
  
  /*sceGuOffset(2048 - (480/2),2048 - (272/2));
	sceGuViewport(2048,2048,480,272);*/
    		
	sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGB);
  
  sceGuDrawBufferList(GU_PSM_5551,(void*)(512*272*2*swap_buf),512);
  sceGuEnable(GU_SCISSOR_TEST);
  
#ifdef RELEASE
  cx=(480-dst_w)/2;
  cy=(272-dst_h)/2;
#else
	cx=cy=0;
#endif	    
  sceGuScissor(cx,cy,cx+dst_w,cy+dst_h);
  
      
  sceGuDisable(GU_DEPTH_TEST);
  sceGuDisable(GU_ALPHA_TEST);

  sceGuTexMode(GU_PSM_5551,0,0,0);
  sceGuTexImage(0,src_w,src_w,src_pitch,video_buffer);
  sceGuTexFunc(GU_TFX_REPLACE,0);
  if (os9x_smoothing) sceGuTexFilter(GU_LINEAR,GU_LINEAR);
  else sceGuTexFilter(GU_NEAREST,GU_NEAREST);
  sceGuTexScale(1.0f/src_w,1.0f/src_w); // scale UVs to 0..1
  sceGuTexOffset(0,0);
  sceGuTexWrap(GU_CLAMP,GU_CLAMP);
  
  vertices = (struct Vertex*)sceGuGetMemory(2*(1+src_w/SLICE_SIZE) * sizeof(struct Vertex));    	
  vertices_ptr=vertices;
  	
  for (j = 0; (j+SLICE_SIZE) < src_w; j = j+SLICE_SIZE) {
    
    vertices_ptr[0].u = j; vertices_ptr[0].v = (os9x_snesheight-src_h)/2;        
    vertices_ptr[0].x = cx+j*(dst_w+os9x_screenWidth)/src_w+os9x_screenLeft; vertices_ptr[0].y = cy+0+os9x_screenTop; vertices_ptr[0].z = 0;    
    vertices_ptr[1].u = j+SLICE_SIZE; vertices_ptr[1].v = src_h-(os9x_snesheight-src_h)/2;
    vertices_ptr[1].x = cx+(j+SLICE_SIZE)*(dst_w+os9x_screenWidth)/src_w+os9x_screenLeft; vertices_ptr[1].y = cy+dst_h+os9x_screenHeight+os9x_screenTop; vertices_ptr[1].z = 0;            
    
    vertices_ptr+=2;
  }
  if (j<src_w){  
    vertices_ptr[0].u = j; vertices_ptr[0].v = (os9x_snesheight-src_h)/2;    
    vertices_ptr[0].x = cx+j*(dst_w+os9x_screenWidth)/src_w+os9x_screenLeft; vertices_ptr[0].y = cy+0+os9x_screenTop; vertices_ptr[0].z = 0;
    vertices_ptr[1].u = src_w; vertices_ptr[1].v = src_h-(os9x_snesheight-src_h)/2;
    vertices_ptr[1].x = cx+(dst_w+os9x_screenWidth)+os9x_screenLeft; vertices_ptr[1].y = cy+dst_h+os9x_screenHeight+os9x_screenTop; vertices_ptr[1].z = 0;            
    vertices_ptr+=2;
  }
  sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,vertices_ptr-vertices,0,vertices);
  
      
  sceGuFinish();
  sceGeListEnQueue(gu_list->start,gu_list->current,g_cbid,NULL);
//  sceGuSync(0,0);
//  if (os9x_vsync) sceDisplayWaitVblankStart();		
//  sceGuSwapBuffers();  
//  swap_buf^=1;
//	pg_drawframe=swap_buf^1;
}

void blit_shutdown(){
  sceGuTerm();
}
