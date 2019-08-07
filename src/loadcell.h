#pragma once




#include "run.h"



#define CLOCK_PIN	6
#define DATA_PIN	5



double read_loadcell(struct run_data *rd);
void loadcell_setup();
void loadcell_reset();
unsigned long loadcell_read_bytes();
double read_loadcell(struct run_data *rd);




// vim: ft=c
