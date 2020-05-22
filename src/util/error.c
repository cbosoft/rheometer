#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "../args/args.h"
#include "display.h"
#include "error.h"


static int verbosity = 2;

void set_silent()
{
  verbosity = 0;
}

void set_quiet()
{
  verbosity = 1;
}


void set_loud()
{
  verbosity = 2;
}


int get_loud()
{
  return !get_quiet();
}

int get_quiet()
{
  return verbosity < 2;
}

int get_silent()
{
  return verbosity < 1;
}


void ferr(const char* source, const char* fmt, ...)
{
  size_t mesglen = 256;
  char *mesg = calloc(mesglen, sizeof(char));

  va_list ap;

  va_start(ap, fmt);
  vsnprintf(mesg, mesglen, fmt, ap);
  va_end(ap);

  fprintf(stderr, "  "BGRED"FATAL ERROR!"RESET" in "FGBLUE"%s"RESET": %s\n", source, mesg);
  if (errno)
    fprintf(stderr, "  --> (%d)  %s\n", errno, strerror(errno));
  exit(1);
}




void argerr(const char *fmt, ...)
{
  size_t mesglen = 256;
  char *mesg = calloc(mesglen, sizeof(char));

  va_list ap;

  va_start(ap, fmt);
  vsnprintf(mesg, mesglen, fmt, ap);
  va_end(ap);

  fprintf(stderr, "  \033[41mUSAGE ERROR!\033[0m %s\n", mesg);
  usage();
  exit(1);
}




void warn(const char *source, const char *fmt, ...)
{
  if (get_silent()) return;

  size_t mesglen = 256;
  char *mesg = calloc(mesglen, sizeof(char));

  va_list ap;

  va_start(ap, fmt);
  vsnprintf(mesg, mesglen, fmt, ap);
  va_end(ap);

  fprintf(stderr, "  "FGYELLOW"WARNING!"RESET" in "FGBLUE"%s"RESET": %s\n", source, mesg);
}




void info(const char *fmt, ...)
{
  if (get_quiet()) return;

  size_t mesglen = 256;
  char *mesg = calloc(mesglen, sizeof(char));

  va_list ap;

  va_start(ap, fmt);
  vsnprintf(mesg, mesglen, fmt, ap);
  va_end(ap);

  fprintf(stderr, "  "FGBLUE"%s"RESET"\n", mesg);
}
