
#include <wiringPi.h>

#include "../../util/sleep.h"
#include "hx711.h"

#define CLOCK_PIN	6
#define DATA_PIN	5

#define RESET_US 100.0
#define PULSE_US 10.0





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




unsigned long hx711_read()
{
  /*
   * This is not the right way to do this, I made a mistake. However, I have
   * calibrated with this mistake in tow and I do not want to re-do the
   * calibration. This begin wrong doesn't effect the efficacy of the result,
   * just makes the numbers weird and huge.
   * */

	long count = 0;

  while(digitalRead(DATA_PIN));

  blocking_sleep_us(1);

  // 24 pulses
  for(int i = 0; i < 24; i++) {
    count = count << 1;
    clock_pulse(PULSE_US);
    if (digitalRead(DATA_PIN)) count++;
  }

  // one more pulse
  clock_pulse(PULSE_US);

  // count is in 2s complement
  if (count & 0x800000) {
    count |= (long) ~0xffffff;
  }

  static long halfway = (1 << 31);
  return (unsigned long)(count + halfway);

}

void hx711_setup()
{
  pinMode(DATA_PIN, INPUT);
  pinMode(CLOCK_PIN, OUTPUT);

  digitalWrite(CLOCK_PIN, LOW);
}
