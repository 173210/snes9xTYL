float sinf(float x)
{
	float res;

	__asm__ (
		"mtv	%1, S000\n"
		"vcst.s	S001, VFPU_2_PI\n"
		"vmul.s	S000, S000, S001\n"
		"vsin.s	S000, S000\n"
		"mfv	%0, S000\n"
		: "=r"(res) : "r"(x));

	return res;
}

float cosf(float x)
{
	float res;

	__asm__ (
		"mtv	%1, S000\n"
		"vcst.s	S001, VFPU_2_PI\n"
		"vmul.s	S000, S000, S001\n"
		"vcos.s	S000, S000\n"
		"mfv	%0, S000\n"
		: "=r"(res) : "r"(x));

	return res;
}

float tanf(float x)
{
	float res;

	__asm__ (
		"mtv	%1, S000\n"
		"vcst.s	S001, VFPU_2_PI\n"
		"vmul.s	S000, S000, S001\n"
		"vrot.p	C002, S000, [s, c]\n"
		"vdiv.s	S000, S002, S003\n"
		"mfv	%0, S000\n"
	: "=r"(res) : "r"(x));

	return res;
}

float atanf(float x)
{
	float res;

	__asm__ volatile (
		"mtv	%1, S000\n"
		"vmul.s	S001, S000, S000\n"
		"vadd.s	S001, S001, S001[1]\n"
		"vrsq.s	S001, S001\n"
		"vmul.s	S000, S000, S001\n"
		"vasin.s	S000, S000\n"
		"vcst.s	S001, VFPU_PI_2\n"
		"vmul.s	S000, S000, S001\n"
		"mfv	%0, S000\n"
	: "=r"(res) : "r"(x));

	return (res);
}

float roundf(float x)
{
	return __builtin_allegrex_round_w_s(x);
}

float sqrtf(float x)
{
	return __builtin_allegrex_sqrt_s(x);
}

float powf(float x, float y)
{
	float res;

	__asm__ (
		"mtv	%1, S000\n"
		"mtv	%2, S001\n"
		"vlog2.s	S000, S000\n"
		"vmul.s	S000, S000, S001\n"
		"vexp2.s	S000, S000\n"
		"mfv	%0, S000\n"
		: "=r"(res) : "r"(x), "r"(y));

	return res;
}

double floor(double x)
{
	return __builtin_allegrex_floor_w_s(x);
}

double pow(double x, double y)
{
	return powf(x, y);
}
