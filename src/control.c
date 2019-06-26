#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>

#include "rheo.h"
#include "cJSON.h"


static const char *control_schemes[] = {
  "constant",
  "pid"
};



int
ctlidx_from_str(const char *s)
{
  for (unsigned int i = 0; i < sizeof(control_schemes)/sizeof(char *); i++) {
    if (strcmp(s, control_schemes[i]) == 0)
      return i;
  }

  return -1;
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

  if (count == 0) {
    warn("speed requested but no optenc events have been recorded.");
  }

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
  float err = input - td->control_params->set_point;
  
  // Proportional control
  dca += td->control_params->kp * err;

  // Intregral control
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
  return (unsigned int)(td->control_params->c * 10.24);
}




control_func_t
ctlfunc_from_int(int i)
{
  switch (i) {
    case control_constant:
      return &constant_control;
    case control_pid:
      return &pid_control;
    default:
      ferr("unrecognised control scheme index");
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

    if (control_action > 1024)
      control_action = 1024;

    pwmWrite(PWM_PIN, control_action);

    td->last_ca = control_action;

    nsleep(td->control_params->sleep_ns);

  }

  return NULL;
}




void
get_control_scheme_parameter(cJSON *json, const char *paramname, const char *schemename, const char *description, double *value)
{
  cJSON *dbl = cJSON_GetObjectItem(json, paramname);

  if (dbl == NULL) {
    char err_mesg[1000] = {0};
    sprintf(err_mesg, "%s requires a parameter \"%s\" (%s)", schemename, paramname, description);
    cJSON_Delete(json);
    ferr(err_mesg);
  }

  if (!cJSON_IsNumber(dbl)) {
    char err_mesg[1000] = {0};
    sprintf(err_mesg, "%s requires \"%s\" to be a number (%s)", schemename, paramname, description);
  }
  
  (*value) = dbl->valuedouble;
}




void
get_optional_control_scheme_parameter(cJSON *json, const char *paramname, const char *description, double *dbl_value, unsigned int *int_value, char **str_value)
{
  cJSON *param = cJSON_GetObjectItem(json, paramname);

  if (param == NULL)
    return;

  if (cJSON_IsNumber(param)) {

    if (dbl_value == NULL) {

      if (int_value == NULL) {
        char err_mesg[1000] = {0};
        sprintf(err_mesg, "JSON parse error. \"%s\" is not expected to be a number (%s).", paramname, description);
        ferr(err_mesg);
      }
      else {
        (*int_value) = param->valueint;
      }

    }
    else {
      (*dbl_value) = param->valuedouble;
    }

  } 
  else if (cJSON_IsString(param)) {

    if (str_value == NULL) {
        char err_mesg[1000] = {0};
        sprintf(err_mesg, "JSON parse error. \"%s\" is not expected to be a string (%s).", paramname, description);
        ferr(err_mesg);
    }
    else {
      (*str_value) = param->valuestring;
    }

  }
  else if (cJSON_IsBool(param)) {

    if (int_value == NULL) {
        char err_mesg[1000] = {0};
        sprintf(err_mesg, "JSON parse error. \"%s\" is not expected to be a boolean (%s).", paramname, description);
        ferr(err_mesg);
    }
    else {
      if (cJSON_IsTrue(param))
        (*int_value) = 1;
      else
        (*int_value) = 0;
    }

  }
  else {
    ferr("Unknown type encountered while processing control scheme JSON.");
  }
}



void
read_control_scheme(thread_data_t *td, const char *control_scheme_string)
{

  if (access(control_scheme_string, F_OK) == -1) {
    argerr("control scheme must be a json file describing the scheme.");
  }

  FILE *fp = fopen(control_scheme_string, "r");

  if (fp == NULL)
    ferr("Something went wrong while opening control scheme.");
  
  char ch;
  unsigned int count = 0, i = 0;

  while ( (unsigned char)(ch = fgetc(fp)) != (unsigned char)EOF ) {
    count++;
    if (count > 1000)
      ferr("control scheme is certainly not over 1000 characters long.");
  }

  if (fseek(fp, 0L, SEEK_SET) != 0)
    ferr("Something went wrong repositioning file.");

  char *json_str = calloc(count+1, sizeof(char));
  while ( (unsigned char)(ch = fgetc(fp)) != (unsigned char)EOF) {
    json_str[i] = ch;
    i++;
  }

  fclose(fp);

  cJSON *json = cJSON_Parse(json_str);
  free(json_str);

  if (json == NULL) {
    const char *eptr = cJSON_GetErrorPtr();
    char err_mesg[1000] = {0};
    if (eptr != NULL)
      sprintf(err_mesg, "JSON parse failed before %s", eptr);
    else
      sprintf(err_mesg, "JSON parse failed.");
    cJSON_Delete(json);
    ferr(err_mesg);
  }

  cJSON *control_scheme_name_json = cJSON_GetObjectItem(json, "name");
  if (cJSON_IsString(control_scheme_name_json) && (control_scheme_name_json->valuestring != NULL)) {
    td->control_scheme = calloc(strlen(control_scheme_name_json->valuestring)+1, sizeof(char));
    strcpy(td->control_scheme, control_scheme_name_json->valuestring);
  }
  else {
    cJSON_Delete(json);
    ferr("Control scheme json must name a scheme.\n  e.g. { ... \"name\": \"constant\" ... }");
  }
  
  int schemeidx = ctlidx_from_str(td->control_scheme);
  if (schemeidx < 0) {
    cJSON_Delete(json);
    char *err_mesg = calloc(1000, sizeof(char));
    strcat(err_mesg, "Control scheme JSON must name a known scheme. Known schemes:");
    for (unsigned int i = 0; i < sizeof(control_schemes)/sizeof(char*); i++) {
      strcat(err_mesg, "\n   - ");
      strcat(err_mesg, control_schemes[i]);
    }
    ferr(err_mesg);
  }

  control_params_t *params = malloc(sizeof(control_params_t));
  params->c = 0.0;
  params->kp = 0.0;
  params->ki = 0.0;
  params->kd = 0.0;
  params->set_point = 0.0;
  params->sleep_ns = 100*1000*1000;
  params->is_stress_controlled = 0;

  if (schemeidx == ctlidx_from_str("constant")) {
    get_control_scheme_parameter(json, "c", "constant", "output value, double", &params->c);
  }
  else {
    get_control_scheme_parameter(json, "kp", "pid", "proportional control coefficient, double", &params->kp);
    get_control_scheme_parameter(json, "ki", "pid", "integral control coefficient, double", &params->ki);
    get_control_scheme_parameter(json, "kd", "pid", "derivative control coefficient, double", &params->kd);
    get_control_scheme_parameter(json, "setpoint", "pid", "set point/target, double", &params->set_point);

    // pid specific optional
    get_optional_control_scheme_parameter(json, "stress_controlled", "boolean: true if stress is controlled parameter, or false for strainrate control", NULL, &params->sleep_ns, NULL);
  }

  // optional params
  get_optional_control_scheme_parameter(json, "sleep_ns", "interval between control calculations, nanoseconds", NULL, &params->sleep_ns, NULL);

  cJSON_Delete(json);

  td->control_params = params;

}
