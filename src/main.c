#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sched.h>
#include <string.h>
#include <sys/time.h>

#include <wiringPi.h>

#include "run.h"
#include "args.h"
#include "error.h"
#include "motor.h"
#include "loadcell.h"
#include "opt.h"
#include "control.h"
#include "log.h"
#include "adc.h"
#include "util.h"
#include "tar.h"
#include "display.h"




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
  struct run_data *rd = init_run_data();

  parse_args((unsigned int)argc, argv, rd);

  if (signal(SIGINT, inthandle) == SIG_ERR)
    ferr("main", "could not create signal handler");

  // struct sched_param sched = {0};
  // sched.sched_priority = sched_get_priority_max(SCHED_RR);
  // if (sched_setscheduler(0, SCHED_RR, &sched) == -1) {
  //   ferr("main", "Error setting high priority.");
  // }

  generate_log_prefix(rd);

  rd->adc_handle = adc_open("/dev/spidev0.1");
  info("connected to ADC");

  if (wiringPiSetupGpio() == -1) {
    adc_close(rd->adc_handle);
    ferr("main", "failed to set up wiringpi lib");
  }

  info("setup gpio");

  opt_setup(rd);
  info("set up optical encoder");

  loadcell_setup();
  info("set up loadcell");

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
  if (pthread_create(&adc_thread, NULL, adc_thread_func, rd))
    ferr("main", "could not create adc thread");
  while (!rd->adc_ready) rh_nsleep(100);
  pthread_setname_np(adc_thread, "ADC");
  info("  adc ready!");

  pthread_t ctl_thread;
  if (pthread_create(&ctl_thread, NULL, ctl_thread_func, rd))
    ferr("main", "could not create adc thread");
  while (!rd->ctl_ready) rh_nsleep(100);
  pthread_setname_np(ctl_thread, "ctl");
  info("  controller ready!");

  pthread_t log_thread;
  if (pthread_create(&log_thread, NULL, log_thread_func, rd))
    ferr("main", "could not create log thread");
  while (!rd->log_ready) rh_nsleep(100);
  pthread_setname_np(log_thread, "log");
  info("  logger ready!");

  info("begin!");
  unsigned int tish = 0;
  while ( (!cancelled) && (tish <= rd->length_s) ) {
    sleep(1);
    tish ++;
    display_thread_data(rd);
    display_titles();
  }
  rd->stopped = 1;
  fprintf(stderr, "\n");

#ifndef DEBUG
  system("gpio unexport 16");
  system("gpio unexport 20");
  system("gpio unexport 21");
  info("optenc stopped");
#endif

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
  save_run_params_to_json(rd);
  info("  params written");
  tidy_logs(rd);
  info("  logs tar'd to "FGBLUE"\"%s.tar.bz2\""RESET, rd->log_pref);
  free_run_data(rd);
  info("  data free'd");
  info("done!");
  return 0;
}
