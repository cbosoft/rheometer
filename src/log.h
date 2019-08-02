#pragma once




#include "run.h"




#define TAGDEFAULT "DELME"




void save_run_params_to_json(struct run_data *rd);
void *log_thread_func(void *vptr);




// vim: ft=c
