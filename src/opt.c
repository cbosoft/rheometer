#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
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
opt_mark(FILE *logf) 
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  fprintf(logf, "%lu.06%lu", tv.tv_sec, tv.tv_usec);
  fflush(logf);
}
