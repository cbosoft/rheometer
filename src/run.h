#pragma once


struct run_data {
  // {{{

  // actual data
  unsigned long *time_s;
  unsigned long *time_us;
  unsigned long start_time_s;
  unsigned long start_time_us;
  double time_s_f;
  unsigned long *adc;
  unsigned long loadcell_bytes;
  double loadcell_units;
  float *temperature;
  
  // process control stuff
  float speed_ind;
  float strainrate_ind;
  float stress_ind;
  float viscosity_ind;
  float **ptimes;
  char *control_scheme;
  char *control_scheme_path;
  struct control_params *control_params;
  unsigned int last_ca;
  float *errhist;

  // run_data
  unsigned int length_s;
  float fill_depth;
  char *tag;
  char *log_pref;
  char **log_paths;
  unsigned int log_count;

  // program control stuff
  struct adc_handle *adc_handle;
  unsigned int log_ready;
  unsigned int adc_ready;
  unsigned int tmp_ready;
  unsigned int opt_ready;
  unsigned int ctl_ready;

  unsigned int adc_busy;
  
  unsigned int stopped;
  unsigned int errored;
  const char *error_string;

  // }}}
};




struct run_data *init_run_data();
void free_run_data(struct run_data *td);




// vim: ft=c foldmethod=marker
