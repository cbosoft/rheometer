#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <wiringPi.h>

#include "rheo.h"
#include "cJSON.h"

#define PARAM_REQ 0
#define PARAM_OPT 1

#define STREQ(A,B) (strcmp(A, B) == 0)



typedef enum control_scheme {
  CONTROL_CONSTANT, 
  CONTROL_PID,
  CONTROL_SINE,
  CONTROL_BISTABLE
} control_scheme_enum;




void
control_help(void)
{
  fprintf(stderr, 
      "  "BOLD"Control Schemes"RESET"\n"
      "    Control schemes are defined in JSON files (stored in the data/ directory).\n"
      "    These JSON files dictate what function should be used, and what parameters \n"
      "    are passed. Different functions and their associated parameters are discussed\n"
      "    below. Scheme names, and all parameter names, are expected to be written in\n"
      "    lowercase in the JSON file.\n"
      "\n"
      "  "BOLD"Constant voltage"RESET", 'constant'\n"
      "    A constant is output, the only required parameter is \"c\", the constant (integer)\n"
      "    control action to output.\n"
      "\n"
      "  "BOLD"PID control"RESET", 'pid'\n"
      "    Uses the standard PID algoirithm to reject disturbance and maintain a set point:\n"
      "      caₙ = ca₍ₙ₋₁₎ + (Kp × err) + (Σⁿ Ki × errₙ) + (Kd × derr/dt)\n"
      "    The three tuning parameters (kp, ki, kd) are required, along with a single \n"
      "    setpoint parameter. It is usual to not use all three parts of the algorithm, a\n"
      "    section can be turned off by setting the relevant coefficient to zero.\n"
      "\n"
      "  "BOLD"Sine"RESET", 'sine'\n"
      "    Sine function operating on the time the experiment has run for. Evaluated as:\n"
      "      ca = ( sin( 2 × Pi × (t / period) ) × magnitude) + magnitude + mean\n"
      "    This will produce a sine wave which varies around \"mean\" by an amount \"magnitude\"\n"
      "    with period \"period\".\n"
      "\n"
      "  "BOLD"Bistable"RESET", 'bistable'\n"
      "    Switch output between two values (\"upper\" and \"lower\") every \"period\" seconds.\n"
      "\n"
  );
}



void
calculate_control_indicators(thread_data_t *td) 
{
  float dt_tot = 0.0;
  unsigned int count = 0;
  for (unsigned int i = 0; i < OPTENC_COUNT; i++) {
    for (unsigned int j = 1; j < SPD_HIST; j++, count++) {
      if (td->ptimes[i][j] == 0)
        break;
      // ptimes... low index is newest
      dt_tot += td->ptimes[i][j-1] - td->ptimes[i][j];
    }
  }

#ifndef DEBUG

  if (count == 0)
    warn("calculate_control_indicators", "speed requested but no optenc events have been recorded.");

  float dt_av = dt_tot / ((float)count);
#else
  float dt_av = (count == 0) ? 0.01 : dt_tot / ((float)count);
#endif

  float speed_hz = ((1.0/6.0) / dt_av); // rotations per second
  td->speed_ind = speed_hz;

  float strainrate_invs = speed_hz * PI * 2.0 * RI / (RO - RI);
  td->strainrate_ind = strainrate_invs;

  float torque_Nm = STRESS_M * td->adc[TSTS_CHANNEL] + STRESS_C;
  float stress_Pa = torque_Nm / (2.0 * PI * RI * RI * td->fill_depth);
  td->stress_ind = stress_Pa;

  td->viscosity_ind = stress_Pa / strainrate_invs;
}




unsigned int
pid_control(thread_data_t *td)
{
  // TODO

  float dca = 0.0;
  float input = (td->control_params->is_stress_controlled) ? td->stress_ind : td->strainrate_ind;
  float err = td->control_params->set_point - input;
  
  // Proportional control
  dca += td->control_params->kp * err;

  // Integral control
  for (unsigned int i = 0; i < ERR_HIST; i++) {
    if (td->errhist[i] == 0)
      break;
    dca += td->errhist[i] * td->control_params->ki;
  }
  
  // Add error to history
  float *errhist = calloc(ERR_HIST, sizeof(float)), *tmp;
  errhist[0] = err;
  for (unsigned int i = 0; i < ERR_HIST; i++) {
    errhist[i+1] = td->errhist[i];
  }
  tmp = td->errhist;
  td->errhist = errhist;
  free(tmp);

  // Derivative control
  // TODO

  return (unsigned int)(dca + td->last_ca);
}




