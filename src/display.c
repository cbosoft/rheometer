#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "rheo.h"


int
get_column_widths(unsigned int *timew, unsigned int* restw)
{
  struct winsize ws;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

  unsigned int width_total = (unsigned int)ws.ws_col;
  unsigned int min_time = 20;
  unsigned int min_rest = 4;
  unsigned int number_rest = 11;

  // timew is at least 20 chars, the rest are at least 4 chars.
  // there is one time column, and 11 other columns, plus a 
  // space between each.
  // this gives a total width of 20 + 11*(4+1) = 75 characters.

  unsigned int min_total = min_time + (number_rest * (min_rest+1));

  if (width_total < min_total)
    return 1;

  unsigned int extra_each = (width_total - min_total) / (number_rest+1);

  (*timew) = min_time + extra_each;
  (*restw) = min_rest + extra_each;
  
  return 0;
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

  unsigned int timew = 10, restw = 4;
  if (get_column_widths(&timew, &restw)) {
    // warn or something?
  }

  char *time = centre("time", timew);
  fprintf(stderr, "%s%s ", BOLD, time);
  for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
    char *adcv = calloc(10, sizeof(char));
    sprintf(adcv, "adc%u", channel);
    char *cadcv = centre(adcv, restw);
    fprintf(stderr, "%s ", cadcv);
    free(cadcv);
    free(adcv);
  }
  char *speed = centre("speed", restw);
  fprintf(stderr, "%s ", speed);
  char *ca = centre("ca", restw);
  fprintf(stderr, "%5s ", ca);
  char *temp = centre("temp", restw);
  fprintf(stderr, "%5s%s\r", temp, RESET);

  free(time);
  free(speed);
  free(ca);
  free(temp);
}



void
display_thread_data(thread_data_t *td)
{

  unsigned int timew = 10, restw = 4;
  if (get_column_widths(&timew, &restw)) {
    // warn or something?
  }

  char *time = calloc(21, sizeof(char));
  sprintf(time, "%lu.%06lu", (*td->time_s), (*td->time_us));
  char *ctime = centre(time, timew);
  
  char **adcval = calloc(ADC_COUNT, sizeof(char *));
  char **cadcval = calloc(ADC_COUNT, sizeof(char *));
  for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
    adcval[channel] = calloc(8, sizeof(char));
    sprintf(adcval[channel], "%lu", td->adc[channel]);
    cadcval[channel] = centre(adcval[channel], restw);
  }

  char *speed = calloc(6, sizeof(char));
  sprintf(speed, "%f", td->speed_ind);
  char *cspeed = centre(speed, restw);
  char *ca = calloc(6, sizeof(char));
  sprintf(ca, "%u", td->last_ca);
  char *cca = centre(ca, restw);
  char *temp = calloc(6, sizeof(char));
  sprintf(temp, "%f\n", (*td->temperature));
  char *ctemp = centre(temp, restw);

  fprintf(stderr, "%s ", ctime);
  for (unsigned int channel = 0; channel < ADC_COUNT; channel ++) {
    fprintf(stderr, "%s ", cadcval[channel]);
  }
  fprintf(stderr, "%s %s %s\n", cspeed, cca, ctemp);

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
  free(cspeed);
  for (unsigned int channel = 0; channel < ADC_COUNT; channel ++) {
    free(cadcval[channel]);
  }
  free(cadcval);
  free(ctime);
}


