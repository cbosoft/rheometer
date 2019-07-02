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

  parse_args((unsigned int)argc, argv, td);

  if (signal(SIGINT, inthandle) == SIG_ERR)
    ferr("main", "could not create signal handler");

  init(td);

  td->adc_handle = adc_open("/dev/spidev0.1");
  info("connected to ADC");

  if (wiringPiSetupGpio() == -1)
    ferr("main", "failed to set up wiringpi lib");
  info("setup gpio");

  // This is tentatively working now. It seems to need the unexporting at the end, or the interrupts won't work.
  opt_setup(td);
  pinMode(16, INPUT);
  pinMode(20, INPUT);
  pinMode(21, INPUT);
  void opt_trip_16(void) { opt_mark(td, 0); }
  void opt_trip_20(void) { opt_mark(td, 1); }
  void opt_trip_21(void) { opt_mark(td, 2); }
  if (wiringPiISR(16, INT_EDGE_BOTH, &opt_trip_16) < 0) ferr("main", "failed to set up GPIO interrupt");
  if (wiringPiISR(20, INT_EDGE_BOTH, &opt_trip_20) < 0) ferr("main", "failed to set up GPIO interrupt");
  if (wiringPiISR(21, INT_EDGE_BOTH, &opt_trip_21) < 0) ferr("main", "failed to set up GPIO interrupt");
  info("set up optical encoder");

  // pthread_t tmp_thread;
  // if (pthread_create(&tmp_thread, NULL, tmp_thread_func, td))
  //   ferr("could not create thermometer thread");
  // info("started thermometer thread");
  // while (!td->tmp_ready) nsleep(100);
  // info("....... thermometer thread ready!"); // TODO: temperature

  motor_setup();
  info("warming up motor...");
  motor_warmup(650);
  info("motor ready!");

  info("starting threads...");
  pthread_t adc_thread;
  if (pthread_create(&adc_thread, NULL, adc_thread_func, td))
    ferr("main", "could not create adc thread");
  while (!td->adc_ready) nsleep(100);
  info("  adc ready!");

  pthread_t ctl_thread;
  if (pthread_create(&ctl_thread, NULL, ctl_thread_func, td))
    ferr("main", "could not create adc thread");
  while (!td->ctl_ready) nsleep(100);
  info("  controller ready!");

  pthread_t log_thread;
  if (pthread_create(&log_thread, NULL, log_thread_func, td))
    ferr("main", "could not create log thread");
  while (!td->log_ready) nsleep(100);
  info("  logger ready!");

  info("begin!");
  unsigned int tish = 0;
  while ( (!cancelled) && (tish <= td->length_s) ) {
    sleep(1);
    tish ++;
    display_thread_data(td);
    display_titles();
  }
  td->stopped = 1;
  fprintf(stderr, "\n");

#ifndef DEBUG
  system("gpio unexport 16");
  system("gpio unexport 20");
  system("gpio unexport 21");
  info("optenc stopped");
#endif

  for (unsigned int i = 0; i < OPTENC_COUNT; i++) {
    fclose(td->opt_log_fps[i]);
  }

  motor_shutdown();
  info("motor off");

  if (cancelled)
    warn("main", "received interrupt.");
  info("waiting for threads to finish...");

  if (pthread_join(log_thread, NULL))
    ferr("main", "log thread could not rejoin");
  else
    info("  logger done");

  if (pthread_join(ctl_thread, NULL))
    ferr("main", "control thread could not rejoin");
  else
    info("  controller done");

  if (pthread_join(adc_thread, NULL))
    ferr("main", "adc thread could not rejoin");
  else
    info("  adc done");
  
  // if (pthread_join(tmp_thread, NULL))
  //   ferr("thermometer thread could not rejoin");
  // else
  //   info("  thermometer done");
  
  info("cleaning up...");
  save_run_params_to_json(td);
  info("  params written");
  tidy_logs(td);
  info("  logs tar'd to "FGBLUE"\"%s.tar.bz2\""RESET, td->log_pref);
  free_thread_data(td);
  info("  data free'd");
  info("done!");
  return 0;
}
