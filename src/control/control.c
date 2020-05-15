#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <glob.h>

#include <dlfcn.h>
#include <wiringPi.h>

#include "../util/error.h"
#include "../util/display.h"
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
  fprintf(stdout,"%s",
      "  "BOLD"Control Schemes"RESET"\n"
      "    Control schemes are defined in JSON files (stored in the data/ directory).\n"
      "    These JSON files dictate what function should be used, and what parameters \n"
      "    are passed. Different functions and their associated parameters are discussed\n"
      "    below. Scheme names, and all parameter names, are expected to be written in\n"
      "    lowercase in the JSON file.\n");

  glob_t res;
  glob("./controllers/*.so", 0, NULL, &res);

  for (size_t i = 0; i < res.gl_pathc; i++) {
    ControllerHandle *h = load_controller_path(res.gl_pathv[i]);
    fprintf(stdout, "%s\n", h->doc);
    free_controller(h);
  }

  globfree(&res);


  fprintf(stdout, "%s",
      "\n"
      "  "BOLD"Setpoint (setter) Schemes"RESET"\n"
      "    Setpoint for the controller is set by one of these functions. It takes in sensor \n"
      "    readings and time, then deciding the new setpoint. Normally this is a constant, \n"
      "    being varied by the user between different constants between runs. It is possible that\n"
      "    dynamic testing may be applied which would require the use of one of the other functions.\n"
      "    This is a property within the control scheme json file. See the ./dat folder for examples.\n");
  
  glob("./setters/*.so", 0, NULL, &res);

  for (size_t i = 0; i < res.gl_pathc; i++) {
    SetterHandle *h = load_setter_path(res.gl_pathv[i]);
    fprintf(stdout, "%s\n", h->doc);
    free_setter(h);
  }

  globfree(&res);

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


ControllerHandle *load_controller(const char *name)
{
  char *path = malloc(401*sizeof(char));
  snprintf(path, 400, "./controllers/%s.so", name);
  ControllerHandle *rv = load_controller_path(path);
  free(path);

  return rv;
}

ControllerHandle *load_controller_path(const char *path)
{
  ControllerHandle *h = calloc(1, sizeof(ControllerHandle));

  h->handle = dlopen(path, RTLD_LAZY);
  if (!h->handle) {
    ferr("load_controller", "%s", dlerror());
  }

  dlerror();
  char **doc_ptr = dlsym(h->handle, "doc");
  char *error = NULL;
  if ((error = dlerror()) != NULL) {
    ferr("load_controller", "%s", error);
  }
  h->doc = *doc_ptr;
  fprintf(stderr, "%s\n", (char*)doc_ptr);

  h->get_control_action = dlsym(h->handle, "get_control_action");
  if ((error = dlerror()) != NULL) {
    ferr("load_controller", "%s", error);
  }

  return h;
}

void free_controller(ControllerHandle *h)
{
  free(h);
}

SetterHandle *load_setter(const char *name)
{
  char *path = malloc(401*sizeof(char));
  snprintf(path, 400, "./setters/%s.so", name);
  SetterHandle *rv = load_setter_path(path);
  free(path);
  return rv;
}

SetterHandle *load_setter_path(const char *path)
{
  SetterHandle *h = calloc(1, sizeof(SetterHandle));

  h->handle = dlopen(path, RTLD_LAZY);

  if (!h->handle) {
    ferr("load_setter", "%s", dlerror());
  }

  dlerror();
  char **doc_ptr = dlsym(h->handle, "doc");
  char *error = NULL;
  if ((error = dlerror()) != NULL) {
    ferr("load_setter", "%s", error);
  }
  h->doc = *doc_ptr;

  h->get_setpoint = dlsym(h->handle, "get_setpoint");
  if ((error = dlerror()) != NULL) {
    ferr("load_setter", "%s", error);
  }

  return h;
}

void free_setter(SetterHandle *h)
{
  free(h);
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

















void analyse(int length, struct run_data *rd)
{
  double *error = calloc(length, sizeof(double));
  double total = 0.0;
  int plotting = 1;
  FILE *fp = fopen("control_analysis_data.csv", "w");

  if (fp == NULL) plotting = 0;

  fprintf(stderr, "Gathering data...\n");
  for (int i = 0; i < length; i++) {
    double input = (rd->control_params->is_stress_controlled) ? rd->stress_ind : rd->strainrate_ind;
    double err = rd->control_params->setpoint - input;
    error[i] = err;
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

  system("gnuplot ./scripts/plot_control_analysis.gplot");
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

        int i = atoi(variable);

        if ((i > 0) && (i < rd->control_params->n_control_params)) {

          rd->control_params->control_params = realloc(rd->control_params->control_params, (i+1)*sizeof(double) );

          if ((rd->control_params->n_control_params - i) > 1) {

            for (int j = rd->control_params->n_control_params; j < i+1; j++) {
              rd->control_params->control_params[i] = 0;
            }

          }
          
          rd->control_params->n_control_params = i+1;

        }

        rd->control_params->control_params[i] = value;

        // TODO log tuning parameter change

      }
      else if (strcmp(cmd, "analyse") == 0) {
        
        analyse(analyse_length, rd);

      }
      else if (strcmp(cmd, "autotune") == 0) {

        // TODO

      }
      else if (strcmp(cmd, "show") == 0) {
        for (int i = 0; i < rd->control_params->n_control_params; i++) {
          fprintf(stderr, "  p[%d] = %f\n", i, rd->control_params->control_params[i]);
        }
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





double get_control_param_or_default(struct run_data *rd, int index, double def)
{
  int l = rd->control_params->n_control_params;

  if ((index < l) && (index >= 0)) {
    return rd->control_params->control_params[index];
  }

  return def;
}

double get_setter_param_or_default(struct run_data *rd, int index, double def)
{
  int l = rd->control_params->n_setter_params;

  if ((index < l) && (index >= 0)) {
    return rd->control_params->setter_params[index];
  }

  return def;
}
