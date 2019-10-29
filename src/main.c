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
#include "webcam.h"
#include "log.h"
#include "adc.h"
#include "util.h"
#include "tar.h"
#include "display.h"
#include "thermometer.h"

  pthread_t tmp_thread;

#define SETUP_THREAD(THREADVAR,THREADFUNC,NAME,WATCHVAR) \
  if (pthread_create(& THREADVAR, NULL, THREADFUNC, rd)) \
    ferr("main", "could not create "NAME" thread"); \
  while (! WATCHVAR); \
  pthread_setname_np(THREADVAR, NAME); \
  info("  "NAME" ready!");

#define THREAD_JOIN(THREADVAR,NAME) \
  if (pthread_join(THREADVAR, NULL)) \
    ferr("main", "error in "NAME"thread."); \
  else \
    info("  "NAME" done"); \




static unsigned int cancelled = 0;




void inthandle(int signo)
{
  if (signo == SIGINT) {
    fprintf(stderr, "\r");
    cancelled ++;
  }

  if (cancelled > 10) {
    fprintf(stderr, "\n\n CONTINUE PRESSING CTRL+C TO HALT EXECUTION FORCEFULLY.\n THIS WILL RUIN THE RUN.\n DON'T TAKE THIS WARNING LIGHTLY.\n\n");
    //warn("interrupt_handler", "continue pressing CTRL+c to halt execution forcefully.")
  }
  else if (cancelled > 20) {
    exit(1);
  }
  else {
    fprintf(stderr, "\n\nInterrupted...\n\n");
  }
}




int main (int argc, const char ** argv)
{
  struct run_data *rd = init_run_data();

  parse_args((unsigned int)argc, argv, rd);

  if (signal(SIGINT, inthandle) == SIG_ERR)
    ferr("main", "could not create signal handler");

  generate_log_prefix(rd);
  info("unique ID: %s%s%s", BOLD, rd->uid, RESET);

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


  info("starting sensor threads...");
  pthread_t tmp_thread, adc_thread, ctl_thread, log_thread, lc_thread, vid_thread;
  SETUP_THREAD(tmp_thread, thermometer_thread_func, "thermometer", rd->tmp_ready);
  SETUP_THREAD(adc_thread, adc_thread_func, "adc", rd->adc_ready);
  SETUP_THREAD(lc_thread, loadcell_thread_func, "loadcell", rd->lc_ready);
  sleep(1);

  info("starting logging thread...");
  SETUP_THREAD(log_thread, log_thread_func, "log", rd->log_ready);
  if (rd->video_device != NULL) {
    SETUP_THREAD(vid_thread, cam_thread_func, "video", rd->cam_ready);
  }
  sleep(1);

  motor_setup();
  info("warming up motor...");
  motor_warmup(rd, 512 );
  info("  motor ready!");

  info("starting control thread...");
  SETUP_THREAD(ctl_thread, ctl_thread_func, "control", rd->ctl_ready);
  sleep(3);
  
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

  THREAD_JOIN(log_thread, "log");
  if (rd->video_device != NULL) {
    THREAD_JOIN(vid_thread, "video");
  }
  THREAD_JOIN(ctl_thread, "control");
  THREAD_JOIN(lc_thread, "loadcell");
  THREAD_JOIN(adc_thread, "adc");
  THREAD_JOIN(tmp_thread, "thermometer");

  info("cleaning up...");
  save_run_params_to_json(rd);
  info("  params written");
  tidy_logs(rd);
  info("  logs tar'd to "FGBLUE"\"%s.tar.bz2\""RESET, rd->log_pref);
  info("  unique ID: %s%s%s", BOLD, rd->uid, RESET);

  // invalid pointer error... trying to free that which has not been alloc'd?
  free_run_data(rd);
  info("  data free'd");
  info("done!");
  return 0;
}
