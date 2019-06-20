#pragma once

#define VERSION "0.1"

// rheo_error.c
void rheo_ferr(const char *mesg);
void rheo_warn(const char *mesg);

// rheo_adc.c {{{

typedef struct rheo_adc_ctrl {
  const char *device;
  int fd;
  unsigned int mode;
  unsigned int bits;
  unsigned int speed;
  unsigned int delay;
} rheo_adc_ctrl;


rheo_adc_ctrl *rheo_adc_open(const char *device);
void rheo_adc_close(rheo_adc_ctrl *h);
unsigned int rheo_read_adc_value(rheo_adc_ctrl *h, unsigned int channel);

// }}}


// vim: foldmethod=marker ft=c
