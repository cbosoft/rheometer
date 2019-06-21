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




void *
log_thread_func(void *vt_d) {
  thread_data *t_d = (thread_data *)vt_d;
  
  struct timeval tv;
  unsigned long *sec, *usec, *psec, *pusec;
  
  char *logpref = "logs/rpir";
  char *logsuff = ".csv";
  char *date = calloc(50, sizeof(char));
  char *controlscheme = "constantXX";
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  // TODO: replace hour-minute-second with (count)
  strftime(date, 50, "%Y-%m-%d-%H-%M-%S", timeinfo);
  
  char *logpath = calloc(256, sizeof(char));
  sprintf(logpath, "%s_%s_%s_%s%s", logpref, date, controlscheme, t_d->run_d->tag, logsuff);

  fprintf(stderr, "Logging initialised; log file \033[34;1m\"%s\"\033[0m\n", logpath);

  FILE *log_fp = fopen(logpath, "w");

  t_d->log_ready = 1;
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
    
    fprintf(log_fp, "%lu.%06lu,", (*sec), (*usec));
    for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
      fprintf(log_fp, "%lu,", t_d->adc[channel]);
    }
    fprintf(log_fp, "%f\n", (*t_d->temperature));

    
    nsleep(100000);
  }

  fclose(log_fp);
  
  return NULL;
}



void *
adc_thread_func(void *vt_d) {
  thread_data *t_d = (thread_data *)vt_d;
  adc_handle *adc_h = t_d->adc_h;
  
  unsigned long *adc, *padc;

  t_d->adc_ready = 1;
  while ( (!t_d->stopped) && (!t_d->errored) ) {
    
    adc = malloc(ADC_COUNT*sizeof(unsigned long));
    for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
      adc[channel] = read_adc_value(adc_h, channel);
    }
    
    padc = t_d->adc;
    t_d->adc = adc;

    free(padc);

    nsleep(10000);

  }


  return NULL;
}
