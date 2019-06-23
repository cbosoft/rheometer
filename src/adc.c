#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <wiringPi.h>

#include "rheo.h"
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

unsigned int
read_adc_value(adc_handle *h, unsigned int channel)
{
#ifndef DEBUG
  int ret;
  uint8_t tx[] = {
    4 + (channel>>2), (channel&3)<<6, 0
  };
  unsigned int L = ARRAY_SIZE(tx);
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
    ferr("can't send spi message");
  
  unsigned int total = 0;
  for (unsigned int i = 0; i < L; i++) {
    total += rx[i];
  }

  return total;
#else
  return 314;
#endif
}



adc_handle *
adc_open(const char *device)
{
#ifndef DEBUG
  int fd = open(device, O_RDWR);
  if (fd < 0)
    ferr("could not open spi device");

  adc_handle *h = malloc(sizeof(adc_handle));

  h->device = device;
  h->fd = fd;
  
  // default values
  h->delay = 0;
  h->speed = 500000;
  h->bits = 8;

  return h;
#else
  adc_handle *h = malloc(sizeof(adc_handle));
  return h;
#endif
}



void
adc_close(adc_handle *h)
{
#ifndef DEBUG
  close(h->fd);
#endif
  free(h);
}




void *
adc_thread_func(void *vtd) {
  thread_data *td = (thread_data *)vtd;
  adc_handle *adc_h = td->adc_h;
  
  unsigned long *adc, *padc;

  td->adc_ready = 1;
  while ( (!td->stopped) && (!td->errored) ) {
    
    adc = malloc(ADC_COUNT*sizeof(unsigned long));
    for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
      adc[channel] = read_adc_value(adc_h, channel);
    }
    
    padc = td->adc;
    td->adc = adc;

    free(padc);

    nsleep(10000);

  }


  return NULL;
}
