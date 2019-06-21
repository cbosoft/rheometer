CC = gcc
CFLAGS = -Wall -pedantic
LINK = -lpthread
HDR = src/rheo.h
RHEO = src/rheo.o src/rheo_adc.o src/rheo_error.o src/rheo_thread.o src/rheo_args.o

src/%.o: src/%.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@ $(LINK)

rheometer: $(RHEO)
	$(CC) $(CFLAGS) $(RHEO) -o $@ $(LINK)

clean:
	rm -rf src/*.o rheometer


