# rheometer controller

This repo contains the source for the rheometer controller software for a new,
flexible, low-cost, open-source rheometer. The rheometer is designed for the
investigation of dense suspensions, but due to the nature of the rheometer,
it is able to change to meet the needs of the researcher.

The control software is a simple command line application for setting up runs,
controlling flow, and data acquisition.


# Installation

The software is intended to be installed on a Raspberry Pi computer, with any
(GNU) Linux operating system.

Clone this repo somewhere, then build with make:

```bash
$ make
```

That's it! You can run the software from the repo with:
```bash
$ ./rheometer <options>
```

Check out more information with
```bash
$ ./rheometer --help | less
```


# Control schemes

Control schemes are JSON files which specific the control module and setter
combo to use in a run, along with any parameters the modules require. These
small text files are stored in the `data/` directory. Having these files helps
ensure consistency between runs.

Control schemes are JSON files and so look like this:

```JSON
{
  "control":"pid",
  "control_params": [0.1, 0.001, 0.0],
  "setter":"constant",
  "setter_params": [5]
}
```

`"control"` specifies the name of a control module, `"setter"` a setter module.
`"_params"` are arrays of double which are available to the control/setter
modules to calculate the control action. See the documentation for each module
(`const char *doc` in the module source `src/control/controllers/*.c`, 
`src/control/setters/*.c`, or see `./rheometer --help`).


# Control modules

The rheometer software is designed for speed, but also flexibility. Control
modules are shared objects which dynamically link in to the program when you
start it. This just means a control module is a piece of a program that can be
loaded on the fly or modified without modifying the whole program.

You can write a control module very easily:

```c
#include "../../run.h"
#include "../control.h"

const char *doc = "write a descriptive comment here: what params are necessary etc.";

unsigned int get_control_action(struct run_data *rd)
{
  // do some calculation with rd, a struct containing the current run
  // information: speed, stress, temperature, torque etc
  return 3.14; // new control action
}
```

To get access to the accessor functions for the run data (get_speed(rd) etc),
the shared object needs to link with the run.o object. To facilitate this, its
easiest to put your custom modules in the same folder as the default ons. Put
your new module source file in the `src/control/controllers/` directory and build
with:

```bash
make modules
```

Then you can create a controlscheme JSON in the `data/` directory to make use of
your new controller!


# Setter modules

Similar to control modules, you can write a custom setter to recalculate the
setpoint for the controller over the course of the experiment. The format is
very similar, but it should be placed in the `src/control/setters/` directory:

```c
#include "../../run.h"
#include "../control.h"

const char *doc = "write a descriptive comment here.";

double get_setpoint(struct run_data *rd)
{
  // do some calculation with rd, a struct containing the current run
  // information: speed, stress, temperature, torque etc
  return 3.14; // new setpoint
}
```
