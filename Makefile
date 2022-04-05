.POSIX:
.PHONY: clean examples all install

PREFIX = /usr/local

CC = cc
CFLAGS = -std=c99 -pedantic -Wall \
	 -Wno-deprecated-declarations

OBJ = server.o client.o
HDR = sscilib.h

EXAMPLES = \
	example-client \
	example-http \
	example-server

all: libssci.a examples

install: libssci.a
	@echo installing to ${PREFIX}
	mkdir -p ${PREFIX}/lib
	cp libssci.a ${PREFIX}/lib/libssci.a
	mkdir -p ${PREFIX}/include
	cp sscilib.h ${PREFIX}/include/sscilib.h

libssci.a: ${OBJ}
	@echo AR $@
	@ar -rc $@ ${OBJ}

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

clean:
	rm -f libssci.a ${EXAMPLES} *.o

example-server: example-server.o server.o
	@echo LD $@
	@${CC} -o $@ example-server.o server.o

example-client: example-client.o client.o
	@echo LD $@
	@${CC} -o $@ example-client.o client.o

example-http: example-http.o server.o
	@echo LD $@
	@${CC} -o $@ example-http.o server.o

examples: ${EXAMPLES}
