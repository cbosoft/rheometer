CC = gcc
CFLAGS = -Wall -Wextra
LINK = -lwiringPi -lpthread -lm
RHEO = obj/adc.o \
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
			 obj/run.o \
			 obj/tar.o \
			 obj/thermometer.o \
			 obj/uid.o \
WPI = wpi/libwiringPi.so


rheometer: $(RHEO)
	$(CC) $(CFLAGS) $(RHEO) -o $@ $(LINK)

wpi: $(WPI)

debug: wpi rheometer
	touch debug

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LINK)

wpi/libwiringPi.so: wpi/wiringPi.c wpi/wiringPi.h
	$(CC) -shared -o $@ $< -lpthread
	sudo cp wpi/libwiringPi.so /usr/lib/.
	sudo cp wpi/wiringPi.h /usr/include/.

clean:
	rm -rf obj/*.o rheometer wpi/*.so


