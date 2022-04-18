CC = gcc
CFLAG = -ggdb
INC = -I../include
SRCD = ../src/
TESTD = ../test/

#attribute
attribute: list
	${CC} ${CFLAG} ${INC} -o obj/attribute.o -c ${SRCD}attribute.c 
attribute-clean:
	rm -f obj/attribute.o

#conn
conn: list attribute
	${CC} ${CFLAG} ${INC} -o obj/conn.o -c ${SRCD}conn.c 
conn-clean:
	rm -f obj/conn.o

#utils
utils: list strmap
list: prepare
	${CC} ${CFLAG} ${INC} -o obj/list.o -c ${SRCD}list.c
strmap: prepare
	${CC} ${CFLAG} ${INC} -o obj/strmap.o -c ${SRCD}strmap.c
utils-clean:
	rm -rf obj/list.o obj/strmap.o

#common
prepare:
	mkdir -p obj test

#test
test: test-conn test-utils
test-conn: conn utils
	${CC} ${CFLAG} ${INC} -o test/test-conn ${TESTD}src/test-conn.c obj/list.o obj/conn.o obj/attribute.o -lssh
test-utils: utils
	${CC} ${CFLAG} ${INC} -o test/test-utils ${TESTD}src/test-utils.c obj/list.o obj/strmap.o
		
