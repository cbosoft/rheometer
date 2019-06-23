#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "rheo.h"

void
usage ()
{
  // TODO
  fprintf(stderr, 
      "\n"
      "  \033[1mrheometer\033[0m control program (v"VERSION")\n"
      "\n"
      "  Usage:\n"
      "    rheometer [-t|--tag <tag>] -l|--length <length> -c|--control-scheme <control scheme>\n"
      "    rheometer -h|--help\n"
      "\n"
  );
}

unsigned int
s_match_either(const char *a, const char *b, const char *c)
{
  if ( (strcmp(a, b) == 0) || (strcmp(a, c) == 0))
    return 1;
  return 0;
}

unsigned int
parse_length_string(const char *length_s_str) {
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
    argerr("length arg must be a number or suffixed by a single 's' or 'm' to explicitly specify 'seconds' or 'minutes'");
  }

  unsigned int toi = atoi(length_s_str);

  if (length_s_str[len-1] == 's' || justnumber) {
    return toi;
  }

  if (length_s_str[len-1] == 'm') {
    return toi * 60;
  }

  argerr("length arg syntax error");

  // this will bever run, but it makes the linter happy.
  return -1; //unsigned, so this is actually INT_MAX?
}

void
check_argc(int i, int argc) 
{
  if (i >= argc) {
    argerr("Option needs a value!");
  }
}

void
parse_args(int argc, const char **argv, thread_data *td) 
{
  if (getuid() != 0)
    argerr("Hardware PWM needs root.");

  td->length_s = 60;
  td->tag = "DELME";
  td->control_scheme = 0;

  unsigned int cs_set = 0, l_set = 0;

  for (unsigned int i = 1; i < argc; i++) {
    if (s_match_either(argv[i], "-l", "--length")) {
      i++;
      check_argc(i, argc);
      td->length_s = parse_length_string(argv[i]);
      l_set = 1;
    }
    else if (s_match_either(argv[i], "-c", "--control-scheme")) {
      // TODO
      i++;
      check_argc(i, argc);
      cs_set = 1;
    }
    else if (s_match_either(argv[i], "-t", "--tag")) {
      i++;
      check_argc(i, argc);
      td->tag = argv[i];
    }
    else if (s_match_either(argv[i], "-h", "--help")) {
      usage();
      exit(0);
    }
    else {
      argerr("given unknown arg");
    }
  }

  if (!cs_set || !l_set)
    argerr("Need to specify both length and control scheme.");

  // finished setting args
  fprintf(stderr, 
      "  \033[1mrheometer\033[0m v%s\n"
      "  Running with options:\n"
      "    tag: \"%s\"\n"
      "    control scheme: %s\n"
      "    length: %us\n",
      VERSION, td->tag, "CONTROLSCHEME", td->length_s);
}
