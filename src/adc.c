#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "rheo.h"
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

unsigned int
read_adc_value(adc_handle *h, unsigned int channel)
{

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
}



adc_handle *
adc_open(const char *device)
{
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
}



void
adc_close(adc_handle *h)
{
  close(h->fd);
  free(h);
}




void *
adc_thread_func(void *vt_d) {
  thread_data *t_d = (thread_data *)vt_d;
  adc_handle *adc_h = t_d->adc_h;
  
  unsigned long *adc, *padc;

  t_d->adc_ready = 1;
  while ( (!t_d->stopped) && (!t_d->errored) ) {
    
    adc = malloc(ADC_COUNT*sizeof(unsigned long));
    for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
      adc[channel] = read_adc_value(adc_h, channel);
    }
    
    padc = t_d->adc;
    t_d->adc = adc;

    free(padc);

    nsleep(10000);

  }


  return NULL;
}
