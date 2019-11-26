.PHONY: clean examples all

CC = cc
CFLAGS = -std=c99 -pedantic -Wall \
	 -Wno-deprecated-declarations

OBJ = server.o client.o
HDR = sscilib.h

all: libssci.a examples

libssci.a: ${OBJ}
	@echo AR $@
	@ar -rc $@ ${OBJ}
.c.o: ${HDR}
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

clean:
	rm libssci.a example-server example-client *.o

example-server: example-server.o server.o
	@echo LD $@
	@${CC} -o $@ example-server.o server.o

example-client: example-client.o client.o
	@echo LD $@
	@${CC} -o $@ example-client.o client.o

examples: example-server example-client
