#pragma once




#include "../run/run.h"




#define TAGDEFAULT "DELME"




void save_run_params_to_json(struct run_data *rd);
void *log_thread_func(void *vptr);
int add_log(struct run_data *rd, const char *name, const char * fmt, ...);
int remove_log(struct run_data *rd, unsigned int index);
void generate_log_prefix(struct run_data *rd);




// vim: ft=c
