CC = gcc
CFLAG = -ggdb
INC = -I../include
SRCD = ../src/
RAWFLAG = 
TESTD = ../test/

#remotefs
all: conn utils test mirror record rawtp
	${CC} ${CFLAG} ${INC} ${RAWFLAG} -o RemoteFS ${SRCD}remotefs.c obj/entry.o obj/conn.o obj/list.o obj/map.o obj/mirror.o obj/record.o obj/fileoperation.o obj/transfer.o obj/byteorder.o obj/connection.o -lssh -lfuse3 -lpthread -lsqlite3

#entry
entry: list
	${CC} ${CFLAG} ${INC} -o obj/entry.o -c ${SRCD}entry.c 
entry-clean:
	rm -f obj/entry.o

#conn
conn: list entry rawtp
	${CC} ${CFLAG} ${INC} -o obj/conn.o -c ${SRCD}conn.c 
conn-clean:
	rm -f obj/conn.o

#rawtp
rawtp: 
	make -C ../src/rawtp all

#cache
cache: utils
	${CC} ${CFLAG} ${INC} -o obj/cache ${SRCD}cache.c obj/list.o obj/map.o
cache-clean:
	rm -f obj/cache

#mirror
mirror: utils conn
	${CC} ${CFLAG} ${INC} -o obj/mirror.o -c ${SRCD}mirror.c
mirror-clean:
	rm -f obj/cache

#record
record: utils conn
	${CC} ${CFLAG} ${INC} -o obj/record.o -c ${SRCD}record.c
record-clean:
	rm -f obj/record.o

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
test: test-conn test-utils test-clean test-mirror
	echo "Hello connRead test." > /tmp/connRead.txt
	touch /tmp/connWrite.txt
test-conn: conn utils rawtp
	${CC} ${CFLAG} ${INC} -o test/test-conn ${TESTD}test-conn.c obj/list.o obj/conn.o obj/entry.o obj/fileoperation.o obj/transfer.o obj/connection.o obj/byteorder.o  -lssh -lpthread
#test-utils: utils
#	${CC} ${CFLAG} ${INC} -o test/test-utils ${TESTD}test-utils.c obj/list.o obj/map.o
#test-mirror: mirror
#	${CC} ${CFLAG} ${INC} -o test/test-mirror ${TESTD}test-mirror.c obj/mirror.o obj/list.o obj/conn.o obj/entry.o obj/fileoperation.o obj/transfer.o obj/connection.o obj/byteorder.o -lssh -lpthread -lsqlite3

test-clean:
	rm -f /tmp/connRead.txt /tmp/connWrite.txt
		
