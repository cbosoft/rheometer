#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "../util/display.h"
#include "../util/error.h"
#include "../util/help.h"
#include "../control/control.h"
#include "../version.h"

#include "args.h"


unsigned int parse_length_string(const char *length_s_str)
{
  unsigned int len = strlen(length_s_str);

  // is just numbers?
  unsigned int justnumber = 1;
  unsigned int notnumbers = 0;
  for (unsigned int i = 0; i < len; i++) {
    unsigned int ic = ((unsigned int)length_s_str[i]);
    if ( ic < 48 || ic > 57) {
      justnumber = 0;
      notnumbers ++;
    }
  }

  if (notnumbers > 1) {
    argerr("length arg must be a number or suffixed by a single 's' or 'm' to explicitly specify 'seconds' or 'minutes' (%s)", length_s_str);
  }

  unsigned int toi = atoi(length_s_str);

  if (length_s_str[len-1] == 's' || justnumber) {
    return toi;
  }

  if (length_s_str[len-1] == 'm') {
    return toi * 60;
  }

  argerr("length arg syntax error");

  // this will never run, but it makes the linter happy.
  return 1;
}



char *parse_tag_string(const char *s)
{
  unsigned int l = strlen(s);
  char *rv = calloc(l+1, sizeof(char));
  for (unsigned int i = 0; i < l; i++) {
    if (s[i] == ' ' || s[i] == '_')
      rv[i] = '-';
    else
      rv[i] = s[i];
  }
  return rv;
}


#define argv (*argv_ptr)
struct common_args *parse_common(int *argc, const char ***argv_ptr) {

  struct common_args *ca = default_common_args();
  const int i = 0;
  (*argc)--;
  (*argv_ptr)++;

  while (1) {

    if (!*argc) {
      break;
    }

    if (ARGEQ("--quiet")) {
      set_quiet();
    }
    else if (ARGEQ("--silent")) {
      set_silent();
    }
    else if (ARGEITHER("-h", "--help")) {
      show_help();
      exit(0);
    }
    else if (ARGEQ("run")) {
      ca->rc = RC_RUN;
    }
    else if (ARGEQ("config")) {
      ca->rc = RC_CONFIG;
    }
    else if (ARGEQ("schedule")) {
      ca->rc = RC_SCHEDULE;
    }
    else {
      break;
    }

    (*argc)--;
    (*argv_ptr)++;
  }

  return ca;
}
#undef argv


struct common_args *default_common_args()
{
  struct common_args *ca = malloc(sizeof(struct common_args));
  ca->rc = RC_RUN;
  return ca;
}

void free_common_args(struct common_args *ca)
{
  free(ca);
}
