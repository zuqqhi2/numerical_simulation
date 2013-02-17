#ifndef _H_TIMER
#define _H_TIMER

#include <stdio.h>
#include <sys/time.h>

#define print_timer(te) {printf("time of %s:%f[msec]\n", #te, te*1.0e-3);}

inline unsigned long long gettimeval(void) {
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  return ((unsigned long long)tv.tv_sec)*1000000+tv.tv_usec;
}

inline void start_timer(unsigned int *startt) {
  *startt = (unsigned int)gettimeval();
  return;
}

inline unsigned int stop_timer(unsigned int *startt) {
  unsigned int stopt = (unsigned int)gettimeval();
  return (stopt >= *startt) ? (stopt-*startt) : (stopt);
}

#endif
