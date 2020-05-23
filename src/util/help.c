#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../args/args.h"
#include "../control/control.h"

#include "help.h"

void show_help()
{
  usage();
  args_help();
  control_help();

  if (isatty(fileno(stdout)))
    fprintf(stderr, "It may help to use \"less\" to navigate help easier: `./rheometer --help | less`\n");
}
