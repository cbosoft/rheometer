CC = gcc
CFLAGS = -Wall -pedantic
LINK = -lwiringPi -lpthread
HDR = src/rheo.h
RHEO = obj/main.o \
			 obj/adc.o \
			 obj/error.o \
			 obj/thread.o \
			 obj/args.o \
			 obj/log.o \
			 obj/control.o \
			 obj/motor.o \
			 obj/opt.o \
			 obj/tar.o \
			 obj/cJSON.o
WPI = wpi/libwiringPi.so


rheometer: $(RHEO) $(HDR)
	$(CC) $(CFLAGS) $(RHEO) -o $@ $(LINK)

debug: $(WPI) $(RHEO) $(HDR)
	$(CC) $(CFLAGS) $(RHEO) -o rheometer $(LINK)

obj/%.o: src/%.c obj $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@ $(LINK)

obj:
	mkdir obj

wpi/libwiringPi.so: wpi/wiringPi.c wpi/wiringPi.h
	$(CC) $(CFLAGS) -shared -o $@ $< -lpthread
	sudo cp wpi/libwiringPi.so /usr/lib/.
	sudo cp wpi/wiringPi.h /usr/include/.

clean:
	rm -rf obj rheometer wpi/*.so


