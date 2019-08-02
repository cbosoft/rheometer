#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>

#include "log.h"
#include "run.h"
#include "cJSON.h"
#include "util.h"
#include "error.h"
#include "adc.h"
#include "io.h"




#define CHECKJSON(C) if (C == NULL) { warn("CHECKJSON", "Error creating run params JSON. \n  Write down the fill depth for this run!"); return; }




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




void * log_thread_func(void *vptr) {

  struct run_data *rd = (struct run_data *)vptr;

  if (rd->log_pref == NULL)
    ferr("log_thread_func", "data must be initialised before logging is started.");
  
  struct timeval tv;
  unsigned long *sec, *usec, *psec, *pusec;
  
  rd->log_paths[0] = calloc(256, sizeof(char));
  sprintf(rd->log_paths[0], "%s.csv", rd->log_pref);

  FILE *log_fp = fopen(rd->log_paths[0], "w");
  rd->log_count ++;

  rd->log_ready = 1;
  while ( (!rd->stopped) && (!rd->errored) ) {
    // set the time
    gettimeofday(&tv, 0);
    psec = rd->time_s;
    pusec = rd->time_us;
    sec = malloc(sizeof(unsigned long));
    usec = malloc(sizeof(unsigned long));
    (*sec) = tv.tv_sec;
    (*usec) = tv.tv_usec;
    rd->time_s = sec;
    rd->time_us = usec;

    unsigned long dt_sec = (*sec) - rd->start_time_s, dt_usec;
    if ((*usec) < rd->start_time_us) {
      dt_sec -= 1;
      dt_usec = rd->start_time_us - (*usec);
    }
    else {
      dt_usec = (*usec) - rd->start_time_us;
    }
    rd->time_s_f = (double)dt_sec + (0.001 * 0.001 * ((double)dt_usec));

    free(psec);
    free(pusec);

    fprintf(log_fp, "%lu.%06lu,", (*sec), (*usec));
    for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
      fprintf(log_fp, "%lu,", rd->adc[channel]);
    }
    fprintf(log_fp, "%u", rd->last_ca);
    fprintf(log_fp, "%f\n", (*rd->temperature));

    rh_usleep(100);
  }

  fclose(log_fp);
  pthread_exit(0);

  return NULL;
}
