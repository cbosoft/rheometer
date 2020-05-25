#include "../../run/run.h"
#include "../../util/display.h"
#include "../control.h"

const char *name = "Bistable Setter";
const char *ident = "bistable";
const char *doc = "Switch output between two values (\"upper\" and \"lower\") every \"period\" seconds.";
int n_params = 3;
const char *params[] = {
    "upper", "700",
    "lower", "300",
    "period", "1"
};

double get_setpoint(struct run_data *rd)
{
  double upper  = GET_SETTER_PARAM_OR_DEFAULT(rd, 0, 700);
  double lower  = GET_SETTER_PARAM_OR_DEFAULT(rd, 1, 300);
  double period = GET_SETTER_PARAM_OR_DEFAULT(rd, 2, 1.0);

  int periods_passed = (int)(rd->time_s_f / period);
  return periods_passed % 2 ? upper : lower;
}
