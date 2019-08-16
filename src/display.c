#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "display.h"
#include "error.h"
#include "adc.h"
#include "run.h"




unsigned int
get_column_width(void)
{
  struct winsize ws;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

  unsigned int width_total = (unsigned int)ws.ws_col;
  unsigned int min_cols = 7;
  unsigned int number_cols = 0;
  number_cols += 1; // time
  //number_cols += 8; // adc
  number_cols += 1; // adc_dt
  number_cols += 1; // speed
  number_cols += 1; // strainrate
  number_cols += 1; // stress (bytes)
  number_cols += 1; // stress (units)
  number_cols += 1; // control action
  number_cols += 1; // temperature
  unsigned int min_total = (number_cols * (min_cols+1));

  if (width_total < min_total) {
    warn("get_column_width", "screen too small");
    return min_cols;
  }

  unsigned int extra_each = (width_total - min_total) / (number_cols);

  return min_cols + extra_each;
}




char *
centre(char *s, unsigned int w)
{
  char *rv = calloc(w+3, sizeof(char));
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

  char *time = centre("t/s", colw);
  fprintf(stderr, "%s%s ", BOLD, time);
  free(time);
  
  // for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
  //   char *adcv = calloc(10, sizeof(char));
  //   sprintf(adcv, "A%u/b", channel);
  //   char *cadcv = centre(adcv, colw);
  //   fprintf(stderr, "%s ", cadcv);
  //   free(cadcv);
  //   free(adcv);
  // }
  
  char *adcdt = centre("dt/s", colw);
  fprintf(stderr, "%s ", adcdt);
  free(adcdt);

  char *speed = centre("s/RPS", colw);
  fprintf(stderr, "%s ", speed);
  free(speed);

  char *strainrate = centre("SR/hz", colw);
  fprintf(stderr, "%s ", strainrate);
  free(strainrate);

  char *stress_bytes = centre("LC/24b", colw);
  fprintf(stderr, "%s ", stress_bytes);
  free(stress_bytes);

  char *stress = centre("S/Pa", colw);
  fprintf(stderr, "%s ", stress);
  free(stress);

  char *ca = centre("ca/b", colw);
  fprintf(stderr, "%5s ", ca);
  free(ca);

  char *temp = centre("T/C", colw);
  fprintf(stderr, "%5s%s\r", temp, RESET);
  free(temp);
}



void
display_thread_data(struct run_data *rd)
{

  unsigned int colw = get_column_width();
  
  unsigned long secs = (unsigned int)(rd->time_s_f);

  char *time = calloc(5, sizeof(char));
  sprintf(time, "%lu", secs);
  char *ctime = centre(time, colw);
  
  //char **adcval = calloc(ADC_COUNT, sizeof(char *));
  //char **cadcval = calloc(ADC_COUNT, sizeof(char *));
  //for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
  //  adcval[channel] = calloc(20, sizeof(char));
  //  sprintf(adcval[channel], "%lu", rd->adc[channel]);
  //  cadcval[channel] = centre(adcval[channel], colw);
  //}

  char *adcdt = calloc(20, sizeof(char));
  sprintf(adcdt, "%f", rd->adc_dt);
  char *cadcdt = centre(adcdt, colw);

  char *speed = calloc(20, sizeof(char));
  sprintf(speed, "%f", rd->speed_ind);
  char *cspeed = centre(speed, colw);

  char *strainrate = calloc(20, sizeof(char));
  sprintf(strainrate, "%f", rd->strainrate_ind);
  char *cstrainrate = centre(strainrate, colw);

  char *stress_bytes = calloc(20, sizeof(char));
  sprintf(stress_bytes, "%lu", (*rd->loadcell_bytes));
  char *cstress_bytes = centre(stress_bytes, colw);

  char *stress = calloc(20, sizeof(char));
  sprintf(stress, "%f", (*rd->loadcell_units));
  char *cstress = centre(stress, colw);

  char *ca = calloc(20, sizeof(char));
  sprintf(ca, "%u", rd->last_ca);
  char *cca = centre(ca, colw);
  
  char *temp = calloc(20, sizeof(char));
  sprintf(temp, "%f", (*rd->temperature));
  char *ctemp = centre(temp, colw);

  fprintf(stderr, "%s ", ctime);
  //for (unsigned int channel = 0; channel < ADC_COUNT; channel ++) {
  //  fprintf(stderr, "%s ", cadcval[channel]);
  //}
  fprintf(stderr, "%s %s %s %s %s %s %s\n", cadcdt, cspeed, cstrainrate, cstress_bytes, cstress, cca, ctemp);

  free(temp);
  free(ca);
  free(speed);
  //for (unsigned int channel = 0; channel < ADC_COUNT; channel ++) {
  //  free(adcval[channel]);
  //}
  //free(adcval);
  free(time);

  free(ctemp);
  free(cca);
  free(cstress);
  free(cstrainrate);
  free(cspeed);
  //for (unsigned int channel = 0; channel < ADC_COUNT; channel ++) {
  //  free(cadcval[channel]);
  //}
  //free(cadcval);
  free(ctime);
}


