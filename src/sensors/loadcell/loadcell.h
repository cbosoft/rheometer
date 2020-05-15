#pragma once

#include "../../run/run.h"

void read_loadcell(struct run_data *rd);
double loadcell_cal(struct run_data *rd, unsigned long bytes);
void *loadcell_thread_func(void *vptr);

// vim: ft=c
