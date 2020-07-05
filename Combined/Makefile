# Default to optimized compile
opt?=-O3
fsanitize?=
$(info Optimization flag: $(opt))
$(info Defaults to "-O3"; use "opt=X" to change it.)

RICHDEM_GIT_HASH="-NA-"
RICHDEM_COMPILE_TIME=`date -u +'%Y-%m-%d %H:%M:%S UTC'`
export RD_CXX_FLAGS=-I../common/richdem/include \
	-DRICHDEM_GIT_HASH="\"$(RICHDEM_GIT_HASH)\"" \
	-DRICHDEM_COMPILE_TIME="\"$(RICHDEM_COMPILE_TIME)\""
export CXXFLAGS=--std=c++17 ${opt} -g -Wall -Wno-unknown-pragmas
# use -fsanitize=address (above) to check for memory errors
export LIBS=-lnetcdf -fopenmp
# After compiling Pybind11 here; should also work if you apt install it
# (in which case the first -I will not matter, but the Python one may still)
export PYBINDFLAGS=-shared -fPIC `python3 -m pybind11 --includes`

objectsTWSM = twsm.o parameters.o richdem.o ArrayPack.o \
	      transient_groundwater.o
objectsDH = run_dephier.o parameters.o richdem.o ArrayPack.o
objectsPYTWSM = pytwsm.o parameters.o richdem.o ArrayPack.o \
	      transient_groundwater.o

all : pytwsm.custom-pyconfig twsm.x dephier.x

dephier.x : $(objectsDH)
	g++ $(CXXFLAGS) $(RD_CXX_FLAGS) $(objectsDH) $(LIBS) -o dephier.x

run_dephier.o : run_dephier.cpp
	g++ -c -fPIC $(CXXFLAGS) $(RD_CXX_FLAGS) run_dephier.cpp $(LIBS) \
		-o run_dephier.o

twsm.x : $(objectsTWSM)
	g++ $(CXXFLAGS) $(RD_CXX_FLAGS) $(objectsTWSM) $(LIBS) \
			-o twsm.x

twsm.o : TWSM.cpp
	g++ -c -fPIC $(CXXFLAGS) $(RD_CXX_FLAGS) TWSM.cpp $(LIBS) -o twsm.o

transient_groundwater.o : transient_groundwater.cpp
	g++ -c -fPIC $(CXXFLAGS) $(RD_CXX_FLAGS) transient_groundwater.cpp $(LIBS) -o transient_groundwater.o

richdem.o : ../common/richdem/include/richdem/richdem.cpp
	g++ -fPIC -c $(CXXFLAGS) $(RD_CXX_FLAGS) ../common/richdem/include/richdem/richdem.cpp $(LIBS) -o richdem.o

ArrayPack.o : ArrayPack.cpp  ArrayPack.hpp
	g++ -fPIC -c $(CXXFLAGS) $(RD_CXX_FLAGS) ArrayPack.cpp $(LIBS) -o ArrayPack.o

parameters.o : parameters.cpp parameters.hpp
	g++ -fPIC -c $(CXXFLAGS) $(RD_CXX_FLAGS) parameters.cpp $(LIBS) -o parameters.o

pytwsm.custom-pyconfig : $(objectsPYTWSM)
	g++ $(CXXFLAGS) $(RD_CXX_FLAGS) $(PYBINDFLAGS) $(objectsPYTWSM) $(LIBS) \
		-o pytwsm`python3-config --extension-suffix`

pytwsm.o : pytwsm.cpp
	g++ -c -fPIC $(CXXFLAGS) $(RD_CXX_FLAGS) \
		$(PYBINDFLAGS) pytwsm.cpp $(LIBS) -o pytwsm.o

clean:
	rm twsm.x
	rm dephier.x
	rm pytwsm`python3-config --extension-suffix`

distclean:
	rm $(objectsTWSM)
	rm run_dephier.o
	rm pytwsm.o

