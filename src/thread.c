#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <glob.h>
#include <sys/time.h>

#include "rheo.h"

thread_data *
create_thread_data()
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
init(int argc, const char **argv, thread_data *td)
{
  const char *log_dir = "logs";
  const char *genpref = "rpir";
  char *controlscheme = "constant-XX";

  char *date = calloc(15, sizeof(char));
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(date, 50, "%Y-%m-%d", timeinfo);

  char *pattern = calloc(256, sizeof(char));
  sprintf(pattern, "%s/%s_%s(*)_%s_%s.csv", log_dir, genpref, date, controlscheme, td->tag);

  glob_t glob_res;
  glob((const char *)pattern, GLOB_NOSORT, NULL, &glob_res);
  free(pattern);

  char *log_pref = calloc(256, sizeof(char));
  sprintf(log_pref, "%s/%s_%s(%u)_%s_%s", log_dir, genpref, date, (unsigned int)glob_res.gl_pathc, controlscheme, td->tag);
  td->log_pref = log_pref;
  td->log_paths = calloc(10, sizeof(char *));
  td->log_count = 0;
  td->opt_log_fps = calloc(OPTENC_COUNT, sizeof(FILE *));
}




void
free_thread_data(thread_data *dat)
{
  adc_close(dat->adc_h);
  for (unsigned int i = 0; i < dat->log_count; i++) {
    free(dat->log_paths[i]);
  }
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
