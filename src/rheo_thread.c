#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <stdio.h>
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
  rv->adc = malloc(ADC_COUNT * sizeof(float));
  rv->temperature = malloc(sizeof(float));

  rv->stopped = 0;
  rv->errored = 0;
  rv->adc_ready = 0;
  rv->opt_ready = 0;
  rv->log_ready = 0;
  rv->tmp_ready = 0;
  rv->error_string = "all is well";
  return rv;
}




void
free_thread_data(thread_data *dat)
{
  adc_close(dat->adc_h);
  free_run_data(dat->run_d);
  free(dat->time_s);
  free(dat->time_us);
  free(dat->adc);
  free(dat->temperature);
  free(dat);
}



void
nsleep(unsigned int delay_ns)
{
  struct timespec delay, res;
  delay.tv_sec = 0;
  delay.tv_nsec = delay_ns;
  nanosleep(&delay, &res);
}
