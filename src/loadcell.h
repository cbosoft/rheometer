#pragma once




#include "run.h"



#define CLOCK_PIN	6
#define DATA_PIN	5

#define RESET_US 100.0
#define PULSE_US 1.0



void read_loadcell(struct run_data *rd);
void loadcell_setup();
void loadcell_reset();
unsigned long loadcell_read_bytes();




// vim: ft=c
