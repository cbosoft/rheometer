#include <stdlib.h>
#include <stdio.h>

#include "../util/display.h"
#include "../control/control.h"
#include "../defaults.h"
#include "../version.h"

#include "args.h"

void usage(void)
{
  fprintf(stdout, 
      "\n"
      "  "BOLD"rheometer"RESET" control program v"VERSION"\n"
      "\n"
      "  "BOLD"Usage:"RESET"\n"
      "    rheometer ["FGYELLOW"common options"RESET"] ["FGGREEN"command"RESET"] ["FGBLUE"command options"RESET"]\n"
      "    rheometer "FGGREEN"run "FGBLUE"-l <length> -d <fill-depth> ("FGMAGENTA"-j <control scheme> | -c <controller> -s <setter>"FGBLUE") -w <hardware version>"RESET" [more options below]\n"
      "    rheometer "FGGREEN"schedule"RESET" ["FGBLUE"schedule options"RESET"] ["FGBLUE"run options"RESET"] \n"
      //"    rheometer config (add|edit|remove) (controlscheme|calibration|schedule)\n"
      "    rheometer --help\n"
      "\n"
  );
}




void args_help()
{
  fprintf(stdout,
      "  "BOLD"Common Options:"RESET"\n"
      "    "FGYELLOW"--quiet"RESET"\n"
      "    Suppress most output; progress is shown by progress bar.\n"
      "\n"                                                                           //
      "  "BOLD""FGGREEN"Run"RESET""BOLD" Options:"RESET"\n"
      "    "FGBLUE"-l, --length"RESET"\n"
      "    Length of run, can be given in seconds or minutes, dictated by a suffixed\n"
      "    character. e.g. \"10s\" is 10 seconds, \"10m\" is 10 minutes. If unit char\n"
      "    is ignored, the value is taken as seconds.\n"
      "\n"                                                                           //
      "    "FGBLUE"-d, --fill-depth"RESET"\n"
      "    Fill depth of Couette cell. This is the height fluid extends up on the \n"
      "    "BOLD"inner"RESET" cylinder, in mm. Total height of inner cylinder is\n"
      "    ~73mm.\n"
      "\n"                                                                           //
      "    "FGBLUE"-d, --fill-depth"RESET"\n"
      "    Needle depth in the Couette cell. This is the height needle is in fluid, in\n"
      "    mm. Total height of needle is ~37mm.\n"
      "\n"                                                                           //
      "    "FGBLUE"-c, --controller"RESET"\n"
      "    "FGBLUE"-s, --setter"RESET"\n"
      "    Respectively, controller and setter module names. Must be used, or specified\n"
      "    using a control scheme.\n"
      "\n"                                                                           //
      "    "FGBLUE"-pc, --controller-params"RESET"\n"
      "    "FGBLUE"-ps, --setter-params"RESET"\n"
      "    Parameters passed to controller and setter, respectively. Optional.\n"
      "\n"                                                                           //
      "    "FGBLUE"-j, --control-scheme"RESET"\n"
      "    Control scheme JSON file path. More information in the 'Control Schemes' \n"
      "    section below.\n"
      "\n"                                                                           //
      "    "FGBLUE"-a, --loadcell-calibration"RESET"\n"
      "    Selects an alternate loadcell calibration name from\n"
      "    \"data/loadcell_calibration.json\".\n"
      "\n"                                                                           //
      "    "FGBLUE"-w, --hardware-version"RESET"\n"
      "    Hardware version specifying when the hardware was last changed. This should \n"
      "    be given as a ISO8601 date spec without separating dashes (YYYYMMDD).\n"
      "\n"                                                                           //
      "    "FGBLUE"--motor"RESET"\n"
      "    Motor name or identifier. Added to resulting run parameters.\n"
      "\n"
      "    "FGBLUE"--material"RESET"\n"
      "    Material name or identifier. Added to resulting run parameters.\n"
      "\n"
      "    "FGBLUE"-t, --tag"RESET"\n"
      "    A short descriptive name for the test run. Underscores and spaces will be\n"
      "    replaced by hyphens '-'. Optional. Default is \""TAGDEFAULT"\".\n"
      "\n"                                                                           //
      "    "FGBLUE"--calm-start"RESET"\n"
      "    Enable this flag to remove the high power motor start up. This is useful for\n"
      "    tuning position of the outer cylinder for very thick or non-Newtonian fluids.\n"
      "    However, the motor is more likely to stall on start, so be wary about doing\n"
      "    tests with this data.\n"
      "\n"                                                                           //
      "    "FGBLUE"-v, --video"RESET"\n"
      "    By setting this option, video logging is enabled. A video of the name \n"
      "    \"${prefix}_video.mp4\" will be created and stored alongside the other log\n"
      "    files. Uses RPi camera module.\n"
      "\n"                                                                           //
      "    "FGBLUE"-p, --photo"RESET"\n"
      "    By setting this option, a photo of the name \"${prefix}_photo.mp4\" will be\n"
      "    created and stored alongside the other log files, taken before the logging\n"
      "    begins. Uses RPi camera module.\n"
      "\n"                                                                           //
      "  "BOLD""FGGREEN"Schedules"RESET""BOLD":"RESET"\n"
      "    Schedules are defined as a series of controllers, and a series of setters.\n"
      "    Each controller is used in combination with a series of setters. If the\n"
      "    setter or controller is the same and the number of parameters is the same,\n"
      "    then the parameters will be interpolated between. An example:\n"
      "\n"                                                                           //
      "    For controllers \"none\" and \"pid\", and constant setpoints of 100,200,\n"
      "    and 2 interpolation points, the schedule looks like:\n"
      "\n"                                                                           //
      "      controller: none; setpoint: 100\n"
      "      controller: none; setpoint: 150\n"
      "      controller: none; setpoint: 200\n"
      "      controller: pid; setpoint: 100\n"
      "      controller: pid; setpoint: 150\n"
      "      controller: pid; setpoint: 200\n"
      "\n"                                                                           //
      "    Schedules are defined by file, or directly on the command line. A schedule\n"
      "    file is a JSON file; an array of objects where each element defines either\n"
      "    a controller or setter. In addition you can provide overall schedule settings\n"
      "    by adding an object to the array the the type \"params\". You can add as many\n"
      "    as you like, any conflicting params are overidden by subsequent param objects.\n"
      "    An example of what you can put in a schedule json file:\n"
      "\n"                                                                           //
      FGYELLOW"    [\n"RESET
      FGYELLOW"      { \n"RESET
      FGYELLOW"        \"name\":\"pid\", \n"RESET
      FGYELLOW"        \"type\":\"controller\", \n"RESET
      FGYELLOW"        \"params\": [0.1, 0.1, 0.0]\n"RESET
      FGYELLOW"      }, \n"RESET
      FGYELLOW"      { \n"RESET
      FGYELLOW"        \"name\":\"constant\", \n"RESET
      FGYELLOW"        \"type\":\"setter\", \n"RESET
      FGYELLOW"        \"params\": 1.22 \n"RESET
      FGYELLOW"      }, \n"RESET
      FGYELLOW"      { \n"RESET
      FGYELLOW"        \"name\":\"pid\", \n"RESET
      FGYELLOW"        \"type\":\"controller\", \n"RESET
      FGYELLOW"        \"params\": 2.2 \n"RESET
      FGYELLOW"      }, \n"RESET
      FGYELLOW"      { \n"RESET
      FGYELLOW"        \"type\":\"params\", \n"RESET
      FGYELLOW"        \"n_interpolation_points\": 5 \n"RESET
      FGYELLOW"        \"interpolation_type\": \"log\" \n"RESET
      FGYELLOW"        \"each_run_length\": \"15s\" \n"RESET
      FGYELLOW"      } \n"RESET
      FGYELLOW"    ]\n"RESET
      "\n"                                                                           //
      "  "BOLD""FGGREEN"Schedule"RESET""BOLD" Options:"RESET"\n"
      "    "FGBLUE"-p, --params"RESET"\n"
      "    Set the params for either the setter or controller.\n"
      "\n"
      "    "FGBLUE"-s, --setter"RESET"\n"
      "    Set the setter module name. Resets params\n"
      "\n"
      "    "FGBLUE"-c, --controller"RESET"\n"
      "    Set the controller module name. Resets params.\n"
      "\n"
      "    "FGBLUE"-f, --schedule-file"RESET"\n"
      "    Load a series of schedule points (setters, controllers, parameters) from file.\n"
      "\n"                                                                           //
      "    "FGBLUE"--interp-number"RESET"\n"
      "    Number of interpolation points to use. Set to zero to disable. Optional,\n"
      "    default is %d.\n"
      "\n"
      "    "FGBLUE"--interp-linear"RESET"\n"
      "    Use linear interpolation. Does nothing, this is the default.\n"
      "\n"
      "    "FGBLUE"--interp-log"RESET"\n"
      "    Use logarithmic interpolation instead of linear.\n"
      "\n"
  , INTERP_N_DEFAULT); // TODO: config options
}
