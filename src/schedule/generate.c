#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../util/range.h"
#include "../util/double_array.h"

#include "schedule.h"


void append_argset(ArgSet *as,
    char *controller_name, double *controller_params, int n_controller_params,
    char *setter_name, double *setter_params, int n_setter_params,
    int each_run_length)
{

  // --controller "name" --controller_params 1,2,3 --setter "name" --setter_params 1,2,3 [additional required params -d,-l,and -w]
  int argc = 8;

  if (!n_controller_params)
    argc -= 2;

  if (!n_setter_params)
    argc -= 2;

  ArgList *al = arglist_new();
  arglist_add(al, "--controller");
  arglist_add(al, controller_name);

  if (n_controller_params) {
    arglist_add(al, "--controller-params");
    char *s = darr2str(controller_params, n_controller_params);
    arglist_add(al, s);
    free(s);
  }

  arglist_add(al, "--setter");
  arglist_add(al, setter_name);

  if (n_setter_params) {
    arglist_add(al, "--setter-params");
    char *s = darr2str(setter_params, n_setter_params);
    arglist_add(al, s);
    free(s);
  }

  if (each_run_length > 0) {
    arglist_add(al, "--length");
    char *each_run_length_carr = calloc(10, sizeof(char));
    snprintf(each_run_length_carr, 9, "%d", each_run_length);
    arglist_add(al, each_run_length_carr);
  }

  argset_add(as, al);
}


ArgSet *generate_schedule(struct schedule_data *sd)
{

  double **controller_interp = NULL;
  int n_controller_interp = 0;
  int controller_interp_needs_free = 0;
  int n_controller_params = 0;

  double **setter_interp = NULL;
  int n_setter_interp = 0;
  int setter_interp_needs_free = 0;
  int n_setter_params = 0;

  int each_run_length = sd->each_run_length;

  ArgSet *rv = argset_new();

  for (int cname_i = 0; cname_i < sd->n_controllers; cname_i++) {
    char *controller = sd->controller_names[cname_i];

    if ((sd->n_controllers > cname_i+1) &&
        (sd->n_controller_params[cname_i] == sd->n_controller_params[cname_i+1]) &&
        (strcmp(sd->controller_names[cname_i], sd->controller_names[cname_i+1]) == 0)) {
      controller_interp = linear_interp_vector(sd->controller_params[cname_i], sd->controller_params[cname_i+1], sd->n_controller_params[cname_i], sd->n_interpolation_points);
      n_controller_params = sd->n_controller_params[cname_i];
      n_controller_interp = sd->n_interpolation_points;
      controller_interp_needs_free = 1;
    }
    else {
      controller_interp = malloc(sizeof(double*));
      controller_interp[0] = sd->controller_params[cname_i];
      n_controller_params = sd->n_controller_params[cname_i];
      n_controller_interp = 1;
      controller_interp_needs_free = 0;
    }

    for (int ctrl_i = 0; ctrl_i < n_controller_interp; ctrl_i++) {
      for (int sname_i = 0; sname_i < sd->n_setters; sname_i++) {

        char *setter = sd->setter_names[sname_i];

        if ((sd->n_setters > sname_i+1) &&
            (sd->n_setter_params[sname_i] == sd->n_setter_params[sname_i+1]) &&
            (strcmp(sd->setter_names[sname_i], sd->setter_names[sname_i+1]) == 0)) {
          setter_interp = linear_interp_vector(sd->setter_params[sname_i], sd->setter_params[sname_i+1], sd->n_setter_params[sname_i], sd->n_interpolation_points);
          n_setter_params = sd->n_setter_params[sname_i];
          n_setter_interp = sd->n_interpolation_points;
          setter_interp_needs_free = 1;
        }
        else {
          setter_interp = malloc(sizeof(double*));
          setter_interp[0] = sd->setter_params[sname_i];
          n_setter_params = sd->n_setter_params[sname_i];
          n_setter_interp = 1;
          setter_interp_needs_free = 0;
        }

        for (int str_i = 0; str_i < n_setter_interp; str_i++) {

          append_argset(rv,
              controller, controller_interp[ctrl_i], n_controller_params,
              setter, setter_interp[str_i], n_setter_params,
              each_run_length);

        }


        if (n_setter_interp) {
          if (setter_interp_needs_free) {
            for (int i = 0; i < n_setter_interp; i++) {
              free(setter_interp[i]);
            }
          }
          free(setter_interp);
        }

      }
    }

    if (n_controller_interp) {
      if (controller_interp_needs_free) {
        for (int i = 0; i < n_controller_interp; i++) {
          free(controller_interp[i]);
        }
      }
      free(controller_interp);
    }

  }

  return rv;
}
