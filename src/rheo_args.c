#include <stdlib.h>

#include "rheo.h"

run_data *
parse_args(int argc, const char **argv) 
{
  run_data *rv = malloc(sizeof(run_data));

  return rv;
}

void
free_run_data(run_data *r_d)
{
  free(r_d);
}
