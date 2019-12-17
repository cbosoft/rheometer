#include "wiringPi.h"
#include "wiringPiI2C.h"

int wiringPiSetupGpio(void)
{
  return 0;
}

int wiringPiISR(int pin, int type, void (*f)(void))
{
  (void) pin;
  (void) type;
  (void) f;
  return 0;
}

int pinMode(int pin, int mode)
{
  (void) pin;
  (void) mode;
  return 0;
}

int pwmWrite(int pin, int value)
{
  (void) pin;
  (void) value;
  return 0;
}

int digitalWrite(int pin, int value)
{
  (void) pin;
  (void) value;
  return 0;
}

int digitalRead(int pin)
{
  (void) pin;
  return 0;
}


int wiringPiI2CSetup(int devid)
{
  (void) devid;
  return 314;
}

int wiringPiI2CRead(int fd)
{
  (void) fd;
  return 314;
}

int wiringPiI2CReadReg8(int fd, int reg)
{
  (void) fd;
  (void) reg;
  return 314;
}

int wiringPiI2CReadReg16(int fd, int reg)
{
  (void) fd;
  (void) reg;
  return 314;
}

int wiringPiI2CWrite(int fd, int data)
{
  (void) fd;
  (void) data;
  return 314;
}

int wiringPiI2CWriteReg8(int fd, int reg, int data)
{
  (void) fd;
  (void) reg;
  (void) data;
  return 314;
}

int wiringPiI2CWriteReg16(int fd, int reg, int data)
{
  (void) fd;
  (void) reg;
  (void) data;
  return 314;
}
