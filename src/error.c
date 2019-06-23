#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "rheo.h"

void
ferr (const char* mesg)
{
  fprintf(stderr, "\033[31mFATAL ERROR!\033[0m %s\n", mesg);
  if (errno)
    fprintf(stderr, "  (%d)  %s\n", errno, strerror(errno));
  exit(1);
}

void
argerr(const char* mesg)
{
  fprintf(stderr, "\033[31mUSAGE ERROR!\033[0m %s\n", mesg);
  usage();
  exit(1);
}

void
warn (const char* mesg)
{
  fprintf(stderr, "\033[33mWARNING!\033[0m %s\n", mesg);
}

void
info (const char *mesg)
{
  fprintf(stderr, "\033[34m%s\033[0m\n", mesg);
}
