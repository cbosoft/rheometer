#pragma once
#include <pthread.h>

enum MODE{ MODE_NORMAL, MODE_TUNING };
enum PHASE{ PHASE_INIT, PHASE_WARMUP, PHASE_SETTLE, PHASE_READING };


struct run_data {
  // {{{

  // actual data
  unsigned long time_s;
  unsigned long time_us;
  unsigned long start_time_s;
  unsigned long start_time_us;
  double time_s_f;

  unsigned long *adc;
  unsigned long loadcell_bytes;
  double loadcell_units;
  double ambient_temperature;
  double cylinder_temperature;
  struct {
    const char *name;
    double k_lc_to_m;
    double k_omega_to_m;
    double lc_z;
  } loadcell_calibration;
  
  // process control stuff
  double speed_ind;
  double speed_ind_timeout;
  double strainrate_ind;
  double stress_ind;
  double viscosity_ind;
  unsigned int last_ca;
  struct {
    const char *controller_name;
    double *control_params;
    int n_control_params;

    const char *setter_name;
    double *setter_params;
    int n_setter_params;

    int is_stress_controlled;
    double setpoint;
    unsigned int sleep_ms;
  } control_scheme;

  // run_data
  unsigned int length_s;
  double fill_depth;
  double needle_depth;
  char *tag;
  char *log_pref;
  char **log_paths;
  char **log_names;
  unsigned int log_count;
  char *uid;
  int phase;
  int hardware_version;

  // program control stuff
  struct adc_handle *adc_handle;
  unsigned int log_ready;
  unsigned int adc_ready;
  unsigned int tmp_ready;
  unsigned int opt_ready;
  unsigned int ctl_ready;
  unsigned int lc_ready;
  unsigned int cam_ready;
  int calm_start;
  int mode;

  unsigned int adc_busy;
  double adc_dt;
  
  unsigned int stopped;
  unsigned int errored;
  const char *error_string;
  int log_video;
  int log_photo;
  long cam_start;
  long cam_end;
  int motor_name_set;
  char *motor_name;

  // locks
  pthread_mutex_t lock_time;
  pthread_mutex_t lock_adc;
  pthread_mutex_t lock_adcdt;
  pthread_mutex_t lock_control;
  pthread_mutex_t lock_loadcell;
  pthread_mutex_t lock_temperature;
  pthread_mutex_t lock_speed;

  // }}}
};



struct run_data *init_run_data();
void free_run_data(struct run_data *td);

// speed getter/setter
void rd_set_speed(struct run_data *rd, double value);
void rd_set_speed_conversion_timeout(struct run_data *rd, double value);
double rd_get_speed(struct run_data *rd);
double rd_get_strainrate(struct run_data *rd);
double rd_get_speed_conversion_timeout(struct run_data *rd);
double rd_get_fill_depth(struct run_data *rd);

// temperature getter/setter
void rd_set_ambient_temperature(struct run_data *rd, double value);
double rd_get_ambient_temperature(struct run_data *rd);
void rd_set_cylinder_temperature(struct run_data *rd, double value);
double rd_get_cylinder_temperature(struct run_data *rd);

// loadcell getter/setter
void rd_set_loadcell_bytes(struct run_data *rd, unsigned long value);
unsigned long rd_get_loadcell_bytes(struct run_data *rd);
double rd_get_loadcell_units(struct run_data *rd);
double rd_get_stress(struct run_data *rd);

// control getter/setter
void rd_set_last_control_action(struct run_data *rd, unsigned int value);
unsigned int rd_get_last_control_action(struct run_data *rd);
void rd_set_strainrate_controlled(struct run_data *rd);
void rd_set_stress_controlled(struct run_data *rd);
int rd_get_is_stress_controlled(struct run_data *rd);
double rd_get_setpoint(struct run_data *rd);
unsigned int rd_get_control_interval(struct run_data *rd);

// time getter/setter
void rd_set_start_time(struct run_data *rd);
void rd_set_time(struct run_data *rd);
double rd_get_time(struct run_data *rd);
void rd_get_time_parts(struct run_data *rd, unsigned long *time_s, unsigned long *time_us);

// adc getter/setter
unsigned long rd_get_adc(struct run_data *rd, int i);
unsigned long *rd_swap_adc(struct run_data *rd, unsigned long *value);

// vim: ft=c foldmethod=marker
