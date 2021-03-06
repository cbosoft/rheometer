#pragma once

#include "../run/run.h"

#define SPD_HIST 10
#define ERR_HIST 10
#define CONTROL_MAXIMUM 1024
#define CONTROL_MINIMUM 0

#define GET_CONTROL_PARAM_OR_DEFAULT(rd, I, D)  (((I < rd->control_scheme.n_control_params) && (I >= 0)) ? rd->control_scheme.control_params[I] : D)
#define GET_SETTER_PARAM_OR_DEFAULT(rd, I, D)  (((I < rd->control_scheme.n_setter_params) && (I >= 0)) ? rd->control_scheme.setter_params[I] : D)

typedef unsigned int (*control_func_t)(struct run_data *);
typedef double (*setter_func_t)(struct run_data *);


typedef struct ControllerHandle {
  unsigned int (*get_control_action)(struct run_data *rd);
  void *handle;
  const char *doc;
  const char *name;
  const char *ident;
  int n_params;
  const char **params;
} ControllerHandle;


typedef struct SetterHandle {
  double (*get_setpoint)(struct run_data *rd);
  void *handle;
  const char *doc;
  const char *name;
  const char *ident;
  int n_params;
  const char **params;
} SetterHandle;







void *ctl_thread_func(void *vtd);
void read_control_scheme(struct run_data *rd, const char *control_scheme_string);
void control_help(void);
void update_setpoint(struct run_data *rd);
void do_tuning(struct run_data *rd);

ControllerHandle *load_controller(const char *name);
ControllerHandle *load_controller_path(const char *path);
SetterHandle *load_setter(const char *name);
SetterHandle *load_setter_path(const char *path);


// vim: ft=c
