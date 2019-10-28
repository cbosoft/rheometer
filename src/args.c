#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "version.h"
#include "args.h"
#include "display.h"
#include "log.h"
#include "control.h"
#include "error.h"


#define EITHER(A,B,C) ( (strcmp(A, B) == 0) || (strcmp(A, C) == 0) )


void
usage(void)
{
  fprintf(stdout, 
      "\n"
      "  "BOLD"rheometer"RESET" control program v"VERSION"\n"
      "\n"
      "  "BOLD"Usage:"RESET"\n"
      "    rheometer -l <length> -d <depth> -c <control scheme> [-t <tag>] [--calm-start]\n"
      "    rheometer -h|--help\n"
      "\n"
  );
}




void
help(void)
{
  usage();
  fprintf(stdout,
      "  "BOLD"Options:"RESET"\n"
      "    -l | --length    Length of run, can be given in seconds or minutes, dictated\n"
      "                     by a suffixed character. e.g. \"10s\" is 10 seconds, \"10m\"\n"
      "                     is 10 minutes. If ignored, the value is taken as seconds.\n"
      "\n"
      "    -d | --depth     Fill depth of Couette cell. This is the height fluid extends \n"
      "                     up on the "BOLD"inner"RESET" cylinder, in mm.\n"
      "\n"
      "    -c | --control-scheme    Control scheme JSON file path. More information in the\n"
      "                     'Control Schemes' section in the help.\n"
      "\n"
      "    -t | --tag       A short descriptive name for the test run. Underscores and spaces\n"
      "                     will be replaced by hyphens. Optional. Default is \""TAGDEFAULT"\".\n"
      "\n"
      "    --calm-start     Enable this flag to remove the high power motor start up. This is\n"
      "                     useful for tuning position of the outer cylinder for very thick or\n"
      "                     non-Newtonian fluids. However, the motor is more likely to stall on \n"
      "                     start, so be wary about doing tests with this data.\n"
      "\n"
      "    -v | --video-device    This argument informs the location of the video device to capture\n"
      "                     from during logging. By setting this option, video logging is enabled.\n"
      "                     A video of the name \"${prefix}_video.mp4\" will be created and stored\n"
      "                     alongside the other log files.\n"
      "\n"
  );
  control_help();
}




unsigned int parse_length_string(const char *length_s_str)
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



char * parse_tag_string(const char *s) 
{
  unsigned int l = strlen(s);
  char *rv = calloc(l+1, sizeof(char));
  for (unsigned int i = 0; i < l; i++) {
    if (s[i] == ' ' || s[i] == '_')
      rv[i] = '-';
    else
      rv[i] = s[i];
  }
  return rv;
}




void check_argc(unsigned int i, unsigned int argc) 
{
  if (i >= argc) {
    argerr("Option needs a value!");
  }
}




void parse_args(unsigned int argc, const char **argv, struct run_data *rd) 
{
  rd->tag = TAGDEFAULT;
  unsigned int cs_set = 0, l_set = 0, d_set = 0;

  for (unsigned int i = 1; i < argc; i++) {
    if (EITHER(argv[i], "-l", "--length")) {
      i++;
      check_argc(i, argc);
      rd->length_s = parse_length_string(argv[i]);
      l_set = 1;
    }
    else if (EITHER(argv[i], "-c", "--control-scheme")) {
      i++;
      check_argc(i, argc);
      read_control_scheme(rd, argv[i]);
      cs_set = 1;
    }
    else if (EITHER(argv[i], "-t", "--tag")) {
      i++;
      check_argc(i, argc);
      rd->tag = parse_tag_string(argv[i]);
    }
    else if (EITHER(argv[i], "-h", "--help")) {
      help();
      exit(0);
    }
    else if (EITHER(argv[i], "-d", "--depth")) {
      i++;
      check_argc(i, argc);
      rd->fill_depth = atof(argv[i]);
      d_set = 1;
    }
    else if (EITHER(argv[i], "-v", "--video-device")) {
      i++;
      check_argc(i, argc);
      rd->video_device = calloc(strlen(argv[i])+1, sizeof(char *));
      strcpy(rd->video_device, argv[i]);
      d_set = 1;
    }
    else if (strcmp(argv[i], "--calm-start") == 0) {
      rd->calm_start = 1;
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
      "  "BOLD"rheometer"RESET" v%s\n"
      "\n"
      "  Run options:\n"
      "    "FGYELLOW"tag"RESET": \"%s\"\n"
      "    "FGYELLOW"control scheme"RESET": %s\n"
      "    "FGYELLOW"setter scheme"RESET": %s\n"
      "      "FGMAGENTA"kp"RESET": %.3f\n"
      "      "FGMAGENTA"ki"RESET": %.3f\n"
      "      "FGMAGENTA"kd"RESET": %.3f\n"
      "      "FGMAGENTA"setpoint"RESET": %.3f\n"
      "      "FGMAGENTA"control interval"RESET": %f ms\n"
      "      "FGMAGENTA"controlled variable"RESET": %s\n"
      "    "FGYELLOW"length"RESET": %u s\n",
      VERSION,
      rd->tag, 
      rd->control_scheme, 
      rd->setter_scheme, 
      rd->control_params->kp, 
      rd->control_params->ki, 
      rd->control_params->kd, 
      rd->control_params->setpoint, 
      (double)(rd->control_params->sleep_ms),
      rd->control_params->is_stress_controlled ? "stress" : "strainrate",
      rd->length_s);
}
