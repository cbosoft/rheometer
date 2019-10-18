#pragma once




#include "run.h"




#define SPD_HIST 10
#define ERR_HIST 10
#define CONTROL_MAXIMUM 1024
#define CONTROL_MINIMUM 200

typedef unsigned int (*control_func_t)(struct run_data *);
typedef double (*setter_func_t)(struct run_data *);



struct control_params {

  /* Control information */
  // pid
  double kp;
  double ki;
  double kd;
  double setpoint;
  unsigned int is_stress_controlled;

  // none
  double mult;

  /* Setter information */
  // constant
  double c;

  // sin
  double period;
  double magnitude;
  double mean;

  // bistable
  // also period
  double lower;
  double upper;
  
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




// vim: ft=c
