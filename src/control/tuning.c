#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include "../util/display.h"
#include "control.h"


void analyse(int length, struct run_data *rd)
{
  double *error = calloc(length, sizeof(double));
  double total = 0.0;
  int plotting = 1;
  FILE *fp = fopen("control_analysis_data.csv", "w");

  if (fp == NULL) plotting = 0;

  fprintf(stderr, "Gathering data...\n");
  for (int i = 0; i < length; i++) {
    double input = rd_get_is_stress_controlled(rd) ? rd->stress_ind : rd->strainrate_ind;
    double err = rd_get_setpoint(rd) - input;
    error[i] = err;
    total += error[i];
    fprintf(stderr, "  %d/%d    \r", i+1, length);
    if (plotting) {
      fprintf(fp, "%f\n", error[i]);
    }
    sleep(1);
  }
  fprintf(stderr, "\n");
  fclose(fp);

  double average_error = total/((double)length);

  double sumsqdiff = 0.0;
  for (int i = 0; i < length; i++)
    sumsqdiff += pow(error[i] - average_error, 2.0);

  double stddev_error = pow( sumsqdiff/((double)(length - 1)) ,0.5);

  system("gnuplot ./scripts/plot_control_analysis.gplot");
  fprintf(stderr,
      BOLD"Analysis results"RESET": Eav: %f, std: %f\n",
      average_error,
      stddev_error);
}











void do_tuning(struct run_data *rd) {
  /*
   * User selects setpoint and an initial set of params, from a scheme.json.
   * Rheometer runs for a period of time, before allowing the user to select a
   * new set of tuning parameters.  */

  int l = 10, analyse_length = 0;
  fprintf(stderr, 
      "  "BOLD"Welcome to the TD tuner."RESET"\n"
      "\n"
      "    The controller will run for %ds, before dropping to an interactive shell\n"
      "    so you may alter tuning parameters. In the shell, type 'help' to get a list\n"
      "    of valid commands.\n",
      l);
  
  
  double value = 0.0;
  char input[100], cmd[100], variable[50];

  while ( (!rd->stopped) && (!rd->errored) ) {
    
    fprintf(stderr, "Waiting...\n");
    for (int i = 0; i < l; i++) {
      fprintf(stderr, "  %d/%d    \r", i+1, l);
      sleep(1);
    }
    fprintf(stderr, "\n");

    // TODO create new log holding the changes in tuning

    while (1) {
      // read user input
      fprintf(stderr, "%s", ": ");

      fgets(input, 100, stdin);
      int inlen = strlen(input);
      if (inlen > 1){
        input[inlen-1] = '\0';
      }

      if (strncmp(input, "set", 3) == 0) {
        int nmatch = sscanf(input, "%s %s %lf", cmd, variable, &value);

        if (nmatch < 3) {
          fprintf(stderr, "'set' needs two arguments.\n");
          continue;
        }

      }
      else if (strncmp(input, "analyse", 7) == 0) {
        int nmatch = sscanf(input, "%s %d", cmd, &analyse_length);

        if (nmatch < 2) {
          analyse_length = 30;
        }
      }
      else {
        snprintf(cmd, 100, "%s", input);
      }

      if (strcmp(cmd, "help") == 0) {
        fprintf(stderr, 
            "  "BOLD"Commands:"RESET"\n"
            "    show                 show the current params\n"
            "    set <var> <val>      set var to value, valid vars: kp, ki, kd\n"
            "    analyse [<len>]      analyse a <len=30> second section of data.\n"
            //"    autotune [<method>]  run an autotuning algorithm.\n" // TODO
            "    done                 finish tuning\n"
            );
      }
      else if (strcmp(cmd, "set") == 0) {

        int i = atoi(variable);

        if ((i > 0) && (i < rd->control_scheme.n_control_params)) {

          rd->control_scheme.control_params = realloc(rd->control_scheme.control_params, (i+1)*sizeof(double) );

          if ((rd->control_scheme.n_control_params - i) > 1) {

            for (int j = rd->control_scheme.n_control_params; j < i+1; j++) {
              rd->control_scheme.control_params[i] = 0;
            }

          }
          
          rd->control_scheme.n_control_params = i+1;

        }

        rd->control_scheme.control_params[i] = value;

        // TODO log tuning parameter change

      }
      else if (strcmp(cmd, "analyse") == 0) {
        
        analyse(analyse_length, rd);

      }
      else if (strcmp(cmd, "autotune") == 0) {

        // TODO

      }
      else if (strcmp(cmd, "show") == 0) {
        for (int i = 0; i < rd->control_scheme.n_control_params; i++) {
          fprintf(stderr, "  p[%d] = %f\n", i, rd->control_scheme.control_params[i]);
        }
      }
      else if (strcmp(cmd, "done") == 0) {
        fprintf(stderr, BOLD"Tuning finished!"RESET"\n");
        return;
      }
      else {
        fprintf(stderr, "Input not understood. Try 'help' for help.\n");
      }
    }
  }
}
