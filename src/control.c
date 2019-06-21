#include "rheo.h"

double 
pid_control(double input, control_params *params)
{
  // TODO
  return 0.0;
}

double
constant_control(double input, control_params *params)
{
  return params->c;
}
