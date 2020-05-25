#include <math.h>

#include "../../run/run.h"
#include "../control.h"

const char *name = "Sine Setter";
const char *ident = "sine";
const char *doc =
"Sine function operating on the time the experiment has run for. Evaluated as:\n"
"\n"
"ca = ( sin( 2 × Pi × (t / period) ) × magnitude) + mean\n"
"\n"
"This will produce a sine wave which varies around \"mean\" by an amount \"magnitude\" "
"with period \"period\".";
int n_params = 3;
const char *params[] = {
    "mean", "100",
    "magnitude", "1",
    "period", "1"
};

double get_setpoint(struct run_data *rd)
{
  // TODO get sensible defaults
  double mean      = GET_SETTER_PARAM_OR_DEFAULT(rd, 0, 100);
  double magnitude = GET_SETTER_PARAM_OR_DEFAULT(rd, 1, 1.0);
  double period    = GET_SETTER_PARAM_OR_DEFAULT(rd, 2, 1.0);

  double theta = 2.0 * 3.1415926 * rd->time_s_f / period;
  double rv = (sin(theta) * magnitude) + mean;
  return (unsigned int)rv;
}
