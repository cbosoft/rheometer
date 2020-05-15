#include "../../run/run.h"
#include "../../util/display.h"
#include "../control.h"

const char *doc =
      "  - "BOLD"Bistable"RESET", 'bistable'\n"
      "      Switch output between two values (\"upper\" and \"lower\") every \"period\" seconds.\n";

double get_setpoint(struct run_data *rd)
{
  double upper = get_setter_param_or_default(rd, 0, 700);
  double lower = get_setter_param_or_default(rd, 1, 300);
  double period = get_setter_param_or_default(rd, 2, 1.0);

  int periods_passed = (int)(rd->time_s_f / period);
  return periods_passed % 2 ? upper : lower;
}
