# makefile for nauty 2.2
# Stripped down to bare essential components for use in nibac.
# Modified by Sebastian Raaphorst, 2018.

# Generated automatically from makefile.in by configure.

SHELL = /bin/sh
CC=gcc
CFLAGS= -O4 
SAFECFLAGS= -O4
LDFLAGS=
LOK=1         # 0 if no 64-bit integers
TESTPROGS=dreadtest dreadtestB dreadtestS dreadtestS1 dreadtest4K dreadtest1 dreadtestW1 dreadtestL1 dreadtestL
BIGTEST=1         # set to 0 if less than 512MB is available

SMALL=-DMAXN=WORDSIZE
BIG=-DBIGNAUTY
L1=-DMAXN=WORDSIZE -DWORDSIZE=64
W1=-DMAXN=WORDSIZE -DWORDSIZE=32
S1=-DMAXN=WORDSIZE -DWORDSIZE=16
S=-DWORDSIZE=16
L=-DWORDSIZE=64
FOURK=-DMAXN=4096

all : nautil.o naugraph.o nauty.o

nautil.o: nauty.h nautil.c
	${CC} -c ${CFLAGS} nautil.c
nauty.o: nauty.h nauty.c
	${CC} -c ${CFLAGS} nauty.c
naugraph.o: nauty.h naugraph.c
	${CC} -c ${CFLAGS} naugraph.c

clean:
	rm -f *.o config.log config.cache config.status


install:
	mkdir -p /Users/vorpal/Development/C++/nibac/extern/lib
	mkdir -p /Users/vorpal/Development/C++/nibac/extern/include
	cp nautil.o naugraph.o nauty.o /Users/vorpal/Development/C++/nibac/extern/lib
	cp nauty.h /Users/vorpal/Development/C++/nibac/extern/include
