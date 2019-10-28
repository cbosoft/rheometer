#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

#include "util.h"
#include "error.h"


#define BILLION 1000*1000*1000


double time_elapsed(struct timeval end, struct timeval start)
{
  double seconds = end.tv_sec - start.tv_sec;
  double usec = end.tv_usec - start.tv_usec;

  return seconds + (usec*0.001*0.001);
}


#ifdef BLOCKING_SLEEP

static void _sleep(double delay_s)
{
  struct timeval start, now;
  gettimeofday(&start, NULL);
  while (1) {
    gettimeofday(&now, NULL);
    if (time_elapsed(now, start) > delay_s)
      break;
  }
}

#else

static void _sleep(double delay_s)
{
  struct timespec delay;
  delay.tv_sec = (int)(delay_s);
  delay.tv_nsec = (((int)delay_s)*BILLION) % BILLION;
  int rv = nanosleep(&delay, NULL);

  if (rv)
    warn("nonblocking_sleep", "nanosleep interrupted.");
}

#endif


void sleep_us(double delay_us)
{
  _sleep( delay_us * 1e-6 );
}


void sleep_ms(double delay_ms)
{
  _sleep( delay_ms * 1e-3 );
}
