#include <stdlib.h>

#include "run.h"
#include "util.h"
#include "control.h"
#include "adc.h"



struct run_data *init_run_data()
{
  struct run_data *rd = calloc(1, sizeof(struct run_data));

  rd->time_s = calloc(1, sizeof(unsigned long));
  rd->time_us = calloc(1, sizeof(unsigned long));
  rd->start_time_s = 0;
  rd->start_time_us = 0;
  rd->time_s_f = 0.0;
  rd->adc = calloc(ADC_COUNT, sizeof(double));
  rd->temperature = calloc(1, sizeof(double));
  rd->loadcell_bytes = calloc(1, sizeof(unsigned long));
  rd->loadcell_units = calloc(1, sizeof(double));

  rd->speed_ind_timeout = 0.1;
  rd->last_ca = 0;
  rd->err1 = 0.0;
  rd->err2 = 0.0;
  rd->calm_start = 0;

  rd->log_paths = NULL;
  rd->log_names = NULL;
  rd->log_count = 0;

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
  return rd;
}




void free_run_data(struct run_data *td)
{
  adc_close(td->adc_handle);
  for (unsigned int i = 0; i < td->log_count; i++)
    free(td->log_paths[i]);

  if (td->time_s != NULL) free(td->time_s);
  if (td->time_us != NULL) free(td->time_us);

  if (td->log_pref != NULL) free(td->log_pref);
 
  if (td->adc != NULL) free(td->adc);
  if (td->temperature != NULL) free(td->temperature);
  if (td->loadcell_bytes != NULL) free(td->loadcell_bytes);
  if (td->loadcell_units != NULL) free(td->loadcell_units);

  free(td);
}
