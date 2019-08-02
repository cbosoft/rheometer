#include <unistd.h>

#include <wiringPi.h>

#include "motor.h"
#include "util.h"




void motor_setup()
{
  pinMode(PWM_PIN, PWM_OUTPUT);
  pwmWrite(PWM_PIN, 0);
}




void motor_warmup(unsigned int target)
{
  // run high for 1.5s, then go to target for 3
  pwmWrite(PWM_PIN, 800);
  rh_msleep(1500);
  pwmWrite(PWM_PIN, target);
#ifndef DEBUG
  sleep(3);
#endif
}




void motor_shutdown() {
  pwmWrite(PWM_PIN, 0);
}
