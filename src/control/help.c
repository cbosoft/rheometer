#include <stdlib.h>
#include <stdio.h>
#include <glob.h>

#include "../util/display.h"
#include "control.h"

void control_help(void)
{
  fprintf(stdout,"%s",
      "  "BOLD"Control Schemes"RESET"\n"
      "    Control schemes are defined in JSON files (stored in the data/ directory).\n"
      "    These simple files dictate what controller and setter to use. See the json \n"
      "    files in the ./data/* directory for examples.\n"
      "\n"
      "  "BOLD"Control Module"RESET"\n"
      "    Control modules contain functions to set the motor supply voltage in response\n"
      "    to sensor data. These modules are dynamically linked shared objects so you\n"
      "    can add extra functionality as you wish. Modules found:\n"
      "\n");

  glob_t res;
  glob("./controllers/*.so", 0, NULL, &res);

  for (size_t i = 0; i < res.gl_pathc; i++) {
    ControllerHandle *h = load_controller_path(res.gl_pathv[i]);
    fprintf(stdout, "%s\n", h->doc);
    free(h);
  }

  globfree(&res);


  fprintf(stdout, "%s",
      "  "BOLD"Setpoint (setter) Module"RESET"\n"
      "    Setpoint for the controller is set by the setter module. It takes in sensor \n"
      "    readings and time, then deciding the new setpoint. Modules found:\n"
      "\n");
  
  glob("./setters/*.so", 0, NULL, &res);

  for (size_t i = 0; i < res.gl_pathc; i++) {
    SetterHandle *h = load_setter_path(res.gl_pathv[i]);
    fprintf(stdout, "%s\n", h->doc);
    free(h);
  }

  globfree(&res);

}
