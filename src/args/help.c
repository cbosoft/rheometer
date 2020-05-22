#include <stdlib.h>
#include <stdio.h>

#include "../util/display.h"
#include "../log/tag.h"
#include "../control/control.h"
#include "../version.h"

#include "args.h"

void usage(void)
{
  fprintf(stdout, 
      "\n"
      "  "BOLD"rheometer"RESET" control program v"VERSION"\n"
      "\n"
      "  "BOLD"Usage:"RESET"\n"
      "    rheometer [common options] [command] [command options]\n"
      "    rheometer [run] -l <length> -d <fill-depth> -c <control scheme> -w <hardware version> [-t <tag>] [-n <needle-depth>] [--calm-start] [-v <video-dev>] [-p <photo-dev>]\n"
      "    rheometer config (add|edit|remove) (controlscheme|calibration|schedule)\n"
      "    rheometer schedule <schedule-file>\n"
      "    rheometer -h|--help\n"
      "\n"
  );
}




void args_help()
{
  fprintf(stdout,
      "  "BOLD"Common Options:"RESET"\n"
      "    --quiet          suppress most output; progress is given by a bar.\n"
      "\n"
      "  "BOLD"Run Options:"RESET"\n"
      "    -l | --length    Length of run, can be given in seconds or minutes, dictated\n"
      "                     by a suffixed character. e.g. \"10s\" is 10 seconds, \"10m\"\n"
      "                     is 10 minutes. If ignored, the value is taken as seconds.\n"
      "\n"
      "    -d | --fill-depth     Fill depth of Couette cell. This is the height fluid extends \n"
      "                     up on the "BOLD"inner"RESET" cylinder, in mm. Total height of inner \n"
      "                     cylinder is ~73mm.\n"
      "\n"
      "    -d | --fill-depth     Needle depth in the Couette cell. This is the height needle is in \n"
      "                     fluid, in mm. Total height of needle is ~37mm.\n"
      "\n"
      "    -c | --control-scheme    Control scheme JSON file path. More information in the\n"
      "                     'Control Schemes' section in the help.\n"
      "\n"
      "    -a | --loadcell-calibration    Selects an alternate loadcell calibration name from \n"
      "                     \"data/loadcell_calibration.json\"."
      "\n"
      "    -w | --hardware-version  Hardware version specifying when the hardware was last\n"
      "                     changed. This should be given as a ISO8601 date spec without \n"
      "                     separating dashes (YYYYMMDD).\n"
      "\n"
      "    -t | --tag       A short descriptive name for the test run. Underscores and spaces\n"
      "                     will be replaced by hyphens. Optional. Default is \""TAGDEFAULT"\".\n"
      "\n"
      "    --calm-start     Enable this flag to remove the high power motor start up. This is\n"
      "                     useful for tuning position of the outer cylinder for very thick or\n"
      "                     non-Newtonian fluids. However, the motor is more likely to stall on \n"
      "                     start, so be wary about doing tests with this data.\n"
      "\n"
      "    -v | --video-device    This argument informs the location of the video device to capture\n"
      "                     from during logging. By setting this option, video logging is enabled.\n"
      "                     A video of the name \"${prefix}_video.mp4\" will be created and stored\n"
      "                     alongside the other log files.\n"
      "\n"
      "    -p | --photo-device    This argument informs the location of the video device to capture\n"
      "                     a still photo from before the run begins. By setting this option, a photo\n"
      "                     of the name \"${prefix}_photo.mp4\" will be created and stored\n"
      "                     alongside the other log files, taken before the logging begins.\n"
      "\n"
  ); // TODO: config and schedule options
  control_help();
}
