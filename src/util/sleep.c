#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

#include "time.h"
#include "sleep.h"
#include "error.h"


#define BILLION 1000*1000*1000


//////////////////////////////////////////////////////////////////////////////
// blocking sleep functions
/////////////////////////////////////////////////////////////////////////////

static void _blocking_sleep(double delay_s)
{
  struct timeval start, now;
  gettimeofday(&start, NULL);
  while (1) {
    gettimeofday(&now, NULL);
    if (time_elapsed(now, start) > delay_s)
      break;
  }
}


void blocking_sleep_us(double delay_us)
{
  _blocking_sleep( delay_us * 1e-6 );
}


void blocking_sleep_ms(double delay_ms)
{
  _blocking_sleep( delay_ms * 1e-3 );
}



//////////////////////////////////////////////////////////////////////////////
// non-blocking sleep functions
/////////////////////////////////////////////////////////////////////////////


static void _sleep(double delay_s)
{
  struct timespec delay;
  delay.tv_sec = (int)(delay_s);
  delay.tv_nsec = ((long)(delay_s*((double)(BILLION)))) % (BILLION);
  int rv = nanosleep(&delay, NULL);

  if (rv)
    warn("nonblocking_sleep", "nanosleep interrupted.");
}


void sleep_us(double delay_us)
{
  _sleep( delay_us * 1e-6 );
}


void sleep_ms(double delay_ms)
{
  _sleep( delay_ms * 1e-3 );
}
