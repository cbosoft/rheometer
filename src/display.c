#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "rheo.h"



static unsigned long start_secs = 0;




unsigned int
get_column_width(void)
{
  struct winsize ws;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

  unsigned int width_total = (unsigned int)ws.ws_col;
  unsigned int min_cols = 7;
  unsigned int number_cols = 14;
  unsigned int min_total = (number_cols * (min_cols+1));

  if (width_total < min_total) {
    warn("screen too small");
    return min_cols;
  }

  unsigned int extra_each = (width_total - min_total) / (number_cols);

  return min_cols + extra_each;
}




char *
centre(char *s, unsigned int w)
{
  char *rv = calloc(w+1, sizeof(char));
  char padchar = ' ';
  
  unsigned int l = strlen(s);
  if (l > w) {
    for (unsigned int i = 0; i < w; i++) {
      rv[i] = s[i];
    }
  }
  else {
    unsigned int diff = w - l;
    unsigned int p = ( (diff % 2) == 1 ) ? ((diff-1)/2) : (diff/2);
    unsigned int i = 0;

    for (; i < p; i++)
      rv[i] = padchar;

    for (; i < (p+l); i++)
      rv[i] = s[i-p];

    for (; i < w; i++)
      rv[i] = padchar;
  }
  return rv;
}




void
display_titles(void)
{
  unsigned int colw = get_column_width();

  char *time = centre("t (s)", colw);
  fprintf(stderr, "%s%s ", BOLD, time);
  for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
    char *adcv = calloc(10, sizeof(char));
    sprintf(adcv, "A%u (b)", channel);
    char *cadcv = centre(adcv, colw);
    fprintf(stderr, "%s ", cadcv);
    free(cadcv);
    free(adcv);
  }
  char *speed = centre("s (hz)", colw);
  fprintf(stderr, "%s ", speed);
  char *strainrate = centre("GD (1/s)", colw);
  fprintf(stderr, "%s ", strainrate);
  char *stress = centre("S (Pa)", colw);
  fprintf(stderr, "%s ", stress);
  char *ca = centre("ca (b)", colw);
  fprintf(stderr, "%5s ", ca);
  char *temp = centre("T (C)", colw);
  fprintf(stderr, "%5s%s\r", temp, RESET);

  free(time);
  free(speed);
  free(strainrate);
  free(stress);
  free(ca);
  free(temp);
}



void
display_thread_data(thread_data_t *td)
{

  unsigned int colw = get_column_width();
  
  unsigned long secs = (*td->time_s);
  if (start_secs == 0)
    start_secs = secs;
  secs -= start_secs;

  char *time = calloc(5, sizeof(char));
  sprintf(time, "%lu", secs);
  char *ctime = centre(time, colw);
  
  char **adcval = calloc(ADC_COUNT, sizeof(char *));
  char **cadcval = calloc(ADC_COUNT, sizeof(char *));
  for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
    adcval[channel] = calloc(20, sizeof(char));
    sprintf(adcval[channel], "%lu", td->adc[channel]);
    cadcval[channel] = centre(adcval[channel], colw);
  }

  char *speed = calloc(20, sizeof(char));
  sprintf(speed, "%f", td->speed_ind);
  char *cspeed = centre(speed, colw);

  char *strainrate = calloc(20, sizeof(char));
  sprintf(strainrate, "%f", td->strainrate_ind);
  char *cstrainrate = centre(strainrate, colw);

  char *stress = calloc(20, sizeof(char));
  sprintf(stress, "%f", td->stress_ind);
  char *cstress = centre(stress, colw);

  char *ca = calloc(20, sizeof(char));
  sprintf(ca, "%u", td->last_ca);
  char *cca = centre(ca, colw);
  
  char *temp = calloc(20, sizeof(char));
  sprintf(temp, "%f", (*td->temperature));
  char *ctemp = centre(temp, colw);

  fprintf(stderr, "%s ", ctime);
  for (unsigned int channel = 0; channel < ADC_COUNT; channel ++) {
    fprintf(stderr, "%s ", cadcval[channel]);
  }
  fprintf(stderr, "%s %s %s %s %s\n", cspeed, cstrainrate, cstress, cca, ctemp);

  free(temp);
  free(ca);
  free(speed);
  for (unsigned int channel = 0; channel < ADC_COUNT; channel ++) {
    free(adcval[channel]);
  }
  free(adcval);
  free(time);

  free(ctemp);
  free(cca);
  free(cstress);
  free(cstrainrate);
  free(cspeed);
  for (unsigned int channel = 0; channel < ADC_COUNT; channel ++) {
    free(cadcval[channel]);
  }
  free(cadcval);
  free(ctime);
}


