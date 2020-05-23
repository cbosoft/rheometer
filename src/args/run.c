#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "../util/error.h"
#include "../util/double_array.h"
#include "../log/tag.h"
#include "../control/control.h"

#include "args.h"


void parse_run_args(int argc, const char **argv, struct run_data *rd) 
{
  rd->tag = TAGDEFAULT;
  int l_set=0, d_set=0, hwver_set=0, c_set=0, s_set=0;


  for (int i = 0; i < argc; i++) {
    if (ARGEITHER("-l", "--length")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      rd->length_s = parse_length_string(argv[i]);
      l_set = 1;
    }
    else if (ARGEITHER("-c", "--control-scheme")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      read_control_scheme(rd, argv[i]);
      c_set = 1;
      s_set = 1;
    }
    else if (ARGEQ("--controller")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      rd->control_scheme.controller_name = strdup(argv[i]);
      c_set = 1;
    }
    else if (ARGEQ("--controller-params")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      str2darr(argv[i], &rd->control_scheme.control_params, &rd->control_scheme.n_control_params);
    }
    else if (ARGEQ("--setter")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      rd->control_scheme.setter_name = strdup(argv[i]);
      s_set = 1;
    }
    else if (ARGEQ("--setter-params")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      str2darr(argv[i], &rd->control_scheme.setter_params, &rd->control_scheme.n_setter_params);
    }
    else if (ARGEITHER("-t", "--tag")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      rd->tag = parse_tag_string(argv[i]);
    }
    else if (ARGEITHER("-d", "--fill-depth")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      rd->fill_depth = atof(argv[i]);
      d_set = 1;
    }
    else if (ARGEITHER("-n", "--needle-depth")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      rd->needle_depth = atof(argv[i]);
    }
    else if (ARGEITHER("-v", "--video-device")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      rd->video_device = strdup(argv[i]);
    }
    else if (ARGEITHER("-p", "--photo-device")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      rd->photo_device = strdup(argv[i]);
    }
    else if (ARGEITHER("-w", "--hardware-version")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      rd->hardware_version = atoi(argv[i]);
      hwver_set = 1;
    }
    else if (ARGEITHER("-a", "--loadcell-calibration")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      rd->loadcell_calibration.name = strdup(argv[i]);
    }
    else if (ARGEQ("--calm-start")) {
      rd->calm_start = 1;
    }
    else if (ARGEQ("--tune")) {
      rd->mode = MODE_TUNING;
    }
    else if (ARGEQ("--quiet")) {
      set_quiet();
    }
    else if (ARGEQ("--silent")) {
      set_silent();
    }
    else {
      argerr("Run argument \"%s\" not understood (is it a misplaced common arg?).", argv[i]);
    }
  }

  if (getuid() != 0)
    argerr("Hardware PWM needs root.");

  if (!c_set || !s_set || !l_set || !d_set || !hwver_set)
    argerr("Length, controller and setter (or scheme), hardware_version, and fill depth are required parameters.");
}