#pragma once




#include "../run/run.h"
#include "../config/config.h"
#include "../schedule/schedule.h"


typedef enum {RC_RUN, RC_SCHEDULE, RC_CONFIG} RunCommand;

struct common_args {
  RunCommand rc;
};


struct common_args *default_common_args();
struct common_args *parse_common(int *argc, const char ***argv);
void free_common_args(struct common_args *ca);
void parse_run_args(int argc, const char **argv, struct run_data *rd);
void parse_config_args(int argc, const char **argv, struct config_data *rd);
void parse_schedule_args(int argc, const char **argv, struct schedule_data *rd);
unsigned int parse_length_string(const char *length_s_str);
char *parse_tag_string(const char *s);
void usage();
void args_help();


#define ARGEQ(B) (strcmp(argv[i], B) == 0)
#define ARGEITHER(B,C) (ARGEQ(B) || ARGEQ(C))
#define CHECK_ARG_HAS_VALUE \
{\
  if ((i >= argc) || (argv[i][0] == '-')) {\
    argerr("Option \"%s\" needs a value!", argv[i-1]);\
  }\
}


// vim: ft=c
