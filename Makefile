CC = gcc
CFLAGS = -Wall -pedantic
LINK = -lpthread
HDR = src/rheo.h
RHEO = obj/rheo.o obj/rheo_adc.o obj/rheo_error.o obj/rheo_thread.o obj/rheo_args.o


rheometer: $(RHEO) $(HDR)
	$(CC) $(CFLAGS) $(RHEO) -o $@ $(LINK)

obj/%.o: src/%.c obj $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@ $(LINK)

obj:
	mkdir obj

clean:
	rm -rf obj rheometer


