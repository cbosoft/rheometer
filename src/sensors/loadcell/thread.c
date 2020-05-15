#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "../../run/run.h"

#include "hx711.h"
#include "loadcell.h"

void *loadcell_thread_func(void *vptr)
{
  struct run_data *rd = (struct run_data *)vptr;

  rd->lc_ready = 1;
  while ( (!rd->stopped) && (!rd->errored) )
    read_loadcell(rd);

  pthread_exit(0);
  return NULL;
}
