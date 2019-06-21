#pragma once
#include <stdint.h>

#define VERSION "0.1"
#define ADC_COUNT 8
#define OPTENC_COUNT 3

//const int optenc_pins[OPTENC_COUNT] = {16, 20, 21};

// rheo_thread.c {{{

typedef struct thread_data{
  unsigned long *time_s;
  unsigned long *time_us;
  float **adc;
  float *temperature;
  float *speed_ind;
  uint8_t stopped;
  uint8_t errored;
  const char *error_string;
} thread_data;

thread_data *new_thread_data();
void free_thread_data(thread_data *dat);
void *log_thread_func(void *rt_d);

// }}}
// rheo_error.c {{{

void ferr(const char *mesg);
void warn(const char *mesg);

// }}}
// rheo_adc.c {{{

typedef struct adc_handle {
  const char *device;
  int fd;
  unsigned int mode;
  unsigned int bits;
  unsigned int speed;
  unsigned int delay;
} adc_handle;

adc_handle *adc_open(const char *device);
void adc_close(adc_handle *h);
unsigned int read_adc_value(adc_handle *h, unsigned int channel);
unsigned int *read_adc_all_values(adc_handle *h, unsigned int channel);

// }}}
// rheo_control.c {{{

typedef enum control_scheme {constant, pid} control_scheme_enum;

double control_PID(double tuning[3], double input);

// }}}
// rheo_args.h {{{

typedef struct run_data {
  unsigned int length_s;
  control_scheme_enum control_scheme;
  const char *tag;
} run_data;

run_data *parse_args(int argc, const char **argv);
void free_run_data(run_data *r_d);

// }}}
// vim: foldmethod=marker ft=c
