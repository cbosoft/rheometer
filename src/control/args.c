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
      "    These JSON files dictate what function should be used, and what parameters \n"
      "    are passed. Different functions and their associated parameters are discussed\n"
      "    below. Scheme names, and all parameter names, are expected to be written in\n"
      "    lowercase in the JSON file.\n");

  glob_t res;
  glob("./controllers/*.so", 0, NULL, &res);

  for (size_t i = 0; i < res.gl_pathc; i++) {
    ControllerHandle *h = load_controller_path(res.gl_pathv[i]);
    fprintf(stdout, "%s\n", h->doc);
    free(h);
  }

  globfree(&res);


  fprintf(stdout, "%s",
      "\n"
      "  "BOLD"Setpoint (setter) Schemes"RESET"\n"
      "    Setpoint for the controller is set by one of these functions. It takes in sensor \n"
      "    readings and time, then deciding the new setpoint. Normally this is a constant, \n"
      "    being varied by the user between different constants between runs. It is possible that\n"
      "    dynamic testing may be applied which would require the use of one of the other functions.\n"
      "    This is a property within the control scheme json file. See the ./dat folder for examples.\n");
  
  glob("./setters/*.so", 0, NULL, &res);

  for (size_t i = 0; i < res.gl_pathc; i++) {
    SetterHandle *h = load_setter_path(res.gl_pathv[i]);
    fprintf(stdout, "%s\n", h->doc);
    free(h);
  }

  globfree(&res);

}
