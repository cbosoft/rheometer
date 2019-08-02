#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <glob.h>
#include <sys/time.h>

#include "run.h"
#include "util.h"
#include "control.h"
#include "adc.h"



struct run_data *init_run_data()
{
  struct run_data *rd = calloc(1, sizeof(struct run_data));

  rd->time_s = calloc(1, sizeof(unsigned long));
  rd->time_us = calloc(1, sizeof(unsigned long));
  rd->start_time_s = 0;
  rd->start_time_us = 0;
  rd->time_s_f = 0.0;
  rd->adc = calloc(ADC_COUNT, sizeof(float));
  rd->temperature = calloc(1, sizeof(float));

  rd->last_ca = 0;
  rd->errhist = calloc(ERR_HIST, sizeof(float));

  rd->stopped = 0;
  rd->errored = 0;
  rd->adc_ready = 0;
  rd->opt_ready = 0;
  rd->log_ready = 0;
  rd->tmp_ready = 0;

  rd->adc_busy = 0;

  rd->error_string = "all is well";
  return rd;
}




void setup_run_data(struct run_data *rd)
{
  const char *log_dir = "logs";
  const char *genpref = "rpir";

  char *date = calloc(15, sizeof(char));
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(date, 50, "%Y-%m-%d", timeinfo);

  char *pattern = calloc(256, sizeof(char));
  sprintf(pattern, "%s/%s_%s(*)_%s_%s.csv", log_dir, genpref, date, rd->control_scheme, rd->tag);

  glob_t glob_res;
  glob((const char *)pattern, GLOB_NOSORT, NULL, &glob_res);
  free(pattern);

  char *log_pref = calloc(256, sizeof(char));
  sprintf(log_pref, "%s/%s_%s(%u)_%s_%s", log_dir, genpref, date, (unsigned int)glob_res.gl_pathc, rd->control_scheme, rd->tag);
  rd->log_pref = log_pref;
  rd->log_paths = calloc(10, sizeof(char *));
  rd->log_count = 0;

  struct timeval tv;
  gettimeofday(&tv, 0);
  rd->start_time_s = tv.tv_sec;
  rd->start_time_us = tv.tv_usec;

  free(date);
}




void free_run_data(struct run_data *td)
{
  adc_close(td->adc_handle);
  for (unsigned int i = 0; i < td->log_count; i++)
    free(td->log_paths[i]);

  if (td->time_s != NULL) free(td->time_s);
  if (td->time_us != NULL) free(td->time_us);

  if (td->log_pref != NULL) free(td->log_pref);
  if (td->errhist != NULL) free(td->errhist);
  
  if (td->adc != NULL) free(td->adc);
  if (td->temperature != NULL) free(td->temperature);
  if (td->tag != NULL) free(td->tag);

  free(td);
}
