#include <stdlib.h>
#include <stdio.h>

#include "../args/args.h"

#include "main.h"

int main(int argc, const char **argv)
{

  struct common_args *ca = parse_common(&argc, &argv);

  switch (ca->rc) {

    case RC_RUN:
      return run_main(argc, argv);

    case RC_SCHEDULE:
      return schedule_main(argc, argv);

    case RC_CONFIG:
      return config_main(argc, argv);
  }

  return -1;
}
