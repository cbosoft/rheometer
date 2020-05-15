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

struct controller_analysis_result {
  double stddev_error;
  double average_error;
};
extern pthread_mutex_t lock_control, lock_speed, lock_loadcell;




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

  pthread_mutex_lock(&lock_speed);
  rd->speed_ind = get_speed(rd); // speed in rotations per second (hz)
  pthread_mutex_unlock(&lock_speed);

  double strainrate_invs = rd->speed_ind * PI * 2.0 * RI / (RO - RI);

  pthread_mutex_lock(&lock_loadcell);
  double stress_Pa = rd->loadcell_units / (2.0 * PI * RI * RI * rd->fill_depth);
  pthread_mutex_unlock(&lock_loadcell);

  rd->strainrate_ind = strainrate_invs;
  rd->stress_ind = stress_Pa;
  rd->viscosity_ind = stress_Pa / strainrate_invs;
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

    pthread_mutex_lock(&lock_control);
    rd->last_ca = control_action;
    pthread_mutex_unlock(&lock_control);

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

















void analyse(int length, struct run_data *rd)
{
  double *error = calloc(length, sizeof(double));
  double total = 0.0;
  int plotting = 1;
  FILE *fp = fopen("control_analysis_data.csv", "w");

  if (fp == NULL) plotting = 0;

  fprintf(stderr, "Gathering data...\n");
  for (int i = 0; i < length; i++) {
    error[i] = rd->err1;
    total += error[i];
    fprintf(stderr, "  %d/%d    \r", i+1, length);
    if (plotting) {
      fprintf(fp, "%f\n", error[i]);
    }
    sleep(1);
  }
  fprintf(stderr, "\n");
  fclose(fp);

  double average_error = total/((double)length);

  double sumsqdiff = 0.0;
  for (int i = 0; i < length; i++)
    sumsqdiff += pow(error[i] - average_error, 2.0);

  double stddev_error = pow( sumsqdiff/((double)(length - 1)) ,0.5);

  system("gnuplot /home/pi/gits/rheometer/plot_control_analysis.gplot");
  fprintf(stderr, 
      BOLD"Analysis results"RESET": Eav: %f, std: %f\n", 
      average_error, 
      stddev_error);
}











void do_tuning(struct run_data *rd) {
  /*
   * User selects setpoint and an initial set of params, from a scheme.json.
   * Rheometer runs for a period of time, before allowing the user to select a
   * new set of tuning parameters.  */

  int l = 10, analyse_length = 0;
  fprintf(stderr, 
      "  "BOLD"Welcome to the TD tuner."RESET"\n"
      "\n"
      "    The controller will run for %ds, before dropping to an interactive shell\n"
      "    so you may alter tuning parameters. In the shell, type 'help' to get a list\n"
      "    of valid commands.\n",
      l);
  
  
  double value = 0.0;
  char input[100], cmd[100], variable[50];

  while ( (!rd->stopped) && (!rd->errored) ) {
    
    fprintf(stderr, "Waiting...\n");
    for (int i = 0; i < l; i++) {
      fprintf(stderr, "  %d/%d    \r", i+1, l);
      sleep(1);
    }
    fprintf(stderr, "\n");

    // TODO create new log holding the changes in tuning

    while (1) {
      // read user input
      fprintf(stderr, "%s", ": ");

      fgets(input, 100, stdin);
      int inlen = strlen(input);
      if (inlen > 1){
        input[inlen-1] = '\0';
      }

      if (strncmp(input, "set", 3) == 0) {
        int nmatch = sscanf(input, "%s %s %lf", cmd, variable, &value);

        if (nmatch < 3) {
          fprintf(stderr, "'set' needs two arguments.\n");
          continue;
        }

      }
      else if (strncmp(input, "analyse", 7) == 0) {
        int nmatch = sscanf(input, "%s %d", cmd, &analyse_length);

        if (nmatch < 2) {
          analyse_length = 30;
        }
      }
      else {
        strncpy(cmd, input, 99);
      }

      if (strcmp(cmd, "help") == 0) {
        fprintf(stderr, 
            "  "BOLD"Commands:"RESET"\n"
            "    show                 show the current params\n"
            "    set <var> <val>      set var to value, valid vars: kp, ki, kd\n"
            "    analyse [<len>]      analyse a <len=30> second section of data.\n"
            //"    autotune [<method>]  run an autotuning algorithm.\n" // TODO
            "    done                 finish tuning\n"
            );
      }
      else if (strcmp(cmd, "set") == 0) {

        if (strcmp(variable, "kp") == 0) {
          rd->control_params->kp = value;
        }
        else if (strcmp(variable, "ki") == 0) {
          rd->control_params->ki = value;
        }
        else if (strcmp(variable, "kd") == 0) {
          rd->control_params->kd = value;
        }
        // TODO log tuning parameter change

      }
      else if (strcmp(cmd, "analyse") == 0) {
        
        analyse(analyse_length, rd);

      }
      else if (strcmp(cmd, "autotune") == 0) {

        // TODO

      }
      else if (strcmp(cmd, "show") == 0) {
        fprintf(stderr, 
            "  KP = %f\n"
            "  KI = %f\n"
            "  KD = %f\n",
            rd->control_params->kp,
            rd->control_params->ki,
            rd->control_params->kd);
      }
      else if (strcmp(cmd, "done") == 0) {
        fprintf(stderr, BOLD"Tuning finished!"RESET"\n");
        return;
      }
      else {
        fprintf(stderr, "Input not understood. Try 'help' for help.\n");
      }
    }
  }
}
