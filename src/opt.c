#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>

#include <wiringPi.h>

#include "opt.h"
#include "control.h"
#include "error.h"
#include "log.h"



#define OPT_MARK(IDX) opt_mark_ ## IDX
#define OPT_SETUP(IDX) \
  if (wiringPiISR(opt_pins[IDX], INT_EDGE_BOTH, &(OPT_MARK(IDX))) < 0) \
    ferr("opt_setup", "failed to set up interrupt for pin %d", opt_pins[0]);




// globals required for optenc operation
static const uint8_t opt_pins[OPTENC_COUNT] = {20, 21};
struct run_data *ord = NULL;

void opt_mark(struct run_data *rd, unsigned int i);
void opt_mark_0(void) { opt_mark(ord, 0); }
void opt_mark_1(void) { opt_mark(ord, 1); }

unsigned int tripc[OPTENC_COUNT] = {0};
time_t last_convert = {0};
double last_speed = 0.0;
int opt_log_idxs[OPTENC_COUNT] = {0};




void opt_setup(struct run_data *rd)
{
  ord = rd;
  char logname[10] = {0};
  for (uint8_t i = 0; i < OPTENC_COUNT; i++) {

    snprintf(logname, 10, "opt-%d", i);
    opt_log_idxs[i] = add_log(rd, logname, "%s_opt%d-combined.csv", rd->log_pref, opt_pins[i]);

    // Create empty log file for appending to later
    FILE *fp = fopen(rd->log_paths[opt_log_idxs[i]], "w");
    fclose(fp);

    pinMode(opt_pins[i], INPUT);

  }
  OPT_SETUP(0);
  OPT_SETUP(1);
  last_convert = time(NULL);
  sleep(1);
}




void opt_mark(struct run_data *rd, unsigned int i)
{
  if (rd->stopped)
    pthread_exit(0);

  struct timeval tv;
  gettimeofday(&tv, 0);
  FILE *fp = fopen(rd->log_paths[opt_log_idxs[i]], "a");
  fprintf(fp, "%lu.06%lu\n", tv.tv_sec, tv.tv_usec);
  fflush(fp);
  fclose(fp);

  tripc[i] ++;

}




double get_speed()
{
  time_t now = time(NULL);
  double dt = difftime(now, last_convert);

  if (dt > 1.0) {
    last_convert = now;
    int count = 0;
    for (int i = 0; i < OPTENC_COUNT; i++) {

#ifdef DEBUG
      count += 3;
#else
      count += tripc[i];
      tripc[i] = 0;
#endif

    }

    double dtripc = ((double)count) / ((double)OPTENC_COUNT);
    last_speed = dtripc / (12.0 * dt); // returns speed in hz (rev. per second)

  }

  return last_speed;
}
