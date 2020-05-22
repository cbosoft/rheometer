#include <stdlib.h>
#include <stdio.h>

#include "../args/args.h"

#include "main.h"

int main(int argc, const char **argv)
{

  argc--; argv++;
  RunCommand rc = get_run_command(&argc, &argv);

  switch (rc) {

    case RC_RUN:
      run_main(argc, argv);
      break;

    case RC_SCHEDULE:
      schedule_main(argc, argv);
      break;

    case RC_CONFIG:
      config_main(argc, argv);
      break;
  }

  return 0;
}
