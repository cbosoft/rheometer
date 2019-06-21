#include <stdlib.h>

#include "rheo.h"

void
parse_args(int argc, const char **argv, thread_data *td) 
{
  td->length_s = 60;
  td->tag = "DELME";
  td->control_scheme = 0;
}
