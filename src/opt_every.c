#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>

#include <wiringPi.h>

#include "opt.h"
#include "control.h"




static const uint8_t opt_pins[OPTENC_COUNT] = {16, 20, 21}; // TODO reduce to only the two that are actually used



void opt_setup(struct run_data *rd)
{
  for (uint8_t i = 0; i < OPTENC_COUNT; i++) {

    rd->log_paths[i+1] = calloc(265, sizeof(char));
    sprintf(rd->log_paths[i+1], "%s_opt%d-combined.csv", rd->log_pref, opt_pins[i]);

    // Create empty log file for appending to later
    FILE *fp = fopen(rd->log_paths[i+1], "w");
    fclose(fp);

    rd->log_count ++;
    pinMode(opt_pins[i], INPUT);

  }
}




void opt_mark(struct run_data *rd, unsigned int i)
{
  if (rd->stopped)
    pthread_exit(0);

  struct timeval tv;
  gettimeofday(&tv, 0);
  FILE *fp = fopen(rd->log_paths[i+1], "a");
  fprintf(fp, "%lu.06%lu\n", tv.tv_sec, tv.tv_usec);
  fflush(fp);
  fclose(fp);

  float secs = (float)(tv.tv_sec % 1000);
  float usecs = (float)tv.tv_usec;
  float time = secs + (0.001*0.001*usecs);

  float *ptimes = calloc(SPD_HIST, sizeof(float)), *tmp;
  
  ptimes[0] = time;
  for (unsigned int j = 0; j < (SPD_HIST-1); j++) {
    ptimes[j+1] = rd->ptimes[i][j];
  }

  tmp = rd->ptimes[i];
  rd->ptimes[i] = ptimes;
  free(tmp);
}
