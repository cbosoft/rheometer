#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "../../run/run.h"
#include "../../util/json.h"
#include "../../util/error.h"

#include "hx711.h"
#include "loadcell.h"


static double get_loadcell_calibration_parameter(cJSON *json, const char *paramname, const char *description)
{
  cJSON *param = cJSON_GetObjectItem(json, paramname);

  if (param == NULL) {
    cJSON_Delete(json);
    ferr("get_loadcell_calibration_parameter", "Loadcell calibration requires a parameter \"%s\" (%s)", paramname, description);
  }

  if (cJSON_IsNumber(param))
    return param->valuedouble;

  ferr("get_control_scheme_parameter", "JSON parse error. \"%s\" is not expected to be a number (%s).", paramname, description);

  // won't return due to ferr
  return -1;
}




static cJSON *get_calibration_json(cJSON *json, const char *name)
{
  static int recursion_count = 0;

  cJSON *cal_json = cJSON_GetObjectItem(json, name);
  if (cal_json == NULL) {
    ferr("loadcell_setup", "Loadcell calibration with name \"%s\" not found.", name);
  }

  if (cJSON_IsString(cal_json)) {
    recursion_count++;
    return get_calibration_json(json, cal_json->valuestring);
  }
  else if (cJSON_IsObject(cal_json)) {
    return cal_json;
  }

  ferr("get_calibration_json", "calibration with name \"%s\" has wrong type; should be object.");
  return NULL;
}


static void loadcell_setup(struct run_data *rd)
{
  cJSON *json = read_json("data/loadcell_calibration.json");
  cJSON *cal_json = get_calibration_json(json, rd->loadcell_calibration.name);
  rd->loadcell_calibration.k_lc_to_m = get_loadcell_calibration_parameter(cal_json, "k_lc_to_m", "coefficient converting loadcell value to total torque");
  rd->loadcell_calibration.k_omega_to_m = get_loadcell_calibration_parameter(cal_json, "k_omega_to_m", "coefficient converting rotation rate to friction torque");
  rd->loadcell_calibration.lc_z = get_loadcell_calibration_parameter(cal_json, "lc_z", "loadcell value zero offset");
}


void *loadcell_thread_func(void *vptr)
{
  struct run_data *rd = (struct run_data *)vptr;

  loadcell_setup(rd);

  hx711_setup();
  rd->lc_ready = 1;
  while ( (!rd->stopped) && (!rd->errored) )
    read_loadcell(rd);

  pthread_exit(0);
  return NULL;
}
