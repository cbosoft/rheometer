// This file written with help from https://github.com/ggurov/hx711
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <stdlib.h>

#include <wiringPi.h>

#include "loadcell.h"
#include "run.h"
#include "util.h"




enum HX711_GAIN {
  CHA_128, // default
  CHB_32,
  CHA_64
};

const int GAIN = CHA_64;
const double LOADCELL_M = 1.0;
const double LOADCELL_C = 0.0;



void clock_pulse(double us)
{
  /*
    Initially, I wanted everything to run with the lowest timings possible, 
    however nanosecond resolution is not easy. Instead, going for the middle
    ground.

    Minimum timings: 0.2 us
    Maximum timings: 50 us

    Middle ground: 20 us
   */
  digitalWrite(CLOCK_PIN, HIGH);
	sleep_us(us);
  digitalWrite(CLOCK_PIN, LOW);
	sleep_us(us);
}




void set_gain(int r)
{

	while(digitalRead(DATA_PIN));

	for (int i = 0; i < 24 + r; i++)
    clock_pulse(20);

}




void loadcell_setup()
{

  pinMode(DATA_PIN, INPUT);
  pinMode(CLOCK_PIN, OUTPUT);

  digitalWrite(CLOCK_PIN, 0);

  set_gain(CHA_64);

}




void loadcell_reset()
{
  clock_pulse(100.0);
}




unsigned long loadcell_read_bytes()
{
	unsigned long count = 0;

  set_gain(CHA_64);

  while(digitalRead(DATA_PIN));

  sleep_us(10);

  for(int i = 0; i < 24; i++) {
    count = count << 1;
    clock_pulse(20.0);
    if (digitalRead(DATA_PIN)) count++;
  }

  for (int i = 0; i < GAIN + 1; i++)
    clock_pulse(20.0);

  if (count & 0x800000) {
    count |= (long) ~0xffffff;
  }

  return count;

}




double read_loadcell(struct run_data *rd)
{
  unsigned long *loadcell, *ploadcell;
  double *loadcell_units, *ploadcell_units;

  loadcell = malloc(sizeof(unsigned long));
  loadcell_units = malloc(sizeof(double));

  (*loadcell) = loadcell_read_bytes();
  (*loadcell_units) = (((double)(*loadcell)) * LOADCELL_M) + LOADCELL_C;

  ploadcell = rd->loadcell_bytes;
  ploadcell_units = rd->loadcell_units;
  rd->loadcell_bytes = loadcell;
  rd->loadcell_units = loadcell_units;
  free(ploadcell);
  free(ploadcell_units);

  return (*rd->loadcell_units);
}
