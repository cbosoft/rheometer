#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "rheo.h"

thread_data *
new_thread_data()
{
  thread_data *rv = malloc(sizeof(thread_data));
  
  rv->time_s = malloc(sizeof(unsigned long));
  rv->time_us = malloc(sizeof(unsigned long));
  rv->adc = malloc(ADC_COUNT * sizeof(float *));
  for (int i = 0; i < ADC_COUNT; i++)
    rv->adc[i] = malloc(sizeof(float));
  rv->temperature = malloc(sizeof(float));

  rv->stopped = 0;
  rv->errored = 0;
  rv->error_string = "all is well";
  return rv;
}




void
free_thread_data(thread_data *dat)
{
  free(dat->time_s);
  free(dat->time_us);
  for (int i = 0; i < ADC_COUNT; i++)
    free(dat->adc[i]);
  free(dat->adc);
  free(dat->temperature);
  free(dat);
}




void *
log_thread_func(void *vt_d) {
  thread_data *t_d = (thread_data *)vt_d;
  
  struct timeval tv;
  struct timespec delay, res;
  delay.tv_sec = 0;
  delay.tv_nsec = 10000;
  unsigned long *sec, *usec, *psec, *pusec;
  while ( (!t_d->stopped) && (!t_d->errored) ) {
    // set the time
    gettimeofday(&tv, 0);
    psec = t_d->time_s;
    pusec = t_d->time_us;
    sec = malloc(sizeof(unsigned long));
    usec = malloc(sizeof(unsigned long));
    (*sec) = tv.tv_sec;
    (*usec) = tv.tv_usec;
    t_d->time_s = sec;
    t_d->time_us = usec;
    free(psec);
    free(pusec);

    nanosleep(&delay, &res);
  }
  
  return NULL;
}
