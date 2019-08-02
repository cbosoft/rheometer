#pragma once




#include "run.h"




#define OPTENC_COUNT 3



void opt_setup(struct run_data *rd);
void opt_mark(struct run_data *rd, unsigned int i);
double get_speed(struct run_data *rd);




// vim: ft=c
