#pragma once

#include <stdio.h>

// macros {{{

// MISC
#define VERSION "0.1"

// ADC
#define ADC_COUNT 8
#define PND_CHANNEL 1
#define TSTS_CHANNEL 2

// OPT
#define OPTENC_COUNT 3

// GPIO
#define PWM_PIN 18

// CONTROL
#define SPD_HIST 100
#define ERR_HIST 100

// UI
#define BOLD  "\033[1m"
#define RESET "\033[0m"
// FOREGROUND
#define FGBLACK   "\033[30m"
#define FGRED     "\033[31m"
#define FGGREEN   "\033[32m"
#define FGYELLOW  "\033[33m"
#define FGBLUE    "\033[34m"
#define FGMAGENTA "\033[35m"
#define FGCYAN    "\033[36m"
#define FGWHITE   "\033[37m"
// BACKGROUND
#define BGBLACK   "\033[40m"
#define BGRED     "\033[41m"
#define BGGREEN   "\033[42m"
#define BGYELLOW  "\033[43m"
#define BGBLUE    "\033[44m"
#define BGMAGENTA "\033[45m"
#define BGCYAN    "\033[46m"
#define BGWHITE   "\033[47m"

// LOGS
#define TAGDEFAULT "DELME"

// STRESS CAL
#define STRESS_M 1.0
#define STRESS_C 1.0

// GEOMETRY (m)
#define RI 0.01505
#define RO 0.01975

// MATH
#define PI 3.1415926535

// }}}
// types, structs, and enums {{{

typedef enum control_scheme {control_constant, control_pid} control_scheme_enum;

// control scheme {{{

typedef struct adc_handle_t {
  const char *device;
  int fd;
  unsigned int mode;
  unsigned int bits;
  unsigned int speed;
  unsigned int delay;
} adc_handle_t;

// }}}
// control_params_t {{{

typedef struct control_params_t {
  // constant
  double c;
  
  // pid
  double kp;
  double ki;
  double kd;
  double set_point;
  unsigned int is_stress_controlled;
  
  // universal
  unsigned int sleep_ns;
} control_params_t;

// }}}
// thread_data_t {{{

typedef struct thread_data_t {

  // actual data
  unsigned long *time_s;
  unsigned long *time_us;
  unsigned long *adc;
  float *temperature;
  
  // process control stuff
  float speed_ind;
  float strainrate_ind;
  float stress_ind;
  float viscosity_ind;
  float **ptimes;
  char *control_scheme;
  control_params_t *control_params;
  unsigned int last_ca;
  float *errhist;

  // run_data
  unsigned int length_s;
  float fill_depth;
  const char *tag;
  char *log_pref;
  char **log_paths;
  FILE **opt_log_fps;
  unsigned int log_count;

  // program control stuff
  adc_handle_t *adc_handle;
  unsigned int log_ready;
  unsigned int adc_ready;
  unsigned int tmp_ready;
  unsigned int opt_ready;
  unsigned int ctl_ready;

  unsigned int adc_busy;
  
  unsigned int stopped;
  unsigned int errored;
  const char *error_string;

} thread_data_t;

typedef unsigned int (*control_func_t)(thread_data_t *);

// }}}

// }}}
// display.c {{{

char *centre(char *s, unsigned int w);
void display_thread_data(thread_data_t *td);
void display_titles();

// }}}
// thread.c {{{

thread_data_t *create_thread_data(void);
void init(unsigned int argc, const char** argv, thread_data_t *td);
void nsleep(unsigned int delay_ns);
void free_thread_data(thread_data_t *dat);

// }}}
// log.c {{{ 

void save_run_params_to_json(thread_data_t *td);
void *log_thread_func(void *rtd);

// }}}
// error.c {{{

void ferr(const char *mesg);
void argerr(const char *mesg);
void warn(const char *mesg);
void info(const char *mesg);

// }}}
// adc.c {{{

adc_handle_t *adc_open(const char *device);
void adc_close(adc_handle_t *h);
unsigned int read_adc_value(adc_handle_t *h, unsigned int channel);
void *adc_thread_func(void *rtd);

// }}}
// opt.c {{{

void opt_setup(thread_data_t *td);
void opt_mark(thread_data_t *td, unsigned int i);

// }}}
// control.c {{{

double control_PID(double tuning[3], double input);
int ctlidx_from_str(const char *s);
void *ctl_thread_func(void *vtd);
void read_control_scheme(thread_data_t *td, const char *control_scheme_string);

// }}}
// args.c {{{

void parse_args(int argc, const char **argv, thread_data_t *td);
void usage();

// }}}
// motor.c {{{

void motor_setup();
void motor_warmup(unsigned int target);
void motor_shutdown();

// }}}
// tar.c {{{

void tidy_logs(thread_data_t *td);

// }}}
// vim: foldmethod=marker ft=c
