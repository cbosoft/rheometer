#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../util/double_array.h"
#include "../util/json.h"
#include "../util/error.h"
#include "../args/args.h"
#include "../defaults.h"
#include "schedule.h"


struct schedule_data *get_default_schedule_data()
{
  struct schedule_data *sd = calloc(1, sizeof(struct schedule_data));

  sd->controller_names = NULL;
  sd->controller_params = NULL;
  sd->n_controller_params = 0;

  sd->setter_names = NULL;
  sd->setter_params = NULL;
  sd->n_setter_params = 0;

  sd->interpolation_type = IT_Linear;
  sd->n_interpolation_points = INTERP_N_DEFAULT;
  sd->each_run_length = -1;
  return sd;
}


void free_schedule_data(struct schedule_data *sd)
{
  for (int i = 0; i < sd->n_controllers; i++) {
    if (sd->controller_names[i])
      free(sd->controller_names[i]);

    if (sd->controller_params[i])
      free(sd->controller_params[i]);
  }

  if (sd->controller_names) {
    free(sd->controller_names);
    free(sd->controller_params);
    free(sd->n_controller_params);
  }

  for (int i = 0; i < sd->n_setters; i++) {
    if (sd->setter_names[i])
      free(sd->setter_names[i]);

    if (sd->setter_params[i])
      free(sd->setter_params[i]);
  }

  if (sd->setter_names) {
    free(sd->setter_names);
    free(sd->setter_params);
    free(sd->n_setter_params);
  }
}



void sd_add_controller(struct schedule_data *sd, const char *name, double *params, int nparams)
{
  int n = ++sd->n_controllers;
  sd->controller_names = realloc(sd->controller_names, n*sizeof(char *));

  sd->controller_names[n-1] = strdup(name);

  sd->controller_params = realloc(sd->controller_params, n*sizeof(double *));
  sd->controller_params[n-1] = darr_copy(params, nparams);

  sd->n_controller_params = realloc(sd->n_controller_params, n*sizeof(int));
  sd->n_controller_params[n-1] = nparams;
}


void sd_add_setter(struct schedule_data *sd, const char *name, double *params, int nparams)
{
  int n = ++sd->n_setters;
  sd->setter_names = realloc(sd->setter_names, n*sizeof(char *));

  sd->setter_names[n-1] = strdup(name);

  sd->setter_params = realloc(sd->setter_params, n*sizeof(double *));
  sd->setter_params[n-1] = darr_copy(params, nparams);

  sd->n_setter_params = realloc(sd->n_setter_params, n*sizeof(int));
  sd->n_setter_params[n-1] = nparams;
}

void sd_add_params(struct schedule_data *sd, cJSON *json)
{
  cJSON *npoints_json = cJSON_GetObjectItem(json, "n_interpolation_points");
  if (npoints_json != NULL) {
    if (cJSON_IsNumber(npoints_json)) {
      sd->n_interpolation_points = npoints_json->valueint;
    }
    else {
      ferr("schedule_add_params", "\"n_interpolation_points\" must be a number.");
    }
  }

  cJSON *int_type_json = cJSON_GetObjectItem(json, "interpolation_type");
  if (int_type_json != NULL) {
    if (cJSON_IsString(int_type_json)) {
      char *s = int_type_json->valuestring;
      if (strcmp(s, "linear") == 0) {
        sd->interpolation_type = IT_Linear;
      }
      else if (strcmp(s, "log") == 0) {
        sd->interpolation_type = IT_Log;
      }
      else {
        ferr("schedule_add_params", "\"interpolation_type\" must be either \"linear\" or \"log\".");
      }
    }
    else {
      ferr("schedule_add_params", "\"interpolation_type\" must be either \"linear\" or \"log\".");
    }
  }

  cJSON *each_run_length_json = cJSON_GetObjectItem(json, "each_run_length");
  if (each_run_length_json != NULL) {
    if (cJSON_IsNumber(each_run_length_json)) {
      sd->each_run_length = each_run_length_json->valueint;
    }
    else if (cJSON_IsString(each_run_length_json)) {
      char *s = each_run_length_json->valuestring;
      sd->each_run_length = (int)(parse_length_string(s));
    }
    else {
      ferr("schedule_add_params", "\"each_run_length\" must be a number.");
    }
  }

}


