CC = gcc
CFLAG = -ggdb
INC = -I../../include

all: utils testutils.c
	${CC} ${CFLAG} ${INC} -o ../testutils testutils.c ../../obj/list.o ../../obj/strmap.o

utils:
	make -C ../../src/utils
clean:
	rm -f ../../obj/* ../testutils
prepare:
	mkdir -p ../../obj
