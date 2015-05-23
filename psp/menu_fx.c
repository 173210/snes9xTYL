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
	ScePspFVector3 position;
};

#define GRID_WIDTH 12 //18
#define GRID_HEIGHT 12 //18

float __attribute__((aligned(64))) parametersSH[8] = {0,2,2,2,2,2,2,2};

static float hue2rgb(float m1, float m2, float h)
{
	h += h < 0.0f ? 1.0f : h > 1.0f ? -1.0f : 0.0f;

	if ((h*6.0f) < 1.0f)
		return m1 + (m2-m1) * h * 6.0f;

	if ((h*2.0f) < 1.0f)
		return m2;

	if ((h*3.0f) < 2.0f)
		return m1 + (m2-m1) * ((2.0f/3.0f)-h) * 6.0f;

	return m1;
}

static unsigned short hsl2rgb(float h, float s, float l)
{
	float m1,m2;
	int r,g,b;

	m2 = l < 0.5f ? l * (s+1.0f) : (l+s) - (l*s);
	m1 = l*2.0f-m2;

	r = (int)(31.0f * hue2rgb(m1,m2,h+(1.0f/3.0f)));
	g = (int)(31.0f * hue2rgb(m1,m2,h));
	b = (int)(31.0f * hue2rgb(m1,m2,h-(1.0f/3.0f)));

	return (0x1 << 15) | (b << 10) | (g << 5) | r;
}

static void evalSH(const float *m, struct Vertex3D *vtx)
{
	const float du = 4.0f / (GRID_WIDTH-1);//(GU_PI*2) / (GRID_WIDTH-1);
	const float dv = 2.0f / (GRID_WIDTH-1);//GU_PI / (GRID_WIDTH-1);
	int i, j;

	__asm__ volatile (
		"lv.q	C000, 0(%0)\n"	//m0,1,2,3 in C000
		"lv.q	C010, 16(%0)\n"	//m4,5,6,7 in C010
		"mtv	%1, S020\n"
		"mtv	%2, S021\n"
		"vzero.s	S022\n"	//theta in S022
		:: "r"(m), "r"(du), "r"(dv));

	for (i = 0; i < GRID_WIDTH; i++) {
		__asm__ volatile (
			"vadd.s	S022, S022, S020\n"
			"vzero.s	S023\n");	//phi in S023

		for (j = 0; j < GRID_HEIGHT; ++j) {
			__asm__ volatile (
				"vscl.p	C030, C010, S023\n"	//S030=m[0]*phi, S031=m[1]*phi
				"vscl.p	C032, C012, S022\n"	//S032=m[2]*theta, S033=m[3]*theta

				"vsin.s	S030, S030\n"	//S030 = sin(m[0]*phi)
				"vcos.s	S031, S031\n"	//S031 = cos(m[1]*phi)
				"vsin.s	S032, S032\n"	//S032 = sin(m[2]*theta)
				"vcos.s	S033, S033\n"	//S033 = cos(m[3]*theta)

				"vabs.q	C030, C030\n"
				"vlog2.q	C030, C030\n"
				"vmul.q	C030, C030, C010\n"
				"vexp2.q	C030, C030\n"
				"vadd.p	C032, C030, C032\n"
				"vadd.s	S100, S032, S033\n" // S100 = r

				"vcos.p	C030, C022\n"
				"vsin.p	C032, C022\n"

				"vmul.s	S030, S030, S013\n"
				"vmul.s	S032, S032, S013\n"

				"vscl.t	C030, C030, S100\n"

				"mfv	$a0, S030\n"
				"sw	$a0, 0 + %[p]\n"
				"mfv	$a0, S031\n"
				"sw	$a0, 4 + %[p]\n"
				"mfv	$a0, S032\n"
				"sw	$a0, 8 + %[p]\n"

				"vadd.s	S023, S023, S021\n"
				: [p] "+m"(vtx->position) :: "a0");
				vtx++;
		}
	}
}

static struct Vertex3D __attribute__((aligned(64))) vertices3D_[GRID_WIDTH * GRID_HEIGHT];
static unsigned short __attribute__((aligned(64))) indices_[GRID_WIDTH * GRID_HEIGHT * 6];

static struct Vertex3D *vertices3D;
static unsigned short *indices;


