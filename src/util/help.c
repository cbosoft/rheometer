#include "../args/args.h"
#include "../control/control.h"

#include "help.h"

void show_help()
{
  usage();
  args_help();
  control_help();
}
