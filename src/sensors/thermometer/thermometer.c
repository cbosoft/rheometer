#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glob.h>
#include <pthread.h>

#include <wiringPiI2C.h>

#include "../../util/error.h"
#include "../../run/run.h"
#include "thermometer.h"



// before this is used, you must enable loading of the required modules by the
// kernel at boot-time.
//
// Append 
//
// w1-gpio
// w1-therm
//
// to /etc/modules.



static char **devices = NULL;
static int device_count = 0;
static int cyl_thermo_fd = -1;


void thermometer_setup()
{

  glob_t g;
  if (glob("/sys/bus/w1/devices/28-**/w1_slave", GLOB_NOSORT, NULL, &g)) {
    return;
  }

  if (!g.gl_pathc)
    return;
  
  devices = malloc(g.gl_pathc * sizeof(char*));
  for (int i = 0; i < (int)g.gl_pathc; i++) {
    devices[i] = calloc(strlen(g.gl_pathv[i]) + 1, sizeof(char));
    strcpy(devices[i], g.gl_pathv[i]);
  }
  device_count = g.gl_pathc;

  cyl_thermo_fd = wiringPiI2CSetup(0x60);
  if (cyl_thermo_fd < 0)
    warn("thermometer_setup", "Failed to get fd for thermocouple (cylinder).");
  else {
    //wiringPiI2CWriteReg8(cyl_thermo_fd, 0x06, 0b11000001);
    wiringPiI2CWriteReg8(cyl_thermo_fd, 0x06, 0b11010000);
  }

}




double read_device(char *device_path)
{
  //
  // temperature using the dsb1820 1wire sensor
  //
  // read the device file to obtain the output.
  // typical output is:
  //   ca 01 4b 46 7f ff 06 10 65 : crc=65 YES
  //   ca 01 4b 46 7f ff 06 10 65 t=28625
  // 
  // i.e.
  //   <HEX address> : <crc res>
  //   <HEX address> t=<temperature> 

  FILE *fp = fopen(device_path, "r");
  if (fp == NULL) {
    warn("read_device", "failed to read from thermometer (was it disconnected?)");
    return 1000.0;
  }


  int i = 0;
  int ch = 0;

  // skip past the address, crc, and the second address, to where we have 't'
  while ((ch = fgetc(fp)) != (int)'t' && ++i < 1000);

  // skip the equals sign
  fgetc(fp);

  // read in the five or so digits after this
  char temp[100] = {0};
  i = 0;
  while ((ch = fgetc(fp)) != EOF) {
    temp[i] = (char)ch;
    i++;
  }
  
  fclose(fp);

  return atof(temp)*0.001;
}




double read_ambient_temperature()
{
  if (!device_count)
    return -314.0;

  double total = 0.0;

  for (int i = 0; i < device_count; i++) {

    double t = read_device(devices[i]);

    if (t < 1000.0)
      total += t;

  }

  double average = total / ((double)device_count);

  return average;

}



double read_cylinder_temperature()
{
  // https://www.mikroe.com/thermo-k-click
  // http://ww1.microchip.com/downloads/en/DeviceDoc/MCP9600-Data-Sheet-DS20005426D.pdf
  // https://github.com/adafruit/Adafruit_MCP9600/blob/master/Adafruit_MCP9600.cpp
  // http://wiringpi.com/reference/i2c-library/
  
  wiringPiI2CWriteReg8(cyl_thermo_fd, 0x06, 0b11000000);
  int rx = wiringPiI2CReadReg16(cyl_thermo_fd, 0x00);

  int bytes[2] = {
    rx & 255,
    (rx >> 8) & 255
  };

  return ((double)bytes[0])*16.0 + ((double)bytes[1])*0.0625;
}





void *thermometer_thread_func(void *vptr)
{
  struct run_data *rd = (struct run_data *)vptr;
  double temp_ambient, temp_cylinder;

  thermometer_setup();

  rd_set_ambient_temperature(rd, 0.0);
  rd_set_cylinder_temperature(rd, 0.0);

  if (!device_count) {
    rd->tmp_ready = 1;
    return NULL;
  }
  else {
    // initialise
    rd->ambient_temperature = read_ambient_temperature();
  }

  rd->tmp_ready = 1;
  while ( (!rd->stopped) && (!rd->errored) ) {

    temp_ambient = read_ambient_temperature();
    temp_cylinder = read_cylinder_temperature();

    rd_set_ambient_temperature(rd, temp_ambient);
    rd_set_cylinder_temperature(rd, temp_cylinder);

    sleep(1);

  }

  return NULL;
}
