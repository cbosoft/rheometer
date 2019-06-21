#include <stdlib.h>

#include "rheo.h"

run_data *
parse_args(int argc, const char **argv) 
{
  run_data *rv = malloc(sizeof(run_data));
  rv->tag = "DELME";
  rv->length_s = 60;

  return rv;
}

void
free_run_data(run_data *r_d)
{
  free(r_d);
}
