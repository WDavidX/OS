#all:
#	gcc -g main_malloc.c mm_public.c -o main_malloc
#	gcc -g main_mm.c mm_public.c -o main_mm
#	gcc -g packet_public.c mm_public.c -o packet_public

all: mm packet

mm: mm_public.o
	gcc -g main_mm.c mm_public.o -o main_mm
	gcc -g main_malloc.c mm_public.o -o main_malloc

packet: mm_public.o
	gcc -g packet_public.c mm_public.o -o packet_public

mm_public.o:
	gcc -g -c mm_public.c
