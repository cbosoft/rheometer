#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glob.h>

#include "../util/display.h"
#include "control.h"

void fprint_indented_to_width(FILE *f, const char *s, int indent, int w)
{
  if (!s) return;

  //fprintf(stderr, "%s\n", s);

  int l = strlen(s);

  char buffer[1000] = {0};
  int bi = 0;

  int ci = indent;
  for (int j = 0; j < indent; j++)
    fprintf(f, " ");

  for (int i = 0; i < l; i++, ci++) {

    if ((s[i] == ' ') || (s[i] == '\n')) {
      if (ci > w) {
        fprintf(f, "\n");
        ci = indent;
        for (int j = 0; j < indent; j++)
          fprintf(f, " ");
      }
      fprintf(f, "%s%c", buffer, s[i]);
      if (s[i] == '\n') {
        ci = indent;
        for (int j = 0; j < indent; j++)
          fprintf(f, " ");
      }
      bi = 0;
      buffer[bi] = 0;
    }
    else {
      buffer[bi++] = s[i];
      buffer[bi] = 0;
    }

  }

  fprintf(f, "%s\n", buffer);
}

void control_help(void)
{
  fprintf(stdout,"%s",
      "  "BOLD""FGMAGENTA"Control Schemes"RESET"\n"
      "    Control schemes are defined in JSON files (stored in the data/ directory).\n"
      "    These simple files dictate what controller and setter to use. See the json \n"
      "    files in the ./data/* directory for examples.\n"
      "\n"
      "  "BOLD""FGMAGENTA"Control Modules"RESET"\n"
      "    Control modules contain functions to set the motor supply voltage in response\n"
      "    to sensor data. These modules are dynamically linked shared objects so you\n"
      "    can add extra functionality as you wish. Modules found:\n"
      "\n");

  glob_t res;
  glob("./controllers/*.so", 0, NULL, &res);

  for (size_t i = 0; i < res.gl_pathc; i++) {
    ControllerHandle *h = load_controller_path(res.gl_pathv[i]);
    fprintf(stdout, "    "BOLD"%s"RESET", '%s'\n", (h->name?h->name:"??"), (h->ident?h->ident:"??"));
    fprint_indented_to_width(stdout, h->doc, 6, 80);
    fprintf(stdout, "\n");
    fprintf(stdout, "      "BOLD"Params:"RESET"\n");
    for (int j = 0; j < h->n_params; j++) {
      int pi = 2*j;
      int di = pi+1;
      fprintf(stdout, "        - %s%s (%s) %s\n", FGBLUE, h->params[pi], h->params[di], RESET);
    }
    fprintf(stdout, "\n");
    free(h);
  }

  globfree(&res);


  fprintf(stdout, "%s",
      "  "BOLD""FGMAGENTA"Setpoint (setter) Modules"RESET"\n"
      "    Setpoint for the controller is set by the setter module. It takes in sensor \n"
      "    readings and time, then deciding the new setpoint. Modules found:\n"
      "\n");
  
  glob("./setters/*.so", 0, NULL, &res);

  for (size_t i = 0; i < res.gl_pathc; i++) {
    SetterHandle *h = load_setter_path(res.gl_pathv[i]);
    fprintf(stdout, "    "BOLD"%s"RESET", '%s'\n", (h->name?h->name:"??"), (h->ident?h->ident:"??"));
    fprint_indented_to_width(stdout, h->doc, 6, 80);
    fprintf(stdout, "\n");
    fprintf(stdout, "      "BOLD"Params:"RESET"\n");
    for (int j = 0; j < h->n_params; j++) {
      int pi = 2*j;
      int di = pi+1;
      fprintf(stdout, "        - %s%s (%s) %s\n", FGBLUE, h->params[pi], h->params[di], RESET);
    }
    fprintf(stdout, "\n");
    free(h);
  }

  globfree(&res);

}
