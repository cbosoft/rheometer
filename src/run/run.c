#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "../util/sleep.h"
#include "../util/error.h"
#include "../control/control.h"
#include "../sensors/adc/adc.h"
#include "../sensors/loadcell/loadcell.h"
#include "../geometry.h"
#include "run.h"



// Speed {{{

void rd_set_speed(struct run_data *rd, double speed)
{
  double strainrate_invs = speed * PI * 2.0 * RI / (RO - RI);
  pthread_mutex_lock(&rd->lock_speed);
  rd->speed_ind = speed;
  rd->strainrate_ind = strainrate_invs;
  pthread_mutex_unlock(&rd->lock_speed);
}


double rd_get_speed(struct run_data *rd)
{
  double rv;
  pthread_mutex_lock(&rd->lock_speed);
  rv = rd->speed_ind;
  pthread_mutex_unlock(&rd->lock_speed);
  return rv;
}


double rd_get_strainrate(struct run_data *rd)
{
  double rv;
  pthread_mutex_lock(&rd->lock_speed);
  rv = rd->strainrate_ind;
  pthread_mutex_unlock(&rd->lock_speed);
  return rv;
}

// }}}


// Temperature {{{

void rd_set_ambient_temperature(struct run_data *rd, double value)
{
  pthread_mutex_lock(&rd->lock_temperature);
  rd->ambient_temperature = value;
  pthread_mutex_unlock(&rd->lock_temperature);
}

double rd_get_ambient_temperature(struct run_data *rd)
{
  double rv;
  pthread_mutex_lock(&rd->lock_temperature);
  rv = rd->ambient_temperature;
  pthread_mutex_unlock(&rd->lock_temperature);
  return rv;
}

void rd_set_cylinder_temperature(struct run_data *rd, double value)
{
  pthread_mutex_lock(&rd->lock_temperature);
  rd->cylinder_temperature = value;
  pthread_mutex_unlock(&rd->lock_temperature);
}

double rd_get_cylinder_temperature(struct run_data *rd)
{
  double rv;
  pthread_mutex_lock(&rd->lock_temperature);
  rv = rd->cylinder_temperature;
  pthread_mutex_unlock(&rd->lock_temperature);
  return rv;
}

// }}}


// Loadcell {{{

void rd_set_loadcell_bytes(struct run_data *rd, unsigned long bytes)
{
  double loadcell_units = loadcell_cal(rd, bytes);
  double stress_Pa = rd->loadcell_units / (2.0 * PI * RI * RI * rd->fill_depth);

  pthread_mutex_lock(&rd->lock_loadcell);
  rd->loadcell_bytes = bytes;
  rd->loadcell_units = loadcell_units;
  rd->stress_ind = stress_Pa;
  pthread_mutex_unlock(&rd->lock_loadcell);
}

unsigned long rd_get_loadcell_bytes(struct run_data *rd)
{
  unsigned long rv;
  pthread_mutex_lock(&rd->lock_loadcell);
  rv = rd->loadcell_bytes;
  pthread_mutex_unlock(&rd->lock_loadcell);
  return rv;
}

double rd_get_loadcell_units(struct run_data *rd)
{
  double rv;
  pthread_mutex_lock(&rd->lock_loadcell);
  rv = rd->loadcell_units;
  pthread_mutex_unlock(&rd->lock_loadcell);
  return rv;
}

double rd_get_stress(struct run_data *rd)
{
  double rv;
  pthread_mutex_lock(&rd->lock_loadcell);
  rv = rd->stress_ind;
  pthread_mutex_unlock(&rd->lock_loadcell);
  return rv;
}

// }}}


// Control {{{

void rd_set_last_control_action(struct run_data *rd, unsigned int value)
{
  pthread_mutex_lock(&rd->lock_control);
  rd->last_ca = value;
  pthread_mutex_unlock(&rd->lock_control);
}

