#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pthread.h>

#include "display.h"
#include "error.h"
#include "adc.h"
#include "run.h"


#define CENTER_AND_DISPLAY(VAL, FMT)  \
  snprintf(formatted, colw, FMT, VAL);\
  centre(formatted, colw, &centered);\
  fprintf(stderr, "%s ", centered);

extern pthread_mutex_t lock_time, lock_adc, lock_control, lock_loadcell, lock_temperature, lock_speed, lock_adcdt;

unsigned int get_column_width(void)
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




void centre(char *s, unsigned int w, char *c[])
{
  char padchar = ' ';
  
  unsigned int l = strlen(s);
  if (l > w) {
    for (unsigned int i = 0; i < w; i++) {
      (*c)[i] = s[i];
    }
  }
  else {
    unsigned int diff = w - l;
    unsigned int p = ( (diff % 2) == 1 ) ? ((diff-1)/2) : (diff/2);
    unsigned int i = 0;

    for (; i < p; i++)
      (*c)[i] = padchar;

    for (; i < (p+l); i++)
      (*c)[i] = s[i-p];

    for (; i < w; i++)
      (*c)[i] = padchar;
  }
}



// TODO create a modular/pluggable/argument driven way to choose what to display
void display_titles(void)
{
  unsigned int colw = get_column_width();
  char 
    *formatted = calloc(colw+1, sizeof(char)), 
    *centered  = calloc(colw+1, sizeof(char));

  CENTER_AND_DISPLAY("t/s", "%s");
  
  // for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
  //   CENTER_AND_DISPLAY(channel, "A%u/b");
  // }
  
  //CENTER_AND_DISPLAY("dt/s", "%s");
  CENTER_AND_DISPLAY("s/RPS", "%s");
  CENTER_AND_DISPLAY("SR/Hz", "%s");
  CENTER_AND_DISPLAY("LC/24b", "%s");
  CENTER_AND_DISPLAY("S/Pa", "%s");
  CENTER_AND_DISPLAY("ca/b", "%s");
  CENTER_AND_DISPLAY("Tc/C", "%s");
  CENTER_AND_DISPLAY("Ta/C", "%s");

  fprintf(stderr, "\r");

  free(formatted);
  free(centered);
}



void display_thread_data(struct run_data *rd)
{

  unsigned int colw = get_column_width();
  
  unsigned long secs = (unsigned int)(rd->time_s_f);
  char 
    *formatted = calloc(colw+1, sizeof(char)), 
    *centered  = calloc(colw+1, sizeof(char));

  CENTER_AND_DISPLAY(secs, "%lu");

  // pthread_mutex_lock(&lock_adcdt);
  // CENTER_AND_DISPLAY(rd->adc_dt, "%f");
  // pthread_mutex_unlock(&lock_adcdt);

  // for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
  //   pthread_mutex_lock(&lock_adc);
  //   CENTER_AND_DISPLAY(rd->adc[channel], "%lu");
  //   pthread_mutex_unlock(&lock_adc);
  // }

  pthread_mutex_lock(&lock_speed);
  CENTER_AND_DISPLAY(rd->speed_ind, "%f");
  CENTER_AND_DISPLAY(rd->strainrate_ind, "%f");
  pthread_mutex_unlock(&lock_speed);

  pthread_mutex_lock(&lock_loadcell);
  CENTER_AND_DISPLAY( rd->loadcell_bytes, "%lu");
  CENTER_AND_DISPLAY( rd->loadcell_units, "%f");
  pthread_mutex_unlock(&lock_loadcell);

  pthread_mutex_lock(&lock_control);
  CENTER_AND_DISPLAY( rd->last_ca, "%u");
  pthread_mutex_unlock(&lock_control);

  pthread_mutex_lock(&lock_temperature);
  CENTER_AND_DISPLAY( rd->cylinder_temperature, "%f");
  CENTER_AND_DISPLAY( rd->ambient_temperature, "%f");
  pthread_mutex_unlock(&lock_temperature);

  fprintf(stderr, "\n");

  free(formatted);
  free(centered);
}


