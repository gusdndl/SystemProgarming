CC=gcc
CFLAGS=-pthread -o
LIBS=-lwiringPi

all: client inner_server outer_server

client: client.c
	$(CC) $(CFLAGS) $@ $^ $(LIBS)

inner_server: inner_server.c
	$(CC) $(CFLAGS) $@ $^ $(LIBS)

outer_server: outer_server.c
	$(CC) $(CFLAGS) $@ $^ $(LIBS)

clean:
	rm -f client inner_server outer_server

install:
	git clone https://github.com/WiringPi/WiringPi.git

.PHONY: all clean install