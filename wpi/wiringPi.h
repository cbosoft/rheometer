#define INPUT 0
#define OUTPUT 0
#define PWM_OUTPUT 0
#define INT_EDGE_BOTH 0

#define DEBUG

int wiringPiSetupGpio(void);
int wiringPiISR(int pin, int type, void (*f)(void));
int pinMode(int pin, int mode);
int pwmWrite(int pin, int value);
