#pragma once

void str2darr(const char *s, double **darr, int *n);
char *darr2str(double *darr, int n);
double *darr_copy(double *a, int n);
void darr_append(double **darr, int *n, double a);
