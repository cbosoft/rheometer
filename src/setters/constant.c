#include "../run.h"
#include "../control.h"
#include "../display.h"

const char *doc =
      "  - "BOLD"Constant value setter"RESET", 'constant'\n"
      "      A constant setpoint, the only required parameter is \"setpoint\", the constant (double)\n"
      "      value of stress/strainrate to provide the controller.\n";

double get_setpoint(struct run_data *rd)
{
  return get_setter_param_or_default(rd, 0, 0.0);
}
