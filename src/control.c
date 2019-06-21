#include "rheo.h"

void *
ctlfunc_from_int(int i)
{
  // TODO
  return NULL;
}

unsigned int
pid_control(double input, control_params *params)
{
  // TODO
  return 0.0;
}

unsigned int
constant_control(double input, control_params *params)
{
  return params->c;
}
