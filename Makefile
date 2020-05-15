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

MAIN = obj/adc.o \
			 obj/args.o \
			 obj/cJSON.o \
			 obj/control.o \
			 obj/display.o \
			 obj/error.o \
			 obj/json.o \
			 obj/loadcell.o \
			 obj/log.o \
			 obj/main.o \
			 obj/motor.o \
			 obj/nouns.o \
			 obj/opt.o \
			 obj/photo.o \
			 obj/run.o \
			 obj/tar.o \
			 obj/thermometer.o \
			 obj/uid.o \
			 obj/util.o \
			 obj/webcam.o

HDR = src/run.h
WPI = wpi/libwiringPi.so
RHEO = $(MAIN) $(CONTROLLERS) $(SETTERS)
VERSION = $(shell scripts/get_version.sh)


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


