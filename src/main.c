#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>

#include <wiringPi.h>

#include "rheo.h"


static unsigned int cancelled = 0;

void 
inthandle(int signo)
{
  if (signo == SIGINT) {
    fprintf(stderr, "\r");
    cancelled = 1;
  }
}


char *centre(char *s, unsigned int w)
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
  char *time = centre("time", 20);
  fprintf(stderr, "%s%s ", BOLD, time);
  for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
    char *adcv = calloc(10, sizeof(char));
    sprintf(adcv, "adc%u", channel);
    char *cadcv = centre(adcv, 5);
    fprintf(stderr, "%s ", cadcv);
    free(cadcv);
    free(adcv);
  }
  char *speed = centre("speed", 5);
  fprintf(stderr, "%s ", speed);
  char *ca = centre("ca", 5);
  fprintf(stderr, "%5s ", ca);
  char *temp = centre("temp", 5);
  fprintf(stderr, "%5s%s\r", temp, RESET);

  free(time);
  free(speed);
  free(ca);
  free(temp);
}



void
display_thread_data(thread_data_t *td) {
  char *time = calloc(21, sizeof(char));
  sprintf(time, "%lu.%06lu", (*td->time_s), (*td->time_us));
  char *ctime = centre(time, 20);
  
  char **adcval = calloc(ADC_COUNT, sizeof(char *));
  char **cadcval = calloc(ADC_COUNT, sizeof(char *));
  for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
    adcval[channel] = calloc(8, sizeof(char));
    sprintf(adcval[channel], "%5lu", td->adc[channel]);
    cadcval[channel] = centre(adcval[channel], 5);
  }

  char *speed = calloc(6, sizeof(char));
  sprintf(speed, "%5f", td->speed_ind);
  char *cspeed = centre(speed, 5);
  char *ca = calloc(6, sizeof(char));
  sprintf(ca, "%5u", td->last_ca);
  char *cca = centre(ca, 5);
  char *temp = calloc(6, sizeof(char));
  sprintf(temp, "%2.3f\n", (*td->temperature));
  char *ctemp = centre(temp, 5);

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



int
main (int argc, const char ** argv)
{
  thread_data_t *td = create_thread_data();

  parse_args(argc, argv, td);

  if (signal(SIGINT, inthandle) == SIG_ERR)
    ferr("could not create signal handler");

  init(argc, argv, td);

  td->adc_handle = adc_open("/dev/spidev0.1");
  info("connected to ADC");

  if (wiringPiSetupGpio() == -1)
    ferr("failed to set up wiringpi lib");
  info("setup gpio");

  opt_setup(td);
  pinMode(16, INPUT);
  pinMode(20, INPUT);
  pinMode(21, INPUT);
  void opt_trip_16(void) { opt_mark(td, 0); }
  void opt_trip_20(void) { opt_mark(td, 1); }
  void opt_trip_21(void) { opt_mark(td, 2); }
  if (wiringPiISR(16, INT_EDGE_BOTH, &opt_trip_16) < 0) ferr("failed to set up GPIO interrupt");
  if (wiringPiISR(20, INT_EDGE_BOTH, &opt_trip_20) < 0) ferr("failed to set up GPIO interrupt");
  if (wiringPiISR(21, INT_EDGE_BOTH, &opt_trip_21) < 0) ferr("failed to set up GPIO interrupt");
  /*
    This doesn't seem to be working. The source uses the 'gpio' program to set the interrupts, but it
    doesn't succeed unless I run the python first for some reason.

    The line in the source:
      execl ("/usr/local/bin/gpio", "gpio", "edge", pinS, modeS, (char *)NULL) ;
    From:
      https://github.com/WiringPi/WiringPi/blob/master/wiringPi/wiringPi.c

    gpio program source at:
      https://github.com/WiringPi/WiringPi/blob/master/gpio/gpio.c
    
    Try running this manually, see what it comes out with. Maybe its failing but not
    failing properly? Try a MWE of the python prep script, could run that from this to set up GPIO properly?

    Does the pin need to be setup as input first? :O
   */
  info("set up optical encoder");

  // pthread_t tmp_thread;
  // if (pthread_create(&tmp_thread, NULL, tmp_thread_func, td))
  //   ferr("could not create thermometer thread");
  // info("started thermometer thread");
  // while (!td->tmp_ready) nsleep(100);
  // info("....... thermometer thread ready!"); // TODO: temperature

  pthread_t adc_thread;
  if (pthread_create(&adc_thread, NULL, adc_thread_func, td))
    ferr("could not create adc thread");
  info("started adc thread");
  while (!td->adc_ready) nsleep(100);
  info("....... adc thread ready!");

  pthread_t ctl_thread;
  if (pthread_create(&ctl_thread, NULL, ctl_thread_func, td))
    ferr("could not create adc thread");
  info("started control thread");
  while (!td->ctl_ready) nsleep(100);
  info("....... control thread ready!");

  pthread_t log_thread;
  if (pthread_create(&log_thread, NULL, log_thread_func, td))
    ferr("could not create log thread");
  info("started log thread");
  while (!td->log_ready) nsleep(100);
  info("....... log thread ready!");

  motor_setup();
  info("set up motor");
  info("warming up motor...");
  motor_warmup(256);

  info("begin!");
  unsigned int tish = 0;
  while ( (!cancelled) && (tish <= td->length_s) ) {
    sleep(1);
    tish ++;
    display_thread_data(td);
    display_titles();
  }
  td->stopped = 1;

  for (unsigned int i = 0; i < OPTENC_COUNT; i++) {
    fclose(td->opt_log_fps[i]);
  }

  motor_shutdown();

  if (cancelled)
    warn("received interrupt.");
  info("waiting for threads to rejoin...");

  if (pthread_join(log_thread, NULL))
    ferr("log thread could not rejoin");
  else
    info("log thread rejoined");

  if (pthread_join(ctl_thread, NULL))
    ferr("control thread could not rejoin");
  else
    info("control thread rejoined");

  if (pthread_join(adc_thread, NULL))
    ferr("adc thread could not rejoin");
  else
    info("adc thread rejoined");
  
  // if (pthread_join(tmp_thread, NULL))
  //   ferr("thermometer thread could not rejoin");
  // else
  //   info("thermometer thread rejoined");
  
  info("cleaning up...");
  tidy_logs(td);
  free_thread_data(td);
  info("done!");
  return 0;
}
