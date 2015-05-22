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

float hue2rgb(float m1, float m2, float h)
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

unsigned short hsl2rgb(float h, float s, float l)
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


float vfpucos(float angle)
{
	float res;
	__asm__ (		
		"lv.s			s000, %[a]\n"				
		"vcst.s			s001, VFPU_2_PI\n"		
		"vmul.s			s000, s000, s001\n"		
		"vcos.s			s000, s000\n"			
		"sv.s			s000, %[r]\n"				
		: [r]"=m"(res ): [a]"m"(angle)
	);
	return (res);
}
float vfpusin(float angle)
{
	float res;
	__asm__ (		
		"lv.s			s000, %[a]\n"				
		"vcst.s			s001, VFPU_2_PI\n"		
		"vmul.s			s000, s000, s001\n"		
		"vsin.s			s000, s000\n"			
		"sv.s			s000, %[r]\n"				
		: [r]"=m"(res ): [a]"m"(angle)
	);
	return (res);
}
float vfpumulsin(float mul,float angle,float range)
{
	float res;	
	__asm__ (		
		"lv.s			s000, %[a]\n"
		"lv.s			s001, %[mul]\n"
		"lv.s			s002, %[range]\n"
		"vsub.s			s000, s000, s002\n"
		"vdiv.s			s000, s000, s002\n"
		"vsin.s			s003, s000\n"
		"vmul.s			s000, s000, s001\n"
		"sv.s			s000, %[r]\n"
		: [r]"=m"(res ): [mul]"m"(mul),[a]"m"(angle),[range]"m"(range)
	);
	return (res);
}

float vfpupow(float x, float y) {
	float res;
  if (x==0) return 0;
  if (x<0) x=-x;
	__asm__ volatile (	
		"lv.s			s000,%[x]\n"				// s000 = x
		"lv.s			s001,%[y]\n"				// s001 = y
		"vlog2.s	s000, s000\n"			
		"vmul.s		s000, s000, s001\n"		
		"vexp2.s	s000, s000\n"			// s000 = 2^(y*ln2(x)) = 2^(ln2(x^y))=x^y
		"sv.s			s000,%[r]\n"				// d    = s000
		: [r]"=m"(res) : [x]"m"(x), [y]"m"(y)
	);
	return (res);
}


float __attribute__((aligned(64))) mres[4];
void evalSH(float theta, float phi, const float* m, ScePspFVector3* p) {
	float r=0;			
	__asm__ volatile (			
			"lv.q			C010, %[m]\n"   //m0,1,2,3 in C010			
			"lv.s			S001, %[phi]\n"   //phi in S001
			"lv.s			S002, %[theta]\n"   //theta in S002
			"lv.q			C020, %[m2]\n"   //m4,5,6,7 in C020
			"vscl.p 	C010,C010, S001\n" //S010=m[0]*phi, S011=m[1]*phi
			"vscl.p 	C012,C012, S002\n" //S012=m[2]*theta, S013=m[3]*theta
			"vsin.s 	S010,S010\n"	//S010 = sin(m[0]*phi)
			"vcos.s 	S011,S011\n"	//S011 = cos(m[1]*phi)
			"vsin.s 	S012,S012\n"	//S012 = sin(m[2]*theta)
			"vcos.s 	S013,S013\n"	//S013 = cos(m[3]*theta)			
			"sv.q			C010, %[mres]\n"			
		: [mres] "+m"(mres) : [m] "m"(*m), [m2] "m"(*(float *)((uintptr_t)m + 16)), [theta] "m"(theta) , [phi] "m"(phi));
				
	r += vfpupow(mres[0],(float)m[4]);
	r += vfpupow(mres[1],(float)m[5]);
	r += vfpupow(mres[2],(float)m[6]);
	r += vfpupow(mres[3],(float)m[7]);
	
	__asm__ volatile (
			"lv.s			S000, %[phi]\n"   //phi in S001
			"lv.s			S001, %[theta]\n"   //theta in S002
			"lv.s			S002, %[r]\n"   //theta in S002			
			"vcos.s		S010,S001\n" //S010=cos(theta)
			"vcos.s		S011,S000\n" //S011=cos(phi)						
			"vsin.s		S012,S001\n" //S012=sin(theta)
			"vsin.s		S013,S000\n" //S013=sin(phi)
			"vmul.s		S010,S013,S010\n"			
			"vmul.s		S012,S012,S013\n"
			"vscl.q		C010,C010,S002\n"
			"usv.s			S010, 0 + %[p]\n"
			"usv.s			S011, 4 + %[p]\n"
			"usv.s			S012, 8 + %[p]\n"
		: [p] "+m"(*p) : [r] "m"(r) , [theta] "m"(theta) , [phi] "m"(phi));
	
	
	/*r += powf(sinf(m[0]*phi),(float)m[4]);
	r += powf(cosf(m[1]*phi),(float)m[5]);
	r += powf(sinf(m[2]*theta),(float)m[6]);
	r += powf(cosf(m[3]*theta),(float)m[7]);
			
	p->x = r2 * sinf(phi) * cosf(theta);
	p->y = r2 * cosf(phi);
	p->z = r2 * sinf(phi) * sinf(theta);*/
}

struct Vertex3D __attribute__((aligned(64))) vertices3D_[GRID_WIDTH * GRID_HEIGHT];
unsigned short __attribute__((aligned(64))) indices_[GRID_WIDTH * GRID_HEIGHT * 6];

struct Vertex3D *vertices3D;
unsigned short *indices;


void indicesSH()
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

void setupSH()
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

void vfpu_gumCrossProductNormalize(ScePspFVector3* r, const ScePspFVector3* a, const ScePspFVector3* b) {
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

float __attribute__((aligned(64))) u;
float __attribute__((aligned(64))) v;
void renderSH(const float* m)
{
	struct Vertex3D* vtx = vertices3D;
	struct Vertex3D* currvtx = vtx;

	float du = 4.0f / (GRID_WIDTH-1);//(GU_PI*2) / (GRID_WIDTH-1);
	float dv = 2.0f / (GRID_HEIGHT-1);//GU_PI / (GRID_HEIGHT-1);	
	

	unsigned int i,j;

	// position

  u=0;
	for (i = 0; i < GRID_WIDTH; ++i)
	{
		//u = fmod(i * du,GU_PI*2);
		u += du;

		v=0;
		for (j = 0; j < GRID_HEIGHT; ++j)
		{
			v += dv;//(j * dv);

			evalSH(u,v,m,&(currvtx->position));
			currvtx++;
		}
	}

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

struct LightSettings
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

