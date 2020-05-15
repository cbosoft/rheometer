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

const double K_LC_to_M = 1.0;
const double K_W_to_M = 1.0;
const double LCZ = 1.0;


double loadcell_cal(struct run_data *rd, unsigned long bytes)
{
  double speed = get_speed(rd);

  double M_fric = speed * K_W_to_M;
  double lcmlcz = ((double)bytes) - LCZ;
  double M_total = lcmlcz * K_LC_to_M;
  double M_load = M_total - M_fric;

  return M_load;
}

void read_loadcell(struct run_data *rd)
{

  set_loadcell_bytes(rd, hx711_read());

}
