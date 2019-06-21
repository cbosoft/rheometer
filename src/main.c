#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>

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
  if (signal(SIGINT, inthandle) == SIG_ERR)
    ferr("could not create signal handler");

  thread_data *td = new_thread_data();
  td->adc_h = adc_open("/dev/spidev0.1");
  td->run_d = parse_args(argc, argv);
  
  //pthread_t *optenc_threads = setup_optenc_threads();

  pthread_t log_thread;
  if (pthread_create(&log_thread, NULL, log_thread_func, td))
    ferr("could not create log thread");

  pthread_t adc_thread;
  if (pthread_create(&adc_thread, NULL, adc_thread_func, td))
    ferr("could not create adc thread");
  
  // Order is important here.
  //while (!td->tmp_ready) nsleep(100);
  while (!td->adc_ready) nsleep(100);
  //while (!td->ctl_ready) nsleep(100);
  //while (!td->opt_ready) nsleep(100);
  while (!td->log_ready) nsleep(100);

  unsigned int tish = 0;
  while ( (!cancelled) && (tish <= td->run_d->length_s) ) {
    sleep(1);
    tish ++;
    display_thread_data(td);
  }
  td->stopped = 1;

  if (cancelled)
    fprintf(stderr, "\033[33mCancelled!\033[0m\n");
  fprintf(stderr, "Waiting for threads to rejoin...\n");
  
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
  
  //fprintf(stderr, "Cleaning logs...\n");
  // TODO: clean up logs into single .tar.bz2 file

  fprintf(stderr, "Done!\n");
  free_thread_data(td);
  return 0;
}
