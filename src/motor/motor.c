#include <unistd.h>

#include <wiringPi.h>

#include "../util/sleep.h"
#include "../config/config.h"
#include "../delays.h"

#include "motor.h"




void motor_setup()
{
  pinMode(PWM_PIN, PWM_OUTPUT);

  // broadcom invented their own "balanced" pwm
  // method, which is unpredictable. "mark:space" is
  // the traditional method.
  pwmSetMode(PWM_MODE_MS);
  
  // init to zero speed
  pwmWrite(PWM_PIN, 0);
}




void motor_warmup(struct run_data *rd)
{
  // run high, then go to target
  if (!rd->calm_start) { 
    pwmWrite(PWM_PIN, 1024);
    sleep_ms(DELAY_MOTOR_HIGH_MS);
  }
  pwmWrite(PWM_PIN, 512);
#ifndef DEBUG
  sleep_ms(DELAY_MOTOR_LOW_MS);
#endif
}




void motor_shutdown() {
  pwmWrite(PWM_PIN, 0);
}
