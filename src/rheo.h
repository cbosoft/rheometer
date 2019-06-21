#pragma once
// macros {{{

#define VERSION "0.1"
#define ADC_COUNT 8
#define OPTENC_COUNT 3

// }}}
// types and enums {{{

typedef enum control_scheme {constant, pid} control_scheme_enum;

// }}}
// structs {{{

typedef struct adc_handle {
  const char *device;
  int fd;
  unsigned int mode;
  unsigned int bits;
  unsigned int speed;
  unsigned int delay;
} adc_handle;

typedef struct thread_data{

  // actual data
  unsigned long *time_s;
  unsigned long *time_us;
  unsigned long *adc;
  float *temperature;
  float *speed_ind;

  // run_data
  unsigned int length_s;
  control_scheme_enum control_scheme;
  const char *tag;
  const char *log_pref;
  char **log_paths;
  unsigned int log_count;

  // control stuff
  adc_handle *adc_h;
  unsigned int log_ready;
  unsigned int adc_ready;
  unsigned int tmp_ready;
  unsigned int opt_ready;
  unsigned int stopped;
  unsigned int errored;
  const char *error_string;

} thread_data;


// }}}
// thread.c {{{

thread_data *init(int argc, const char** argv);
void nsleep(unsigned int delay_ns);
void free_thread_data(thread_data *dat);

// }}}
// log.c {{{ 

void *log_thread_func(void *rtd);

// }}}
// error.c {{{

void ferr(const char *mesg);
void warn(const char *mesg);

// }}}
// adc.c {{{

adc_handle *adc_open(const char *device);
void adc_close(adc_handle *h);
unsigned int read_adc_value(adc_handle *h, unsigned int channel);
void *adc_thread_func(void *rtd);

// }}}
// opt.c {{{

void *opt_thread_func(void *rtd);

// }}}
// control.c {{{

double control_PID(double tuning[3], double input);

// }}}
// args.h {{{

void parse_args(int argc, const char **argv, thread_data *td);

// }}}
// vim: foldmethod=marker ft=c
