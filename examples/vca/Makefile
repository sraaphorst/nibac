# Makefile
#
# By Sebastian Raaphorst, 2004.

# Paths to various installs
# *** Modify these to suit your local configuration. ***
NIBACHEADERPATH=$(HOME)/include/nibac
NIBACLIBRARYPATH=$(HOME)/lib

# Tools and necessary flags
CC=g++
LD=g++
CFLAGS=-O3 -Wall -ansi -DNDEBUG -DNODEGROUPS
#CFLAGS=-g -Wall -ansi -DDEBUG -DNODEGROUPS
INCLUDES=-I$(NIBACHEADERPATH)
LIBS=-L$(NIBACLIBRARYPATH)

# Binaries created by this library.
BINS=\
	VCA

all:				$(BINS)


clean:
				rm -rf *~ $(BINS)

ASC.o:				ASC.h ASC.cpp
				$(CC) $(CFLAGS) $(INCLUDES) -o ASC.o -c ASC.cpp

VCA:				VCA.h VCA.cpp
				$(CC) $(CFLAGS) $(INCLUDES) -o VCA VCA.cpp ASC.o $(LIBS) -lnibac
