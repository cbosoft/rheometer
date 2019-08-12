#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

#include "util.h"




double time_elapsed(struct timeval end, struct timeval start)
{
  double seconds = end.tv_sec - start.tv_sec;
  double usec = end.tv_usec - start.tv_usec;

  return seconds + (usec*0.001*0.001);
}




void sleep_us(double delay_us)
{
  blocking_sleep( delay_us * 1e-6 );
}




void sleep_ms(double delay_ms)
{
  blocking_sleep( delay_ms * 1e-3 );
}




void blocking_sleep(double delay_s)
{
  struct timeval start, now;
  gettimeofday(&start, NULL);
  while (1) {
    gettimeofday(&now, NULL);
    if (time_elapsed(now, start) > delay_s)
      break;
  }
}
