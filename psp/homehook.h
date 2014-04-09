/******************************************************************************

	homehook.prx

******************************************************************************/

#ifndef HOMEHOOK_PRX_H
#define HOMEHOOK_PRX_H

#ifdef __cplusplus
extern "C" {
#endif

void initHomeButton(int devkit_version);
u32 readHomeButton(void);

#ifdef __cplusplus
}
#endif

#endif /* HOMEHOOK_PRX_H */
