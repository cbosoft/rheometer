#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <wiringPi.h>

#include "opt.h"
#include "error.h"


#define OPT_TRIP(PIN) opt_trip_ ## PIN
#define SETUP_OPT_PIN(PIN, I) \
  pinMode(PIN, INPUT); \
  void OPT_TRIP(PIN)(void) { opt_mark(rd, I); } \
  if (wiringPiISR(PIN, INT_EDGE_BOTH, &(OPT_TRIP(PIN))) < 0) { \
    ferr("opt_setup", "failed to set up interrupt for pin %d", PIN); \
  }




static const int opt_pins[OPTENC_COUNT] = {16, 20, 21}; // TODO reduce to only the two that are actually used
volatile unsigned long long tripc[OPTENC_COUNT] = {0};
static time_t time_last[OPTENC_COUNT] = {0};



void opt_setup(struct run_data *rd)
{
  SETUP_OPT_PIN(16, 0);
  SETUP_OPT_PIN(20, 1);
  SETUP_OPT_PIN(21, 2);
}




void opt_mark(struct run_data *rd, unsigned int i)
{
  tripc[i] ++;
}




double get_speed(struct run_data *rd)
{
  time_t now = time(NULL);
  double speed = 0.0;
  for (int i = 0; i < OPTENC_COUNT; i++) {
    double dt = difftime(now, time_last[i]);
    double dtripc = tripc[i];
    tripc[i] = 0;
    speed += dtripc / dt;
  }
  speed /= (double)OPTENC_COUNT; // units of trips per second
  return speed;
}
