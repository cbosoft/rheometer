#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../util/error.h"
#include "../util/double_array.h"
#include "args.h"

void parse_schedule_args(int *argc_ptr, const char *** argv_ptr, struct schedule_data *sd)
{
  const char *name = NULL;
  double *params = NULL;
  int nparams = 0;
  int is_controller = -1;
  const int i = 0;
  int is_set = 0, is_params_set = 0;

#define argc (*argc_ptr)
#define argv (*argv_ptr)
#define ADVANCE argc--; argv++;
#define CHECK_AND_ADD(S)\
  if (S) { \
    if (is_controller) {\
      sd_add_controller(sd, name, params, nparams);\
    }\
    else {\
      sd_add_setter(sd, name, params, nparams);\
    }\
    is_set = 0;\
    is_params_set = 0;\
  }

  while (argc) {

    if (ARGEITHER("-c", "--controller")) {
      CHECK_AND_ADD(is_set||is_params_set);
      ADVANCE;
      name = argv[i];
      is_controller = 1;
      if (params) {
        free(params);
        params = NULL;
         nparams = 0;
      }
      is_set = 1;
    }
    else if (ARGEITHER("-s", "--setter")) {
      CHECK_AND_ADD(is_set||is_params_set);
      ADVANCE;
      name = argv[i];
      is_controller = 0;
      if (params) {
        free(params);
        params = NULL;
        nparams = 0;
      }
      is_set = 1;
    }
    else if (ARGEITHER("-p", "--params")) {
      ADVANCE;
      CHECK_AND_ADD(is_params_set);

      if (params) {
        free(params);
        params = NULL;
        nparams = 0;
      }

      str2darr(argv[i], &params, &nparams);
      is_params_set = 1;
    }
    else if (ARGEITHER("-f", "--schedule-file")) {
      ADVANCE;

      sd_add_from_file(sd, argv[i]);
    }
    else if (ARGEQ("--interp-number")) {
      ADVANCE;
      sd->n_interpolation_points = atoi(argv[i]);
    }
    else if (ARGEQ("--interp-linear")) {
      ADVANCE;
      sd->interpolation_type = IT_Linear;
    }
    else if (ARGEQ("--interp-log")) {
      ADVANCE;
      sd->interpolation_type = IT_Log;
    }
    else {
      break;
    }

    ADVANCE;
  }

  CHECK_AND_ADD(is_set||is_params_set);

#undef argv
#undef argc
#undef ADVANCE
#undef CHECK_AND_ADD

}
