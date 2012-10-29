# Last modified by Weichao
# Last modified date: Oct 16, 2012
# Note:
# 1 In the $(OBJ), the $(SOURCE) if put right after the source file name to
# 2 Added another dependence wrapper.o so that the wrapper is not compiled everytime
# 3 Added some screen echoing message for wrapper.o
# 4 clean both brower and .o files
# 5 In ubuntu, following packages are needed with apt-get
#    libgtc2.0-dev, libwebkitgtc*, etc..

CC=gcc
CFLAGS=-g -Wall `pkg-config --cflags gtk+-2.0 webkit-1.0`
LDFLAGS+=`pkg-config --libs gtk+-2.0 webkit-1.0`
LIB32=/usr/lib
LIB64=/usr/lib
#LIB32=/soft/webkit/1.1.15.4/jaunty32/lib
#LIB64=/soft/webkit/1.1.15.4/jaunty64/lib
SOURCES=wrapper.c wrapper.h browser.c
OBJ=browser

LBITS := $(shell getconf LONG_BIT)
ifeq ($(LBITS),64)
   LIB=${LIB64}
else
   LIB=${LIB32}
endif

all:  $(SOURCES) $(OBJ)

$(OBJ): browser.c wrapper.o
	$(CC) browser.c wrapper.o -L ${LIB} $(CFLAGS) $(LDFLAGS)  -o $(OBJ)

wrapper.o: wrapper.c wrapper.h
	@echo "\n==================== Compiling the wrapper ===================="
	$(CC) wrapper.c wrapper.h -c -O2 -L ${LIB} $(CFLAGS) $(LDFLAGS)
	@echo "==================== End compiling the wrapper ====================\n"

clean:
	rm -rf $(OBJ)
	rm -rf *.o
