#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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
      "    rheometer -l <length> -d <depth> -c <control scheme> [-t <tag>]\n"
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
      "    -d | --depth     Fill depth of Couette cell. This is the height fluid extends \n"
      "                     up on the "BOLD"inner"RESET" cylinder, in mm.\n"
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
check_argc(unsigned int i, unsigned int argc) 
{
  if (i >= argc) {
    argerr("Option needs a value!");
  }
}




void
parse_args(unsigned int argc, const char **argv, thread_data_t *td) 
{
  td->tag = "DELME";
  unsigned int cs_set = 0, l_set = 0, d_set = 0;

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
      read_control_scheme(td, argv[i]);
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
    else if (EITHER(argv[i], "-d", "--depth")) {
      i++;
      check_argc(i, argc);
      td->fill_depth = atof(argv[i]);
      d_set = 1;
    }
    else {
      argerr("given unknown arg");
    }
  }

  if (getuid() != 0)
    argerr("Hardware PWM needs root.");

  if (!cs_set || !l_set || !d_set)
    argerr("Length, control scheme, and depth are required parameters.");

  // finished setting args, display
  fprintf(stderr, 
      "  "BOLD"rheometer"RESET" v"VERSION"\n"
      "\n"
      "  Run options:\n"
      "    "FGYELLOW"tag"RESET": \"%s\"\n"
      "    "FGYELLOW"control scheme"RESET": %s\n"
      "      "FGMAGENTA"c"RESET": %.3f\n"
      "      "FGMAGENTA"kp"RESET": %.3f\n"
      "      "FGMAGENTA"ki"RESET": %.3f\n"
      "      "FGMAGENTA"kd"RESET": %.3f\n"
      "      "FGMAGENTA"set_point"RESET": %.3f\n"
      "      "FGMAGENTA"control interval"RESET": %f ms\n"
      "      "FGMAGENTA"controlled variable"RESET": %s\n"
      "    "FGYELLOW"length"RESET": %u s\n",
      td->tag, 
      td->control_scheme, 
        td->control_params->c, 
        td->control_params->kp, 
        td->control_params->ki, 
        td->control_params->kd, 
        td->control_params->set_point, 
        (float)(td->control_params->sleep_ns)*0.001*0.001,
        td->control_params->is_stress_controlled ? "stress" : "strainrate",
      td->length_s);
}
