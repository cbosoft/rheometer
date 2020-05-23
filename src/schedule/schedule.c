#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>

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
  sd->n_interpolation_points = 5;
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
  sd->controller_params[n-1] = params;

  sd->n_controller_params = realloc(sd->n_controller_params, n*sizeof(int));
  sd->n_controller_params[n-1] = nparams;
}


void sd_add_setter(struct schedule_data *sd, const char *name, double *params, int nparams)
{
  int n = ++sd->n_setters;
  sd->setter_names = realloc(sd->setter_names, n*sizeof(char *));

  sd->setter_names[n-1] = strdup(name);

  sd->setter_params = realloc(sd->setter_params, n*sizeof(double *));
  sd->setter_params[n-1] = params;

  sd->n_setter_params = realloc(sd->n_setter_params, n*sizeof(int));
  sd->n_setter_params[n-1] = nparams;
}


void sd_set_interpolation(struct schedule_data *sd, InterpolationType type, int n)
{
  sd->interpolation_type = type;
  sd->n_interpolation_points = n;
}
