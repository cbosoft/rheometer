#pragma once




#include "../run/run.h"


typedef enum {RC_RUN, RC_SCHEDULE, RC_CONFIG} RunCommand;


RunCommand get_run_command(int argc, const char **argv);
void parse_args(unsigned int argc, const char **argv, struct run_data *rd);
void usage();




// vim: ft=c
