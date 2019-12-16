// This file written with help from https://github.com/ggurov/hx711
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <wiringPi.h>

#include "loadcell.h"
#include "run.h"
#include "util.h"




enum HX711_GAIN {
  CHA_128, // default
  CHB_32,
  CHA_64
};

int GAIN = CHA_128; // default
const long halfway = (long)(1 << 31);
extern pthread_mutex_t lock_loadcell;



void clock_pulse(double us)
{
  /*
    Initially, I wanted everything to run with the lowest timings possible, 
    however nanosecond resolution is not easy. Minimum acheivable timing is 
    1us.

    Minimum timings: 0.2 us (1us acheivable)
    Maximum timings: 50 us

   */
  digitalWrite(CLOCK_PIN, HIGH);
	blocking_sleep_us(us);
  digitalWrite(CLOCK_PIN, LOW);
	blocking_sleep_us(us);
}




void set_gain(int gain)
{

	while(digitalRead(DATA_PIN));

	for (int i = 0; i < 24 + gain + 1; i++)
    clock_pulse(PULSE_US);

  GAIN = gain;

}




void loadcell_setup()
{

  pinMode(DATA_PIN, INPUT);
  pinMode(CLOCK_PIN, OUTPUT);

  digitalWrite(CLOCK_PIN, LOW);

}




void loadcell_reset()
{
  clock_pulse(RESET_US);
}




unsigned long loadcell_read_bytes()
{
	long count = 0;

  while(digitalRead(DATA_PIN));

  blocking_sleep_us(1);

  // 24 pulses
  for(int i = 0; i < 24; i++) {
    count = count << 1;
    clock_pulse(PULSE_US);
    if (digitalRead(DATA_PIN)) count++;
  }

  // at least one more pulse
  for (int i = 0; i < GAIN + 1; i++)
    clock_pulse(PULSE_US);

  // count is in 2s complement
  if (count & 0x800000) {
    count |= (long) ~0xffffff;
  }

  return (unsigned long)(count + halfway);

}




void read_loadcell(struct run_data *rd)
{
  unsigned long lc_bytes = loadcell_read_bytes();

  pthread_mutex_lock(&lock_loadcell);
  rd->loadcell_bytes = lc_bytes;
  rd->loadcell_units = (((double)rd->loadcell_bytes) * LOADCELL_CAL_M) + LOADCELL_CAL_C;
  pthread_mutex_unlock(&lock_loadcell);

}




void *loadcell_thread_func(void *vptr)
{
  struct run_data *rd = (struct run_data *) vptr;

  rd->lc_ready = 1;
  while ( (!rd->stopped) && (!rd->errored) )
    read_loadcell(rd);

  pthread_exit(0);
  return NULL;
}
