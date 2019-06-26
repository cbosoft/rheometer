#include <stdlib.h>
#include <string.h>

#include <wiringPi.h>

#include "rheo.h"

char *
escape_parens(char *cmd)
{
  unsigned int cmdlen = strlen(cmd);
  char *rv = calloc(cmdlen*2, sizeof(char));
  
  unsigned int j = 0;
  for (unsigned int i = 0; i < cmdlen; i++, j++) {
    if (cmd[i] == '(' || cmd[i] == ')') {
      rv[j] = '\\';
      j++;
    }
    rv[j] = cmd[i];
  }
  return rv;
}

void
tidy_logs(thread_data_t *td) 
{
  unsigned int cmdlen = 0;
  cmdlen += 8; // "tar cjf "
  for (unsigned int i = 0; i < td->log_count; i++)
    cmdlen += (td->log_paths[i] == NULL) ? 0 : strlen(td->log_paths[i]) + 1;
  cmdlen += 8; // ".tar.bz2"
  cmdlen += 1; // " "
  cmdlen += 1; // "\0"
  cmdlen += 100; // this is overkill, for some reason not calculating amount of memory correctly.

  char  *tar_cmd = calloc(cmdlen, sizeof(char));
  strcat(tar_cmd, "tar cjf ");
  strcat(tar_cmd, td->log_pref);
  strcat(tar_cmd, ".tar.bz2");

  for (unsigned int i = 0; i < td->log_count; i++) {
    if (td->log_paths[i] == NULL) {
      char err_mesg[35] = {0};
      sprintf(err_mesg, "log path %u is NULL, skipping.", i);
      warn(err_mesg);
      continue;
    }
    strcat(tar_cmd, " ");
    strcat(tar_cmd, td->log_paths[i]);
  }

  char *tar_cmd_escaped = escape_parens(tar_cmd);
  int rv = system(tar_cmd_escaped);
  free(tar_cmd);
  free(tar_cmd_escaped);

  if (rv == -1) {
    ferr("error when tidying logs");
  } // TODO: what about other rvs?
}
