CC = gcc
CFLAGS = -Wall -pedantic
LINK = -lwiringPi -lpthread
HDR = src/rheo.h
RHEO = obj/adc.o \
			 obj/args.o \
			 obj/cJSON.o \
			 obj/control.o \
			 obj/display.o \
			 obj/error.o \
			 obj/log.o \
			 obj/main.o \
			 obj/motor.o \
			 obj/opt.o \
			 obj/tar.o \
			 obj/thread.o
WPI = wpi/libwiringPi.so


rheometer: $(RHEO) $(HDR)
	$(CC) $(CFLAGS) $(RHEO) -o $@ $(LINK)

debug: $(WPI) $(RHEO) $(HDR)
	$(CC) $(CFLAGS) $(RHEO) -o rheometer $(LINK)
	touch debug

obj/%.o: src/%.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@ $(LINK)

wpi/libwiringPi.so: wpi/wiringPi.c wpi/wiringPi.h
	$(CC) $(CFLAGS) -shared -o $@ $< -lpthread
	sudo cp -n wpi/libwiringPi.so /usr/lib/.
	sudo cp -n wpi/wiringPi.h /usr/include/.

clean:
	rm -rf obj rheometer wpi/*.so


