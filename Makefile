CC = gcc
CFLAGS = -Wall -Wextra
LINK = -lwiringPi -lpthread -lm -ldl

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
					 obj/sensors/loadcell/loadcell.o

CAMERA = \
				 obj/sensors/camera/video.o \
				 obj/sensors/camera/photo.o

ENCODER = \
			obj/sensors/encoder/encoder.o

SENSORS = $(ADC) $(THERMO) $(LOADCELL) $(ENCODER) $(CAMERA)

UTIL = \
			 obj/util/cJSON.o \
			 obj/util/json.o \
			 obj/util/display.o \
			 obj/util/error.o \
			 obj/util/unique_name.o \
			 obj/util/time.o \
			 obj/util/sleep.o \
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
			obj/run/defaults.o

RHEO = obj/main.o obj/args.o \
			 $(RUN) \
			 $(LOG) \
			 $(MOTOR) \
			 $(CONTROL) \
			 $(UTIL) \
			 $(SENSORS)

HDR = $(shell ls **/*.h)
WPI = wpi/libwiringPi.so
VERSION = $(shell scripts/get_version.sh)

.PHONY: all

all: rheometer $(MODULES)

rheometer: $(RHEO) $(HDR)
	$(CC) $(CFLAGS) $(RHEO) -o $@ $(LINK)
	touch src/args.c

wpi: $(WPI)

debug: wpi rheometer
	touch debug

controllers/%.so: obj/controllers/%.o
	$(CC) $(CFLAGS) -shared $< -o $@

setters/%.so: obj/setters/%.o
	$(CC) $(CFLAGS) -shared $< -o $@

obj/%.o: src/%.c $(HDR)
	mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -c $< -o $@ -DVERSION=\"$(VERSION)\"

wpi/libwiringPi.so: wpi/wiringPi.c wpi/wiringPi.h
	$(CC) -shared -o $@ $< -lpthread
	sudo cp wpi/libwiringPi.so /usr/lib/.
	sudo cp wpi/*.h /usr/include/.

clean:
	rm -rf obj/*.o rheometer wpi/*.so


