#pragma once

#include "../run/run.h"

#define SPD_HIST 10
#define ERR_HIST 10
#define CONTROL_MAXIMUM 1024
#define CONTROL_MINIMUM 0

typedef unsigned int (*control_func_t)(struct run_data *);
typedef double (*setter_func_t)(struct run_data *);


typedef struct {
  const char *doc;
  unsigned int (*get_control_action)(struct run_data *rd);
  void *handle;
} ControllerHandle;


typedef struct {
  const char *doc;
  double (*get_setpoint)(struct run_data *rd);
  void *handle;
} SetterHandle;



struct control_params {

  /* Control information */
  double *control_params;
  int n_control_params;
  unsigned int is_stress_controlled;
  ControllerHandle *controller;


  /* Setter information */
  double *setter_params;
  int n_setter_params;
  double setpoint;
  SetterHandle *setter;

  /* universal */
  unsigned int sleep_ms;
};







double control_PID(double tuning[3], double input);

control_func_t ctlfunc_from_str(char *s);
setter_func_t setfunc_from_str(char *s);
int ctlidx_from_str(const char *s);
int setidx_from_str(const char *s);

void *ctl_thread_func(void *vtd);
void read_control_scheme(struct run_data *rd, const char *control_scheme_string);
void control_help(void);
void update_setpoint(struct run_data *rd);
void do_tuning(struct run_data *rd);

double get_control_param_or_default(struct run_data *rd, int index, double def);
double get_setter_param_or_default(struct run_data *rd, int index, double def);

ControllerHandle *load_controller(const char *name);
ControllerHandle *load_controller_path(const char *path);
void free_controller(ControllerHandle *h);
SetterHandle *load_setter(const char *name);
SetterHandle *load_setter_path(const char *path);
void free_setter(SetterHandle *h);


// vim: ft=c
