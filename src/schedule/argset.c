#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>

#include "schedule.h"

void add_head_tail_to_argset(char ****vargv, int **vargc, int margc,
    char **headv, int headc, char **tailv, int tailc)
{
  char ***new_vargv = malloc(margc*sizeof(char***));
  for (int i = 0; i < margc; i++) {
    int old_vargc = (*vargc)[i];
    (*vargc)[i] += headc + tailc;
    new_vargv[i] = malloc((*vargc)[i]*sizeof(char**));

    int vargv_i = 0;
    for (int j = 0; j < headc; j++, vargv_i++) {
      new_vargv[i][vargv_i] = strdup(headv[j]);
    }
    for (int j = 0; j < old_vargc; j++, vargv_i++) {
      new_vargv[i][vargv_i] = (*vargv)[i][j];
    }
    for (int j = 0; j < tailc; j++, vargv_i++) {
      new_vargv[i][vargv_i] = strdup(tailv[j]);
    }
  }

  char ***old_vargv = (*vargv);
  (*vargv) = new_vargv;
  for (int i = 0; i < margc; i++) {
    free(old_vargv[i]);
  }
  free(old_vargv);
}


void free_argset(char ***vargv, int *vargc, int margc)
{
  for (int i = 0; i < margc; i++) {
    for (int j = 0; j < vargc[i]; j++) {
      free(vargv[i][j]);
    }
    free(vargv[i]);
  }
  free(vargc);
  free(vargv);
}
