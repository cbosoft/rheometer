#include <time.h>

#include "util.h"




void rh_nsleep(unsigned int delay_ns)
{
  struct timespec delay, res;
  delay.tv_sec = 0;
  delay.tv_nsec = delay_ns;
  nanosleep(&delay, &res);
}




void rh_usleep(unsigned int delay_us)
{
  rh_nsleep(delay_us * 1000);
}




void rh_msleep(unsigned int delay_ms)
{
  rh_nsleep(delay_ms * 1000 * 1000);
}
