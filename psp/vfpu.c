/*
 * LuaPlayer Euphoria
 * ------------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE for details.
 *
 * Copyright (c) 2005 Frank Buss <fb@frank-buss.de> (aka Shine)
 * Copyright (c) 2009 Danny Glover <danny86@live.ie> (aka Zack) 
 *
 * Official Forum : http://www.retroemu.com/forum/forumdisplay.php?f=148
 * For help using LuaPlayer, code help, tutorials etc please visit the official site : http://www.retroemu.com/forum/forumdisplay.php?f=148
 *
 * Credits:
 * 
 * (from Shine/Zack) 
 *
 *   many thanks to the authors of the PSPSDK from http://forums.ps2dev.org
 *   and to the hints and discussions from #pspdev on freenode.net
 *
 * (from Zack Only)
 *
 * Thanks to Brunni for Oslib. Some features/functions were ported from oslib to luaPlayers library. 
 * Thanks to Arshia001 for AALIB. It is the sound engine used in LuaPlayer Euphoria. 
 * Thanks to HardHat for being a supportive friend and advisor.
 * Thanks to Jono for the moveToVram code + His md2 Model : Loading & Animation code.
 * Thanks to Osgeld, Dan369 & Cmbeke for testing LuaPlayer Euphoria for me and coming up with some neat ideas for it.
 * Thanks to the entire LuaPlayer Euphoria userbase, for using it and for supporting it's development. You guys rock!
 *
 * And finally thanks to myself for making this all work in lua and not throwing in the towel :)
 *
 */
 
#include "pspmath.h"

void printMatrixFloat(int matid) {
	float m[16];

	#define SV(N)					\
		asm("usv.q	R"#N"00,  0 + %0\n"	\
		    "usv.q	R"#N"01, 16 + %0\n"	\
		    "usv.q	R"#N"02, 32 + %0\n"	\
		    "usv.q	R"#N"03, 48 + %0\n"	\
		    : "=m"(m))

	switch (matid) {
		case 0:		SV(0); break;
		case 1:		SV(1); break;
		case 2:		SV(2); break;
		case 3:		SV(3); break;
		case 4:		SV(4); break;
		case 5:		SV(5); break;
		case 6:		SV(6); break;
		case 7:		SV(7); break;
	}

	printf("\n\n");
	printf("      C%d00    C%d10    C%d20    C%d30\n", matid, matid, matid, matid);
	printf("R%d00: %0.6f %0.6f %0.6f %0.6f\n", matid, m[0], m[1], m[2], m[3]);
	printf("R%d01: %0.6f %0.6f %0.6f %0.6f\n", matid, m[4], m[5], m[6], m[7]);
	printf("R%d02: %0.6f %0.6f %0.6f %0.6f\n", matid, m[8], m[9], m[10], m[11]);
	printf("R%d03: %0.6f %0.6f %0.6f %0.6f\n", matid, m[12], m[13], m[14], m[15]);
}

