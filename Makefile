CC = cc
CFLAGS = -std=c99 -pedantic -Wall \
	 -Wno-deprecated-declarations \
	 -Wno-incompatible-function-pointer-types

OBJ = server.o client.o

ssci.a: ${OBJ}
	@echo AR $@
	ar -rc $@ ${OBJ}
.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<
