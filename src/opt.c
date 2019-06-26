#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>

#include <wiringPi.h>

#include "rheo.h"

static const uint8_t opt_pins[OPTENC_COUNT] = {16, 20, 21};

void
opt_setup(thread_data_t *td)
{
  for (uint8_t i = 0; i < OPTENC_COUNT; i++) {
    td->log_paths[i+1] = calloc(265, sizeof(char));
    sprintf(td->log_paths[i+1], "%s_opt%d-combined.csv", td->log_pref, opt_pins[i]);
    td->opt_log_fps[i] = fopen(td->log_paths[i+1], "w");
    td->log_count ++;
    pinMode(opt_pins[i], INPUT);
  }
}

void
opt_mark(thread_data_t *td, unsigned int i)
{
  if (td->stopped)
    pthread_exit(0);

  struct timeval tv;
  gettimeofday(&tv, 0);
  fprintf(td->opt_log_fps[i], "%lu.06%lu\n", tv.tv_sec, tv.tv_usec);
  fflush(td->opt_log_fps[i]);

  float secs = (float)(tv.tv_sec % 1000);
  float usecs = (float)tv.tv_usec;
  float time = secs + (0.001*0.001*usecs);

  float *ptimes = calloc(SPD_HIST, sizeof(float)), *tmp;
  
  ptimes[0] = time;
  for (unsigned int j = 0; j < (SPD_HIST-1); j++) {
    ptimes[j+1] = td->ptimes[i][j];
  }

  tmp = td->ptimes[i];
  td->ptimes[i] = ptimes;
  free(tmp);
}
