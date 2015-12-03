# Makefile
#
# By Sebastian Raaphorst, 2003

# *** Modify these to suit your local configuration. ***
HEADERINSTALLPATH=$(HOME)/include/nibac
LIBRARYINSTALLPATH=$(HOME)/lib
NAUTYBASEPATH=$(HOME)/lib/nauty22
NAUTYINCLUDEPATH=$(NAUTYBASEPATH)
NAUTYLIBRARYPATH=$(NAUTYBASEPATH)

# *** Comment one or the other out, depending on architecture. ***
SUFFIX=dylib
# SUFFIX=so
LINKERFLAGS=-dynamiclib -install_name libnibac.dylib
# LINKERFLAGS=-shared -W,-soname,libnibac.so -lc

# *** NOTE: Only one solver can be used at compile time or conflicts arise. ***
#     Ensure that the others are commented out. In this default, GLPK is
#     enabled and the others are disabled.

# *** Modify for GPLK if using. ***
GLPKHEADERPATH=/usr/local/glpk-4.47/include
GLPKLIBRARYPATH=/usr/local/glpk-4.47/lib
GLPKINCLUDES=-I$(GLPKHEADERPATH)
GLPKLIBS=-L$(GLPKLIBRARYPATH) -lglpk
SOLVER=glpk

# *** Modify for CPLEX if using. ***
# Note that NIBAC currently supports CPLEX 7.x and 8.x.
# It may work with more recent versions using the 8.x line, but this
# is not guaranteed.
#CPLEXBASEDIR=/local/comb/sfw/sparc_SunOS_5.9/ilog

# CPLEX 7.x includes
#CPLEXCHEADERPATH=$(CPLEXBASEDIR)/cplex70/include
#CPLEXCPPHEADERPATH=$(CPLEXBASEDIR)/concert10/include
#CPLEXCLIBRARYPATH=$(CPLEXBASEDIR)/cplex70/lib/ultrasparc_5_5.0/static_pic_mt
#CPLEXCPPLIBRARYPATH=$(CPLEXBASEDIR)/concert10/lib/ultrasparc_5_5.0/static_pic_mt
#CPLEXCFLAGS=-DCPLEX7

# CPLEX 8.x includes
#CPLEXCHEADERPATH=$(CPLEXBASEDIR)/cplex81/include
#CPLEXCPPHEADERPATH=$(CPLEXBASEDIR)/concert13/include
#CPLEXCLIBRARYPATH=$(CPLEXBASEDIR)/cplex81/lib/ultrasparc_5_5.0/static_pic_mt
#CPLEXCPPLIBRARYPATH=$(CPLEXBASEDIR)/concert13/lib/ultrasparc_5_5.0/static_pic_mt
#CPLEXCFLAGS=-DCPLEX8

# Uncomment this for CPLEX.
#SOLVER=cplex

# Final CPLEX configuration. Do not modify unless you know what you're doing.
#CPLEXINCLUDES=-I$(CPLEXCHEADERPATH) -I$(CPLEXCPPHEADERPATH)
#CPLEXLIBS=-L$(CPLEXCLIBRARYPATH) -L$(CPLEXCPPLIBRARYPATH) -lilocplex -lcplex -lsocket -lnsl -lm -lconcert

# Tools and necessary flags
# *** Modify the OBJCFLAGS to obtain the desired options ***
# Here is a summary of the compile time options:
# DEBUG
#	outputs debugging information to the standard error channel
# DNDEBUG
#	suppress any debugs and assertions
# SSDEBUG
#	outputs extra debugging information for the Schreier-Sims scheme
# NODEGROUPS
#	use one group per node instead of one group per tree; reduces
#	dramatically the amount of reverse downing that needs to be done
#	and is HIGHLY recommended
# NOPERMPOOL
#	do not use a pool of permutations, i.e. allocate memory for
#	permutations on the fly instead of grabbing from a pool; may
#	reduce performance slightly but is more stable
# RECURSIVEENTER
#	use Margot's recursive definition of enter for the Schreier-Sims
#	scheme instead of a looping one; may affect performance slightly
# MARGOTTIMERS
#	add additional timers to time Margot's algorithms; output at end
# USEUSETS
#	specifies that, in addition to lists of permutations for the
#	Schreier-Sims scheme, we also maintain a table of permutations
#	- the advantage to this is that permutation lookups become
#	constant, but the disadvantage is the huge amount of memory in
#	terms of pointer allocation that such a table takes to store
CC=g++
LD=g++
OBJCFLAGS=-fno-common -O3 -Wall -ansi -DNDEBUG -DNODEGROUPS -DNOPERMPOOL -DRECURSIVEENTER -DMARGOTTIMERS
#OBJCFLAGS=-fno-common -g -Wall -ansi -DDEBUG -DNODEGROUPS -DNOPERMPOOL -DRECURSIVEENTER -DMARGOTTIMERS
#OBJCFLAGS=-fPIC -O3 -Wall -ansi -DNDEBUG -DNODEGROUPS -DNOPERMPOOL -DRECURSIVEENTER -DMARGOTTIMERS
BINCFLAGS=-O3 -Wall -ansi
INCLUDES=-I. -I$(NAUTYINCLUDEPATH)

