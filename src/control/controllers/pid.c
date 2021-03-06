#include "../../run/run.h"
#include "../control.h"

const char *name = "PID Control";

const char *ident = "pid";

const char *doc =
"Uses the velocity PID algoirithm to reject disturbance and maintain a set point:\n"
"\n"
"Δcaₙ = (KP×(errₙ - errₙ₋₁)\n"
"     + (KI×errₙ×Δt)\n"
"     + (KD×(errₙ - (2×errₙ₋₁) + errₙ₋₂)/Δt)\n"
"\n"
"Three tuning parameters (kp, ki, kd) are required, delta time is measured setpoint "
"parameter. It is usual to not use all three parts of the algorithm, a section can be "
"turned off by setting the relevant coefficient to zero.";

int n_params = 3;

const char *params[] = {
  "kp", "0.0",
  "ki", "0.0",
  "kd", "0.0"
};

unsigned int get_control_action(struct run_data *rd)
{
  /*
    PID - proportional -- integral -- derivative control

    Usual velocity algorthm:

    dCA = KP*dErr + KI*Err*dt + KD * (Err - 2Err1 + Err2)/dt

   */

  double dca = 0.0;
  double input = (rd_get_is_stress_controlled(rd)) ? rd->stress_ind : rd->strainrate_ind;
  double err = rd_get_setpoint(rd) - input;
  double delta_t = rd_get_control_interval(rd) * 0.001;

  double kp = GET_CONTROL_PARAM_OR_DEFAULT(rd, 0, 0.0);
  double ki = GET_CONTROL_PARAM_OR_DEFAULT(rd, 1, 0.0);
  double kd = GET_CONTROL_PARAM_OR_DEFAULT(rd, 2, 0.0);
  static double errhist[2] = {0.0, 0.0};

  // Proportional control
  dca += kp * (err - errhist[0]);
  
  // Integral control
  dca += ki * err * delta_t;

  // Derivative control
  dca += kd * (err - (2.0*errhist[0]) + errhist[1]) / delta_t;

  // Update error history
  errhist[1] = errhist[0];
  errhist[0] = err;

  return (unsigned int)(dca + rd->last_ca);
}
