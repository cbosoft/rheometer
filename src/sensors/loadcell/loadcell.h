#pragma once

#include "../../run/run.h"

void read_loadcell(struct run_data *rd);
void *loadcell_thread_func(void *vptr);

// vim: ft=c
