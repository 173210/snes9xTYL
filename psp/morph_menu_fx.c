#include "psp.h"

#include <pspge.h>
#include <pspgu.h>
#include <pspgum.h>



#define min( a, b ) ( ((a)<(b))?(a):(b) )
#define max( a, b ) ( ((a)>(b))?(a):(b) )

static int val;

struct Vertex3D
{
	unsigned short color;
	ScePspFVector3 normal;
	ScePspFVector3 pos;
};

struct MorphVertex
{
	struct Vertex3D v0;
	struct Vertex3D v1;
};

#define ROWS (32)
#define COLS (32)

unsigned short __attribute__((aligned(16))) indices[(ROWS+1)*(COLS+1)*6];
struct MorphVertex __attribute__((aligned(16))) verticesFX[ROWS*COLS];

void fx_init() {
	unsigned int i,j;
	// generate sphere & cube that can blend between each other (a bit messy right now)

	for (i = 0; i < ROWS; ++i)
	{
		float di = (((float)i)/ROWS);
		float s = di * GU_PI * 2;
		ScePspFVector3 v = { cosf(s), cosf(s), sinf(s) };

		for(j = 0; j < COLS; ++j)
		{
			unsigned short* curr = &indices[(j+(i*COLS))*6];
			unsigned int i1 = (i+1)%ROWS, j1 = (j+1)%COLS;

			float t = (((float)j)/COLS) * GU_PI * 2;

			ScePspFVector3 v2 = { v.x * cosf(t), v.y * sinf(t), v.z };
			ScePspFVector3 v3;

			// cheap mans sphere -> cube algo :D
			v3.x = v2.x > 0 ? min(v2.x * 5.0f,0.5f) : max(v2.x * 5.0f,-0.5f);
			v3.y = v2.y > 0 ? min(v2.y * 5.0f,0.5f) : max(v2.y * 5.0f,-0.5f);
			v3.z = v2.z > 0 ? min(v2.z * 5.0f,0.5f) : max(v2.z * 5.0f,-0.5f);

			verticesFX[j+i*COLS].v0.color = ((int)(fabsf(v2.x) * 31.0f) << 10)|((int)(fabsf(v2.y) * 31.0f) << 5)|((int)(fabsf(v2.z) * 31.0f));
			verticesFX[j+i*COLS].v0.normal = v2;
			verticesFX[j+i*COLS].v0.pos = v2;

			verticesFX[j+i*COLS].v1.color = verticesFX[j+i*COLS].v0.color;
			verticesFX[j+i*COLS].v1.normal = v3;
			gumNormalize(&verticesFX[j+i*COLS].v1.normal);
			verticesFX[j+i*COLS].v1.pos = v3;

			// indices
			*curr++ = j + i * COLS;
			*curr++ = j1 + i * COLS;
			*curr++ = j + i1 * COLS;

			*curr++ = j1 + i * COLS;
			*curr++ = j1 + i1 * COLS;
			*curr++ = j + i1 * COLS;
		}
	}

	sceKernelDcacheWritebackAll();
	
	// setup GU
	sceGuStart(GU_DIRECT,list);
	sceGuDrawBuffer(GU_PSM_5551,(void*)0,512);
	sceGuDispBuffer(480,272,(void*)FRAMESIZE,512);
	//in order to not lose snes rendering buffer for screen calibration option,
	//put the Zbuffer far away ...
	sceGuDepthBuffer((void*)(FRAMESIZE*4),256);	 
	sceGuOffset(2048 - (480/2),2048 - (272/2));
	sceGuViewport(2048,2048,480,272);
	
	sceGuDepthRange(0xc350,0x2710);
	
	sceGuScissor(0,0,480,272);
	
	sceGuDisable(GU_ALPHA_TEST);
	sceGuDisable(GU_TEXTURE_2D);
	
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_LIGHTING);
	sceGuEnable(GU_LIGHT0);
	sceGuFinish();
	sceGuSync(0,0);	

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
	
	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective(75.0f,16.0f/9.0f,0.5f,1000.0f);

	sceGumMatrixMode(GU_VIEW);
	{
		ScePspFVector3 pos = {0.0f,0.0f,-2.5f};

		sceGumLoadIdentity();
		sceGumTranslate(&pos);
	}		
}

void fx_main(void *drawbuffer) {
	// run sample
	struct timeval now;
	sceKernelLibcGettimeofday( &now, 0 );
	val=(now.tv_usec+now.tv_sec*1000000)>>14;
			
	ScePspFVector3 lpos0 = { 1, 0, 1 };
	
		sceGuStart(GU_DIRECT,list);
		
		
		sceGuDrawBuffer(GU_PSM_5551,(void*)drawbuffer,512);

		

		//sceGuClearColor(0xff554433);
		sceGuClearDepth(0);
		sceGuLight(0,GU_DIRECTIONAL,GU_DIFFUSE_AND_SPECULAR,&lpos0);
		sceGuLightColor(0,GU_DIFFUSE_AND_SPECULAR,0xffffffff);
				
		sceGuClear(/*GU_COLOR_BUFFER_BIT|*/GU_DEPTH_BUFFER_BIT);
		sceGuSpecular(12.0f);
                                                                        

		// rotate morphing mesh

		sceGumMatrixMode(GU_MODEL);
		{
			ScePspFVector3 rot = {val * 0.79f * (GU_PI/200.0f), val * 0.98f * (GU_PI/200.0f), val * 1.32f * (GU_PI/200.0f)};

			sceGumLoadIdentity();
			sceGumRotateXYZ(&rot);
		}

		sceGuAmbientColor(0xffffffff);

		// draw cube

		sceGuMorphWeight(0,0.5f * sinf(val * (GU_PI/180.0f)) + 0.5f);
		sceGuMorphWeight(1,-0.5f * sinf(val * (GU_PI/180.0f)) + 0.5f);
		sceGumDrawArray(GU_TRIANGLES,GU_COLOR_5551|GU_NORMAL_32BITF|GU_VERTEX_32BITF|GU_VERTICES(2)|GU_INDEX_16BIT|GU_TRANSFORM_3D,sizeof(indices)/sizeof(unsigned short),indices,verticesFX);

		sceGuFinish();
		sceGuSync(0,0);
	
}

void fx_close() {
	sceGuDisable(GU_LIGHTING);
	sceGuDisable(GU_LIGHT0);
}

