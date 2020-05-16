# Writing Modules

A module is a plugin for the rheometer which allows for custom control and
setpoint calculation. Each module is a single source file in either of
`src/control/controllers/` or `src/control/setters`. The modules contain a
function which is called to obtain either the control action (for a controller)
or the setpoint (for a setter). These functions are passed the state of the
rheometer as input, through a structure `struct run_data`.

## `struct run_data`

The run data struct contains all of the information available to the rheometer:
stress, strainrate, loadcell, time, and so on. These values may be accessed
using the getter functions:

```c
// speed getter/setter
double get_speed(struct run_data *rd);
double get_strainrate(struct run_data *rd);
double get_fill_depth(struct run_data *rd);

// temperature getter/setter
double get_ambient_temperature(struct run_data *rd);
double get_cylinder_temperature(struct run_data *rd);

// loadcell getter/setter
unsigned long get_loadcell_bytes(struct run_data *rd);
double get_loadcell_units(struct run_data *rd);
double get_stress(struct run_data *rd);

// control getter/setter
unsigned int get_last_control_action(struct run_data *rd);
int get_is_stress_controlled(struct run_data *rd)

// time getter/setter
double get_time(struct run_data *rd);
void get_time_parts(struct run_data *rd, unsigned long *time_s, unsigned long *time_us);

// adc getter/setter
unsigned long get_adc(struct run_data *rd, int i);
```


## Controller Modules

A controller module is called to calculate how to control the motor in order to
achieve the setpoint. Motor is controlled by Pulse Width Modulation (PWM), which
changes rotation rate by Duty Cycle (DC). For the PWM on the Raspberry Pi we
have a `10-bit` number to work with: integers in the range 0-1023. If the output
from `get_control_action` is outwith this range, it is silently truncated to
within the range.


## Setter Modules

A setter module is called to get the value for setpoint desired for the
rheometer. Setpoint is either stress or strainrate, depending on the value of
`int get_is_stress_controlled(struct run_data *rd)`.


## Building

Each module is a shared object `.so` which is loaded by the rheometer executable
at run-time. In order to use the `run_data` access functions, the module source
file needs to be linked to `run.o` at build time. This is managed by the
included make file.  It is therefore recommended to place your modules in the
same directory as the included modules: `src/control/controllers` or
`src/control/setters`. Then the module can be built with 

```bash
$ make modules
```

However, if you want to build the modules otherwise, copy `src/run/run.c`,
`src/run/run.h`, and `src/control/control.h` source and headers to wherever your
module source is and build using:

```bash
$ gcc -Wall -Wextra -Werror -shared src/MODULE.c src/run.c -o MODULE.so
```

Then copy the resulting `.so` to the `controllers/` or `setters/` directory in
the rheometer repo.
