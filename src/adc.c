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
#include <sys/time.h>

#include <wiringPi.h>

#include "adc.h"
#include "run.h"
#include "util.h"
#include "util.h"
#include "error.h"
#include "loadcell.h"


extern pthread_mutex_t lock_adc;

#define NBYTES 3
unsigned int read_adc_value(struct adc_handle *h, unsigned int channel)
{
#ifdef DEBUG
  (void) h;
  (void) channel;
  return 314;
#else
  uint8_t tx[NBYTES] = { 4 + (channel>>2), (channel&3)<<6, 0 };
  uint8_t rx[NBYTES] = {                0,              0, 0 };

  struct spi_ioc_transfer tr = {
    .tx_buf = (unsigned long)tx,
    .rx_buf = (unsigned long)rx,
    .len = NBYTES,
    .delay_usecs = h->delay,
    .speed_hz = h->speed,
    .bits_per_word = h->bits,
  };

  if (ioctl(h->fd, SPI_IOC_MESSAGE(1), &tr) < 1)
    ferr("read_adc_value", "can't send spi message");
  
  return ((rx[1] & 0b1111) << 8) | rx[2];
#endif
}




struct adc_handle *adc_open(const char *device)
{

#ifdef DEBUG
  int fd = open("/dev/null", O_RDWR);
#else
  int fd = open(device, O_RDWR);
#endif

  if (fd < 0)
    ferr("adc_open", "could not open spi device");

  struct adc_handle *h = malloc(sizeof(struct adc_handle));

  h->device = device;
  h->fd = fd;
  
  // default values
  h->delay = 0;
  h->speed = 500000;
  h->bits = 8;

  return h;
}



void adc_close(struct adc_handle *h)
{
  close(h->fd);
  free(h);
}




void *adc_thread_func(void *vptr) {

  struct run_data *rd = (struct run_data *)vptr;

  unsigned long *adc, *padc;

  struct timeval start, now;
  gettimeofday(&start, NULL);

  rd->adc_ready = 1;
  while ( (!rd->stopped) && (!rd->errored) ) {
    
    adc = malloc(ADC_COUNT*sizeof(unsigned long));
    for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
      adc[channel] = read_adc_value(rd->adc_handle, channel);
    }
    
    padc = rd->adc;

    pthread_mutex_lock(&lock_adc);
    rd->adc = adc;
    pthread_mutex_unlock(&lock_adc);

    free(padc);

    gettimeofday(&now, NULL);

    rd->adc_dt = time_elapsed(now, start);
    start = now;

  }

  pthread_exit(0);

  return NULL;
}
