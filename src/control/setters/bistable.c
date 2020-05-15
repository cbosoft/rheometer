#include "../../run/run.h"
#include "../../util/display.h"
#include "../control.h"

const char *doc =
      "  - "BOLD"Bistable"RESET", 'bistable'\n"
      "      Switch output between two values (\"upper\" and \"lower\") every \"period\" seconds.\n";

double get_setpoint(struct run_data *rd)
{
  double upper  = GET_SETTER_PARAM_OR_DEFAULT(rd, 0, 700);
  double lower  = GET_SETTER_PARAM_OR_DEFAULT(rd, 1, 300);
  double period = GET_SETTER_PARAM_OR_DEFAULT(rd, 2, 1.0);

  int periods_passed = (int)(rd->time_s_f / period);
  return periods_passed % 2 ? upper : lower;
}
