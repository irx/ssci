.PHONY: clean examples all

CC = cc
CFLAGS = -std=c99 -pedantic -Wall \
	 -Wno-deprecated-declarations \
	 -Wno-incompatible-function-pointer-types

OBJ = server.o client.o

all: libssci.a examples

libssci.a: ${OBJ}
	@echo AR $@
	@ar -rc $@ ${OBJ}
.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

clean:
	rm -f libssci.a example-server example-client *.o

example-server: example-server.o server.o
	@echo LD $@
	@${CC} -o $@ example-server.o server.o

example-client: example-client.o server.o
	@echo LD $@
	@${CC} -o $@ example-client.o client.o

examples: example-server example-client
