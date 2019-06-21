#include "rheo.h"

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
