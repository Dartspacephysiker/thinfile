CC = gcc
CFLAGS = -std=gnu99 -pipe -ggdb -O2 -Wall -D_FILE_OFFSET_BITS=64  -D_LARGEFILE64_SOURCE
LDFLAGS = -pipe -Wall -Wno-unused-result 
LDLIBS =

SOURCES = thinfile.c make_example_thinfile.c
OBJECTS = $(SOURCES:.c=.o)
EXECS = thinfile make_example_thinfile

all: $(SOURCES) $(EXECS)

thinfile: thinfile.o
	$(CC) -o $@ $(LDFLAGS) $^ $(LDLIBS)

make_example_thinfile: make_example_thinfile.o
	$(CC) -o $@ $(LDFLAGS) $^ $(LDLIBS)

.c.o:
	$(CC) -o $@ $(CFLAGS) -c $<

clean:
	rm *.o $(EXECS)
