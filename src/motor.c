#include <unistd.h>

#include <wiringPi.h>

#include "rheo.h"

#define PWM_PIN 18

void
motor_setup()
{
  pinMode(PWM_PIN, PWM_OUTPUT);
  pwmWrite(PWM_PIN, 0);
}

void
motor_warmup(unsigned int target)
{
  // run high for 1.5s, then go to target for 3
  pwmWrite(PWM_PIN, 800);
  nsleep(1500*1000*1000);
  pwmWrite(PWM_PIN, target);
  sleep(3);
}

void
motor_shutdown() {
  pwmWrite(PWM_PIN, 0);
}