# Only one of these had better be defined or problems will arise.
LIBS=$(GLPKLIBS) $(CPLEXLIBS)

# Object files created by this library.
PLUGINOBJ=$(SOLVER)solver.o
OBJS=\
	bac.o \
	bacoptions.o \
	bitstring.o \
	block.o \
	blockgroup.o \
	branchingscheme.o \
	cliquecutproducer.o \
	closestvaluebranchingscheme.o \
	column.o \
	commandlineprocessing.o \
	constraint.o \
	cutproducer.o \
	defaultsolutionmanager.o \
	formulation.o \
	generatedgroup.o \
	graph.o \
	group.o \
	immediatesolutionmanager.o \
	isomorphismcut.o \
	isomorphismcutproducer.o \
	lexicographicvariableorder.o \
	lowestindexbranchingscheme.o \
	lpsolver.o \
	margotbac.o \
	margotbacoptions.o \
	matrix.o \
	matrixgroup.o \
	nibacexception.o \
	node.o \
	nodestack.o \
	permutationpool.o \
	problem.o \
	randomvariableorder.o \
	rankedbranchingscheme.o \
	schreiersimsgroup.o \
	solutionmanager.o \
	statistics.o \
	subsetproducer.o \
	superduper.o \
	timer.o \
	util.o \
	variableorder.o

# Header files pertinent to this library.
PLUGINHEADER=$(SOLVER)solver.h
HEADERS=\
	bac.h \
	baclibrary.h \
	bacoptions.h \
	bitstring.h \
	block.h \
	blockgroup.h \
	branchingscheme.h \
	cliquecutproducer.h \
	closestvaluebranchingscheme.h \
	column.h \
	commandlineprocessing.h \
	common.h \
	constraint.h \
	cutproducer.h \
	defaultsolutionmanager.h \
	formulation.h \
	generatedgroup.h \
	graph.h \
	group.h \
	immediatesolutionmanager.h \
	isomorphismcut.h \
	isomorphismcutproducer.h \
	lexicographicvariableorder.h \
	lowestindexbranchingscheme.h \
	lpsolver.h \
	margotbac.h \
	margotbacoptions.h \
	matrix.h \
	matrixgroup.h \
	nibacexception.h \
	node.h \
	nodestack.h \
	permutationpool.h \
	problem.h \
	randomvariableorder.h \
	rankedbranchingscheme.h \
	schreiersimsgroup.h \
	solutionmanager.h \
	statistics.h \
	subsetproducer.h \
	superduper.h \
	timer.h \
	util.h \
	variableorder.h

all:				libnibac.$(SUFFIX)

install:			all
				mkdir -p $(HEADERINSTALLPATH)
				mkdir -p $(LIBRARYINSTALLPATH)
				cp -f $(PLUGINHEADER) $(HEADERS) $(HEADERINSTALLPATH)
				cp -f libnibac.$(SUFFIX) $(LIBRARYINSTALLPATH)

clean:
				rm -rf *~ $(PLUGINOBJ) $(OBJS) libnibac.$(SUFFIX)


libnibac.$(SUFFIX):		$(PLUGINOBJ) $(OBJS)
				$(LD) $(LINKERFLAGS) -o libnibac.$(SUFFIX) $(PLUGINOBJ) $(OBJS) \
					$(NAUTYLIBRARYPATH)/nauty.o \
					$(NAUTYLIBRARYPATH)/nautil.o \
					$(NAUTYLIBRARYPATH)/naugraph.o $(LIBS)

bac.o:				common.h\
				bac.h\
				bac.cpp\
				formulation.h\
				group.h\
				solutionmanager.h\
				cutproducer.h\
				nibacexception.h\
				nodestack.h\
				node.h\
				lpsolver.h\
				statistics.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o bac.o -c bac.cpp

