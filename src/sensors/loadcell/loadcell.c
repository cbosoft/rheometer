// This file written with help from https://github.com/ggurov/hx711
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "../../run/run.h"
#include "../../util/sleep.h"

#include "hx711.h"
#include "loadcell.h"



extern pthread_mutex_t lock_loadcell;

double get_torque_from_loadcell(struct run_data *rd, unsigned long bytes)
{
  (void) rd;
  (void) bytes;
  return 0.1;
}



void read_loadcell(struct run_data *rd)
{
  unsigned long lc_bytes = hx711_read();

  pthread_mutex_lock(&lock_loadcell);
  rd->loadcell_bytes = lc_bytes;
  rd->loadcell_units = get_torque_from_loadcell(rd, lc_bytes);
  pthread_mutex_unlock(&lock_loadcell);

}
