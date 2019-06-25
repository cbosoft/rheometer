#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "cJSON.h"
#include "rheo.h"

#define EITHER(A,B,C) ( (strcmp(A, B) == 0) || (strcmp(A, C) == 0) )




void
usage(void)
{
  fprintf(stderr, 
      "\n"
      "  "BOLD"rheometer"RESET" control program v"VERSION"\n"
      "\n"
      "  Usage:\n"
      "    rheometer -l|--length <length> -c|--control-scheme <control scheme> [-t|--tag <tag>]\n"
      "    rheometer -h|--help\n"
      "\n"
  );
}




void
help(void)
{
  usage();
  fprintf(stderr,
      "  Options:\n"
      "    -l | --length    Length of run, can be given in seconds or minutes, dictated\n"
      "                     by a suffixed character. e.g. \"10s\" is 10 seconds, \"10m\"\n"
      "                     is 10 minutes. If ignored, the value is taken as seconds.\n"
      "\n"
      "    -c | --control-scheme    Control scheme JSON file path. The JSON object\n"
      "                     describes the control scheme to use in the run. Each control\n"
      "                     scheme has different parameters it requires. E.g. the\n"
      "                     constant scheme only requires one parameter; \"c\" which is\n"
      "                     the constant value. PID control requires three different\n"
      "                     tuning params, and a set point.\n"
      "\n"
      "    -t | --tag       A short descriptive name for the test run. Underscores in the\n"
      "                     tag will be replaced by hyphens. Optional, default is \""TAGDEFAULT"\".\n"
      "\n"
  );
}




unsigned int
parse_length_string(const char *length_s_str)
{
  unsigned int len = strlen(length_s_str);

  // is just numbers?
  unsigned int justnumber = 1;
  unsigned int notnumbers = 0;
  for (unsigned int i = 0; i < len; i++) {
    unsigned int ic = ((unsigned int)length_s_str[i]);
    if ( ic < 48 || ic > 57) {
      justnumber = 0;
      notnumbers ++;
    }
  }

  if (notnumbers > 1) {
    argerr("length arg must be a number or suffixed by a single 's' or 'm' to explicitly specify 'seconds' or 'minutes'");
  }

  unsigned int toi = atoi(length_s_str);

  if (length_s_str[len-1] == 's' || justnumber) {
    return toi;
  }

  if (length_s_str[len-1] == 'm') {
    return toi * 60;
  }

  argerr("length arg syntax error");

  // this will bever run, but it makes the linter happy.
  return -1; //unsigned, so this is actually INT_MAX?
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
}



void
parse_contol_scheme_string(thread_data_t *td, const char *control_scheme_string)
{
  info("reading control scheme");

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

  info("parsing control scheme");
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
    ferr("Control scheme json must name a known scheme.\n  i.e. \"constant\" or \"pid\"");
  }

  control_params_t *params = malloc(sizeof(control_params_t));
  params->c = 0.0;
  params->kp = 0.0;
  params->ki = 0.0;
  params->kd = 0.0;
  params->set_point = 0.0;
  params->sleep_ns = 100*1000*1000;

  if (schemeidx == ctlidx_from_str("constant")) {
    get_control_scheme_parameter(json, "c", "constant", "output value, double", &params->c);
  }
  else {
    get_control_scheme_parameter(json, "kp", "pid", "proportional control coefficient, double", &params->kp);
    get_control_scheme_parameter(json, "ki", "pid", "integral control coefficient, double", &params->ki);
    get_control_scheme_parameter(json, "kd", "pid", "derivative control coefficient, double", &params->kd);
    get_control_scheme_parameter(json, "setpoint", "pid", "set point/target, double", &params->set_point);
  }

  // optional params
  get_optional_control_scheme_parameter(json, "sleep_ns", "interval between control calculations, nanoseconds", NULL, &params->sleep_ns, NULL);

  cJSON_Delete(json);

  td->control_params = params;

}




void
check_argc(int i, int argc) 
{
  if (i >= argc) {
    argerr("Option needs a value!");
  }
}

void
parse_args(int argc, const char **argv, thread_data_t *td) 
{
  td->tag = "DELME";
  unsigned int cs_set = 0, l_set = 0;

  for (unsigned int i = 1; i < argc; i++) {
    if (EITHER(argv[i], "-l", "--length")) {
      i++;
      check_argc(i, argc);
      td->length_s = parse_length_string(argv[i]);
      l_set = 1;
    }
    else if (EITHER(argv[i], "-c", "--control-scheme")) {
      i++;
      check_argc(i, argc);
      parse_contol_scheme_string(td, argv[i]);
      cs_set = 1;
    }
    else if (EITHER(argv[i], "-t", "--tag")) {
      i++;
      check_argc(i, argc);
      td->tag = argv[i];
    }
    else if (EITHER(argv[i], "-h", "--help")) {
      help();
      exit(0);
    }
    else {
      argerr("given unknown arg");
    }
  }

  if (getuid() != 0)
    argerr("Hardware PWM needs root.");

  if (!cs_set || !l_set)
    argerr("Need to specify both length and control scheme.");

  // finished setting args
  fprintf(stderr, 
      "  "BOLD"rheometer"RESET" v"VERSION"\n"
      "  Running with options:\n"
      "    tag: \"%s\"\n"
      "    control scheme: %s\n"
      "      c: %.3f\n"
      "      kp: %.3f\n"
      "      ki: %.3f\n"
      "      kd: %.3f\n"
      "    length: %us\n",
      td->tag, td->control_scheme, td->control_params->c, td->control_params->kp, td->control_params->ki, td->control_params->kd, td->length_s);
}
