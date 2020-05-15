#include <unistd.h>

#include <wiringPi.h>

#include "motor.h"
#include "../util/sleep.h"




void motor_setup()
{
  pinMode(PWM_PIN, PWM_OUTPUT);
  pwmWrite(PWM_PIN, 0);
}




void motor_warmup(struct run_data *rd, unsigned int target)
{
  // run high for 1.5s, then go to target for 3
  if (!rd->calm_start) { 
    pwmWrite(PWM_PIN, 800);
    sleep_ms(1.5e3);
  }
  pwmWrite(PWM_PIN, target);
#ifndef DEBUG
  sleep_ms(3e3);
#endif
}




void motor_shutdown() {
  pwmWrite(PWM_PIN, 0);
}