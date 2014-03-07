CC=gcc
CFLAGS=-Wall -fPIC
INCDIRS=-I./ -I../DataCollector
LIBS=../DataCollector/libDC.a

OBJS=jmodule.o jnetbuf.o jpeer.o jconfig.o jutilities.o jframework.o jhandler.o

all: ${OBJS}

.c.o:
	${CC} ${CFLAGS} ${INCDIRS} -c $< -o $@

clean:
	rm -rf ${OBJS}
