#ifndef FILER_H
#define FILER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern char LastPath[], FilerMsg[];

int getExtId(const char *szFilePath);

int searchFile(const char *path, const char *name);
int getFilePath(char *out,int can_exit);
int getNoExtFilePath(char *out,int can_exit);
int filer_init(char*msg,char*path);

// —LŒø‚ÈŠg’£Žq
enum {    
  EXT_SMC,
  EXT_SFC,
  EXT_FIG,
  EXT_BIN,
  EXT_1,
  EXT_ZIP,
  EXT_SPC,
  EXT_TXT,
  EXT_UNKNOWN
};


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
