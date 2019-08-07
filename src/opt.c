#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <wiringPi.h>

#include "opt.h"
#include "error.h"

void opt_mark(unsigned int i);

#define OPT_TRIP(PIN) opt_trip_ ## PIN
#define SETUP_OPT_PIN(PIN, I) \
  pinMode(PIN, INPUT); \
  if (wiringPiISR(PIN, INT_EDGE_BOTH, &(OPT_TRIP(PIN))) < 0) \
    ferr("opt_setup", "failed to set up interrupt for pin %d", PIN); \


void opt_trip_20(void)
{
  opt_mark(0);
}

void opt_trip_21(void)
{
  opt_mark(1);
}

const int opt_pins[OPTENC_COUNT] = {20, 21}; // TODO reduce to only the two that are actually used
volatile unsigned long long tripc[OPTENC_COUNT] = {0};
time_t time_last[OPTENC_COUNT] = {0};



void opt_setup()
{
  SETUP_OPT_PIN(20, 0);
  SETUP_OPT_PIN(21, 1);
}




void opt_mark(unsigned int i)
{
  tripc[i] ++;
  //fprintf(stderr, "!!");
}




double get_speed(struct run_data *rd)
{
  time_t now = time(NULL);
  double speed = 0.0;

  unsigned long long local_tripc[OPTENC_COUNT];
  for (int i = 0; i < OPTENC_COUNT; i++) { local_tripc[i] = tripc[i]; tripc[i] = 0; }
  for (int i = 0; i < OPTENC_COUNT; i++) {
    double dt = difftime(now, time_last[i]);
    double dtripc = (double)local_tripc[i];
    //fprintf(stderr, "TRIPC: %llu\n DT: %f DTRIP: %f\n", local_tripc[i], dt, dtripc);
    time_last[i] = now;
    speed += dtripc / dt;
  }
  speed /= (double)OPTENC_COUNT; // units of trips per second
  speed /= 12.0; // units of rev per second
  //speed *= 3.1415926 * 2.0; // units of rad/s
  return speed;
}
