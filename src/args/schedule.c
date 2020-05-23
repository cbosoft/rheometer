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

  (void)sd;

#define argc (*argc_ptr)
#define argv (*argv_ptr)
#define ADVANCE argc--; argv++;
  while (argc) {

    if (ARGEITHER("-c", "--controller")) {
      ADVANCE;
      name = argv[i];
      is_controller = 1;
      if (params) {
        free(params);
        params = NULL;
         nparams = 0;
      }
    }
    else if (ARGEITHER("-s", "--setter")) {
      ADVANCE;
      name = argv[i];
      is_controller = 0;
      if (params) {
        free(params);
        params = NULL;
        nparams = 0;
      }
    }
    else if (ARGEITHER("-p", "--params")) {
      ADVANCE;

      if (params) {
        free(params);
        params = NULL;
        nparams = 0;
      }

      str2darr(argv[i], &params, &nparams);
    }
    else if (ARGEITHER("-a", "--add")) {
      if (is_controller) {
        sd_add_controller(sd, name, params, nparams);
      }
      else {
        sd_add_setter(sd, name, params, nparams);
      }
    }
    else {
      break;
    }

    ADVANCE;
  }
#undef argv
#undef argc
#undef ADVANCE

}