bacoptions.o:			common.h\
				bacoptions.h\
				bacoptions.cpp\
				branchingscheme.h\
				cutproducer.h\
				nibacexception.h\
				solutionmanager.h\
				statistics.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o bacoptions.o -c bacoptions.cpp

bitstring.o:			common.h\
				bitstring.h\
				bitstring.cpp
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o bitstring.o -c bitstring.cpp

block.o:			common.h\
				block.h\
				block.cpp\
				superduper.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o block.o -c block.cpp

blockgroup.o:			common.h\
				blockgroup.h\
				blockgroup.cpp\
				permutationpool.h\
				superduper.h\
				schreiersimsgroup.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o blockgroup.o -c blockgroup.cpp

branchingscheme.o:		common.h\
				branchingscheme.h\
				branchingscheme.cpp
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o branchingscheme.o -c branchingscheme.cpp

cliquecutproducer.o:		common.h\
				cliquecutproducer.h\
				cliquecutproducer.cpp\
				bac.h\
				node.h\
				group.h\
				graph.h\
				column.h\
				cutproducer.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o cliquecutproducer.o -c cliquecutproducer.cpp

closestvaluebranchingscheme.o:	common.h\
				closestvaluebranchingscheme.h\
				closestvaluebranchingscheme.cpp\
				nibacexception.h\
				node.h\
				rankedbranchingscheme.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o closestvaluebranchingscheme.o -c closestvaluebranchingscheme.cpp

column.o:			common.h\
				column.h\
				column.cpp
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o column.o -c column.cpp

commandlineprocessing.o:	common.h\
				commandlineprocessing.h\
				commandlineprocessing.cpp\
				bacoptions.h\
				branchingscheme.h\
				cutproducer.h\
				margotbacoptions.h\
				nibacexception.h\
				solutionmanager.h\
				util.h\
				variableorder.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o commandlineprocessing.o -c commandlineprocessing.cpp

constraint.o:			common.h\
				constraint.h\
				constraint.cpp\
				formulation.h\
				lpsolver.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o constraint.o -c constraint.cpp

cplexsolver.o:			common.h\
				cplexsolver.h\
				cplexsolver.cpp\
				formulation.h\
				bac.h\
				bacoptions.h\
				node.h\
				lpsolver.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) $(CPLEXCINCLUDES) -o cplexsolver.o -c cplexsolver.cpp

cutproducer.o:			common.h\
				cutproducer.h\
				cutproducer.cpp\
				bac.h\
				node.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o cutproducer.o -c cutproducer.cpp

defaultsolutionmanager.o:	common.h\
				defaultsolutionmanager.h\
				defaultsolutionmanager.cpp\
				node.h\
				solutionmanager.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o defaultsolutionmanager.o -c defaultsolutionmanager.cpp

formulation.o:			common.h\
				formulation.h\
				formulation.cpp\
				lpsolver.h\
				constraint.h\
				column.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o formulation.o -c formulation.cpp

generatedgroup.o:		common.h\
				generatedgroup.h\
				generatedgroup.cpp\
				bitstring.h\
				formulation.h\
				group.h\
				nibacexception.h\
				permutationpool.h\
				timer.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o generatedgroup.o -c generatedgroup.cpp

glpksolver.o:			common.h\
				glpksolver.h\
				glpksolver.cpp\
				bac.h\
				formulation.h\
				nibacexception.h\
				node.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) $(GLPKINCLUDES) -o glpksolver.o -c glpksolver.cpp

graph.o:			common.h\
				graph.h\
				graph.cpp
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o graph.o -c graph.cpp

group.o:			common.h\
				group.h\
				group.cpp\
				permutationpool.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o group.o -c group.cpp

immediatesolutionmanager.o:	common.h\
				immediatesolutionmanager.h\
				immediatesolutionmanager.cpp\
				node.h\
				solutionmanager.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o immediatesolutionmanager.o -c immediatesolutionmanager.cpp

isomorphismcut.o:		common.h\
				isomorphismcut.h\
				isomorphismcut.cpp
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o isomorphismcut.o -c isomorphismcut.cpp

isomorphismcutproducer.o:	common.h\
				isomorphismcutproducer.h\
				isomorphismcutproducer.cpp\
				isomorphismcut.h\
				bac.h\
				margotbac.h\
				permutationpool.h\
				node.h\
				group.h\
				cutproducer.h\
				lpsolver.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o isomorphismcutproducer.o -c isomorphismcutproducer.cpp

