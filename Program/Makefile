
SYSTEM     = x86-64_linux
LIBFORMAT  = static_pic
CCC = g++
BASEFOLDER =  /opt/ibm/ILOG
CPLEXVERSION = CPLEX_Studio1210
ifeq ($(myOS), MacOSX)
	SYSTEM     = x86-64_osx
	LIBFORMAT  = static_pic
	CCC = clang++
	BASEFOLDER = /Applications
endif

CPLEXDIR       = $(BASEFOLDER)/$(CPLEXVERSION)/cplex
CONCERTDIR     = $(BASEFOLDER)/$(CPLEXVERSION)/concert

CCOPT = -m64 -O3 -w -fPIC -fno-strict-aliasing -fexceptions -DNDEBUG -DIL_STD -std=c++14

CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CPLEXINCDIR   = $(CPLEXDIR)/include
CONCERTINCDIR = $(CONCERTDIR)/include
CCFLAGS = $(CCOPT) -I$(CONCERTINCDIR) -I$(CPLEXINCDIR) 
CCLNDIRS  = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR)
CCLNFLAGS = -lconcert -lilocplex -lcplex -lm -pthread -ldl
TARGETDIR = .

OBJS2 = \
	$(TARGETDIR)/commandline.o \
	$(TARGETDIR)/main.o \
    	$(TARGETDIR)/Pb_Data.o \
        $(TARGETDIR)/Solution.o \
        $(TARGETDIR)/Solver_Primal.o \
        $(TARGETDIR)/Solver_Sampling.o \
        $(TARGETDIR)/Solver_CombinatorialBenders.o 

# ------------------------------------------------------------
EXEC=svm-nonconvex

all:$(EXEC)

$(EXEC): libsvm $(OBJS2)
	$(CCC) $(CCFLAGS) $(CCLNDIRS) -o $(EXEC) $(TARGETDIR)/libsvm-3.24/svm.o $(OBJS2) $(CCLNFLAGS) 

$(TARGETDIR)/Pb_Data.o: Pb_Data.h Pb_Data.cpp
	$(CCC) $(CCFLAGS)  $(CPPFLAGS) -c Pb_Data.cpp -o $(TARGETDIR)/Pb_Data.o   	

$(TARGETDIR)/Solution.o: Solution.h Solution.cpp
	$(CCC) $(CCFLAGS)  $(CPPFLAGS) -c Solution.cpp -o $(TARGETDIR)/Solution.o  	

$(TARGETDIR)/Solver_Primal.o: Solver_Primal.h Solver_Primal.cpp
	$(CCC) $(CCFLAGS)  $(CPPFLAGS) -c Solver_Primal.cpp -o $(TARGETDIR)/Solver_Primal.o  	
	
$(TARGETDIR)/main.o: main.cpp
	$(CCC) $(CCFLAGS) $(LIBS)  $(CPPFLAGS) -c main.cpp -o $(TARGETDIR)/main.o  	

$(TARGETDIR)/Solver_Sampling.o: Solver_Sampling.h Solver_Sampling.cpp
	$(CCC) $(CCFLAGS)  $(CPPFLAGS) -c Solver_Sampling.cpp -o $(TARGETDIR)/Solver_Sampling.o  	

$(TARGETDIR)/Solver_CombinatorialBenders.o: Solver_CombinatorialBenders.h Solver_CombinatorialBenders.cpp
	$(CCC) $(CCFLAGS)  $(CPPFLAGS) -c Solver_CombinatorialBenders.cpp -o $(TARGETDIR)/Solver_CombinatorialBenders.o 	

$(TARGETDIR)/commandline.o: commandline.h commandline.cpp
	$(CCC) $(CCFLAGS)  $(CPPFLAGS) -c commandline.cpp -o $(TARGETDIR)/commandline.o  	

libsvm: 
	cd libsvm-3.24 && make

clean: cleanlibsvm
	rm -rf *.o $(EXEC)

cleanlibsvm:
	cd libsvm-3.24 && make clean

test: clean all
	./svm-nonconvex ../Datasets/Brooks_SVM_Data/n60d2ATj0 -instanceFormat 2 -problem SAMPLING_CB_HARD_IP -pen 1 -timeBudget 30 -seed 1 -timeProportionCBCuts 0.2 -timeProportionSampling 0.5 -maxCBCuts -1