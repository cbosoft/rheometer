#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <wiringPi.h>

#include "control.h"
#include "display.h"
#include "error.h"
#include "opt.h"
#include "motor.h"
#include "util.h"
#include "json.h"
#include "geometry.h"
#include "loadcell.h"




#define PARAM_REQ 0
#define PARAM_OPT 1
#define PI 3.1415926

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
calculate_control_indicators(struct run_data *rd) 
{

  float dt_av = get_speed();

  float speed_hz = ((1.0/6.0) / dt_av); // rotations per second
  rd->speed_ind = speed_hz;

  float strainrate_invs = speed_hz * PI * 2.0 * RI / (RO - RI);
  rd->strainrate_ind = strainrate_invs;

  float stress_Pa = rd->loadcell_units / (2.0 * PI * RI * RI * rd->fill_depth);
  rd->stress_ind = stress_Pa;

  rd->viscosity_ind = stress_Pa / strainrate_invs;
}




unsigned int
pid_control(struct run_data *rd)
{

  float dca = 0.0;
  float input = (rd->control_params->is_stress_controlled) ? rd->stress_ind : rd->strainrate_ind;
  float err = rd->control_params->set_point - input;
  
  // Proportional control
  dca += rd->control_params->kp * err;

  // Integral control
  for (unsigned int i = 0; i < ERR_HIST; i++) {
    if (rd->errhist[i] == 0)
      break;
    dca += rd->errhist[i] * rd->control_params->ki;
  }
  
  // Add error to history
  float *errhist = calloc(ERR_HIST, sizeof(float)), *tmp;
  errhist[0] = err;
  for (unsigned int i = 0; i < ERR_HIST; i++) {
    errhist[i+1] = rd->errhist[i];
  }
  tmp = rd->errhist;
  rd->errhist = errhist;
  free(tmp);

  // Derivative control
  // TODO

  return (unsigned int)(dca + rd->last_ca);
}




unsigned int
constant_control(struct run_data *rd)
{
  return (unsigned int)(rd->control_params->c);
}



unsigned int
sine_control(struct run_data *rd)
{
  double theta = 2.0 * PI * rd->time_s_f / rd->control_params->period;
  double rv = ( (sin(theta) + 1.0) * (double)rd->control_params->magnitude ) + rd->control_params->magnitude;
  return (unsigned int)rv;
}



unsigned int
bistable_control(struct run_data *rd)
{
  unsigned int periods_passed = (unsigned int)(rd->time_s_f / rd->control_params->period);
  return periods_passed % 2 ? rd->control_params->upper : rd->control_params->lower;
}



int
ctlidx_from_str(const char *s)
{
  fprintf(stderr, "CONTROL SCHEME: \"%s\"\n", s);
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
  }
  ferr("ctlfunc_from_int", "unrecognised control scheme index");
  return NULL;
}




void *ctl_thread_func(void *vptr)
{
  struct run_data *rd = (struct run_data *)vptr;
  
  control_func_t ctlfunc = ctlfunc_from_int(ctlidx_from_str(rd->control_scheme));
  
  rd->ctl_ready = 1;

  while ( (!rd->stopped) && (!rd->errored) ) {

    calculate_control_indicators(rd);

    unsigned int control_action = ctlfunc(rd);

    if (control_action > CONTROL_MAXIMUM)
      control_action = CONTROL_MAXIMUM;

    if (control_action < CONTROL_MINIMUM)
      control_action = CONTROL_MINIMUM;

    pwmWrite(PWM_PIN, control_action);

    rd->last_ca = control_action;

    //rh_nsleep(rd->control_params->sleep_ns);
    sleep(1);

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
read_control_scheme(struct run_data *rd, const char *control_scheme_json_path)
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

    rd->control_scheme = calloc(strlen(control_scheme_name_json->valuestring)+1, sizeof(char));
    strcpy(rd->control_scheme, control_scheme_name_json->valuestring);

    rd->control_scheme_path = calloc(strlen(control_scheme_json_path)+1, sizeof(char));
    strcpy(rd->control_scheme_path, control_scheme_json_path);

  }
  else {
    cJSON_Delete(json);
    ferr("read_control_scheme", "control scheme json must name a scheme.\n  e.g. { ... \"name\": \"constant\" ... }");
  }
  
  int schemeidx = ctlidx_from_str(rd->control_scheme);
  if (schemeidx < 0) {
    cJSON_Delete(json);
    ferr("read_control_scheme", "control scheme JSON must name a known scheme. See rheometer -h for a list.");
  }

  struct control_params *params = malloc(sizeof(struct control_params));
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

  rd->control_params = params;

}
