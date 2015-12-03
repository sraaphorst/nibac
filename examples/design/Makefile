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
	colexicographicvariableorder.o \
	specialcliquecutproducer.o

# Binaries created by this library.
BINS=\
	design

all:				$(OBJS) $(BINS)


clean:
				rm -rf *~ $(OBJS) $(BINS)

design:				design.h design.cpp $(OBJS)
				$(CC) $(CFLAGS) $(INCLUDES) -o design design.cpp $(OBJS) $(LIBS) -lnibac

colexicographicvariableorder.o:	colexicographicvariableorder.h\
				colexicographicvariableorder.cpp
				$(CC) $(CFLAGS) $(INCLUDES) -o colexicographicvariableorder.o -c colexicographicvariableorder.cpp

specialcliquecutproducer.o:	specialcliquecutproducer.h\
				specialcliquecutproducer.cpp
				$(CC) $(CFLAGS) $(INCLUDES) -o specialcliquecutproducer.o -c specialcliquecutproducer.cpp