static void indicesSH()
{
	unsigned short i,j;

	for (i = 0; i < GRID_WIDTH; ++i)
	{
		for (j = 0; j < GRID_HEIGHT; ++j)
		{
			unsigned short* curr = &indices[(j + (i * (GRID_HEIGHT))) * 6];
			unsigned short i1 = (i+1) % GRID_WIDTH;
			unsigned short j1 = (j+1) % GRID_HEIGHT;

			*curr++ = j + i * GRID_HEIGHT;
			*curr++ = j1 + i * GRID_HEIGHT;
			*curr++ = j + i1 * GRID_HEIGHT;

			*curr++ = j1 + i * GRID_HEIGHT;
			*curr++ = j1 + i1 * GRID_HEIGHT;
			*curr++ = j + i1 * GRID_HEIGHT;
		}
	}

	//sceKernelDcacheWritebackAll();
}

static void setupSH()
{
	struct Vertex3D* vtx = vertices3D;
	struct Vertex3D* currvtx = vtx;

	float dh = 1.0f / GRID_WIDTH;

	unsigned int i,j;

	for (i = 0; i < GRID_WIDTH; ++i)
	{
		for (j = 0; j < GRID_HEIGHT; ++j)
		{
			currvtx->color = hsl2rgb(i * dh, 1.0f, 0.5f);
			currvtx++;
		}
	}

}

static void vfpu_gumCrossProductNormalize(ScePspFVector3* r, const ScePspFVector3* a, const ScePspFVector3* b) {
	__asm__ volatile (
		"lv.s		S010, 0+%[a]\n"
		"lv.s		S011, 4+%[a]\n"
		"lv.s		S012, 8+%[a]\n"
		"lv.s		S020, 0+%[b]\n"
		"lv.s		S021, 4+%[b]\n"
		"lv.s		S022, 8+%[b]\n"
		"vcrsp.t 	C000,C010,C020\n"
		"vdot.q		S010, C000, C000\n"
		"vzero.s	S011\n"
		"vcmp.s		EZ,S010\n"
		"vrsq.s		s010, s010\n"			
		"vcmovt.s	s010, s011, 0\n"		
		"vscl.q		C000, C000, S010\n"
		"sv.s		S000, 0 + %[r]\n"
		"sv.s		S001, 4 + %[r]\n"
		"sv.s		S002, 8 + %[r]\n"	
	: [r] "+m"(*r) : [a] "m"(*a),[b] "m"(*b));
	/*r->x = (a->y * b->z) - (a->z * b->y);
	r->y = (a->z * b->x) - (a->x * b->z);
	r->z = (a->x * b->y) - (a->y * b->x);*/
}

/*void vfpu_gumNormalize(ScePspFVector3* v)
{
	float l = sqrtf((v->x*v->x) + (v->y*v->y) + (v->z*v->z));
	if (l > GUM_EPSILON)
	{
		float il = 1.0f / l;
		v->x *= il; v->y *= il; v->z *= il;
	}
}*/

static void renderSH(const float* m)
{
	struct Vertex3D* vtx = vertices3D;
	

	unsigned int i,j;

	// position
	evalSH(m, vtx);

	// normal

	for (i = 0; i < GRID_WIDTH; ++i)
	{
		for (j = 0; j < GRID_HEIGHT; ++j)
		{
			ScePspFVector3 __attribute__((aligned(64))) l1,l2;
			unsigned short* curr = &indices[(j + (i * (GRID_HEIGHT))) * 6];
			ScePspFVector3* normal = &vtx[curr[0]].normal;

			l1.x = vtx[curr[1]].position.x - vtx[curr[0]].position.x;
			l1.y = vtx[curr[1]].position.y - vtx[curr[0]].position.y;
			l1.z = vtx[curr[1]].position.z - vtx[curr[0]].position.z;

			l2.x = vtx[curr[2]].position.x - vtx[curr[0]].position.x;
			l2.y = vtx[curr[2]].position.y - vtx[curr[0]].position.y;
			l2.z = vtx[curr[2]].position.z - vtx[curr[0]].position.z;

			//gumCrossProduct(normal,&l1,&l2);
			//gumNormalize(normal);
			vfpu_gumCrossProductNormalize(normal,&l1,&l2);
		}
	}

	//sceKernelDcacheWritebackAll();

	sceGumDrawSpline(GU_NORMAL_32BITF|GU_COLOR_5551|GU_VERTEX_32BITF,
			GRID_HEIGHT,GRID_WIDTH,3,3,indices,vertices3D);
}

