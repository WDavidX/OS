#all:
#	gcc -g main_malloc.c mm_public.c -o main_malloc
#	gcc -g main_mm.c mm_public.c -o main_mm
#	gcc -g packet_public.c mm_public.c -o packet_public

all:   packet

mm: mm_public.o main_mm.c main_malloc.c
	gcc -g main_mm.c mm_public.o -o main_mm
	gcc -g main_malloc.c mm_public.o -o main_malloc

packet: mm_public.o packet_public.c packet_public.h
	gcc -g packet_public.c mm_public.o -o packet_public

mm_public.o: mm_public.c  mm_public.h
	gcc -g -c mm_public.c
	
clean:
	rm -f *.o
	rm -f main_malloc
	rm -f packet_public
	rm -f main_mm