unsigned int
constant_control(thread_data_t *td)
{
  return (unsigned int)(td->control_params->c);
}



unsigned int
sine_control(thread_data_t *td)
{
  double theta = 2.0 * PI * td->time_s_f / td->control_params->period;
  double rv = ( (sin(theta) + 1.0) * (double)td->control_params->magnitude ) + td->control_params->magnitude;
  return (unsigned int)rv;
}



unsigned int
bistable_control(thread_data_t *td)
{
  unsigned int periods_passed = (unsigned int)(td->time_s_f / td->control_params->period);
  return periods_passed % 2 ? td->control_params->upper : td->control_params->lower;
}



int
ctlidx_from_str(const char *s)
{
  if STREQ(s, "constant")
    return CONTROL_CONSTANT;
  else if STREQ(s, "pid")
    return CONTROL_PID;
  else if STREQ(s, "sine")
    return CONTROL_SINE;
  else if STREQ(s, "bistable")
    return CONTROL_BISTABLE;
  return -1;
}




control_func_t
ctlfunc_from_int(int i)
{
  switch (i) {
    case CONTROL_CONSTANT:
      return &constant_control;
    case CONTROL_PID:
      return &pid_control;
    case CONTROL_SINE:
      return &sine_control;
    case CONTROL_BISTABLE:
      return &bistable_control;
    default:
      ferr("ctlfunc_from_int", "unrecognised control scheme index");
      break;
  }
  return NULL;
}




void *
ctl_thread_func(void *vtd) 
{
  thread_data_t *td = (thread_data_t *)vtd;
  
  control_func_t ctlfunc = ctlfunc_from_int(ctlidx_from_str(td->control_scheme));
  
  td->ctl_ready = 1;

  while ( (!td->stopped) && (!td->errored) ) {

    calculate_control_indicators(td);

    unsigned int control_action = ctlfunc(td);

    if (control_action > CONTROL_MAXIMUM)
      control_action = CONTROL_MAXIMUM;

    if (control_action < CONTROL_MINIMUM)
      control_action = CONTROL_MINIMUM;

    pwmWrite(PWM_PIN, control_action);

    td->last_ca = control_action;

    nsleep(td->control_params->sleep_ns);

  }

  pthread_exit(0);

  return NULL;
}




void
get_control_scheme_parameter(cJSON *json, unsigned int type, const char *schemename, const char *paramname, const char *description, double *dbl_value, unsigned int *int_value, char **str_value)
{
  cJSON *param = cJSON_GetObjectItem(json, paramname);

  if (param == NULL) {
    if (type == PARAM_REQ) {
      cJSON_Delete(json);
      ferr("get_control_scheme_parameter", "%s requires a parameter \"%s\" (%s)", schemename, paramname, description);
    }
    else {
      return;
    }
  }

  if (cJSON_IsNumber(param)) {

    if (dbl_value == NULL) {

      if (int_value == NULL)
        ferr("get_control_scheme_parameter", "JSON parse error. \"%s\" is not expected to be a number (%s).", paramname, description);
      else
        (*int_value) = param->valueint;

    }
    else {
      (*dbl_value) = param->valuedouble;
    }

  } 
  else if (cJSON_IsString(param)) {

    if (str_value == NULL)
        ferr("get_control_scheme_parameter", "JSON parse error. \"%s\" is not expected to be a string (%s).", paramname, description);
    else
      (*str_value) = param->valuestring;

  }
  else if (cJSON_IsBool(param)) {

    if (int_value == NULL)
        ferr("get_control_scheme_parameter", "JSON parse error. \"%s\" is not expected to be a boolean (%s).", paramname, description);
    else
      (*int_value) = (cJSON_IsTrue(param)) ? 1 : 0;

  }
  else
    ferr("get_control_scheme_parameter", "Unknown type encountered while processing control scheme JSON param: %s", paramname);
}