lexicographicvariableorder.o:	common.h\
				lexicographicvariableorder.h\
				lexicographicvariableorder.cpp\
				variableorder.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o lexicographicvariableorder.o -c lexicographicvariableorder.cpp

lowestindexbranchingscheme.o:	common.h\
				lowestindexbranchingscheme.h\
				lowestindexbranchingscheme.cpp\
				branchingscheme.h\
				node.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o lowestindexbranchingscheme.o -c lowestindexbranchingscheme.cpp

lpsolver.o:			common.h\
				lpsolver.h\
				lpsolver.cpp\
				formulation.h\
				bac.h\
				node.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o lpsolver.o -c lpsolver.cpp

margotbac.o:			common.h\
				margotbac.h\
				margotbac.cpp\
				bac.h\
				bacoptions.h\
				margotbacoptions.h\
				formulation.h\
				statistics.h\
				node.h\
				cutproducer.h\
				solutionmanager.h\
				group.h\
				lpsolver.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o margotbac.o -c margotbac.cpp

margotbacoptions.o:		common.h\
				margotbacoptions.h\
				margotbacoptions.cpp\
				bacoptions.h\
				nibacexception.h\
				variableorder.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o margotbacoptions.o -c margotbacoptions.cpp

matrix.o:			common.h\
				matrix.h\
				matrix.cpp
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o matrix.o -c matrix.cpp

matrixgroup.o:			common.h\
				matrixgroup.h\
				matrixgroup.cpp\
				schreiersimsgroup.h\
				permutationpool.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o matrixgroup.o -c matrixgroup.cpp

nibacexception.o:		common.h\
				nibacexception.h\
				nibacexception.cpp
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o nibacexception.o -c nibacexception.cpp

node.o:				common.h\
				node.h\
				node.cpp\
				bac.h\
				bacoptions.h\
				lpsolver.h\
				constraint.h\
				formulation.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o node.o -c node.cpp

nodestack.o:			common.h\
				nodestack.h\
				nodestack.cpp\
				branchingscheme.h\
				lpsolver.h\
				node.h\
				statistics.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o nodestack.o -c nodestack.cpp

permutationpool.o:		common.h\
				permutationpool.h\
				permutationpool.cpp
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o permutationpool.o -c permutationpool.cpp

problem.o:			common.h\
				problem.h\
				problem.cpp\
				formulation.h\
				group.h\
				schreiersimsgroup.h\
				bac.h\
				margotbac.h\
				solutionmanager.h\
				defaultsolutionmanager.h\
				immediatesolutionmanager.h\
				cutproducer.h\
				isomorphismcutproducer.h\
				cliquecutproducer.h\
				bacoptions.h\
				margotbacoptions.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o problem.o -c problem.cpp

randomvariableorder.o:		common.h\
				randomvariableorder.h\
				randomvariableorder.cpp\
				variableorder.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o randomvariableorder.o -c randomvariableorder.cpp

rankedbranchingscheme.o:	common.h\
				rankedbranchingscheme.h\
				rankedbranchingscheme.cpp\
				branchingscheme.h\
				nibacexception.h\
				node.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o rankedbranchingscheme.o -c rankedbranchingscheme.cpp

schreiersimsgroup.o:		common.h\
				schreiersimsgroup.h\
				schreiersimsgroup.cpp\
				generatedgroup.h\
				nibacexception.h\
				permutationpool.h\
				timer.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o schreiersimsgroup.o -c schreiersimsgroup.cpp

solutionmanager.o:		common.h\
				solutionmanager.h\
				solutionmanager.cpp
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o solutionmanager.o -c solutionmanager.cpp

statistics.o:			common.h\
				statistics.h\
				statistics.cpp\
				timer.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o statistics.o -c statistics.cpp

subsetproducer.o:		common.h\
				subsetproducer.h\
				subsetproducer.cpp
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o subsetproducer.o -c subsetproducer.cpp

superduper.o:			superduper.h\
				superduper.c
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o superduper.o -c superduper.c

timer.o:			common.h\
				timer.h\
				timer.cpp
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o timer.o -c timer.cpp

util.o:				common.h\
				util.h\
				util.cpp
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o util.o -c util.cpp

variableorder.o:		common.h\
				variableorder.h\
				variableorder.cpp\
				nibacexception.h
				$(CC) $(OBJCFLAGS) $(INCLUDES) -o variableorder.o -c variableorder.cpp
