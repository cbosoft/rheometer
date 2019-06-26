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

  // This is tentatively working now. It seems to need the unexporting at the end, or the interrupts won't work.
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
  save_run_params_to_json(td);
  info("  params written");
  tidy_logs(td);
  info("  logs tar'd");
  free_thread_data(td);
  info("  data free'd");
#ifndef DEBUG
  system("gpio unexport 16");
  system("gpio unexport 20");
  system("gpio unexport 21");
  info("  gpios unexported");
#endif
  info("done!");
  return 0;
}
