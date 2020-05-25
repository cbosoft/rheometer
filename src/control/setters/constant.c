#include "../../run/run.h"
#include "../control.h"

const char *name = "Constant Setter";
const char *ident = "constant";
const char *doc = "A constant setpoint, the only parameter is \"setpoint\", the constant (double) value of stress/strainrate to provide the controller.";
int n_params = 1;
const char *params[] = {
    "value", "0.0"
};

double get_setpoint(struct run_data *rd)
{
  return GET_SETTER_PARAM_OR_DEFAULT(rd, 0, 0.0);
}
