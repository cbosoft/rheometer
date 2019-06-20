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
rheo_read_adc_value(rheo_adc_ctrl *h, unsigned int channel)
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
    rheo_ferr("can't send spi message");
  
  unsigned int total = 0;
  for (unsigned int i = 0; i < L; i++) {
    total += rx[i];
  }

  return total;
}



rheo_adc_ctrl *
rheo_adc_open(const char *device)
{
  int fd = open(device, O_RDWR);
  if (fd < 0)
    rheo_ferr("could not open spi device");

  rheo_adc_ctrl *h = malloc(sizeof(rheo_adc_ctrl));

  h->device = device;
  h->fd = fd;
  
  // default values
  h->delay = 0;
  h->speed = 500000;
  h->bits = 8;

  return h;
}



void
rheo_adc_close(rheo_adc_ctrl *h)
{
  close(h->fd);
  free(h);
}
