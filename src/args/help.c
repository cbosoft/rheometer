#include <stdlib.h>
#include <stdio.h>

#include "../util/display.h"
#include "../control/control.h"
#include "../log/tag.h"
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
      "    rheometer "FGGREEN"run "FGBLUE"-l <length> -d <fill-depth> (-c <control scheme> | -c <controller> -s <setter>) -w <hardware version>"RESET" [more options below]\n"
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
      "    "FGBLUE"-s, --controller"RESET"\n"
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
      "  "BOLD""FGGREEN"Schedule"RESET""BOLD" Options:"RESET"\n"
      "    Schedules are defined by file ('-f') option, or directly on the command line, using the \n"
      "    '-s', '-c', '-p' and '-a' options. For the latter mode, each time the option appears, it\n"
      "    overrides a previous option. '-a' adds the current settings to the schedule. Any options\n"
      "    other than these five are interpreted as the end of the schedule, and are passed to run\n"
      "    and so should be valid run options.\n"
      "\n"
      "    Schedules with concurrent controllers/setters of the same type, with the same numbers of\n"
      "    parameters are interpolated between. This allows the user to set up a schedule of runs \n"
      "    where the kp/ki coefficients of the pid controller are varied, and at the same time the \n"
      "    setpoint is increased from 10 to 100 DC, for example."
      "\n"
      "    "FGBLUE"-p, --params"RESET"\n"
      "    Set the params for either the setter or controller.\n"
      "\n"
      "    "FGBLUE"-s, --setter"RESET"\n"
      "    Set the setter module name. Resets params\n"
      "\n"
      "    "FGBLUE"-c, --controller"RESET"\n"
      "    Set the controller module name. Resets params.\n"
      "\n"
      "    "FGBLUE"-c, --controller"RESET"\n"
      "    Set the controller module name.\n"
      "\n"
      "    "FGBLUE"-f, --schedule-file"RESET"\n"
      "    Load a series of schedule points (setters, controllers, parameters) from file."
      "\n"
      "    "FGBLUE"--interp-number"RESET"\n"
      "    Number of interpolation points to use.\n"
      "\n"
      "    "FGBLUE"--interp-linear"RESET"\n"
      "    Use linear interpolation.\n"
      "\n"
      "    "FGBLUE"--interp-log"RESET"\n"
      "    Use logarithmic interpolation.\n"
      "\n"
  ); // TODO: config options
}
