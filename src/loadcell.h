#pragma once




#include "run.h"



#define CLOCK_PIN	6
#define DATA_PIN	5

#define RESET_US 100.0
#define PULSE_US 10.0

#define LOADCELL_CAL_M -2.15940989e-04
#define LOADCELL_CAL_C 9.27446776e+05



void read_loadcell(struct run_data *rd);
void loadcell_setup();
void loadcell_reset();
unsigned long loadcell_read_bytes();
void *loadcell_thread_func(void *vptr);




// vim: ft=c
