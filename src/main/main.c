#include <stdlib.h>
#include <stdio.h>

#include "../args/args.h"

#include "main.h"

int main(int argc, const char **argv)
{

  struct common_args *ca = parse_common(&argc, &argv);

  switch (ca->rc) {

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
