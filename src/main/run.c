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

#include "../run/run.h"
#include "../args/args.h"
#include "../util/error.h"
#include "../motor/motor.h"
#include "../sensors/loadcell/loadcell.h"
#include "../sensors/encoder/encoder.h"
#include "../sensors/camera/video.h"
#include "../sensors/camera/photo.h"
#include "../sensors/thermometer/thermometer.h"
#include "../control/control.h"
#include "../log/log.h"
#include "../sensors/adc/adc.h"
#include "../util/sleep.h"
#include "../log/tar.h"
#include "../version.h"
#include "../util/display.h"
#include "../delays.h"
#include "main.h"

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

#ifdef DEBUG

  (void) signo;
  exit(1);

#else

  if (signo == SIGINT) {
    fprintf(stderr, "\r");
    cancelled ++;
  }

  if (cancelled > 20) {
    exit(1);
  }
  else if (cancelled == 10) {
    fprintf(stderr, "%s",
        "\n"
        "\n"
        "CONTINUE PRESSING CTRL+C TO HALT EXECUTION FORCEFULLY.\n"
        "THIS WILL RUIN THE RUN.\n"
        "DON'T TAKE THIS WARNING LIGHTLY.\n"
        "\n");
  }
  else if ((cancelled == 1) && (!get_quiet())) {
    fprintf(stderr, "\n\nInterrupted...\n\n");
  }
#endif
}



int run_main(int argc, const char ** argv)
{

#ifndef DEBUG
  if (getuid() != 0)
    argerr("Hardware PWM needs root.");
#endif

  struct run_data *rd = init_run_data();

  parse_run_args((unsigned int)argc, argv, rd);

  fprintf(stderr, "  "BOLD"rheometer"RESET" v%s%c", VERSION, (get_quiet()?'\r':'\n'));

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

  if (rd->log_photo) {
    info("photographing device");
    take_photo(rd);
  }

  info("starting sensor threads...");
  pthread_t tmp_thread, adc_thread, ctl_thread, log_thread, lc_thread, vid_thread;
  SETUP_THREAD(tmp_thread, thermometer_thread_func, "thermometer", rd->tmp_ready);
  SETUP_THREAD(adc_thread, adc_thread_func, "adc", rd->adc_ready);
  SETUP_THREAD(lc_thread, loadcell_thread_func, "loadcell", rd->lc_ready);
  sleep_ms(DELAY_THREADS_MS);

  info("starting logging thread...");
  SETUP_THREAD(log_thread, log_thread_func, "log", rd->log_ready);
  if (rd->log_video) {
    SETUP_THREAD(vid_thread, cam_thread_func, "video", rd->cam_ready);
  }
  sleep_ms(DELAY_LOG_MS);

  motor_setup();
  rd->phase = PHASE_WARMUP;
  info("warming up motor...");
  motor_warmup(rd);
  info("  motor ready!");

  info("starting control thread...");
  rd->phase = PHASE_SETTLE;
  SETUP_THREAD(ctl_thread, ctl_thread_func, "control", rd->ctl_ready);

  if (strcmp(rd->control_scheme.controller_name, "none") != 0)
    sleep(DELAY_CONTROL_MS);

  rd->phase = PHASE_READING;

  switch (rd->mode) {

    case MODE_TUNING:
      do_tuning(rd);
      break;

    case MODE_NORMAL:  /* fall through */
    default:
      info("begin!");
      unsigned int tish = 0;
      rd_set_start_time(rd);
      while ( (!cancelled) && (tish <= rd->length_s) ) {
        sleep(1);
        tish ++;
        display_thread_data(rd);
        display_titles();
      }
      break;
      
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
  if (rd->log_video) {
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
  return cancelled;
}
