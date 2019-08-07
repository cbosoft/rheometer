#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <glob.h>

#include "log.h"
#include "run.h"
#include "json.h"
#include "util.h"
#include "error.h"
#include "adc.h"
#include "loadcell.h"


#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define CHECKJSON(C) if (C == NULL) { warn("CHECKJSON", "Error creating run params JSON. \n  Write down the fill depth for this run!"); return; }




void generate_log_prefix(struct run_data *rd)
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

  free(date);

  struct timeval tv;
  gettimeofday(&tv, 0);
  rd->start_time_s = tv.tv_sec;
  rd->start_time_us = tv.tv_usec;

}




void save_run_params_to_json(struct run_data *rd)
{
  cJSON *params = cJSON_CreateObject();
  CHECKJSON(params);
  
  cJSON *control_scheme_json = read_json(rd->control_scheme_path);
  CHECKJSON(control_scheme_json);
  cJSON_AddItemToObject(params, "control_scheme", control_scheme_json);

  cJSON *length_s_json = cJSON_CreateNumber(rd->length_s);
  CHECKJSON(length_s_json);
  cJSON_AddItemToObject(params, "length_s", length_s_json);

  cJSON *depth_mm_json = cJSON_CreateNumber(rd->fill_depth);
  CHECKJSON(depth_mm_json);
  cJSON_AddItemToObject(params, "depth_mm", depth_mm_json);

  char *params_json_str = cJSON_Print(params);
  char *params_path = calloc(300, sizeof(char));
  sprintf(params_path, "%s_runparams.json", rd->log_pref);
  
  FILE *fp = fopen(params_path, "w");
  fprintf(fp, "%s\n", params_json_str);
  fclose(fp);
  
  rd->log_paths[rd->log_count] = calloc(256, sizeof(char));
  strcpy(rd->log_paths[rd->log_count], params_path);
  rd->log_count ++;

  free(params_path);
  free(params_json_str);
  cJSON_Delete(params);
}




void add_log(struct run_data *rd, const char* fmt, ...)
{
  if (rd->log_paths == NULL) {
    rd->log_paths = malloc(sizeof(char*));
  }
  else {
    rd->log_paths = realloc(rd->log_paths, (rd->log_count + 1)*sizeof(char*));
  }

  rd->log_paths[rd->log_count] = calloc(PATH_MAX, sizeof(char));

  va_list ap;

  va_start(ap, fmt);
  vsnprintf(rd->log_paths[rd->log_count], PATH_MAX, fmt, ap);
  va_end(ap);

  rd->log_count ++;
}



void set_time(struct run_data *rd)
{
  struct timeval tv;
  unsigned long *sec, *usec, *psec, *pusec;
  psec = rd->time_s;
  pusec = rd->time_us;

  sec = malloc(sizeof(unsigned long));
  usec = malloc(sizeof(unsigned long));

  gettimeofday(&tv, 0);
  (*sec) = tv.tv_sec;
  (*usec) = tv.tv_usec;

  rd->time_s = sec;
  rd->time_us = usec;

  free(psec);
  free(pusec);

  unsigned long dt_sec = (*sec) - rd->start_time_s, dt_usec;
  if ((*usec) < rd->start_time_us) {
    dt_sec -= 1;
    dt_usec = rd->start_time_us - (*usec);
  }
  else {
    dt_usec = (*usec) - rd->start_time_us;
  }
  rd->time_s_f = (double)dt_sec + (0.001 * 0.001 * ((double)dt_usec));
}




void *log_thread_func(void *vptr) {

  struct run_data *rd = (struct run_data *)vptr;

  if (rd->log_pref == NULL)
    ferr("log_thread_func", "data must be initialised before logging is started.");

  add_log(rd, "%s.csv", rd->log_pref);

  FILE *log_fp = fopen(rd->log_paths[0], "w");
  rd->log_count ++;

  rd->log_ready = 1;
  while ( (!rd->stopped) && (!rd->errored) ) {
    set_time(rd);
    fprintf(log_fp, "%lu.%06lu,", (*rd->time_s), (*rd->time_us));
    for (unsigned int channel = 0; channel < ADC_COUNT; channel++)
      fprintf(log_fp, "%lu,", rd->adc[channel]);
    fprintf(log_fp, "%u,", rd->last_ca);
    fprintf(log_fp, "%f,", (*rd->temperature));
    fprintf(log_fp, "%f\n", read_loadcell(rd));
    //TODO read speed/strainrate

    rh_usleep(900);
  }

  fclose(log_fp);
  pthread_exit(0);

  return NULL;
}
