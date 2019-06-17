## Makefile - generic for CPLEX programs
## @author Matt Stallmann, 2019-05-02

# object and header files used for utilities used by cplex_ilp
OBJECTS = CmdLine.o
HEADERS = CmdLine.h ClockTimer.h

# Executables
EXECS = cplex_ilp

# seems to be universal; if not, modify build.sh accordingly
LIBFORMAT  = static_pic

CPLEXDIR      = $(ROOT_DIR)/cplex
CONCERTDIR    = $(ROOT_DIR)/concert

# ---------------------------------------------------------------------
# Compiler selection 
# ---------------------------------------------------------------------

CCC = g++

# ---------------------------------------------------------------------
# Compiler options 
# ---------------------------------------------------------------------

CCOPT = -Wall -g -O -fexceptions -fPIC -DUNIX -DNDEBUG -DIL_STD

# ---------------------------------------------------------------------
# Link options and libraries
# ---------------------------------------------------------------------

CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT) 
CPLEXLIBDIR = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT) 
CCLNFLAGS = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR) -lilocplex -lcplex -lconcert -lm -lpthread -ldl $(DISTMIP)

# ---------------------------------------------------------------------
# Includes and complete compiler flags
# ---------------------------------------------------------------------

CONCERTINCDIR = $(CONCERTDIR)/include/
CPLEXINCDIR   = $(CPLEXDIR)/include/

CCFLAGS = $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) 

# ------------------------------------------------------------
.SUFFIXES: .cpp
.cpp.o:
	$(CCC) -g -Wall -c -O $*.cpp

all : $(EXECS)

clean :
	/bin/rm -rf *.o *~ $(EXECS)

# ------------------------------------------------------------
#
# The examples
#

cplex_ilp: cplex_ilp.o $(OBJECTS) Makefile
	$(CCC) $(CCFLAGS) cplex_ilp.o $(OBJECTS) -o cplex_ilp $(CCLNFLAGS)
cplex_ilp.o: cplex_ilp.cpp $(HEADERS) Makefile
	$(CCC) -c $(CCFLAGS) cplex_ilp.cpp -o cplex_ilp.o

CmdLine.o: CmdLine.cpp CmdLine.h Makefile

StrNode.o: StrNode.cpp StrNode.h Makefile

StrTabNode.o: StrTabNode.cpp StrTabNode.h Makefile

StrQueue.o: StrQueue.cpp StrQueue.h StrNode.h Makefile

StrTable.o: StrTable.cpp StrTable.h StrTabNode.h Makefile

install : cplex_ilp
	mv cplex_ilp ~/bin
	/bin/rm -rf *.o *~