void
read_control_scheme(thread_data_t *td, const char *control_scheme_json_path)
{
  cJSON *json = read_json(control_scheme_json_path);

  if (json == NULL) {
    const char *eptr = cJSON_GetErrorPtr();
    if (eptr != NULL)
      ferr("read_control_scheme", "JSON parse failed before %s", eptr);
    else
      ferr("read_control_scheme", "JSON parse failed.");
    cJSON_Delete(json);
  }

  cJSON *control_scheme_name_json = cJSON_GetObjectItem(json, "name");
  if (cJSON_IsString(control_scheme_name_json) && (control_scheme_name_json->valuestring != NULL)) {

    td->control_scheme = calloc(strlen(control_scheme_name_json->valuestring)+1, sizeof(char));
    strcpy(td->control_scheme, control_scheme_name_json->valuestring);

    td->control_scheme = calloc(strlen(control_scheme_json_path)+1, sizeof(char));
    strcpy(td->control_scheme, control_scheme_json_path);

  }
  else {
    cJSON_Delete(json);
    ferr("read_control_scheme", "control scheme json must name a scheme.\n  e.g. { ... \"name\": \"constant\" ... }");
  }
  
  int schemeidx = ctlidx_from_str(td->control_scheme);
  if (schemeidx < 0) {
    cJSON_Delete(json);
    ferr("read_control_scheme", "control scheme JSON must name a known scheme. See rheometer -h for a list.");
  }

  control_params_t *params = malloc(sizeof(control_params_t));
  params->c = 0.0;
  params->kp = 0.0;
  params->ki = 0.0;
  params->kd = 0.0;
  params->set_point = 0.0;
  params->sleep_ns = 100*1000*1000;
  params->is_stress_controlled = 0;
  
  switch (schemeidx) {
    case CONTROL_CONSTANT:
      get_control_scheme_parameter(json, PARAM_REQ, "constant", "c", "output value, double", NULL, &params->c, NULL);
      break;
    case CONTROL_PID:
      get_control_scheme_parameter(json, PARAM_REQ, "pid", "kp", "proportional control coefficient, double", &params->kp, NULL, NULL);
      get_control_scheme_parameter(json, PARAM_REQ, "pid", "ki", "integral control coefficient, double", &params->ki, NULL, NULL);
      get_control_scheme_parameter(json, PARAM_REQ, "pid", "kd", "derivative control coefficient, double", &params->kd, NULL, NULL);
      get_control_scheme_parameter(json, PARAM_REQ, "pid", "setpoint", "set point/target, double", &params->set_point, NULL, NULL);
      get_control_scheme_parameter(json, PARAM_OPT, "pid", "stress_controlled", "boolean: true if stress is controlled parameter, or false for strainrate control", NULL, &params->is_stress_controlled, NULL);
      break;
    case CONTROL_SINE:
      get_control_scheme_parameter(json, PARAM_REQ, "sine", "magnitude", "amplitude of the sine wave (mean to peak), double", &params->magnitude, NULL, NULL);
      get_control_scheme_parameter(json, PARAM_REQ, "sine", "mean", "mean value of the sine wave, double", &params->mean, NULL, NULL);
      get_control_scheme_parameter(json, PARAM_REQ, "sine", "period", "period of the sine wave in seconds, double", &params->period, NULL, NULL);
      break;
    case CONTROL_BISTABLE:
      get_control_scheme_parameter(json, PARAM_REQ, "bistable", "lower", "lower stable value, integer 0-1024", NULL, &params->lower, NULL);
      get_control_scheme_parameter(json, PARAM_REQ, "bistable", "upper", "upper stable value, integer 0-1024", NULL, &params->upper, NULL);
      get_control_scheme_parameter(json, PARAM_REQ, "bistable", "period", "switching period in seconds, double", &params->period, NULL, NULL);
      break;
  }

  // optional params
  get_control_scheme_parameter(json, PARAM_OPT, "*", "sleep_ns", "interval between control calculations, nanoseconds", NULL, &params->sleep_ns, NULL);

  cJSON_Delete(json);

  td->control_params = params;

}
