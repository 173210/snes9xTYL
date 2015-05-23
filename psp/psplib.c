/*#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <sys/stat.h>*/

#include "psp.h"

#define PATH_MAX 256

#if 0

char *strcpy( char *dest, const char *src );
int sprintf( char *buffer, const char *format, ... ){
	strcpy( buffer, format );
	return 0;
}

int printf( const char *format, ... ){
}

int puts( const char *string ){
}

int memcmp(const void *s1, const void *s2, size_t len){
	unsigned long int	a0, b0, res;
	int		i;

	i = 0;
	while ( len != 0 ){
		a0 = ((unsigned char*)s1)[i];
		b0 = ((unsigned char*)s2)[i];
		res = a0 - b0;
		if ( res != 0 )
			return res;
		len--;
    }

	return 0;
}

# define op_t	unsigned long int
# define OPSIZ	(sizeof(op_t))

void *memset(void *buf, int ch, size_t n){
	int		i;

	if (n == 0) return buf;

	if ( ch ){
		for ( i = 0; i < n; i++ ){
			((char*)buf)[i] = ch;
		}
	} else {
		long int dstp = (long int)buf;

		if ( n >= 8 ){
			size_t	xlen;

			while ( (dstp % OPSIZ) != 0 ){
				((char*)dstp)[0] = 0;
				dstp += 1;
				n -= 1;
			}

			xlen = n / (OPSIZ * 8);
			while ( xlen != 0 ){
				((op_t*)dstp)[0] = 0;
				((op_t*)dstp)[1] = 0;
				((op_t*)dstp)[2] = 0;
				((op_t*)dstp)[3] = 0;
				((op_t*)dstp)[4] = 0;
				((op_t*)dstp)[5] = 0;
				((op_t*)dstp)[6] = 0;
				((op_t*)dstp)[7] = 0;
				dstp += 8 * OPSIZ;
				xlen -= 1;
			}
			n %= OPSIZ * 8;

			xlen = n / OPSIZ;
			while ( xlen != 0 ){
				((op_t*)dstp)[0] = 0;
				dstp += OPSIZ;
				xlen -= 1;
			}
			n %= OPSIZ;
		}

		while ( n != 0 ){
			((char*)dstp)[0] = 0;
			dstp += 1;
			n -= 1;
		}
	}

	return buf;
}

void *memcpy(void *dest, const void *src, size_t n){
	int		i;

	if (n == 0) return dest;

	for ( i = 0; i < n; i++ ){
		((char*)dest)[i] = ((char*)src)[i];
	}
	return dest;
}

void* memmove( void* dest, const void* src, size_t n ){
	unsigned long int dstp = (long int) dest;
	unsigned long int srcp = (long int) src;
	int		i;

	if (n == 0) return dest;

	if ( dstp - srcp >= n ){
		for ( i = 0; i < n; i++ ){
			((char*)dstp)[i] = ((char*)srcp)[i];
		}
	} else {
		for ( i = 0; i < n; i++ ){
			((char*)dstp)[n - i - 1] = ((char*)srcp)[n - i - 1];
		}
	}
	return dest;
}

size_t strlen( const char *string ){
	int		i;

	for ( i = 0; string[i]; i++ );

	return i;
}

char *strcpy( char *dest, const char *src )
{
	int		i;

	for ( i = 0; src[i]; i++ ){
		dest[i] = src[i];
	}
	dest[i] = 0;

	return dest;
}

char *strncpy( char *dest, const char *src, size_t count ){
	int		i;

	for ( i = 0; i < count; i++ ){
		dest[i] = src[i];
	}
	dest[i] = 0;

	return dest;
}

char *strcat(char *dest, const char *src){
	strcpy( &dest[strlen( dest )], src );

	return dest;
}

char *strstr (char *s1, char*s2){
  register char *p = s1;
  register int len = strlen (s2);

  for (; (p = strchr (p, *s2)) != 0; p++)
    {
      if (strncmp (p, s2, len) == 0)
	{
	  return (p);
	}
    }
  return (0);
}


char *strchr(const char *s, int c){
	int		size;
	int		i;

	size = strlen( s );
	for ( i = 0; i < size; i++ ){
		if ( s[i] == c ){
			return (char*)&s[i];
		}
	}
	return 0;
}



