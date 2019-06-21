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
    fprintf(stderr, "\rctrl-c pressed!\n");
    cancelled = 1;
  }
}

void
display_thread_data(thread_data *t_d) {
  fprintf(stderr, "%lu.%06lu \n", (*t_d->time_s), (*t_d->time_us));
}


int
main (int argc, const char ** argv)
{
  run_data *r_d = parse_args(argc, argv);
  thread_data *t_d = new_thread_data();
  //pthread_t *optenc_threads = setup_optenc_threads();
  pthread_t log_thread;

  if (pthread_create(&log_thread, NULL, log_thread_func, t_d))
    ferr("could not create log thread");

  if (signal(SIGINT, inthandle) == SIG_ERR)
    exit(1);
  
  while (!cancelled) {
    display_thread_data(t_d);
    sleep(1);
  }

  t_d->stopped = 1;
  
  if (pthread_join(log_thread, NULL))
    ferr("could not join thread");

  free_thread_data(t_d);
  free_run_data(r_d);
  return 0;
}
