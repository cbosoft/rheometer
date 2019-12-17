#pragma once

#ifndef DEBUG
#define DEBUG
#endif

int wiringPiI2CSetup(int devid);

int wiringPiI2CRead(int fd);
int wiringPiI2CReadReg8(int fd, int reg);
int wiringPiI2CReadReg16(int fd, int reg);

int wiringPiI2CWrite(int fd, int val);
int wiringPiI2CWriteReg8(int fd, int reg, int data);
int wiringPiI2CWriteReg16(int fd, int reg, int data);
