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

#include "../run/run.h"
#include "../control/control.h"
#include "../util/json.h"
#include "../util/sleep.h"
#include "../util/error.h"
#include "../sensors/adc/adc.h"
#include "../sensors/loadcell/loadcell.h"
#include "../sensors/thermometer/thermometer.h"
#include "../util/unique_name.h"
#include "../version.h"
#include "../defaults.h"

#include "log.h"


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
  strftime(date, 14, "%Y-%m-%d", timeinfo);

  char *pattern = calloc(256, sizeof(char));
  sprintf(pattern, "%s/%s_%s_*_%s.csv", log_dir, genpref, date, rd->tag);

  glob_t glob_res;
  glob((const char *)pattern, GLOB_NOSORT, NULL, &glob_res);
  free(pattern);

  char *log_pref = calloc(256, sizeof(char));
  sprintf(log_pref, "%s/%s_%s_%03u_%s", log_dir, genpref, date, (unsigned int)glob_res.gl_pathc, rd->tag);
  rd->log_pref = log_pref;

  rd->uid = get_unique_name();

  free(date);

  struct timeval tv;
  gettimeofday(&tv, 0);
  rd->start_time_s = tv.tv_sec;
  rd->start_time_us = tv.tv_usec;

}


cJSON *construct_save_control_scheme_json(struct run_data *rd)
{
  cJSON *control_scheme_json = cJSON_CreateObject();
  cJSON_AddStringToObject(control_scheme_json, "control", rd->control_scheme.controller_name);
  cJSON_AddStringToObject(control_scheme_json, "setter", rd->control_scheme.setter_name);

  if (rd->control_scheme.n_control_params) {
    cJSON *control_params_json = cJSON_CreateDoubleArray(rd->control_scheme.control_params, rd->control_scheme.n_control_params);
    cJSON_AddItemToObject(control_scheme_json, "control_params", control_params_json);
  }

  if (rd->control_scheme.n_setter_params) {
    cJSON *setter_params_json = cJSON_CreateDoubleArray(rd->control_scheme.setter_params, rd->control_scheme.n_setter_params);
    cJSON_AddItemToObject(control_scheme_json, "setter_params", setter_params_json);
  }

  char *date = calloc(15, sizeof(char));
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(date, 14, "%Y-%m-%d", timeinfo);

  char *scheme_path = calloc(300, sizeof(char));
  snprintf(scheme_path, 299, "data/generated_%s_%s_%s_*_%s.json",
      rd->control_scheme.controller_name,
      rd->control_scheme.setter_name,
      date,
      rd->tag);

  glob_t glob_res;
  glob((const char *)scheme_path, GLOB_NOSORT, NULL, &glob_res);

  snprintf(scheme_path, 299, "data/generated_%s_%s_%s_%d_%s.json",
      rd->control_scheme.controller_name,
      rd->control_scheme.setter_name,
      date,
      (int)glob_res.gl_pathc,
      rd->tag);

  FILE *f = fopen(scheme_path, "w");
  char *contents = cJSON_Print(control_scheme_json);
  fprintf(f, "%s\n", contents);
  fclose(f);
  free(contents);
  free(date);

  return control_scheme_json;
}



