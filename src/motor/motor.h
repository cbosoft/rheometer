#pragma once

#include "../run/run.h"

#define PWM_PIN 18

void motor_setup();
void motor_warmup(struct run_data *rd);
void motor_set_DC(double DC);
void motor_shutdown();

// vim: ft=c
