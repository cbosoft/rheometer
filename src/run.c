#include <stdlib.h>

#include "run.h"
#include "util.h"
#include "control.h"
#include "adc.h"



struct run_data *init_run_data()
{
  struct run_data *rd = calloc(1, sizeof(struct run_data));

  rd->time_s = 0;
  rd->time_us = 0;
  rd->start_time_s = 0;
  rd->start_time_us = 0;
  rd->time_s_f = 0.0;
  rd->adc = calloc(ADC_COUNT, sizeof(double));
  rd->temperature = 0;
  rd->loadcell_bytes = 0;
  rd->loadcell_units = 0;

  rd->speed_ind_timeout = 0.1;
  rd->last_ca = 0;
  rd->err1 = 0.0;
  rd->err2 = 0.0;
  rd->calm_start = 0;
  rd->mode = MODE_NORMAL;

  rd->log_paths = NULL;
  rd->log_names = NULL;
  rd->log_count = 0;
  rd->uid = NULL;
  rd->phase = PHASE_INIT;
  rd->hardware_version = 0;

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

  rd->error_string = "all is well";
  rd->video_device = NULL;
  rd->photo_device = NULL;
  rd->cam_start = -1;
  rd->cam_end = -1;
  return rd;
}




void free_run_data(struct run_data *td)
{
  adc_close(td->adc_handle);
  for (unsigned int i = 0; i < td->log_count; i++)
    free(td->log_paths[i]);

  if (td->log_pref != NULL) free(td->log_pref);
  if (td->uid != NULL) free(td->uid);
 
  if (td->adc != NULL) free(td->adc);

  free(td);
}
