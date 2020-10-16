#include <stdlib.h>
#include <stdio.h>

#include <wiringPi.h>


#define PWM_PIN 18


int main()
{

  if (wiringPiSetupGpio() == -1) {
    return 1;
  }

  pinMode(PWM_PIN, PWM_OUTPUT);
  pwmSetMode(PWM_MODE_MS);

  int value = 512;
  fprintf(stderr, "PWM DC = %d\n", value);
  while (1) {
    pwmWrite(PWM_PIN, value);
    fprintf(stderr, "PWM DC = ");
    if (scanf("%d", &value) != 1) {
      fprintf(stderr, "Error encountered. Stopping motor.\n");
      pwmWrite(PWM_PIN, 0);
      break;
    }
  }
}
