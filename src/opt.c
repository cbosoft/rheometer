#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "rheo.h"

static const uint8_t opt_pins[OPTENC_COUNT] = {16, 20, 21};

void *
opt_thread(void *vtd)
{
  thread_data *td = (thread_data *)vtd;

  char *gpio_paths[OPTENC_COUNT];
  FILE *logs[OPTENC_COUNT];
  for (uint8_t pi = 0; pi < OPTENC_COUNT; pi++) {
    td->log_paths[td->log_count] = calloc(265, sizeof(char));
    gpio_paths[pi] = calloc(265, sizeof(char));
    sprintf(td->log_paths[td->log_count], "%s_opt%d-combined.csv", td->log_pref, opt_pins[pi]);
    logs[pi] = fopen(td->log_paths[td->log_count], "w");
    td->log_count ++;
  }

  // TODO: init GPIOs

  td->opt_ready = 1;
  while ( (!td->errored) && (!td->stopped) ) {
    // TODO: for each gpio, check if is same as prev val
    nsleep(10000);
  }

  for (uint8_t pi = 0; pi < OPTENC_COUNT; pi++) {
    fclose(logs[pi]);
    free(gpio_paths[pi]);
  }

  return NULL;
}
