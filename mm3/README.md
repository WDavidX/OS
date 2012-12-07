/* CSci4061 F2012 Assignment 3

section: 3
login: schm2225
date: 11/12/12
names: Aaron Schmitz, Weichao Xu
id: 3891645, 4284387 */
Files: main_malloc.c - a reference file using malloc main_mm.c - a test file for the memory manager Makefile - a file that describes how to compile the program mm_public.c - the main code for the memory manager mm_public.h - the header for the memory manager packet_public.c - the main code for the packet program packet_public.h - the header file for the packet program README - this file

Shell: To run the program from the terminal, no arguments are needed. Compile the program using make all. Execute the program by running ./packet_public.

Functionality: This program receives a number of (fake) packets via signals. The handler processes each signal and uses the custom memory manager to store the data. Once all the packets have been received, the program combines them in the proper order. The memory is then freed, the data is outputted, and the program terminates.

Responsibility: Weichao programmed the memory manager and worked on the packet handler. Aaron helped with the packet handler, performed most of the testing, and wrote the readme.
