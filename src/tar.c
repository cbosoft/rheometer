#include <stdlib.h>
#include <string.h>

#include <wiringPi.h>

#include "rheo.h"

void
tidy_logs(thread_data *td) 
{
#ifndef DEBUG
  char  *tar_cmd = calloc(1000, sizeof(char));
  strcat(tar_cmd, "tar cjf ");
  strcat(tar_cmd, td->log_pref);
  strcat(tar_cmd, ".tar.bz2");

  for (int i = 0; i < td->log_count; i++) {
    strcat(tar_cmd, " ");
    strcat(tar_cmd, td->log_paths[i]);
  }
  int rv = system(tar_cmd);
  free(tar_cmd);

  if (rv == -1) {
    ferr("error when tidying logs");
  } // TODO: what about other rvs?
#endif
}
