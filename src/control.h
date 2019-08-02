#pragma once




#include "run.h"




#define SPD_HIST 10
#define ERR_HIST 10
#define CONTROL_MAXIMUM 1024
#define CONTROL_MINIMUM 200




struct control_params {
  // constant
  unsigned int c;
  
  // pid
  double kp;
  double ki;
  double kd;
  double set_point;
  unsigned int is_stress_controlled;

  // sin
  double period;
  double magnitude;
  double mean;

  // bistable
  // also period
  unsigned int lower;
  unsigned int upper;
  
  // universal
  unsigned int sleep_ns;
};



typedef unsigned int (*control_func_t)(struct run_data *);




double control_PID(double tuning[3], double input);
int ctlidx_from_str(const char *s);
void *ctl_thread_func(void *vtd);
void read_control_scheme(struct run_data *rd, const char *control_scheme_string);
void control_help(void);




// vim: ft=c
