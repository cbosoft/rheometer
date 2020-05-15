#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>

#include <wiringPi.h>

#include "../util/error.h"
#include "../sensors/encoder/encoder.h"
#include "../motor/motor.h"
#include "../util/sleep.h"
#include "../util/json.h"
#include "../geometry.h"
#include "../sensors/loadcell/loadcell.h"

#include "control.h"




#define PARAM_REQ 0
#define PARAM_OPT 1
#define PI 3.1415926

#define STREQ(A,B) (strcmp(A, B) == 0)








void calculate_control_indicators(struct run_data *rd)
{

  // update speed from optenc
  set_speed(rd, measure_speed());
  // speed in rotations per second (hz)

}








void *ctl_thread_func(void *vptr)
{
  struct run_data *rd = (struct run_data *)vptr;

  ControllerHandle *controller = load_controller(rd->control_scheme);
  SetterHandle *setter = load_setter(rd->setter_scheme);

  rd->ctl_ready = 1;

  while ( (!rd->stopped) && (!rd->errored) ) {

    calculate_control_indicators(rd);

    rd->control_params->setpoint = setter->get_setpoint(rd);

    unsigned int control_action = controller->get_control_action(rd);

    if (control_action > CONTROL_MAXIMUM)
      control_action = CONTROL_MAXIMUM;

    if ((int)control_action < CONTROL_MINIMUM)
      control_action = CONTROL_MINIMUM;

    pwmWrite(PWM_PIN, control_action);

    set_last_control_action(rd, control_action);

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

  struct control_params *params = malloc(sizeof(struct control_params));
  params->sleep_ms = 100;
  params->is_stress_controlled = 0;
  params->controller = load_controller(rd->control_scheme);
  params->setter = load_setter(rd->setter_scheme);
  params->setpoint = 0.0;


  cJSON *control_params_json = cJSON_GetObjectItem(json, "control_params");
  if (cJSON_IsArray(control_params_json)) {

    params->control_params = malloc(cJSON_GetArraySize(control_params_json)*sizeof(double));
    int i = 0;
    for (cJSON *it = control_params_json->child; it != NULL; it = it->next, i++) {
      params->control_params[i] = it->valuedouble;
    }
    params->n_control_params = i;

  }
  else {
    warn("read_control_scheme", "could not read control params.");
  }

  cJSON *setter_params_json = cJSON_GetObjectItem(json, "setter_params");
  if (cJSON_IsArray(setter_params_json)) {

    params->setter_params = malloc(cJSON_GetArraySize(setter_params_json)*sizeof(double));
    int i = 0;
    for (cJSON *it = setter_params_json->child; it != NULL; it = it->next, i++) {
      params->setter_params[i] = it->valuedouble;
    }
    params->n_setter_params = i;

  }
  else {
    warn("read_control_scheme", "could not read setter params.");
  }



  // universal (optional) params
  get_control_scheme_parameter(json, PARAM_OPT, "*", "sleep_ms", "interval between control calculations, milliseconds", NULL, &params->sleep_ms, NULL);

  cJSON_Delete(json);

  rd->control_params = params;

}
