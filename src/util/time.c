#include "time.h"

double time_elapsed(struct timeval end, struct timeval start)
{
  double seconds = end.tv_sec - start.tv_sec;
  double usec = end.tv_usec - start.tv_usec;

  return seconds + (usec*0.001*0.001);
}
