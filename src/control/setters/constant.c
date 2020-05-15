#include "../../run/run.h"
#include "../../util/display.h"
#include "../control.h"

const char *doc =
      "  - "BOLD"Constant value setter"RESET", 'constant'\n"
      "      A constant setpoint, the only required parameter is \"setpoint\", the constant (double)\n"
      "      value of stress/strainrate to provide the controller.\n";

double get_setpoint(struct run_data *rd)
{
  return GET_SETTER_PARAM_OR_DEFAULT(rd, 0, 0.0);
}
