#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
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

void
display_titles(void)
{
  fprintf(stderr, "\033[1m%20s ", "t");
  for (unsigned int channel = 0; channel < ADC_COUNT; channel++)
    fprintf(stderr, "%5u ", channel);
  fprintf(stderr, "%5s ", "spd");
  fprintf(stderr, "%5s ", "ca");
  fprintf(stderr, "%5s\033[0m\n", "T");
}




void
display_thread_data(thread_data_t *td) {
  fprintf(stderr, "%14lu.%06lu ", (*td->time_s), (*td->time_us));

  for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
    fprintf(stderr, "%5lu ", td->adc[channel]);
  }

  fprintf(stderr, "%5f ", td->speed_ind);
  fprintf(stderr, "%5u ", td->last_ca);
  fprintf(stderr, "%2.3f\n", (*td->temperature));
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
    if ((tish % 10) == 1)
      display_titles();
    display_thread_data(td);
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
