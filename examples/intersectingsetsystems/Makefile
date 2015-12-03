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

# Object files created by this library.
OBJS=\
	colexicographicvariableorder.o

# Binaries created by this library.
BINS=\
	intersectingsetsystem

all:				$(OBJS) $(BINS)


clean:
				rm -rf *~ $(OBJS) $(BINS)

intersectingsetsystem:		intersectingsetsystem.h intersectingsetsystem.cpp $(OBJS)
				$(CC) $(CFLAGS) $(INCLUDES) -o intersectingsetsystem intersectingsetsystem.cpp $(OBJS) $(LIBS) -lnibac

colexicographicvariableorder.o:	colexicographicvariableorder.h\
				colexicographicvariableorder.cpp
				$(CC) $(CFLAGS) $(INCLUDES) -o colexicographicvariableorder.o -c colexicographicvariableorder.cpp
