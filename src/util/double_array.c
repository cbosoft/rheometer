#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "double_array.h"


double *darr_copy(double *a, int n)
{
  double *rv = malloc(n*sizeof(double));
  for (int i = 0; i < n ; i++) {
    rv[i] = a[i];
  }
  return rv;
}


void str2darr(const char *s, double **rv, int *n)
{
  int l = strlen(s);
  char *buffer = calloc(l+3, sizeof(char));
  int bufferi = 0;
  for (int i = 0; i < l; i++) {

    if (s[i] == ',') {
      (*n)++;
      (*rv) = realloc(*rv, (*n)*sizeof(double));
      (*rv)[(*n)-1] = atof(buffer);
      bufferi=0;
    }
    else {
      buffer[bufferi++] = s[i];
      buffer[bufferi] = 0;
    }
    
  }

  (*n)++;
  (*rv) = realloc(*rv, (*n)*sizeof(double));
  (*rv)[(*n)-1] = atof(buffer);
  bufferi=0;
}


char *darr2str(double *darr, int n)
{
  const int char_per_float = 12;
  char *rv = malloc((char_per_float*n + 1)*sizeof(char));
  rv[0] = 0;

  for (int i = 0; i < n; i++) {

    char *fc = calloc(char_per_float+2, sizeof(char));
    snprintf(fc, char_per_float+1, "%g,", darr[i]);
    strncat(rv, fc, char_per_float+1);
    free(fc);
  }
  rv[strlen(rv)-1] = 0;
  rv = realloc(rv, (strlen(rv)+1)*sizeof(char));
  return rv;
}

