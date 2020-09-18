#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "../util/error.h"
#include "../util/double_array.h"
#include "../control/control.h"

#include "args.h"


void parse_run_args(int argc, const char **argv, struct run_data *rd) 
{
  int l_set=0, d_set=0, hwver_set=0, c_set=0, s_set=0, mat_set=0;


  for (int i = 0; i < argc; i++) {
    if (ARGEITHER("-l", "--length")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      rd->length_s = parse_length_string(argv[i]);
      l_set = 1;
    }
    else if (ARGEITHER("-j", "--control-scheme")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      read_control_scheme(rd, argv[i]);
      c_set = 1;
      s_set = 1;
    }
    else if (ARGEITHER("-c", "--controller")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      rd->control_scheme.controller_name = strdup(argv[i]);
      c_set = 1;
    }
    else if (ARGEITHER("-pc", "--controller-params")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      str2darr(argv[i], &rd->control_scheme.control_params, &rd->control_scheme.n_control_params);
    }
    else if (ARGEITHER("-s", "--setter")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      rd->control_scheme.setter_name = strdup(argv[i]);
      s_set = 1;
    }
    else if (ARGEITHER("-ps", "--setter-params")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      str2darr(argv[i], &rd->control_scheme.setter_params, &rd->control_scheme.n_setter_params);
    }
    else if (ARGEITHER("-t", "--tag")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      free(rd->tag);
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
    else if (ARGEITHER("-v", "--video")) {
      rd->log_video = 1;
    }
    else if (ARGEITHER("-p", "--photo")) {
      rd->log_photo = 1;
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
    else if (ARGEQ("--loud")) {
      set_loud();
    }
    else if (ARGEQ("--motor")) {
      rd->motor_name_set = 1;
      i++;
      CHECK_ARG_HAS_VALUE;
      rd->motor_name = strdup(argv[i]);
    }
    else if (ARGEQ("--material")) {
      i++;
      CHECK_ARG_HAS_VALUE;
      rd->material_name = strdup(argv[i]);
      mat_set = 1;
    }
    else {
      argerr("Run argument \"%s\" not understood (is it a misplaced common arg?).", argv[i]);
    }
  }

  char *missing_params = calloc(1001, sizeof(char));
  int add_comma_space = 0;

#define append_to_missing(S)\
  if (add_comma_space) {\
    strncat(missing_params, ", ", 1000-strlen(missing_params)-1);\
  } \
  else { \
    add_comma_space = 1; \
  }\
  strncat(missing_params, S, 1000-strlen(missing_params)-1);

  if (!c_set) {
    append_to_missing("controller");
  }

  if (!s_set) {
    append_to_missing("setter");
  }

  if (!l_set) {
    append_to_missing("length");
  }

  if (!d_set) {
    append_to_missing("depth");
  }

  if (!hwver_set) {
    append_to_missing("hardware version");
  }

  if (!mat_set) {
    append_to_missing("material");
  }
#undef append_to_missing

  if (!c_set || !s_set || !l_set || !d_set || !hwver_set || !mat_set)
    argerr("There are missing required parameters: %s." ,missing_params);

  free(missing_params);
}
