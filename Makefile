CC ?= gcc
CFLAGS = -Wall -Wextra -Werror -g
LINK = -lwiringPi -lpthread -lm -ldl

HDR = $(shell ls **/*.h)
WPI = wpi/libwiringPi.so
VERSION = $(shell scripts/get_version.sh)
PREFIX ?= "/usr"

CONTROLLERS = \
							controllers/pid.so \
							controllers/none.so

SETTERS = \
					setters/constant.so \
					setters/sine.so \
					setters/bistable.so

MODULES = $(CONTROLLERS) $(SETTERS)

ADC = \
			obj/sensors/adc/adc.o

THERMO = \
				 obj/sensors/thermometer/thermometer.o

LOADCELL = \
					 obj/sensors/loadcell/loadcell.o \
					 obj/sensors/loadcell/thread.o \
					 obj/sensors/loadcell/hx711.o

CAMERA = \
				 obj/sensors/camera/video.o \
				 obj/sensors/camera/photo.o

ENCODER = \
			obj/sensors/encoder/encoder.o

SENSORS = $(ADC) $(THERMO) $(LOADCELL) $(ENCODER) $(CAMERA)

UTIL = \
			 obj/util/cJSON.o \
			 obj/util/help.o \
			 obj/util/double_array.o \
			 obj/util/json.o \
			 obj/util/display.o \
			 obj/util/error.o \
			 obj/util/unique_name.o \
			 obj/util/time.o \
			 obj/util/sleep.o \
			 obj/util/range.o \
			 obj/util/nouns.o

CONTROL = \
					obj/control/control.o \
					obj/control/load.o \
					obj/control/help.o \
					obj/control/tuning.o

MOTOR = \
				obj/motor/motor.o

LOG = \
			obj/log/log.o \
			obj/log/tar.o

RUN = \
			obj/run/run.o \
			obj/run/free.o \
			obj/run/defaults.o

SCHEDULE = \
					 obj/schedule/schedule.o \
					 obj/schedule/argset.o \
					 obj/schedule/generate.o

MAIN = \
			 obj/main/main.o \
			 obj/main/run.o \
			 obj/main/schedule.o \
			 obj/main/config.o

ARGS = \
			 obj/args/args.o \
			 obj/args/help.o \
			 obj/args/run.o \
			 obj/args/schedule.o \
			 obj/args/config.o

RHEO = $(MAIN) \
			 $(ARGS) \
			 $(RUN) \
			 $(LOG) \
			 $(MOTOR) \
			 $(CONTROL) \
			 $(UTIL) \
			 $(SENSORS) \
			 $(SCHEDULE)

## Colours
COL_OBJ = $(shell tput setaf 3 2>/dev/null)
COL_EXE = $(shell tput setaf 4 2>/dev/null)
COL_SO = $(shell tput setaf 5 2>/dev/null)
COL_RST = $(shell tput sgr0 2>/dev/null)
COL_BLD = $(shell tput bold 2>/dev/null)

.PHONY: all modules

all: rheometer modules

rheometer: $(RHEO) $(HDR)
	@printf "$(COL_OBJ)LINKING OBJECTS TO EXECUTABLE $@$(COL_RST)\n"
	$(CC) $(CFLAGS) $(RHEO) -o $@ $(LINK)

modules: $(shell scripts/get_modules.sh)

wpi: $(WPI)

debug: wpi rheometer
	touch debug

controllers/%.so: obj/control/controllers/%.o obj/run/run.o
	@printf "$(COL_SO)ASSEMBLING MODULE $@ (CONTROLLER)$(COL_RST)\n"
	@mkdir -p `dirname $@`
	@$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

setters/%.so: obj/control/setters/%.o obj/run/run.o
	@printf "$(COL_SO)ASSEMBLING MODULE $@ (SETTER)$(COL_RST)\n"
	@mkdir -p `dirname $@`
	@$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

obj/%.o: src/%.c $(HDR)
	@printf "$(COL_OBJ)ASSEMBLING OBJECT $@$(COL_RST)\n"
	@mkdir -p `dirname $@`
	@$(CC) $(CFLAGS) -c $< -o $@ -DVERSION=\"$(VERSION)\"

wpi/libwiringPi.so: wpi/wiringPi.c wpi/wiringPi.h
	@printf "$(COL_SO)ASSEMBLING MODULE $@ (DUMMY)$(COL_RST)\n"
	@$(CC) -shared -o $@ $< -lpthread
	@sudo cp wpi/libwiringPi.so $(PREFIX)/lib/.
	@sudo cp wpi/*.h $(PREFIX)/include/.

clean:
	rm -rf obj/* rheometer wpi/*.so controllers/*.so setters/*.so
