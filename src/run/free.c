#include <stdlib.h>

#include "run.h"
#include "../sensors/adc/adc.h"

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
