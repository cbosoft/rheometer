#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "../schedule/schedule.h"
#include "../args/args.h"
#include "../util/error.h"
#include "../delays.h"
#include "main.h"

int main(int argc, const char **argv);

unsigned int get_length(ArgList *al)
{
  for (int i = 0; i < al->argc; i++) {
    if ((strcmp(al->argv[i], "-l") == 0) || (strcmp(al->argv[i], "--length") == 0)) {
      return parse_length_string(al->argv[++i]) + (DELAY_THREADS_MS + DELAY_MOTOR_HIGH_MS + DELAY_MOTOR_AFTER_MS + DELAY_LOG_MS + DELAY_CONTROL_MS)/1000;
    }
  }

  return -1;
}

unsigned int *get_times_taken(ArgSet *as)
{
  unsigned int *n_sv = malloc(as->margc*sizeof(unsigned int));

  for (int i = 0; i < as->margc; i++) {
    ArgList *al = as->vargv[i];
    n_sv[i] = get_length(al);
  }

  return n_sv;
}

unsigned int total_mult_from(unsigned int *v, int n, double m, int from)
{
  unsigned int t = 0;
  for (int i = from; i < n; i++)
  {
    t += (unsigned int)(((double)v[i])*m);
  }
  return t;
}

char *format_length(unsigned int n_s)
{
  char *s = malloc(100*sizeof(char));
  if (n_s > 60) {

    snprintf(s, 99, "%u minutes and %u seconds", n_s/60, n_s%60);

  }
  else {

    snprintf(s, 99, "%u seconds", n_s);

  }

  return s;
}

int schedule_main(int argc, const char **argv)
{
  (void) argc;
  (void) argv;

  struct schedule_data *sd = get_default_schedule_data();
  parse_schedule_args(&argc, &argv, sd);

  ArgSet *argset = generate_schedule(sd);

  if (!argset->margc) {
    ferr("schedule_main", "no runs scheduled!");
  }

  const char *head_sv[3] = {"schedule", "--quiet", "run"};
  ArgList *head = arglist_from_strvec(head_sv, 3);
  ArgList *tail = arglist_from_strvec(argv, argc);
  argset_add_head_tail(&argset, head, tail);
  arglist_free(head);
  arglist_free(tail);

  unsigned int *times = get_times_taken(argset);
  double mod = 1.0;

  int rv = 0;
  for (int i = 0; i < argset->margc; i++) {

    unsigned int this_time_taken_guess = times[i]*mod;

    char *fmt_total = format_length(total_mult_from(times, argset->margc, mod, i)),
         *fmt_this = format_length(this_time_taken_guess);
    fprintf(stderr, "(%d/%d) %s to go, this next run will take approx %s\n", i+1, argset->margc, fmt_total, fmt_this);
    free(fmt_total);
    free(fmt_this);

    unsigned int start = time(NULL);
    if (main(argset->vargv[i]->argc, (const char **)argset->vargv[i]->argv)) {
      rv = 1;
      break;
    }
    unsigned this_time_taken_real = time(NULL) - start;
    double discrepency = ((double)this_time_taken_real)/((double)this_time_taken_guess);
    mod = pow(mod*discrepency, 0.5);



  }

  free_schedule_data(sd);
  argset_free(argset);
  return rv;
}
