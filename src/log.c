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
#include "thermometer.h"
#include "loadcell.h"
#include "uid.h"
#include "version.h"


#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define CHECKJSON(C) if (C == NULL) { warn("CHECKJSON", "Error creating run params JSON. \n  Write down the fill depth for this run!"); return; }

extern pthread_mutex_t lock_time, lock_adc, lock_control, lock_loadcell, lock_temperature;




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

  rd->uid = get_uid();

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



void set_time(struct run_data *rd)
{
  struct timeval tv;
  unsigned long sec, usec;

  gettimeofday(&tv, 0);
  sec = tv.tv_sec;
  usec = tv.tv_usec;

  pthread_mutex_lock(&lock_time);
  rd->time_s = sec;
  rd->time_us = usec;
  pthread_mutex_unlock(&lock_time);

  unsigned long dt_sec = sec - rd->start_time_s, dt_usec;
  if (usec < rd->start_time_us) {
    dt_sec -= 1;
    dt_usec = rd->start_time_us - usec;
  }
  else {
    dt_usec = usec - rd->start_time_us;
  }

  pthread_mutex_lock(&lock_time);
  rd->time_s_f = (double)dt_sec + (0.001 * 0.001 * ((double)dt_usec));
  pthread_mutex_unlock(&lock_time);
}




void *log_thread_func(void *vptr) {

  struct run_data *rd = (struct run_data *)vptr;

  if (rd->log_pref == NULL)
    ferr("log_thread_func", "data must be initialised before logging is started.");

  int log_idx = add_log(rd, "log", "%s.csv", rd->log_pref);

  FILE *log_fp = fopen(rd->log_paths[log_idx], "w");

  rd->log_ready = 1;
  while ( (!rd->stopped) && (!rd->errored) ) {
    set_time(rd);
    pthread_mutex_lock(&lock_time);
    fprintf(log_fp, "%lu.%06lu,", rd->time_s, rd->time_us);
    pthread_mutex_unlock(&lock_time);

    pthread_mutex_lock(&lock_adc);
    for (unsigned int channel = 0; channel < ADC_COUNT; channel++)
      fprintf(log_fp, "%lu,", rd->adc[channel]);
    pthread_mutex_unlock(&lock_adc);

    pthread_mutex_lock(&lock_control);
    fprintf(log_fp, "%u,", rd->last_ca);
    pthread_mutex_unlock(&lock_control);

    pthread_mutex_lock(&lock_temperature);
    fprintf(log_fp, "%f,", rd->cylinder_temperature);
    pthread_mutex_unlock(&lock_temperature);

    pthread_mutex_lock(&lock_loadcell);
    fprintf(log_fp, "%lu,", rd->loadcell_bytes);
    pthread_mutex_unlock(&lock_loadcell);

    fprintf(log_fp, "%d\n", rd->phase);
    
    sleep_us(900);
  }

  fclose(log_fp);
  pthread_exit(0);

  return NULL;
}
