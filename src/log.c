#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "rheo.h"




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
