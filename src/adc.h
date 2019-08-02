#pragma once

#include "run.h"



#define ADC_COUNT 8
#define PND_CHANNEL 1
#define TSTS_CHANNEL 2




struct adc_handle {
  const char *device;
  int fd;
  unsigned int mode;
  unsigned int bits;
  unsigned int speed;
  unsigned int delay;
};




struct adc_handle *adc_open(const char *device);
void adc_close(struct adc_handle *h);
unsigned int read_adc_value(struct adc_handle *h, unsigned int channel);
void *adc_thread_func(void *vptr);




// vim: ft=c
