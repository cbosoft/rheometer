#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>

#include "thermometer.h"
#include "error.h"




char **devices = NULL;
int device_count = 0;




void thermometer_setup()
{
  system("modprobe w1-gpio");
  system("modprobe w1-therm");
  
  glob_t g;
  glob("/sys/bus/w1/devices/28-*", GLOB_NOSORT, NULL, &g);
  
  devices = malloc(g.gl_pathc * sizeof(char*));
  for (int i = 0; i < (int)g.gl_pathc; i++) {
    devices[i] = calloc(strlen(g.gl_pathv[i]) + 1, sizeof(char));
    strcpy(devices[i], g.gl_pathv[i]);
  }
  device_count = g.gl_pathc;

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
  char ch = 0;

  // skip past the address, crc, and the second address, to where we have 't'
  while ((ch = fgetc(fp)) != 't' && ++i < 1000);

  // skip the equals sign
  fgetc(fp);

  // read in the five or so digits after this
  char temp[100] = {0};
  i = 0;
  while ((ch = fgetc(fp)) != EOF) {
    temp[i] = ch;
    i++;
  }
  
  // unless its incredibly cold (unlikely), the temperature should be five digits
  if (i != 5)
    warn("read_device", "temperature may not be correct: %s", temp);

  fclose(fp);

  return atof(temp)*0.001;
}




double read_thermometer()
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