static struct LightSettings
{
	ScePspFVector3 position;
	unsigned int diffuse;
	unsigned int specular;
} lsettings[3] =
{
	{{-1,-1,-1}, 0xffffffff, 0xffffffff},	// key
	{{1,0,-1}, 0xff202020, 0xff000000},	// fill
	{{0,-1,1}, 0xff808080, 0xff000000}	// back
};




void fx_init() {		
	int i;
	struct timeval now;
	sceKernelLibcGettimeofday( &now, 0 );
	val=(now.tv_usec+now.tv_sec*1000000)>>16;	
	srand(val);
	for (i = 0; i < 8; ++i)
				parametersSH[i] = (float)((rand() / ((float)RAND_MAX)) * 6.0f);
	
	vertices3D=(struct Vertex3D *)( ((int)vertices3D_)|0x40000000);
	indices=(unsigned short *)( ((int)indices_)|0x40000000);
	
	// setup GU
	sceGuStart(GU_DIRECT,list);
	//sceGuDrawBuffer(GU_PSM_5551,(void*)0,512);
	//sceGuDispBuffer(480,272,(void*)FRAMESIZE,512);
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
//	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_CLIP_PLANES);

	sceGuFinish();
	sceGuSync(0,0);	

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
	
	setupSH();
	indicesSH();
}

void fx_main(void *drawbuffer) {
	// run sample
	static int cpt=0;
	struct timeval now;
	sceKernelLibcGettimeofday( &now, 0 );
	val=(now.tv_usec+now.tv_sec*1000000)>>16;
	
	sceGuStart(GU_DIRECT,list);
	
	//set draw buffer
	sceGuDrawBuffer(GU_PSM_5551,(void*)drawbuffer,512);

		// clear depth buffer		
		sceGuClearDepth(0);
		sceGuClear(GU_DEPTH_BUFFER_BIT);

		// light settings

		sceGuEnable(GU_LIGHTING);
		sceGuEnable(GU_LIGHT0);
		sceGuEnable(GU_LIGHT1);
		sceGuEnable(GU_LIGHT2);
		sceGuLightMode(1);
		{
			unsigned int i;

			for (i = 0; i < 3; ++i)
			{
				sceGuLight(i,GU_DIRECTIONAL,GU_DIFFUSE_AND_SPECULAR,&lsettings[i].position);
				sceGuLightColor(i,GU_DIFFUSE,lsettings[i].diffuse);
				sceGuLightColor(i,GU_SPECULAR,lsettings[i].specular);
				sceGuLightAtt(i,0.0f,1.0f,0.0f);
			}
			sceGuSpecular(12.0f);
			sceGuAmbient(0x000000);
		}

		// setup matrices

		sceGumMatrixMode(GU_PROJECTION);
		sceGumLoadIdentity();
		sceGumPerspective(75.0f,16.0f/9.0f,0.5f,1000.0f);

		sceGumMatrixMode(GU_VIEW);
		{
			ScePspFVector3 pos = { 0, 0, -5.0f };

			sceGumLoadIdentity();
			sceGumTranslate(&pos);
		}

		sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity();
		{
			ScePspFVector3 rot = { val * 0.79f * (GU_PI/240.0f), val * 0.98f * (GU_PI/240.0f), val * 1.32f * (GU_PI/240.0f) };
			sceGumRotateXYZ(&rot);
		}

		// setup texture

		sceGuAmbientColor(0xffffffff);

		// draw

		renderSH(parametersSH);

		sceGuFinish();
		sceGuSync(0,0);

//		sceDisplayWaitVblankStart();
//		sceGuSwapBuffers();

//		val++;

		if ((cpt++>600))
		{			
			unsigned int i;
			cpt=0;
			for (i = 0; i < 8; ++i)
				parametersSH[i] = (float)((rand() / ((float)RAND_MAX)) * 6.0f);
		}
	
}

void fx_close() {
	sceGuDisable(GU_LIGHTING);
	sceGuDisable(GU_LIGHT0);
	sceGuDisable(GU_LIGHT1);
	sceGuDisable(GU_LIGHT2);
	sceGuDisable(GU_CLIP_PLANES);
}

