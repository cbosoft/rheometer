#pragma once




#define PWM_PIN 18




void motor_setup();
void motor_warmup(unsigned int target);
void motor_set_DC(double DC);
void motor_shutdown();




// vim: ft=c
