#include <string.h>
#include <wiringPi.h>

#include "rheo.h"


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




unsigned int
pid_control(thread_data_t *td)
{
  // TODO
  return 0.0;
}




unsigned int
constant_control(thread_data_t *td)
{
  return (unsigned int)(td->control_params->c * 10.24);
}




control_func_t
ctlfunc_from_int(int i)
{
  switch (i) {
    case control_constant:
      return &constant_control;
    case control_pid:
      return &pid_control;
    default:
      ferr("unrecognised control scheme index");
      break;
  }
  return NULL;
}




void *
ctl_thread_func(void *vtd) 
{
  thread_data_t *td = (thread_data_t *)vtd;
  
  control_func_t ctlfunc = ctlfunc_from_int(ctlidx_from_str(td->control_scheme));
  
  td->ctl_ready = 1;

  while ( (!td->stopped) && (!td->errored) ) {

    unsigned int control_action = ctlfunc(td);

    if (control_action > 1024)
      control_action = 1024;

    pwmWrite(PWM_PIN, control_action);

    td->last_ca = control_action;

    nsleep(td->control_params->sleep_ns);

  }

  return NULL;
}
