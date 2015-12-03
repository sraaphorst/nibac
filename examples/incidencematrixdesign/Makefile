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
	incidencematrixdesign

all:				$(BINS)


clean:
				rm -rf *~ $(BINS)

incidencematrixdesign:		incidencematrixdesign.h incidencematrixdesign.cpp
				$(CC) $(CFLAGS) $(INCLUDES) -o incidencematrixdesign incidencematrixdesign.cpp $(LIBS) -lnibac
