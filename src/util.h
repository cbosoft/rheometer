#pragma once



#include <sys/time.h>




double time_elapsed(struct timeval end, struct timeval start);
void sleep_us(double delay_us);
void sleep_ms(double delay_us);
void blocking_sleep_us(double delay_us);
void blocking_sleep_ms(double delay_us);




// vim: ft=c
