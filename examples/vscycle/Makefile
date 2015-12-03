# Makefile
#
# By Sebastian Raaphorst, 2004.
#
# $Author$
# $Date$

CC=CC

# Location of CPLEX
CPLEXBASEDIR=/local/comb/sfw/sparc_SunOS_5.9/ilog

# CPLEX 7.x includes
CPLEXCINCLUDES=$(CPLEXBASEDIR)/cplex70/include
CPLEXCPPINCLUDES=$(CPLEXBASEDIR)/concert10/include
CPLEXCLIBS=$(CPLEXBASEDIR)/cplex70/lib/ultrasparc_5_5.0/static_pic_mt
CPLEXCPPLIBS=$(CPLEXBASEDIR)/concert10/lib/ultrasparc_5_5.0/static_pic_mt

# CPLEX 8.x includes
#CPLEXCINCLUDES=$(CPLEXBASEDIR)/cplex81/include
#CPLEXCPPINCLUDES=$(CPLEXBASEDIR)/concert13/include
#CPLEXCLIBS=$(CPLEXBASEDIR)/cplex81/lib/ultrasparc_5_5.0/static_pic_mt
#CPLEXCPPLIBS=$(CPLEXBASEDIR)/concert13/lib/ultrasparc_5_5.0/static_pic_mt

# Location of nauty
NAUTYBASEDIR=../../nauty22

# nauty includes
NAUTYINCLUDES=$(NAUTYBASEDIR)
NAUTYLIBS=$(NAUTYBASEDIR)

# BACLIBRARY includes
BACLIBRARYINCLUDES=../../src
BACLIBRARYLIBS=../../src

# The first set of CFLAGS are for debugging, and the second set are
# for release.
# Here is a summary of the compile time options:
# DEBUG
#	outputs debugging information to the standard error channel
# CPLEX8
#	indicates that we are using CPLEX version 8.x (default is 7.x)
# SCHREIERSIMSTECHNIQUE
#	this flag indicates that we wish to determine
#	the symmetry group using the included permutation backtracking
#	method (as opposed to simply stating it explicitly in terms of
#	its generators)
CFLAGS=-fast -DNDEBUG -DNODEGROUPS -DSCHREIERSIMSTECHNIQUE
#CFLAGS=-g -DDEBUG -xcheck=%all +w -DNODEGROUPS -DSCHREIERSIMSTECHNIQUE



LIBS=\
	vscycle.o

PROGS=\
	vscycle

all:			$(PROGS) $(LIBS)


clean:
			rm -rf *~ $(PROGS) $(LIBS) SunWS_cache ir.out

vscycle:		$(LIBS)
			$(CC) $(CFLAGS) -I$(BACLIBRARYINCLUDES) -L$(CPLEXCLIBS) -L$(CPLEXCPPLIBS) -L$(BACLIBRARYLIBS) \
			-o vscycle $(LIBS) ../../src/*.o -lilocplex -lcplex -lsocket -lnsl -lm -lconcert $(NAUTYLIBS)/nauty.o $(NAUTYLIBS)/nautil.o $(NAUTYLIBS)/naugraph.o

vscycle.o:		vscycle.h\
			vscycle.cpp
			$(CC) $(CFLAGS) -I$(BACLIBRARYINCLUDES) -o vscycle.o -c vscycle.cpp
