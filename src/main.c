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
display_thread_data(thread_data *td) {
  fprintf(stderr, "t = %lu.%06lu ", (*td->time_s), (*td->time_us));

  for (unsigned int channel = 0; channel < ADC_COUNT; channel++) {
    fprintf(stderr, "%5lu ", td->adc[channel]);
  }

  fprintf(stderr, "%f\n", (*td->temperature));
}



int
main (int argc, const char ** argv)
{
  if (getuid() != 0)
    ferr("Hardware PWM needs root.");
  
  fprintf(stderr, "rheometer v%s\n", VERSION);

  if (signal(SIGINT, inthandle) == SIG_ERR)
    ferr("could not create signal handler");

  thread_data *td = init(argc, argv);

  td->adc_h = adc_open("/dev/spidev0.1");
  info("connected to ADC");
  
  pthread_t log_thread;
  if (pthread_create(&log_thread, NULL, log_thread_func, td))
    ferr("could not create log thread");
  info("started log thread");

  pthread_t adc_thread;
  if (pthread_create(&adc_thread, NULL, adc_thread_func, td))
    ferr("could not create adc thread");
  info("started adc thread");

  if (wiringPiSetupGpio() == -1)
    ferr("failed to set up wiringpi lib");
  info("setup gpio");
  
  motor_setup();
  info("setup motor");

  opt_setup(td);
  void opt_trip_16(void) { opt_mark(td->opt_log_fps[0]); }
  void opt_trip_20(void) { opt_mark(td->opt_log_fps[1]); }
  void opt_trip_21(void) { opt_mark(td->opt_log_fps[2]); }
  if (wiringPiISR(16, INT_EDGE_BOTH, &opt_trip_16) < 0) ferr("failed to set up GPIO interrupt");
  if (wiringPiISR(20, INT_EDGE_BOTH, &opt_trip_20) < 0) ferr("failed to set up GPIO interrupt");
  if (wiringPiISR(21, INT_EDGE_BOTH, &opt_trip_21) < 0) ferr("failed to set up GPIO interrupt");
  info("setup optical encoder");

  
  // Order is important here.
  //while (!td->tmp_ready) nsleep(100); // TODO: temperature
  while (!td->adc_ready) nsleep(100);
  info("adc thread ready!");
  //while (!td->ctl_ready) nsleep(100); // TODO: control
  while (!td->log_ready) nsleep(100);
  info("log thread ready!");

  info("warming up motor...");
  motor_warmup(256);

  info("begin!");
  unsigned int tish = 0;
  while ( (!cancelled) && (tish <= td->length_s) ) {
    sleep(1);
    tish ++;
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
    ferr("log_thread could not rejoin");
  if (pthread_join(adc_thread, NULL))
    ferr("adc_thread could not rejoin");
  //if (pthread_join(tmp_thread, NULL))
  //  ferr("tmp_thread could not rejoin");
  //if (pthread_join(opt_thread, NULL))
  //  ferr("opt_thread could not rejoin");
  //if (pthread_join(ctl_thread, NULL))
  //  ferr("ctl_thread could not rejoin");
  
  //info("cleaning logs...");
  // TODO: clean up logs into single .tar.bz2 file

  info("done!");
  free_thread_data(td);
  return 0;
}
