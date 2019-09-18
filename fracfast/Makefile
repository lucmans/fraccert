##
# Makefile: the makefile for fraccert
# @author Luc de Jonckheere
##

# Min required -std=c++11


# General compiler flags
CXX = g++
CXXFLAGS = -std=c++11 -fopenmp
WARNINGS = -Wall -Wextra -Wfloat-equal
OPTIMIZATION = -O2 -ffast-math
LIBS = -lgmp
CORES = 8

# Library building and linking info
LIBNAME = fracfast
OBJ = shapes.o fractal.o mandelbrot.o julia.o


all: static shared

static:
	make -j 8 -B lib$(LIBNAME).a

shared:
	make -j 8 -B lib$(LIBNAME).so


lib$(LIBNAME).a: $(OBJ)
	ar rcs $@ $^

lib$(LIBNAME).so: SHAREDCOMP := -fPIC
lib$(LIBNAME).so: $(OBJ)
	g++ $(OPTIMIZATION) -shared -Wl,-soname,$@ -o $@ $^

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) $(SHAREDCOMP) $(WARNINGS) $(OPTIMIZATION) -c $< -o $@


# For studying the generated assembly
%.s: %.cpp  %.h
	$(CXX) -S -fverbose-asm -g -O2 $<


# Various 'script'
test:
	./fraccert -t &> testlog.txt

lines:
	wc -l *.h *.cpp

todo:
	grep -n TODO *.cpp *.h

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f *.s
	rm -f vgcore*
