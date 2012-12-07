all: public_tb
	
remake: clean public_tb
	
public_tb:mm_public.o public_tb.c 
	gcc public_tb.c mm_public.o -g -o public_tb

mm_public.o: mm_public.c mm_public.h
	gcc -g -c mm_public.c

clean:
	rm -f *.o
	rm -f public_tb