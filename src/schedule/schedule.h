#pragma once

#include "argset.h"

typedef enum {IT_Linear, IT_Log} InterpolationType;
typedef enum {SD_JSON_CONTROLLER, SD_JSON_SETTER, SD_JSON_PARAMS, SD_JSON_UNKNOWN} ScheduleJsonObjectType;

struct schedule_point {
  char *controller_name;
  char *setter_name;
  double *controller_params;
  double *setter_params;
  int n_controller_params;
  int n_setter_params;
};

struct schedule_data {
  char **controller_names;
  double **controller_params;
  int *n_controller_params;
  int n_controllers;

  char **setter_names;
  double **setter_params;
  int *n_setter_params;
  int n_setters;

  // settings
  InterpolationType interpolation_type;
  int n_interpolation_points;
  ArgList *extra_arguments;
};


struct schedule_data *get_default_schedule_data();
void free_schedule_data(struct schedule_data *sd);
ArgSet *generate_schedule(struct schedule_data *sd);

void sd_add_from_file(struct schedule_data *sd, const char *path);
void sd_add_controller(struct schedule_data *sd, const char *name, double *params, int nparams);
void sd_add_setter(struct schedule_data *sd, const char *name, double *params, int nparams);
void sd_set_interpolation(struct schedule_data *sd, InterpolationType type, int n);