void sd_set_interpolation(struct schedule_data *sd, InterpolationType type, int n)
{
  sd->interpolation_type = type;
  sd->n_interpolation_points = n;
}

static cJSON *safe_get(cJSON *obj, const char *s, int only_warn)
{
  cJSON *rv = cJSON_GetObjectItem(obj, s);
  const char *fmt = "%s expected, but not specified in schedule json.";

  if (!rv) {
    if (only_warn)
      warn("schedule_add_from_file", fmt, s);
    else
      ferr("schedule_add_from_file", fmt, s);
  }

  return rv;
}

static const char *sd_json_get_name(cJSON *json)
{
  cJSON *name_json = safe_get(json, "name", 0);

  if (cJSON_IsString(name_json)) {
    return name_json->valuestring;
  }
  else {
    ferr("schedule_add_from_file", "schedule point name should be a string.");
    return NULL;
  }
}

static ScheduleJsonObjectType sd_json_get_type(cJSON *json)
{
  cJSON *type_json = safe_get(json, "type", 0);
  
  if (cJSON_IsString(type_json)) {
    const char *s = type_json->valuestring;

    if (strcmp(s, "controller") == 0) {
      return SD_JSON_CONTROLLER;
    }
    else if (strcmp(s, "setter") == 0) {
      return SD_JSON_SETTER;
    }
    else if (strcmp(s, "params") == 0) {
      return SD_JSON_PARAMS;
    }
    else {
      ferr("schedule_add_from_file", "schedule point type should be \"controller\", \"setter\" or \"params\", not %s", s);
      return SD_JSON_UNKNOWN;
    }
  }

  ferr("schedule_add_from_file", "schedule point type should be a string.");
  return SD_JSON_UNKNOWN;
}

static void sd_json_maybe_get_params(cJSON *json, double **arr_ptr, int *n_ptr)
{
  cJSON *params_json = safe_get(json, "params", 1);
  if (cJSON_IsArray(params_json)) {
    cJSON *elem_json = NULL;
    cJSON_ArrayForEach(elem_json, params_json) {
      darr_append(arr_ptr, n_ptr, elem_json->valuedouble);
    }
  }
  else if (cJSON_IsNumber(params_json)) {
    darr_append(arr_ptr, n_ptr, params_json->valuedouble);
  }
}

void sd_add_from_json(struct schedule_data *sd, cJSON *json)
{
  if (cJSON_IsArray(json)) {
    cJSON *elem_json = NULL;
    cJSON_ArrayForEach(elem_json, json) {
      sd_add_from_json(sd, elem_json);
    }
  }
  else {
    const char *name = NULL;
    double *params = NULL;
    int n_params = 0;

    ScheduleJsonObjectType type = sd_json_get_type(json);


    switch (type) {
      case SD_JSON_CONTROLLER:
        name = sd_json_get_name(json);
        sd_json_maybe_get_params(json, &params, &n_params);
        sd_add_controller(sd, name, params, n_params);
        break;

      case SD_JSON_SETTER:
        name = sd_json_get_name(json);
        sd_json_maybe_get_params(json, &params, &n_params);
        sd_add_setter(sd, name, params, n_params);
        break;

      case SD_JSON_PARAMS:
        sd_add_params(sd, json);
        break;

      case SD_JSON_UNKNOWN:
        warn("schedule_add_from_file", "Unknown schedule point found.");
        break;

    }

  }
}

void sd_add_from_file(struct schedule_data *sd, const char *path)
{
  cJSON *json = read_json(path);

  if (!json) {
    argerr("Error loading schedule json \"%s\"", path);
  }

  sd_add_from_json(sd, json);

}