char* strrchr(const char *string, int c){
	/* use the asm strchr to do strrchr */
	char* lastmatch;
	char* result;

	/* if char is never found then this will return 0 */
	/* if char is found then this will return the last matched location
	   before strchr returned 0 */

	lastmatch = 0;
	result = strchr(string,c);

	while ((int)result != 0)
	{
		lastmatch=result;
		result = strchr(lastmatch+1,c);
	}

	return lastmatch;
}

int strcmp(const char *p1, const char *p2){
	register const unsigned char *s1 = (const unsigned char *) p1;
	register const unsigned char *s2 = (const unsigned char *) p2;
	unsigned char c1, c2;

	do {
		c1 = (unsigned char) *s1++;
		c2 = (unsigned char) *s2++;
		if (c1 == '\0')
			return c1 - c2;
	} while (c1 == c2);

	return c1 - c2;
}

int strncmp(const char *s1, const char* s2, size_t n){
	unsigned char c1 = '\0';
	unsigned char c2 = '\0';

	while (n > 0){
		c1 = (unsigned char) *s1++;
		c2 = (unsigned char) *s2++;
		if (c1 == '\0' || c1 != c2)
			return c1 - c2;
		n--;
	}

	return c1 - c2;
}

int isupper(int c){
	if (c < 'A')
		return 0;

	if (c > 'Z')
		return 0;

	// passed both criteria, so it
	// is an upper case alpha char
	return 1;
}

int islower(int c){
	if (c < 'a')
		return 0;

	if (c > 'z')
		return 0;

	// passed both criteria, so it
	// is a lower case alpha char
	return 1;
}

int isalpha(int c){
	if ( islower( c ) || isupper( c ) ){
		return 1;
	}
	return 0;
}

int tolower(int c){
	if ( isupper( c ) ){
		c += 32;
	}
	return c;
}

int toupper(int c){
	if ( islower( c ) ){
		c -= 32;
	}
	return c;
}

int strcasecmp( const char *s1, const char *s2 ){
	const unsigned char*	p1 = (const unsigned char*)s1;
	const unsigned char*	p2 = (const unsigned char*)s2;
	unsigned char		c1, c2;
	int result;

	if ( p1 == p2 ){
		return 0;
	}

	do {
		c1 = *p1++;
		c2 = *p2++;

        if ( c1 >= 'A' && c1 <= 'Z' ){
            c1 += 0x20;
		}
        if ( c2 >= 'A' && c2 <= 'Z' ){
            c2 += 0x20;
		}

		if ( (result = c1 - c2) ){
			break;
		}
		if ( !c1 ){
			break;
		}
	} while ( 1 );

	return result;
}

int strncasecmp(const char *s1, const char *s2, unsigned n){
	const unsigned char*	p1 = (const unsigned char*)s1;
	const unsigned char*	p2 = (const unsigned char*)s2;
	unsigned char		c1, c2;
	int result;

	if ( p1 == p2 || n == 0 ){
		return 0;
	}

	do {
		c1 = *p1++;
		c2 = *p2++;

        if ( c1 >= 'A' && c1 <= 'Z' ){
            c1 += 0x20;
		}
        if ( c2 >= 'A' && c2 <= 'Z' ){
            c2 += 0x20;
		}

		if ( (result = c1 - c2) ){
			break;
		}
		if ( !c1 || --n == 0 ){
			break;
		}
	} while ( 1 );

	return result;
}

int isdigit(int c){
	if (c < '0')
		return 0;

	if (c > '9')
		return 0;

	// passed both criteria, so it
	// is a numerical char
	return 1;
}

// -------------------------------
unsigned int         __stdlib_rand_seed = 92384729;

int rand(void){
// I don't agree with it...
//  return (__stdlib_rand_seed = ((((__stdlib_rand_seed * 214013) + 2531011) >> 16) & 0xffff));
  unsigned long long t = __stdlib_rand_seed;

  t *= 254124045ull;
  t += 76447ull;
  __stdlib_rand_seed = t;
  // We return a number between 0 and RAND_MAX, which is 2^31-1.
  return (t >> 16) & 0x7FFFFFFF;
}

typedef int		cmp_t(const void *, const void *);
static __inline char	*med3(char *, char *, char *, cmp_t *, void *);
static __inline void	 swapfunc(char *, char *, int, int);

