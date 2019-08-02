#pragma once




#include "run.h"




#define TAGDEFAULT "DELME"




void save_run_params_to_json(struct run_data *rd);
void *log_thread_func(void *vptr);
void add_log(struct run_data *rd, const char * fmt, ...);
void generate_log_prefix(struct run_data *rd);




// vim: ft=c
