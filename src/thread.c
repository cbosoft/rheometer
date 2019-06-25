#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <glob.h>
#include <sys/time.h>

#include "rheo.h"



void
nsleep(unsigned int delay_ns)
{
  struct timespec delay, res;
  delay.tv_sec = 0;
  delay.tv_nsec = delay_ns;
  nanosleep(&delay, &res);
}




thread_data_t *
create_thread_data()
{
  thread_data_t *rv = calloc(1, sizeof(thread_data_t));

  rv->time_s = calloc(1, sizeof(unsigned long));
  rv->time_us = calloc(1, sizeof(unsigned long));
  rv->adc = calloc(ADC_COUNT, sizeof(float));
  rv->temperature = calloc(1, sizeof(float));

  rv->last_ca = 0;

  rv->stopped = 0;
  rv->errored = 0;
  rv->adc_ready = 0;
  rv->opt_ready = 0;
  rv->log_ready = 0;
  rv->tmp_ready = 0;

  rv->adc_busy = 0;

  rv->error_string = "all is well";
   return rv;
}




void
init(int argc, const char **argv, thread_data_t *td)
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
  sprintf(pattern, "%s/%s_%s(*)_%s_%s.csv", log_dir, genpref, date, td->control_scheme, td->tag);

  glob_t glob_res;
  glob((const char *)pattern, GLOB_NOSORT, NULL, &glob_res);
  free(pattern);

  char *log_pref = calloc(256, sizeof(char));
  sprintf(log_pref, "%s/%s_%s(%u)_%s_%s", log_dir, genpref, date, (unsigned int)glob_res.gl_pathc, td->control_scheme, td->tag);
  td->log_pref = log_pref;
  td->log_paths = calloc(10, sizeof(char *));
  td->log_count = 0;
  td->opt_log_fps = calloc(OPTENC_COUNT, sizeof(FILE *));

  td->ptimes = calloc(OPTENC_COUNT, sizeof(float *));
  for (unsigned int i = 0; i < OPTENC_COUNT; i++)
    td->ptimes[i] = calloc(SPD_HIST, sizeof(float));

  free(date);
}




void
free_thread_data(thread_data_t *td)
{
  adc_close(td->adc_handle);
  for (unsigned int i = 0; i < td->log_count; i++)
    free(td->log_paths[i]);
  free(td->time_s);
  free(td->log_pref);
  free(td->time_us);
  for (unsigned int i = 0; i < OPTENC_COUNT; i++)
    free(td->ptimes[i]);
  free(td->ptimes);
  free(td->adc);
  free(td->temperature);
  free(td);
}