//Below function taken from PGE
float vfpu_round(float x)
{
	float result;

	__asm__ volatile (
		"mtv      %1, S000\n"
		"vf2in.s  S000, S000, 0\n"
		"vi2f.s	  S000, S000, 0\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x));
	
	return result;
}


//Below Function taken from PGE - Phoenix Game Engine - Greets IWN!
float vfpu_abs(float x) {
    float result;

	__asm__ volatile (
		"mtv      %1, S000\n"
		"vabs.s   S000, S000\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x));

	return result;
}

float vfpu_acosf(float x) {
    float result;
    __asm__ volatile (
        "mtv     %1, S000\n"
        "vcst.s  S001, VFPU_PI_2\n"
        "vasin.s S000, S000\n"
        "vocp.s  S000, S000\n"
        "vmul.s  S000, S000, S001\n"
        "mfv     %0, S000\n"
        : "=r"(result) : "r"(x));
    return result;
}

void vfpu_add_vector(ScePspFVector4 *vout, ScePspFVector4 *va, ScePspFVector4 *vb) {
   __asm__ volatile (
       "lv.q    C000, %1\n"
       "lv.q    C010, %2\n"
       "vadd.t  C020, C000, C010\n"
       "sv.q    C020, %0\n"
       : "+m"(*vout): "m"(*va), "m"(*vb));
}

float vfpu_asinf(float x) {
    float result;
    __asm__ volatile (
        "mtv     %1, S000\n"
        "vcst.s  S001, VFPU_PI_2\n"
        "vasin.s S000, S000\n"
        "vmul.s  S000, S000, S001\n"
        "mfv     %0, S000\n"
        : "=r"(result) : "r"(x));
    return result;
}

float vfpu_atanf(float x) {
	float result;
	// result = asinf(x/sqrt(x*x+1))
	__asm__ volatile (
		"mtv      %1, S000\n"
		"vmul.s   S001, S000, S000\n"
		"vadd.s   S001, S001, S001[1]\n"
		"vrsq.s   S001, S001\n"
		"vmul.s   S000, S000, S001\n"
		"vasin.s  S000, S000\n"
		"vcst.s   S001, VFPU_PI_2\n"
		"vmul.s   S000, S000, S001\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x));
	return result;
}

#define PI   3.14159265358979f
#define PI_2 1.57079632679489f

inline static float fabsf(float x) {
	float r;
	__asm__ volatile( "abs.s %0, %1" : "=f"(r) :"f"(x):"memory");
	return r;
}

float vfpu_atan2f(float y, float x) {
	float r;

	if (fabsf(x) >= fabsf(y)) {
		r = vfpu_atanf(y/x);
		if   (x < 0.0f) r += (y>=0.0f ? PI : -PI);
	} else {
		r = -vfpu_atanf(x/y);
		r += (y < 0.0f ? -PI_2 : PI_2);
	}
	return r;
}

//Below Function taken from PGE - Phoenix Game Engine - Greets IWN!
float vfpu_ceil(float x) {
    float result;
	
	__asm__ volatile (
		"mtv      %1, S000\n"
		"vf2iu.s  S000, S000, 0\n"
		"vi2f.s	  S000, S000, 0\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x));
	
	return result;
}

float vfpu_cosf(float rad) {
    float result;
    __asm__ volatile (
        "mtv     %1, S000\n"
        "vcst.s  S001, VFPU_2_PI\n"
        "vmul.s  S000, S000, S001\n"
        "vcos.s  S000, S000\n"
        "mfv     %0, S000\n"
        : "=r"(result) : "r"(rad));
    return result;
}

float vfpu_coshf(float x) {
	float result;
	__asm__ volatile (
		"mtv      %1, S000\n"
		"vcst.s   S001, VFPU_LN2\n"
		"vrcp.s   S001, S001\n"
		"vmov.s   S002, S000[|x|]\n"
        "vmul.s   S002, S001, S002\n"
        "vexp2.s  S002, S002\n"
        "vrcp.s   S003, S002\n"
        "vadd.s   S002, S002, S003\n"
        "vmul.s   S002, S002, S002[1/2]\n"
        "mfv      %0, S002\n"
	: "=r"(result) : "r"(x));
	return result;
}

float vfpu_ease_in_out(float t) {
	float r;
	__asm__ volatile (
		"mtv     %1, S000\n"
		"vmov.q  C000[0:1,,,], C000[x, 2, 1, 1/2]\n"	// C000 = [0<t<1, 2, 1, 1/2]
		"vmul.s  S000, S000, S001\n"				// raise S000 to 0..2 range
		"vsub.s  S000, S000, S002\n"				// S000 now in -1 to 1 range
		"vsin.s  S000, S000\n"						// S000 = sin(S000)
		"vmul.s  S000, S000, S003\n"				// S000 = sin(t)/2
		"vadd.s  S000, S000, S003\n"				// S000 = 0.5 + sin(t)/2
		"mfv     %0, S000\n"
	: "=r"(r) : "r"(t));
	return r;
}

void vfpu_envmap_matrix(ScePspFVector4 *envmat, float r) {
	__asm__ volatile (
		"mtv      %1, S000\n"
		"vcst.s   S001, VFPU_2_PI\n"
		"vmul.s   S000, S000, S001\n"
		"vrot.q   C010, S000, [c, s, 0, 0]\n"
		"vrot.q   C020, S000, [-s, c, 0, 0]\n"
		"sv.q     C010, 0  + %0\n"
		"sv.q     C020, 16 + %0\n"
	:"=m"(*envmat):"r"(r));
}

float vfpu_expf(float x) {
    float result;
    __asm__ volatile (
        "mtv     %1, S000\n"
        "vcst.s  S001, VFPU_LN2\n"
        "vrcp.s  S001, S001\n"
        "vmul.s  S000, S000, S001\n"
        "vexp2.s S000, S000\n"
        "mfv     %0, S000\n"
        : "=r"(result) : "r"(x));
    return result;
}

float vfpu_fabsf(float x) {
	float result;
	__asm__ volatile (
		"mtv     %1, S000\n"
		"vmov.s  S000, S000[|x|]\n"
		"mfv     %0, S000\n"
	: "=r"(result) : "r"(x));
	return result;
}

//Blow Function taken from PGE - Phoenix Game Engine - Greets IWN!
float vfpu_floor(float x) {
    float result;
	
	__asm__ volatile (
		"mtv      %1, S000\n"
		"vf2id.s  S000, S000, 0\n"
		"vi2f.s	  S000, S000, 0\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x));
	
	return result;
}

float vfpu_fmaxf(float x, float y) {
	float result;
	__asm__ volatile (
		"mtv      %1, S000\n"
		"mtv      %2, S001\n"
		"vmax.s   S002, S000, S001\n"
		"mfv      %0, S002\n"
	: "=r"(result) : "r"(x), "r"(y));
	return result;
}

float vfpu_fminf(float x, float y) {
	float result;
	__asm__ volatile (
		"mtv      %1, S000\n"
		"mtv      %2, S001\n"
		"vmin.s   S002, S000, S001\n"
		"mfv      %0, S002\n"
	: "=r"(result) : "r"(x), "r"(y));
	return result;
}

float vfpu_fmodf(float x, float y) {
	float result;
	// return x-y*((int)(x/y));
	__asm__ volatile (
		"mtv       %2, S001\n"
		"mtv       %1, S000\n"
		"vrcp.s    S002, S001\n"
		"vmul.s    S003, S000, S002\n"
		"vf2iz.s   S002, S003, 0\n"
		"vi2f.s    S003, S002, 0\n"
		"vmul.s    S003, S003, S001\n"
		"vsub.s    S000, S000, S003\n"
		"mfv       %0, S000\n"
	: "=r"(result) : "r"(x), "r"(y));
	return result;
}

void vfpu_identity_matrix(ScePspFMatrix4 *m) {
	__asm__ volatile (
		"vmidt.q	M000\n"
		"sv.q		C000, 0  + %0\n"
		"sv.q		C010, 16 + %0\n"
		"sv.q		C020, 32 + %0\n"
		"sv.q		C030, 48 + %0\n"
	:"=m"(*m));
}

float vfpu_logf(float x) {
    float result;
    __asm__ volatile (
        "mtv     %1, S000\n"
        "vcst.s  S001, VFPU_LOG2E\n"
        "vrcp.s  S001, S001\n"
        "vlog2.s S000, S000\n"
        "vmul.s  S000, S000, S001\n"
        "mfv     %0, S000\n"
        : "=r"(result) : "r"(x));
    return result;
}

void vfpu_normalize_vector(ScePspFVector4 *v) {
   __asm__ volatile (
       "lv.q   C000, %0\n"
       "vdot.t S010, C000, C000\n"
       "vrsq.s S010, S010\n"
       "vscl.t C000, C000, S010\n"
       "sv.q   C000, %0\n"
       : "+m"(*v));
}

void vfpu_ortho_matrix(ScePspFMatrix4 *m, float left, float right, float bottom, float top, float near, float far) {
	__asm__ volatile (
		"vmidt.q M100\n"						// set M100 to identity
		"mtv     %2, S000\n"					// C000 = [right, ?,      ?,  ]
		"mtv     %4, S001\n"					// C000 = [right, top,    ?,  ]
		"mtv     %6, S002\n"					// C000 = [right, top,    far ]
		"mtv     %1, S010\n"					// C010 = [left,  ?,      ?,  ]
		"mtv     %3, S011\n"					// C010 = [left,  bottom, ?,  ]
		"mtv     %5, S012\n"                	// C010 = [left,  bottom, near]
		"vsub.t  C020, C000, C010\n"			// C020 = [  dx,   dy,   dz]
		"vrcp.t  C020, C020\n"              	// C020 = [1/dx, 1/dy, 1/dz]
		"vmul.s  S100, S100[2], S020\n"     	// S100 = m->x.x = 2.0 / dx
		"vmul.s  S111, S111[2], S021\n"     	// S110 = m->y.y = 2.0 / dy
		"vmul.s  S122, S122[2], S022[-x]\n"		// S122 = m->z.z = -2.0 / dz
		"vsub.t  C130, C000[-x,-y,-z], C010\n"	// C130 = m->w[x, y, z] = [-(right+left), -(top+bottom), -(far+near)]
												// we do vsub here since -(a+b) => (-1*a) + (-1*b) => -a - b
		"vmul.t  C130, C130, C020\n"			// C130 = [-(right+left)/dx, -(top+bottom)/dy, -(far+near)/dz]
		"sv.q    C100, 0  + %0\n"
		"sv.q    C110, 16 + %0\n"
		"sv.q    C120, 32 + %0\n"
		"sv.q    C130, 48 + %0\n"
	:"=m"(*m) : "r"(left), "r"(right), "r"(bottom), "r"(top), "r"(near), "r"(far));
}

void vfpu_transform_vector(ScePspFMatrix4 *m, ScePspFVector4 *vin, ScePspFVector4 *vout) {
	__asm__ volatile (
		"lv.q    C000, 0  + %1\n"
		"lv.q    C010, 16 + %1\n"
		"lv.q    C020, 32 + %1\n"
		"lv.q    C030, 48 + %1\n"
		"lv.q    C100, %2\n"
		"vtfm4.q C110, M000, C100\n"
		"sv.q    C110, %0\n"
	:"+m"(*vout): "m"(*m), "m"(*vin));
}

void vfpu_perspective_matrix(ScePspFMatrix4 *m, float fovy, float aspect, float near, float far) {
	__asm__ volatile (
		"vmzero.q M100\n"					// set M100 to all zeros
		"mtv     %1, S000\n"				// S000 = fovy
		"viim.s  S001, 90\n"				// S002 = 90.0f
		"vrcp.s  S001, S001\n"				// S002 = 1/90
		"vmul.s  S000, S000, S000[1/2]\n"	// S000 = fovy * 0.5 = fovy/2
		"vmul.s  S000, S000, S001\n"		// S000 = (fovy/2)/90
		"vrot.p  C002, S000, [c, s]\n"		// S002 = cos(angle), S003 = sin(angle)
		"vdiv.s  S100, S002, S003\n"		// S100 = m->x.x = cotangent = cos(angle)/sin(angle)
		"mtv     %3, S001\n"				// S001 = near
		"mtv     %4, S002\n"				// S002 = far
		"vsub.s  S003, S001, S002\n"		// S003 = deltaz = near-far
		"vrcp.s  S003, S003\n"				// S003 = 1/deltaz
		"mtv     %2, S000\n"				// S000 = aspect
		"vmov.s  S111, S100\n"				// S111 = m->y.y = cotangent
		"vdiv.s  S100, S100, S000\n"		// S100 = m->x.x = cotangent / aspect
		"vadd.s  S122, S001, S002\n"        // S122 = m->z.z = far + near
		"vmul.s  S122, S122, S003\n"		// S122 = m->z.z = (far+near)/deltaz
		"vmul.s  S132, S001, S002\n"        // S132 = m->w.z = far * near
		"vmul.s  S132, S132, S132[2]\n"     // S132 = m->w.z = 2 * (far*near)
		"vmul.s  S132, S132, S003\n"        // S132 = m->w.z = 2 * (far*near) / deltaz
		"vsub.s   S123, S123, S123[1]\n"	// S123 = m->z.w = -1.0
		"sv.q	 C100, 0  + %0\n"
		"sv.q	 C110, 16 + %0\n"
		"sv.q	 C120, 32 + %0\n"
		"sv.q	 C130, 48 + %0\n"
	:"=m"(*m): "r"(fovy),"r"(aspect),"r"(near),"r"(far));
}

float vfpu_powf(float x, float y) {
	float result;
	// result = exp2f(y * log2f(x));
	__asm__ volatile (
		"mtv      %1, S000\n"
		"mtv      %2, S001\n"
		"vlog2.s  S001, S001\n"
		"vmul.s   S000, S000, S001\n"
		"vexp2.s  S000, S000\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x), "r"(y));
	return result;
}

void vfpu_quaternion_copy(ScePspQuatMatrix *dst, ScePspQuatMatrix *src) {
	__asm__ volatile (
		"lv.q C000, %1\n"
		"sv.q C000, %0\n"
		:"+m"(*dst) : "m"(*src));
}

void vfpu_quaternion_exp(ScePspQuatMatrix *qout, ScePspQuatMatrix *qin) {
	//float r  = sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
	//float et = exp(a[3]);
	//float s  = r>=0.00001f? et*sin(r)/r: 0.f;
	//return quat(s*a[0],s*a[1],s*a[2],et*cos(r));
	__asm__ volatile (
		"lv.q     C000, 0 + %1\n"			// C000 = [x, y, z, w]
		"vdot.t   S010, C000, C000\n"		// S010 = x^2 + y^2 + z^2
		"vsqrt.s  S010, S010\n"				// S010 = r = sqrt(x^2 + y^2 + z^2)
		"vcst.s   S011, VFPU_LN2\n"			// S011 = ln(2)
		"vrcp.s   S011, S011\n"				// S011 = 1/ln(2)
		"vmul.s   S011, S011, S003\n"		// S011 = w*(1/ln(2))
		"vexp2.s  S011, S011\n"				// S011 = et = exp(w)
		"vcst.s   S012, VFPU_2_PI\n"		// S012 = 2/PI
		"vmul.s   S012, S012, S010\n"		// S012 = r * 2/PI
		"vrot.p   R003, S012, [c,s]\n"		// S003 = cos(r), S013 = sin(r)
		"vdiv.s   S013, S013, S010\n"		// S013 = sin(r)/r
		"vscl.p   R003, R003, S011\n"  		// S003 = et * cos(r), S013 = et * sin(r)/r
		"vscl.t   C000, C000, S013\n"		// C000 = [s*x, s*y, s*z, et*cos(r)]
		"sv.q     C000, 0 + %0\n"
	: "=m"(*qout) : "m"(*qin));
}

#define PI_3 3.14159265358979323846f
const float piover180 = PI_3/180.0f;

void vfpu_quaternion_from_euler(ScePspQuatMatrix *res, float x, float y, float z) {
	__asm__ volatile (
		"mtv	%1, S000\n\t"				// S000 = x
		"mtv	%2, S001\n\t"				// S001 = y
		"mtv	%3, S002\n\t"				// S002 = z
		"lv.s   S010, 0(%4)\n"
		"vscl.t C000, C000, S010\n\t"		// x *= pi/180, y *= pi/180, z *= pi/180

		"vfim.s S010, 0.5\n\t"				// S010 = 0.5
		"vscl.t C000, C000, S010\n\t"		// x *= 0.5, y *= 0.5, z *= 0.5
		"vcst.s S010, VFPU_2_PI\n\t"		// load 2/PI into S010, S011 and S012
		"vscl.t C000, C000, S010\n\t"		// x *= 2/PI, y *= 2/pi, z *= pi/2

		"vrot.p C010, S000, [s, c]\n\t"		// S010 = sr, S011 = cr
		"vrot.p C020, S001, [s, c]\n\t"		// S020 = sp, S021 = cp
		"vrot.p C030, S002, [s, c]\n\t"		// S030 = sy, S031 = cy

		// fear the madness of prefixes
		"vmul.q R100, C010[x,y,y,x], C020[y,x,x,y]\n"
		"vmul.q R100, R100,          C030[y,x,y,x]\n"
		"vmul.q R101, C010[y,x,y,x], C020[y,x,y,x]\n"
		"vmul.q R101, R101,          C030[x,y,y,x]\n"
		"vadd.q C000, R100[x,z,0,0], R100[-y,w,0,0]\n"
		"vadd.q C000, C000, R101[0,0,x,z]\n"
		"vadd.q C000, C000, R101[0,0,-y,w]\n"
		"usv.q  C000, %0\n\t"
		:"=m"(*res) : "r"(x), "r"(y), "r"(z), "r"(&piover180));
	vfpu_quaternion_normalize(res);
}

void vfpu_quaternion_hermite_tangent(ScePspQuatMatrix *qout, ScePspQuatMatrix *p1, ScePspQuatMatrix *p2, float bias) {
	__asm__ volatile (
		// load p1 and p2
		"lv.q    C000, 0(%1)\n"
		"lv.q    C010, 0(%2)\n"

		// load bias
		"mtv     %3, S100\n"

		// C020 = C010 - C000
		"vsub.q  C020, C010, C000\n"

		// scale C020 by bias
		"vscl.q  C020, C020, S100\n"

		// store result
		"sv.q    C020, 0(%0)\n"
	:"+r"(qout): "r"(p1), "r"(p2), "r"(bias));
}

void vfpu_quaternion_identity(ScePspQuatMatrix *q) {
	__asm__ volatile (
		"vidt.q C030\n"		// column is important here, we need the w component set to 1
		"sv.q   C030, %0\n"
		: "+m"(*q));
}

void vfpu_quaternion_ln(ScePspQuatMatrix *qout, ScePspQuatMatrix *qin) {
	//float r  = sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
	//float t  = r>0.00001f? atan2(r,a[3])/r: 0.f;
	//return quat(t*a[0],t*a[1],t*a[2],0.5*log(norm(a)));
	float r;
	__asm__ volatile (
		"lv.q     C010, 0 + %1\n"
		"vdot.t   S020, C010, C010\n"		// r = x^2 + y^2 + z^2
		"vsqrt.s  S020, S020\n"				// r = sqrt(x^2 + y^2 + z^2)
		"mfv      %0, S020\n"
	:"=r"(r): "m"(*qin));
	r = vfpu_atan2f(r, qin->w)/r;
	__asm__ volatile (
		"mtv      %1, S021\n"				// t = atan2(r, w) / r
		"vdot.q   S022, C010, C010\n"		// norm = x^2 + y^2 + z^2 + w^2
		"vcst.s   S023, VFPU_LOG2E\n"
        "vrcp.s   S023, S023\n"
        "vlog2.s  S013, S022\n"
        "vmul.s   S013, S013, S023\n"
		"vmul.s   S013, S013, S013[1/2]\n"
        "vscl.t   C010, C010, S021\n"
        "sv.q     C010, 0 + %0\n"
	: "=m"(*qout) : "r"(r));
}

void vfpu_quaternion_multiply(ScePspQuatMatrix *qout, ScePspQuatMatrix *a, ScePspQuatMatrix *b) {
	__asm__ volatile (
		"lv.q    C000, %1\n"			// load quaternion a
		"lv.q    C010, %2\n"			// load quaternion b
		"vqmul.q C020, C000, C010\n"	// C000 = quat a * quat b (quaternion multiply)
		"sv.q    C020, %0\n"			// store result
		: "+m"(*qout) : "m"(*a), "m"(*b));
}

void vfpu_quaternion_normalize(ScePspQuatMatrix *res) {
	__asm__ volatile (
		"lv.q    C000, %0\n"			// load quaternion into C000
		"vdot.q  S010, C000, C000\n"	// S010 = x^2 + y^2 + z^2 + w^2
		"vrsq.s  S010, S010\n"			// S020 = 1.0 / sqrt(S100)
		"vscl.q  C000, C000, S010\n"	// C000 = C000 * S010 (normalized quaternion)
		"sv.q    C000, %0\n"			// store into quaternion result
		: "+m"(*res));
}

// constants for the hermite curve functions
ScePspFMatrix4 hermite = {
	{  2, -3,  0,  1 },
	{ -2,  3,  0,  0 },
	{  1, -2,  1,  0 },
	{  1, -1,  0,  0 }
};

void vfpu_quaternion_sample_hermite(ScePspQuatMatrix *qout, ScePspQuatMatrix *a, ScePspQuatMatrix *b, ScePspQuatMatrix *at, ScePspQuatMatrix *bt, float t) {
	__asm__ volatile (
		// load hermite transform matrix
		"lv.q    C000,  0(%6)\n"
		"lv.q    C010, 16(%6)\n"
		"lv.q    C020, 32(%6)\n"
		"lv.q    C030, 48(%6)\n"

		// load a, b, at, bt
		"lv.q    C100,  0(%1)\n"
		"lv.q    C110,  0(%2)\n"
		"lv.q    C120,  0(%3)\n"
		"lv.q    C130,  0(%4)\n"

		// C200 = [ t^3, t^2, t, 1]
		"mtv     %5, S202\n"
		"vmul.s  S201, S202, S202\n"
		"vmul.s  S200, S202, S201\n"
		"vone.s  S203\n"

		// multiply M000 by C200
		// C000 = [  2*t^3, -3*t^2, 0, 1]
		// C010 = [ -2*t^3,  3*t^2, 0, 0]
		// C020 = [    t^3, -2*t^2, t, 0]
		// C030 = [    t^3,   -t^2, 0, 0]

		"vmul.q  C000, C000, C200\n"
		"vmul.q  C010, C010, C200\n"
		"vmul.q  C020, C020, C200\n"
		"vmul.q  C030, C030, C200\n"

		// sum the terms
		// S210 =  2*t^3 - 3*t^2 + 1
		// S211 = -2*t^3 + 3*t^2
		// S212 =    t^3 - 2*t^2 + t
		// S213 =    t^3 -   t^2

		"vfad.q  S210, C000\n"
		"vfad.q  S211, C010\n"
		"vfad.q  S212, C020\n"
		"vfad.q  S213, C030\n"

		// scale the qaternions with terms
		"vscl.q  C100, C100, S210\n"
		"vscl.q  C110, C110, S211\n"
		"vscl.q  C120, C120, S212\n"
		"vscl.q  C130, C130, S213\n"

		// sum the results
		"vadd.q  C100, C100, C110\n"
		"vadd.q  C100, C100, C120\n"
		"vadd.q  C100, C100, C130\n"

		// and return results
		"sv.q    C100, 0(%0)\n"
	:"+r"(qout): "r"(a), "r"(b), "r"(at), "r"(bt), "r"(t), "r"(&hermite));
}

void vfpu_quaternion_sample_linear(ScePspQuatMatrix *qout, ScePspQuatMatrix *a, ScePspQuatMatrix *b, float t) {
	__asm__ volatile (
		"lv.q     C000, 0 + %1\n"
		"lv.q     C010, 0 + %2\n"
		"mtv      %3, S020\n"
		"vocp.s   S021, S020\n"
		"vscl.q   C000, C000, S021\n"
		"vscl.q   C010, C010, S020\n"
		"vadd.q   C000, C000, C010\n"
		"sv.q     C000, 0 + %0\n"
	: "=m"(*qout) : "m"(*a), "m"(*b), "r"(t));
}

void vfpu_quaternion_to_matrix(ScePspQuatMatrix *q, ScePspFMatrix4 *m) {
	__asm__ volatile (
       "lv.q      C000, %1\n"                               // C000 = [x,  y,  z,  w ]
       "vmul.q    C010, C000, C000\n"                       // C010 = [x2, y2, z2, w2]
       "vcrs.t    C020, C000, C000\n"                       // C020 = [yz, xz, xy ]
       "vmul.q    C030, C000[x,y,z,1], C000[w,w,w,2]\n"	    // C030 = [wx, wy, wz ]

       "vadd.q    C100, C020[0,z,y,0], C030[0,z,-y,0]\n"    // C100 = [0,     xy+wz, xz-wy]
       "vadd.s    S100, S011, S012\n"                       // C100 = [y2+z2, xy+wz, xz-wy]

       "vadd.q    C110, C020[z,0,x,0], C030[-z,0,x,0]\n"    // C110 = [xy-wz, 0,     yz+wx]
       "vadd.s    S111, S010, S012\n"                       // C110 = [xy-wz, x2+z2, yz+wx]

       "vadd.q    C120, C020[y,x,0,0], C030[y,-x,0,0]\n"    // C120 = [xz+wy, yz-wx, 0    ]
       "vadd.s    S122, S010, S011\n"                       // C120 = [xz+wy, yz-wx, x2+y2]

       "vmscl.t   M100, M100, S033\n"                       // C100 = [2*(y2+z2), 2*(xy+wz), 2*(xz-wy)]
                                                            // C110 = [2*(xy-wz), 2*(x2+z2), 2*(yz+wx)]
                                                            // C120 = [2*(xz+wy), 2*(yz-wx), 2*(x2+y2)]

       "vocp.s    S100, S100\n"                             // C100 = [1-2*(y2+z2), 2*(xy+wz),   2*(xz-wy)  ]
       "vocp.s    S111, S111\n"                             // C110 = [2*(xy-wz),   1-2*(x2+z2), 2*(yz+wx)  ]
       "vocp.s    S122, S122\n"                             // C120 = [2*(xz+wy),   2*(yz-wx),   1-2*(x2+y2)]

       "vidt.q    C130\n"                                   // C130 = [0, 0, 0, 1]

       "sv.q      R100, 0  + %0\n"
       "sv.q      R101, 16 + %0\n"
       "sv.q      R102, 32 + %0\n"
       "sv.q      R103, 48 + %0\n"
	: "=m"(*m) : "m"(*q));
}

unsigned int vfpu_rand_8888(int min, int max) {
	unsigned int result;
	__asm__ volatile (
		"mtv      %1, S020\n"
		"mtv      %2, S021\n"
		"vmov.t   C000, C020[x, x, x]\n"
		"vmov.t   C010, C020[y, y, y]\n"
		"vi2f.t   C000, C000, 0\n"
		"vi2f.t   C010, C010, 0\n"
		"vsub.t   C010, C010, C000\n"
		"vrndf1.t C020\n"
		"vsub.t   C020, C020, C020[1, 1, 1]\n"
		"vmul.t   C020, C020, C010\n"
		"vadd.t   C020, C020, C000\n"
		"vf2iz.t  C020, C020, 23\n"
		"viim.s   S023, 255\n"
		"vf2iz.s  S023, S023, 23\n"
		"vi2uc.q  S000, C020\n"
		"mfv      %0, S000\n"
	:"=r"(result): "r"(min), "r"(max));
	return result;
}

float vfpu_randf(float min, float max) {
    float result;
    __asm__ volatile (
		"mtv      %1, S000\n"
        "mtv      %2, S001\n"
        "vsub.s   S001, S001, S000\n"
        "vrndf1.s S002\n"
        "vone.s	  S003\n"
        "vsub.s   S002, S002, S003\n"
        "vmul.s   S001, S002, S001\n"
        "vadd.s   S000, S000, S001\n"
        "mfv      %0, S000\n"
        : "=r"(result) : "r"(min), "r"(max));
    return result;
}

void vfpu_scale_vector(ScePspFVector4 *vout, ScePspFVector4 *vin, float scale) {
   __asm__ volatile (
       "lv.q    C000, %1\n"
       "mtv     %2, S010\n"
       "vscl.t  C000, C000, S010\n"
       "sv.q    C000, %0\n"
       : "=m"(*vout) : "m"(*vin), "r"(scale));
}

void vfpu_sincos(float r, float *s, float *c) {
	__asm__ volatile (
		"mtv      %2, S002\n"
		"vcst.s   S003, VFPU_2_PI\n"
		"vmul.s   S002, S002, S003\n"
		"vrot.p   C000, S002, [s, c]\n"
		"mfv      %0, S000\n"
		"mfv      %1, S001\n"
	: "=r"(*s), "=r"(*c): "r"(r));
}

float vfpu_sinf(float rad) {
    float result;
    __asm__ volatile (
        "mtv     %1, S000\n"
        "vcst.s  S001, VFPU_2_PI\n"
        "vmul.s  S000, S000, S001\n"
        "vsin.s  S000, S000\n"
        "mfv     %0, S000\n"
        : "=r"(result) : "r"(rad));
    return result;
}

float vfpu_sinhf(float x) {
	float result;
	__asm__ volatile (
		"mtv      %1, S000\n"
		"vcst.s   S001, VFPU_LN2\n"
		"vrcp.s   S001, S001\n"
		"vmov.s   S002, S000[|x|]\n"
		"vcmp.s   NE, S000, S002\n"
        "vmul.s   S002, S001, S002\n"
        "vexp2.s  S002, S002\n"
        "vrcp.s   S003, S002\n"
        "vsub.s   S002, S002, S003\n"
        "vmul.s   S002, S002, S002[1/2]\n"
        "vcmov.s  S002, S002[-x], 0\n"
        "mfv      %0, S002\n"
	: "=r"(result) : "r"(x));
	return result;
}

void vfpu_sphere_to_cartesian(float az, float ze, float rad, float *x, float *y, float *z) {
	__asm__ volatile (
		"mtv      %3, S000\n"
		"mtv      %4, S001\n"
		"mtv      %5, S002\n"
		"vcst.s   S003, VFPU_2_PI\n" 							// C000 = [az, ze, rad, 2/pi]
		"vscl.p   C000, C000, S003\n"							// C000 = [az*2/pi, ze*2/pi, rad, 2/pi]
		"vrot.p   C010, S000, [s, c]\n"							// C010 = [sin(az), cos(az), ?, ?]
		"vrot.p   C012, S001, [s, c]\n"							// C010 = [sin(az), cos(az), sin(ze), cos(ze)]
		"vmul.q   C020, C010[y, 1, x, 0], C010[z, w, z, 0]\n"   // C020 = [0, cos(az)*sin(ez), cos(ze), sin(az)*sin(ze)]
		"vscl.t   C020, C020, S002\n"							// C020 = [0, r*cos(az)*sin(ez), r*cos(ze), r*sin(az)*sin(ze)]
		//"sv.q     C020, 0 + %0\n"
		"sv.s     S020, %0\n"
		"sv.s     S021, %1\n"
		"sv.s     S022, %2\n"
	:"+m"(*x), "+m"(*y), "+m"(*z)
	:"r"(az), "r"(ze), "r"(rad));
}

float vfpu_sqrtf(float x) {
	float result;
	__asm__ volatile (
		"mtv     %1, S000\n"
		"vsqrt.s S000, S000\n"
		"mfv     %0, S000\n"
	: "=r"(result) : "r"(x));
	return result;
}

void vfpu_srand(unsigned int x) {
	__asm__ volatile ( "mtv %0, S000\n vrnds.s S000" : "=r"(x));
}

float vfpu_tanf(float x) {
	float result;
	// result = sin(x)/cos(x);
	__asm__ volatile (
		"mtv      %1, S000\n"
		"vcst.s   S001, VFPU_2_PI\n"
        "vmul.s   S000, S000, S001\n"
        "vrot.p   C002, S000, [s, c]\n"
        "vdiv.s   S000, S002, S003\n"
        "mfv      %0, S000\n"
	: "=r"(result) : "r"(x));
	return result;
}

float vfpu_tanhf(float x) {
	float result;
	//y = exp(x+x);
	//return (y-1)/(y+1);
	__asm__ volatile (
		"mtv      %0, S000\n"
		"vadd.s   S000, S000, S000\n"
		"vcst.s   S001, VFPU_LN2\n"
		"vrcp.s   S001, S001\n"
		"vmul.s   S000, S000, S001\n"
        "vexp2.s  S000, S000\n"
        "vone.s   S001\n"
        "vbfy1.p  C002, C000\n"
        "vdiv.s   S000, S003, S002\n"
        "mfv      %0, S000\n"
	: "=r"(result): "r"(x));
	return result;
}

void vfpu_translate_matrix(ScePspFMatrix4 *m, float x, float y, float z)
{
	__asm__ volatile (
		"vmidt.q M000\n"
		"mtv        %1, S030\n"
		"mtv        %2, S031\n"
		"mtv        %3, S032\n"
		//"vmidt.q M100\n"
		//"lv.q   C200, %1\n"
		//"vmov.t  C130, C200\n"
		//"vmmul.q M200, M100, M000\n"
		"sv.q C000,  0 + %0\n"
		"sv.q C010, 16 + %0\n"
		"sv.q C020, 32 + %0\n"
		"sv.q C030, 48 + %0\n"
		: "=m"(*m) : "r"(x), "r"(y), "r"(z));
}

void vfpu_zero_vector(ScePspFVector4 *v) {
   __asm__ volatile (
       "vzero.t C000\n"
       "sv.q    C000, %0\n"
       : "+m"(*v));
}

