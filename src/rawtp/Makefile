CC = gcc
CFLAG = -ggdb -I../../include
OBJD = ../../build/obj/

all: fileoperation transfer connection byteorder list

clean:
	rm -f server client *.o

transfer: transfer.c 
	${CC} ${CFLAG} -o ${OBJD}transfer.o -c transfer.c

fileoperation: fileoperation.c 
	${CC} ${CFLAG} -o ${OBJD}fileoperation.o -c fileoperation.c

test: test.c fileoperation transfer connection byteorder misc list
	${CC} ${CFLAG} -o test test.c fileoperation.o transfer.o byteorder.o connection.o list.o

byteorder:
	${CC} ${CFLAG} -o ${OBJD}byteorder.o -c byteorder.c

connection: 
	${CC} ${CFLAG} -o ${OBJD}connection.o -c connection.c 

misc:
	echo "hello world.\nThis file is a sample for attempte of struct data transportion" > sample.txt

list:
	${CC} ${CFLAG} -o ${OBJD}listrawtp.o -c ../list.c 
