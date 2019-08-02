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
			 obj/opt.o \
			 obj/tar.o \
			 obj/thread.o \
			 obj/util.o
WPI = wpi/libwiringPi.so


rheometer: $(RHEO)
	$(CC) $(CFLAGS) $(RHEO) -o $@ $(LINK)

debug: $(WPI) $(RHEO)
	$(CC) $(CFLAGS) $(RHEO) -o rheometer $(LINK)
	touch debug

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LINK)

wpi/libwiringPi.so: wpi/wiringPi.c wpi/wiringPi.h
	$(CC) -shared -o $@ $< -lpthread
	sudo cp -n wpi/libwiringPi.so /usr/lib/.
	sudo cp -n wpi/wiringPi.h /usr/include/.

clean:
	rm -rf obj/*.o rheometer wpi/*.so


