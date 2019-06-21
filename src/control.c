#include <string.h>

#include "rheo.h"

typedef unsigned int (*control_func)(thread_data *, control_params *);

static const char *control_schemes[] = {
  "constant",
  "pid",
  ""
};

int
ctlidx_from_str(const char *s)
{
  for (unsigned int i = 0; i < sizeof(control_schemes)/sizeof(char *); i++) {
    if (strcmp(s, control_schemes[i]) == 0)
      return i;
  }

  return -1;
}

control_func
ctlfunc_from_int(int i)
{
  switch (i) {
    case control_constant:
      break;
    case control_pid:
      break;
    default:
      ferr("somehow executing impossible code, ya done goofed.");
      break;
  }
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
