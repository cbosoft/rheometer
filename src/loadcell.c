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



void clock_pulse(int ns)
{
  digitalWrite(CLOCK_PIN, HIGH);
	rh_nsleep(ns);
  digitalWrite(CLOCK_PIN, LOW);
	rh_nsleep(ns);
}




void set_gain(int r)
{

	while(digitalRead(DATA_PIN));

	for (int i = 0; i < 24 + r; i++)
    clock_pulse(200);

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
	digitalWrite(CLOCK_PIN, HIGH);
	rh_usleep(60);
	digitalWrite(CLOCK_PIN, LOW);
	rh_usleep(60);
}




unsigned long loadcell_read_bytes()
{
	unsigned long count = 0;

  set_gain(CHA_64);

  while(digitalRead(DATA_PIN));

  rh_nsleep(100);

  for(int i = 0; i < 24; i++) {
    count = count << 1;
    clock_pulse(200);
    if (digitalRead(DATA_PIN)) count++;
  }

  for (int i = 0; i < GAIN + 1; i++)
    clock_pulse(200);

  if (count & 0x800000) {
    count |= (long) ~0xffffff;
  }

  return count;

}




double read_loadcell(struct run_data *rd)
{
  rd->loadcell_bytes = loadcell_read_bytes();
  rd->loadcell_units = (((double)rd->loadcell_bytes) * LOADCELL_M) + LOADCELL_C;
  return rd->loadcell_units;
}
