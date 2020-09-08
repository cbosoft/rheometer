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
  sd->extra_arguments = arglist_new();
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

  cJSON *elem;
  cJSON_ArrayForEach(elem, json) {
    char *key = elem->string;

    if (strcmp(key, "type") == 0) {
      // do nothing
    }
    else if (strcmp(key, "interpolation_type") == 0) {
      if (cJSON_IsString(elem)) {
        char *s = elem->valuestring;
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
    else if (strcmp(key, "n_interpolation_points") == 0) {
      if (cJSON_IsNumber(elem)) {
        sd->n_interpolation_points = elem->valueint;
      }
      else {
        ferr("schedule_add_params", "\"n_interpolation_points\" must be a number.");
      }
    }
    else {
      // extra arguments
      char *arg = NULL;
      int arg_needs_free = 0, arg_okay = 0;

      if (cJSON_IsString(elem)) {
        arg = elem->valuestring;
        arg_okay = 1;
      }
      else if (cJSON_IsNumber(elem)) {
        arg = calloc(10, sizeof(char));
        arg_needs_free = 1;
        snprintf(arg, 9, "%f", elem->valuedouble);
        arg_okay = 1;
      }

      if (arg_okay) {
        int argnamelen = strlen(key)+3;
        char *argname = calloc(argnamelen+1, sizeof(char));
        snprintf(argname, argnamelen, "--%s", key);

        arglist_add(sd->extra_arguments, argname);
        arglist_add(sd->extra_arguments, arg);

        free(argname);
      }

      if (arg_needs_free) {
        free(arg);
      }

    }
  }

  //ferr("schedule.c", "DEBUG EXIT");

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
      double dv;
      int okay = 0;

      if (cJSON_IsNumber(elem_json)) {
        dv = elem_json->valuedouble;
        okay = 1;
      }
      else {
        warn("setter_add_from_file", "Unexpected param type: params should be numbers."); 
      }

      if (okay)
        darr_append(arr_ptr, n_ptr, dv);
    }
  }
  else if (cJSON_IsNumber(params_json)) {
    darr_append(arr_ptr, n_ptr, params_json->valuedouble);
  }
  else if (cJSON_IsString(params_json)) {
    char *s = params_json->valuestring;
    int l = strlen(s);
    // TODO properly check if is number followed by '%' (use regex)
    if (s[l-1] == '%') {
      double perc = atof(params_json->valuestring);
      double dv = perc*0.01*1024.0;
      darr_append(arr_ptr, n_ptr, dv);
    }
    else {
      warn("setter_add_from_file", "Unexpected string found, and is not a percentage: params should be numbers."); 
    }
  }
  else {
    warn("setter_add_from_file", "Unexpected param type: params should be numbers."); 
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