void save_run_params_to_json(struct run_data *rd)
{
  cJSON *params = cJSON_CreateObject();
  CHECKJSON(params);

  cJSON *control_scheme_json = construct_save_control_scheme_json(rd);

  CHECKJSON(control_scheme_json);
  cJSON_AddItemToObject(params, "control_scheme", control_scheme_json);

  cJSON *length_s_json = cJSON_CreateNumber(rd->length_s);
  CHECKJSON(length_s_json);
  cJSON_AddItemToObject(params, "length_s", length_s_json);

  cJSON *fill_depth_mm_json = cJSON_CreateNumber(rd->fill_depth);
  CHECKJSON(fill_depth_mm_json);
  cJSON_AddItemToObject(params, "fill_depth_mm", fill_depth_mm_json);

  if (rd->needle_depth >= 0.0) {
    cJSON *needle_depth_mm_json = cJSON_CreateNumber(rd->needle_depth);
    CHECKJSON(needle_depth_mm_json);
    cJSON_AddItemToObject(params, "needle_depth_mm", needle_depth_mm_json);
  }

  cJSON *software_version = cJSON_CreateString(VERSION);
  CHECKJSON(software_version);
  cJSON_AddItemToObject(params, "software_version", software_version);

  cJSON *hardware_version = cJSON_CreateNumber(rd->hardware_version);
  CHECKJSON(hardware_version);
  cJSON_AddItemToObject(params, "hardware_version", hardware_version);

  if (rd->video_device != NULL) {
    info("video_dev");
    cJSON *video_device = cJSON_CreateString(rd->video_device);
    CHECKJSON(video_device);
    cJSON_AddItemToObject(params, "video_device", video_device);

    info("video_start");
    cJSON *video_start_json = cJSON_CreateNumber(rd->cam_start);
    CHECKJSON(video_start_json);
    cJSON_AddItemToObject(params, "video_start", video_start_json);

    info("video_end");
    cJSON *video_end_json = cJSON_CreateNumber(rd->cam_end);
    CHECKJSON(video_end_json);
    cJSON_AddItemToObject(params, "video_end", video_end_json);
  }

  info("saving");
  char *params_json_str = cJSON_Print(params);
  char *params_path = calloc(300, sizeof(char));
  sprintf(params_path, "%s_runparams.json", rd->log_pref);
  add_log(rd, "%s", params_path);

  FILE *fp = fopen(params_path, "w");
  fprintf(fp, "%s\n", params_json_str);
  fclose(fp);

  free(params_path);
  free(params_json_str);
  cJSON_Delete(params);
}




int add_log(struct run_data *rd, const char* name, const char* fmt, ...)
{
  if (rd->log_paths == NULL) {
    rd->log_paths = malloc(sizeof(char*));
    rd->log_names = malloc(sizeof(char*));
  }
  else {
    rd->log_paths = realloc(rd->log_paths, (rd->log_count + 1)*sizeof(char*));
    rd->log_names = realloc(rd->log_names, (rd->log_count + 1)*sizeof(char*));
  }

  rd->log_paths[rd->log_count] = calloc(PATH_MAX, sizeof(char));
  rd->log_names[rd->log_count] = calloc(strlen(name)+1, sizeof(char));

  va_list ap;

  va_start(ap, fmt);
  vsnprintf(rd->log_paths[rd->log_count], PATH_MAX, fmt, ap);
  va_end(ap);

  strcpy(rd->log_names[rd->log_count], name);

  rd->log_count ++;

  return rd->log_count - 1;
}




int remove_log(struct run_data *rd, unsigned int index)
{
  if (index >= rd->log_count) {
    warn("remove_log", "Log index outside of list (%d out of %d)", index, rd->log_count);
  }

  char **log_paths = rd->log_paths, **log_names = rd->log_names;

  rd->log_paths = calloc(rd->log_count-1, sizeof(char*));
  rd->log_names = calloc(rd->log_count-1, sizeof(char*));

  for (unsigned int i = 0, j = 0; i < rd->log_count; i++) {
    
    if (i == index)
      continue;

    rd->log_paths[j] = log_paths[i];
    rd->log_names[j] = log_names[i];

    j++;

  }

  rd->log_count --;

  return 0;
}



void *log_thread_func(void *vptr) {

  struct run_data *rd = (struct run_data *)vptr;

  if (rd->log_pref == NULL)
    ferr("log_thread_func", "data must be initialised before logging is started.");

  int log_idx = add_log(rd, "log", "%s.csv", rd->log_pref);

  FILE *log_fp = fopen(rd->log_paths[log_idx], "w");

  unsigned long time_s, time_us;
  rd->log_ready = 1;
  while ( (!rd->stopped) && (!rd->errored) ) {
    rd_set_time(rd);
    rd_get_time_parts(rd, &time_s, &time_us);
    fprintf(log_fp, "%lu.%06lu,", time_s, time_us);

    for (unsigned int channel = 0; channel < ADC_COUNT; channel++)
      fprintf(log_fp, "%lu,", rd_get_adc(rd, channel));

    fprintf(log_fp, "%u,", rd_get_last_control_action(rd));

    fprintf(log_fp, "%f,", rd_get_cylinder_temperature(rd));
    fprintf(log_fp, "%lu,", rd_get_loadcell_bytes(rd));
    fprintf(log_fp, "%d,", rd->phase);
    fprintf(log_fp, "%f,", rd_get_ambient_temperature(rd));

    fprintf(log_fp, "\n");
    sleep_us(900);
  }

  fclose(log_fp);
  pthread_exit(0);

  return NULL;
}
