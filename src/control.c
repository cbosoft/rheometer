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
  CONTROL_PID,
  CONTROL_NONE
} control_scheme_enum;


typedef enum setter_scheme {
  SETTER_CONSTANT, 
  SETTER_SINE,
  SETTER_BISTABLE
} setter_scheme_enum;




void control_help(void)
{
  fprintf(stdout,
      "  "BOLD"Control Schemes"RESET"\n"
      "    Control schemes are defined in JSON files (stored in the data/ directory).\n"
      "    These JSON files dictate what function should be used, and what parameters \n"
      "    are passed. Different functions and their associated parameters are discussed\n"
      "    below. Scheme names, and all parameter names, are expected to be written in\n"
      "    lowercase in the JSON file.\n"
      "\n"
      "  - "BOLD"PID control"RESET", 'pid'\n"
      "      Uses the velocity PID algoirithm to reject disturbance and maintain a set point:\n\n"
      "        Δcaₙ = (KP×(errₙ - errₙ₋₁) + (KI×errₙ×Δt) + (KD×(errₙ - (2×errₙ₋₁) + errₙ₋₂)/Δt)\n\n"
      "      Three tuning parameters (kp, ki, kd) are required, delta time is measured setpoint \n"
      "      parameter. It is usual to not use all three parts of the algorithm, a section can be \n"
      "      turned off by setting the relevant coefficient to zero.\n"
      "\n"
      "  - "BOLD"No control"RESET", 'none'\n"
      "      None actual controlling of the variable is performed; just a simple conversion from \n"
      "      the stress/strainrate setpoint to DC. A single required parameter is \"mult\", the \n"
      "      multiplier which does the conversion. This method is not intended to provide robust \n"
      "      control, but is intended to take the controller out of the equation.\n"
      "\n"
      "  "BOLD"Setpoint (setter) Schemes"RESET"\n"
      "    Setpoint for the controller is set by one of these functions. It takes in sensor \n"
      "    readings and time, then deciding the new setpoint. Normally this is a constant, \n"
      "    being varied by the user between different constants between runs. It is possible that\n"
      "    dynamic testing may be applied which would require the use of one of the other functions.\n"
      "    This is a property within the control scheme json file. See the ./dat folder for examples.\n"
      "\n"
      "  - "BOLD"Constant value setter"RESET", 'constant'\n"
      "      A constant setpoint, the only required parameter is \"setpoint\", the constant (double)\n"
      "      value of stress/strainrate to provide the controller.\n"
      "\n"
      "  - "BOLD"Sinusoid setter"RESET", 'sine'\n"
      "      Sine function operating on the time the experiment has run for. Evaluated as:\n"
      "        ca = ( sin( 2 × Pi × (t / period) ) × magnitude) + magnitude + mean\n"
      "      This will produce a sine wave which varies around \"mean\" by an amount \"magnitude\"\n"
      "      with period \"period\".\n"
      "\n"
      "  - "BOLD"Bistable"RESET", 'bistable'\n"
      "      Switch output between two values (\"upper\" and \"lower\") every \"period\" seconds.\n"
      "\n"
  );
}



void calculate_control_indicators(struct run_data *rd) 
{

  rd->speed_ind = get_speed(rd); // speed in rotations per second (hz)

  double strainrate_invs = rd->speed_ind * PI * 2.0 * RI / (RO - RI);
  rd->strainrate_ind = strainrate_invs;

  double stress_Pa = (*rd->loadcell_units) / (2.0 * PI * RI * RI * rd->fill_depth);
  rd->stress_ind = stress_Pa;

  rd->viscosity_ind = stress_Pa / strainrate_invs;
}


/*
  Control algorithms: functions that convert a setpoint (in stress, or
  strainrate) to a DC.
 */

unsigned int pid_control(struct run_data *rd)
{
  /*
    PID - proportional -- integral -- derivative control

    Usual velocity algorthm:

    dCA = KP*dErr + KI*Err*dt + KD * (Err - 2Err1 + Err2)/dt

   */

  double dca = 0.0;
  double input = (rd->control_params->is_stress_controlled) ? rd->stress_ind : rd->strainrate_ind;
  double err = rd->control_params->setpoint - input;
  double delta_t = rd->control_params->sleep_ms * 0.001;

  
  // Proportional control
  dca += rd->control_params->kp * (err - rd->err1);
  
  // Integral control
  dca += rd->control_params->ki * err * delta_t;

  // Derivative control
  dca += rd->control_params->kd * (err - rd->err1 - rd->err1 + rd->err2) / delta_t;

  // Update error history
  rd->err2 = rd->err1;
  rd->err1 = err;

  return (unsigned int)(dca + rd->last_ca);
}




