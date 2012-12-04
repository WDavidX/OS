CC=gcc
cflags= -Wall -g -D_REENTRANT -lpthread -lcurses

all: editor

editor: textbuff.o editor.c
	@echo "\nCompiling editor"
	$(CC) editor.c textbuff.o  $(cflags) -o editor
	

textbuff.o: textbuff.c textbuff.h
	@echo "\nCompiling textbuff"
	$(CC) -c textbuff.c  -o textbuff.o $(cflags)

.PHONY: clean

clean:
	rm -f *.o *.gch