#define min(a, b)	(a) < (b) ? (a) : (b)

/*
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
 */
#define swapcode(TYPE, parmi, parmj, n) { 		\
	long i = (n) / sizeof (TYPE); 			\
	register TYPE *pi = (TYPE *) (parmi); 		\
	register TYPE *pj = (TYPE *) (parmj); 		\
	do { 						\
		register TYPE	t = *pi;		\
		*pi++ = *pj;				\
		*pj++ = t;				\
        } while (--i > 0);				\
}

#define SWAPINIT(a, es) swaptype = ((char *)a - (char *)0) % sizeof(long) || \
	es % sizeof(long) ? 2 : es == sizeof(long)? 0 : 1;

static __inline void swapfunc(char *a, char *b, int n, int swaptype){
	if(swaptype <= 1)
		swapcode(long, a, b, n)
	else
		swapcode(char, a, b, n)
}

#define swap(a, b)					\
	if (swaptype == 0) {				\
		long t = *(long *)(a);			\
		*(long *)(a) = *(long *)(b);		\
		*(long *)(b) = t;			\
	} else						\
		swapfunc((char*)a, b, es, swaptype)

#define vecswap(a, b, n) 	if ((n) > 0) swapfunc((char*)a, b, n, swaptype)

#define	CMP(t, x, y) (cmp((x), (y)))

static __inline char * med3(char *a, char *b, char *c, cmp_t *cmp, void *thunk){
	return CMP(thunk, a, b) < 0 ?
	       (CMP(thunk, b, c) < 0 ? b : (CMP(thunk, a, c) < 0 ? c : a ))
              :(CMP(thunk, b, c) > 0 ? b : (CMP(thunk, a, c) < 0 ? a : c ));
}