unsigned int rd_get_last_control_action(struct run_data *rd)
{
  unsigned int rv;
  pthread_mutex_lock(&rd->lock_control);
  rv = rd->last_ca;
  pthread_mutex_unlock(&rd->lock_control);
  return rv;
}

void rd_set_stress_controlled(struct run_data *rd)
{
  pthread_mutex_lock(&rd->lock_control);
  rd->control_scheme.is_stress_controlled = 1;
  pthread_mutex_unlock(&rd->lock_control);
}

void rd_set_strainrate_controlled(struct run_data *rd)
{
  pthread_mutex_lock(&rd->lock_control);
  rd->control_scheme.is_stress_controlled = 0;
  pthread_mutex_unlock(&rd->lock_control);
}

int rd_get_is_stress_controlled(struct run_data *rd)
{
  int rv;
  pthread_mutex_lock(&rd->lock_control);
  rv = rd->control_scheme.is_stress_controlled;
  pthread_mutex_unlock(&rd->lock_control);
  return rv;
}

double rd_get_setpoint(struct run_data *rd)
{
  double rv;
  pthread_mutex_lock(&rd->lock_control);
  rv = rd->control_scheme.setpoint;
  pthread_mutex_unlock(&rd->lock_control);
  return rv;
}

unsigned int rd_get_control_interval(struct run_data *rd)
{
  unsigned int rv;
  pthread_mutex_lock(&rd->lock_control);
  rv = rd->control_scheme.sleep_ms;
  pthread_mutex_unlock(&rd->lock_control);
  return rv;
}

// }}}

// Time {{{

void rd_set_start_time(struct run_data *rd)
{
  struct timeval tv;
  unsigned long sec, usec;

  gettimeofday(&tv, 0);
  sec = tv.tv_sec;
  usec = tv.tv_usec;

  pthread_mutex_lock(&rd->lock_time);
  rd->start_time_s = sec;
  rd->start_time_us = usec;
  pthread_mutex_unlock(&rd->lock_time);
}

void rd_set_time(struct run_data *rd)
{
  struct timeval tv;
  unsigned long sec, usec;

  gettimeofday(&tv, 0);
  sec = tv.tv_sec;
  usec = tv.tv_usec;

  pthread_mutex_lock(&rd->lock_time);
  rd->time_s = sec;
  rd->time_us = usec;
  pthread_mutex_unlock(&rd->lock_time);

  unsigned long dt_sec = sec - rd->start_time_s, dt_usec;
  if (usec < rd->start_time_us) {
    dt_sec -= 1;
    dt_usec = rd->start_time_us - usec;
  }
  else {
    dt_usec = usec - rd->start_time_us;
  }

  pthread_mutex_lock(&rd->lock_time);
  rd->time_s_f = (double)dt_sec + (0.001 * 0.001 * ((double)dt_usec));
  pthread_mutex_unlock(&rd->lock_time);
}

double rd_get_time(struct run_data *rd)
{
  double rv;
  pthread_mutex_lock(&rd->lock_time);
  rv = rd->time_s_f;
  pthread_mutex_unlock(&rd->lock_time);
  return rv;
}

void rd_get_time_parts(struct run_data *rd, unsigned long *time_s, unsigned long *time_us)
{
  pthread_mutex_lock(&rd->lock_time);
  (*time_s) = rd->time_s;
  (*time_us) = rd->time_us;
  pthread_mutex_unlock(&rd->lock_time);
}

// }}}
// ADC {{{

unsigned long *rd_swap_adc(struct run_data *rd, unsigned long *value)
{
  unsigned long *rv = NULL;
  pthread_mutex_lock(&rd->lock_adc);
  rv = rd->adc;
  rd->adc = value;
  pthread_mutex_unlock(&rd->lock_adc);
  return rv;
}

unsigned long rd_get_adc(struct run_data *rd, int i)
{
  unsigned long rv;
  pthread_mutex_lock(&rd->lock_adc);
  rv = rd->adc[i];
  pthread_mutex_unlock(&rd->lock_adc);
  return rv;
}

// }}}
