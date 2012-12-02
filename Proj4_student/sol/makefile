cc = gcc
CFLAGS = -g
LDFLAGS = -lpthread -lcurses

all: editor

editor:
	${CC} ${CFLAGS} -o editor editor.o textbuff.o ${LDFLAGS}

clean:
	rm -rf editor
