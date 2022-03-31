CC = gcc
CFLAG = -ggdb
INC = -I../../include

all: list.o strmap.o

list.o:
	${CC} ${CFLAG} ${INC} -o ../../obj/list.o -c list.c

strmap.o:
	${CC} ${CFLAG} ${INC} -o ../../obj/strmap.o -c strmap.c

clean:
	rm -rf ../../obj/list.o ../../obj/strmap.o
prepare:
	mkdir -p ../../obj ../../tmp