unsigned int no_control(struct run_data *rd) {

  /*
    "no_control" - multiplies the setpoint by a number to get a DC, no adaptive
    control or anything.
   */

  return (unsigned int)(rd->control_params->mult * rd->control_params->setpoint);
}





// setpoint updaters "setters"

double constant_setter(struct run_data *rd)
{
  return rd->control_params->setpoint;
}



double sine_setter(struct run_data *rd)
{
  double theta = 2.0 * PI * rd->time_s_f / rd->control_params->period;
  double rv = (sin(theta) * (double)rd->control_params->magnitude) + rd->control_params->mean;
  return (unsigned int)rv;
}



double bistable_setter(struct run_data *rd)
{
  int periods_passed = (int)(rd->time_s_f / rd->control_params->period);
  return periods_passed % 2 ? rd->control_params->upper : rd->control_params->lower;
}



int ctlidx_from_str(const char *s)
{
  if STREQ(s, "pid")
    return CONTROL_PID;
  else if STREQ(s, "none")
    return CONTROL_NONE;
  return -1;
}


int setidx_from_str(const char *s)
{
  if STREQ(s, "constant")
    return SETTER_CONSTANT;
  else if STREQ(s, "bistable")
    return SETTER_BISTABLE;
  else if STREQ(s, "sine")
    return SETTER_SINE;
  return -1;
}




control_func_t ctlfunc_from_str(char *s)
{
  if STREQ(s, "pid")
    return &pid_control;
  else if STREQ(s, "none")
    return &no_control;
  ferr("ctlfunc_from_int", "unrecognised control scheme index");
  return NULL;
}


setter_func_t setfunc_from_str(char *s)
{
  if STREQ(s, "constant")
    return &constant_setter;
  else if STREQ(s, "bistable")
    return &bistable_setter;
  else if STREQ(s, "sine")
    return &sine_setter;
  ferr("setfunc_from_str", "unrecognised setter scheme index");
  return NULL;
}




void *ctl_thread_func(void *vptr)
{
  struct run_data *rd = (struct run_data *)vptr;
  
  control_func_t ctlfunc = ctlfunc_from_str(rd->control_scheme);
  setter_func_t setfunc = setfunc_from_str(rd->setter_scheme);
  
  rd->ctl_ready = 1;

  while ( (!rd->stopped) && (!rd->errored) ) {

    calculate_control_indicators(rd);

    rd->control_params->setpoint = setfunc(rd);

    unsigned int control_action = ctlfunc(rd);

    if (control_action > CONTROL_MAXIMUM)
      control_action = CONTROL_MAXIMUM;

    if ((int)control_action < CONTROL_MINIMUM)
      control_action = CONTROL_MINIMUM;

    pwmWrite(PWM_PIN, control_action);

    rd->last_ca = control_action;

    sleep_ms(rd->control_params->sleep_ms);

  }

  pthread_exit(0);

  return NULL;
}




void get_control_scheme_parameter(cJSON *json, unsigned int type, const char *schemename, const char *paramname, const char *description, double *dbl_value, unsigned int *int_value, char **str_value)
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




