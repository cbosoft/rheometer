# rheometer controller

This repo contains the source for the rheometer controller software for a new,
flexible, low-cost, open-source rheometer. The rheometer is designed for the
investigation of dense suspensions, but due to the nature of the rheometer,
it is able to change to meet the needs of the researcher.

The control software is a simple command line application for setting up runs,
controlling flow, and data acquisition.

The rheometer software is designed for speed, but also flexibility. Control
modules are shared objects which dynamically link in to the program when you
start it. This just means a control module is a piece of a program that can be
loaded on the fly or modified without modifying the whole program. This is to
facilitate investigation of control algorithms for use with dense suspensions.


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

There are a few modules included as default ("none", and "pid" for controller,
"constant", "sine", "bistable" for setter modules), but you can write a custom
controller/setter very easily.

Information on writing control modules is documented [here](writing_modules.md).
