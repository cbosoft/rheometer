#pragma once

typedef enum {IT_Linear} InterpolationType;

struct schedule_data {
  // define by file maybe?
  char *schedule_file_path;

  // direct definition
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
};


struct schedule_data *get_default_schedule_data();
void free_schedule_data(struct schedule_data *sd);
void generate_schedule(struct schedule_data *sd, char ****vargv, int **vargc, int *meta_argc);


void add_head_tail_to_argset(char ****vargv, int **vargc, int margc,
    char **headv, int headc, char **tailv, int tailc);
void free_argset(char ***vargv, int *vargc, int margc);

void sd_add_controller(struct schedule_data *sd, const char *name, double *params, int nparams);
void sd_add_setter(struct schedule_data *sd, const char *name, double *params, int nparams);
void sd_set_interpolation(struct schedule_data *sd, InterpolationType type, int n);
