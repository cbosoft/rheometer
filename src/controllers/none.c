#include "../run.h"
#include "../display.h"
#include "../control.h"

const char *doc =
      "  - "BOLD"No control"RESET", 'none'\n"
      "      None actual controlling of the variable is performed; just a simple conversion from \n"
      "      the stress/strainrate setpoint to DC. A single required parameter is \"mult\", the \n"
      "      multiplier which does the conversion. This method is not intended to provide robust \n"
      "      control, but is intended to take the controller out of the equation.\n";

unsigned int get_control_action(struct run_data *rd)
{
  /*
    "no_control" - multiplies the setpoint by a number to get a DC, no adaptive
    control or anything.
   */

  double m = get_control_param_or_default(rd, 0, 1.0);
  return (unsigned int)(m * rd->control_params->setpoint);
}
