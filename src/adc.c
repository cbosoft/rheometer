#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <wiringPi.h>

#include "adc.h"
#include "run.h"
#include "util.h"




unsigned int read_adc_value(struct adc_handle *h, unsigned int channel)
{
#ifndef DEBUG
  int ret;
  uint8_t tx[] = {
    4 + (channel>>2), (channel&3)<<6, 0
  };
  unsigned int L = sizeof(tx)/sizeof(tx[0]);
  uint8_t rx[] = {0, 0, 0};

  struct spi_ioc_transfer tr = {
    .tx_buf = (unsigned long)tx,
    .rx_buf = (unsigned long)rx,
    .len = L,
    .delay_usecs = h->delay,
    .speed_hz = h->speed,
    .bits_per_word = h->bits,
  };

  ret = ioctl(h->fd, SPI_IOC_MESSAGE(1), &tr);
  if (ret < 1)
    ferr("read_adc_value", "can't send spi message");
  
  unsigned int total = 0;
  for (unsigned int i = 0; i < L; i++) {
    total += rx[i];
  }

  return total;
#else
  return 314;
#endif
}




struct adc_handle *adc_open(const char *device)
{
#ifndef DEBUG
  int fd = open(device, O_RDWR);
  if (fd < 0)
    ferr("adc_open", "could not open spi device");

  adc_handle_t *h = malloc(sizeof(adc_handle_t));

  h->device = device;
  h->fd = fd;
  
  // default values
  h->delay = 0;
  h->speed = 500000;
  h->bits = 8;

  return h;
#else
  struct adc_handle *h = malloc(sizeof(struct adc_handle));
  return h;
#endif
}



void
adc_close(struct adc_handle *h)
{
#ifndef DEBUG
  close(h->fd);
#endif
  free(h);
}




void *adc_thread_func(void *vptr) {

  struct run_data *rd = (struct run_data *)vptr;

  unsigned long *adc, *padc;

  rd->adc_ready = 1;
  while ( (!rd->stopped) && (!rd->errored) ) {
    
    adc = malloc(ADC_COUNT*sizeof(unsigned long));
    for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
      adc[channel] = read_adc_value(rd->adc_handle, channel);
    }
    
    padc = rd->adc;
    rd->adc = adc;

    free(padc);

    rh_usleep(10);

  }

  pthread_exit(0);

  return NULL;
}
