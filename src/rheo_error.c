#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


#include "rheo.h"

void
rheo_ferr (const char* mesg)
{
  fprintf(stderr, "\033[31mFATAL ERROR!\033[0m : %s\n  (%d)  %s\n", mesg, errno, strerror(errno));
  exit(1);
}

void
rheo_warn (const char* mesg)
{
  fprintf(stderr, "\033[33mWARNING!\033[0m : %s\n", mesg);
  exit(1);
}
