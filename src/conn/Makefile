CC = gcc
CFLAG = -ggdb
INC = -I../../include
LIB = -lssh
OBJ = ../../obj/

sftpconn: sftpconn.c list
	${CC} ${CFLAG} ${INC} -o ${OBJ}$@ sftpconn.c ${OBJ}list.o ${LIB}

list:
	make -C ../utils
clean:
	rm -f ${OBJ}sftpconn
prepare:
	mkdir -p ${OBJ}
