CC=gcc
CFLAGS=-Ofast -ggdb
CLIBS=-lSDL2

INSTALLDIR=/usr/local
BINDIR=${INSTALLDIR}/bin/
CONFIGDIR=~/

EXECUTABLE=pong
CONFIG_FILE=pong.cfg

OBJECTS= build/pong.o \
		 build/draw.o \
		 build/main_sdl.o

all: $(EXECUTABLE)

build/%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(CLIBS)

install: $(EXECUTABLE)
	install $(EXECUTABLE) $(BINDIR)
	install $(CONFIG_FILE) $(CONFIGDIR)

clean:
	rm $(OBJECTS) $(EXECUTABLE)