#include <stdlib.h>

#include "run.h"
#include "../sensors/adc/adc.h"

void free_run_data(struct run_data *rd)
{
  adc_close(rd->adc_handle);
  for (unsigned int i = 0; i < rd->log_count; i++)
    free(rd->log_paths[i]);

  if (rd->log_pref != NULL) free(rd->log_pref);
  if (rd->uid != NULL) free(rd->uid);
 
  if (rd->adc != NULL) free(rd->adc);

  free(rd->tag);

  if (rd->motor_name != NULL) free(rd->motor_name);
  if (rd->material_name != NULL) free(rd->material_name);

  free(rd);
}
