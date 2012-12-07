CC = gcc
CFLAGS = -Wall -g -D_REENTRANT
LDFLAGS = -lpthread

all: web_server

web_server:  util.o server.o
	${CC} -o web_server server.o util.o ${CFLAGS} ${LDFLAGS} 

server.o: server.c
	${CC} server.c -c -o server.o  ${CFLAGS} ${LDFLAGS}
	
util.o: util.h util.c
	${CC} util.c -c -o util.o  ${CFLAGS} ${LDFLAGS} 	 

.PHONY: clean

clean:
	rm -f *.o web_server

