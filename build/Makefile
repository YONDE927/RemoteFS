CC = gcc
CFLAG = -ggdb
INC = -I../include
SRCD = ../src/
TESTD = ../test/

#remotefs
all: conn utils test
	${CC} ${CFLAG} ${INC} -o RemoteFS ${SRCD}remotefs.c obj/entry.o obj/conn.o obj/list.o obj/map.o -lssh -lfuse

#attribute
attribute: list
	${CC} ${CFLAG} ${INC} -o obj/entry.o -c ${SRCD}entry.c 
attribute-clean:
	rm -f obj/entry.o

#conn
conn: list attribute
	${CC} ${CFLAG} ${INC} -o obj/conn.o -c ${SRCD}conn.c 
conn-clean:
	rm -f obj/conn.o

#utils
utils: list map
list: prepare
	${CC} ${CFLAG} ${INC} -o obj/list.o -c ${SRCD}list.c
map: prepare
	${CC} ${CFLAG} ${INC} -o obj/map.o -c ${SRCD}map.c
utils-clean:
	rm -rf obj/list.o obj/map.o

#common
prepare:
	mkdir -p obj test

#test
test: test-conn test-utils test-clean
	echo "Hello connRead test." > /tmp/connRead.txt
	touch /tmp/connWrite.txt
test-conn: conn utils
	${CC} ${CFLAG} ${INC} -o test/test-conn ${TESTD}test-conn.c obj/list.o obj/conn.o obj/attribute.o -lssh
test-utils: utils
	${CC} ${CFLAG} ${INC} -o test/test-utils ${TESTD}test-utils.c obj/list.o obj/map.o
test-clean:
	rm -f /tmp/connRead.txt /tmp/connWrite.txt
		
