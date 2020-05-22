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


double loadcell_cal(struct run_data *rd, unsigned long bytes)
{
  double speed = rd_get_speed(rd);

  double M_fric = speed * rd->loadcell_calibration.k_omega_to_m;
  double lcmlcz = ((double)bytes) - rd->loadcell_calibration.lc_z;
  double M_total = lcmlcz * rd->loadcell_calibration.k_lc_to_m;
  double M_load = M_total - M_fric;

  return M_load;
}


void read_loadcell(struct run_data *rd)
{

  rd_set_loadcell_bytes(rd, hx711_read());

}
