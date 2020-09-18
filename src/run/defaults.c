#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>

#include "../util/sleep.h"
#include "../util/error.h"
#include "../control/control.h"
#include "../sensors/adc/adc.h"
#include "../defaults.h"

#include "run.h"


#define INIT_MUTEX(MUTEX) \
    if (pthread_mutex_init(&MUTEX, NULL) != 0) {\
      ferr("main", "error setting up mutex");\
    }

struct run_data *init_run_data()
{
  struct run_data *rd = calloc(1, sizeof(struct run_data));

  rd->time_s = 0;
  rd->time_us = 0;
  rd->start_time_s = 0;
  rd->start_time_us = 0;
  rd->time_s_f = 0.0;
  rd->adc = calloc(ADC_COUNT, sizeof(double));
  rd->cylinder_temperature = 0;
  rd->ambient_temperature = 0;
  rd->loadcell_bytes = 0;
  rd->loadcell_units = 0;
  rd->loadcell_calibration.name = "default";


  rd->control_scheme.sleep_ms = 100;
  rd->control_scheme.is_stress_controlled = 0;
  rd->control_scheme.setpoint = 0.0;
  rd->control_scheme.control_params = NULL;
  rd->control_scheme.n_control_params = 0;
  rd->control_scheme.setter_params = NULL;
  rd->control_scheme.n_setter_params = 0;

  rd->speed_ind_timeout = 0.5;
  rd->last_ca = 0;
  rd->calm_start = 0;
  rd->mode = MODE_NORMAL;

  rd->tag = strdup(TAGDEFAULT);
  rd->log_paths = NULL;
  rd->log_names = NULL;
  rd->log_count = 0;
  rd->uid = NULL;
  rd->phase = PHASE_INIT;
  rd->hardware_version = 0;
  rd->fill_depth = -1;
  rd->needle_depth = -1;

  rd->stopped = 0;
  rd->errored = 0;
  rd->adc_ready = 0;
  rd->opt_ready = 0;
  rd->log_ready = 0;
  rd->tmp_ready = 0;
  rd->lc_ready = 0;
  rd->cam_ready = 0;

  rd->adc_busy = 0;
  rd->adc_dt = 0.0;

  rd->log_photo = 0;
  rd->log_video = 0;
  rd->cam_start = -1;
  rd->cam_end = -1;
  rd->motor_name_set = 0;
  rd->motor_name = NULL;
  rd->material_name = NULL;

  INIT_MUTEX(rd->lock_time);
  INIT_MUTEX(rd->lock_adc);
  INIT_MUTEX(rd->lock_adcdt);
  INIT_MUTEX(rd->lock_control);
  INIT_MUTEX(rd->lock_loadcell);
  INIT_MUTEX(rd->lock_temperature);
  INIT_MUTEX(rd->lock_speed);

  return rd;
}
