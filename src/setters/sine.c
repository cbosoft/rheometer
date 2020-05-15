#include <math.h>

#include "../run.h"
#include "../control.h"
#include "../display.h"

const char *doc =
      "  - "BOLD"Sinusoid setter"RESET", 'sine'\n"
      "      Sine function operating on the time the experiment has run for. Evaluated as:\n"
      "        ca = ( sin( 2 × Pi × (t / period) ) × magnitude) + magnitude + mean\n"
      "      This will produce a sine wave which varies around \"mean\" by an amount \"magnitude\"\n"
      "      with period \"period\".\n";

double get_setpoint(struct run_data *rd)
{
  // TODO get sensible defaults
  double mean = get_setter_param_or_default(rd, 0, 100);
  double magnitude = get_setter_param_or_default(rd, 1, 1.0);
  double period = get_setter_param_or_default(rd, 2, 1.0);

  double theta = 2.0 * 3.1415926 * rd->time_s_f / period;
  double rv = (sin(theta) * magnitude) + mean;
  return (unsigned int)rv;
}
