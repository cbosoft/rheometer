#include <stdlib.h>
#include <stdio.h>

#include "../schedule/schedule.h"
#include "../args/args.h"
#include "../util/error.h"
#include "main.h"

int main(int argc, const char **argv);

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

  int rv = 0;
  for (int i = 0; i < argset->margc; i++) {
    for (int j = 0; j < argset->vargv[i]->argc; j++) {
      fprintf(stderr, "%s ", argset->vargv[i]->argv[j]);
    }
    fprintf(stderr, "\n");
    // TODO: print command details

    if (main(argset->vargv[i]->argc, (const char **)argset->vargv[i]->argv)) {
      rv = 1;
      break;
    }
  }

  free_schedule_data(sd);
  argset_free(argset);
  return rv;
}
