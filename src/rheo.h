#pragma once
#include <stdint.h>

#define VERSION "0.1"
#define ADC_COUNT 8
#define OPTENC_COUNT 3

//const int optenc_pins[OPTENC_COUNT] = {16, 20, 21};

typedef enum control_scheme {constant, pid} control_scheme_enum;

typedef struct run_data {
  unsigned int length_s;
  control_scheme_enum control_scheme;
  const char *tag;
} run_data;

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

  // control stuff
  adc_handle *adc_h;
  run_data *run_d;
  uint8_t log_ready;
  uint8_t adc_ready;
  uint8_t tmp_ready;
  uint8_t opt_ready;
  uint8_t stopped;
  uint8_t errored;
  const char *error_string;

} thread_data;



// thread.c {{{

thread_data *new_thread_data();
void nsleep(unsigned int delay_ns);
void free_thread_data(thread_data *dat);

// }}}
// log.c {{{ 

void *log_thread_func(void *rt_d);

// }}}
// error.c {{{

void ferr(const char *mesg);
void warn(const char *mesg);

// }}}
// adc.c {{{

adc_handle *adc_open(const char *device);
void adc_close(adc_handle *h);
unsigned int read_adc_value(adc_handle *h, unsigned int channel);
void *adc_thread_func(void *rt_d);

// }}}
// control.c {{{

double control_PID(double tuning[3], double input);

// }}}
// args.h {{{

run_data *parse_args(int argc, const char **argv);
void free_run_data(run_data *r_d);

// }}}
// vim: foldmethod=marker ft=c