void read_control_scheme(struct run_data *rd, const char *control_scheme_json_path)
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

  cJSON *control_scheme_name_json = cJSON_GetObjectItem(json, "control");
  if (cJSON_IsString(control_scheme_name_json) && (control_scheme_name_json->valuestring != NULL)) {

    rd->control_scheme = calloc(strlen(control_scheme_name_json->valuestring)+1, sizeof(char));
    strcpy(rd->control_scheme, control_scheme_name_json->valuestring);

    rd->control_scheme_path = calloc(strlen(control_scheme_json_path)+1, sizeof(char));
    strcpy(rd->control_scheme_path, control_scheme_json_path);

  }
  else {
    cJSON_Delete(json);
    ferr("read_control_scheme", "control scheme json must name a control scheme.\n  e.g. { ... \"control\": \"pid\" ... }");
  }

  cJSON *setter_scheme_name_json = cJSON_GetObjectItem(json, "setter");
  if (cJSON_IsString(setter_scheme_name_json) && (setter_scheme_name_json->valuestring != NULL)) {

    rd->setter_scheme = calloc(strlen(setter_scheme_name_json->valuestring)+1, sizeof(char));
    strcpy(rd->setter_scheme, setter_scheme_name_json->valuestring);

  }
  else {
    cJSON_Delete(json);
    ferr("read_control_scheme", "control scheme json must name a setter scheme.\n  e.g. { ... \"setter\": \"constant\" ... }");
  }
  
  int control_idx = ctlidx_from_str(rd->control_scheme);
  if (control_idx < 0) {
    cJSON_Delete(json);
    ferr("read_control_scheme", "control scheme JSON must name a known control scheme. See rheometer -h for a list.");
  }

  int setter_idx = setidx_from_str(rd->setter_scheme);
  if (setter_idx < 0) {
    cJSON_Delete(json);
    ferr("read_control_scheme", "control scheme JSON must name a known setter scheme. See rheometer -h for a list.");
  }

  struct control_params *params = malloc(sizeof(struct control_params));
  params->kp = 0.0;
  params->ki = 0.0;
  params->kd = 0.0;
  params->setpoint = 0.0;
  params->sleep_ms = 100;
  params->is_stress_controlled = 0;
  params->mult = 1.0;
  
  switch (control_idx) {
    case CONTROL_PID:
      get_control_scheme_parameter(json, PARAM_REQ, "pid", "kp", "proportional control coefficient, double", &params->kp, NULL, NULL);
      get_control_scheme_parameter(json, PARAM_REQ, "pid", "ki", "integral control coefficient, double", &params->ki, NULL, NULL);
      get_control_scheme_parameter(json, PARAM_REQ, "pid", "kd", "derivative control coefficient, double", &params->kd, NULL, NULL);
      get_control_scheme_parameter(json, PARAM_OPT, "pid", "stress_controlled", "boolean: true if stress is controlled parameter, or false for strainrate control", NULL, &params->is_stress_controlled, NULL);
      break;
    case CONTROL_NONE:
      get_control_scheme_parameter(json, PARAM_REQ, "none", "mult", "multiplier: used to convert the constant setpoint to a DC. Accuracy of this is not assumed, this is a very very rough conversion.", &params->mult, NULL, NULL);
      break;
    default:
      // will not get here; is checked in ctlidx_from_str(char *)
      break;
  }

  switch(setter_idx) {
    case SETTER_CONSTANT:
      get_control_scheme_parameter(json, PARAM_REQ, "constant", "setpoint", "set point/target, double", &params->setpoint, NULL, NULL);
      break;
    case SETTER_SINE:
      get_control_scheme_parameter(json, PARAM_REQ, "sine", "magnitude", "amplitude of the sine wave (mean to peak), double", &params->magnitude, NULL, NULL);
      get_control_scheme_parameter(json, PARAM_REQ, "sine", "mean", "mean value of the sine wave, double", &params->mean, NULL, NULL);
      get_control_scheme_parameter(json, PARAM_REQ, "sine", "period", "period of the sine wave in seconds, double", &params->period, NULL, NULL);
      break;
    case SETTER_BISTABLE:
      get_control_scheme_parameter(json, PARAM_REQ, "bistable", "lower", "lower stable value, double", &params->lower, NULL, NULL);
      get_control_scheme_parameter(json, PARAM_REQ, "bistable", "upper", "upper stable value, double", &params->upper, NULL, NULL);
      get_control_scheme_parameter(json, PARAM_REQ, "bistable", "period", "switching period in seconds, double", &params->period, NULL, NULL);
      break;
    default:
      // will not get here; is checked in ctlidx_from_str(char *)
      break;
  }

  // universal (optional) params
  get_control_scheme_parameter(json, PARAM_OPT, "*", "sleep_ms", "interval between control calculations, milliseconds", NULL, &params->sleep_ms, NULL);

  cJSON_Delete(json);

  rd->control_params = params;

}
