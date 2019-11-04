#include <stdlib.h>
#include <string.h>

#include <wiringPi.h>




#include "tar.h"
#include "error.h"




char *escape_parens(char *cmd)
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




void tidy_logs(struct run_data *rd) 
{
  unsigned int cmdlen = 0;
  cmdlen += 8; // "tar cjf "
  for (unsigned int i = 0; i < rd->log_count; i++)
    cmdlen += (rd->log_paths[i] == NULL) ? 0 : strlen(rd->log_paths[i]) + 1;
  cmdlen += 8; // ".tar.bz2"
  cmdlen += 1; // " "
  cmdlen += 1; // "\0"
  cmdlen += 100; // this is overkill, for some reason not calculating amount of memory correctly.

  char  *tar_cmd = calloc(cmdlen, sizeof(char));
  strcat(tar_cmd, "tar cjf ");
  strcat(tar_cmd, rd->log_pref);
  strcat(tar_cmd, ".tar.bz2");

  for (unsigned int i = 0; i < rd->log_count; i++) {
    if (rd->log_paths[i] == NULL) {
      warn("tidy_logs", "log path \"%s\" is NULL, skipping.", rd->log_names[i]);
      continue;
    }
    strcat(tar_cmd, " ");
    strcat(tar_cmd, rd->log_paths[i]);
  }

  char *tar_cmd_escaped = escape_parens(tar_cmd);
  int rv = system(tar_cmd_escaped);
  free(tar_cmd);
  free(tar_cmd_escaped);

  if (rv) {
    ferr("tidy_logs", "error when tidying logs");
  }

}
