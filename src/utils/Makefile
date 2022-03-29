CC = gcc
CFLAG = -ggdb
INC = -I../../include

all: list.o strmap.o

list.o:
	${CC} ${CFLAG} ${INC} -o ../../tmp/list.o -c list.c

strmap.o:
	${CC} ${CFLAG} ${INC} -o ../../tmp/strmap.o -c strmap.c

clean:
	rm -rf ../../tmp/list.o ../../tmp/strmap.o
prepare:
	mkdir -p ../../obj ../../tmp
