#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>

#include "rheo.h"
#include "cJSON.h"

#define CHECKJSON(C) if (C == NULL) { warn("CHECKJSON", "Error creating run params JSON. \n  Write down the fill depth for this run!"); return; }

void
save_run_params_to_json(thread_data_t *td)
{
  cJSON *controlscheme = NULL, *length_s = NULL, *depth_mm = NULL;

  cJSON *params = cJSON_CreateObject();
  CHECKJSON(params);
  
  controlscheme = cJSON_CreateString(td->control_scheme);
  CHECKJSON(controlscheme);
  cJSON_AddItemToObject(params, "controlscheme", controlscheme);

  length_s = cJSON_CreateNumber(td->length_s);
  CHECKJSON(length_s);
  cJSON_AddItemToObject(params, "length_s", length_s);

  depth_mm = cJSON_CreateNumber(td->fill_depth);
  CHECKJSON(depth_mm);
  cJSON_AddItemToObject(params, "depth_mm", depth_mm);

  char *params_json_str = cJSON_Print(params);
  char *params_path = calloc(300, sizeof(char));
  sprintf(params_path, "%s_runparams.json", td->log_pref);
  
  FILE *fp = fopen(params_path, "w");
  fprintf(fp, "%s\n", params_json_str);
  fclose(fp);
  
  td->log_paths[td->log_count] = calloc(256, sizeof(char));
  strcpy(td->log_paths[td->log_count], params_path);
  td->log_count ++;

  free(params_path);
  free(params_json_str);
  cJSON_Delete(params);
}


void *
log_thread_func(void *vtd) {
  thread_data_t *td = (thread_data_t *)vtd;

  if (td->log_pref == NULL)
    ferr("log_thread_func", "data must be initialised before logging is started.");
  
  struct timeval tv;
  unsigned long *sec, *usec, *psec, *pusec;
  
  td->log_paths[0] = calloc(256, sizeof(char));
  sprintf(td->log_paths[0], "%s.csv", td->log_pref);

  FILE *log_fp = fopen(td->log_paths[0], "w");
  td->log_count ++;

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

    unsigned long dt_sec = (*sec) - td->start_time_s, dt_usec;
    if ((*usec) < td->start_time_us) {
      dt_sec -= 1;
      dt_usec = td->start_time_us - (*usec);
    }
    else {
      dt_usec = (*usec) - td->start_time_us;
    }
    td->time_s_f = (double)dt_sec + (0.001 * 0.001 * ((double)dt_usec));

    free(psec);
    free(pusec);

    
    fprintf(log_fp, "%lu.%06lu,", (*sec), (*usec));
    for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
      fprintf(log_fp, "%lu,", td->adc[channel]);
    }
    fprintf(log_fp, "%u", td->last_ca);
    fprintf(log_fp, "%f\n", (*td->temperature));

    
    nsleep(100000);
  }

  fclose(log_fp);
  pthread_exit(0);
  
  return NULL;
}
