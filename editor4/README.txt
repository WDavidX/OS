/* CSci4061 F2012 Assignment 4
* section: 3
* login: schm2225
* date: 12/3/12
* names: Aaron Schmitz, Weichao Xu
* id: 3891645, 4284387
*/

Files:
textbuff.h: the header file that defines the struct and functions used in the text buffer.
textbuf.c: the implementation the requried functions for the text buffer.
editor.c:  the main program. It has multiple threads for UI, autosaving and command handling.
Makefile:  makefile for program compilation. Text buffer is compiled into an object file first then linked into the editor.
README.txt: this file.

Shell: to compile this file, type $ Make in the termianl; to execute the prgoram, type $ ./editor {file_name}

Purpose: This programs clones the basic functionality of a text editor Vim. The exact program behavior follows the sample executable given by the TA.

Functionality: File textbuff(.h,.c) defined the struct for each line and functions associated withit, main file editor.c are implemented with three threads: main thread reads messages and acts accordingly; UI thread generates commands sent to main thread; auto-save thread saves the text buffer periodically.

Responsibility: Weichao started this lab, helped the editor and wrote the README. Aaron programmed the main part of this lab text buffer and the skeleton of the editor.

