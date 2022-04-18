CC = gcc
CFLAG = -ggdb
INC = -I../include
SRCD = ../src/

#conn
conn: list
	${CC} ${CFLAG} ${INC} -o obj/conn.o ${SRCD}conn.c obj/list.o -lssh
conn-clean:
	rm -f ${OBJ}sftpconn
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
	mkdir -p obj
