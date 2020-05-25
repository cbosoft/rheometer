#include "../../run/run.h"
#include "../control.h"

const char *name = "No control";
const char *ident = "none";
const char *doc =
"No actual controlling of the variable is performed; just a simple conversion "
"from stress/strainrate setpoint to motor PWM DC. A single optional parameter is "
"the multiplier which does the conversion. This method is not intended to "
"provide robust control, but is intended to take the controller out of the "
"equation during the investigation of complex systems. ";
int n_params = 1;
const char *params[] = {
  "mult", "1.0"
};

unsigned int get_control_action(struct run_data *rd)
{
  /*
    "no_control" - multiplies the setpoint by a number to get a DC, no adaptive
    control or anything.
   */

  double m = GET_CONTROL_PARAM_OR_DEFAULT(rd, 0, 1.0);
  return (unsigned int)(m * rd_get_setpoint(rd));
}
