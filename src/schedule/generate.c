#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "schedule.h"


char *darr2str(double *darr, int n)
{
  const int char_per_float = 12;
  char *rv = malloc((char_per_float*n + 1)*sizeof(char));
  rv[0] = 0;

  for (int i = 0; i < n; i++) {
    char *p = rv + (char_per_float+1)*i;

    char *fc = calloc(char_per_float+2, sizeof(char));
    snprintf(fc, char_per_float+1, "%f,", darr[i]);
    strncat(p, fc, char_per_float+1);
    free(fc);
  }

  rv = realloc(rv, (strlen(rv)+1)*sizeof(char));
  return rv;
}

void append_argset(char ****vargv, int **vargc, int *margc,
    char *controller_name, double *controller_params, int n_controller_params,
    char *setter_name, double *setter_params, int n_setter_params)
{

  // --controller "name" --controller_params 1,2,3 --setter "name" --setter_params 1,2,3 [additional required params -d,-l,and -w]
  int argc = 8;

  if (!n_controller_params)
    argc -= 2;

  if (!n_setter_params)
    argc -= 2;

  char **new_argset = calloc(argc, sizeof(char*));
  int i = 0;
  new_argset[i++] = strdup("--controller");
  new_argset[i++] = strdup(controller_name);
  if (n_controller_params) {
    new_argset[i++] = strdup("--controller-params");
    new_argset[i++] = darr2str(controller_params, n_controller_params);
  }
  new_argset[i++] = strdup("--setter");
  new_argset[i++] = strdup(setter_name);
  if (n_setter_params) {
    new_argset[i++] = strdup("--setter-params");
    new_argset[i++] = darr2str(setter_params, n_setter_params);
  }

  int n = ++(*margc);
  (*vargv) = realloc(*vargv, n*sizeof(char**));
  (*vargv)[(*margc)-1] = new_argset;

  (*vargc) = realloc(*vargc, n*sizeof(int));
  (*vargc)[(*margc)-1] = argc;
}


void generate_schedule(struct schedule_data *sd, char ****vargv,
    int **vargc, int *meta_argc)
{

  double **controller_interp = NULL;
  int n_controller_interp = 0;
  int controller_interp_needs_free = 0;
  int n_controller_params = 0;

  double **setter_interp = NULL;
  int n_setter_interp = 0;
  int setter_interp_needs_free = 0;
  int n_setter_params = 0;

  for (int cname_i = 0; cname_i < sd->n_controllers; cname_i++) {
    char *controller = sd->controller_names[cname_i];

    if ((sd->n_controllers > cname_i+1) &&
        (strcmp(sd->controller_names[cname_i], sd->controller_names[cname_i+1]) == 0)) {
      // TODO controller interp here
      // n is number of interpsteps MINUS ONE as the last step is the start of
      // the next loop
    }
    else {
      controller_interp = malloc(sizeof(double*));
      controller_interp[0] = sd->controller_params[0];
      n_controller_params = sd->n_controller_params[0];
      n_controller_interp = 1;
      controller_interp_needs_free = 0;
    }

    for (int ctrl_i = 0; ctrl_i < n_controller_interp; ctrl_i++) {
      for (int sname_i = 0; sname_i < sd->n_setters; sname_i++) {

        char *setter = sd->setter_names[sname_i];

        if ((sd->n_setters > sname_i+1) &&
            (strcmp(sd->setter_names[sname_i], sd->setter_names[sname_i+1]) == 0)) {
          // TODO setter interp here
          // n is number of interpsteps MINUS ONE as the last step is the start of
          // the next loop
        }
        else {
          setter_interp = malloc(sizeof(double*));
          setter_interp[0] = sd->setter_params[0];
          n_setter_params = sd->n_setter_params[0];
          n_setter_interp = 1;
          setter_interp_needs_free = 0;
        }

        for (int str_i = 0; str_i < n_setter_interp; str_i++) {

          append_argset(vargv, vargc, meta_argc,
              controller, controller_interp[ctrl_i], n_controller_params,
              setter, setter_interp[str_i], n_setter_params);

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

}
