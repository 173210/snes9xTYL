#ifndef __PROFILER_H__
#define __PROFILER_H__

#include <sys/time.h>

#ifdef PROFILE
typedef struct  {
  long time_S9xDoHBlankProcessing;  
  long time_S9xMainLoop;
} profile_t;

extern profile_t profile_data;

#define START_PROFILE_FUNC(name)     \
  static timeval __func_time_start;      \
  static timeval __func_time_finish;     \
                                         \
  gettimeofday (& __func_time_start, 0);

#define FINISH_PROFILE_FUNC(name)         \
  gettimeofday (& __func_time_finish, 0);     \
                                              \
  profile_data.time_##name += (                \
    (((__func_time_finish.tv_sec -            \
       __func_time_start.tv_sec) * 1000000) + \
     (__func_time_finish.tv_usec) -           \
      __func_time_start.tv_usec)              \
  );
  
#define RESET_PROFILE_FUNC(name)         \
	profile_data.time_##name = 0;
	
#else
#define START_PROFILE_FUNC(name)  ;
#define FINISH_PROFILE_FUNC(name) ;
#define RESET_PROFILE_FUNC(name)  ;
#endif /* PROFILE_GFX */




#endif /* __PROFILER_H__ */
