#include <stdlib.h>
#include "range.h"

double *linear_interp(double a, double b, int nsteps)
{
  double step = (b-a) / ((double)nsteps);

  double *rv = malloc(nsteps*sizeof(double));
  for (int i = 0; i < nsteps; i++) {
    rv[i] = i*step + a;
  }

  return rv;
}

double **linear_interp_vector(double *a, double *b, int n, int nsteps)
{
  double **rv = malloc(nsteps*sizeof(double*));
  for (int i = 0; i < nsteps; i++) {
    rv[i] = malloc(n*sizeof(double));
  }

  double **rvt = malloc(n*sizeof(double*));
  for (int i = 0; i < n; i++) {
    rvt[i] = linear_interp(a[i], b[i], nsteps);
  }

  for (int i = 0; i < nsteps; i++) {
    for (int j = 0; j < n; j++) {
      rv[i][j] = rvt[j][i];
    }
  }

  for (int i = 0; i < n; i++) {
    free(rvt[i]);
  }
  free(rvt);

  return rv;
}
