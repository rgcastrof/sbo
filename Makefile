CC= cc
SRC= sbo.c
PRGNAM= sbo
OBJ= ${SRC:.c=.o}
CFLAGS= -std=c99 -Wall -pedantic -O2 -D_DEFAULT_SOURCE

all: $(PRGNAM)
.c.o:
	${CC} -c ${CFLAGS} $<

$(OBJ): config.h

$(PRGNAM): ${OBJ}
	${CC} -o $@ ${OBJ}

clean:
	rm -f ${PRGNAM} ${OBJ}
