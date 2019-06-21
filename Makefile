CC = gcc
CFLAGS = -Wall -pedantic
LINK = -lwiringPi -lpthread
HDR = src/rheo.h
RHEO = obj/main.o obj/adc.o obj/error.o obj/thread.o obj/args.o obj/log.o obj/control.o obj/motor.o obj/opt.o


rheometer: $(RHEO) $(HDR)
	$(CC) $(CFLAGS) $(RHEO) -o $@ $(LINK)

obj/%.o: src/%.c obj $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@ $(LINK)

obj:
	mkdir obj

clean:
	rm -rf obj rheometer


