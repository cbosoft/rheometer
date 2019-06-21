#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "rheo.h"




void *
log_thread_func(void *vtd) {
  thread_data *td = (thread_data *)vtd;
  
  struct timeval tv;
  unsigned long *sec, *usec, *psec, *pusec;

  char *logpath = calloc(256, sizeof(char));
  sprintf(logpath, "%s.csv", td->log_pref);
  fprintf(stderr, "Main log file \033[34;1m\"%s\"\033[0m\n", logpath);

  FILE *log_fp = fopen(logpath, "w");

  td->log_ready = 1;
  while ( (!td->stopped) && (!td->errored) ) {
    // set the time
    gettimeofday(&tv, 0);
    psec = td->time_s;
    pusec = td->time_us;
    sec = malloc(sizeof(unsigned long));
    usec = malloc(sizeof(unsigned long));
    (*sec) = tv.tv_sec;
    (*usec) = tv.tv_usec;
    td->time_s = sec;
    td->time_us = usec;
    free(psec);
    free(pusec);
    
    fprintf(log_fp, "%lu.%06lu,", (*sec), (*usec));
    for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
      fprintf(log_fp, "%lu,", td->adc[channel]);
    }
    fprintf(log_fp, "%f\n", (*td->temperature));

    
    nsleep(100000);
  }

  fclose(log_fp);
  free(logpath);
  
  return NULL;
}
