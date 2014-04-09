#ifndef __PSPLIB_H__
#define __PSPLIB_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


int format_int( char* buf, int value );
void _splitpath( const char *path, char *drive, char *dir, char *fname, char *ext );
void _makepath( char *path, const char *drive, const char *dir, const char *fname, const char *ext );
char* convert_path(char *path);


void* memmove_psp( void* dest, const void* src, size_t n );

/*void StartAnalyze();
void StopAnalyze();*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