#define	thunk NULL
void
qsort(void *a, size_t n, size_t es, cmp_t *cmp){
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	int d, r, swaptype, swap_cnt;

loop:	SWAPINIT(a, es);
	swap_cnt = 0;
	if (n < 7) {
		for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
			for (pl = pm; pl > (char *)a && CMP(thunk, pl - es, pl) > 0;
			     pl -= es)
				swap(pl, pl - es);
		return;
	}
	pm = (char *)a + (n / 2) * es;
	if (n > 7) {
		pl = (char*)a;
		pn = (char *)a + (n - 1) * es;
		if (n > 40) {
			d = (n / 8) * es;
			pl = med3(pl, pl + d, pl + 2 * d, cmp, thunk);
			pm = med3(pm - d, pm, pm + d, cmp, thunk);
			pn = med3(pn - 2 * d, pn - d, pn, cmp, thunk);
		}
		pm = med3(pl, pm, pn, cmp, thunk);
	}
	swap(a, pm);
	pa = pb = (char *)a + es;

	pc = pd = (char *)a + (n - 1) * es;
	for (;;) {
		while (pb <= pc && (r = CMP(thunk, pb, a)) <= 0) {
			if (r == 0) {
				swap_cnt = 1;
				swap(pa, pb);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (r = CMP(thunk, pc, a)) >= 0) {
			if (r == 0) {
				swap_cnt = 1;
				swap(pc, pd);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swap(pb, pc);
		swap_cnt = 1;
		pb += es;
		pc -= es;
	}
	if (swap_cnt == 0) {  /* Switch to insertion sort */
		for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
			for (pl = pm; pl > (char *)a && CMP(thunk, pl - es, pl) > 0;
			     pl -= es)
				swap(pl, pl - es);
		return;
	}

	pn = (char *)a + n * es;
	r = min(pa - (char *)a, pb - pa);
	vecswap(a, pb - r, r);
	r = min(pd - pc, pn - pd - es);
	vecswap(pb, pn - r, r);
	if ((r = pb - pa) > es)
		qsort(a, r / es, es, cmp);
	if ((r = pd - pc) > es) {
		/* Iterate rather than recurse to save stack space */
		a = pn - r;
		n = r / es;
		goto loop;
	}
}
// -------------------------------

FILE *fopen(const char *path, const char *mode){
	int		psp_mode;
	int		fd;

	switch ( mode[0] ){
	case 'r':
		psp_mode = O_RDONLY;
		break;

	case 'w':
		psp_mode = O_WRONLY | O_CREAT | O_TRUNC;
		break;

	case 'a':
		psp_mode = O_RDWR | O_APPEND;
		break;
	}

	fd = open(path,psp_mode);//sceIoOpen( path, psp_mode, 0777 );
	if ( fd < 0 ){
		return NULL;
	}

	return (FILE*)fd;
}

int fclose(FILE *fp){
	sceIoClose( (int)fp );

	return 0;
}

int fgetc(FILE *fp){
	char	c;

	sceIoRead( (int)fp, &c, sizeof(char) );

	return (int)c;
}

int  fputc(int c,FILE *fp){	
	sceIoWrite( (int)fp, &c, sizeof(char) );
	return (int)c;
}

long  ftell ( FILE * fd ){
	return sceIoLseek((int)fd, 0, SEEK_CUR);
}
  
int  fseek ( FILE * fd, long offset , int origin ) {
	/*SEEK_SET (0) Beginning of file. 
SEEK_CUR (1) Current position of the file pointer. 
SEEK_END (2) End of file. 
*/
  sceIoLseek((int)fd, offset, origin);	
	return 0;
}

size_t fread(void *buf, size_t size, size_t n, FILE *fp){
	int		ret;

	ret = sceIoRead( (int)fp, buf, size * n );

	return ret / size;
}

size_t fwrite(const void *buf, size_t size, size_t n, FILE *fp){
	sceIoWrite( (int)fp, (void*)buf, size * n );

	return n;
}




#define	CHECKFILE(file)	if (file<3) return -1





static inline int seterrno(int ret){
	if (ret>=0) return ret;
	errno = ret;
	return -1;
}





int open (const char *path,int flags){
	char fullpath[PATH_MAX];
  char *p = convert_path(fullpath,path);
  return seterrno(sceIoOpen(p,flags,0777));
}


int read (int file,char *ptr,int len){
  CHECKFILE(file);
  return seterrno(sceIoRead(file,ptr,len));
}

int lseek (int file,int ptr,int dir){
  CHECKFILE(file);
  return seterrno(sceIoLseek(file,ptr,dir));
}

int write ( int file,const char *ptr,int len){
  switch(file) {
  case 0: /* stdin */
    return -1;
  case 1: /* stdout */
  case 2: /* stderr */
//    dbg_write(ptr,len);
//	pspeDebugWrite(ptr,len);
    return len;
  default:
    return seterrno(sceIoWrite(file,ptr,len));
  }
}

int close (int file) {
  CHECKFILE(file);
  sceIoClose(file);
//  fh[file]=0;
  return 0;
}


int fstat (int file,struct stat *st) {
  int size,cur;
  CHECKFILE(file);

  cur = sceIoLseek(file,0,SEEK_CUR);
  size = sceIoLseek(file,0,SEEK_END);
  sceIoLseek(file,cur,SEEK_SET);

  memset(st,0,sizeof(*st));

  st->st_mode = S_IFREG;
  st->st_size = size;
  return 0;
}


int creat (const char *path,int mode){
  return open(path,O_WRONLY|O_TRUNC|O_CREAT);
}

int rename (const char *oldpath,const char *newpath){
  char fulloldpath[PATH_MAX];
  char fullnewpath[PATH_MAX];
  return seterrno(sceIoRename(convert_path(fulloldpath,oldpath),convert_path(fullnewpath,newpath)) );
}

int unlink (const char *path){
  char fullpath[PATH_MAX];
  return seterrno(sceIoRemove(convert_path(fullpath,path)) );
}


int mkdir(const char *path,mode_t mode){
  char fullpath[PATH_MAX];
  return seterrno(sceIoMkdir(convert_path(fullpath,path),mode));
}

int rmdir(const char *path){
  char fullpath[PATH_MAX];
  return seterrno(sceIoRmdir(convert_path(fullpath,path)));
}



void _exit (int n){
  sceKernelExitGame();
}


clock_t clock(void){
	return sceKernelLibcClock();
}

time_t time(time_t *tm){
	return sceKernelLibcTime(tm);
}

int gettimeofday(struct timeval *__p, struct timezone *__z){
	return sceKernelLibcGettimeofday(__p,__z);
}

int isatty (int fd){
//  if (fd<3) return 1;
  return 0;
}

int link (char *old, char *new){
  return -1;
}

int kill (int n, int m){
  return -1;
}

int getpid (int n){
  return 1;
}

_raise (){
}



static struct timeval s_analyze;
void StartAnalyze(){
	sceKernelLibcGettimeofday( &s_analyze, 0 );
}

void StopAnalyze(){
	struct timeval now;
	int		diff;

	sceKernelLibcGettimeofday( &now, 0 );

	diff  = (now.tv_sec - s_analyze.tv_sec) * 1000000 + now.tv_usec - s_analyze.tv_usec;

	debug_int( "time:", diff );
}

#else
#include <ctype.h>
#endif

#define SLASH_STR "/"
#define SLASH_CHAR '/'

void _makepath( char *path, const char *drive, const char *dir, const char *fname, const char *ext ){
	if ( drive && *drive ){
		*path       = *drive;
		*(path + 1) = ':';
		*(path + 2) = 0;
	} else {
		*path = 0;
	}
	
	if ( dir && *dir ){
		strcat( path, dir );
		if ( strlen( dir ) != 1 || *dir != '\\' ){
			strcat( path, SLASH_STR );
		}
	}
	
	if ( fname ){
		strcat( path, fname );
	}
	if ( ext && *ext ){
		//strcat( path, "." );
		strcat( path, ext );
	}
}

void _splitpath( const char *path, char *drive, char *dir, char *fname, char *ext ){
	if ( *path && *(path + 1) == ':' ){
		*drive = toupper( (int)*path );
		path += 2;
	} else {
		*drive = 0;
	}

	char*	slash = strrchr( path, SLASH_CHAR );
	if ( !slash ){
		slash = strrchr( path, '/' );
	}
	char*	dot = strrchr( path, '.' );
	if ( dot && slash && dot < slash ){
		dot = NULL;
	}

	if ( !slash ){
		if ( *drive ){
			strcpy( dir, "\\" );
		} else {
			strcpy( dir, "" );
		}
		strcpy( fname, path );
		if ( dot ){
			*(fname + (dot - path)) = 0;
			strcpy( ext, dot + 1 );
		} else {
			strcpy( ext, "" );
		}
	} else {
		if ( *drive && *path != '\\' ){
			strcpy( dir, "\\" );
			strcat( dir, path );
			*(dir + (slash - path) + 1) = 0;
		} else {
			strcpy( dir, path );
			if ( (slash - path) == 0 ){
				*(dir + 1) = 0;
			} else {
				*(dir + (slash - path)) = 0;
			}
		}

		strcpy( fname, slash + 1 );
		if ( dot ){
			*(fname + (dot - slash) - 1) = 0;
			strcpy( ext, dot + 1 );
		} else {
			strcpy( ext, "" );
		}
	}
}
#ifndef atoi
int  atoi ( const char * s ){
	int i,r;
	if (s==NULL) return 0;		
	
	i=0;
	r=0;
	while (s[i]){
	 if ((s[i]>='0')&&(s[i]<='9')) r=r*10+(s[i]-'0');
	 else break;
	 i++;
	}
	
	
	return r;
}
#endif

int format_int( char* buf, int value ){
	char*	org;
	int		div;
	int		val;
	char	bFirst;
	int		i;

	org    = buf;
	bFirst = 1;
	div    = 1000000000;
	for ( i = 0; i < 10; i++ ){
		val = (unsigned)value / div;

		if ( !bFirst || val ){
			*buf++ = val + '0';
			bFirst = 0;
		}

		value %= div;
		div   /= 10;
	}

	if ( bFirst ){
		*buf++ = '0';
	}
	*buf = 0;

	return strlen( org );
}

/*void* memmove_psp( void* dest, const void* src, size_t n ){
	unsigned long int dstp = (long int) dest;
	unsigned long int srcp = (long int) src;
	int		i;

	if (n == 0) return dest;

	if ( dstp - srcp >= n ){
		for ( i = 0; i < n; i++ ){
			((char*)dstp)[i] = ((char*)srcp)[i];
		}
	} else {
		for ( i = 0; i < n; i++ ){
			((char*)dstp)[n - i - 1] = ((char*)srcp)[n - i - 1];
		}
	}
	return dest;
}*/

